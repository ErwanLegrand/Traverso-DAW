#CMAKE SETUP
#Require 2.4.2 for Qt finding
#Require 2.4.3 for moc change detection
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.3 FATAL_ERROR)

SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)
SET(CMAKE_SKIP_RULE_DEPENDENCY TRUE)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

#PROJECT SETUP
PROJECT(traverso)
SET (VERSION_MAJOR "0")
SET (VERSION_MINOR "42")
SET (VERSION_PATCH "0")
SET (VERSION_SUFFIX "")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})

MESSAGE(STATUS "Traverso ${VERSION} will be built to install into ${CMAKE_INSTALL_PREFIX}")

OPTION(WANT_JACK    	"Include Jack (Jack Audio Connection Kit) support" ON)
OPTION(WANT_ALSA    	"Include ALSA (Advanced Linux Sound Architecture) support" ON)
OPTION(WANT_PORTAUDIO	"Include PortAudio support (Driver support for Mac OS X and Windows" OFF)
OPTION(WANT_LV2		"Include LV2 Plugin support" ON)
OPTION(USE_SYSTEM_SLV2_LIBRARY "Use system installed slv2 library. If not found, the internal one is compiled and used which needs the rdf development headers" OFF)
OPTION(WANT_MP3_DECODE	"Include mp3 decoding support, for playing mp3 files" ON)
OPTION(WANT_MP3_ENCODE	"Include mp3 encoding support, for creating mp3 files" OFF)
OPTION(WANT_PCH     	"Use precompiled headers" OFF)
OPTION(WANT_DEBUG   	"Debug build" OFF)
OPTION(WANT_OPENGL	"Build Traverso with OpenGL support" OFF)
OPTION(WANT_TRAVERSO_DEBUG "Provides 4 levels of debug ouput on the command line, always on for DEBUG builds" OFF)
OPTION(WANT_THREAD_CHECK	"Checks at runtime if functions are called from the correct thread, used by developers for debugging" OFF)
OPTION(WANT_VECLIB_OPTIMIZATIONS "Build with veclib optimizations (Only for PPC based Mac OS X)" OFF)


SET(MAIN_DIR_NAME "src")
IF(TAG_VERSION)
  MESSAGE(STATUS "Previously selected version tag: ${TAG_VERSION}")
  SET(WANT_VERSIONING 1)
  SET(CACHED_VERSIONING 1)
ENDIF(TAG_VERSION)
IF (WANT_VERSIONING AND NOT CACHED_VERSIONING)
  IF (NOT CUSTOM_VERSIONTAG)
    SET(TAG_VERSION "-${VERSION}" CACHE TYPE STRING)
  ELSE (NOT CUSTOM_VERSIONTAG)
    SET(TAG_VERSION ${CUSTOM_VERSIONTAG} CACHE TYPE STRING)
  ENDIF (NOT CUSTOM_VERSIONTAG)
ENDIF (WANT_VERSIONING AND NOT CACHED_VERSIONING)



# Static build is perfectly fine now, targets are properly relinked when needed!
SET(TRAVERSO_DEFINES -DSTATIC_BUILD)

#BUILD SETUP
#None, Debug, Release, .. or custom ones
IF(WANT_DEBUG)
  SET(CMAKE_BUILD_TYPE DEBUG)
  SET(WANT_TRAVERSO_DEBUG ON)
ELSE(WANT_DEBUG)
  SET(CMAKE_BUILD_TYPE RELEASE)
  LIST(APPEND TRAVERSO_DEFINES -DQT_NO_DEBUG)
ENDIF(WANT_DEBUG)


IF(WANT_TRAVERSO_DEBUG)
  LIST(APPEND TRAVERSO_DEFINES -DUSE_DEBUGGER)
ENDIF(WANT_TRAVERSO_DEBUG)


# Moves the Traverso exec. to the root dir and libs to the root's lib dir.
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)


SET(MINIMAL_ALSA_VERSION 1.0.0)
SET(MINIMAL_JACK_VERSION 0.100)
SET(MINIMAL_PORTAUDIO_VERSION 19)
SET(MINIMAL_SLV2_VERSION 0.3.0)
SET(MINIMAL_WAVPACK_VERSION 4.40.0)
SET(MINIMAL_OGG_VORBIS_VERSION 1.1.2)
SET(MINIMAL_MAD_VERSION 0.15.0)
SET(MINIMAL_FLAC_VERSION 1.1.2)
SET(MINIMAL_FFTW_VERSION 3.0.0)
SET(MINIMAL_REDLAND_VERSION 1.0.2)

SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -Wall ${CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -fPIC ${CXX_FLAGS}")

# Detect the flags to be used for large file support
execute_process(
	COMMAND getconf LFS_CFLAGS
	OUTPUT_VARIABLE getconf_invoke_result
	RESULT_VARIABLE getconf_failed
)

# remove linebrakes, white spaces (getconf_invoke_result)
string(REGEX REPLACE "[\r\n]"                  " " getconf_invoke_result "${getconf_invoke_result}")

# now we have the proper flags for large file support, so set these!
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${getconf_invoke_result}")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${getconf_invoke_result}")


INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)


# Check for PKG-CONFIG
FIND_PACKAGE(PkgConfig REQUIRED)
IF(PKG_CONFIG_FOUND)
    MESSAGE(STATUS "Program pkg-config found (${PKG_CONFIG_EXECUTABLE})")
ELSE(PKG_CONFIG_FOUND)
    MESSAGE(FATAL_ERROR "Program pkg-config not found")
ENDIF(PKG_CONFIG_FOUND)


#FIND DEPENDENCIES
SET (QT_MIN_VERSION "4.2.0")
FIND_PACKAGE(Qt4 REQUIRED)


include(${QT_USE_FILE})


INCLUDE (ConfigureChecks.cmake)

IF(WANT_OPENGL)
	IF(QT_QTOPENGL_FOUND)
		LIST(APPEND TRAVERSO_DEFINES -DQT_OPENGL_SUPPORT) 
		SET(HAVE_OPENGL TRUE)
	ENDIF(QT_QTOPENGL_FOUND)
ELSE(WANT_OPENGL)
	SET(HAVE_OPENGL FALSE)
ENDIF(WANT_OPENGL)



IF(WANT_PORTAUDIO)
	CHECK_INCLUDE_FILE("portaudio.h" HAVE_PORTAUDIO_H)
	IF(NOT HAVE_PORTAUDIO_H)
		MESSAGE(FATAL_ERROR "PortAudio support requested, but PortAudio development headers could not be found!\nPlease install the PortAudio development package, remove CMakeCache.txt and run cmake again")
	ENDIF(NOT HAVE_PORTAUDIO_H)
	PKG_CHECK_MODULES(PORTAUDIO portaudio-2.0>=${MINIMAL_PORTAUDIO_VERSION})
	IF(PORTAUDIO_FOUND)
		LIST(APPEND TRAVERSO_DEFINES -DPORTAUDIO_SUPPORT)
		MESSAGE("-- PortAudio Library Found OK")
		SET(HAVE_PORTAUDIO TRUE)
	ENDIF(PORTAUDIO_FOUND)
ELSE(WANT_PORTAUDIO)
		SET(HAVE_PORTAUDIO FALSE)
ENDIF(WANT_PORTAUDIO)


IF(WANT_LV2)
	IF(USE_SYSTEM_SLV2_LIBRARY)
		CHECK_INCLUDE_FILE("slv2/slv2.h" HAVE_SLV2_H)
		IF(NOT HAVE_SLV2_H)
			MESSAGE(FATAL_ERROR "lv2 support requested, but slv2 development headers could not be found!\nPlease install the slv2 development package (Usually called libslv2-dev), remove CMakeCache.txt and run cmake again")
		ENDIF(NOT HAVE_SLV2_H)
		
		PKG_CHECK_MODULES(SLV2  libslv2>=${MINIMAL_SLV2_VERSION})
		IF (SLV2_FOUND)
			MESSAGE("-- slv2 Library Found OK")
			SET(HAVE_SYSTEM_SLV2 TRUE)
		ENDIF(SLV2_FOUND)
	ELSE(USE_SYSTEM_SLV2_LIBRARY)
		PKG_CHECK_MODULES(REDLAND  redland>=${MINIMAL_REDLAND_VERSION})
		IF (REDLAND_FOUND)
			MESSAGE("-- REDLAND Library Found OK")
			INCLUDE_DIRECTORIES(/usr/include/redland /usr/include/rasqal)
		ELSE(REDLAND_FOUND)
			MESSAGE("-- REDLAND Library not found with pkg-config, looking for headers now")
			INCLUDE_DIRECTORIES(/usr/include/redland /usr/include/rasqal)
			CHECK_INCLUDE_FILE("librdf.h" HAVE_LIBRDF_H)
			IF(NOT HAVE_LIBRDF_H)
				CHECK_INCLUDE_FILE("redland/librdf.h" HAVE_REDLAND_LIBRDF_H)
				IF(NOT HAVE_REDLAND_LIBRDF_H)
					MESSAGE(FATAL_ERROR "lv2 support requested using internal slv2 lib, but the needed rdf development headers could not be found!\nPlease install the rdf development package (Usually called librdf-dev), remove CMakeCache.txt and run cmake again")
				ENDIF(NOT HAVE_REDLAND_LIBRDF_H)
			ENDIF(NOT HAVE_LIBRDF_H)
			MESSAGE("-- REDLAND Library Found OK")
		ENDIF(REDLAND_FOUND)
		SET(USE_INTERNAL_SLV2_LIB TRUE)
	ENDIF(USE_SYSTEM_SLV2_LIBRARY)
	
	LIST(APPEND TRAVERSO_DEFINES -DLV2_SUPPORT) 
	SET(HAVE_SLV2 TRUE)
ELSE(WANT_LV2)
	SET(HAVE_SLV2 FALSE)
ENDIF(WANT_LV2)


CHECK_INCLUDE_FILE("wavpack/wavpack.h" HAVE_WAVPACK_H)
IF(NOT HAVE_WAVPACK_H)
	MESSAGE(FATAL_ERROR "WavPack development headers could not be found!\nPlease install the WavPack development package, remove CMakeCache.txt and run cmake again")
ENDIF(NOT HAVE_WAVPACK_H)
PKG_CHECK_MODULES(WAVPACK  wavpack>=${MINIMAL_WAVPACK_VERSION})
IF (WAVPACK_FOUND)
  MESSAGE("-- WavPack Library Found OK")
ELSE(WAVPACK_FOUND)
  MESSAGE(FATAL_ERROR "No WavPack found (WavPack >= ${MINIMAL_WAVPACK_VERSION} needs to be installed!)")
ENDIF(WAVPACK_FOUND)


CHECK_INCLUDE_FILE("vorbis/vorbisfile.h" HAVE_VORBISFILE_H)
IF(NOT HAVE_VORBISFILE_H)
	MESSAGE(FATAL_ERROR "Ogg Vorbis development headers could not be found!\nPlease install the Ogg Vorbis development package, remove CMakeCache.txt and run cmake again")
ENDIF(NOT HAVE_VORBISFILE_H)
PKG_CHECK_MODULES(VORBIS  vorbis>=${MINIMAL_OGG_VORBIS_VERSION})
IF (VORBIS_FOUND)
  MESSAGE("-- Ogg Vorbis Library Found OK")
ELSE(VORBIS_FOUND)
  MESSAGE(FATAL_ERROR "No Ogg Vorbis found (Ogg Vorbis >= ${MINIMAL_OGG_VORBIS_VERSION} needs to be installed!)")
ENDIF(VORBIS_FOUND)


CHECK_INCLUDE_FILE("FLAC/export.h" HAVE_FLAC_H)
IF(NOT HAVE_FLAC_H)
	MESSAGE(FATAL_ERROR "FLAC development headers could not be found!\nPlease install the FLAC development package, remove CMakeCache.txt and run cmake again")
ENDIF(NOT HAVE_FLAC_H)
PKG_CHECK_MODULES(FLAC  flac>=${MINIMAL_FLAC_VERSION})
IF (FLAC_FOUND)
  MESSAGE("-- FLAC Library Found OK")
ELSE(FLAC_FOUND)
  MESSAGE(FATAL_ERROR "No FLAC found (FLAC >= ${MINIMAL_FLAC_VERSION} needs to be installed!)")
ENDIF(FLAC_FOUND)


IF(WANT_MP3_DECODE)
	CHECK_INCLUDE_FILE("mad.h" HAVE_MAD_H)
	IF(NOT HAVE_MAD_H)
		MESSAGE(FATAL_ERROR "MP3 decoding support requested, but mp3 development headers (mad.h) could not be found!\nPlease install the mp3 development package (Usually called libmad0-dev), remove CMakeCache.txt and run cmake again")
	ENDIF(NOT HAVE_MAD_H)
	PKG_CHECK_MODULES(MAD mad>=${MINIMAL_MAD_VERSION})
	IF(MAD_FOUND)
		LIST(APPEND TRAVERSO_DEFINES -DMP3_DECODE_SUPPORT)
		MESSAGE("-- MP3 Decoding Library Found OK")
		SET(HAVE_MP3_DECODING TRUE)
	ENDIF(MAD_FOUND)
ELSE(WANT_MP3_DECODE)
	SET(HAVE_MP3_DECODING FALSE)
ENDIF(WANT_MP3_DECODE)


IF(WANT_MP3_ENCODE)
	CHECK_INCLUDE_FILE("lame/lame.h" HAVE_LAME_H)
	IF(NOT HAVE_LAME_H)
		MESSAGE(FATAL_ERROR "MP3 encoding support requested, but mp3 development headers (lame.h) could not be found!\nPlease install the lame development package (Usually called liblame-dev), remove CMakeCache.txt and run cmake again")
	ENDIF(NOT HAVE_LAME_H)
		LIST(APPEND TRAVERSO_DEFINES -DMP3_ENCODE_SUPPORT)
		MESSAGE("-- Building with mp3 encoding support enabled")
		SET(HAVE_MP3_ENCODING TRUE)
	ENDIF(LAME_FOUND)
ELSE(WANT_MP3_ENCODE)
	SET(HAVE_MP3_ENCODING FALSE)
ENDIF(WANT_MP3_ENCODE)


CHECK_INCLUDE_FILE("fftw3.h" HAVE_FFTW3_H)
IF(NOT HAVE_FFTW3_H)
	MESSAGE(FATAL_ERROR "FFTW3 development headers could not be found!\nPlease install the FFTW3 development package (fftw3-dev), remove CMakeCache.txt and run cmake again")
ENDIF(NOT HAVE_FFTW3_H)
PKG_CHECK_MODULES(FFTW3  fftw3>=${MINIMAL_FFTW_VERSION})
IF (FFTW3_FOUND)
  MESSAGE("-- FFTW3 Library Found OK")
ELSE(FFTW3_FOUND)
  MESSAGE(FATAL_ERROR "No FFTW3 found (FFTW3 >= ${MINIMAL_FFTW_VERSION} needs to be installed!)")
ENDIF(FFTW3_FOUND)


CHECK_INCLUDE_FILE("sys/vfs.h" HAVE_SYS_VFS_H)
IF(HAVE_SYS_VFS_H)
	LIST(APPEND TRAVERSO_DEFINES -DHAVE_SYS_VFS_H)
ENDIF(HAVE_SYS_VFS_H)

CHECK_INCLUDE_FILE("sys/stat.h" HAVE_SYS_STAT_H)
IF(HAVE_SYS_STAT_H)
	LIST(APPEND TRAVERSO_DEFINES -DHAVE_SYS_STAT_H)
ENDIF(HAVE_SYS_STAT_H)


#Create configure files.. config.h
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)


#uninstall
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


#Set up include dirs with all found packages
INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/common
${QT_INCLUDE_DIR}
${QT_QTCORE_INCLUDE_DIR}
)


#Set up libraries to link with all found packages
LINK_LIBRARIES (
${QT_QT_LIBRARY}
)


CHECK_FUNCTION_EXISTS(posix_memalign HAVE_POSIX_MEMALIGN)
IF(NOT HAVE_POSIX_MEMALIGN)
	LIST(APPEND TRAVERSO_DEFINES -DNO_POSIX_MEMALIGN)
	MESSAGE("Did not find posix_memalign(), using malloc")
ENDIF(NOT HAVE_POSIX_MEMALIGN)


CHECK_FUNCTION_EXISTS(mlock HAVE_MLOCK)
IF(HAVE_MLOCK)
	LIST(APPEND TRAVERSO_DEFINES -DUSE_MLOCK)
ENDIF(HAVE_MLOCK)


IF(UNIX)
	
#BEGIN Mac OS X section
	IF(APPLE)
# @ MAC OS X builders, I've NO IDEA what this should be, please have a look at it
# It should be possible to create a working bundle with CPack using stuff
# defined here ....???
		SET( ENV{MACOSX_DEPLOYMENT_TARGET} 10.3 ) 
		SET( ENV{LD_PREBIND} 1 )
		SET( ENV{LD_PREBIND_ALLOW_OVERLAP} 1 )

#		Bundle setup
		SET(BUILD_MAC_BUNDLE 1)
		set(MACOSX_BUNDLE_ICON_FILE ${PROJECT_SOURCE_DIR}/resources/images/traverso_mac.icns)
		
		
		IF(WANT_VECLIB_OPTIMIZATIONS)
			LIST(APPEND TRAVERSO_DEFINES -DBUILD_VECLIB_OPTIMIZATIONS)
			LINK_LIBRARIES( "-framework Accelerate" ) 
		ENDIF(WANT_VECLIB_OPTIMIZATIONS)
		
# This was copy pasted from another project, please remove if it's not needed!!
		LINK_LIBRARIES( "-undefined dynamic_lookup" ) 
		LINK_LIBRARIES( "-framework CoreFoundation" ) 
	ENDIF(APPLE)
# END Mac OS X section


#BEGIN LINUX section
	IF(WANT_ALSA)
		CHECK_INCLUDE_FILE("alsa/asoundlib.h" HAVE_ASOUNDLIB_H)
		IF(NOT HAVE_ASOUNDLIB_H)
			MESSAGE(FATAL_ERROR "alsa support requested, but alsa development headers could not be found!\nPlease install the alsa development package (Usually called libasound-dev), remove CMakeCache.txt and run cmake again")
		ENDIF(NOT HAVE_ASOUNDLIB_H)
		
		PKG_CHECK_MODULES(ALSA  alsa>=${MINIMAL_ALSA_VERSION})
		IF (ALSA_FOUND)
			MESSAGE("-- ALSA Library Found OK")
			LIST(APPEND TRAVERSO_DEFINES -DALSA_SUPPORT)
			SET(HAVE_ALSA TRUE)
		ENDIF(ALSA_FOUND)
	ELSE(WANT_ALSA)
		MESSAGE("-- ALSA support: Disabled")
		SET(HAVE_ALSA FALSE)
	ENDIF(WANT_ALSA)


	find_program(RELAYTOOL_EXECUTABLE NAMES relaytool)
	IF(RELAYTOOL_EXECUTABLE)
		LIST(APPEND TRAVERSO_DEFINES -DRELAYTOOL_PRESENT)
	ENDIF(RELAYTOOL_EXECUTABLE)

# END LINUX section


	IF(WANT_JACK)
		CHECK_INCLUDE_FILE("jack/jack.h" HAVE_JACK_H)
		IF(NOT HAVE_JACK_H)
			MESSAGE(FATAL_ERROR "jack support requested, but jack development headers could not be found!\nPlease install the jack development package, remove CMakeCache.txt and run cmake again")
		ENDIF(NOT HAVE_JACK_H)
		
		PKG_CHECK_MODULES(JACK jack>=${MINIMAL_JACK_VERSION})
		IF(JACK_FOUND)
			LIST (APPEND TRAVERSO_DEFINES -DJACK_SUPPORT)
			MESSAGE("-- Jack Library Found OK")
			SET(HAVE_JACK TRUE)
			SET(JACK_LIBS ${JACK_LIBRARIES})
			LIST(APPEND JACK_LIB_DIR ${JACK_LIBDIR} ${JACK_LIBRARY_DIRS})
			LIST(APPEND JACK_INC_DIR ${JACK_INCLUDEDIR} ${JACK_INCLUDE_DIRS})
		ELSE(JACK_FOUND)
			MESSAGE(FATAL_ERROR "jack support requested, but jack libraries could not be found!\nPlease install the jack runtime libraries, remove CMakeCache.txt and run cmake again")
		ENDIF(JACK_FOUND)
	ELSE(WANT_JACK)
		MESSAGE("-- Jack support: Disabled")
		SET(HAVE_JACK FALSE)
	ENDIF(WANT_JACK)

	
	IF(NOT WANT_DEBUG)
#		Detect the architecture we are compiling on
		execute_process(
			COMMAND uname -m
			OUTPUT_VARIABLE uname_invoke_result
			RESULT_VARIABLE uname_failed
		)
# 		Get the cpu flags to add the most optimal optimization flags to the compiler
		execute_process(
			COMMAND cat /proc/cpuinfo
			COMMAND grep ^flags
			OUTPUT_VARIABLE cat_invoke_result
			RESULT_VARIABLE cat_failed
		)
		
		SET(HOST_SUPPORTS_SSE FALSE)
		
		IF(cat_invoke_result MATCHES sse)
			SET(HOST_SUPPORTS_SSE TRUE)
			LIST(APPEND TRAVERSO_DEFINES -DSSE_OPTIMIZATIONS)
		ENDIF(cat_invoke_result MATCHES sse)
		
		IF(cat_invoke_result MATCHES mmx)
			SET(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -mmmx")
		ENDIF(cat_invoke_result MATCHES mmx)
				
		IF(cat_invoke_result MATCHES 3dnow)
			SET(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -m3dnow")
		ENDIF(cat_invoke_result MATCHES 3dnow)
				
		IF(uname_invoke_result MATCHES i586)
			SET(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -march=i586")
		ENDIF(uname_invoke_result MATCHES i586)
				
		IF(uname_invoke_result MATCHES i686)
			SET(uname_invoke_result  "${CMAKE_CXX_FLAGS_RELEASE} -march=i686")
			IF(HOST_SUPPORTS_SSE)
				SET(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -msse -mfpmath=sse")
				LIST(APPEND TRAVERSO_DEFINES -DUSE_XMMINTRIN)
			ENDIF(HOST_SUPPORTS_SSE)
		ENDIF(uname_invoke_result MATCHES i686)
				
		IF(uname_invoke_result MATCHES x86_64)
			IF(HOST_SUPPORTS_SSE)
				SET(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -msse -mfpmath=sse")
				LIST(APPEND TRAVERSO_DEFINES -DUSE_XMMINTRIN -DUSE_X86_64_ASM)
				SET(IS_ARCH_X86_64 TRUE)
			ENDIF(HOST_SUPPORTS_SSE)
		ENDIF(uname_invoke_result MATCHES x86_64)
		
		IF(uname_invoke_result MATCHES i[456]86)
			LIST(APPEND TRAVERSO_DEFINES -DARCH_X86)
			SET(IS_ARCH_X86 TRUE)
		ENDIF(uname_invoke_result MATCHES i[456]86)
	ENDIF(NOT WANT_DEBUG)


ELSE(UNIX)
	
# Windows section
	IF(WIN32)
		LINK_LIBRARIES(
		${QT_QTMAIN_LIBRARY}
		)
		LIST (APPEND TRAVERSO_DEFINES -DQT_DLL -DWIN_BUILD -DPORTAUDIO_SUPPORT)
	ENDIF(WIN32)


ENDIF(UNIX)


IF(WANT_THREAD_CHECK)
	LIST(APPEND TRAVERSO_DEFINES -DTHREAD_CHECK)
ENDIF(WANT_THREAD_CHECK)


# Check GCC for PCH support
SET(USE_PCH FALSE)

IF(WANT_PCH)
FIND_PACKAGE(PCHSupport)
IF(PCHSupport_FOUND)
	SET(USE_PCH TRUE)
	SET(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -include ${CMAKE_SOURCE_DIR}/src/precompile.h -Winvalid-pch")
	SET(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -include ${CMAKE_SOURCE_DIR}/src/precompile.h -Winvalid-pch")
	MESSAGE(STATUS "Enabling precompiled headers for GCC ${gcc_compiler_version}") 
ENDIF(PCHSupport_FOUND)
ELSE(WANT_PCH)
	#QT_DEFENITIONS are added by pch, so when no pch support add them now!
	ADD_DEFINITIONS(${QT_DEFINITIONS})
ENDIF(WANT_PCH)

ADD_DEFINITIONS(${TRAVERSO_DEFINES})
SET(TRAVERSO_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/buildfiles)


#Add our source subdirs
ADD_SUBDIRECTORY(src)


# TODO
#Install our READMEs etc.
# INSTALL(FILES
# AUTHORS
# README
# 
#   DESTINATION ${CMAKE_SOURCE_DIR}/src/
# )


# If the cmake version includes cpack, use it
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Traverso is an open source Digital Audio Workstation for Linux, Mac OSX and Windows")
  SET(CPACK_PACKAGE_VENDOR "Traverso Team")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/README")
  SET(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
  SET(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
  SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Traverso ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  SET(CPACK_PACKAGE_EXECUTABLES "traverso${TAG_VERSION}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME "traverso-${VERSION}")
  SET(CPACK_SOURCE_GENERATOR TBZ2)
## I'll make this prettier later - ignore any tarballs in the root
  SET(CPACK_SOURCE_IGNORE_FILES 
	CMakeCache.txt
	"~$"
	"Makefile$"
	"\\\\.cvsignore$" 
	"^${PROJECT_SOURCE_DIR}.*/*tar.*"
	"^${PROJECT_SOURCE_DIR}/cmake_uninstall.cmake"
	"^${PROJECT_SOURCE_DIR}.*/precompile.h.gch"
	"^${PROJECT_SOURCE_DIR}.*/cmake_install.cmake"
	"^${PROJECT_SOURCE_DIR}/CPackConfig.cmake"
	"^${PROJECT_SOURCE_DIR}/CPackSourceConfig.cmake"
	"^${PROJECT_SOURCE_DIR}/make-autopackage.sh"
	"^${PROJECT_SOURCE_DIR}/_CPack_Packages/"
	"^${PROJECT_SOURCE_DIR}.*/CMakeFiles/"
	"^${PROJECT_SOURCE_DIR}.*/CVS/"
	"^${PROJECT_SOURCE_DIR}.*/bin/"
	"^${PROJECT_SOURCE_DIR}.*/build/"
	"^${PROJECT_SOURCE_DIR}.*/buildfiles/"
	"^${PROJECT_SOURCE_DIR}.*/lib/"
	"^${PROJECT_SOURCE_DIR}/debian/"
	"^${PROJECT_SOURCE_DIR}/autopackage/"
	"^${PROJECT_SOURCE_DIR}.*/CVSROOT/"
	"^${PROJECT_SOURCE_DIR}/doc/")
  INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")


IF(USE_INTERNAL_SLV2_LIB)
	SET(SLV2OPTIONS "${HAVE_SLV2} (Using internal library)")
ENDIF(USE_INTERNAL_SLV2_LIB)
IF(USE_SYSTEM_SLV2_LIBRARY)
	SET(SLV2OPTIONS "${HAVE_SLV2} (Using system library)")
ENDIF(USE_SYSTEM_SLV2_LIBRARY)
IF(NOT HAVE_SLV2)
	SET(SLV2OPTIONS "${HAVE_SLV2}")
ENDIF(NOT HAVE_SLV2)


# Mark the following optoins as advanced
MARK_AS_ADVANCED(
QT_X11_X11_LIBRARY
QT_X11_m_LIBRARY
QT_X11_Xext_LIBRARY
LIBRARY_OUTPUT_PATH
WANT_THREAD_CHECK
)


MESSAGE("")
MESSAGE("Build options:")
MESSAGE("Building in mode	:	${CMAKE_BUILD_TYPE}")
MESSAGE("ALSA support		:	${HAVE_ALSA}")
MESSAGE("Jack support		:	${HAVE_JACK}")
MESSAGE("PortAudio support	:	${HAVE_PORTAUDIO}")
MESSAGE("SLV2 support		:	${SLV2OPTIONS}")
MESSAGE("MP3 read support	:	${HAVE_MP3_DECODING}")
MESSAGE("MP3 writing support	:	${HAVE_MP3_ENCODING}")
MESSAGE("OpenGL support		:	${HAVE_OPENGL}")
MESSAGE("")
