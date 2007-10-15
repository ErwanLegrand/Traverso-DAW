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
	-ltraversoaudiofileio \
	-ltraversocommands \
	-ltraversoaudiobackend \
	-ltraversoplugins \
	-lsndfile \
	-lsamplerate \
	-lfftw3

HEADERS += \
	BusMonitor.h \
	ContextDialog.h \
	FadeContextDialog.h \
	Main.h \
	ExportWidget.h \
	PluginSelectorDialog.h \
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
	dialogs/InsertSilenceDialog.h \
	dialogs/MarkerDialog.h \
	dialogs/BusSelectorDialog.h \
	dialogs/project/NewSongDialog.h \
	dialogs/project/NewProjectDialog.h \
	dialogs/project/OpenProjectDialog.h \
	dialogs/project/NewTrackDialog.h \
	songcanvas/PositionIndicator.h \
	widgets/ResourcesWidget.h \
	dialogs/AudioClipEditDialog.h \
	dialogs/RestoreProjectBackupDialog.h \
	dialogs/ProjectConverterDialog.h  \
	dialogs/ExportDialog.h
SOURCES += \
	Traverso.cpp \
	BusMonitor.cpp \
	ContextDialog.cpp \
	FadeContextDialog.cpp \
	Main.cpp \
	ExportWidget.cpp \
	PluginSelectorDialog.cpp \
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
	dialogs/InsertSilenceDialog.cpp \
	dialogs/MarkerDialog.cpp \
	dialogs/BusSelectorDialog.cpp \
	dialogs/project/NewSongDialog.cpp \
	dialogs/project/NewProjectDialog.cpp \
	dialogs/project/OpenProjectDialog.cpp \
	dialogs/project/NewTrackDialog.cpp \
	songcanvas/PositionIndicator.cpp \
	widgets/ResourcesWidget.cpp \
	dialogs/AudioClipEditDialog.cpp \
	dialogs/RestoreProjectBackupDialog.cpp \
	dialogs/ProjectConverterDialog.cpp \
	dialogs/ExportDialog.cpp
FORMS += ui/ExportWidget.ui \
	ui/AudioSourcesManagerWidget.ui \
	ui/SpectralMeterConfigWidget.ui \
	ui/QuickDriverConfigWidget.ui \
	ui/DriverConfigPage.ui \
	ui/AlsaDevicesPage.ui \
	ui/PaDriverPage.ui \
	ui/KeyboardConfigPage.ui \
	ui/BehaviorConfigPage.ui \
	ui/RecordingConfigPage.ui \
	ui/ThemeConfigPage.ui \
	ui/PerformanceConfigPage.ui \
	ui/SongManagerDialog.ui \
	ui/ProjectManagerDialog.ui \
	ui/MarkerDialog.ui \
	ui/BusSelectorDialog.ui \
	ui/OpenProjectDialog.ui \
	ui/NewProjectDialog.ui \
	ui/NewSongDialog.ui \
	ui/NewTrackDialog.ui \
	ui/ResourcesWidget.ui \
	ui/QuickStart.ui \
	ui/InsertSilenceDialog.ui \
	ui/AudioClipEditWidget.ui \
	ui/RestoreProjectBackupDialog.ui \
	ui/ProjectConverterDialog.ui \
	ui/ExportDialog.ui

INCLUDEPATH += 	../core \
		../commands \
		../engine \
		../audiofileio/decode \
		../audiofileio/encode \
		../plugins \
		../plugins/LV2 \
		../plugins/native \
		songcanvas \


contains(DEFINES, ALSA_SUPPORT): LIBS += -lasound
!contains(DEFINES, ALSA_SUPPORT): FORMS -= ui/AlsaDevicesPage.ui

contains(DEFINES, PORTAUDIO_SUPPORT): LIBS += -lportaudio
!contains(DEFINES, PORTAUDIO_SUPPORT): FORMS -= ui/PaDriverPage.ui

contains(DEFINES, LV2_SUPPORT){
    LIBS += -lslv2 -lrdf -lrasqal -lraptor
    INCLUDEPATH +=	../3rdparty/slv2 ../plugins/LV2
}

contains(DEFINES, QT_OPENGL_SUPPORT){
    QT += opengl
}

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

    contains(DEFINES, JACK_SUPPORT){
        system(which relaytool 2>/dev/null >/dev/null){
            LIBS += $$system(relaytool --multilink libjack.so.0 libjack-0.100.0.so.0 --relay jack -ljack)
        }        else{
            LIBS += -ljack
        }
    }

    system(which relaytool 2>/dev/null >/dev/null){
        LIBS += $$system(relaytool --relay FLAC -lFLAC)
        LIBS += $$system(relaytool --relay mad -lmad)
        LIBS += $$system(relaytool --relay mp3lame -lmp3lame)
        LIBS += $$system(relaytool --relay ogg -logg)
        LIBS += $$system(relaytool --relay vorbis -lvorbis)
        LIBS += $$system(relaytool --relay vorbisfile -lvorbisfile)
        LIBS += $$system(relaytool --relay vorbisenc -lvorbisenc)
        LIBS += $$system(relaytool --relay wavpack -lwavpack)
    }    else{
        LIBS += -logg \
            -lvorbis \
            -lvorbisfile \
            -lvorbisenc \
            -lmad \
            -lmp3lame \
            -lFLAC \
            -lwavpack
    }
}

!unix{    #non-unix systems don't have relaytool
    LIBS += -logg \
        -lvorbis \
        -lvorbisfile \
        -lvorbisenc \
        -lmad \
        -lmp3lame \
        -lFLAC \
        -lwavpack
}

macx{
    contains(DEFINES, JACK_SUPPORT){
        LIBS += -ljack
    }

    contains(DEFINES, LV2_SUPPORT){
        LIBS += -lraptor
    }
}

win32{
    LIBS -= -lslv2 -lfftw3
    # -lwinmm is needed for wmme support!!
    LIBS += -lfftw3-3 -lwinmm
    INCLUDEPATH -= ../../src/plugins/LV2
    INCLUDEPATH += ../../3thparty/include .

    QMAKE_LIBDIR = ../../lib ../../3thparty/lib
    RC_FILE = traverso.rc
}

contains(DEFINES, STATIC_BUILD){
    contains(DEFINES, LV2_SUPPORT){
        POST_TARGETDEPS += ../3rdparty/slv2
    }

    POST_TARGETDEPS += ../engine ../commands ../core ../audiofileio ../plugins songcanvas
}
