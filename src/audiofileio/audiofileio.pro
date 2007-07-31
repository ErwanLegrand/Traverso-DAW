include(../libbase.pri)

INCLUDEPATH += decode encode \
	../core \
	../engine
	
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
	decode/MadAudioReader.cpp
HEADERS = decode/AbstractAudioReader.h \
	decode/SFAudioReader.h \
	decode/FlacAudioReader.h \
	decode/ResampleAudioReader.h \
	decode/VorbisAudioReader.h \
	decode/WPAudioReader.h \
	decode/MadAudioReader.h
macx{
    QMAKE_LIBDIR += /usr/local/qt/lib
}

win32{
    INCLUDEPATH += ../../3thparty/include .
}
