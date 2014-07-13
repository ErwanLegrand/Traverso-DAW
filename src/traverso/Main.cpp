/*
Copyright (C) 2005-2009 Remon Sijrier 

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

#include <cstdio>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <QLocale>
#include <QTranslator>
#include <QtPlugin>

#include "TConfig.h"
#include "Traverso.h"
#include "Project.h"
#include "ProjectManager.h"
#include "TMainWindow.h"
#include "Main.h"
#include "../config.h"
#include <cstdlib>
#include <unistd.h>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Traverso* traverso;

int signalcount = 0;


#if defined (Q_WS_X11) || defined (Q_WS_MAC)
void catch_signal(int sig_num)
{
	if (!signalcount) {
		++signalcount;
		traverso->shutdown(sig_num);
		exit(0);
	} else {
                printf("Caught multiple signals, aborting !\n");
		kill (-getpgrp(), SIGABRT);
		exit(-1);
	}
}
#endif


int main( int argc, char **argv )
{
	TRACE_OFF();
	MEM_ON();

#if defined (Q_WS_X11) || defined (Q_WS_MAC)
	signal(SIGINT, catch_signal);
	signal(SIGSEGV, catch_signal);
#endif

	TraversoDebugger::set_debug_level(TraversoDebugger::OFF);
	if (argc > 1) {
		for (int i=1; i<argc; i++) {
			if (strcmp(argv[i],"--d1")==0)
					TraversoDebugger::set_debug_level(TraversoDebugger::BASIC);
			if (strcmp(argv[i],"--d2")==0)
					TraversoDebugger::set_debug_level(TraversoDebugger::FLOOD);
			if (strcmp(argv[i],"--d3")==0)
					TraversoDebugger::set_debug_level(TraversoDebugger::SUPER_FLOOD);
			if (strcmp(argv[i],"--d4")==0)
					TraversoDebugger::set_debug_level(TraversoDebugger::ALL);
			if (strcmp(argv[i],"--log")==0)
					TraversoDebugger::create_log("traverso.log");
			if ((strcmp(argv[i],"--help")==0) || (strcmp(argv[i],"-h")==0)) {
				printf("\nUsage: traverso [OPTIONS]\n\n");
				printf("Valid OPTIONS are :\n\n");
				printf("\t--help or -h \t This help text\n");
				printf("\t -v   \t\t Print version number\n");
				printf("\t--d1  \t\t Set debug level to 1 (BASIC)\n");
				printf("\t--d2  \t\t Set debug level to 2 (FLOOD)\n");
				printf("\t--d3  \t\t Set debug level to 3 (SUPER_FLOOD)\n");
				printf("\t--d4  \t\t Set debug level to 4 (ALL)\n");
				printf("\t--log \t\t Create a ~/traverso.log file instead of dumping debug messages to stdout\n");
				printf("\t--show-compile-options\t\t Print options used during compilation\n");
                                printf("\t--fft-meter   \t\t Start Traverso as a Spectral Analyzer\n");
                                printf("\n");
				return 0;
			}
			if (strcmp(argv[i],"--memtrace")==0)
					TRACE_ON();
			if (strcmp(argv[i],"-v")==0) {
				printf("Traverso %s\n", VERSION);
				return 0;
			}
			if (strcmp(argv[i],"--show-compile-options")==0) {
				printf("Traverso compile options: %s\n", TRAVERSO_DEFINES);
				return 0;
			}
		}
	}
	PENTER;


        // Since Qt 4.2 the event loop by default runs on the glib event loop
        // sadly, this introduces a rather high cpu load when using timers that
        // fire quite often (<= 20 ms).
        // T doesn't need the glib event loop so turn it of:
#if defined(Q_OS_UNIX)
        setenv("QT_NO_GLIB", "1", true);
#endif

	// using the raster graphics backend is faster on my system
	// then using native (X11). Not sure if this is the case on
	// systems with a high-end graphics card that properly accelerate
	// all the XRender calls used by QPainter ?
	QApplication::setGraphicsSystem("raster");

	traverso = new Traverso(argc, argv);
	
	QTranslator traversoTranslator;
	QString systemLanguage = QLocale::system().name();
	QString userLanguage = config().get_property("Interface", "LanguageFile", "").toString();
	if (userLanguage.isEmpty() || userLanguage.isNull()) {
		traversoTranslator.load(":/translations/traverso_" + systemLanguage );
	} else {
		traversoTranslator.load(userLanguage);
	}
	traverso->installTranslator(&traversoTranslator);
	
        traverso->create_interface();

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


	traverso->exec();
	
	delete traverso;

	MEM_OFF();

	printf("Thank you for using Traverso !\n");
	return 0;
}

