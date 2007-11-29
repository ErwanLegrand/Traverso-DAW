INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/audiofileio/decode
${CMAKE_SOURCE_DIR}/src/audiofileio/encode
${CMAKE_SOURCE_DIR}/src/commands
${CMAKE_SOURCE_DIR}/src/common
${CMAKE_SOURCE_DIR}/src/core
${CMAKE_SOURCE_DIR}/src/engine
${CMAKE_SOURCE_DIR}/src/plugins
${CMAKE_SOURCE_DIR}/src/plugins/LV2
${CMAKE_SOURCE_DIR}/src/plugins/native
${CMAKE_SOURCE_DIR}/src/traverso/songcanvas
${QT_QTGUI_INCLUDE_DIR}
${QT_QTXML_INCLUDE_DIR}
)

IF(USE_INTERNAL_SLV2_LIB)
	INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/src/3rdparty/slv2
	)
ENDIF(USE_INTERNAL_SLV2_LIB)


SET(TRAVERSO_GUI_SOURCES
${CMAKE_SOURCE_DIR}/src/common/fpu.cc
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


SET(JACK_LIB jack)
SET(FLAC_LIB FLAC)
SET(MAD_LIB mad)
SET(LAME_LIB mp3lame)
SET(OGG_LIB ogg)
SET(VORBIS_LIB vorbis)
SET(VORBIS_FILE_LIB vorbisfile)
SET(VORBIS_ENC_LIB vorbisenc)
SET(WAVPACK_LIB wavpack)

IF(LINUX)
find_program(RELAYTOOL_EXECUTABLE NAMES relaytool)
IF(RELAYTOOL_EXECUTABLE)
	
	execute_process(
		COMMAND relaytool --multilink libjack.so.0 libjack-0.100.0.so.0 --relay jack -ljack
		OUTPUT_VARIABLE relaytool_invoke_result
		RESULT_VARIABLE relaytool_failed
	)
	SET(JACK_LIB relaytool_invoke_result)

	execute_process(
		COMMAND relaytool --relay FLAC -lFLAC
		OUTPUT_VARIABLE relaytool_invoke_result
		RESULT_VARIABLE relaytool_failed
	)
	SET(FLAC_LIB relaytool_invoke_result)

	execute_process(
		COMMAND relaytool --relay mad -lmad
		OUTPUT_VARIABLE relaytool_invoke_result
		RESULT_VARIABLE relaytool_failed
	)
	SET(MAD_LIB relaytool_invoke_result)

	execute_process(
		COMMAND relaytool --relay mp3lame -lmp3lame
		OUTPUT_VARIABLE relaytool_invoke_result
		RESULT_VARIABLE relaytool_failed
	)
	SET(LAME_LIB relaytool_invoke_result)

	execute_process(
		COMMAND relaytool --relay ogg -logg
		OUTPUT_VARIABLE relaytool_invoke_result
		RESULT_VARIABLE relaytool_failed
	)
	SET(OGG_LIB relaytool_invoke_result)

	execute_process(
		COMMAND relaytool --relay vorbis -lvorbis
		OUTPUT_VARIABLE relaytool_invoke_result
		RESULT_VARIABLE relaytool_failed
	)
	SET(VORBIS_LIB relaytool_invoke_result)

	execute_process(
		COMMAND relaytool --relay vorbisfile -lvorbisfile
		OUTPUT_VARIABLE relaytool_invoke_result
		RESULT_VARIABLE relaytool_failed
	)
	SET(VORBIS_FILE_LIB relaytool_invoke_result)

	execute_process(
		COMMAND relaytool --relay vorbisenc -lvorbisenc
		OUTPUT_VARIABLE relaytool_invoke_result
		RESULT_VARIABLE relaytool_failed
	)
	SET(VORBIS_ENC_LIB relaytool_invoke_result)

	execute_process(
		COMMAND relaytool --relay wavpack -lwavpack
		OUTPUT_VARIABLE relaytool_invoke_result
		RESULT_VARIABLE relaytool_failed
	)
	SET(WAVPACK_LIB relaytool_invoke_result)

ENDIF(RELAYTOOL_EXECUTABLE)
ENDIF(LINUX)


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
	${WAVPACK_LIB}
	${OGG_LIB}
	${VORBIS_LIB}
	${VORBIS_FILE_LIB}
	${VORBIS_ENC_LIB}
	${FLAC_LIB}
	fftw3
)

IF(HAVE_PORTAUDIO)
	TARGET_LINK_LIBRARIES(traverso
		portaudio
	)
ENDIF(HAVE_PORTAUDIO)

IF(HAVE_SLV2)
	IF(USE_SYSTEM_SLV2_LIBRARY)
		TARGET_LINK_LIBRARIES(traverso
			slv2
		)
	ELSE(USE_SYSTEM_SLV2_LIBRARY)
		TARGET_LINK_LIBRARIES(traverso
			rdf
			rasqal
			raptor
			traversoslv2
		)
	ENDIF(USE_SYSTEM_SLV2_LIBRARY)
ENDIF(HAVE_SLV2)

IF(HAVE_MP3_DECODING)
	TARGET_LINK_LIBRARIES(traverso
		${MAD_LIB}
	)
ENDIF(HAVE_MP3_DECODING)

IF(HAVE_MP3_ENCODING)
	TARGET_LINK_LIBRARIES(traverso
		${LAME_LIB}
	)
ENDIF(HAVE_MP3_ENCODING)


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
	${JACK_LIB}
)
ENDIF(HAVE_ALSA)

IF(USE_PCH)
    ADD_DEPENDENCIES(traverso precompiled_headers)
ENDIF(USE_PCH)

SET(EXE_NAME "traverso${TAG_VERSION}")

INSTALL(TARGETS ${EXE_NAME}
  RUNTIME DESTINATION bin)
