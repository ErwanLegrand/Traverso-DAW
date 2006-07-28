# Dit bestand is gegenereerd door KDevelop's QMake-Manager.
# ------------------------------------------- 
# De submap relatief aan de projectmap: ./src/traverso
# Het Target is een toepassing:  traverso

include(../appbase.pri)

RESOURCES += traverso.qrc
PRECOMPILED_HEADER = precompile.h

LIBS += 	-ltraversocore \
		-ltraversocommands \
		-ltraverso \
		-lsndfile \
		-lsamplerate

HEADERS += AudioPluginSelector.h \
#	   CursorWidget.h \
           BorderLayout.h \
           BusMonitor.h \
           BusSelector.h \
           ColorManager.h \
           Help.h \
           InfoBox.h \
           Main.h \
           ExportWidget.h \
           ManagerWidget.h \
           MessageWidget.h \
           OverViewWidget.h \
           ProjectManagerWidget.h \
           ProjectInfoWidget.h \
           SongManagerWidget.h \
           SystemInfoWidget.h \
           SongInfoWidget.h \
           AudioSourcesManagerWidget.h \
           GlobalPropertiesWidget.h \
           Traverso.h \
           TrackView.h \
           ViewItem.h \
           AudioClipView.h \
           Interface.h \
           ViewPort.h \
           SongView.h \
	   VUMeter.h \
	   VUMeterLevel.h \
           Cursor.h \
           PanelLed.h \
           LocatorView.h  \
           precompile.h



SOURCES += Traverso.cpp \
#	   CursorWidget.cpp \
           AudioPluginSelector.cpp \
           BorderLayout.cpp \
           BusMonitor.cpp \
           BusSelector.cpp \
           ColorManager.cpp \
           Help.cpp \
           InfoBox.cpp \
           Main.cpp \
           ExportWidget.cpp \
           ProjectManagerWidget.cpp \
           ProjectInfoWidget.cpp \
           MessageWidget.cpp \
           ManagerWidget.cpp \
           OverViewWidget.cpp \
           SongManagerWidget.cpp \
           SongInfoWidget.cpp \
           SystemInfoWidget.cpp \
           AudioSourcesManagerWidget.cpp \
           GlobalPropertiesWidget.cpp \
           TrackView.cpp \
           ViewItem.cpp \
           AudioClipView.cpp \
           Interface.cpp \
           ViewPort.cpp \
           SongView.cpp \
	   VUMeter.cpp \
	   VUMeterLevel.cpp \
           Cursor.cpp \
           PanelLed.cpp \
           LocatorView.cpp

FORMS += 	ui/ProjectManagerWidget.ui \
			ui/ProjectInfoWidget.ui \
			ui/SongInfoWidget.ui \
			ui/ExportWidget.ui \
			ui/SongManagerWidget.ui \
			ui/SystemInfoWidget.ui \
			ui/ManagerWidget.ui \
			ui/GlobalPropertiesWidget.ui \
			ui/AudioSourcesManagerWidget.ui 


contains(DEFINES, ALSA_SUPPORT):LIBS += -lasound
contains(DEFINES, JACK_SUPPORT):LIBS += -ljack

INCLUDEPATH += 	../../src/core \
		../../src/commands \
		../../src/engine
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
	QMAKE_LIBDIR += /usr/local/qt/lib
	DEFINES -= HAVE_SYS_VFS_H
}
