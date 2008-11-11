/*
Copyright (C) 2008 Remon Sijrier 

(November 2008) Ported to C++ for Traverso by Remon Sijrier
Copyright (C) 2001 Paul Davis 

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

*/

/* CoreAudio driver based on jack-audio-connection-kit-0.xxx.x core_audio_driver.c */


#include "CoreAudioDriver.h"
#include "AudioChannel.h"
#include <Utils.h>

#include "Debugger.h"


CoreAudioDriver::CoreAudioDriver(AudioDevice * dev, int rate, nframes_t bufferSize)
	: Driver(dev, rate, bufferSize)
{
	PENTERCONS;
}

CoreAudioDriver::~ CoreAudioDriver()
{
	PENTERDES;
} 

int CoreAudioDriver::setup(bool capture, bool playback, const QString & cardDevice)
{
	return -1;
}

//eof
