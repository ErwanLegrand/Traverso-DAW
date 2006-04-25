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

$Id: Traverso.cpp,v 1.2 2006/04/25 17:22:13 r_sijrier Exp $
*/

#include <signal.h>
#include "../config.h"

#include <QTimer>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

#include "ColorManager.h"
#include "Traverso.h"
#include "Interface.h"
#include "PluginLoader.h"
#include "Mixer.h"
#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const char* CONFIG_FILE_VERSION = "1";

Traverso::Traverso(int argc, char **argv )
		: QApplication ( argc, argv )
{
#if QT_VERSION >= 0x040100
	QSettings::setPath ( QSettings::NativeFormat, QSettings::UserScope, QDir::homePath () + "/.traverso" );
#endif

	QCoreApplication::setOrganizationName("Traverso");
	QCoreApplication::setApplicationName("Traverso");


	interface = new Interface();

	init();

	interface->create();

	QMetaObject::invokeMethod(this, "prepare_audio_device", Qt::QueuedConnection);
	QMetaObject::invokeMethod(&pm(), "start", Qt::QueuedConnection);
	
	connect(this, SIGNAL(lastWindowClosed()), &pm(), SLOT(exit()));
}

Traverso::~Traverso()
{
	PENTERDES;
	delete interface;
}


void Traverso::reset_settings( )
{
	QSettings settings;
	settings.clear();

	settings.setValue("ProgramVersion", VERSION);
	settings.setValue("ConfigFileVersion", CONFIG_FILE_VERSION);
	settings.setValue("trackCreationCount", 6);
	settings.setValue("hzoomLevel", 11);

	settings.beginGroup("Project");
	settings.setValue("current", "Untitled");
	settings.setValue("loadLastUsed", 1);
	settings.setValue("directory", "");
	settings.endGroup();

	settings.beginGroup("CCE");
	settings.setValue("clearTime", 2000);
	settings.setValue("holdTimeout", 200);
	settings.setValue("doublefactTimeout", 200);
	settings.endGroup();

	settings.beginGroup("Hardware");
	settings.setValue("samplerate", 44100);
	settings.setValue("bufferSize", 1024);
	settings.setValue("drivertype", "ALSA");
	settings.endGroup();
}

void Traverso::shutdown( int signal )
{
	bool save = false;
	// Just in case the mouse was grabbed...
	cpointer().release_mouse();
	switch(signal) {
	case SIGINT:
		printf("\nCatched the SIGINT signal!\nSaving your work....\n\nShutting down Traverso!\n\n");
		save = true;
		break;
	case SIGSEGV:
		printf("\nCatched the SIGSEGV signal!\n");
		switch (QMessageBox::critical( interface, "Crash",
					"The program made an invalid operation and crashed :-(\n"
					"should I try to save your work?",
					"Yes",
					"No",
					0, 1)) {
		case 0 :
			save = true;
			break;
		case 1 :
			break;
		}
	}
	if (save) {
		Project* project = pm().get_project();
		if (project) {
			if (project->save() > 0)
				printf("Succesfully saved your work!\n");
			else
				printf("Couldn't save your work\n");
		}
	}
	audiodevice().shutdown();
	printf("Stopped\n");
}


int Traverso::init( )
{
	bool generic_mix_functions = true;
	if (true) {

#if defined (SSE_OPTIMIZATIONS)

		unsigned int use_sse = 0;

		asm volatile (
			"mov $1, %%eax\n"
			"pushl %%ebx\n"
			"cpuid\n"
			"popl %%ebx\n"
			"andl $33554432, %%edx\n"
			"movl %%edx, %0\n"
			: "=m" (use_sse)
			:
			: "%eax", "%ecx", "%edx", "memory");

		if (use_sse) {
			PMESG("Enabling SSE optimized routines");

			// SSE SET
			Mixer::compute_peak		= x86_sse_compute_peak;
			Mixer::apply_gain_to_buffer 	= x86_sse_apply_gain_to_buffer;
			Mixer::mix_buffers_with_gain 	= x86_sse_mix_buffers_with_gain;
			Mixer::mix_buffers_no_gain 	= x86_sse_mix_buffers_no_gain;

			generic_mix_functions = false;
		}
#endif

	}

	if (generic_mix_functions) {
		Mixer::compute_peak 		= default_compute_peak;
		Mixer::apply_gain_to_buffer 	= default_apply_gain_to_buffer;
		Mixer::mix_buffers_with_gain 	= default_mix_buffers_with_gain;
		Mixer::mix_buffers_no_gain 	= default_mix_buffers_no_gain;

		PMESG("No Hardware specific optimizations in use");
	}

	// Initialize random number generator
	srand ( time(NULL) );

	QSettings settings;

	// Detect if the confile file versions match, if not, there has been most likely a changeAlsaDriver
	// overwrite with the newest version...

	if (settings.value("ConfigFileVersion").toString() != CONFIG_FILE_VERSION)
		reset_settings();

	QString projects_path = settings.value("Project/directory").toString();

	QDir dir;
	if ( (projects_path.isEmpty()) || (!dir.exists(projects_path)) ) {
		if (projects_path.isEmpty())
			projects_path = QDir::homePath();

		QString newPath = QFileDialog::getExistingDirectory(0,
				tr("Choose an existing or create a new Project Directory"),
				projects_path);
		if (dir.exists(newPath)) {
			QMessageBox::information( interface, tr("Traverso - Information"), tr("Using existing Project directory: %1\n").arg(newPath), "OK", 0 );
		} else if (!dir.mkpath(newPath)) {
			QMessageBox::warning( interface, tr("Traverso - Warning"), tr("Unable to create Project directory! \n") +
					tr("Please check permission for this directory: %1").arg(newPath) );
			return -1;
		} else {
			QMessageBox::information( interface, tr("Traverso - Information"), tr("Created new Project directory for you here: %1\n").arg(newPath), "OK", 0 );
		}
		settings.setValue("Project/directory", newPath);
	}


	ie().init_map(":/keymap");
	ie().set_clear_time(settings.value("CCE/clearTime").toInt());
	ie().set_hold_sensitiveness(settings.value("CCE/holdTimeout").toInt());
	ie().set_double_fact_interval(settings.value("CCE/doublefactTimeout").toInt());

	return 1;
}

void Traverso::prepare_audio_device( )
{
	QSettings settings;

	int rate = settings.value("Hardware/samplerate").toInt();
	int bufferSize = settings.value("Hardware/bufferSize").toInt();
	QString driverType = settings.value("Hardware/drivertype").toString();

	if (bufferSize == 0) {
		qWarning("BufferSize read from Settings is 0 !!!");
		bufferSize = 2048;
	}
	if (rate == 0) {
		qWarning("Samplerate read from Settings is 0 !!!");
		rate = 44100;
	}
	if (driverType.isEmpty()) {
		qWarning("Driver type read from Settings is an empty String !!!");
		driverType = "ALSA";
	}

	audiodevice().set_parameters(rate, bufferSize, driverType);
}

void Traverso::saveState( QSessionManager &  manager)
{
	manager.setRestartHint(QSessionManager::RestartIfRunning);
	QStringList command;
	command << "traverso" << "-session" <<  QApplication::sessionId();
	manager.setRestartCommand(command);
}

void Traverso::commitData( QSessionManager &  )
{
	pm().save_project();
}


// eof

