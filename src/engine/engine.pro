# Dit bestand is gegenereerd door KDevelop's QMake-Manager.
# ------------------------------------------- 
# De submap relatief aan de projectmap: ./src/engine
# Het Target is een bibliotheek:  traverso

include(../libbase.pri)

#PRECOMPILED_HEADER = libtraverso.h 

INCLUDEPATH += ../../src/core 

TARGET = traverso 
DESTDIR = ../../lib 

TEMPLATE = lib 
LIBS += -lasound

HEADERS += AudioDevice.h \
	   AudioBus.h \
	   AudioDeviceThread.h \
	   Client.h \
	   AlsaDriver.h \
	   JackDriver.h \
	   AudioChannel.h \
	   Driver.h \
	   memops.h \
           libtraverso.h \
           AudioPlugin.h \
           bitset.h \
           defines.h

SOURCES += AudioDevice.cpp \
	   AudioBus.cpp \
	   AudioDeviceThread.cpp \
	   Client.cpp \
	   AlsaDriver.cpp \
	   JackDriver.cpp \
	   Driver.cpp \
	   AudioChannel.cpp \
	   memops.cpp \
           AudioPlugin.cpp

unix {
	contains(DEFINES, SSE_OPTIMIZATIONS):SOURCES += sse_functions.s
}


macx {
	SOURCES -= AlsaDriver.cpp
}
