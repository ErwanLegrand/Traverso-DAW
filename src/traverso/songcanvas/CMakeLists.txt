INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/commands
${CMAKE_SOURCE_DIR}/src/common
${CMAKE_SOURCE_DIR}/src/core
${CMAKE_SOURCE_DIR}/src/engine
${CMAKE_SOURCE_DIR}/src/plugins
${CMAKE_SOURCE_DIR}/src/plugins/native
${CMAKE_SOURCE_DIR}/src/plugins/LV2
${CMAKE_SOURCE_DIR}/src/3rdparty/slv2
${CMAKE_SOURCE_DIR}/src/traverso
${CMAKE_SOURCE_DIR}/src/traverso/dialogs
${QT_QTGUI_INCLUDE_DIR}
${QT_QTXML_INCLUDE_DIR}
)

IF(HAVE_OPENGL)
INCLUDE_DIRECTORIES(
${QT_QTOPENGL_INCLUDE_DIR}
)
ENDIF(HAVE_OPENGL)

SET(TRAVERSO_SONGCANVAS_SOURCES
AudioClipView.cpp
ClipsViewPort.cpp
Cursors.cpp
CurveNodeView.cpp
CurveView.cpp
FadeContextDialogView.cpp
FadeView.cpp
LineView.cpp
MarkerView.cpp
PluginChainView.cpp
PluginView.cpp
PositionIndicator.cpp
SongView.cpp
SongWidget.cpp
TimeLineView.cpp
TimeLineViewPort.cpp
TrackPanelView.cpp
TrackPanelViewPort.cpp
TrackView.cpp
ViewItem.cpp
dialogs/AudioClipEditDialog.cpp
)

SET(TRAVERSO_SONGCANVAS_MOC_CLASSES
AudioClipView.h
ClipsViewPort.h
Cursors.h
CurveNodeView.h
FadeView.h
MarkerView.h
PluginChainView.h
PluginView.h
PositionIndicator.h
TrackView.h
ViewItem.h
dialogs/AudioClipEditDialog.h
CurveView.h
SongView.h
SongWidget.h
TrackPanelView.h
TimeLineView.h
)

QT4_WRAP_CPP(TRAVERSO_SONGCANVAS_MOC_SOURCES ${TRAVERSO_SONGCANVAS_MOC_CLASSES})
QT4_WRAP_UI(TRAVERSO_SONGCANVAS_UI_SOURCES ${TRAVERSO_SONGCANVAS_UI_FILES})


SET(TRAVERSO_SONGCANVAS_LIBRARY "traversosongcanvas")

ADD_LIBRARY(${TRAVERSO_SONGCANVAS_LIBRARY} STATIC 
	${TRAVERSO_SONGCANVAS_SOURCES} 
	${TRAVERSO_SONGCANVAS_MOC_SOURCES} 
	${TRAVERSO_SONGCANVAS_UI_SOURCES}
)

