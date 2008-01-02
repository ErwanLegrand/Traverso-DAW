ADD_SUBDIRECTORY(plugins ${TRAVERSO_BUILD_DIR})

INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/commands
${CMAKE_SOURCE_DIR}/src/common
${CMAKE_SOURCE_DIR}/src/core
${CMAKE_SOURCE_DIR}/src/traverso
${CMAKE_SOURCE_DIR}/src/traverso/songcanvas
${CMAKE_SOURCE_DIR}/src/plugins
${CMAKE_SOURCE_DIR}/src/plugins/native
${QT_QTGUI_INCLUDE_DIR}
${QT_QTXML_INCLUDE_DIR}
)

SET(TRAVERSO_COMMANDS_SOURCES
AddRemove.cpp
ClipSelection.cpp
CommandGroup.cpp
Fade.cpp
Gain.cpp
Import.cpp
MoveClip.cpp
MultiMove.cpp
MoveEdge.cpp
PCommand.cpp
SplitClip.cpp
TrackPan.cpp
Zoom.cpp
Scroll.cpp
AudioClipExternalProcessing.cpp
ArmTracks.cpp
PlayHeadMove.cpp
WorkCursorMove.cpp
RemoveClip.cpp
)

SET(TRAVERSO_COMMANDS_MOC_CLASSES
Gain.h
MoveClip.h
AudioClipExternalProcessing.h
)

SET(TRAVERSO_COMMANDS_UI_FILES
ui/ExternalProcessing.ui
)

QT4_WRAP_CPP(TRAVERSO_COMMANDS_MOC_SOURCES ${TRAVERSO_COMMANDS_MOC_CLASSES})
QT4_WRAP_UI(TRAVERSO_COMMANDS_UI_SOURCES ${TRAVERSO_COMMANDS_UI_FILES})


SET(TRAVERSO_COMMANDS_LIBRARY "traversocommands")

ADD_LIBRARY(${TRAVERSO_COMMANDS_LIBRARY} STATIC ${TRAVERSO_COMMANDS_SOURCES} ${TRAVERSO_COMMANDS_UI_SOURCES} ${TRAVERSO_COMMANDS_MOC_SOURCES})

IF(USE_PCH)
    ADD_DEPENDENCIES(traversocommands precompiled_headers)
ENDIF(USE_PCH)
