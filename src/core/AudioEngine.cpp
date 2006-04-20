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
 
    $Id: AudioEngine.cpp,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#include "libtraverso.h"

#include "AudioEngine.h"
#include "Client.h"
#include "Song.h"
#include <QString>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

AudioEngine::AudioEngine(Song* song)
                : m_song(song)
{
        m_client = audiodevice().new_client("song_" + QByteArray::number(song->get_id()));
        m_client->set_process_callback(MakeDelegate(this, &AudioEngine::process_callback));
}

AudioEngine::~AudioEngine()
{
        audiodevice().unregister_client(m_client);
}

int AudioEngine::process_callback(nframes_t nframes)
{
        return m_song->process( nframes );
}

int AudioEngine::start()
{
        return 1;
}

int AudioEngine::stop()
{
        return 1;
}
//eof

