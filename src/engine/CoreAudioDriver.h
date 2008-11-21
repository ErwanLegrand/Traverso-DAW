/*
Copyright (C) 2008 Remon Sijrier 
Copyright (C) Grame, 2003.
Copyright (C) Johnny Petrantoni, 2003.

(November 2008) Ported to C++ for Traverso by Remon Sijrier

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

Grame Research Laboratory, 9, rue du Garet 69001 Lyon - France
grame@rd.grame.fr
	
Johnny Petrantoni, johnny@lato-b.com - Italy, Rome.

30-01-04, Johnny Petrantoni: first code of the coreaudio driver.

*/

#ifndef CORE_AUDIO_DRIVER_H
#define CORE_AUDIO_DRIVER_H

#include "Driver.h"

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioConverter.h>
#include <AudioUnit/AudioUnit.h>

#include "defines.h"


class CoreAudioDriver : public Driver
{
public:
	CoreAudioDriver(AudioDevice* dev, int rate, nframes_t bufferSize);
	~CoreAudioDriver();
	
// 	int start();
// 	int stop();
// 	int _read(nframes_t nframes);
// 	int _write(nframes_t nframes);
// 	int _null_cycle(nframes_t nframes);
// 	int _run_cycle();
// 	int attach();
// 	int detach();
// 	int bufsize(nframes_t nframes);
// 	int restart();
	int setup(bool capture=true, bool playback=true, const QString& cardDevice="none");


private:
	AudioUnit au_hal;
	AudioBufferList* input_list;
	AudioDeviceID device_id;
	int state;

};
 
#endif

//eof
