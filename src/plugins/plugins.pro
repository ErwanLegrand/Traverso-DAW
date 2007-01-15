include(../libbase.pri) 


INCLUDEPATH += ../../src/core \
		../../src/engine \
		../../src/commands \
		../../src/plugins/LV2 \
		../../src/plugins/native \
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
	LV2/LV2PluginPropertiesDialog.h \
	native/CorrelationMeter.h \
	native/SpectralMeter.h \

SOURCES += Plugin.cpp \
	AudioInputPort.cpp \
	PluginChain.cpp \
	PluginPort.cpp \
	AudioOutputPort.cpp \
	PluginManager.cpp \
	PluginSlider.cpp \
	LV2/LV2Plugin.cpp \
	LV2/LV2ControlPort.cpp \
	LV2/LV2PluginPropertiesDialog.cpp \
	native/CorrelationMeter.cpp \
	native/SpectralMeter.cpp \

win32 {
	HEADERS -= 	LV2/LV2Plugin.h \
			LV2/LV2ControlPort.h \
			LV2/LV2PluginPropertiesDialog.h
	SOURCES -= 	LV2/LV2Plugin.cpp \
			LV2/LV2ControlPort.cpp \
			LV2/LV2PluginPropertiesDialog.cpp
	INCLUDEPATH -= ../../src/plugins/LV2

}