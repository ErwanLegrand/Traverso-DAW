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

SET(TRAVERSO_SONGCANVAS_SOURCES
AudioClipView.cpp
AudioTrackView.cpp
ClipsViewPort.cpp
Cursors.cpp
CurveNodeView.cpp
CurveView.cpp
FadeView.cpp
MarkerView.cpp
PluginChainView.cpp
PluginView.cpp
PositionIndicator.cpp
TrackView.cpp
SheetView.cpp
SheetWidget.cpp
SubGroupView.cpp
TimeLineView.cpp
TimeLineViewPort.cpp
TrackPanelView.cpp
TrackPanelViewPort.cpp
ViewItem.cpp
dialogs/AudioClipEditDialog.cpp
)

SET(TRAVERSO_SONGCANVAS_MOC_CLASSES
AudioClipView.h
AudioTrackView.h
ClipsViewPort.h
Cursors.h
CurveNodeView.h
FadeView.h
MarkerView.h
PluginChainView.h
PluginView.h
TrackView.h
PositionIndicator.h
SubGroupView.h
ViewItem.h
dialogs/AudioClipEditDialog.h
CurveView.h
SheetView.h
SheetWidget.h
TrackPanelView.h
TimeLineView.h
LineView.h
)

SET(TRAVERSO_SONGCANVAS_UI_FILES
ui/AudioClipEditDialog.ui
)

QT4_WRAP_CPP(TRAVERSO_SONGCANVAS_MOC_SOURCES ${TRAVERSO_SONGCANVAS_MOC_CLASSES})
QT4_WRAP_UI(TRAVERSO_SONGCANVAS_UI_SOURCES ${TRAVERSO_SONGCANVAS_UI_FILES})


SET(TRAVERSO_SONGCANVAS_LIBRARY "traversosheetcanvas")

ADD_LIBRARY(${TRAVERSO_SONGCANVAS_LIBRARY} STATIC 
	${TRAVERSO_SONGCANVAS_SOURCES} 
	${TRAVERSO_SONGCANVAS_MOC_SOURCES} 
	${TRAVERSO_SONGCANVAS_UI_SOURCES}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(traversosheetcanvas precompiled_headers)
ENDIF(USE_PCH)
