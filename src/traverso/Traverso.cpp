/*
Copyright (C) 2005-2007 Remon Sijrier

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

#include <signal.h>
#include "../config.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

#include "Traverso.h"
#include "Mixer.h"
#include "ProjectManager.h"
#include <Project.h>
#include "TMainWindow.h"
#include "Themer.h"
#include <Config.h>
#include <AudioDevice.h>
#include <ContextPointer.h>
#include <Information.h>
#include "defines.h"
#include "fpu.h"
#ifdef __SSE__
#include <xmmintrin.h>
#endif
#if defined (__APPLE__)
#include <Carbon/Carbon.h> // For Gestalt
#endif


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/**
 * 	\mainpage Traverso developers documentation
 *
 *	Traverso makes use of frameworks provided by the 'core' library,
	to implement the Contextual Interface, <br /> thread save insertion / 
	removal of objects in the audio processing chain, and creating
	historable actions.<br />

	<b>The framework that forms the Contextual or "Soft Selection' enabled
	Interface is provided by:</b>

	ViewPort, ContextItem, ContextPointer, Command, InputEngine and the Qt 
	Undo Framework.<br />
	The Qt Undo Framework and the Command class account for the 'History' 
	framework in Traverso.

	The ViewPort, ContextPointer, InputEngine and Command classes together 
	also make it possible to create 'analog' type of actions. <br />
	The actual implementation for analog actions is done by reimplementing
	the virtual Command::jog() function.

	<br />
	<b>The traversoengine library provides a driver abstraction</b><br />
	for the audio hardware, currently ALSA, PortAudio and Jack are supported as drivers.

	<br />
	The Tsar class (singleton) is the key behind lockless, thus non blocking removing
	and adding of audio processing objects in the audio processing chain.

	<br />
	The AddRemove Command class is to be used for adding/removing items
	to/from ContextItem objects. <br />This class detects if the add/remove function
	can be called directly, or in a thread save way, and uses therefore the
	Tsar class, and the Sheet object in case it was given as a parameter.<br />
	See for more information the AddRemove, AudioDevice and Tsar class
	documentation.
 */




Traverso::Traverso(int &argc, char **argv )
	: QApplication ( argc, argv )
{
	QCoreApplication::setOrganizationName("Traverso-DAW");
	QCoreApplication::setApplicationName("Traverso");
	QCoreApplication::setOrganizationDomain("traverso-daw.org");
		
	qRegisterMetaType<InfoStruct>("InfoStruct");
	qRegisterMetaType<TimeRef>("TimeRef");
	
	config().check_and_load_configuration();
	
	// Initialize random number generator
	srand ( time(NULL) );
	
	init_sse();
	
        create_interface();

        if (argc > 1) {
                for (int i=1; i<argc; i++) {
                        if (strcmp(argv[i],"--fft-meter")==0) {
                                printf("Using Traverso in FFT Meter only mode\n");
                                if (!pm().project_exists("fft-meter")) {
                                        Project* project = pm().create_new_project(1, 1, "fft-meter");
                                        project->save();
                                        delete project;

                                }
                                pm().load_project("fft-meter");
                                TMainWindow::instance()->show_fft_meter_only();
                        }
                }
        }
	
	connect(this, SIGNAL(lastWindowClosed()), &pm(), SLOT(exit()));
}

Traverso::~Traverso()
{
	PENTERDES;
	delete TMainWindow::instance();
	delete themer();
        config().save();
}

void Traverso::create_interface( )
{
	themer()->load();
        TMainWindow* tMainWindow = TMainWindow::instance();
        tMainWindow->show();

        config().init_input_engine();
	
	QString projectToLoad = "";
	
	foreach(QString string, QCoreApplication::arguments ()) {
		if (string.contains("project.tpf")) {
			projectToLoad = string;
			break;
		}
	}
	
	// The user clicked on a project.tpf file, start extracting the 
	// baseproject directory, and the project name from the filename.
	if (!projectToLoad.isEmpty()) {
		QFileInfo fi(projectToLoad);
		QDir projectdir(fi.path());
		QDir baseprojectdir(fi.path());
		baseprojectdir.cdUp();
		QString baseprojectdirpath = baseprojectdir.path();
		QString projectdirpath = projectdir.path();
		QString projectname = projectdirpath.mid(baseprojectdirpath.length() + 1, projectdirpath.length());
		
		if (!projectname.isEmpty() && ! baseprojectdirpath.isEmpty()) {
			pm().start(baseprojectdirpath, projectname);
			return;
		}
	}
}

void Traverso::shutdown( int signal )
{
	PENTER;
	
	// Just in case the mouse was grabbed...
	cpointer().jog_finished();
	QApplication::sendPostedEvents();
	
	switch(signal) {
		case SIGINT:
                        printf("\nCaught the SIGINT signal!\nShutting down Traverso!\n\n");
			pm().exit();
			return;
			break;
		case SIGSEGV:
                        printf("\nCaught the SIGSEGV signal!\n");
			QMessageBox::critical( TMainWindow::instance(), "Crash",
				"The program made an invalid operation and crashed :-(\n"
				"Please, report this to us!");
	}

	printf("Stopped\n");

	exit(0);
}


void Traverso::init_sse( )
{
	bool generic_mix_functions = true;
	
	FPU fpu;

#if (defined (ARCH_X86) || defined (ARCH_X86_64)) && defined (SSE_OPTIMIZATIONS)

	if (fpu.has_sse()) {

		printf("Using SSE optimized routines\n");

		// SSE SET
		Mixer::compute_peak		= x86_sse_compute_peak;
		Mixer::apply_gain_to_buffer 	= x86_sse_apply_gain_to_buffer;
		Mixer::mix_buffers_with_gain 	= x86_sse_mix_buffers_with_gain;
		Mixer::mix_buffers_no_gain 	= x86_sse_mix_buffers_no_gain;

		generic_mix_functions = false;

	}

#elif defined (__APPLE__) && defined (BUILD_VECLIB_OPTIMIZATIONS)
	long sysVersion = 0;

	if (noErr != Gestalt(gestaltSystemVersion, &sysVersion))
		sysVersion = 0;

	if (sysVersion >= 0x00001040) { // Tiger at least
		Mixer::compute_peak           = veclib_compute_peak;
		Mixer::apply_gain_to_buffer   = veclib_apply_gain_to_buffer;
		Mixer::mix_buffers_with_gain  = veclib_mix_buffers_with_gain;
		Mixer::mix_buffers_no_gain    = veclib_mix_buffers_no_gain;

		generic_mix_functions = false;

		printf("Apple VecLib H/W specific optimizations in use\n");
	}
#endif

	/* consider FPU denormal handling to be "h/w optimization" */

	setup_fpu ();

	
	if (generic_mix_functions) {
		Mixer::compute_peak 		= default_compute_peak;
		Mixer::apply_gain_to_buffer 	= default_apply_gain_to_buffer;
		Mixer::mix_buffers_with_gain 	= default_mix_buffers_with_gain;
		Mixer::mix_buffers_no_gain 	= default_mix_buffers_no_gain;

		printf("No Hardware specific optimizations in use\n");
	}

}


void Traverso::setup_fpu()
{

	// export TRAVERSO_RUNNING_UNDER_VALGRIND to disable assembler stuff below!
	if (getenv("TRAVERSO_RUNNING_UNDER_VALGRIND")) {
		printf("TRAVERSO_RUNNING_UNDER_VALGRIND=TRUE\n");
		// valgrind doesn't understand this assembler stuff
		// September 10th, 2007
		return;
	}

#if (defined(ARCH_X86) || defined(ARCH_X86_64)) && defined(USE_XMMINTRIN)
        
	int MXCSR;
	FPU fpu;

	/* XXX use real code to determine if the processor supports
	DenormalsAreZero and FlushToZero
	*/
	
	if (!fpu.has_flush_to_zero() && !fpu.has_denormals_are_zero()) {
		return;
	}

	MXCSR  = _mm_getcsr();

/*	switch (Config->get_denormal_model()) {
		case DenormalNone:
			MXCSR &= ~(_MM_FLUSH_ZERO_ON|0x8000);
			break;

		case DenormalFTZ:
			if (fpu.has_flush_to_zero()) {
				MXCSR |= _MM_FLUSH_ZERO_ON;
			}
			break;

		case DenormalDAZ:*/
			MXCSR &= ~_MM_FLUSH_ZERO_ON;
			if (fpu.has_denormals_are_zero()) {
				MXCSR |= 0x8000;
			}
// 			break;
// 		
// 		case DenormalFTZDAZ:
// 			if (fpu.has_flush_to_zero()) {
// 				if (fpu.has_denormals_are_zero()) {
// 					MXCSR |= _MM_FLUSH_ZERO_ON | 0x8000;
// 				} else {
// 					MXCSR |= _MM_FLUSH_ZERO_ON;
// 				}
// 			}
// 			break;
// 	}

	_mm_setcsr (MXCSR);

#endif
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
