include(../libbase.pri)

PRECOMPILED_HEADER = precompile.h 

LIBS += -ltraversocommands \
        -ltraversoaudiobackend

INCLUDEPATH += ../commands \
	../commands/build \
	../common \
	../engine \
	../audiofileio/decode \
	../audiofileio/encode \
	../plugins \
	../plugins/native

QMAKE_LIBDIR = ../../lib 
TARGET = traversocore 
DESTDIR = ../../lib 

TEMPLATE = lib 

SOURCES = \
 	../common/Utils.cpp \
	../common/Tsar.cpp \
	../common/Debugger.cpp \
	../common/Mixer.cpp \
	../common/RingBuffer.cpp \
	../common/Resampler.cpp \
	AudioClip.cpp \
	AudioClipManager.cpp \
	AudioSource.cpp \
	Command.cpp \
	Config.cpp \
	ContextPointer.cpp \
	Curve.cpp \
	CurveNode.cpp \
	DiskIO.cpp \
	Export.cpp \
	FadeCurve.cpp \
	FileHelpers.cpp \
	Information.cpp \
	InputEngine.cpp \
	Peak.cpp \
	Project.cpp \
	ProjectManager.cpp \
	ReadSource.cpp \
	ResourcesManager.cpp \
	Song.cpp \
	Track.cpp \
	ViewPort.cpp \
	WriteSource.cpp \
	gdither.cpp \
	SnapList.cpp \
	Snappable.cpp \
	TimeLine.cpp \
	Marker.cpp \
	Themer.cpp \
	AudioFileMerger.cpp \
	ProjectConverter.cpp
HEADERS = precompile.h \
	../common/Utils.h \
	../common/Tsar.h \
	../common/Debugger.h \
	../common/Mixer.h \
	../common/RingBuffer.h \
	../common/RingBufferNPT.h \
	../common/Resampler.h \
	AudioClip.h \
	AudioClipManager.h \
	AudioSource.h \
	Command.h \
	Config.h \
	ContextItem.h \
	ContextPointer.h \
	CurveNode.h \
	Curve.h \
	DiskIO.h \
	Export.h \
	FadeCurve.h \
	FileHelpers.h \
	Information.h \
	InputEngine.h \
	Peak.h \
	Project.h \
	ProjectManager.h \
	ReadSource.h \
	ResourcesManager.h \
	Song.h \
	Track.h \
	ViewPort.h \
	WriteSource.h \
	libtraversocore.h \
	gdither.h \
	gdither_types.h \
	gdither_types_internal.h \
	noise.h \
	SnapList.h \
	Snappable.h \
	CommandPlugin.h \
	TimeLine.h \
	Marker.h \
	Themer.h \
	AudioFileMerger.h \
	ProjectConverter.h
macx{
    QMAKE_LIBDIR += /usr/local/qt/lib
}

win32{
    INCLUDEPATH += ../../3thparty/include .
}
