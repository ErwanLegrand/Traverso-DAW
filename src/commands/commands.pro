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
Scroll.cpp \
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
Scroll.h \
AudioClipExternalProcessing.h \
ArmTracks.h \
PlayHeadMove.h \
WorkCursorMove.h \
RemoveClip.h

FORMS += ui/ExternalProcessing.ui

TARGET = traversocommands
DESTDIR = ../../lib
TEMPLATE = lib

PRECOMPILED_HEADER = precompile.h 
include(../libbase.pri)

INCLUDEPATH += ../core \
../traverso \
../traverso/songcanvas \
../common \
../plugins \
../plugins/native \
../audiofileio/decode \

win32{
    INCLUDEPATH += ../../3thparty/include
}
