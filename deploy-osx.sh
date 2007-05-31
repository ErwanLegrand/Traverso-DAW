#! /bin/bash

###                                                    ###
#    This script is used to create a bundle for OS X     #
###                                                    ###

QT_PATH=/usr/local/Trolltech/Qt-4.3.0

cp /opt/local/bin/cdrdao traverso.app/Contents/MacOS/

mkdir -p traverso.app/Contents/Frameworks
mkdir -p traverso.app/Contents/Frameworks/QtXml.framework/Versions/4/
mkdir -p traverso.app/Contents/Frameworks/QtOpenGL.framework/Versions/4/
mkdir -p traverso.app/Contents/Frameworks/QtGui.framework/Versions/4/
mkdir -p traverso.app/Contents/Frameworks/QtCore.framework/Versions/4/

cp /usr/local/lib/libsndfile.1.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libsamplerate.0.dylib  traverso.app/Contents/Frameworks
cp /usr/local/lib/libportaudio.2.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/librdf.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/librasqal.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libglib-2.0.0.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libintl.8.dylib traverso.app/Contents/Frameworks
cp /usr/local/lib/libraptor.1.dylib traverso.app/Contents/Frameworks
cp $QT_PATH/lib/QtXml.framework/Versions/4/QtXml traverso.app/Contents/Frameworks/QtXml.framework/Versions/4/
cp $QT_PATH/lib/QtOpenGL.framework/Versions/4/QtOpenGL traverso.app/Contents/Frameworks/QtOpenGL.framework/Versions/4/
cp $QT_PATH/lib/QtGui.framework/Versions/4/QtGui traverso.app/Contents/Frameworks/QtGui.framework/Versions/4/
cp $QT_PATH/lib/QtCore.framework/Versions/4/QtCore traverso.app/Contents/Frameworks/QtCore.framework/Versions/4/

install_name_tool -id @executable_path/../Frameworks/libsndfile.1.dylib traverso.app/Contents/Frameworks/libsndfile.1.dylib
install_name_tool -id @executable_path/../Frameworks/libsamplerate.0.dylib traverso.app/Contents/Frameworks/libsamplerate.0.dylib
install_name_tool -id @executable_path/../Frameworks/libportaudio.2.dylib traverso.app/Contents/Frameworks/libportaudio.2.dylib
install_name_tool -id @executable_path/../Frameworks/librdf.0.dylib traverso.app/Contents/Frameworks/librdf.0.dylib
install_name_tool -id @executable_path/../Frameworks/librasqal.0.dylib traverso.app/Contents/Frameworks/librasqal.0.dylib
install_name_tool -id @executable_path/../Frameworks/libglib-2.0.0.dylib traverso.app/Contents/Frameworks/libglib-2.0.0.dylib
install_name_tool -id @executable_path/../Frameworks/libintl.8.dylib traverso.app/Contents/Frameworks/libintl.8.dylib
install_name_tool -id @executable_path/../Frameworks/libraptor.1.dylib traverso.app/Contents/Frameworks/libraptor.1.dylib
install_name_tool -id @executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml traverso.app/Contents/Frameworks/QtXml.framework/Versions/4/QtXml
install_name_tool -id @executable_path/../Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL traverso.app/Contents/Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL
install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui traverso.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore traverso.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore

install_name_tool -change /usr/local/lib/libsndfile.1.dylib @executable_path/../Frameworks/libsndfile.1.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libsamplerate.0.dylib @executable_path/../Frameworks/libsamplerate.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libportaudio.2.dylib @executable_path/../Frameworks/libportaudio.2.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/librdf.0.dylib @executable_path/../Frameworks/librdf.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/librasqal.0.dylib @executable_path/../Frameworks/librasqal.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libglib-2.0.0.dylib @executable_path/../Frameworks/libglib-2.0.0.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libintl.8.dylib @executable_path/../Frameworks/libintl.8.dylib traverso.app/Contents/MacOS/traverso
install_name_tool -change /usr/local/lib/libraptor.1.dylib @executable_path/../Frameworks/libraptor.1.dylib traverso.app/Contents/MacOS/traverso
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
install_name_tool -change /usr/local/lib/libintl.8.dylib @executable_path/../Frameworks/libintl.8.dylib traverso.app/Contents/Frameworks/libglib-2.0.0.dylib

