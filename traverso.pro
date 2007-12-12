# Dit bestand is gegenereerd door KDevelop's QMake-Manager.
# ------------------------------------------- 
# De submap relatief aan de projectmap: .
# Het Target is een project in submap 

 
TEMPLATE = subdirs

SUBDIRS += src/engine \
	   src/commands \
           src/core \
           src/audiofileio \
           src/plugins \
           src/traverso/songcanvas \
	   src/commands/plugins/TraversoCommands \
           src/3rdparty/slv2 \
           src/traverso

DISTFILES += \
	ChangeLog \
	README \
	AUTHORS \
	COPYING \
	COPYRIGHT \
	INSTALL \
	src/config.h \
	src/base.pri \
	src/appbase.pri \
	src/libbase.pri \
	resources/help.text \
	resources/keymap.xml \
	resources/images/cursorFloatOverClip.xpm \
	resources/images/cursorFloatOverTrack.xpm \
	resources/images/cursorFloat.xpm \
	resources/images/cursorDrag.xpm \
	resources/images/cursorMagicZoom.xpm \
	resources/images/cursorHoldLrud.xpm \
	resources/images/cursorHoldLr.xpm \
	resources/images/cursorHoldUd.xpm \
	resources/images/cursorSelect.xpm \
	resources/images/cursorRZoom.xpm \
	resources/images/traverso.xpm \
	resources/images/rec-on.xpm \
	resources/images/rec-off.xpm \
	resources/images/mute-on.xpm \
	resources/images/mute-off.xpm \
	resources/images/solo-on.xpm \
	resources/images/solo-off.xpm \
	resources/images/lock-on.xpm \
	resources/images/lock-off.xpm \
	resources/images/bus-in.xpm \
	resources/images/bus-out.xpm \
	resources/images/icons/22X22/critical.png \
	resources/images/icons/22X22/info.png \
	resources/images/icons/22X22/warning.png \
	resources/images/icons/32X32/advancedsettings.png \
	resources/images/icons/32X32/audiosources.png \
	resources/images/icons/32X32/projects.png \
	resources/images/icons/32X32/songs.png \
	resources/skin/defaultcolors.xml \
	traverso_nl.qm \
	traverso_de.qm

win32{
    SUBDIRS -= src/3rdparty/slv2
}
