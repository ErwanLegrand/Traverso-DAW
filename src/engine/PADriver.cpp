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
        float* in = (float*) m_paInputBuffer;
	
        if (!m_captureChannels.size()) {
		return 0;
	}

        int index = 0;
        for(nframes_t i=0; i<nframes; i++) {
                for (int chan=0; chan<m_captureChannels.size(); chan++) {
                        AudioChannel* channel = m_captureChannels.at(chan);
                        audio_sample_t* buf = channel->get_buffer(nframes);
                        buf[i] = in[index++];
                }
        }

	return 1;
}

int PADriver::_write(nframes_t nframes)
{
        if (!m_playbackChannels.size()) {
		return 0;
	}
	
        float* out = (float*) m_paOutputBuffer;


        int index = 0;
        for(nframes_t i=0; i<nframes; i++) {
                for (int chan=0; chan<m_playbackChannels.size(); chan++) {
                        out[index++] = m_playbackChannels.at(chan)->get_buffer(nframes)[i];
                }
        }
	
        for (int chan=0; chan<m_playbackChannels.size(); chan++) {
                m_playbackChannels.at(chan)->silence_buffer(nframes);
        }
	
	return 1;
}

QStringList PADriver::devices_info(const QString& hostApi)
{
        QStringList list;
        PaError err = Pa_Initialize();

        if( err != paNoError ) {
                return list;
        }

        PaDeviceIndex hostIndex = host_index_for_host_api(hostApi);

        if (hostIndex == paHostApiNotFound) {
                Pa_Terminate();
                return list;
        }

        const PaHostApiInfo* hostApiInfo = Pa_GetHostApiInfo(hostIndex);

        QString device;

        for (int i=0; i<hostApiInfo->deviceCount; ++i) {
                PaDeviceIndex paDeviceIndex= Pa_HostApiDeviceIndexToDeviceIndex(hostIndex, i);
                device = Pa_GetDeviceInfo(paDeviceIndex)->name;
                if (paDeviceIndex == hostApiInfo->defaultInputDevice) {
                        device += "###defaultInputDevice";
                }
                if (paDeviceIndex == hostApiInfo->defaultOutputDevice) {
                        device += "###defaultOutputDevice";
                }
                list.append(device);
        }

        Pa_Terminate();

        return list;
}

int PADriver::host_index_for_host_api(const QString& hostapi)
{
        int hostIndex = paHostApiNotFound;

        if (hostapi == "alsa") {
                hostIndex = Pa_HostApiTypeIdToHostApiIndex(paALSA);
        }

        if (hostapi == "jack") {
                hostIndex = Pa_HostApiTypeIdToHostApiIndex(paJACK);
        }

        if (hostapi == "wmme") {
                hostIndex = Pa_HostApiTypeIdToHostApiIndex(paMME);
        }

        if (hostapi == "directsound") {
                hostIndex = Pa_HostApiTypeIdToHostApiIndex(paDirectSound);
        }

        if (hostapi == "wasapi") {
                hostIndex = Pa_HostApiTypeIdToHostApiIndex(paWASAPI);
        }

        if (hostapi == "wdmks") {
                hostIndex = Pa_HostApiTypeIdToHostApiIndex(paWDMKS);
        }

        if (hostapi == "asio") {
                hostIndex = Pa_HostApiTypeIdToHostApiIndex(paASIO);
        }

        if (hostapi == "coreaudio") {
                hostIndex = Pa_HostApiTypeIdToHostApiIndex(paCoreAudio);
        }

        if (hostIndex >= 0) {
                printf("PADriver:: Found %s host api\n", QS_C(hostapi));
        }

        return hostIndex;
}

int PADriver::setup(bool capture, bool playback, const QString& deviceInfo)
{
	// TODO In case of hostapi == "alsa", the callback thread prio needs to be set to realtime.
	// 	there has been some discussion on this on the pa mailinglist, digg it up!
	
        QStringList deviceInfos = deviceInfo.split("::");
        QString hostapi, inputDeviceName, outputDeviceName;
        if (deviceInfos.size() > 0) {
                hostapi = deviceInfos.at(0);
        }
        if (deviceInfos.size() > 1 && capture) {
                inputDeviceName = deviceInfos.at(1);
        }
        if (deviceInfos.size() > 2 && playback) {
                outputDeviceName = deviceInfos.at(2);
        }

        printf("PADriver:: Setting up Port Audio driver, using PortAudio library: %s\n", Pa_GetVersionText());
        printf("PADriver:: capture=%d, playback=%d, hostapi=%s, inputdevice=%s, outputdevice=%s\n",
                capture, playback, QS_C(hostapi), QS_C(inputDeviceName), QS_C(outputDeviceName));
	
	PaError err = Pa_Initialize();
	
	if( err != paNoError ) {
		device->message(tr("PADriver:: PortAudio error: %1").arg(Pa_GetErrorText( err )), AudioDevice::WARNING);
		Pa_Terminate();
		return -1;
        }
	
	PaStreamParameters outputParameters, inputParameters;
        memset(&inputParameters, 0, sizeof(inputParameters));
        memset(&outputParameters, 0, sizeof(outputParameters));

        PaHostApiIndex hostIndex = host_index_for_host_api(hostapi);

        if (hostIndex == paHostApiNotFound) {
                device->message(tr("PADriver:: hostapi %1 was not found by Portaudio!").arg(hostapi), AudioDevice::WARNING);
                Pa_Terminate();
                return -1;
	}
	
        const PaHostApiInfo* hostApiInfo = Pa_GetHostApiInfo(hostIndex);

        PaDeviceIndex inputDeviceIndex = hostApiInfo->defaultInputDevice;
        PaDeviceIndex outputDeviceIndex = hostApiInfo->defaultOutputDevice;

        for (int i=0; i<hostApiInfo->deviceCount; ++i) {
                if (Pa_GetDeviceInfo(i)->name == inputDeviceName) {
                        inputDeviceIndex = i;
                }
                if (Pa_GetDeviceInfo(i)->name == outputDeviceName) {
                        outputDeviceIndex = i;
                }
//                printf("device index %d, device name %s\n", i, Pa_GetDeviceInfo(i)->name);
        }

//	device->message(tr("PADriver:: using device %1").arg(deviceindex), AudioDevice::INFO);
		
        int inChannelMax = 0;
        int outChannelsMax = 0;

        if( outputDeviceIndex != paNoDevice) {
                outChannelsMax = Pa_GetDeviceInfo(outputDeviceIndex)->maxOutputChannels;
                outputParameters.device = outputDeviceIndex;
                outputParameters.channelCount = outChannelsMax;
                outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
                outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
                outputParameters.hostApiSpecificStreamInfo = NULL;
        }

        if (inputDeviceIndex != paNoDevice) {
                inChannelMax = Pa_GetDeviceInfo(inputDeviceIndex)->maxInputChannels;
                inputParameters.device = inputDeviceIndex;
                inputParameters.channelCount = inChannelMax;
		inputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
                inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
		inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	

        /* Open an audio I/O stream. */
	err = Pa_OpenStream(
			&m_paStream,
                        capture ? &inputParameters : 0,	// The input parameter
                        playback ? &outputParameters : 0,	// The outputparameter
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
        char buf[32];

        if (playback) {
                for (int chn = 0; chn < outChannelsMax; chn++) {

                        snprintf (buf, sizeof(buf) - 1, "playback_%d", chn+1);

                        audiochannel = add_playback_channel(buf);
                        audiochannel->set_latency(frames_per_cycle + capture_frame_latency);
                }
        }

        if (capture) {
                for (int chn = 0; chn < inChannelMax; chn++) {

                        snprintf (buf, sizeof(buf) - 1, "capture_%d", chn+1);

                        audiochannel = add_capture_channel(buf);
                        audiochannel->set_latency(frames_per_cycle + capture_frame_latency);
                }
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
	
        driver->m_paInputBuffer = (void*)inputBuffer;
        driver->m_paOutputBuffer = outputBuffer;
	
	driver->process_callback (framesPerBuffer);
	
	return 0;
}

float PADriver::get_cpu_load( )
{
	return Pa_GetStreamCpuLoad(m_paStream) * 100;
}


//eof
