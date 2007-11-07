INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src/audiofileio/decode
${CMAKE_SOURCE_DIR}/src/audiofileio/encode
)

SET(TRAVERSO_AUDIOFILEIO_SOURCES
decode/AbstractAudioReader.cpp
decode/SFAudioReader.cpp
decode/FlacAudioReader.cpp
decode/ResampleAudioReader.cpp
decode/VorbisAudioReader.cpp
decode/WPAudioReader.cpp
encode/AbstractAudioWriter.cpp
encode/SFAudioWriter.cpp
encode/WPAudioWriter.cpp
encode/FlacAudioWriter.cpp
encode/VorbisAudioWriter.cpp
)

IF(HAVE_MP3_DECODING)
	SET(TRAVERSO_AUDIOFILEIO_SOURCES
		${TRAVERSO_AUDIOFILEIO_SOURCES}
		decode/MadAudioReader.cpp
	)
ENDIF(HAVE_MP3_DECODING)

IF(HAVE_MP3_ENCODING)
	SET(TRAVERSO_AUDIOFILEIO_SOURCES
		${TRAVERSO_AUDIOFILEIO_SOURCES}
		encode/LameAudioWriter.cpp
	)
ENDIF(HAVE_MP3_ENCODING)


SET(TRAVERSO_AUDIOFILEIO_LIBRARY "traversoaudiofileio")

ADD_LIBRARY(${TRAVERSO_AUDIOFILEIO_LIBRARY} STATIC ${TRAVERSO_AUDIOFILEIO_SOURCES})

IF(USE_PCH)
    ADD_DEPENDENCIES(traversoaudiofileio precompiled_headers)
ENDIF(USE_PCH)
