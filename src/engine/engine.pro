include(../libbase.pri)

INCLUDEPATH += ../core

TARGET = traversoaudiobackend
DESTDIR = ../../lib 
TEMPLATE = lib 
LIBS += -lasound


SOURCES += AlsaDriver.cpp \
AudioBus.cpp \
AudioChannel.cpp \
AudioDevice.cpp \
AudioDeviceThread.cpp \
AudioPlugin.cpp \
Client.cpp \
Driver.cpp \
JackDriver.cpp \
memops.cpp \
Tsar.cpp
HEADERS += AlsaDriver.h \
AudioBus.h \
AudioChannel.h \
AudioDevice.h \
AudioDeviceThread.h \
AudioPlugin.h \
bitset.h \
Client.h \
defines.h \
Driver.h \
JackDriver.h \
libtraverso.h \
memops.h \
precompile.h \
Tsar.h


release{
PRECOMPILED_HEADER -= precompile.h 
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
}
