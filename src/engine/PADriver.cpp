/*
    Copyright (C) 2007-2010 Remon Sijrier
 
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

#include "PADriver.h"

#include "AudioDevice.h"
#include "AudioChannel.h"

#include <Utils.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


// TODO Is there an xrun callback for PortAudio? If so, connect to _xrun_callback
// TODO If there is some portaudio shutdown callback, connect to _on_pa_shutdown_callback
//	and make it work!

PADriver::PADriver( AudioDevice * dev , int rate, nframes_t bufferSize)
        : TAudioDriver(dev, rate, bufferSize)
{
	read = MakeDelegate(this, &PADriver::_read);
	write = MakeDelegate(this, &PADriver::_write);
	run_cycle = RunCycleCallback(this, &PADriver::_run_cycle);
}

PADriver::~PADriver( )
{
	PENTER;

}

int PADriver::_read(nframes_t nframes)
{
	float* in = (float*) paInputBuffer;
	
        if (!m_captureChannels.size()) {
		return 0;
	}
	
        float* datachan0 = m_captureChannels.at(0)->get_buffer(nframes);
        float* datachan1 = m_captureChannels.at(1)->get_buffer(nframes);
	
	int j=0;
	
	for (uint i=0; i<nframes*2; i++) {
		datachan0[j] = in[i++];
		datachan1[j] = in[i];
		j++;
	}
	
	return 1;
}

int PADriver::_write(nframes_t nframes)
{
        printf("nframes is %d\n", nframes);

	// TODO use the found maxchannel count instead of assuming 2 !! ( == playbackChannels.size() )
	// Properly iterate over all channel buffers to mixdown the audio
	// in an interleaved way (since our channel buffers represent just that,
	// one channel, no interleaved data there)
	
	// CRITICAL : When messing with this routine, be sure to do it right, at the
	// least, turn the volume of your speakers down, if you want them and your ears
	// to last a little longer :D
	
        if (!m_playbackChannels.size()) {
		return 0;
	}
	
	float* out = (float*) paOutputBuffer;
        float* datachan0 = m_playbackChannels.at(0)->get_buffer(nframes);
        float* datachan1 = m_playbackChannels.at(1)->get_buffer(nframes);
	
	int j=0;
	
	for (uint i=0; i<nframes*2; i++) {
		out[i++] = datachan0[j];
		out[i] = datachan1[j];
		j++;
	}
	
        m_playbackChannels.at(0)->silence_buffer(nframes);
        m_playbackChannels.at(1)->silence_buffer(nframes);
	
	return 1;
}

int PADriver::setup(bool capture, bool playback, const QString& hostapi)
{
	// TODO Only open the capture/playback stream if requested (capture == true, playback == true)
	
	// TODO use hostapi to detect which hostApi to use.
	// hostapi can be any of these:
	// Linux: alsa, jack, oss
	// Mac os x: coreaudio, jack
	// Windows: wmme, directx, asio
	
	// TODO In case of hostapi == "alsa", the callback thread prio needs to be set to realtime.
	// 	there has been some discussion on this on the pa mailinglist, digg it up!
	
        printf("PADriver:: capture, playback, hostapi: %d, %d, %s\n", capture, playback, QS_C(hostapi));
	
	PaError err = Pa_Initialize();
	
	if( err != paNoError ) {
		device->message(tr("PADriver:: PortAudio error: %1").arg(Pa_GetErrorText( err )), AudioDevice::WARNING);
		Pa_Terminate();
		return -1;
	} else {
		printf("PADriver:: Succesfully initialized portaudio\n");
	}
	
	
	PaStreamParameters outputParameters, inputParameters;
	PaDeviceIndex deviceindex = -1;
	
	for (int i=0; i<Pa_GetHostApiCount(); ++i) {
		const PaHostApiInfo* inf = Pa_GetHostApiInfo(i);
		
                device->message(tr("hostapi name is %1, deviceCount is %2").arg(inf->name).arg(inf->deviceCount), AudioDevice::INFO);

		if (hostapi == "alsa" && inf->type == paALSA) {
			printf("PADriver:: Found alsa host api, using device %d\n", i);
			deviceindex = i;
			break;
		}
		
		if (hostapi == "jack" && inf->type == paJACK) {
			printf("PADriver:: Found jack host api, using device %d\n", i);
			deviceindex = i;
			break;
		}

		if (hostapi == "wmme" && inf->type == paMME) {
			printf("PADriver:: Found wmme host api, using device %d\n", i);
			deviceindex = i;
			break;
		}
		
		if (hostapi == "directsound" && inf->type == paDirectSound ) {
			printf("PADriver:: Found directsound host api, using device %d\n", i);
			deviceindex = i;
			break;
		}

                if (hostapi == "asio" && inf->type == paAsio ) {
                        printf("PADriver:: Found asio host api, using device %d\n", i);
                        deviceindex = i;
                        break;
                }
		
                if (hostapi == "coreaudio" && inf->type == paCoreAudio ) {
			printf("PADriver:: Found directsound host api, using device %d\n", i);
			deviceindex = i;
			break;
		}
	}
	

	if (deviceindex == -1) {
		device->message(tr("PADriver:: hostapi %1 was not found by Portaudio!").arg(hostapi), AudioDevice::WARNING);
		return -1;
	}
	
	deviceindex = 0;
//	device->message(tr("PADriver:: using device %1").arg(deviceindex), AudioDevice::INFO);
		
	// Configure output parameters.
	// TODO get the max channel count, and use that instead, of assuming 2
	PaDeviceIndex result = Pa_GetDefaultOutputDevice();
	if( result != paNoDevice) {
		outputParameters.device = result;
		outputParameters.channelCount = 2;
		outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
		outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
		outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	
	result = Pa_GetDefaultInputDevice();
	if( result != paNoDevice) {
		inputParameters.device = result;
		inputParameters.channelCount = 2;
		inputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
		inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
		inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	
	/* Open an audio I/O stream. */
	err = Pa_OpenStream(
			&m_paStream,
			&inputParameters,	// The input parameter
			&outputParameters,	// The outputparameter
			frame_rate,		// Set in the constructor
			frames_per_cycle,	// Set in the constructor
			paNoFlag,		// Don't use any flags
			_process_callback, 	// our callback function
			this );
	
	if( err != paNoError ) {
		device->message(tr("PADriver:: PortAudio error: %1").arg(Pa_GetErrorText( err )), AudioDevice::WARNING);
		Pa_Terminate();
		return -1;
	} else {
		printf("PADriver:: Succesfully opened portaudio stream\n");
	}
	
	AudioChannel* audiochannel;
	int port_flags;
	char buf[32];
	
	// TODO use the found maxchannel count for the playback stream, instead of assuming 2 !!
	for (int chn = 0; chn < 2; chn++) {

		snprintf (buf, sizeof(buf) - 1, "playback_%d", chn+1);

                audiochannel = add_playback_channel(buf);
                audiochannel->set_latency(frames_per_cycle + capture_frame_latency);
	}

	// TODO use the found maxchannel count for the capture stream, instead of assuming 0 !!
	for (int chn = 0; chn < 2; chn++) {

		snprintf (buf, sizeof(buf) - 1, "capture_%d", chn+1);

                audiochannel = add_capture_channel(buf);
                audiochannel->set_latency(frames_per_cycle + capture_frame_latency);
	}
	
	return 1;
}

int PADriver::attach()
{
	return 1;
}

int PADriver::start( )
{
	PENTER;
	
	PaError err = Pa_StartStream( m_paStream );
	
	if( err != paNoError ) {
		device->message((tr("PADriver:: PortAudio error: %1").arg(Pa_GetErrorText( err ))), AudioDevice::WARNING);
		Pa_Terminate();
		return -1;
	} else {
		printf("PADriver:: Succesfully started portaudio stream\n");
	}
	
	return 1;
}

int PADriver::stop( )
{
	PENTER;
	PaError err = Pa_CloseStream( m_paStream );
	
	if( err != paNoError ) {
		device->message((tr("PADriver:: PortAudio error: %1").arg(Pa_GetErrorText( err ))), AudioDevice::WARNING);
		Pa_Terminate();
	} else {
		printf("PADriver:: Succesfully closed portaudio stream\n\n");
	}
	
	return 1;
}

int PADriver::process_callback (nframes_t nframes)
{
	if (device->run_cycle( nframes, 0.0) == -1) {
		return paAbort;
	}
	
	return paContinue;
}

QString PADriver::get_device_name( )
{
	// TODO get it from portaudio ?
	return "AudioDevice";
}

QString PADriver::get_device_longname( )
{
	// TODO get it from portaudio ?
	return "AudioDevice";
}

int PADriver::_xrun_callback( void * arg )
{
	PADriver* driver  = static_cast<PADriver *> (arg);
	driver->device->xrun();
	return 0;
}

void PADriver::_on_pa_shutdown_callback(void * arg)
{
	Q_UNUSED(arg);
}

int PADriver::_process_callback(
	const void *inputBuffer,
	void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *arg )
{
	Q_UNUSED(timeInfo);
	Q_UNUSED(statusFlags);
	
	PADriver* driver  = static_cast<PADriver *> (arg);
	
	driver->paInputBuffer = inputBuffer;
	driver->paOutputBuffer = outputBuffer;
	
	driver->process_callback (framesPerBuffer);
	
	return 0;
}

float PADriver::get_cpu_load( )
{
	return Pa_GetStreamCpuLoad(m_paStream) * 100;
}


//eof
