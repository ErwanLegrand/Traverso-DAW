include(../libbase.pri)

PRECOMPILED_HEADER = precompile.h 

LIBS += -ltraversocommands \
        -ltraversoaudiobackend

INCLUDEPATH += ../commands \
	../engine \
	../plugins \
	../plugins/native

QMAKE_LIBDIR = ../../lib 
TARGET = traversocore 
DESTDIR = ../../lib 

TEMPLATE = lib 

SOURCES = AudioClip.cpp \
	AudioClipList.cpp \
	AudioClipManager.cpp \
	AudioSource.cpp \
	Command.cpp \
	Config.cpp \
	ContextPointer.cpp \
	Curve.cpp \
	CurveNode.cpp \
	Debugger.cpp \
	DiskIO.cpp \
	Export.cpp \
	FadeCurve.cpp \
	FileHelpers.cpp \
	Information.cpp \
	InputEngine.cpp \
	Mixer.cpp \
	Peak.cpp \
	MonoReader.cpp \
	Project.cpp \
	ProjectManager.cpp \
	ReadSource.cpp \
	ResourcesManager.cpp \
	RingBuffer.cpp \
	Song.cpp \
	Track.cpp \
	Utils.cpp \
	ViewPort.cpp \
	WriteSource.cpp \
	gdither.cpp \
	SnapList.cpp \
	Snappable.cpp \
	TimeLine.cpp \
	Marker.cpp \
	Themer.cpp \
	AbstractAudioReader.cpp \
	SFAudioReader.cpp \
	ResampleAudioReader.cpp \
	VorbisAudioReader.cpp \
	MadAudioReader.cpp
HEADERS = precompile.h \
	AudioClip.h \
	AudioClipList.h \
	AudioClipManager.h \
	AudioSource.h \
	Command.h \
	Config.h \
	ContextItem.h \
	ContextPointer.h \
	CurveNode.h \
	Curve.h \
	Debugger.h \
	DiskIO.h \
	Export.h \
	FadeCurve.h \
	FileHelpers.h \
	Information.h \
	InputEngine.h \
	Mixer.h \
	Peak.h \
	MonoReader.h \
	Project.h \
	ProjectManager.h \
	ReadSource.h \
	ResourcesManager.h \
	RingBuffer.h \
	RingBufferNPT.h \
	Song.h \
	Track.h \
	Utils.h \
	ViewPort.h \
	WriteSource.h \
	libtraversocore.h \
	gdither.h \
	gdither_types.h \
	gdither_types_internal.h \
	noise.h \
	FastDelegate.h \
	SnapList.h \
	Snappable.h \
	CommandPlugin.h \
	TimeLine.h \
	Marker.h \
	Themer.h \
	AbstractAudioReader.h \
	SFAudioReader.h \
	ResampleAudioReader.h \
	VorbisAudioReader.h \
	MadAudioReader.h
macx{
    QMAKE_LIBDIR += /usr/local/qt/lib
}

win32{
    INCLUDEPATH += ../../3thparty/include .
}
