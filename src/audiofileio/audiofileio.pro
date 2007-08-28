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
	decode/MadAudioReader.cpp \
	encode/AbstractAudioWriter.cpp \
	encode/SFAudioWriter.cpp \
	encode/WPAudioWriter.cpp \
	decode/PeakDataReader.cpp
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
	decode/PeakDataReader.h
macx{
    QMAKE_LIBDIR += /usr/local/qt/lib
}

win32{
    INCLUDEPATH += ../../3thparty/include .
}

unix{
    system(which relaytool 2>/dev/null >/dev/null){
        DEFINES += RELAYTOOL_FLAC="'extern int libFLAC_is_present; extern int libFLAC_symbol_is_present(char *s);'"

        DEFINES += RELAYTOOL_MAD="'extern int libmad_is_present; extern int libmad_symbol_is_present(char *s);'"

        DEFINES += RELAYTOOL_VORBISFILE="'extern int libvorbisfile_is_present; extern int libvorbisfile_symbol_is_present(char *s);'"
    }    else{
        DEFINES += RELAYTOOL_FLAC="'static const int libFLAC_is_present=1; static int __attribute__((unused)) libFLAC_symbol_is_present(char *) { return 1; }'"

        DEFINES += RELAYTOOL_MAD="'static const int libmad_is_present=1; static int __attribute__((unused)) libmad_symbol_is_present(char *) { return 1; }'"

        DEFINES += RELAYTOOL_VORBISFILE="'static const int libvorbisfile_is_present=1; static int __attribute__((unused)) libvorbisfile_symbol_is_present(char *) { return 1; }'"
    }
}

!unix{
    DEFINES += RELAYTOOL_FLAC="\"static const int libFLAC_is_present=1; static int __attribute__((unused)) libFLAC_symbol_is_present(char *) { return 1; }\""

    DEFINES += RELAYTOOL_MAD="\"static const int libmad_is_present=1; static int __attribute__((unused)) libmad_symbol_is_present(char *) { return 1; }\""

    DEFINES += RELAYTOOL_VORBISFILE="\"static const int libvorbisfile_is_present=1; static int __attribute__((unused)) libvorbisfile_symbol_is_present(char *) { return 1; }\""
}
HEADERS -= PeakDataReader.h

