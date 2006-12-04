# Dit bestand is gegenereerd door KDevelop's QMake-Manager.
# ------------------------------------------- 
# De submap relatief aan de projectmap: ./src/engine
# Het Target is een bibliotheek:  traverso

include(../libbase.pri)

PRECOMPILED_HEADER = precompile.h 

INCLUDEPATH += ../../src/core ./ ./build

TARGET = traversoaudiobackend
DESTDIR = ../../lib 

TEMPLATE = lib 
LIBS += -lasound
LIBS += $$system(pkg-config --libs glib-2.0)
QMAKE_CXXFLAGS += $$system(pkg-config --cflags glib-2.0)

HEADERS += \
	precompile.h \
	AudioDevice.h \
	AudioBus.h \
	AudioDeviceThread.h \
	Client.h \
	JackDriver.h \
	AudioChannel.h \
	Driver.h \
	Tsar.h \
	memops.h \
	libtraverso.h \
	bitset.h \
	defines.h

SOURCES += AudioDevice.cpp \
	   AudioBus.cpp \
	   AudioDeviceThread.cpp \
	   Client.cpp \
	   JackDriver.cpp \
	   Driver.cpp \
	   AudioChannel.cpp \
	   Tsar.cpp \
	   memops.cpp

unix {
	SOURCES += AlsaDriver.cpp
	HEADERS += AlsaDriver.h
	contains(DEFINES, SSE_OPTIMIZATIONS):SOURCES += sse_functions.S
}


macx {
	LIBS -= -lasound
	QMAKE_LIBDIR += /usr/local/qt/lib 
}

win32 {
	LIBS -= -lasound
	SOURCES -= JackDriver.cpp
	HEADERS -= JackDriver.h
}