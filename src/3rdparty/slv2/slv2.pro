include(../../libbase.pri)

contains(DEFINES, LV2_SUPPORT) {

CONFIG -= dll
CONFIG += static
TEMPLATE = lib 
INCLUDEPATH += slv2
QMAKE_CFLAGS_DEBUG += -std=c99
QMAKE_CFLAGS_RELEASE += -std=c99
QMAKE_LIBDIR = ../../../lib 
TARGET = slv2 
DESTDIR = ../../../lib 

SOURCES += \
	src/plugin.c \
	src/plugininstance.c \
	src/pluginlist.c \
	src/port.c \
	src/query.c \
	src/stringlist.c \
	src/util.c \
	src/world.c
}

