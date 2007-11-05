ADD_SUBDIRECTORY(songcanvas ${CMAKE_SOURCE_DIR}/src/bin)

INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/audiofileio/decode
${CMAKE_SOURCE_DIR}/src/audiofileio/encode
${CMAKE_SOURCE_DIR}/src/commands
${CMAKE_SOURCE_DIR}/src/common
${CMAKE_SOURCE_DIR}/src/core
${CMAKE_SOURCE_DIR}/src/engine
${CMAKE_SOURCE_DIR}/src/plugins
${CMAKE_SOURCE_DIR}/src/plugins/LV2
${CMAKE_SOURCE_DIR}/src/3rdparty/slv2
${CMAKE_SOURCE_DIR}/src/plugins/native
${CMAKE_SOURCE_DIR}/src/traverso/songcanvas
${QT_QTGUI_INCLUDE_DIR}
${QT_QTXML_INCLUDE_DIR}
)

SET(TRAVERSO_GUI_SOURCES
Traverso.cpp
BusMonitor.cpp
ContextDialog.cpp
FadeContextDialog.cpp
Main.cpp
PluginSelectorDialog.cpp
Interface.cpp
VUMeter.cpp
CorrelationMeterWidget.cpp
SpectralMeterWidget.cpp
QuickDriverConfigWidget.cpp
dialogs/settings/Pages.cpp
dialogs/settings/SettingsDialog.cpp
dialogs/project/ProjectManagerDialog.cpp
dialogs/InsertSilenceDialog.cpp
dialogs/MarkerDialog.cpp
dialogs/BusSelectorDialog.cpp
dialogs/project/NewSongDialog.cpp
dialogs/project/NewProjectDialog.cpp
dialogs/project/OpenProjectDialog.cpp
dialogs/project/NewTrackDialog.cpp
dialogs/RestoreProjectBackupDialog.cpp
dialogs/ProjectConverterDialog.cpp
dialogs/ExportDialog.cpp
dialogs/CDWritingDialog.cpp
widgets/InfoWidgets.cpp
widgets/MessageWidget.cpp
widgets/ResourcesWidget.cpp
)

SET(TRAVERSO_UI_FILES
ui/AudioSourcesManagerWidget.ui
ui/SpectralMeterConfigWidget.ui
ui/QuickDriverConfigWidget.ui
ui/DriverConfigPage.ui
ui/AlsaDevicesPage.ui
ui/PaDriverPage.ui
ui/KeyboardConfigPage.ui
ui/BehaviorConfigPage.ui
ui/RecordingConfigPage.ui
ui/ThemeConfigPage.ui
ui/PerformanceConfigPage.ui
ui/SongManagerDialog.ui
ui/ProjectManagerDialog.ui
ui/MarkerDialog.ui
ui/BusSelectorDialog.ui
ui/OpenProjectDialog.ui
ui/NewProjectDialog.ui
ui/NewSongDialog.ui
ui/NewTrackDialog.ui
ui/ResourcesWidget.ui
ui/QuickStart.ui
ui/InsertSilenceDialog.ui
ui/RestoreProjectBackupDialog.ui
ui/ProjectConverterDialog.ui
ui/ExportDialog.ui
ui/CDWritingDialog.ui
)

SET(TRAVERSO_GUI_MOC_CLASSES
BusMonitor.h
CorrelationMeterWidget.h
FadeContextDialog.h
Interface.h
PluginSelectorDialog.h
QuickDriverConfigWidget.h
Traverso.h
VUMeter.h
SpectralMeterWidget.h
dialogs/BusSelectorDialog.h
dialogs/CDWritingDialog.h
dialogs/ExportDialog.h
dialogs/InsertSilenceDialog.h
dialogs/MarkerDialog.h
dialogs/ProjectConverterDialog.h
dialogs/RestoreProjectBackupDialog.h
dialogs/project/NewSongDialog.h
dialogs/project/NewTrackDialog.h
dialogs/project/NewProjectDialog.h
dialogs/project/OpenProjectDialog.h
dialogs/project/ProjectManagerDialog.h
dialogs/settings/SettingsDialog.h
dialogs/settings/Pages.h
widgets/InfoWidgets.h
widgets/ResourcesWidget.h
widgets/MessageWidget.h
)

QT4_ADD_RESOURCES(TRAVERSO_RESOURCES
traverso.qrc
)

QT4_WRAP_CPP(TRAVERSO_GUI_MOC_SOURCES ${TRAVERSO_GUI_MOC_CLASSES})

QT4_WRAP_UI(TRAVERSO_GUI_UI_SOURCES ${TRAVERSO_UI_FILES})

ADD_EXECUTABLE(traverso
    ${TRAVERSO_GUI_SOURCES}
    ${TRAVERSO_GUI_UI_SOURCES}
    ${TRAVERSO_GUI_MOC_SOURCES}
    ${TRAVERSO_RESOURCES}
)

TARGET_LINK_LIBRARIES(traverso
	${QT_LIBRARIES}
	${QT_QTXML_LIBRARY}
	traversosongcanvas
	traversocore
	traversoaudiofileio
	traversoaudiobackend
	traversoplugins
	tcp_traversocommands
	traversocommands
	samplerate
	sndfile
	wavpack
	ogg
	vorbis
	vorbisfile
	vorbisenc
	mad
	mp3lame
	FLAC
	traversoslv2
	rdf
	rasqal
	raptor
	fftw3
)

IF(HAVE_OPENGL)
TARGET_LINK_LIBRARIES(traverso
${QT_QTOPENGL_LIBRARY}
)
ENDIF(HAVE_OPENGL)

IF(HAVE_ALSA)
TARGET_LINK_LIBRARIES(traverso
	asound
)
ENDIF(HAVE_ALSA)

IF(HAVE_ALSA)
TARGET_LINK_LIBRARIES(traverso
	jack
)
ENDIF(HAVE_ALSA)

IF(USE_PCH)
    ADD_DEPENDENCIES(traversosongcanvas precompiled_headers)
    ADD_DEPENDENCIES(traversocore precompiled_headers)
    ADD_DEPENDENCIES(traversocommands precompiled_headers)
    ADD_DEPENDENCIES(traversoaudiobackend precompiled_headers)
    ADD_DEPENDENCIES(traversoplugins precompiled_headers)
    ADD_DEPENDENCIES(traversoaudiofileio precompiled_headers)
    ADD_DEPENDENCIES(tcp_traversocommands precompiled_headers)
ENDIF(USE_PCH)

