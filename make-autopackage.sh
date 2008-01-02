#!/bin/bash
unset CXX2 # disable C++ ABI workaround since we're statically linking all C++ libs
export CXX1=g++

#slv2=$PWD/autopackage/static-libs/libslv2.a
# libGLU needs to be statically linked since it's a C++ lib, the others are uncommon
export APBUILD_DEBUG=1
export APBUILD_STATIC="rdf=$PWD/autopackage/static-libs/librdf.a raptor=$PWD/autopackage/static-libs/libraptor.a rasqal=$PWD/autopackage/static-libs/librasqal.a wavpack=$PWD/autopackage/static-libs/libwavpack.a samplerate sqlite3 libXrender"

export QTDIR=/usr/local/Trolltech/Qt-4.3.3
makepackage
