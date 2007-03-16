include(base.pri)

CONFIG += dll

contains(DEFINES, STATIC_BUILD) {
	CONFIG += static
	CONFIG -= dll
}	

#QMAKE_CXXFLAGS_RELEASE += -fPIC
#QMAKE_CXXFLAGS_DEBUG += -fPIC
