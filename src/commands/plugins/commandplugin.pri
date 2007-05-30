include(../../../base.pri)

TEMPLATE = lib
CONFIG   += plugin

contains(DEFINES, STATIC_BUILD): CONFIG += static

INCLUDEPATH  += ../.. \
		../../../core \
		../../../traverso/songcanvas \
		../../../commands \
		../../../engine \
		../../../plugins \
		../../../plugins/native \
		
DESTDIR = ../../../../lib/commandplugins

win32 { 
     INCLUDEPATH += 	../../../../3thparty/include
}