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
 
    $Id: Debugger.cpp,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#include <stdlib.h>
#include <stdio.h>

#include "Debugger.h"

namespace Debugger
{
int ntabs = 0;
int debugLevel = OFF;
FILE* logFile = (FILE*) 0;
QString logFileName = 0;
bool logging = false;
}

void Debugger::fill_tabs()
{
        for (int i=0; i < ntabs; i++)
                printf("|   ");
}

QString Debugger::get_tabs()
{
        QString t="";
        for (int i=0; i < ntabs; i++)
                t=t.append("   ");
        return t;
}


void Debugger::more_tabs()
{
        ntabs++;
}


void Debugger::less_tabs()
{
        ntabs--;
}


void Debugger::set_debug_level(int l)
{
        debugLevel = l;
}


int Debugger::get_debug_level()
{
        return debugLevel;
}


void Debugger::create_log(QString fn)
{
        logFileName = QString(getenv("HOME")) + "/" + fn;
        logFile = fopen(logFileName.toAscii(),"a+");
        if (!logFile) {
                PERROR("Cannot create Debugger Log file (%s)",fn.toAscii().data());
                logging=false;
        } else {
                fclose(logFile);
                logging=true;
        }
}



void Debugger::close_log()
{
        logging=false;
}



void Debugger::log(QString s)
{
        const char* sc = s.toAscii();
        int len = s.length();
        logFile = fopen(logFileName.toAscii(),"a+");
        fwrite(sc,len,1,logFile);
        fclose(logFile);
}

bool Debugger::is_logging()
{
        return logging;
}


#ifdef USE_DEBUGGER

static void print_enter(int lvl, const char* file, const char* function)
{
        using namespace Debugger;

        if (get_debug_level()>=lvl) {
                if (is_logging()) {
                        QString output = get_tabs() + "ENTERING " + QString(file) + "::" + QString(function) + "\n";
                        log(output);
                        more_tabs();
                } else {
                        fill_tabs();
                        CHANGE_COLOR_GREEN;
                        printf("ENTERING ");
                        CHANGE_COLOR_WHITE;
                        printf("%s::%s\n", file, function);
                        more_tabs();
                }
        }
}

static void print_exit(int lvl, const char* file, const char* function)
{
        using namespace Debugger;

        if (get_debug_level()>=lvl) {
                if (is_logging()) {
                        less_tabs();
                        QString output = get_tabs() + "LEAVING " + QString(file) + "::" + QString(function) + "\n";
                        log(output);
                } else {
                        less_tabs();
                        fill_tabs();
                        CHANGE_COLOR_BLUE;
                        printf("LEAVING ");
                        CHANGE_COLOR_WHITE;
                        printf("%s::%s\n", file, function);
                }
        }
}

FunctionEnter::FunctionEnter(int level, const char* file, const char* function)
                : m_file(file), m_function(function), lvl(level)
{
        print_enter(lvl, m_file, m_function);
}

FunctionEnter::~ FunctionEnter( )
{
        print_exit(lvl, m_file, m_function);
}

ConstructorEnter::ConstructorEnter(int level, const char* file, const char* function)
                : m_file(file), m_function(function), lvl(level)
{
        using namespace Debugger;
        if (get_debug_level()>=lvl) {
                if (is_logging()) {
                        QString output = get_tabs() + "ENTERING " + QString(m_function) + " (CONSTRUCTOR)\n";
                        log(output);
                        more_tabs();
                } else {
                        fill_tabs();
                        CHANGE_COLOR_GREEN;
                        printf("ENTERING ");
                        CHANGE_COLOR_WHITE;
                        printf("%s",m_function);
                        CHANGE_COLOR_CYAN;
                        printf(" (CONSTRUCTOR)");
                        CHANGE_COLOR_WHITE;
                        printf("\n");
                        more_tabs();
                }
        }
}

ConstructorEnter::~ ConstructorEnter( )
{
        using namespace Debugger;
        if (get_debug_level()>=lvl) {
                if (is_logging()) {
                        less_tabs();
                        QString output = get_tabs() + "LEAVING " + QString(m_function) + " (CONSTRUCTOR)\n";
                        log(output);
                } else {
                        less_tabs();
                        fill_tabs();
                        CHANGE_COLOR_BLUE;
                        printf("LEAVING ");
                        CHANGE_COLOR_WHITE;
                        printf("%s", m_function);
                        CHANGE_COLOR_CYAN;
                        printf(" (CONSTRUCTOR)");
                        CHANGE_COLOR_WHITE;
                        printf("\n");
                }
        }
}

DestructorEnter::DestructorEnter(int level, const char* file, const char* function)
                : m_file(file), m_function(function), lvl(level)
{
        using namespace Debugger;
        if (get_debug_level()>=lvl) {
                if (is_logging()) {
                        QString output = get_tabs() + "ENTERING " + QString(m_function) + " (DESTRUCTOR)\n";
                        log(output);
                        more_tabs();
                } else {
                        fill_tabs();
                        CHANGE_COLOR_GREEN;
                        printf("ENTERING ");
                        CHANGE_COLOR_WHITE;
                        printf("%s", m_function);
                        CHANGE_COLOR_CYAN;
                        printf(" (DESTRUCTOR)");
                        CHANGE_COLOR_WHITE;
                        printf("\n");
                        more_tabs();
                }
        }
}

DestructorEnter::~ DestructorEnter( )
{
        using namespace Debugger;
        if (get_debug_level()>=lvl) {
                if (is_logging()) {
                        less_tabs();
                        QString output = get_tabs() + "LEAVING " + QString(m_function) + " (DESTRUCTOR)\n";
                        log(output);
                } else {
                        less_tabs();
                        fill_tabs();
                        CHANGE_COLOR_BLUE;
                        printf("LEAVING ");
                        CHANGE_COLOR_WHITE;
                        printf("%s", m_function);
                        CHANGE_COLOR_CYAN;
                        printf(" (DESTRUCTOR)");
                        CHANGE_COLOR_WHITE;
                        printf("\n");
                }
        }
}
#endif


#if defined (USE_MEM_CHECKER)

//: C02:MemCheck.cpp {O}
// From "Thinking in C++, Volume 2", by Bruce Eckel & Chuck Allison.
// (c) 1995-2004 MindView, Inc. All Rights Reserved.
// See source code use permissions stated in the file 'License.txt',
// distributed with the code package available at www.MindView.net.
#include <cstdio>
	#include <cstdlib>
	#include <cassert>
	#include <cstddef>
using namespace std;
#undef new

// Global flags set by macros in MemCheck.h
bool traceFlag = true;
bool activeFlag = false;

namespace
{

// Memory map entry type
struct Info
{
        void* ptr;
        const char* file;
        long line;
        size_t size;
};

// Memory map data
const size_t MAXPTRS = 10000u;
Info memMap[MAXPTRS];
size_t nptrs = 0;

// Searches the map for an address
int findPointer(void* p)
{
        for(size_t i = 0; i < nptrs; ++i)
                if(memMap[i].ptr == p)
                        return i;
        return -1;
}

void delPointer(void* p)
{
        long pos = findPointer(p);
        assert(pos >= 0);
        // Remove pointer from map
        for(size_t i = pos; i < nptrs-1; ++i)
                memMap[i] = memMap[i+1];
        --nptrs;
}

// Dummy type for static destructor
struct Sentinel
{
        ~Sentinel()
        {
                if(nptrs > 0) {
                        printf("Leaked memory at:\n");
                        for(size_t i = 0; i < nptrs; ++i)
                                printf("\t%p (file: %s, line %ld)\n",
                                       memMap[i].ptr, memMap[i].file, memMap[i].line);
                } else
                        printf("No memory leaks found in Traverso!\n");
        }
};

// Static dummy object
Sentinel s;

} // End anonymous namespace

// Overload scalar new
void*
operator new(size_t siz, const char* file, long line)
{
        void* p = malloc(siz);
        if(activeFlag) {
                if(nptrs == MAXPTRS) {
                        printf("memory map too small (increase MAXPTRS)\n");
                        exit(1);
                }
                memMap[nptrs].ptr = p;
                memMap[nptrs].file = file;
                memMap[nptrs].line = line;
                memMap[nptrs].size = siz;

                ++nptrs;
        }
        if(traceFlag) {
                printf("Allocated %u bytes at address %p (file: %s, line: %ld)\n", siz, p, file, line);
        }
        fflush(NULL);
        return p;
}

// Overload array new
void*
operator new[](size_t siz, const char* file, long line)
{
        return operator new(siz, file, line);
}

// Override scalar delete
void operator delete(void* p)
{
        int i;
        if( (i = findPointer(p)) >= 0) {
                free(p);
                assert(nptrs > 0);
                delPointer(p);
                if(traceFlag)
                        printf("Deleted %u bytes at address %p, file: %s, line: %ld\n", memMap[i].size, p, memMap[i].file, memMap[i].line);
        } else if(!p && activeFlag)
                printf("Attempt to delete unknown pointer: %p\n", p);
        fflush(NULL);
}

// Override array delete
void operator delete[](void* p)
{
        operator delete(p);
} ///:~


#endif  //USE_MEM_CHECKER ///:~


//eof

