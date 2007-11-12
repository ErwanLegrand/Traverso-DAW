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
 
    $Id: AudioClipList.cpp,v 1.2 2007/11/12 18:52:13 r_sijrier Exp $
*/

#include "AudioClipList.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

AudioClip * AudioClipList::get_last( )
{
	if (!size()) {
		return 0;
	}
	
	if (size() == 1) {
		return (AudioClip*)begin();
	}
	
	AudioProcessingItem* item = begin();
	AudioProcessingItem* result = item;
	
	while (item) {
		item = item->next;
		if (item) {
			result = item;
		}
	}
		
	return (AudioClip*)result;
}

// Untested functions, which aren't in use anyways...
// AudioClip * AudioClipList::next( AudioClip * clip )
// {
// 	AudioProcessingItem* item = begin();
// 	while(item) {
// 		AudioClip* c = (AudioClip*)item;
// 		if (clip == c) {
// 			return (AudioClip*)c->next;
// 		}
// 		item = item->next;
// 	}
// 	
//         return (AudioClip*) 0;
// }
// 
// AudioClip * AudioClipList::prev( AudioClip * clip )
// {
// 	if (size() <= 1) {
// 		return 0;
// 	}
// 	
// 	AudioProcessingItem* item = begin();
// 	AudioProcessingItem* prev = item;
// 	item = item->next;
// 	
// 	while (item) {
// 		AudioClip* c = (AudioClip*)item;
// 		if (clip == c) {
// 			return (AudioClip*)prev;
// 		}
// 		prev = item;
// 		item = item->next;
// 	}
// 	
// 	return (AudioClip*) 0;
// }


//eof

