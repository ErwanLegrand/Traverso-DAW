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
#include "AudioDevice.h"

#include <Utils.h>

#include <stdio.h>
#include <string.h>

#include "Debugger.h"

//#define PRINTDEBUG 1


CoreAudioDriver::CoreAudioDriver(AudioDevice * dev, int rate, nframes_t bufferSize)
        : TAudioDriver(dev, rate, bufferSize)
{
	PENTERCONS;

        display_device_names();
}

CoreAudioDriver::~ CoreAudioDriver()
{
	PENTERDES;
}



void JCALog(char *fmt, ...)
{
#ifdef PRINTDEBUG
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "JCA: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
#endif
}

void printError(OSStatus err)
{
#ifdef DEBUG
    switch (err) {
        case kAudioHardwareNoError:
            JCALog("error code : kAudioHardwareNoError\n");
            break;
        case kAudioHardwareNotRunningError:
            JCALog("error code : kAudioHardwareNotRunningError\n");
            break;
        case kAudioHardwareUnspecifiedError:
            JCALog("error code : kAudioHardwareUnspecifiedError\n");
            break;
        case kAudioHardwareUnknownPropertyError:
            JCALog("error code : kAudioHardwareUnknownPropertyError\n");
            break;
        case kAudioHardwareBadPropertySizeError:
            JCALog("error code : kAudioHardwareBadPropertySizeError\n");
            break;
        case kAudioHardwareIllegalOperationError:
            JCALog("error code : kAudioHardwareIllegalOperationError\n");
            break;
        case kAudioHardwareBadDeviceError:
            JCALog("error code : kAudioHardwareBadDeviceError\n");
            break;
        case kAudioHardwareBadStreamError:
            JCALog("error code : kAudioHardwareBadStreamError\n");
            break;
        case kAudioDeviceUnsupportedFormatError:
            JCALog("error code : kAudioDeviceUnsupportedFormatError\n");
            break;
        case kAudioDevicePermissionsError:
            JCALog("error code : kAudioDevicePermissionsError\n");
            break;
        default:
            JCALog("error code : unknown %ld\n", err);
            break;
    }
#endif
}

OSStatus get_device_name_from_id(AudioDeviceID id, char name[256])
{
	UInt32 size = sizeof(char) * 256;
	OSStatus res = AudioDeviceGetProperty(id, 0, false,
						kAudioDevicePropertyDeviceName,
						&size,
						&name[0]);
	return res;
}

OSStatus get_device_id_from_uid(char* UID, AudioDeviceID* id)
{
	UInt32 size = sizeof(AudioValueTranslation);
	CFStringRef inIUD = CFStringCreateWithCString(NULL, UID, CFStringGetSystemEncoding());
	AudioValueTranslation value = { &inIUD, sizeof(CFStringRef), id, sizeof(AudioDeviceID) };
	if (inIUD == NULL) {
		return kAudioHardwareUnspecifiedError;
	} else {
		OSStatus res = AudioHardwareGetProperty(kAudioHardwarePropertyDeviceForUID, &size, &value);
		CFRelease(inIUD);
		JCALog("get_device_id_from_uid %s %ld \n", UID, *id);
		return (*id == kAudioDeviceUnknown) ? kAudioHardwareBadDeviceError : res;
	}
}

OSStatus get_default_device(AudioDeviceID * id)
{
	OSStatus res;
	UInt32 theSize = sizeof(UInt32);
	AudioDeviceID inDefault;
	AudioDeviceID outDefault;
  
	if ((res = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, 
					&theSize, &inDefault)) != noErr)
		return res;
	
	if ((res = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, 
					&theSize, &outDefault)) != noErr)
		return res;
		
	JCALog("get_default_device: input %ld output %ld\n", inDefault, outDefault);
	
	// Get the device only if default input and ouput are the same
	if (inDefault == outDefault) {
		*id = inDefault;
		return noErr;
	} else {
		PERROR("CoreAudioDriver:: Default input and output devices are not the same !!");
		return kAudioHardwareBadDeviceError;
	}
}

OSStatus get_default_input_device(AudioDeviceID* id)
{
	OSStatus res;
	UInt32 theSize = sizeof(UInt32);
	AudioDeviceID inDefault;
	
	if ((res = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
						&theSize, &inDefault)) != noErr)
		return res;
	
	JCALog("get_default_input_device: input = %ld \n", inDefault);
	*id = inDefault;
	return noErr;
}

OSStatus get_default_output_device(AudioDeviceID* id)
{
	OSStatus res;
	UInt32 theSize = sizeof(UInt32);
	AudioDeviceID outDefault;
	
	if ((res = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
						&theSize, &outDefault)) != noErr)
		return res;
	
	JCALog("get_default_output_device: output = %ld\n", outDefault);
	*id = outDefault;
	return noErr;
}

OSStatus get_total_channels(AudioDeviceID device, int* channelCount, bool isInput) 
{
	OSStatus			err = noErr;
	UInt32				outSize;
	Boolean				outWritable;
	AudioBufferList*	bufferList = 0;
	AudioStreamID*		streamList = 0;
	int					i, numStream;
	
	err = AudioDeviceGetPropertyInfo(device, 0, isInput, kAudioDevicePropertyStreams, &outSize, &outWritable);
	if (err == noErr) {
		streamList = (AudioStreamID*)malloc(outSize);
		numStream = outSize/sizeof(AudioStreamID);
		JCALog("get_total_channels device stream number = %ld numStream = %ld\n", device, numStream);
		err = AudioDeviceGetProperty(device, 0, isInput, kAudioDevicePropertyStreams, &outSize, streamList);
		if (err == noErr) {
			AudioStreamBasicDescription streamDesc;
			outSize = sizeof(AudioStreamBasicDescription);
			for (i = 0; i < numStream; i++) {
				err = AudioStreamGetProperty(streamList[i], 0, kAudioDevicePropertyStreamFormat, &outSize, &streamDesc);
				JCALog("get_total_channels streamDesc mFormatFlags = %ld mChannelsPerFrame = %ld\n", streamDesc.mFormatFlags, streamDesc.mChannelsPerFrame);
			}
		}
	}
	
	*channelCount = 0;
	err = AudioDeviceGetPropertyInfo(device, 0, isInput, kAudioDevicePropertyStreamConfiguration, &outSize, &outWritable);
	if (err == noErr) {
		bufferList = (AudioBufferList*)malloc(outSize);
		err = AudioDeviceGetProperty(device, 0, isInput, kAudioDevicePropertyStreamConfiguration, &outSize, bufferList);
		if (err == noErr) {								
		for (i = 0; i < bufferList->mNumberBuffers; i++) 
			*channelCount += bufferList->mBuffers[i].mNumberChannels;
		}
	}
		
	if (streamList) 
		free(streamList);
	if (bufferList) 
		free(bufferList);	
			
	return err;
}

OSStatus display_device_names()
{
	UInt32 size;
	Boolean isWritable;
	int i, deviceNum;
	OSStatus err;
	CFStringRef UIname;
	
	err = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, &isWritable);
	if (err != noErr) 
			return err;
		
	deviceNum = size/sizeof(AudioDeviceID);
	AudioDeviceID devices[deviceNum];
	
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size, devices);
	if (err != noErr) 
			return err;
	
	for (i = 0; i < deviceNum; i++) {
        char device_name[256];
		char internal_name[256];
		
		size = sizeof(CFStringRef);
		UIname = NULL;
		err = AudioDeviceGetProperty(devices[i], 0, false, kAudioDevicePropertyDeviceUID, &size, &UIname);
		if (err == noErr) {
			CFStringGetCString(UIname, internal_name, 256, CFStringGetSystemEncoding());
		} else {
			goto error;
		}
		
		size = 256;
		err = AudioDeviceGetProperty(devices[i], 0, false, kAudioDevicePropertyDeviceName, &size, device_name);
		if (err != noErr) 
			return err; 
		printf("ICI\n");
		printf("Device name = \'%s\', internal_name = \'%s\' (to be used as -d parameter)\n", device_name, internal_name); 
	}
	
	return noErr;

error:
	if (UIname != NULL)
		CFRelease(UIname);
	return err;
}

OSStatus CoreAudioDriver::render(AudioUnitRenderActionFlags 	*ioActionFlags,
		const AudioTimeStamp 		*inTimeStamp,
		UInt32 				inBusNumber,
		UInt32 				inNumberFrames,
		AudioBufferList 		*ioData)
{
	int res, i;
	
        AudioUnitRender(m_au_hal, ioActionFlags, inTimeStamp, 1, inNumberFrames, m_input_list);
	
        if (m_xrun_detected > 0) { /* XRun was detected */
		trav_time_t current_time = get_microseconds ();
		device->delay(current_time - (last_wait_ust + period_usecs));
		last_wait_ust = current_time;
                m_xrun_detected = 0;
		return 0;
	} else {
		last_wait_ust = get_microseconds();
		device->transport_cycle_start(get_microseconds());
		res = device->run_cycle(inNumberFrames, 0);
	}
	
        if (m_null_cycle_occured) {
                m_null_cycle_occured = 0;
                for (i = 0; i < m_playback_nchannels; i++) {
			memset((float*)ioData->mBuffers[i].mData, 0, sizeof(float) * inNumberFrames);
		}
	} else {
                // TODO find out if ioData->mBuffers[i] correspond with the m_playbackChannels indices
		audio_sample_t* buf;
                for (int i=0; i<m_playbackChannels.size(); ++i) {
                        AudioChannel* channel = m_playbackChannels.at(i);
			if (!channel->has_data()) {
				continue;
			}
			buf = channel->get_data();
			
			memcpy((float*)ioData->mBuffers[i].mData, buf, sizeof(float) * inNumberFrames);

			// Not sure if this is needed? I think so though
			channel->silence_buffer(inNumberFrames);
		}
	}
	
	return res;
}

OSStatus CoreAudioDriver::render_input(  AudioUnitRenderActionFlags 	*ioActionFlags,
			const AudioTimeStamp 		*inTimeStamp,
			UInt32 				inBusNumber,
			UInt32 				inNumberFrames,
			AudioBufferList 		*ioData)
{
        AudioUnitRender(m_au_hal, ioActionFlags, inTimeStamp, 1, inNumberFrames, m_input_list);
        if (m_xrun_detected > 0) { /* XRun was detected */
		trav_time_t current_time = get_microseconds();
		device->delay(current_time - (last_wait_ust + period_usecs));
		last_wait_ust = current_time;
                m_xrun_detected = 0;
		return 0;
    } else {
		last_wait_ust = get_microseconds();
		device->transport_cycle_start(get_microseconds());
		return device->run_cycle(inNumberFrames, 0);
	}
}


OSStatus CoreAudioDriver::sr_notification(AudioDeviceID inDevice,
        UInt32 inChannel,
        Boolean	isInput,
        AudioDevicePropertyID inPropertyID)
{
	
	switch (inPropertyID) {

		case kAudioDevicePropertyNominalSampleRate: {
			JCALog("JackCoreAudioDriver::SRNotificationCallback kAudioDevicePropertyNominalSampleRate \n");
			state = 1;
			break;
		}
	}
	
	return noErr;
}

OSStatus CoreAudioDriver::notification(AudioDeviceID inDevice,
	UInt32 inChannel,
	Boolean	isInput,
	AudioDevicePropertyID inPropertyID)
{
	switch (inPropertyID) {
		
		case kAudioDeviceProcessorOverload:
                        m_xrun_detected = 1;
			break;
			
		case kAudioDevicePropertyNominalSampleRate: {
			UInt32 outSize =  sizeof(Float64);
			Float64 sampleRate;
			AudioStreamBasicDescription srcFormat, dstFormat;
			
			// FIXME kAudioDeviceSectionGlobal not declared  compile error ?
			OSStatus err;
/*			OSStatus err = AudioDeviceGetProperty(device_id, 0, kAudioDeviceSectionGlobal, kAudioDevicePropertyNominalSampleRate, &outSize, &sampleRate);
			if (err != noErr) {
				PERROR("Cannot get current sample rate");
				return kAudioHardwareUnsupportedOperationError;
			}*/
			JCALog("JackCoreAudioDriver::NotificationCallback kAudioDevicePropertyNominalSampleRate %ld\n", (long)sampleRate);
			outSize = sizeof(AudioStreamBasicDescription);
			
			// Update SR for input
                        err = AudioUnitGetProperty(m_au_hal, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &srcFormat, &outSize);
			if (err != noErr) {
				PERROR("Error calling AudioUnitSetProperty - kAudioUnitProperty_StreamFormat kAudioUnitScope_Input");
			}
			srcFormat.mSampleRate = sampleRate;
                        err = AudioUnitSetProperty(m_au_hal, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &srcFormat, outSize);
			if (err != noErr) {
				PERROR("Error calling AudioUnitSetProperty - kAudioUnitProperty_StreamFormat kAudioUnitScope_Input");
			}
		
			// Update SR for output
                        err = AudioUnitGetProperty(m_au_hal, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &dstFormat, &outSize);
			if (err != noErr) {
				PERROR("Error calling AudioUnitSetProperty - kAudioUnitProperty_StreamFormat kAudioUnitScope_Output");
			}
			dstFormat.mSampleRate = sampleRate;
                        err = AudioUnitSetProperty(m_au_hal, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &dstFormat, outSize);
			if (err != noErr) {
				PERROR("Error calling AudioUnitSetProperty - kAudioUnitProperty_StreamFormat kAudioUnitScope_Output");
			}
			break;
		}
	}
	return noErr;
}



int CoreAudioDriver::attach()
{
	int port_flags;
	channel_t chn;
	AudioChannel* chan;
	char buf[32];
	char channel_name[64];
	OSStatus err;
	UInt32 size;
	UInt32 value1,value2;
	Boolean isWritable;
		
	device->set_buffer_size (frames_per_cycle);
	device->set_sample_rate (frame_rate);

	port_flags = PortIsOutput|PortIsPhysical|PortIsTerminal;
	
        for (chn = 0; chn < m_capture_nchannels; chn++) {
                err = AudioDeviceGetPropertyInfo(m_device_id, chn + 1, true, kAudioDevicePropertyChannelName, &size, &isWritable);
		if (err == noErr && size > 0)  {
                        err = AudioDeviceGetProperty(m_device_id, chn + 1, true, kAudioDevicePropertyChannelName, &size, channel_name);
			if (err != noErr) 
				JCALog("AudioDeviceGetProperty kAudioDevicePropertyChannelName error \n");
                        snprintf(buf, sizeof(buf) - 1, "%s:out_%s%lu", m_capture_driver_name, channel_name, chn + 1);
		} else {
                        snprintf(buf, sizeof(buf) - 1, "%s:out%lu", m_capture_driver_name, chn + 1);
		}
	
                chan = add_capture_channel(buf);
		chan->set_latency( frames_per_cycle + capture_frame_latency );
		
		size = sizeof(UInt32);
		value1 = value2 = 0;
                err = AudioDeviceGetProperty(m_device_id, 0, true, kAudioDevicePropertyLatency, &size, &value1);
		if (err != noErr) 
			JCALog("AudioDeviceGetProperty kAudioDevicePropertyLatency error \n");
                err = AudioDeviceGetProperty(m_device_id, 0, true, kAudioDevicePropertySafetyOffset, &size, &value2);
		if (err != noErr) 
			JCALog("AudioDeviceGetProperty kAudioDevicePropertySafetyOffset error \n");
		
	}
	
	port_flags = PortIsInput|PortIsPhysical|PortIsTerminal;

        for (chn = 0; chn < m_playback_nchannels; chn++) {
                err = AudioDeviceGetPropertyInfo(m_device_id, chn + 1, false, kAudioDevicePropertyChannelName, &size, &isWritable);
		if (err == noErr && size > 0)  {
                        err = AudioDeviceGetProperty(m_device_id, chn + 1, false, kAudioDevicePropertyChannelName, &size, channel_name);
			if (err != noErr) 
				JCALog("AudioDeviceGetProperty kAudioDevicePropertyChannelName error \n");
                        snprintf(buf, sizeof(buf) - 1, "%s:in_%s%lu", m_playback_driver_name, channel_name, chn + 1);
		} else {
                        snprintf(buf, sizeof(buf) - 1, "%s:in%lu", m_playback_driver_name, chn + 1);
		}

                chan = add_playback_channel(buf);
                chan->set_latency( frames_per_cycle + capture_frame_latency );

		size = sizeof(UInt32);
		value1 = value2 = 0;
                err = AudioDeviceGetProperty(m_device_id, 0, false, kAudioDevicePropertyLatency, &size, &value1);
		if (err != noErr) 
			JCALog("AudioDeviceGetProperty kAudioDevicePropertyLatency error \n");
                err = AudioDeviceGetProperty(m_device_id, 0, false, kAudioDevicePropertySafetyOffset, &size, &value2);
		if (err != noErr) 
			JCALog("AudioDeviceGetProperty kAudioDevicePropertySafetyOffset error \n");
	}
	

	// Input buffers do no change : prepare them only once
        for (chn = 0; chn < m_capture_nchannels; chn++) {
                m_input_list->mBuffers[chn].mData = (audio_sample_t*)(m_captureChannels.at(chn)->get_buffer(frames_per_cycle));
	}
	
	return 1;
}


int CoreAudioDriver::setup(bool capture, bool playback, const QString & cardDevice)
{
	return -1;
}



static OSStatus _render(void * inRefCon, AudioUnitRenderActionFlags * ioActionFlags, const AudioTimeStamp * inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList * ioData)
{
	CoreAudioDriver* driver = (CoreAudioDriver*)inRefCon;
	return driver->render(ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
}

static OSStatus _render_input(void * inRefCon, AudioUnitRenderActionFlags * ioActionFlags, const AudioTimeStamp * inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList * ioData)
{
	CoreAudioDriver* driver = (CoreAudioDriver*)inRefCon;
	return driver->render_input(ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
}

static OSStatus _sr_notification(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput, AudioDevicePropertyID inPropertyID, void * inClientData)
{
	CoreAudioDriver* driver = (CoreAudioDriver*)inClientData;
	return driver->sr_notification(inDevice, inChannel, isInput, inPropertyID);
}

static OSStatus _notification(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput, AudioDevicePropertyID inPropertyID, void * inClientData)
{
	CoreAudioDriver* driver = (CoreAudioDriver*)inClientData;
	return driver->notification(inDevice, inChannel, isInput, inPropertyID);
}

