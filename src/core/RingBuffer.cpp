/*
  Copyright (C) 2006 Remon Sijrier
  Ported to C++ for use in Traverso

  Copyright (C) 2000 Paul Davis
  Copyright (C) 2003 Rohan Drape

  This file is part of Traverso

  Traverso is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

  ISO/POSIX C version of Paul Davis's lock free ringbuffer C++ code.
  This is safe for the case of one read thread and one write thread.
*/

#include <stdlib.h>
#include <string.h>
#ifdef USE_MLOCK

#ifdef MAC_OS_BUILD
#include <sys/types.h>
#endif /* MAC_OS_BUILD */

#include <sys/mman.h>
#endif /* USE_MLOCK */

#include "RingBuffer.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/* Create a new ringbuffer to hold at least `sz' bytes of data. The
   actual buffer size is rounded up to the next power of two.  */

RingBuffer::RingBuffer(size_t sz)
{
        int power_of_two;

        for (power_of_two = 1; 1 << power_of_two < (int)sz; power_of_two++)
                ;

        size = 1 << power_of_two;
        size_mask = size;
        size_mask -= 1;
        write_ptr = 0;
        read_ptr = 0;
        buf = new char[size]; 
        mlocked = 0;
}


RingBuffer::~RingBuffer()
{
#ifdef USE_MLOCK
        if (mlocked) {
                munlock (buf, size);
        }
#endif /* USE_MLOCK */
        delete [] buf;
}

/* Lock the data block using the system call 'mlock'.  */

int RingBuffer::mlock_buffer()
{
#if defined (USE_MLOCK)
        if (mlock (buf, size)) {
                return -1;
        }
        mlocked = 1;
#endif /* USE_MLOCK */

        return 0;
}

/* Reset the read and write pointers to zero. This is not thread
   safe. */

void RingBuffer::reset ()
{
        read_ptr = 0;
        write_ptr = 0;
}

/* Return the number of bytes available for reading.  This is the
   number of bytes in front of the read pointer and behind the write
   pointer.  */

size_t RingBuffer::read_space ()
{
        size_t w, r;

        w = write_ptr;
        r = read_ptr;

        if (w > r) {
                return w - r;
        } else {
                return (w - r + size) & size_mask;
        }
}

/* Return the number of bytes available for writing.  This is the
   number of bytes in front of the write pointer and behind the read
   pointer.  */

size_t RingBuffer::write_space ()
{
        size_t w, r;

        w = write_ptr;
        r = read_ptr;

        if (w > r) {
                return ((r - w + size) & size_mask) - 1;
        } else if (w < r) {
                return (r - w) - 1;
        } else {
                return size - 1;
        }
}

/* The copying data reader.  Copy at most `cnt' bytes to
   `dest'.  Returns the actual number of bytes copied. */

size_t RingBuffer::read (char *dest, size_t cnt)
{
        size_t free_cnt;
        size_t cnt2;
        size_t to_read;
        size_t n1, n2;

        if ( ( free_cnt = read_space () ) == 0) {
                return 0;
        }

        to_read = cnt > free_cnt ? free_cnt : cnt;

        cnt2 = read_ptr + to_read;

        if (cnt2 > size) {
                n1 = size - read_ptr;
                n2 = cnt2 & size_mask;
        } else {
                n1 = to_read;
                n2 = 0;
        }

        memcpy (dest, &(buf[read_ptr]), n1 * sizeof(char));
        read_ptr += n1;
        read_ptr &= size_mask;

        if (n2) {
                memcpy (dest + n1, &(buf[read_ptr]), n2 * sizeof(char));
                read_ptr += n2;
                read_ptr &= size_mask;
        }

        return to_read;
}

/* The copying data reader w/o read pointer advance.  Copy at most
   `cnt' bytes from `dest'.  Returns the actual number of bytes copied. */

size_t RingBuffer::peek (char *dest, size_t cnt)
{
        size_t free_cnt;
        size_t cnt2;
        size_t to_read;
        size_t n1, n2;
        size_t tmp_read_ptr;

        tmp_read_ptr = read_ptr;

        if ((free_cnt = read_space ()) == 0) {
                return 0;
        }

        to_read = cnt > free_cnt ? free_cnt : cnt;

        cnt2 = tmp_read_ptr + to_read;

        if (cnt2 > size) {
                n1 = size - tmp_read_ptr;
                n2 = cnt2 & size_mask;
        } else {
                n1 = to_read;
                n2 = 0;
        }

        memcpy (dest, &(buf[tmp_read_ptr]), n1);
        tmp_read_ptr += n1;
        tmp_read_ptr &= size_mask;

        if (n2) {
                memcpy (dest + n1, &(buf[tmp_read_ptr]), n2);
                tmp_read_ptr += n2;
                tmp_read_ptr &= size_mask;
        }

        return to_read;
}


/* The copying data writer.  Copy at most `cnt' bytes to `rb' from
   `src'.  Returns the actual number of bytes copied. */

size_t RingBuffer::write (const char *src, size_t cnt)
{
        size_t free_cnt;
        size_t cnt2;
        size_t to_write;
        size_t n1, n2;

        if ((free_cnt = write_space ()) == 0) {
                return 0;
        }

        to_write = cnt > free_cnt ? free_cnt : cnt;

        cnt2 = write_ptr + to_write;

        if (cnt2 > size) {
                n1 = size - write_ptr;
                n2 = cnt2 & size_mask;
        } else {
                n1 = to_write;
                n2 = 0;
        }

        memcpy (&(buf[write_ptr]), src, n1 * sizeof(char));
        write_ptr += n1;
        write_ptr &= size_mask;

        if (n2) {
                memcpy (&(buf[write_ptr]), src + n1, n2 * sizeof(char));
                write_ptr += n2;
                write_ptr &= size_mask;
        }

        return to_write;
}

/* Advance the read pointer `cnt' places. */

void RingBuffer::read_advance (size_t cnt)
{
        read_ptr += cnt;
        read_ptr &= size_mask;
}

/* Advance the write pointer `cnt' places. */

void RingBuffer::write_advance (size_t cnt)
{
        write_ptr += cnt;
        write_ptr &= size_mask;
}

/* The non-copying data reader.  `vec' is an array of two places.  Set
   the values at `vec' to hold the current readable data'.  If   the readable 
   data is in one segment the second segment has zero length.  */

void RingBuffer::get_read_vector (ringbuffer_data_t * vec)
{
        size_t free_cnt;
        size_t cnt2;
        size_t w, r;

        w = write_ptr;
        r = read_ptr;

        if (w > r) {
                free_cnt = w - r;
        } else {
                free_cnt = (w - r + size) & size_mask;
        }

        cnt2 = r + free_cnt;

        if (cnt2 > size) {

                /* Two part vector: the rest of the buffer after the current write
                ptr, plus some from the start of the buffer. */

                vec[0].buf = &(buf[r]);
                vec[0].len = size - r;
                vec[1].buf = buf;
                vec[1].len = cnt2 & size_mask;

        } else {

                /* Single part vector: just the rest of the buffer */

                vec[0].buf = &(buf[r]);
                vec[0].len = free_cnt;
                vec[1].len = 0;
        }
}

/* The non-copying data writer.  `vec' is an array of two places.  Set
   the values at `vec' to hold the current writeable data.  If the writeable 
   data is in one segment the second segment has zero length.  */

void RingBuffer::get_write_vector (ringbuffer_data_t * vec)
{
        size_t free_cnt;
        size_t cnt2;
        size_t w, r;

        w = write_ptr;
        r = read_ptr;

        if (w > r) {
                free_cnt = ((r - w + size) & size_mask) - 1;
        } else if (w < r) {
                free_cnt = (r - w) - 1;
        } else {
                free_cnt = size - 1;
        }

        cnt2 = w + free_cnt;

        if (cnt2 > size) {

                /* Two part vector: the rest of the buffer after the current write
                ptr, plus some from the start of the buffer. */

                vec[0].buf = &(buf[w]);
                vec[0].len = size - w;
                vec[1].buf = buf;
                vec[1].len = cnt2 & size_mask;
        } else {
                vec[0].buf = &(buf[w]);
                vec[0].len = free_cnt;
                vec[1].len = 0;
        }
}

//eof
