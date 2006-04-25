# Dit bestand is gegenereerd door KDevelop's QMake-Manager.
# ------------------------------------------- 
# De submap relatief aan de projectmap: ./src/core
# Het Target is een bibliotheek:  traversocore

include(../libbase.pri)

PRECOMPILED_HEADER = precompile.h 

LIBS += -ltraversocommands \
        -ltraverso 
        
INCLUDEPATH += ../../src/traverso \
               ../../src/traverso/build \
               ../../src/core \
               ../../src/commands \
               ../../src/engine \
               . 
QMAKE_LIBDIR = ../../lib 
TARGET = traversocore 
DESTDIR = ../../lib 

TEMPLATE = lib 
HEADERS += precompile.h \
	   AudioClipList.h \
           AudioSource.h \
	   ContextItem.h \
           AudioPluginChain.h \
           AudioPluginController.h \
           Track.h \
           AudioClip.h \
           AudioSourcesList.h \
           Command.h \
           Curve.h \
           Debugger.h \
           DiskIO.h \
           Export.h \
           FileHelpers.h \
           HistoryStack.h \
           Mixer.h \
           MtaRegion.h \
           MtaRegionList.h \
           Peak.h \
           PluginLoader.h \
           Project.h \
           ProjectManager.h \
           ReadSource.h \
           RingBuffer.h \
           Song.h \
           Information.h \
           InputEngine.h \
           TraversoAudioPlugin.h \
           ContextPointer.h \
           IEAction.h \
           IEMessage.h \
           Utils.h \
           WriteSource.h \
           libtraversocore.h \
           gdither.h \
           gdither_types.h \
           gdither_types_internal.h \
           noise.h \
           FastDelegate.h
           
SOURCES += AudioClip.cpp \
           ReadSource.cpp \
           WriteSource.cpp \
	   AudioClipList.cpp \
           AudioPluginChain.cpp \
           AudioPluginController.cpp \
           AudioSource.cpp \
           AudioSourcesList.cpp \
           Command.cpp \
	   ContextItem.cpp \
           ContextPointer.cpp \
           Curve.cpp \
           Debugger.cpp \
           DiskIO.cpp \
           Export.cpp \
           FileHelpers.cpp \
           HistoryStack.cpp \
           IEAction.cpp \
           IEMessage.cpp \
           Information.cpp \
           InputEngine.cpp \
           Mixer.cpp \
           MtaRegion.cpp \
           MtaRegionList.cpp \
           Peak.cpp \
           PluginLoader.cpp \
           Project.cpp \
           ProjectManager.cpp \
           RingBuffer.cpp \
           Song.cpp \
           Track.cpp \
           TraversoAudioPlugin.cpp \
           Utils.cpp \
           gdither.cpp
