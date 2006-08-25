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

	$Id: Main.cpp,v 1.4 2006/08/25 11:17:04 r_sijrier Exp $
*/

#include <signal.h>

#include <QLocale>
#include <QTranslator>
#include "Traverso.h"
#include "Main.h"
#include "../config.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Traverso* traverso;

int signalcount = 0;


#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
void catch_signal(int sig_num)
{
	if (!signalcount) {
		++signalcount;
		traverso->shutdown(sig_num);
		exit(0);
	} else {
		printf("Catched multiple signals, aborting !\n");
		kill (-getpgrp(), SIGABRT);
		exit(-1);
	}
}
#endif

int main( int argc, char **argv )
{
	TRACE_OFF();
	MEM_ON();

#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
	signal(SIGINT, catch_signal);
	signal(SIGSEGV, catch_signal);
#endif

	Debugger::set_debug_level(Debugger::OFF);
	if (argc > 1) {
		for (int i=1; i<argc; i++) {
			if (strcmp(argv[i],"--d1")==0)
					Debugger::set_debug_level(Debugger::BASIC);
			if (strcmp(argv[i],"--d2")==0)
					Debugger::set_debug_level(Debugger::FLOOD);
			if (strcmp(argv[i],"--d3")==0)
					Debugger::set_debug_level(Debugger::SUPER_FLOOD);
			if (strcmp(argv[i],"--d4")==0)
					Debugger::set_debug_level(Debugger::ALL);
			if (strcmp(argv[i],"--log")==0)
					Debugger::create_log("traverso.log");
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
// 				printf("\t--scanbuses \t List all hardware in and out buses\n");
				printf("\n");
				return 0;
			}
			if ((strcmp(argv[i],"--scanbuses")==0) || (strcmp(argv[i],"-h")==0)) {
					return 0;
			}
			if (strcmp(argv[i],"--render")==0) {
					printf("Render in batch mode is being implemented.\n");
					// TODO
					return 0;
			}
			if (strcmp(argv[i],"--renderwav")==0) {
					printf("Render in batch mode is being implemented.\n");
					// TODO
					return 0;
			}
			if (strcmp(argv[i],"--memtrace")==0)
					TRACE_ON();
			if (strcmp(argv[i],"-v")==0) {
				printf("Traverso %s\n", VERSION);
				return 0;
			}
		}
	}
	PENTER;



	traverso = new Traverso(argc, argv);

	QTranslator traversoTranslator;
	QString language = QLocale::system().name();
	traversoTranslator.load(":/traverso_" + language );
	traverso->installTranslator(&traversoTranslator);
	traverso->exec();
	delete traverso;

	MEM_OFF();

	printf("Thank you for using Traverso !\n");
	return 0;
}

