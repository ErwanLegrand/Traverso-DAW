include(../../libbase.pri)

TARGET = traversosongcanvas
DESTDIR = ../../../lib
TEMPLATE = lib

PRECOMPILED_HEADER = precompile.h 

INCLUDEPATH += 	../../core \
		../../traverso \
		../../engine \
		../../commands \
		../../plugins \
		../../plugins/LV2 \
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
ViewItem.cpp
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
ViewItem.h
FORMS += ../ui/ExportWidget.ui \
	../ui/AudioSourcesManagerWidget.ui \
	../ui/PluginSelectorDialog.ui \
	../ui/SpectralMeterConfigWidget.ui \
	../ui/QuickDriverConfigWidget.ui \
	../ui/DriverConfigPage.ui \
	../ui/AlsaDevicesPage.ui \
	../ui/KeyboardConfigPage.ui \
	../ui/BehaviorConfigPage.ui \
	../ui/MemoryConfigPage.ui \
	../ui/ThemeConfigPage.ui \
	../ui/SongManagerDialog.ui \
	../ui/ProjectManagerDialog.ui


!contains(DEFINES, LV2_SUPPORT){
    INCLUDEPATH -= ../plugins/LV2
}

QT += opengl
