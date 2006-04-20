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
 
    $Id: ColorManager.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include "ColorManager.h"

#include <QDir>
#include <QTextStream>
#include <QDomDocument>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


ColorManager& cm()
{
        static ColorManager colorManager;
        return colorManager;
}


ColorManager::ColorManager()
{
        load();
}


QColor ColorManager::get
        (QByteArray color)
{
        /*	if (blur[colorsIndex])
        		{
        		QColor x;
        		QColor c = colors.value(colorsIndex);
        		int r = c.red();
        		int g = c.green();
        		int b = c.blue();
        		float fr = (float) r / 255;
        		float fg = (float) g / 255;
        		float fb = (float) b / 255;
        		int dr = (int)(50*fr);
        		int dg = (int)(50*fg);
        		int db = (int)(50*fb);
        		r=90+dr;
        		g=90+dg;
        		b=120+db;
        		x.setRgb(r,g,b);
        		return x;
        		}
        	else*/

        return colors.value(color);
}

void ColorManager::set_blur(int , bool )
{
        // 	blur[index]=stat;
}

void ColorManager::save( )
{
        QDomDocument doc("TraversoTheming");
        QString fileName = QDir::homePath();
        fileName +=  "/.traverso/themes/default.xml";
        QFile data( fileName );

        if (data.open( QIODevice::WriteOnly ) ) {
                QDomElement colorManagerNode = doc.createElement("ColorManager");
                doc.appendChild(colorManagerNode);

                QHash<QByteArray, QColor>::ConstIterator it = colors.begin();
                while (it != colors.end()) {
                        QColor color = it.value();
                        QDomElement colorProperty = doc.createElement("color");
                        colorProperty.setAttribute("red", color.red());
                        colorProperty.setAttribute("green", color.green());
                        colorProperty.setAttribute("blue", color.blue());
                        colorProperty.setAttribute("alpha", color.alpha() );
                        colorProperty.setAttribute("name", it.key().data() );
                        ++it;
                        colorManagerNode.appendChild(colorProperty);
                }

                QTextStream stream(&data);
                doc.save(stream, 4);
                data.close();
        } else {
                PWARN("Could not open ColorManager properties file for writing! (%s)", fileName.toAscii().data());
        }
}

void ColorManager::load( )
{
        QDomDocument doc("TraversoTheming");
        // 	QFile file(QDir::homePath() + "/.traverso/themes/default.xml");
        QFile file(":/colortheme");

        if (!file.open(QIODevice::ReadOnly)) {
                PWARN("Cannot open ColorManager properties file");
        }

        QString errorMsg;
        if (!doc.setContent(&file, &errorMsg)) {
                file.close();
                PWARN("Cannot set Content of XML file (%s)", errorMsg.toAscii().data());
        }

        file.close();

        QDomElement docElem = doc.documentElement();

        QDomNode colorsNode = docElem.firstChildElement("color");

        while (!colorsNode.isNull()) {
                QDomElement e = colorsNode.toElement();
                QColor qColor(e.attribute("red").toUInt(), e.attribute("green").toUInt() , e.attribute("blue").toUInt(), e.attribute("alpha").toInt());
                colors.insert(e.attribute("name").toAscii(), qColor);
                colorsNode = colorsNode.nextSibling();
        }
}

//eof
