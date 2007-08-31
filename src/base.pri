##### Do NOT modify this!###############
CONFIG -= release debug
########################################


###### START USER CONFIGURATION ########
########################################

#
# Choose debug or release build
#

CONFIG += debug
#CONFIG += release

#
# Add support for Jack / ALSA audio driver. If you have a 
# Mac OS X, ALSA support will be disabled automatically.
#

DEFINES += JACK_SUPPORT
DEFINES += ALSA_SUPPORT
#DEFINES += PORTAUDIO_SUPPORT
DEFINES += LV2_SUPPORT
DEFINES += QT_OPENGL_SUPPORT

#
# Uncomment the line which notes your CPU type
#

#QMAKE_CXXFLAGS_RELEASE += -mtune=pentium3
#QMAKE_CXXFLAGS_RELEASE += -mtune=pentium4
#QMAKE_CXXFLAGS_RELEASE += -mtune=pentium-m
# nocona seems to work better for core2duo
# QMAKE_CXXFLAGS_RELEASE += -march=nocona
# gcc 4.3 has a core2 flag, though it's not released yet.
#QMAKE_CXXFLAGS_RELEASE += -march=core2


#########################################
#	END USER CONFIGURATION 		#
#########################################




#################################################
#  OPTIONS WHO COULD BE USEFULL FOR PACKAGERS	#
#################################################

#
# uncomment if you have a patched Qt 4.3.0 !
#
# DEFINES += QT_430_SCROLLBAR_FIX


# DEFINES += STATIC_BUILD

#
# Use Memory Locking 
#
DEFINES += USE_MLOCK


#################################################
#	NO CHANGES BELOW THIS LINE!!		#
#################################################


!macx{
	DEFINES += PRECOMPILED_HEADER
}

# DEFINES += THREAD_CHECK
# QMAKE_CXXFLAGS +=  -fstack-protector-all


CONFIG += warn_on qt
QT += xml

MOC_DIR = build 
UI_DIR = build 
OBJECTS_DIR = build 


debug {
	DEFINES += USE_DEBUGGER
#	DEFINES += USE_MEM_CHECKER
}

release {
	DEFINES += QT_NO_DEBUG
}

unix {
	
	release {
		
#		DEFINES += USE_DEBUGGER
#		QMAKE_CXXFLAGS_RELEASE += -g
#		QMAKE_CXXFLAGS_RELEASE += -finline-functions
		
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
	
	system(which relaytool 2>/dev/null >/dev/null) {
		DEFINES += RELAYTOOL_JACK="'extern int libjack_is_present; extern int libjack_symbol_is_present(char *s);'"

		DEFINES += RELAYTOOL_WAVPACK="'extern int libwavpack_is_present; extern int libwavpack_symbol_is_present(char *s);'"
	} else {
		DEFINES += RELAYTOOL_JACK="'static const int libjack_is_present=1; static int __attribute__((unused)) libjack_symbol_is_present(char *) { return 1; }'"

		# put WAVPACK def here so that we can check if it exists globally
		# so that traverso/ExportWidget.cpp knows if it should offer
		# WAVPACK as an export option
		DEFINES += RELAYTOOL_WAVPACK="'static const int libwavpack_is_present=1; static int __attribute__((unused)) libwavpack_symbol_is_present(char *) { return 1; }'"
	}
	
	contains(DEFINES, PRECOMPILED_HEADER):CONFIG += precompile_header
	
	contains(GCCVERSION, [45].[012345].[012345] ) {
#		Makes only sense if there are loops to vectorize, which isn't the case (yet)
#		Maybe gcc 4.2 will do a better job on certain loops?
#		specially those in memops. would be nice if they get some optimization too...
#		and of course the ones in gdither etc.
#		QMAKE_CXXFLAGS_RELEASE += -ftree-vectorizer-verbose=2 -ftree-vectorize
	}
	
	# Enable large file support
	QMAKE_CXXFLAGS += $$system(getconf LFS_CFLAGS)
	
}

!unix {
	# other systems have no relaytool
	DEFINES += RELAYTOOL_WAVPACK='"static const int libwavpack_is_present=1; static int __attribute__((unused)) libwavpack_symbol_is_present(char *) { return 1; }"'
}

macx {
	DEFINES += OSX_BUILD
	DEFINES += PORTAUDIO_SUPPORT
	DEFINES -= ALSA_SUPPORT
	DEFINES -= JACK_SUPPORT
	QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/

	RC_FILE = ../../resources/images/traverso_mac.icns
}

win32 { 
	DEFINES += WIN_BUILD
	DEFINES += PORTAUDIO_SUPPORT
	DEFINES -= ALSA_SUPPORT
	DEFINES -= JACK_SUPPORT
	DEFINES -= USE_MLOCK
	DEFINES -= LV2_SUPPORT
	
	release {
   	        DEFINES -= USE_DEBUGGER
	}
}
