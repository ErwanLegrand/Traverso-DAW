/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
    This file is part of Traverso
 
    Traverso is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 
    $Id: AudioClipList.cpp,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#include "AudioClipList.h"

#include "AudioClip.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


void AudioClipList::add_clip( AudioClip * clip )
{
        append(clip);
        sort();
}

int AudioClipList::remove_clip( AudioClip * clip )
{
        int index;
        if ( (index = clip_index(clip)) >= 0 ) {
                removeAt(index);
                sort();
                return 1;
        }
        return -1;
}

AudioClip * AudioClipList::next( AudioClip * clip )
{
        if ( (!isEmpty()) &&  (! (clip == last())) ) {
                int index = clip_index(clip);
                if (index >= 0)
                        return at(index + 1);
        }
        return (AudioClip*) 0;
}

AudioClip * AudioClipList::prev( AudioClip * clip )
{
        if ( (!isEmpty()) && (! (clip == first())) ) {
                int index = clip_index(clip);
                //FIXME
                if (index >= 0)
                        return at(index - 1);
        }
        return (AudioClip*) 0;
}

int AudioClipList::clip_index( AudioClip * clip )
{
        AudioClip* c;
        for (int i=0; i<size(); ++i) {
                c = at(i);
                if (c == clip) {
                        return i;
                }
        }
        return -1;
}

void AudioClipList::sort()
{
        if(!isEmpty())
                qSort(begin(), end(), AudioClip::smaller);
}

AudioClip * AudioClipList::get_last( )
{
        if(!isEmpty())
                return last();
        return (AudioClip*) 0;
}

//eof

