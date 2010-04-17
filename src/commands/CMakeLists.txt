ADD_SUBDIRECTORY(plugins)

INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/commands
${CMAKE_SOURCE_DIR}/src/common
${CMAKE_SOURCE_DIR}/src/core
${CMAKE_SOURCE_DIR}/src/traverso
${CMAKE_SOURCE_DIR}/src/sheetcanvas
${CMAKE_SOURCE_DIR}/src/plugins
${CMAKE_SOURCE_DIR}/src/plugins/native
${QT_QTGUI_INCLUDE_DIR}
${QT_QTXML_INCLUDE_DIR}
)

SET(TRAVERSO_COMMANDS_SOURCES
AudioClipExternalProcessing.cpp
AddRemove.cpp
ClipSelection.cpp
CommandGroup.cpp
Crop.cpp
ExternalProcessingDialog.cpp
Fade.cpp
Gain.cpp
Import.cpp
MoveClip.cpp
MoveCurveNode.cpp
MoveEdge.cpp
MoveMarker.cpp
MoveTrack.cpp
PCommand.cpp
SplitClip.cpp
TrackPan.cpp
Zoom.cpp
Scroll.cpp
Shuttle.cpp
ArmTracks.cpp
PlayHeadMove.cpp
WorkCursorMove.cpp
RemoveClip.cpp
MoveCommand.cpp
)

SET(TRAVERSO_COMMANDS_MOC_CLASSES
Crop.h
ExternalProcessingDialog.h
Fade.h
Gain.h
MoveClip.h
MoveCurveNode.h
MoveEdge.h
MoveMarker.h
Shuttle.h
MoveTrack.h
SplitClip.h
TrackPan.h
WorkCursorMove.h
PlayHeadMove.h
MoveCommand.h
Zoom.h
)

SET(TRAVERSO_COMMANDS_UI_FILES
ui/ExternalProcessingDialog.ui
)

QT4_WRAP_CPP(TRAVERSO_COMMANDS_MOC_SOURCES ${TRAVERSO_COMMANDS_MOC_CLASSES})
QT4_WRAP_UI(TRAVERSO_COMMANDS_UI_SOURCES ${TRAVERSO_COMMANDS_UI_FILES})


SET(TRAVERSO_COMMANDS_LIBRARY "traversocommands")

ADD_LIBRARY(${TRAVERSO_COMMANDS_LIBRARY} STATIC  ${TRAVERSO_COMMANDS_SOURCES} ${TRAVERSO_COMMANDS_UI_SOURCES} ${TRAVERSO_COMMANDS_MOC_SOURCES})

TARGET_LINK_LIBRARIES(traversocommands
	traversocore
)

IF(USE_PCH)
    ADD_DEPENDENCIES(traversocommands precompiled_headers)
ENDIF(USE_PCH)
