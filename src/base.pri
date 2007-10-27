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
DEFINES += PORTAUDIO_SUPPORT
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


DEFINES += STATIC_BUILD

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
		
		X86_FLAGS = $$system(cat /proc/cpuinfo | grep '^flags')
		
		HOST_SUPPORTS_SSE = 0
		
		contains(X86_FLAGS, sse) {
			HOST_SUPPORTS_SSE = 1
			DEFINES += SSE_OPTIMIZATIONS
		}
		
		contains(X86_FLAGS, mmx) {
			QMAKE_CXXFLAGS_RELEASE += -mmmx
		}
		
		contains(X86_FLAGS, 3dnow) {
			QMAKE_CXXFLAGS_RELEASE += -m3dnow
		}
		
		contains(MACHINETYPE, i586) {
			QMAKE_CXXFLAGS_RELEASE += -march=i586
		}

		contains(MACHINETYPE, i686) {
			QMAKE_CXXFLAGS_RELEASE += -march=i686
			eval(HOST_SUPPORTS_SSE == 1) {
				QMAKE_CXXFLAGS_RELEASE += -msse -mfpmath=sse
				DEFINES += USE_XMMINTRIN
			}
		}

		contains(MACHINETYPE, x86_64) {
			eval(HOST_SUPPORTS_SSE == 1) {
				QMAKE_CXXFLAGS_RELEASE += -msse -mfpmath=sse
				DEFINES += USE_XMMINTRIN USE_X86_64_ASM
			}
		}
		
		contains(MACHINETYPE, i[456]86) {
			DEFINES += ARCH_X86
		}

		
	}
	
	GCCVERSION = $$system(gcc -dumpversion)
	
	system(which relaytool 2>/dev/null >/dev/null) {
		DEFINES += RELAYTOOL_PRESENT
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


macx {
	DEFINES += OSX_BUILD
	DEFINES += PORTAUDIO_SUPPORT
	DEFINES -= ALSA_SUPPORT
	DEFINES -= JACK_SUPPORT
	QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/

	RC_FILE = ../../resources/images/traverso_mac.icns
	
# Uncomment if dest. target is (at least) tiger (works maybe on other targets as well ?)
# DEFINES += BUILD_VECLIB_OPTIMIZATIONS
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
