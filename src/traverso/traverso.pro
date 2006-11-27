# Dit bestand is gegenereerd door KDevelop's QMake-Manager.
# ------------------------------------------- 
# De submap relatief aan de projectmap: ./src/traverso
# Het Target is een toepassing:  traverso

include(../appbase.pri)

RESOURCES += traverso.qrc

LIBS += 	-ltraversocore \
		-ltraversocommands \
		-ltraversoaudiobackend \
		-ltraversoplugins \
		-lsndfile \
		-lsamplerate \
		-lslv2 \

include(songcanvas/songcanvas.pri)

HEADERS += \
	AudioPluginSelector.h \
	AudioSourcesTreeWidget.h \
#	CursorWidget.h \
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
#	ProjectInfoWidget.h \
	SongManagerWidget.h \
	SystemInfoWidget.h \
#	SongInfoWidget.h \
	AudioSourcesManagerWidget.h \
	GlobalPropertiesWidget.h \
	Traverso.h \
	Interface.h \
	VUMeter.h \
	VUMeterRuler.h \
	VUMeterLevel.h \
	VUMeterOverLed.h \
	MultiMeterWidget.h \
	QuickDriverConfigWidget.h \
	precompile.h 



SOURCES += \
	Traverso.cpp \
	AudioSourcesTreeWidget.cpp \
#	CursorWidget.cpp \
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
#	ProjectInfoWidget.cpp \
	MessageWidget.cpp \
	ManagerWidget.cpp \
	OverViewWidget.cpp \
	PluginSelectorDialog.cpp \
	SongManagerWidget.cpp \
#	SongInfoWidget.cpp \
	SystemInfoWidget.cpp \
	AudioSourcesManagerWidget.cpp \
	GlobalPropertiesWidget.cpp \
	Interface.cpp \
	VUMeter.cpp \
	VUMeterRuler.cpp \
	VUMeterLevel.cpp \
	VUMeterOverLed.cpp \
	MultiMeterWidget.cpp \
	QuickDriverConfigWidget.cpp

FORMS += ui/ProjectManagerWidget.ui \
#	ui/ProjectInfoWidget.ui \
#	ui/SongInfoWidget.ui \
	ui/ExportWidget.ui \
	ui/SongManagerWidget.ui \
	ui/ManagerWidget.ui \
	ui/GlobalPropertiesWidget.ui \
	ui/AudioSourcesManagerWidget.ui \
	ui/PluginSelectorDialog.ui \
	ui/QuickDriverConfigWidget.ui


contains(DEFINES, ALSA_SUPPORT):LIBS += -lasound
contains(DEFINES, JACK_SUPPORT):LIBS += -ljack

INCLUDEPATH += 	../../src/core \
		../../src/commands \
		../../src/engine \
		../../src/plugins \
		../../src/plugins/LV2 \
		../../src/plugins/native \
		../../src/traverso/songcanvas \
		../../src/traverso \
		
QT += opengl
QMAKE_LIBDIR = ./lib ../../lib
TARGET = traverso

target.path = /usr/local/bin
INSTALLS += target

DESTDIR = ../..
TEMPLATE = app
DESTDIR_TARGET = /usr/local/bin

unix {
	# if exists('sys/vfs.h')
	DEFINES += HAVE_SYS_VFS_H
}

macx {
	LIBS -= -lasound
}

win32 {
	LIBS -= -lslv2 -lsamplerate

	INCLUDEPATH -= ../../src/plugins/LV2

}
