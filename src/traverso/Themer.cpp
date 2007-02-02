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

$Id: Themer.cpp,v 1.1 2007/02/02 00:02:24 r_sijrier Exp $
*/

#include "Themer.h"

#include <Utils.h>
#include <Config.h>
#include <QDir>
#include <QTextStream>
#include <QDomDocument>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Themer& themer()
{
	static Themer themer;
	return themer;
}


Themer::Themer()
{
 	m_themefile = config().get_property("Theme", "themepath", ":/defaulttheme").toString() + "/traversotheme.xml";
	connect(&m_watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(reload_on_themefile_change(const QString&)));
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
		printf("Falling back to default theme\n");
		file.setFileName(":/colortheme");
	} else {
	 	m_watcher.addPath(m_themefile);
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

	while(!colorNode.isNull()) {
		
		QDomElement e = colorNode.toElement();
		
		QColor color(
			e.attribute("red").toUInt(),
			e.attribute("green").toUInt(), 
			e.attribute("blue").toUInt(), 
			e.attribute("alpha").toInt()
			);
		QString name = e.attribute("name", "");
		
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


void Themer::reload_on_themefile_change(const QString&)
{
	m_colors.clear();
	m_fonts.clear();
	m_properties.clear();
	load();
}


void Themer::set_theme_path(const QString& path)
{
	m_themefile = path + "/traversotheme.xml";
	config().set_property("Theme", "themepath", path);
	reload_on_themefile_change(path);
}

