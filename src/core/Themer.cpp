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

$Id: Themer.cpp,v 1.2 2007/04/25 12:43:32 r_sijrier Exp $
*/

#include "Themer.h"

#include <Utils.h>
#include <Config.h>
#include <QDir>
#include <QTextStream>
#include <QDomDocument>
#include <QApplication>
#include <QStyle>


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
	if (themepath.isEmpty()) {
		themepath = ":/themes";
	}
	m_currentTheme = config().get_property("Themer", "currenttheme", "TraversoLight").toString();
	
	if (get_builtin_themes().contains(m_currentTheme)) {
		themepath = ":/themes";
	}

	m_themefile =  themepath + "/" + m_currentTheme + "/traversotheme.xml";
	m_coloradjust = -1;
	
	m_systempallete = QApplication::palette();
	
	QString style = config().get_property("Themer", "style", "").toString();
	QString currentStyle = QString(QApplication::style()->metaObject()->className()).remove("Q").remove("Style");
	if (style != currentStyle) {
		QApplication::setStyle(style);
	}
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
	 	m_watcher->addPath(m_themefile);
		printf("Using themefile: %s\n", QS_C(m_themefile));
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
	
	while (!fontNode.isNull()) {
		
		QDomElement e = fontNode.toElement();
		
		QString family = e.attribute("family", "");
		int size = e.attribute("size", "").toInt();
		QString name = e.attribute("name", "");
		QFont font(family, size);
		
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
	return m_colors.value(name);
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
	list << "TraversoLight";
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

//eof
