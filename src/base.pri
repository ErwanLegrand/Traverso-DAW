##### Do NOT modify this!###############
CONFIG -= release debug
################################


###### START USER CONFIGURATION ########

#
# Choose debug or release build
#

#CONFIG += debug
CONFIG += release

#
# Add support for Jack / ALSA audio driver. If you have a 
# Mac OS X, ALSA support will be disabled automatically.
#

DEFINES += JACK_SUPPORT
DEFINES += ALSA_SUPPORT
#DEFINES += USE_MEM_CHECKER

#
# Uncomment the line which notes your CPU type
#

QMAKE_CXXFLAGS_RELEASE += -g -mtune=pentium3
#QMAKE_CXXFLAGS_RELEASE += -mtune=pentium4
#QMAKE_CXXFLAGS_RELEASE += -mtune=pentium-m

#
# Use Memory Locking 
#

DEFINES += USE_MLOCK

####### END USER CONFIGURATION ########




CONFIG += warn_on \
	  link_prl \
	  qt

QT += xml


MOC_DIR = build 
UI_DIR = build 
OBJECTS_DIR = build 




debug {
	DEFINES += USE_DEBUGGER
#	DEFINES += USE_MEM_CHECKER
}


unix {
	release {
		MACHINETYPE = $$system(arch)
		
		contains( MACHINETYPE, x86_64 )	{
			QMAKE_CXXFLAGS_RELEASE += -mtune=athlon64
		}
		
		contains( MACHINETYPE, i[456]86) {
			DEFINES += SSE_OPTIMIZATIONS
			QMAKE_CXXFLAGS_RELEASE += -msse -mfpmath=sse
		}
	}
	
	GCCVERSION = $$system(gcc -dumpversion)
	
	contains(GCCVERSION, [45].[01234].[012345] ) {
		CONFIG += precompile_header
	}
	
}

macx {
	DEFINES -= ALSA_SUPPORT
}
