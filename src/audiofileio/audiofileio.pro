include(../libbase.pri)

INCLUDEPATH += \
	decode \
	encode \
	../common

QMAKE_LIBDIR = ../../lib 
TARGET = traversoaudiofileio 
DESTDIR = ../../lib 

TEMPLATE = lib 

SOURCES = decode/AbstractAudioReader.cpp \
	decode/SFAudioReader.cpp \
	decode/FlacAudioReader.cpp \
	decode/ResampleAudioReader.cpp \
	decode/VorbisAudioReader.cpp \
	decode/WPAudioReader.cpp \
	decode/MadAudioReader.cpp \
	encode/AbstractAudioWriter.cpp \
	encode/SFAudioWriter.cpp \
	encode/WPAudioWriter.cpp \
	encode/FlacAudioWriter.cpp \
	encode/VorbisAudioWriter.cpp \
	encode/LameAudioWriter.cpp

HEADERS = decode/AbstractAudioReader.h \
	decode/SFAudioReader.h \
	decode/FlacAudioReader.h \
	decode/ResampleAudioReader.h \
	decode/VorbisAudioReader.h \
	decode/WPAudioReader.h \
	decode/MadAudioReader.h \
	encode/AbstractAudioWriter.h \
	encode/SFAudioWriter.h \
	encode/WPAudioWriter.h \
	encode/FlacAudioWriter.h \
	encode/VorbisAudioWriter.h \
	encode/LameAudioWriter.h
macx{
    QMAKE_LIBDIR += /usr/local/qt/lib
}

win32{
    INCLUDEPATH += ../../3thparty/include .
}

HEADERS -= PeakDataReader.h \
decode/PeakDataReader.h
SOURCES -= decode/PeakDataReader.cpp

