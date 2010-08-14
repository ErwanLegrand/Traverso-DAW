INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/audiofileio/decode
${CMAKE_SOURCE_DIR}/src/audiofileio/encode
${CMAKE_SOURCE_DIR}/src/commands
${CMAKE_SOURCE_DIR}/src/common
${CMAKE_SOURCE_DIR}/src/engine
${CMAKE_SOURCE_DIR}/src/plugins
${CMAKE_SOURCE_DIR}/src/plugins/native
${QT_QTGUI_INCLUDE_DIR}
${QT_QTXML_INCLUDE_DIR}
)

SET(TRAVERSO_CORE_SOURCES
${CMAKE_SOURCE_DIR}/src/common/Utils.cpp
${CMAKE_SOURCE_DIR}/src/common/Tsar.cpp
${CMAKE_SOURCE_DIR}/src/common/Debugger.cpp
${CMAKE_SOURCE_DIR}/src/common/Mixer.cpp
${CMAKE_SOURCE_DIR}/src/common/RingBuffer.cpp
${CMAKE_SOURCE_DIR}/src/common/Resampler.cpp
AudioClip.cpp
AudioClipGroup.cpp
AudioClipManager.cpp
AudioFileCopyConvert.cpp
AudioFileMerger.cpp
AudioTrack.cpp
AudioSource.cpp
TCommand.cpp
TConfig.cpp
ContextPointer.cpp
Curve.cpp
CurveNode.cpp
DiskIO.cpp
Export.cpp
FadeCurve.cpp
FileHelpers.cpp
Information.cpp
InputEngine.cpp
Peak.cpp
Project.cpp
ProjectManager.cpp
ProcessingData.cpp
ReadSource.cpp
ResourcesManager.cpp
TBusTrack.cpp
TMenuTranslator.cpp
TSend.cpp
TSession.cpp
Sheet.cpp
Track.cpp
WriteSource.cpp
gdither.cpp
SnapList.cpp
Snappable.cpp
TimeLine.cpp
Marker.cpp
Themer.cpp
ProjectConverter.cpp
)

SET(TRAVERSO_CORE_MOC_CLASSES
${CMAKE_SOURCE_DIR}/src/common/Tsar.h
AudioClip.h
AudioClipManager.h
AudioFileMerger.h
AudioFileCopyConvert.h
AudioTrack.h
ProcessingData.h
TConfig.h
ContextPointer.h
Curve.h
DiskIO.h
Export.h
FadeCurve.h
Information.h
Marker.h
Peak.h
Project.h
ProjectConverter.h
ProjectManager.h
ReadSource.h
ResourcesManager.h
Sheet.h
TBusTrack.h
Themer.h
TMenuTranslator.h
TimeLine.h
Track.h
TSession.h
WriteSource.h
ContextItem.h
InputEngine.h
CommandPlugin.h
)

QT4_WRAP_CPP(TRAVERSO_CORE_MOC_SOURCES ${TRAVERSO_CORE_MOC_CLASSES})

IF(UNIX)
	IF(HOST_SUPPORTS_SSE)
		IF(IS_ARCH_X86)
			SET(ASM_FILE ${CMAKE_SOURCE_DIR}/src/common/sse_functions.S)
		ENDIF(IS_ARCH_X86)
		IF(IS_ARCH_X86_64)
			SET(ASM_FILE ${CMAKE_SOURCE_DIR}/src/common/sse_functions_64bit.S)
		ENDIF(IS_ARCH_X86_64)
		
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sse_functions.o
			COMMAND as ${ASM_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/sse_functions.o
			DEPENDS ${ASM_FILE}
		)
		SET(SSE_OBJECT_FILE ${CMAKE_CURRENT_BINARY_DIR}/sse_functions.o)
	ENDIF(HOST_SUPPORTS_SSE)
ENDIF(UNIX)


SET(TRAVERSO_CORE_LIBRARY "traversocore")

ADD_LIBRARY(${TRAVERSO_CORE_LIBRARY} STATIC ${TRAVERSO_CORE_SOURCES} ${TRAVERSO_CORE_MOC_SOURCES} ${SSE_OBJECT_FILE})

IF(USE_PCH)
    ADD_DEPENDENCIES(traversocore precompiled_headers)
ENDIF(USE_PCH)
