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
decode/MadAudioReader.cpp
encode/AbstractAudioWriter.cpp
encode/SFAudioWriter.cpp
encode/WPAudioWriter.cpp
encode/FlacAudioWriter.cpp
encode/VorbisAudioWriter.cpp
encode/LameAudioWriter.cpp
)

SET(TRAVERSO_AUDIOFILEIO_LIBRARY "traversoaudiofileio")

ADD_LIBRARY(${TRAVERSO_AUDIOFILEIO_LIBRARY} STATIC ${TRAVERSO_AUDIOFILEIO_SOURCES})

