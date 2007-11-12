/*
    Copyright (C) 2005-2007 Remon Sijrier 
 
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
 
    $Id: AudioClipList.h,v 1.2 2007/11/12 18:52:13 r_sijrier Exp $
*/

#ifndef AUDIOCLIPLIST_H
#define AUDIOCLIPLIST_H

#include "APILinkedList.h"
#include "AudioClip.h"

class AudioClipList : public APILinkedList
{

public:
	AudioClipList() : APILinkedList() {}
        ~AudioClipList() {}

        void add_clip(AudioClip* clip);
        int remove_clip(AudioClip* clip);
//         AudioClip* next(AudioClip* clip);
//         AudioClip* prev(AudioClip* clip);

        AudioClip* get_last();
};


inline void AudioClipList::add_clip( AudioClip * clip )
{
	AudioProcessingItem* item = begin();
	AudioProcessingItem* after = 0;
	while(item) {
		AudioClip* c = (AudioClip*)item;
		if(c->get_track_start_location() < clip->get_track_start_location()) {
			after = c;
		}
		item = item->next;
	}
	
	if(!after) {
// 		printf("prepending clip\n");
		prepend(clip);
	} else {
// 		AudioClip* c = ((AudioClip*)after);
// 		printf("appending clip\n after: %d, clip %d\n", c->get_track_start_location().to_frame(44100), clip->get_track_start_location().to_frame(44100));
		add_after(after, clip);
	}
}


inline int AudioClipList::remove_clip( AudioClip * clip )
{
	return remove(clip);
}


#endif

//eof


