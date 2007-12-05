#! /bin/bash

###                                                    ###
#    This script is used to create a bundle for OS X     #
###                                                    ###

QT_PATH=/usr/local/Trolltech/Qt-4.3.1

mkdir -p traverso.app/Contents/MacOS/
mkdir -p traverso.app/Contents/Resources/
mkdir -p traverso.app/Contents/Frameworks/
mkdir -p traverso.app/Contents/Frameworks/QtXml.framework/Versions/4/
mkdir -p traverso.app/Contents/Frameworks/QtOpenGL.framework/Versions/4/
mkdir -p traverso.app/Contents/Frameworks/QtGui.framework/Versions/4/
mkdir -p traverso.app/Contents/Frameworks/QtCore.framework/Versions/4/

cp bin/traverso traverso.app/Contents/MacOS/
cp resources/images/traverso_mac.icns traverso.app/Contents/Resources/Traverso.icns
cp resources/Info.plist traverso.app/Contents/
cp /opt/local/bin/cdrdao traverso.app/Contents/MacOS/
cp /usr/local/bin/sox traverso.app/Contents/MacOS/

cp /usr/local/lib/libsndfile.1.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libsamplerate.0.dylib  traverso.app/Contents/Frameworks
cp /usr/local/lib/libportaudio.2.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/librdf.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/librasqal.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libvorbisfile.3.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libvorbisenc.2.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libvorbis.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libogg.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libmad.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libFLAC++.6.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libFLAC.8.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libraptor.1.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libst.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libwavpack.1.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libmp3lame.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/liblo.0.dylib traverso.app/Contents/Frameworks
cp $QT_PATH/lib/QtXml.framework/Versions/4/QtXml traverso.app/Contents/Frameworks/QtXml.framework/Versions/4/
cp $QT_PATH/lib/QtOpenGL.framework/Versions/4/QtOpenGL traverso.app/Contents/Frameworks/QtOpenGL.framework/Versions/4/
cp $QT_PATH/lib/QtGui.framework/Versions/4/QtGui traverso.app/Contents/Frameworks/QtGui.framework/Versions/4/
cp $QT_PATH/lib/QtCore.framework/Versions/4/QtCore traverso.app/Contents/Frameworks/QtCore.framework/Versions/4/

install_name_tool -id @executable_path/../Frameworks/libsndfile.1.dylib traverso.app/Contents/Frameworks/libsndfile.1.dylib
install_name_tool -id @executable_path/../Frameworks/libsamplerate.0.dylib traverso.app/Contents/Frameworks/libsamplerate.0.dylib
install_name_tool -id @executable_path/../Frameworks/libportaudio.2.dylib traverso.app/Contents/Frameworks/libportaudio.2.dylib
install_name_tool -id @executable_path/../Frameworks/librdf.0.dylib traverso.app/Contents/Frameworks/librdf.0.dylib
install_name_tool -id @executable_path/../Frameworks/librasqal.0.dylib traverso.app/Contents/Frameworks/librasqal.0.dylib
install_name_tool -id @executable_path/../Frameworks/libvorbis.0.dylib traverso.app/Contents/Frameworks/libvorbis.0.dylib
install_name_tool -id @executable_path/../Frameworks/libvorbisfile.3.dylib traverso.app/Contents/Frameworks/libvorbisfile.3.dylib
install_name_tool -id @executable_path/../Frameworks/libvorbisenc.2.dylib traverso.app/Contents/Frameworks/libvorbisenc.2.dylib
install_name_tool -id @executable_path/../Frameworks/libogg.0.dylib traverso.app/Contents/Frameworks/libogg.0.dylib
install_name_tool -id @executable_path/../Frameworks/libmad.0.dylib traverso.app/Contents/Frameworks/libmad.0.dylib
install_name_tool -id @executable_path/../Frameworks/libraptor.1.dylib traverso.app/Contents/Frameworks/libraptor.1.dylib
install_name_tool -id @executable_path/../Frameworks/libst.0.dylib traverso.app/Contents/Frameworks/libst.0.dylib
install_name_tool -id @executable_path/../Frameworks/libwavpack.1.dylib traverso.app/Contents/Frameworks/libwavpack.1.dylib
install_name_tool -id @executable_path/../Frameworks/libFLAC++.6.dylib traverso.app/Contents/Frameworks/libFLAC++.6.dylib
install_name_tool -id @executable_path/../Frameworks/libFLAC.8.dylib traverso.app/Contents/Frameworks/libFLAC.8.dylib
install_name_tool -id @executable_path/../Frameworks/libmp3lame.0.dylib traverso.app/Contents/Frameworks/libmp3lame.0.dylib
install_name_tool -id @executable_path/../Frameworks/liblo.0.dylib traverso.app/Contents/Frameworks/liblo.0.dylib
install_name_tool -id @executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml traverso.app/Contents/Frameworks/QtXml.framework/Versions/4/QtXml
install_name_tool -id @executable_path/../Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL traverso.app/Contents/Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL
install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui traverso.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore traverso.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore

install_name_tool -change /usr/local/lib/libsndfile.1.dylib @executable_path/../Frameworks/libsndfile.1.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libsamplerate.0.dylib @executable_path/../Frameworks/libsamplerate.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libportaudio.2.dylib @executable_path/../Frameworks/libportaudio.2.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/librdf.0.dylib @executable_path/../Frameworks/librdf.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/librasqal.0.dylib @executable_path/../Frameworks/librasqal.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libvorbis.0.dylib @executable_path/../Frameworks/libvorbis.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libvorbisfile.3.dylib @executable_path/../Frameworks/libvorbisfile.3.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libvorbisenc.2.dylib @executable_path/../Frameworks/libvorbisenc.2.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libmad.0.dylib @executable_path/../Frameworks/libmad.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libraptor.1.dylib @executable_path/../Frameworks/libraptor.1.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libwavpack.1.dylib @executable_path/../Frameworks/libwavpack.1.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libFLAC++.6.dylib @executable_path/../Frameworks/libFLAC++.6.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libFLAC.8.dylib @executable_path/../Frameworks/libFLAC.8.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libmp3lame.0.dylib @executable_path/../Frameworks/libmp3lame.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/liblo.0.dylib @executable_path/../Frameworks/liblo.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libst.0.dylib @executable_path/../Frameworks/libst.0.dylib traverso.app/Contents/MacOS/sox
install_name_tool -change /usr/local/lib/libsndfile.1.dylib @executable_path/../Frameworks/libsndfile.1.dylib traverso.app/Contents/MacOS/sox
install_name_tool -change /usr/local/lib/libsamplerate.0.dylib @executable_path/../Frameworks/libsamplerate.0.dylib traverso.app/Contents/MacOS/sox
install_name_tool -change $QT_PATH/lib/QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml traverso.app/Contents/MacOS/traverso
install_name_tool -change $QT_PATH/lib/QtOpenGL.framework/Versions/4/QtOpenGL @executable_path/../Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL traverso.app/Contents/MacOS/traverso
install_name_tool -change $QT_PATH/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui traverso.app/Contents/MacOS/traverso
install_name_tool -change $QT_PATH/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore traverso.app/Contents/MacOS/traverso

install_name_tool -change $QT_PATH/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore traverso.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
install_name_tool -change $QT_PATH/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore traverso.app/Contents/Frameworks/QtXml.framework/Versions/4/QtXml
install_name_tool -change $QT_PATH/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore traverso.app/Contents/Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL
install_name_tool -change $QT_PATH/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui traverso.app/Contents/Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL

install_name_tool -change /usr/local/lib/librasqal.0.dylib @executable_path/../Frameworks/librasqal.0.dylib traverso.app/Contents/Frameworks/librdf.0.dylib
install_name_tool -change /usr/local/lib/libraptor.1.dylib @executable_path/../Frameworks/libraptor.1.dylib traverso.app/Contents/Frameworks/librdf.0.dylib
install_name_tool -change /usr/local/lib/libraptor.1.dylib @executable_path/../Frameworks/libraptor.1.dylib traverso.app/Contents/Frameworks/librasqal.0.dylib
install_name_tool -change /usr/local/lib/libst.0.dylib @executable_path/../Frameworks/libst.0.dylib traverso.app/Contents/Frameworks/libst.0.dylib
install_name_tool -change /usr/local/lib/libsndfile.1.dylib @executable_path/../Frameworks/libsndfile.1.dylib traverso.app/Contents/Frameworks/libst.0.dylib
install_name_tool -change /usr/local/lib/libsamplerate.0.dylib @executable_path/../Frameworks/libsamplerate.0.dylib traverso.app/Contents/Frameworks/libst.0.dylib
install_name_tool -change /usr/local/lib/libvorbisfile.3.dylib @executable_path/../Frameworks/libvorbisfile.3.dylib traverso.app/Contents/Frameworks/libvorbisfile.3.dylib
install_name_tool -change /usr/local/lib/libvorbis.0.dylib @executable_path/../Frameworks/libvorbis.0.dylib traverso.app/Contents/Frameworks/libvorbisfile.3.dylib
install_name_tool -change /usr/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib traverso.app/Contents/Frameworks/libvorbisfile.3.dylib

install_name_tool -change /usr/local/lib/libvorbis.0.dylib @executable_path/../Frameworks/libvorbis.0.dylib traverso.app/Contents/Frameworks/libvorbis.0.dylib
install_name_tool -change /usr/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib traverso.app/Contents/Frameworks/libvorbis.0.dylib
install_name_tool -change /usr/local/lib/libvorbis.0.dylib @executable_path/../Frameworks/libvorbis.0.dylib traverso.app/Contents/Frameworks/libvorbisenc.2.dylib
install_name_tool -change /usr/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib traverso.app/Contents/Frameworks/libvorbisenc.2.dylib

install_name_tool -change /usr/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib traverso.app/Contents/Frameworks/libogg.0.dylib
install_name_tool -change /usr/local/lib/libmad.0.dylib @executable_path/../Frameworks/libmad.0.dylib traverso.app/Contents/Frameworks/libmad.0.dylib

install_name_tool -change /usr/local/lib/libFLAC++.6.dylib @executable_path/../Frameworks/libFLAC++.6.dylib traverso.app/Contents/Frameworks/libFLAC++.6.dylib
install_name_tool -change /usr/local/lib/libFLAC.8.dylib @executable_path/../Frameworks/libFLAC.8.dylib traverso.app/Contents/Frameworks/libFLAC++.6.dylib
install_name_tool -change /usr/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib traverso.app/Contents/Frameworks/libFLAC++.6.dylib
install_name_tool -change /usr/local/lib/libFLAC.8.dylib @executable_path/../Frameworks/libFLAC.8.dylib traverso.app/Contents/Frameworks/libFLAC.8.dylib
install_name_tool -change /usr/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib traverso.app/Contents/Frameworks/libFLAC.8.dylib

mv traverso.app Traverso.app

