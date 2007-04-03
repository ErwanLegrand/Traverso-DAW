# Dit bestand is gegenereerd door KDevelop's QMake-Manager.
# ------------------------------------------- 
# De submap relatief aan de projectmap: ./src/traverso
# Het Target is een toepassing:  traverso

include(../appbase.pri)

RESOURCES += traverso.qrc
PRECOMPILED_HEADER = precompile.h

contains(DEFINES, STATIC_BUILD){
    QMAKE_LIBDIR += ../../lib/commandplugins
    LIBS += -L../../lib/commandplugins -ltcp_traversocommands
}

LIBS +=  \
	-ltraversosongcanvas \
	-ltraversocore \
	-ltraversocommands \
	-ltraversoaudiobackend \
	-ltraversoplugins \
	-lsndfile \
	-lsamplerate \
	-lslv2 \
	-lfftw3 \

HEADERS += \
	AudioSourcesTreeWidget.h \
	BusMonitor.h \
	BusSelector.h \
	Themer.h \
	ContextDialog.h \
	FadeContextDialog.h \
	Help.h \
	Main.h \
	ExportWidget.h \
	PluginSelectorDialog.h \
	AudioSourcesManagerWidget.h \
	Traverso.h \
	Interface.h \
	VUMeter.h \
	CorrelationMeterWidget.h \
	SpectralMeterWidget.h \
	QuickDriverConfigWidget.h \
	precompile.h \
	dialogs/settings/Pages.h \
	dialogs/settings/SettingsDialog.h \
	dialogs/project/ProjectManagerDialog.h \
	songcanvas/MarkerView.h \
	widgets/InfoWidgets.h \
	widgets/MessageWidget.h \
	dialogs/CDTextDialog.h \
	dialogs/MarkerDialog.h \
	dialogs/BusSelectorDialog.h \
	dialogs/project/NewSongDialog.h \
	dialogs/project/OpenCreateProjectDialog.h
SOURCES += \
	Traverso.cpp \
	AudioSourcesTreeWidget.cpp \
	BusMonitor.cpp \
	BusSelector.cpp \
	Themer.cpp \
	ContextDialog.cpp \
	FadeContextDialog.cpp \
	Help.cpp \
	Main.cpp \
	ExportWidget.cpp \
	PluginSelectorDialog.cpp \
	AudioSourcesManagerWidget.cpp \
	Interface.cpp \
	VUMeter.cpp \
	CorrelationMeterWidget.cpp \
	SpectralMeterWidget.cpp \
	QuickDriverConfigWidget.cpp \
	dialogs/settings/Pages.cpp \
	dialogs/settings/SettingsDialog.cpp \
	dialogs/project/ProjectManagerDialog.cpp \
	songcanvas/MarkerView.cpp \
	widgets/InfoWidgets.cpp \
	widgets/MessageWidget.cpp \
	dialogs/CDTextDialog.cpp \
	dialogs/MarkerDialog.cpp \
	dialogs/BusSelectorDialog.cpp \
	dialogs/project/NewSongDialog.cpp \
	dialogs/project/OpenCreateProjectDialog.cpp
FORMS += ui/ExportWidget.ui \
	ui/AudioSourcesManagerWidget.ui \
	ui/PluginSelectorDialog.ui \
	ui/SpectralMeterConfigWidget.ui \
	ui/QuickDriverConfigWidget.ui \
	ui/DriverConfigPage.ui \
	ui/AlsaDevicesPage.ui \
	ui/PaDriverPage.ui \
	ui/KeyboardConfigPage.ui \
	ui/BehaviorConfigPage.ui \
	ui/MemoryConfigPage.ui \
	ui/ThemeConfigPage.ui \
	ui/SongManagerDialog.ui \
	ui/ProjectManagerDialog.ui \
	ui/MarkerDialog.ui \
	ui/CDTextDialog.ui \
	ui/BusSelectorDialog.ui \
	ui/OpenCreateProjectDialog.ui \


INCLUDEPATH += 	../core \
		../commands \
		../engine \
		../plugins \
		../plugins/LV2 \
		../plugins/native \
		songcanvas \


contains(DEFINES, ALSA_SUPPORT): LIBS += -lasound
!contains(DEFINES, ALSA_SUPPORT): FORMS -= ui/AlsaDevicesPage.ui

contains(DEFINES, PORTAUDIO_SUPPORT): LIBS += -lportaudio
!contains(DEFINES, PORTAUDIO_SUPPORT): FORMS -= ui/PaDriverPage.ui


contains(DEFINES, JACK_SUPPORT): LIBS += -ljack

!contains(DEFINES, LV2_SUPPORT){
    LIBS -= -lslv2
    INCLUDEPATH -= ../plugins/LV2
}

QT += opengl

QMAKE_LIBDIR = ../../lib 

TARGET = traverso

target.path = /usr/local/bin
INSTALLS += target

DESTDIR = ../..
TEMPLATE = app
DESTDIR_TARGET = /usr/local/bin

unix{
    # if exists('sys/vfs.h')
    DEFINES += HAVE_SYS_VFS_H

    # perhaps this doesn't cover mac os x ?
    # if so, copy paste into  macx section...
    LIBS += $$system(pkg-config --libs glib-2.0)
}


win32{
    LIBS -= -lslv2 -lfftw3
    # -lwinmm is needed for wmme support!!
    LIBS += -lfftw3-3 -lwinmm
    INCLUDEPATH -= ../../src/plugins/LV2
    INCLUDEPATH += ../../3thparty/include .

    QMAKE_LIBDIR = ../../lib ../../3thparty/lib
}
