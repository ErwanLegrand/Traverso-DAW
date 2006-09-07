include(../libbase.pri) 


INCLUDEPATH += ../../src/core \
		../../src/engine \
		../../src/commands \
		../../src/plugins/LV2 \
		./

DESTDIR = ../../lib 

TARGET = traversoplugins 
TEMPLATE = lib 

HEADERS += Plugin.h \
	AudioInputPort.h \
	AudioOutputPort.h \
	PluginChain.h \
	PluginPort.h \
	PluginSlider.h \
	PluginManager.h \
	LV2/LV2Plugin.h \
	LV2/LV2ControlPort.h \
	LV2/LV2PluginPropertiesDialog.h

SOURCES += Plugin.cpp \
	PluginChain.cpp \
	PluginPort.cpp \
	AudioInputPort.cpp \
	AudioOutputPort.cpp \
	PluginManager.cpp \
	PluginSlider.cpp \
	LV2/LV2Plugin.cpp \
	LV2/LV2ControlPort.cpp \
	LV2/LV2PluginPropertiesDialog.cpp

win32 {
	HEADERS -= 	LV2/LV2Plugin.h \
			LV2/LV2ControlPort.h \
			LV2/LV2PluginPropertiesDialog.h
	SOURCES -= 	LV2/LV2Plugin.cpp \
			LV2/LV2ControlPort.cpp \
			LV2/LV2PluginPropertiesDialog.cpp
	INCLUDEPATH -= ../../src/plugins/LV2

}