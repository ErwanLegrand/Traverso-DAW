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
 
    $Id: RingBuffer.h,v 1.3 2006/08/31 12:38:35 r_sijrier Exp $
*/

#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H


#include <sys/types.h>
#include "defines.h"


typedef struct
{
        char  *buf;
        size_t len;
}
ringbuffer_data_t ;


/** 
* The key attribute of a ringbuffer is that it can be safely accessed
* by two threads simultaneously -- one reading from the buffer and
* the other writing to it -- without using any synchronization or
* mutual exclusion primitives.  For this to work correctly, there can
* only be a single reader and a single writer thread.  Their
* identities cannot be interchanged.
*/
class RingBuffer
{

public:
        RingBuffer(size_t size);
        ~RingBuffer();

        /**
        * Fill a data structure with a description of the current readable
        * data held in the ringbuffer.  This description is returned in a two
        * element array of ringbuffer_data_t.  Two elements are needed
        * because the data to be read may be split across the end of the
        * ringbuffer.
        *
        * The first element will always contain a valid @a len field, which
        * may be zero or greater.  If the @a len field is non-zero, then data
        * can be read in a contiguous fashion using the address given in the
        * corresponding @a buf field.
        *
        * If the second element has a non-zero @a len field, then a second
        * contiguous stretch of data can be read from the address given in
        * its corresponding @a buf field.
        *
        * @param vec a pointer to a 2 element array of ringbuffer_data_t.
        *
        */
        void get_read_vector(ringbuffer_data_t *vec);

        /**
        * Fill a data structure with a description of the current writable
        * space in the ringbuffer.  The description is returned in a two
        * element array of ringbuffer_data_t.  Two elements are needed
        * because the space available for writing may be split across the end
        * of the ringbuffer.
        *
        * The first element will always contain a valid @a len field, which
        * may be zero or greater.  If the @a len field is non-zero, then data
        * can be written in a contiguous fashion using the address given in
        * the corresponding @a buf field.
        *
        * If the second element has a non-zero @a len field, then a second
        * contiguous stretch of data can be written to the address given in
        * the corresponding @a buf field.
        *
        * @param vec a pointer to a 2 element array of ringbuffer_data_t.
        */
        void get_write_vector(ringbuffer_data_t *vec);

        /**
        * Read data from the ringbuffer.
        *
        * @param dest a pointer to a buffer where data read from the
        * ringbuffer will go.
        * @param cnt the number of bytes to read.
        *
        * @return the number of bytes read, which may range from 0 to cnt.
        */
        size_t read(char *dest, size_t cnt);

        /**
        * Read data from the ringbuffer. Opposed to read()
        * this function does not move the read pointer. Thus it's
        * a convenient way to inspect data in the ringbuffer in a
        * continous fashion. The price is that the data is copied
        * into a user provided buffer. For "raw" non-copy inspection
        * of the data in the ringbuffer use get_read_vector().
        *
        * @param dest a pointer to a buffer where data read from the
        * ringbuffer will go.
        * @param cnt the number of bytes to read.
        *
        * @return the number of bytes read, which may range from 0 to cnt.
        */
        size_t peek(char *dest, size_t cnt);

        /**
        * Advance the read pointer.
        *
        * After data have been read from the ringbuffer using the pointers
        * returned by get_read_vector(), use this function to
        * advance the buffer pointers, making that space available for future
        * write operations.
        *
        * @param cnt the number of bytes read.
        */
        void read_advance(size_t cnt);

        /**
        * Return the number of bytes available for reading.
        *
        * @return the number of bytes available to read.
        */
        size_t read_space();

        /**
        * Lock a ringbuffer data block into memory.
        *
        * Uses the mlock() system call.  This is not a realtime operation.
        */
        int mlock_buffer();

        /**
        * Reset the read and write pointers, making an empty buffer.
        *
        * This is not thread safe.
        */
        void reset();

        /**
        * Write data into the ringbuffer.
        *
        * @param src a pointer to the data to be written to the ringbuffer.
        * @param cnt the number of bytes to write.
        *
        * @return the number of bytes write, which may range from 0 to cnt
        */
        size_t write(const char *src, size_t cnt);

        /**
        * Advance the write pointer.
        *
        * After data have been written the ringbuffer using the pointers
        * returned by get_write_vector(), use this function
        * to advance the buffer pointer, making the data available for future
        * read operations.
        *
        * @param cnt the number of bytes written.
        */
        void write_advance(size_t cnt);

        /**
        * Return the number of bytes available for writing.
        *
        * @return the amount of free space (in bytes) available for writing.
        */
        size_t write_space();

private:
        char			*buf;
        volatile size_t 	write_ptr;
        volatile size_t 	read_ptr;
        size_t	  		size;
        size_t	  		size_mask;
        int			mlocked;

};

#endif
