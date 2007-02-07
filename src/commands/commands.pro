SOURCES += AddRemoveItemCommand.cpp \
ClipSelection.cpp \
CommandGroup.cpp \
CopyClip.cpp \
Fade.cpp \
Gain.cpp \
Import.cpp \
MoveClip.cpp \
MoveEdge.cpp \
PCommand.cpp \
SplitClip.cpp \
TrackPan.cpp \
Zoom.cpp
HEADERS += AddRemoveItemCommand.h \
ClipSelection.h \
CommandGroup.h \
commands.h \
CopyClip.h \
Fade.h \
Gain.h \
Import.h \
MoveClip.h \
MoveEdge.h \
PCommand.h \
precompile.h \
SplitClip.h \
TrackPan.h \
Zoom.h
TARGET = traversocommands
DESTDIR = ../../lib
TEMPLATE = lib

PRECOMPILED_HEADER = precompile.h 
include(../libbase.pri)

INCLUDEPATH += ../core \
../traverso \
../traverso/songcanvas \
../engine \
