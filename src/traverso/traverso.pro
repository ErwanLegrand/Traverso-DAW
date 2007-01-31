# Dit bestand is gegenereerd door KDevelop's QMake-Manager.
# ------------------------------------------- 
# De submap relatief aan de projectmap: ./src/traverso
# Het Target is een toepassing:  traverso

include(../appbase.pri)

RESOURCES += traverso.qrc
PRECOMPILED_HEADER = precompile.h

LIBS +=  \
	-ltraversocore \
	-ltraversocommands \
	-ltraversoaudiobackend \
	-ltraversoplugins \
	-lsndfile \
	-lsamplerate \
	-lslv2 \
	-lfftw3 \
	$$system(pkg-config --libs glib-2.0) \

HEADERS += \
	songcanvas/AudioClipView.h \
	songcanvas/SongWidget.h \
	songcanvas/ClipsViewPort.h \
	songcanvas/CurveView.h \
	songcanvas/CurveNodeView.h \
	songcanvas/TimeLineViewPort.h \
	songcanvas/TrackPanelViewPort.h \
	songcanvas/TrackView.h \
	songcanvas/ViewItem.h \
	songcanvas/SongView.h \
	songcanvas/TimeLineview.h \
	songcanvas/TrackPanelView.h \
	songcanvas/Cursors.h \
	songcanvas/FadeView.h \
	songcanvas/FadeContextDialogView.h \
	songcanvas/PluginView.h \
	songcanvas/PluginChainView.h \
	AudioPluginSelector.h \
	AudioSourcesTreeWidget.h \
	BusMonitor.h \
	BusSelector.h \
	ColorManager.h \
	ContextDialog.h \
	FadeContextDialog.h \
	Help.h \
	Main.h \
	ExportWidget.h \
	ManagerWidget.h \
	MessageWidget.h \
	OverViewWidget.h \
	PluginSelectorDialog.h \
	ProjectManagerWidget.h \
	SongManagerWidget.h \
	SystemInfoWidget.h \
	AudioSourcesManagerWidget.h \
	GlobalPropertiesWidget.h \
	Traverso.h \
	Interface.h \
	VUMeter.h \
	CorrelationMeterWidget.h \
	SpectralMeterWidget.h \
	QuickDriverConfigWidget.h \
	precompile.h 



SOURCES += \
	songcanvas/AudioClipView.cpp \
	songcanvas/SongWidget.cpp \
	songcanvas/ClipsViewPort.cpp \
	songcanvas/CurveView.cpp \
	songcanvas/CurveNodeView.cpp \
	songcanvas/TimeLineViewPort.cpp \
	songcanvas/TrackPanelViewPort.cpp \
	songcanvas/TrackView.cpp \
	songcanvas/ViewItem.cpp \
	songcanvas/SongView.cpp \
	songcanvas/TimeLineView.cpp \
	songcanvas/TrackPanelView.cpp \
	songcanvas/Cursors.cpp \
	songcanvas/FadeView.cpp \
	songcanvas/FadeContextDialogView.cpp \
	songcanvas/PluginView.cpp \
	songcanvas/PluginChainView.cpp \
	Traverso.cpp \
	AudioSourcesTreeWidget.cpp \
	AudioPluginSelector.cpp \
	BusMonitor.cpp \
	BusSelector.cpp \
	ColorManager.cpp \
	ContextDialog.cpp \
	FadeContextDialog.cpp \
	Help.cpp \
	Main.cpp \
	ExportWidget.cpp \
	ProjectManagerWidget.cpp \
	MessageWidget.cpp \
	ManagerWidget.cpp \
	OverViewWidget.cpp \
	PluginSelectorDialog.cpp \
	SongManagerWidget.cpp \
	SystemInfoWidget.cpp \
	AudioSourcesManagerWidget.cpp \
	GlobalPropertiesWidget.cpp \
	Interface.cpp \
	VUMeter.cpp \
	CorrelationMeterWidget.cpp \
	SpectralMeterWidget.cpp \
	QuickDriverConfigWidget.cpp

FORMS += ui/ProjectManagerWidget.ui \
	ui/ExportWidget.ui \
	ui/SongManagerWidget.ui \
	ui/ManagerWidget.ui \
	ui/GlobalPropertiesWidget.ui \
	ui/AudioSourcesManagerWidget.ui \
	ui/PluginSelectorDialog.ui \
	ui/SpectralMeterConfigWidget.ui \
	ui/QuickDriverConfigWidget.ui


INCLUDEPATH += 	../core \
		../commands \
		../engine \
		../plugins \
		../plugins/LV2 \
		../plugins/native \
		songcanvas \


contains(DEFINES, ALSA_SUPPORT): LIBS += -lasound
contains(DEFINES, JACK_SUPPORT): LIBS += -ljack

!contains(DEFINES, LV2_SUPPORT) {
LIBS -= -lslv2
INCLUDEPATH -= ../plugins/LV2
}

		
QT += opengl
QMAKE_LIBDIR = ./lib ../../lib
TARGET = traverso

target.path = /usr/local/bin
INSTALLS += target

DESTDIR = ../..
TEMPLATE = app
DESTDIR_TARGET = /usr/local/bin

unix{
    # if exists('sys/vfs.h')
    DEFINES += HAVE_SYS_VFS_H
}

macx{
    LIBS -= -lasound
}

win32{
    LIBS -= -lslv2 -lsamplerate
    INCLUDEPATH -= ../../src/plugins/LV2
}
