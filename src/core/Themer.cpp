/*
Copyright (C) 2005-2006 Remon Sijrier 

This file is part of Traverso

Traverso is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

$Id: Themer.cpp,v 1.8 2007/11/24 10:27:15 r_sijrier Exp $
*/

#include "Themer.h"

#include <Utils.h>
#include <Config.h>
#include <QDir>
#include <QTextStream>
#include <QDomDocument>
#include <QApplication>
#include <QStyle>
#include <QFileSystemWatcher>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Themer* Themer::m_instance = 0;

Themer* themer()
{
	return Themer::instance();
}

Themer* Themer::instance()
{
	if (m_instance == 0) {
		m_instance = new Themer();
	}

	return m_instance;
}

Themer::Themer()
{
	m_watcher = new QFileSystemWatcher(this);
	QString themepath = config().get_property("Themer", "themepath", "").toString();
	
	// No theme path specified, fall back on built in themes only
	if (themepath.isEmpty()) {
		themepath = ":/themes";
	}
	
	// Detect and set theme based on current style
	// but only when no theme was specified by the user!
	
	m_currentTheme = config().get_property("Themer", "currenttheme", "").toString();
	m_systempallete = QApplication::palette();
	
	QString style = config().get_property("Themer", "style", "").toString();
	QString currentStyle = QString(QApplication::style()->metaObject()->className()).remove("Q").remove("Style");
	
	if (style.isEmpty()) {
		style = currentStyle;
		config().set_property("Themer", "style", currentStyle);
	}
	
	if (style != currentStyle) {
		QApplication::setStyle(style);
		currentStyle = style;
		config().set_property("Themer", "style", currentStyle);
	}
	
	if (m_currentTheme.isEmpty()) {
		if (currentStyle  == "Cleanlooks") {
			m_currentTheme = "ubuntu";
		} else if (currentStyle == "Plastique") {
			m_currentTheme = "TraversoLight";
		} else {
			m_currentTheme = "TraversoLight";
		}
		config().set_property("Themer", "currenttheme", m_currentTheme);
	}
	
	if (get_builtin_themes().contains(m_currentTheme)) {
		themepath = ":/themes";
	}
	
	m_themefile =  themepath + "/" + m_currentTheme + "/traversotheme.xml";
	m_coloradjust = -1;
	
	bool usestylepallete = config().get_property("Themer", "usestylepallet", "").toBool();
	
	if (usestylepallete) {
		QApplication::setPalette(QApplication::style()->standardPalette());
	}
	
	connect(m_watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(reload_on_themefile_change(const QString&)));
}


void Themer::save( )
{
	QDomDocument doc("TraversoTheming");
	QString fileName = QDir::homePath();
	fileName +=  "/.traverso/themes/default.xml";
	QFile data( fileName );

	if (data.open( QIODevice::WriteOnly ) ) {
		QDomElement colorManagerNode = doc.createElement("Themer");
		doc.appendChild(colorManagerNode);

		QHash<QString, QColor>::ConstIterator it = m_colors.begin();
		while (it != m_colors.end()) {
			QColor color = it.value();
			QDomElement colorProperty = doc.createElement("color");
			colorProperty.setAttribute("red", color.red());
			colorProperty.setAttribute("green", color.green());
			colorProperty.setAttribute("blue", color.blue());
			colorProperty.setAttribute("alpha", color.alpha() );
			colorProperty.setAttribute("name", it.key() );
			QVariant v;
			v = color;
			colorProperty.setAttribute("color", color.name());
			++it;
			colorManagerNode.appendChild(colorProperty);
		}

		QTextStream stream(&data);
		doc.save(stream, 4);
		data.close();
	} else {
		PWARN("Could not open Themer properties file for writing! (%s)", QS_C(fileName));
	}
}

void Themer::load( )
{
	QDomDocument doc("TraversoTheming");
 	
 	QFile file(m_themefile);
 	
 	if ( ! file.exists() ) {
		printf("File %s doesn't exit, falling back to default (TraversoLight) theme\n", QS_C(m_themefile));
		file.setFileName(":/themes/TraversoLight/traversotheme.xml");
	} else {
		if (!m_themefile.contains(":/")) {
	 		m_watcher->addPath(m_themefile);
		}
		printf("Themer:: Using themefile: %s\n", QS_C(m_themefile));
	}

	if (!file.open(QIODevice::ReadOnly)) {
		PWARN("Cannot open Themer properties file");
	}

	QString errorMsg;
	if (!doc.setContent(&file, &errorMsg)) {
		file.close();
		PWARN("Cannot set Content of XML file (%s)", QS_C(errorMsg));
	}

	file.close();

	QDomElement docElem = doc.documentElement();

	QDomNode colorsNode = docElem.firstChildElement("colors");
	QDomNode colorNode = colorsNode.firstChild();
	
	int coloradjust = config().get_property("Themer", "coloradjust", 100).toInt();
	
	// If the m_coloradjust value is set, use that one instead!
	if (m_coloradjust != -1) {
		coloradjust = m_coloradjust;
	}

	while(!colorNode.isNull()) {
		
		QDomElement e = colorNode.toElement();
		
		QColor color(
			e.attribute("red").toUInt(),
			e.attribute("green").toUInt(), 
			e.attribute("blue").toUInt(), 
			e.attribute("alpha").toInt()
			);
		QString name = e.attribute("name", "");
		
		if (coloradjust != 100) {
			int adjust = coloradjust - 100;
			if (adjust < 0) {
				color = color.dark(-1 * adjust + 100);
			} else {
				color = color.light(adjust + 100);
			}
		}
		m_colors.insert(name, color);
		colorNode = colorNode.nextSibling();
	}
		
		
	QDomNode fontsNode = docElem.firstChildElement("fonts");
	QDomNode fontNode = fontsNode.firstChild();
	
	QFont basefont = QApplication::font();

	while (!fontNode.isNull()) {
		
		QDomElement e = fontNode.toElement();
		
		QString name = e.attribute("name", "");
		QFont font(basefont);
		font.setPointSizeF(e.attribute("value", "1.0").toFloat() * (float)basefont.pointSizeF());
		
		m_fonts.insert(name, font);
		fontNode = fontNode.nextSibling();
	}
		
	
	QDomNode propertiesNode = docElem.firstChildElement("properties");
	QDomNode propertyNode = propertiesNode.firstChild();
	
	while (!propertyNode.isNull()) {
		
		QDomElement e = propertyNode.toElement();
		
		QString name = e.attribute("name", "");
		QVariant value = e.attribute("value", "");
		
		m_properties.insert(name, value);
		propertyNode = propertyNode.nextSibling();
	}
	
	m_cursors.clear();
	m_cursors.insert("AudioClip", QCursor(find_pixmap(":/cursorFloatOverClip")));
	m_cursors.insert("Track", QCursor(find_pixmap(":/cursorFloatOverTrack")));
	m_cursors.insert("Plugin", QCursor(find_pixmap(":/cursorFloatOverPlugin")));
	m_cursors.insert("Fade", QCursor(find_pixmap(":/cursorFloatOverFade")));
	m_cursors.insert("Default", QCursor(find_pixmap(":/cursorFloat")));
	m_cursors.insert("Zoom", QCursor(find_pixmap(":/cursorZoom")));
	m_cursors.insert("CurveNode", QCursor(find_pixmap(":/cursorDragNode")));
	
	m_cursors.insert("LRUD", QCursor(find_pixmap(":/cursorHoldLrud")));
	m_cursors.insert("LR", QCursor(find_pixmap(":/cursorHoldLr")));
	m_cursors.insert("UD", QCursor(find_pixmap(":/cursorHoldUd")));
	emit themeLoaded();
}

QColor Themer::get_color(const QString& name) const
{
	if (m_colors.contains(name)) {
		return m_colors.value(name);
	} else {
		return themer()->get_default_color(name);
	}
}

QFont Themer::get_font(const QString& fontname) const
{
	return m_fonts.value(fontname);
}

QVariant Themer::get_property(const QString& propertyname, const QVariant& defaultValue) const
{
	return m_properties.value(propertyname, defaultValue);
}

QCursor Themer::get_cursor(const QString & name) const
{
	return m_cursors.value(name);
}

void Themer::reload_on_themefile_change(const QString&)
{
	m_colors.clear();
	m_fonts.clear();
	m_properties.clear();
	load();
}

void Themer::set_path_and_theme(const QString& path, const QString& theme)
{
	if (m_currentTheme == theme) {
		return;
	}
	m_currentTheme = theme;
	m_themefile = path + "/" + theme + "/traversotheme.xml";
	reload_on_themefile_change("");
}

void Themer::set_color_adjust_value(int value)
{
	m_coloradjust = value;
	reload_on_themefile_change("");
}

QStringList Themer::get_builtin_themes()
{
	QStringList list;
	list << "system-palette" << "medium-contrast" << "ubuntu" << "TraversoLight";
	return list;
}

void Themer::use_builtin_theme(const QString & theme)
{
	if (m_currentTheme == theme) {
		return;
	}
	
	m_currentTheme = theme;
	m_themefile = QString(":/themes/") + theme + "/traversotheme.xml";;
	reload_on_themefile_change("");
}

QColor Themer::get_default_color(const QString & name)
{
	QPalette p = QApplication::style()->standardPalette();
	QColor c = Qt::black;

		if (name == "Text:light") c = p.color(QPalette::BrightText);
		if (name == "Text:dark")  c = p.color(QPalette::Text);
		
		if (name == "AudioClip:wavemacroview:outline") c = p.color(QPalette::WindowText);
		if (name == "AudioClip:wavemacroview:outline:curvemode") c = p.color(QPalette::WindowText);
		if (name == "AudioClip:wavemacroview:outline:muted") c = p.color(QPalette::WindowText);
		if (name == "AudioClip:wavemacroview:brush:top") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:wavemacroview:brush:bottom") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:wavemacroview:brush:hover:top") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:wavemacroview:brush:hover:bottom") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:wavemacroview:brush:muted:top") c = p.color(QPalette::Base);
		if (name == "AudioClip:wavemacroview:brush:muted:bottom") c = p.color(QPalette::Base);
		if (name == "AudioClip:wavemacroview:brush:curvemode:top") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:wavemacroview:brush:curvemode:bottom") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:wavemacroview:brush:curvemode:hover:top") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:wavemacroview:brush:curvemode:hover:bottom") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:wavemicroview") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:wavemicroview:curvemode") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:background:muted:top") c = p.color(QPalette::Base);
		if (name == "AudioClip:background:muted:bottom") c = p.color(QPalette::Base);
		if (name == "AudioClip:background:recording:top") c = p.color(QPalette::Base);
		if (name == "AudioClip:background:recording:bottom") c = p.color(QPalette::Base);
		if (name == "AudioClip:background:muted:mousehover:top") c = p.color(QPalette::Base);
		if (name == "AudioClip:background:muted:mousehover:bottom") c = p.color(QPalette::Base);
		if (name == "AudioClip:background:selected:top") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:background:selected:bottom") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:background:selected:mousehover:top") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:background:selected:mousehover:bottom") c = p.color(QPalette::Highlight);
		if (name == "AudioClip:background:top") c = p.color(QPalette::Base);
		if (name == "AudioClip:background:bottom") c = p.color(QPalette::Base);
		if (name == "AudioClip:background:mousehover:top") c = p.color(QPalette::Base);
		if (name == "AudioClip:background:mousehover:bottom") c = p.color(QPalette::Base);
		if (name == "AudioClip:channelseperator") c = p.color(QPalette::WindowText);
		if (name == "AudioClip:channelseperator:selected") c = p.color(QPalette::WindowText);
		if (name == "AudioClip:contour") c = p.color(QPalette::WindowText);
		if (name == "AudioClip:clipinfobackground") c = p.color(QPalette::AlternateBase);
		if (name == "AudioClip:clipinfobackground:inactive") c = p.color(QPalette::AlternateBase);
		if (name == "AudioClip:sampleoverload") c = Qt::red;
		if (name == "AudioClip:invalidreadsource") c = Qt::red;

		if (name == "Curve:active") c = p.color(QPalette::BrightText);
		if (name == "Curve:inactive") c = p.color(QPalette::BrightText);
		
		if (name == "CurveNode:default") c = p.color(QPalette::BrightText);
		if (name == "CurveNode:blink") c = p.color(QPalette::BrightText);
		
		if (name == "Fade:default") {
			c = p.color(QPalette::Highlight);
			c.setAlpha(150);
		}
		if (name == "Fade:bypassed") {
			c = p.color(QPalette::Highlight);
			c.setAlpha(50);
		}

		if (name == "CorrelationMeter:margin") c = p.color(QPalette::Window);
		if (name == "CorrelationMeter:background") c = p.color(QPalette::Base);
		if (name == "CorrelationMeter:grid") c = p.color(QPalette::Dark);
		if (name == "CorrelationMeter:foreground:center") c = p.color(QPalette::Link);
		if (name == "CorrelationMeter:foreground:side") c = p.color(QPalette::LinkVisited);
		if (name == "CorrelationMeter:centerline") c = p.color(QPalette::Highlight);
		if (name == "CorrelationMeter:text") c = p.color(QPalette::WindowText);
		
		if (name == "FFTMeter:margin") c = p.color(QPalette::Window);
		if (name == "FFTMeter:background") c = p.color(QPalette::Base);
		if (name == "FFTMeter:grid") c = p.color(QPalette::Dark);
		if (name == "FFTMeter:foreground") c = p.color(QPalette::Link);
		if (name == "FFTMeter:curve:average") c = p.color(QPalette::LinkVisited);
		if (name == "FFTMeter:tickmarks:main") c = p.color(QPalette::Dark);
		if (name == "FFTMeter:tickmarks:sub") c = p.color(QPalette::Mid);
		if (name == "FFTMeter:text") c = p.color(QPalette::WindowText);
		
		if (name == "VUMeter:background:widget") c = p.color(QPalette::Window);
		if (name == "VUMeter:background:bar") c = p.color(QPalette::Base);
		if (name == "VUMeter:foreground:6db") c = Qt::red;
		if (name == "VUMeter:foreground:0db") c = Qt::yellow;
		if (name == "VUMeter:foreground:-6db") c = Qt::green;
		if (name == "VUMeter:foreground:-60db") c = Qt::blue;
		if (name == "VUMeter:font:active") c = p.color(QPalette::WindowText);
		if (name == "VUMeter:font:inactive") c = p.color(QPalette::WindowText);
		if (name == "VUMeter:overled:active") c = Qt::red;
		if (name == "VUMeter:overled:inactive") c = p.color(QPalette::Base);
		
		if (name == "InfoWidget:background") c = p.color(QPalette::Window);

		if (name == "Playhead:active") c = Qt::red;
		if (name == "Playhead:inactive") c = p.color(QPalette::Mid);
		
		if (name == "Plugin:background") c = p.color(QPalette::Button);
		if (name == "Plugin:background:bypassed") c = p.color(QPalette::Light);
		if (name == "Plugin:text") c = p.color(QPalette::WindowText);
		if (name == "PluginSlider:background") c = p.color(QPalette::Mid);
		if (name == "PluginSlider:value") c = p.color(QPalette::Highlight);
		
		if (name == "ResourcesBin:alternaterowcolor") c = p.color(QPalette::AlternateBase);

		if (name == "Song:background") c = p.color(QPalette::Base);
		if (name == "SongPanel:background") c = p.color(QPalette::Window);
		
		if (name == "Timeline:background") c = p.color(QPalette::Window);
		if (name == "Timeline:text") c = p.color(QPalette::WindowText);
		
		if (name == "Track:cliptopoffset") c = p.color(QPalette::Dark);
		if (name == "Track:clipbottomoffset") c = p.color(QPalette::Dark);
		if (name == "Track:background") c = p.color(QPalette::Base);
		
		if (name == "TrackPanel:background") c = p.color(QPalette::Window);
		if (name == "TrackPanel:text") c = p.color(QPalette::WindowText);
		if (name == "TrackPanel:slider:background") c = p.color(QPalette::Button);
		if (name == "TrackPanel:head:active") c = p.color(QPalette::Highlight);
		if (name == "TrackPanel:head:inactive") c = p.color(QPalette::Highlight);
		if (name == "TrackPanel:muteled") c = Qt::yellow;
		if (name == "TrackPanel:sololed") c = Qt::green;
		if (name == "TrackPanel:recled") c = Qt::red;
		if (name == "TrackPanel:led:inactive") c = p.color(QPalette::Button);
		if (name == "TrackPanel:trackseparation") c = p.color(QPalette::WindowText);
		if (name == "TrackPanel:led:margin:active") c = p.color(QPalette::Dark);
		if (name == "TrackPanel:led:margin:inactive") c = p.color(QPalette::Dark);
		if (name == "TrackPanel:led:font:active") c = p.color(QPalette::WindowText);
		if (name == "TrackPanel:led:font:inactive") c = p.color(QPalette::WindowText);
		
		if (name == "TrackPanel:bus:font") c = p.color(QPalette::WindowText);
		if (name == "TrackPanel:bus:background") c = p.color(QPalette::Button);
		if (name == "TrackPanel:bus:margin") c = p.color(QPalette::Dark);

		if (name == "Workcursor:default") c = p.color(QPalette::WindowText);
		
		if (name == "Marker:default") c = Qt::red;
		if (name == "Marker:blink") c = p.color(QPalette::Highlight);
		if (name == "Marker:end") c = Qt::blue;
		if (name == "Marker:blinkend") c = p.color(QPalette::Highlight);

	return c;
}

