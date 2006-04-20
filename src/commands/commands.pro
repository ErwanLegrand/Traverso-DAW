# Dit bestand is gegenereerd door KDevelop's QMake-Manager.
# ------------------------------------------- 
# De submap relatief aan de projectmap: ./src/actions
# Het Target is een bibliotheek:  traversoactions

include(../libbase.pri)

INCLUDEPATH += ../../src/traverso \
               ../../src/core \
               ../../src/engine 

QMAKE_CXXFLAGS_RELEASE += -fPIC 
QMAKE_CXXFLAGS_DEBUG += -fPIC 

TARGET = traversocommands
DESTDIR = ../../lib 


TEMPLATE = lib 

HEADERS += ClipGain.h \
           CopyClip.h \
           Crop.h \
           Import.h \
           TrackGain.h \
           MoveClip.h \
           MoveEdge.h \
           MuteClip.h \
           Shuttle.h \
           SplitClip.h \
           Zoom.h \
	   TrackPan.h \
	   commands.h
SOURCES += CopyClip.cpp \
#           Crop.cpp \
           Import.cpp \
           TrackGain.cpp \
           MoveClip.cpp \
           MoveEdge.cpp \
           MuteClip.cpp \
           Shuttle.cpp \
           SplitClip.cpp \
	   Zoom.cpp \
	   ClipGain.cpp \
	   TrackPan.cpp
