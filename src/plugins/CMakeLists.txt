INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/common
${CMAKE_SOURCE_DIR}/src/core
${CMAKE_SOURCE_DIR}/src/engine
${CMAKE_SOURCE_DIR}/src/commands
${CMAKE_SOURCE_DIR}/src/plugins/LV2
${CMAKE_SOURCE_DIR}/src/plugins/native
${CMAKE_SOURCE_DIR}/src/3rdparty/slv2
${QT_QTXML_INCLUDE_DIR}
)

SET(TRAVERSO_PLUGINS_SOURCES
PluginChain.cpp
Plugin.cpp
PluginManager.cpp
PluginSlider.cpp
native/CorrelationMeter.cpp
native/SpectralMeter.cpp
LV2/LV2Plugin.cpp
native/GainEnvelope.cpp
PluginPropertiesDialog.cpp
)

SET(TRAVERSO_PLUGINS_MOC_CLASSES
Plugin.h
PluginChain.h
PluginPropertiesDialog.h
PluginSlider.h
native/CorrelationMeter.h
native/GainEnvelope.h
native/SpectralMeter.h
LV2/LV2Plugin.h
)

QT4_WRAP_CPP(TRAVERSO_PLUGINS_MOC_SOURCES ${TRAVERSO_PLUGINS_MOC_CLASSES})

SET(TRAVERSO_PLUGINS_LIBRARY "traversoplugins")


ADD_LIBRARY(${TRAVERSO_PLUGINS_LIBRARY} STATIC ${TRAVERSO_PLUGINS_SOURCES} ${TRAVERSO_PLUGINS_MOC_SOURCES})

IF(USE_PCH)
    ADD_DEPENDENCIES(traversoplugins precompiled_headers)
ENDIF(USE_PCH)
