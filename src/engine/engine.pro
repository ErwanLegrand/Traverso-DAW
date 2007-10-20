include(../libbase.pri)

INCLUDEPATH += ../common

TARGET = traversoaudiobackend
DESTDIR = ../../lib 
TEMPLATE = lib 
LIBS += -lasound -lportaudio


SOURCES += AlsaDriver.cpp \
AudioBus.cpp \
AudioChannel.cpp \
AudioDevice.cpp \
AudioDeviceThread.cpp \
Client.cpp \
Driver.cpp \
JackDriver.cpp \
memops.cpp \
PADriver.cpp
HEADERS += AlsaDriver.h \
AudioBus.h \
AudioChannel.h \
AudioDevice.h \
AudioDeviceThread.h \
bitset.h \
Client.h \
defines.h \
Driver.h \
JackDriver.h \
libtraverso.h \
memops.h \
precompile.h \
PADriver.h
release{
    PRECOMPILED_HEADER -= precompile.h 
}

!contains(DEFINES, PORTAUDIO_SUPPORT){
    SOURCES -= PADriver.cpp
    HEADERS -= PADriver.h
    LIBS -= -lportaudio
}

!contains(DEFINES, JACK_SUPPORT){
    SOURCES -= JackDriver.cpp
    HEADERS -= JackDriver.h
}

!contains(DEFINES, ALSA_SUPPORT){
    SOURCES -= AlsaDriver.cpp
    HEADERS -= AlsaDriver.h
}

unix{
    contains(DEFINES, SSE_OPTIMIZATIONS): SOURCES += sse_functions.S
}

macx{
    SOURCES -= AlsaDriver.cpp
    HEADERS -= AlsaDriver.h
    LIBS -= -lasound
    QMAKE_LIBDIR += /usr/local/qt/lib 
}

win32{
    LIBS -= -lasound
    SOURCES -= JackDriver.cpp
    HEADERS -= JackDriver.h
    SOURCES -= AlsaDriver.cpp
    HEADERS -= AlsaDriver.h

    INCLUDEPATH += ../../3thparty/include .
}
