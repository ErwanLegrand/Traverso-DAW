include(../../../base.pri)

TEMPLATE = lib
CONFIG   += plugin

contains(DEFINES, STATIC_BUILD): CONFIG += static

INCLUDEPATH  += ../.. \
		../../../core \
		../../../traverso/songcanvas \
		../../../commands \
		../../../engine
		
DESTDIR = ../../../../lib/commandplugins

