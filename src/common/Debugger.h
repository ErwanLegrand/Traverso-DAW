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

    $Id: Debugger.h,v 1.1 2007/10/20 17:38:16 r_sijrier Exp $
*/

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QString>


//Debugging Macros
#define CHANGE_COLOR_BLACK   printf("%c[0;31m",27)
#define CHANGE_COLOR_RED     printf("%c[0;31m",27)
#define CHANGE_COLOR_GREEN   printf("%c[0;32m",27)
#define CHANGE_COLOR_ORANGE  printf("%c[0;33m",27)
#define CHANGE_COLOR_BLUE    printf("%c[0;34m",27)
#define CHANGE_COLOR_MAGENTA printf("%c[0;35m",27)
#define CHANGE_COLOR_CYAN    printf("%c[0;36m",27)
#define CHANGE_COLOR_WHITE   printf("%c[0;37m",27)
#define CHANGE_COLOR_YELLOW  printf("%c[0;33m",27)

#ifdef USE_DEBUGGER

class FunctionEnter
{
	const char* 	m_file;
	const char* 	m_function;
	int 			lvl;
public:
	FunctionEnter(int level, const char* file, const char* function);
	~FunctionEnter();
};

class ConstructorEnter
{
	const char* 	m_file;
	const char* 	m_function;
	int 			lvl;
public:
	ConstructorEnter(int level, const char* file, const char* function);
	~ConstructorEnter();
};

class DestructorEnter
{
	const char* 	m_file;
	const char* 	m_function;
	int 			lvl;
public:
	DestructorEnter(int level, const char* file, const char* function);
	~DestructorEnter();
};

#define PMESG(args...)          { using namespace Debugger; if (get_debug_level()>=BASIC)       { if (is_logging())  { QString x; x.sprintf(args); QString output = get_tabs() + "[ " + x + " ]\n"; log(output); } else { fill_tabs(); CHANGE_COLOR_MAGENTA; printf("[ "); printf(args); printf(" ]"); CHANGE_COLOR_WHITE; printf("\n"); } } }
#define PMESG2(args...)         { using namespace Debugger; if (get_debug_level()>=FLOOD)       { if (is_logging())  { QString x; x.sprintf(args); QString output = get_tabs() + "[ " + x + " ]\n"; log(output); } else { fill_tabs(); CHANGE_COLOR_MAGENTA; printf("[ "); printf(args); printf(" ]"); CHANGE_COLOR_WHITE; printf("\n"); } } }
#define PMESG3(args...)         { using namespace Debugger; if (get_debug_level()>=SUPER_FLOOD) { if (is_logging())  { QString x; x.sprintf(args); QString output = get_tabs() + "[ " + x + " ]\n"; log(output); } else { fill_tabs(); CHANGE_COLOR_MAGENTA; printf("[ "); printf(args); printf(" ]"); CHANGE_COLOR_WHITE; printf("\n"); } } }
#define PMESG4(args...)         { using namespace Debugger; if (get_debug_level()>=ALL)         { if (is_logging())  { QString x; x.sprintf(args); QString output = get_tabs() + "[ " + x + " ]\n"; log(output); } else { fill_tabs(); CHANGE_COLOR_MAGENTA; printf("[ "); printf(args); printf(" ]"); CHANGE_COLOR_WHITE; printf("\n"); } } }

#define PDEBUG(args...)         { using namespace Debugger; if (is_logging()) { QString x; x.sprintf(args); QString output = "DEBUG : " + QString(__FILE__) + "::" + QString(__FUNCTION__) + ":" + x + "\n"; log(output); } else { CHANGE_COLOR_GREEN; printf("DEBUG : ");printf("%s",__FILE__); printf("::"); printf("%s",__FUNCTION__); printf(":"); printf(args); CHANGE_COLOR_WHITE; printf("\n"); } }
#define PERROR(args...)         { using namespace Debugger; if (is_logging()) { QString x; x.sprintf(args); QString output = "\n *** Error in " + QString(__PRETTY_FUNCTION__) + "\n" + x + "\n\n"; } else {  printf("\n"); CHANGE_COLOR_RED; printf("*** Error in "); printf("%s",__PRETTY_FUNCTION__); printf("\n"); printf(args); CHANGE_COLOR_WHITE; printf("\n\n"); } }
#define PERROR2(args...)        { using namespace Debugger; if (is_logging()) { QString x; x.sprintf(args); QString output = "\n *** Error in " + QString(__PRETTY_FUNCTION__) + "\n" + x + "\n\n"; } else if (get_debug_level()>=FLOOD) {  printf("\n"); CHANGE_COLOR_RED; printf("*** Error in "); printf("%s",__PRETTY_FUNCTION__); printf("\n"); printf(args); CHANGE_COLOR_WHITE; printf("\n\n"); } }
#define PWARN(args...)          { using namespace Debugger; if (is_logging()) { QString x; x.sprintf(args); QString output = "WARNING: " + x + "\n"; log(output); } else { CHANGE_COLOR_YELLOW; printf("WARNING: "); printf(args); CHANGE_COLOR_WHITE; printf("\n"); } }
#define PWARN2(args...)         { using namespace Debugger; if (get_debug_level()>=FLOOD) { if (is_logging()) { QString x; x.sprintf(args); QString output = "WARNING: " + x + "\n"; log(output); } else { CHANGE_COLOR_YELLOW; printf("WARNING: "); printf(args); CHANGE_COLOR_WHITE; printf("\n"); } } }


#define PENTER			FunctionEnter enter(Debugger::BASIC, __FILE__, __FUNCTION__)
#define PENTER2			FunctionEnter enter(Debugger::FLOOD, __FILE__, __FUNCTION__)
#define PENTER3			FunctionEnter enter(Debugger::SUPER_FLOOD, __FILE__, __FUNCTION__)
#define PENTER4			FunctionEnter enter(Debugger::ALL, __FILE__, __FUNCTION__)

#define PENTERCONS              ConstructorEnter enter(Debugger::BASIC, __FILE__, __FUNCTION__)
#define PENTERDES                 DestructorEnter enter(Debugger::BASIC, __FILE__, __FUNCTION__)
#define PENTERCONS2            ConstructorEnter enter(Debugger::FLOOD, __FILE__, __FUNCTION__)
#define PENTERDES2               DestructorEnter enter(Debugger::FLOOD, __FILE__, __FUNCTION__)
#define PENTERCONS3            ConstructorEnter enter(Debugger::SUPER_FLOOD, __FILE__, __FUNCTION__)
#define PENTERDES3               DestructorEnter enter(Debugger::SUPER_FLOOD, __FILE__, __FUNCTION__)
#define PENTERCONS4            ConstructorEnter enter(Debugger::ALL, __FILE__, __FUNCTION__)
#define PENTERDES4               DestructorEnter enter(Debugger::ALL, __FILE__, __FUNCTION__)

#else

#define PMESG(args...)        
#define PMESG2(args...)       
#define PMESG3(args...)       
#define PMESG4(args...)       

#define PMESG_START(args...)  
#define PMESG_END(args...)    
#define PMESG2_START(args...) 
#define PMESG2_END(args...)   

#define PDEBUG(args...)       
#define PERROR(args...)       
#define PERROR2(args...)      
#define PWARN(args...)        
#define PWARN2(args...)       
#define PMARK(args...)        

#define PENTER        
#define PENTER2       
#define PENTER3       
#define PENTER4       
#define PENTERCONS    
#define PENTERDES     
#define PENTERCONS2   
#define PENTERDES2    
#define PENTERCONS3   
#define PENTERDES3    
#define PENTERCONS4   
#define PENTERDES4    

#endif
/*!
 Debugger is a collection of macros that makes easier the job of debugging a  Application.

 PENTER - Outputs a message when entering a method in level 1. Used in the FIRST line of a method;
 PEXIT  - Outputs a message when leaving a method in level 1. Used in the LAST line of a method,
            except if the last line is a return statement (in this case is put immediately before
            the return statement
 PENTER2 - Same as PENTER for levels 1  and 2
 PEXIT2  - Same as PEXIT for levels 1 and 2
 PENTER3 - Same as PENTER for levels 1 2 and 3
 PEXIT3  - Same as PEXIT for levels 1 2 and 3
 PENTER4 - Same as PENTER for levels 1 2 3 and 4
 PEXIT4  - Same as PEXIT for levels 1 2 3 and 4
 PMESG(message) - Outputs a message in level 1
 PMESG2(message) - Outputs a message in level 1 and 2
 PENTERCONS - Outputs a message when entering a constructor in levels 2, 3, and 4. Similar to PENTER
 PEXITCONS  - Outputs a message when leaving a constructor in levels 2, 3, and 4. Similar to PEXIT
 PENTERDES  - Outputs a message when entering a destructor in levels 2, 3, and 4. Similar to PENTER
 PEXITDES   - Outputs a message when leaving a destructor in levels 2, 3, and 4. Similar to PEXIT
 Same can be done for PENTERCONS2, PEXITCONS2, PENTERCONS3... and so on...
 */

namespace Debugger
        {
        static const int OFF = 0;         //< no debug output at all
        static const int BASIC = 1;       //< only level 1 calls
        static const int FLOOD = 2;       //< level 1 , level 2 and constructors/destructor calls
        static const int SUPER_FLOOD = 3; //< all previous plus low level JMB messages
        static const int ALL = 4;         //< all messages (including in timer loops)

        //! Used internally by Debugger. Align the output with the level of execution in a given moment
        void fill_tabs();

        //! Used internally by Debugger. Get a " " (space) sequence whch aligns the output with the level of execution in a given moment,
        QString get_tabs();


        //! Used internally by Debugger. Increase one level of execution in output messages
        void more_tabs();

        //! Used internally by Debugger. Decrease one level of execution in output messages
        void less_tabs();

        //! Set the debug level
        void set_debug_level(int l);

        //! Used internally by Debugger. Returns true if debugOn flag is true.
        int get_debug_level();

        //! create a log file "fn" under home dir and enable copy of all debugging messagem to this file.
        void create_log(QString fn);

        //! close the log file
        void close_log();

        //! Used internally by Debugger. Feed the log file.
        void log(QString msg);

        //! Used internally to check if output is stdout or a log file
        bool is_logging();
        }
        

#endif // DEBUGGER_H ///:~




//: C02:MemCheck.h
// From "Thinking in C++, Volume 2", by Bruce Eckel & Chuck Allison.
// (c) 1995-2004 MindView, Inc. All Rights Reserved.
// See source code use permissions stated in the file 'License.txt',
// distributed with the code package available at www.MindView.net.
#ifndef MEMCHECK_H
#define MEMCHECK_H
#include <cstddef>  // For size_t

#if defined (USE_MEM_CHECKER)

// Usurp the new operator (both scalar and array versions)
void* operator new(std::size_t, const char*, long);
void* operator new[](std::size_t, const char*, long);
#define new new (__FILE__, __LINE__)

extern bool traceFlag;
#define TRACE_ON() traceFlag = true
#define TRACE_OFF() traceFlag = false

extern bool activeFlag;
#define MEM_ON() activeFlag = true
#define MEM_OFF() activeFlag = false


#else

#define TRACE_OFF()
#define TRACE_ON()
#define MEM_OFF()
#define MEM_ON()

#endif // MEMCHECK_H ///:~


#endif  //USE_MEM_CHECKER ///:~
