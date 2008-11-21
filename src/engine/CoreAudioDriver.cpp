/*
Copyright (C) 2008 Remon Sijrier 
Copyright (C) Grame, 2003.
Copyright (C) Johnny Petrantoni, 2003.

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
 
 Jan 30, 2004: Johnny Petrantoni: first code of the coreaudio driver, based on portaudio driver by Stephane Letz.
 Feb 02, 2004: Johnny Petrantoni: fixed null cycle, removed double copy of buffers in AudioRender, the driver works fine (tested with Built-in Audio and Hammerfall RME), but no cpu load is displayed.
 Feb 03, 2004: Johnny Petrantoni: some little fix.
 Feb 03, 2004: Stephane Letz: some fix in AudioRender.cpp code.
 Feb 03, 2004: Johnny Petrantoni: removed the default device stuff (useless, in jackosx, because JackPilot manages this behavior), the device must be specified. and all parameter must be correct.
 Feb 04, 2004: Johnny Petrantoni: now the driver supports interfaces with multiple interleaved streams (such as the MOTU 828).
 Nov 05, 2004: S.Letz: correct management of -I option for use with JackPilot.
 Nov 15, 2004: S.Letz: Set a default value for deviceID.
 Nov 30, 2004: S.Letz: In coreaudio_driver_write : clear to avoid playing dirty buffers when the client does not produce output anymore.
 Dec 05, 2004: S.Letz: XRun detection 
 Dec 09, 2004: S.Letz: Dynamic buffer size change
 Dec 23, 2004: S.Letz: Correct bug in dynamic buffer size change : update period_usecs
 Jan 20, 2005: S.Letz: Almost complete rewrite using AUHAL.
 May 20, 2005: S.Letz: Add "systemic" latencies management.
 Jun 06, 2005: S.Letz: Remove the "-I" parameter, change the semantic of "-n" parameter : -n (driver name) now correctly uses the PropertyDeviceUID
					   (persistent accross reboot...) as the identifier for the used coreaudio driver.
 Jun 14, 2005: S.Letz: Since the "-I" parameter is not used anymore, rename the "systemic" latencies management parametes "-I" and "-O" like for the ALSA driver.
 Aug 16, 2005: S.Letz: Remove get_device_id_from_num, use get_default_device instead. If the -n option is not used or the device name cannot
					   be found, the default device is used. Note: the default device can be used only if both default input and default output are the same.
 Dec 19, 2005: S.Letz: Add -d option (display_device_names).
 Apri 7, 2006: S.Letz: Synchronization with the jackdmp coreaudio driver version: improve half-duplex management.
 May 17, 2006: S.Letz: Minor fix in driver_initialize.
 May 18, 2006: S.Letz: Document sample rate default value.
 May 31, 2006: S.Letz: Apply Rui patch for more consistent driver parameter naming.
 Dec 04, 2007: S.Letz: Fix a bug in sample rate management (occuring in particular with "aggregate" devices).
 Dec 05, 2007: S.Letz: Correct sample_rate management in Open. Better handling in sample_rate change listener.
 Nov 11, 2008: Remon Sijrier: Ported to C++ for Traverso

*/

/* CoreAudio driver based on jack-audio-connection-kit-0.xxx.x coreaudio_driver.c */


#include "CoreAudioDriver.h"
#include "AudioChannel.h"
#include <Utils.h>

#include <stdio.h>
#include <string.h>

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
