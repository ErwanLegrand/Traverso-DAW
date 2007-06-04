#!/bin/bash
unset CXX2 # disable C++ ABI workaround since we're statically linking all C++ libs
export CXX1=g++-gcc-3.4.6

#slv2=$PWD/autopackage/static-libs/libslv2.a
# libGLU needs to be statically linked since it's a C++ lib, the others are uncommon
export APBUILD_STATIC="rdf=$PWD/autopackage/static-libs/librdf.a raptor=$PWD/autopackage/static-libs/libraptor.a rasqal=$PWD/autopackage/static-libs/librasqal.a GLU=$PWD/autopackage/static-libs/libGLU.a samplerate sqlite3"

export QTDIR=/usr/local/Trolltech/Qt-4.3.0-apbuild/
makepackage
