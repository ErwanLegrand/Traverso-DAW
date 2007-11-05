include(../../libbase.pri)

TARGET = traversosongcanvas
DESTDIR = ../../../lib
TEMPLATE = lib

PRECOMPILED_HEADER = precompile.h 

INCLUDEPATH += 	../../core \
		../../common \
		../../traverso \
		../../traverso/dialogs \
		../../engine \
		../../commands \
		../../plugins \
		../../plugins/native \
		./

SOURCES += AudioClipView.cpp \
ClipsViewPort.cpp \
Cursors.cpp \
CurveNodeView.cpp \
CurveView.cpp \
FadeContextDialogView.cpp \
FadeView.cpp \
MarkerView.cpp \
PluginChainView.cpp \
PluginView.cpp \
SongView.cpp \
SongWidget.cpp \
TimeLineView.cpp \
TimeLineViewPort.cpp \
TrackPanelView.cpp \
TrackPanelViewPort.cpp \
TrackView.cpp \
ViewItem.cpp \
dialogs/AudioClipEditDialog.cpp

HEADERS += AudioClipView.h \
ClipsViewPort.h \
Cursors.h \
CurveNodeView.h \
CurveView.h \
FadeContextDialogView.h \
FadeView.h \
libtraversosongcanvas.h \
MarkerView.h \
PluginChainView.h \
PluginView.h \
precompile.h \
SongView.h \
SongWidget.h \
TimeLineView.h \
TimeLineViewPort.h \
TrackPanelView.h \
TrackPanelViewPort.h \
TrackView.h \
ViewItem.h \
LineView.h \
dialogs/AudioClipEditDialog.h

FORMS += \
ui/AudioClipEditDialog.ui \


contains(DEFINES, LV2_SUPPORT){
    INCLUDEPATH +=	\
	../../plugins/LV2 \
	../../3rdparty/slv2 
}

contains(DEFINES, QT_OPENGL_SUPPORT){
QT += opengl
}

win32{
    INCLUDEPATH += ../../../3thparty/include
}
