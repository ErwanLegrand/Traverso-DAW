SOURCES += AddRemove.cpp \
ClipSelection.cpp \
CommandGroup.cpp \
Fade.cpp \
Gain.cpp \
Import.cpp \
MoveClip.cpp \
MoveEdge.cpp \
PCommand.cpp \
SplitClip.cpp \
TrackPan.cpp \
Zoom.cpp \
AudioClipExternalProcessing.cpp \
ArmTracks.cpp \
PlayHeadMove.cpp \
WorkCursorMove.cpp \
RemoveClip.cpp
HEADERS += AddRemove.h \
ClipSelection.h \
CommandGroup.h \
commands.h \
Fade.h \
Gain.h \
Import.h \
MoveClip.h \
MoveEdge.h \
PCommand.h \
precompile.h \
SplitClip.h \
TrackPan.h \
Zoom.h \
AudioClipExternalProcessing.h \
ArmTracks.h \
PlayHeadMove.h \
WorkCursorMove.h \
RemoveClip.h
TARGET = traversocommands
DESTDIR = ../../lib
TEMPLATE = lib

PRECOMPILED_HEADER = precompile.h 
include(../libbase.pri)

INCLUDEPATH += ../core \
../traverso \
../traverso/songcanvas \
../engine \

win32{
    INCLUDEPATH += ../../3thparty/include
}
