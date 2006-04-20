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
 
    $Id: AudioClipList.h,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#ifndef AUDIOCLIPLIST_H
#define AUDIOCLIPLIST_H

#include <QList>

class AudioClip;

class AudioClipList : public QList<AudioClip* >
{

public:
        AudioClipList()
        {}
        ~AudioClipList()
        {}

        void add_clip(AudioClip* clip);
        int remove_clip(AudioClip* clip);
        AudioClip* next(AudioClip* clip);
        AudioClip* prev(AudioClip* clip);

        AudioClip* get_last();

private:
        void sort();
        int clip_index(AudioClip* clip);
};

#endif

//eof


