/*
    Copyright (C) 2007 Remon Sijrier 
 
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

#ifndef PORTAUDIO_DRIVER_H
#define PORTAUDIO_DRIVER_H

#include "TAudioDriver.h"
#include "portaudio.h"

class PADriver : public TAudioDriver
{
        Q_OBJECT

public:
	PADriver(AudioDevice* dev, int rate, nframes_t bufferSize);
	~PADriver();

	int  process_callback (nframes_t nframes);
	int _read(nframes_t nframes);
	int _write(nframes_t nframes);
	int _run_cycle() {return 1;}
        int setup(bool capture=true, bool playback=true, const QString& deviceInfo="alsa::default::default");
	int attach();
	int start();
	int stop();

	QString get_device_name();
	QString get_device_longname();
        static QStringList devices_info(const QString& hostApi);
        static int host_index_for_host_api(const QString& hostapi);

	float get_cpu_load();

private:
	PaStream* m_paStream;
        void* m_paInputBuffer;
        void* m_paOutputBuffer;


 	static int _xrun_callback(void *arg);
	static void _on_pa_shutdown_callback(void* arg);
	static int _process_callback( const void *inputBuffer, void *outputBuffer,
					unsigned long framesPerBuffer,
					const PaStreamCallbackTimeInfo* timeInfo,
					PaStreamCallbackFlags statusFlags,
					void *arg );

};


#endif

//eof

