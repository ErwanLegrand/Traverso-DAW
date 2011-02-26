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

$Id: Themer.cpp,v 1.14 2009/04/16 19:38:18 n_doebelin Exp $
*/

#include "Themer.h"

#include <Utils.h>
#include "TConfig.h"
#include <QDir>
#include <QTextStream>
#include <QDomDocument>
#include <QApplication>
#include <QStyle>
#include <QFileSystemWatcher>
#include <QDebug>


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

	// FIXME: undo change till all the other themes are updated.
	m_currentTheme = "Parchment";//config().get_property("Themer", "currenttheme", "").toString();
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
                        m_currentTheme = "Traverso Light";
		} else {
                        m_currentTheme = "Traverso Light";
		}
		config().set_property("Themer", "currenttheme", m_currentTheme);
	}
	
	if (get_builtin_themes().contains(m_currentTheme)) {
		themepath = ":/themes";
	}
	
        m_themefile =  themepath + "/" + m_currentTheme;
	m_coloradjust = -1;
	
	bool usestylepallete = config().get_property("Themer", "usestylepallet", "false").toBool();
	
	if (usestylepallete) {
		QApplication::setPalette(QApplication::style()->standardPalette());
        }

        load_defaults();
	
	connect(m_watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(reload_on_themefile_change(const QString&)));
}


void Themer::save( )
{
	QDomDocument doc("TraversoTheming");
	QString fileName = QDir::homePath();
        fileName +=  "/.traverso/themes/editedtheme.xml";
	QFile data( fileName );

        m_watcher->removePath(fileName);

        if (!data.open( QIODevice::WriteOnly ) ) {
                PWARN("Could not open Themer properties file for writing! (%s)", QS_C(fileName));
                return;
        }

        QDomElement themerNode = doc.createElement("Themer");
        doc.appendChild(themerNode);

        QDomElement properties = doc.createElement("properties");
        QHash<QString, QVariant>::ConstIterator propertiesIt = m_properties.begin();
        while (propertiesIt != m_properties.end()) {
                QDomElement e = doc.createElement("property");
                e.setAttribute("name", propertiesIt.key());
                e.setAttribute("value", propertiesIt.value().toString());
                properties.appendChild(e);
                ++propertiesIt;
        }

        themerNode.appendChild(properties);

        QFont basefont = QApplication::font();

        QDomElement fonts = doc.createElement("fonts");
        QHash<QString, QFont>::ConstIterator fontsIt = m_fonts.begin();
        while (fontsIt != m_fonts.end()) {
                QDomElement e = doc.createElement("font");
                e.setAttribute("name", fontsIt.key());
                QFont font = fontsIt.value();
                e.setAttribute("value", font.pointSizeF() / basefont.pointSizeF());
                fonts.appendChild(e);
                ++fontsIt;
        }

        themerNode.appendChild(fonts);


        QDomElement colors = doc.createElement("colors");
        themerNode.appendChild(colors);

        QHash<QString, QColor>::ConstIterator it = m_colors.begin();
        while (it != m_colors.end()) {
                QColor color = it.value();
                QDomElement colorProperty = doc.createElement("color");
                colorProperty.setAttribute("red", color.red());
                colorProperty.setAttribute("green", color.green());
                colorProperty.setAttribute("blue", color.blue());
                colorProperty.setAttribute("alpha", color.alpha() );
                colorProperty.setAttribute("name", it.key() );
                ++it;
                colors.appendChild(colorProperty);
        }


        QDomElement gradients = doc.createElement("gradients");
        QHash<QString, QLinearGradient>::ConstIterator gradientsIt = m_gradients.begin();
        while(gradientsIt != m_gradients.end()) {
                QDomElement e = doc.createElement("gradient");
                e.setAttribute("name", gradientsIt.key());
                QLinearGradient gradient = gradientsIt.value();
                foreach(QGradientStop gradientstop, gradient.stops()) {
                        QDomElement stopNode = doc.createElement("stop");
                        stopNode.setAttribute("value", gradientstop.first);
                        QColor color = gradientstop.second;
                        stopNode.setAttribute("red", color.red());
                        stopNode.setAttribute("green", color.green());
                        stopNode.setAttribute("blue", color.blue());
                        stopNode.setAttribute("alpha", color.alpha() );
                        e.appendChild(stopNode);
                }
                gradients.appendChild(e);
                ++gradientsIt;
        }

        themerNode.appendChild(gradients);

        QTextStream stream(&data);
        doc.save(stream, 4);
        data.close();

        m_watcher->addPath(fileName);
}

void Themer::load( )
{
	QDomDocument doc("TraversoTheming");
 	
 	QFile file(m_themefile);
 	
 	if ( ! file.exists() ) {
                m_themefile.append(".xml");
                file.setFileName(m_themefile);
                if (!file.exists()) {
                        printf("File %s doesn't exit, falling back to default (Traverso Light) theme\n", QS_C(m_themefile));
                        m_themefile = ":/themes/Traverso Light";
                        config().set_property("Themer", "currenttheme", "Traverso Light");
                        file.setFileName(m_themefile);
                }
        }

        if (!m_themefile.contains(":")) {
                m_watcher->addPath(m_themefile);
        }
        printf("Themer:: Using themefile: %s\n", QS_C(m_themefile));

	if (!file.open(QIODevice::ReadOnly)) {
                printf("Cannot open Themer properties file\n");
                return;
	}

	QString errorMsg;
	if (!doc.setContent(&file, &errorMsg)) {
		file.close();
                printf("Cannot set Content of XML file (%s)\n", QS_C(errorMsg));
                return;
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
		
	QDomNode gradientsNode = docElem.firstChildElement("gradients");
	QDomNode gradientNode = gradientsNode.firstChild();
	
	while(!gradientNode.isNull()) {
		QLinearGradient gradient;
	
		QDomElement e = gradientNode.toElement();	
		QString name = e.attribute("name", "");

		QDomNode gradientStopNode = gradientNode.firstChild();

		while(!gradientStopNode.isNull()) {
			QDomElement ee = gradientStopNode.toElement();	
			QColor color(
				ee.attribute("red").toUInt(),
				ee.attribute("green").toUInt(), 
				ee.attribute("blue").toUInt(), 
				ee.attribute("alpha").toInt()				
			);
			
			if (coloradjust != 100) {
				int adjust = coloradjust - 100;	
				if (adjust < 0) {
					color = color.dark(-1 * adjust + 100);
				} else {
					color = color.light(adjust + 100);
				}
			}

			float value = ee.attribute("value", "0.0").toFloat();
			
			gradient.setColorAt(value, color);
			gradientStopNode = gradientStopNode.nextSibling();
		}
		
		m_gradients.insert(name, gradient);
		gradientNode = gradientNode.nextSibling();
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
	m_cursors.insert("ZoomVertical", QCursor(find_pixmap(":/cursorZoomVertical")));
	m_cursors.insert("ZoomHorizontal", QCursor(find_pixmap(":/cursorZoomHorizontal")));
	m_cursors.insert("CurveNode", QCursor(find_pixmap(":/cursorDragNode")));
	
	m_cursors.insert("LRUD", QCursor(find_pixmap(":/cursorHoldLrud")));
	m_cursors.insert("LR", QCursor(find_pixmap(":/cursorHoldLr")));
        m_cursors.insert("UD", QCursor(find_pixmap(":/cursorHoldUd")));

        validate_loaded_theme();

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

// Returns the brush with the name "name". QPoints "start" and "end" are the
// start and finalStop positions of linear gradients. If a solid colour is 
// returned, both points are ignored.
// If a gradient and a colour with the same name exists, colour brush will be
// returned.
QBrush Themer::get_brush(const QString& name, QPoint start, QPoint stop) const
{

        // check if there is a gradient with that name
	if (m_gradients.contains(name))
	{
		QLinearGradient gradient = m_gradients.value(name);

		// use a solid colour if the gradient only contains one stop point.
		// saves a huge amount of resources in the painting routines.
		if (gradient.stops().size() == 1) {
			return QBrush(gradient.stops().at(0).second);
		}

		gradient.setStart(start);
		gradient.setFinalStop(stop);
		gradient.setSpread(QGradient::ReflectSpread);
	
		return QBrush(gradient);
	}
	
        // if there is no gradient, check if there is a solid colour with that name
        if (m_colors.contains(name))
        {
                return QBrush(m_colors.value(name));
        }

        // not a colour either? return a fallback colour.
	printf("Brush %s was requested, but no such element was found in the theme file\n", QS_C(name));
	return QBrush(themer()->get_default_color(name));
}

// sometimes we need access to the gradient (e.g. if the start and finalStops have to be
// modified
QLinearGradient Themer::get_gradient(const QString& name) const
{
	if (m_gradients.contains(name))
	{
		return m_gradients.value(name);
	} else {
		printf("Gradient %s was requested, but no such element was found in the theme file\n", QS_C(name));
		return QLinearGradient();
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
        m_themefile = path + "/" + theme;
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
        QDir themesdir(":/themes");
        foreach (const QString &fileName, themesdir.entryList(QDir::Files)) {
                list << fileName;
        }
	return list;
}

void Themer::use_builtin_theme(const QString & theme)
{
	if (m_currentTheme == theme) {
		return;
	}
	
	m_currentTheme = theme;
        m_themefile = QString(":/themes/") + theme;
	reload_on_themefile_change("");
}

QColor Themer::get_default_color(const QString & name)
{
        if (!m_defaultColors.contains(name)) {
                printf("Default color %s was requested, but no such default color does exist, please add it to the themer!\n", QS_C(name));
                return QColor(Qt::blue);
        }

        return m_defaultColors.value(name);
}

void Themer::load_defaults()
{
        QPalette p = QApplication::style()->standardPalette();
        QColor c = Qt::black;
        QString name;

        m_defaultColors.insert("Text:light", p.color(QPalette::BrightText));
        m_defaultColors.insert("Text:dark", p.color(QPalette::Text));
        m_defaultColors.insert("AudioClip:wavemacroview:outline", p.color(QPalette::WindowText));
        m_defaultColors.insert("AudioClip:wavemacroview:outline:curvemode", p.color(QPalette::WindowText));
        m_defaultColors.insert("AudioClip:wavemacroview:outline:muted", p.color(QPalette::WindowText));
        m_defaultColors.insert("AudioClip:wavemacroview:brush", p.color(QPalette::Highlight));
        m_defaultColors.insert("AudioClip:wavemacroview:brush:hover", p.color(QPalette::Highlight));
        m_defaultColors.insert("AudioClip:wavemacroview:brush:muted", p.color(QPalette::Base));
        m_defaultColors.insert("AudioClip:wavemacroview:brush:curvemode", p.color(QPalette::Highlight));
        m_defaultColors.insert("AudioClip:wavemacroview:brush:curvemode:hover", p.color(QPalette::Highlight));
        m_defaultColors.insert("AudioClip:wavemicroview", p.color(QPalette::Highlight));
        m_defaultColors.insert("AudioClip:wavemicroview:curvemode", p.color(QPalette::Highlight));
        m_defaultColors.insert("AudioClip:background:muted", p.color(QPalette::Base));
        m_defaultColors.insert("AudioClip:background:recording", p.color(QPalette::Base));
        m_defaultColors.insert("AudioClip:background:muted:mousehover", p.color(QPalette::Base));
        m_defaultColors.insert("AudioClip:background:selected", p.color(QPalette::Highlight));
        m_defaultColors.insert("AudioClip:background:selected:mousehover", p.color(QPalette::Highlight));
        m_defaultColors.insert("AudioClip:background", p.color(QPalette::Base));
        m_defaultColors.insert("AudioClip:background:mousehover", p.color(QPalette::Base));
        m_defaultColors.insert("AudioClip:channelseperator", p.color(QPalette::WindowText));
        m_defaultColors.insert("AudioClip:channelseperator:selected", p.color(QPalette::WindowText));
        m_defaultColors.insert("AudioClip:contour", p.color(QPalette::WindowText));
        m_defaultColors.insert("AudioClip:clipinfobackground", p.color(QPalette::AlternateBase));
        m_defaultColors.insert("AudioClip:clipinfobackground:inactive", p.color(QPalette::AlternateBase));
        m_defaultColors.insert("AudioClip:sampleoverload", QColor(Qt::red));
        m_defaultColors.insert("AudioClip:invalidreadsource", QColor(Qt::red));
        m_defaultColors.insert("Curve:active", p.color(QPalette::BrightText));
        m_defaultColors.insert("CurveNode:default", p.color(QPalette::BrightText));
        m_defaultColors.insert("CurveNode:blink", p.color(QPalette::BrightText));
        c = p.color(QPalette::Highlight);
        c.setAlpha(150);
        m_defaultColors.insert("Fade:default", c);
        c = p.color(QPalette::Highlight);
        c.setAlpha(50);
        m_defaultColors.insert("Fade:bypassed", c);
        m_defaultColors.insert("CorrelationMeter:margin", p.color(QPalette::Window));
        m_defaultColors.insert("CorrelationMeter:background", p.color(QPalette::Base));
        m_defaultColors.insert("CorrelationMeter:grid", p.color(QPalette::Dark));
        m_defaultColors.insert("CorrelationMeter:foreground:center", p.color(QPalette::Link));
        m_defaultColors.insert("CorrelationMeter:foreground:side", p.color(QPalette::LinkVisited));
        m_defaultColors.insert("CorrelationMeter:centerline", p.color(QPalette::Highlight));
        m_defaultColors.insert("CorrelationMeter:text", p.color(QPalette::WindowText));
        m_defaultColors.insert("GainSlider:6db", QColor(Qt::red));
        m_defaultColors.insert("GainSlider:0db", QColor(Qt::yellow));
        m_defaultColors.insert("GainSlider:-6db", QColor(Qt::green));
        m_defaultColors.insert("GainSlider:-60db", QColor(Qt::blue));
        m_defaultColors.insert("FFTMeter:margin", p.color(QPalette::Window));
        m_defaultColors.insert("FFTMeter:background", p.color(QPalette::Base));
        m_defaultColors.insert("FFTMeter:grid", p.color(QPalette::Dark));
        m_defaultColors.insert("FFTMeter:foreground", p.color(QPalette::Link));
        m_defaultColors.insert("FFTMeter:curve:average", p.color(QPalette::LinkVisited));
        m_defaultColors.insert("FFTMeter:tickmarks:main", p.color(QPalette::Dark));
        m_defaultColors.insert("FFTMeter:tickmarks:sub", p.color(QPalette::Mid));
        m_defaultColors.insert("FFTMeter:text", p.color(QPalette::WindowText));
        m_defaultColors.insert("VUMeter:background:widget", p.color(QPalette::Window));
        m_defaultColors.insert("VUMeter:background:bar", p.color(QPalette::Base));
        m_defaultColors.insert("VUMeter:foreground:6db", QColor(Qt::red));
        m_defaultColors.insert("VUMeter:foreground:0db", QColor(Qt::yellow));
        m_defaultColors.insert("VUMeter:foreground:-6db", QColor(Qt::green));
        m_defaultColors.insert("VUMeter:foreground:-60db", QColor(Qt::blue));
        m_defaultColors.insert("VUMeter:font:active", p.color(QPalette::WindowText));
        m_defaultColors.insert("VUMeter:font:inactive", p.color(QPalette::WindowText));
        m_defaultColors.insert("VUMeter:overled:active", QColor(Qt::red));
        m_defaultColors.insert("VUMeter:overled:inactive", p.color(QPalette::Base));
        m_defaultColors.insert("VUMeter:levelseparator", p.color(QPalette::Mid));
        m_defaultColors.insert("InfoWidget:background", p.color(QPalette::Window));
        m_defaultColors.insert("PanSlider:-1", QColor(Qt::red));
        m_defaultColors.insert("PanSlider:0", p.color(QPalette::Base));
        m_defaultColors.insert("PanSlider:1", QColor(Qt::red));
        m_defaultColors.insert("Playhead:active", QColor(255, 0, 0, 180));
        m_defaultColors.insert("Playhead:inactive", QColor(255, 0, 0, 120));
        m_defaultColors.insert("Plugin:background", p.color(QPalette::Button));
        m_defaultColors.insert("Plugin:background:bypassed", p.color(QPalette::Light));
        m_defaultColors.insert("Plugin:text", p.color(QPalette::WindowText));
        m_defaultColors.insert("PluginSlider:background", p.color(QPalette::Mid));
        m_defaultColors.insert("PluginSlider:value", p.color(QPalette::Highlight));
        m_defaultColors.insert("PluginSlider:text", p.color(QPalette::WindowText));
        m_defaultColors.insert("ResourcesBin:alternaterowcolor", p.color(QPalette::AlternateBase));
        m_defaultColors.insert("Sheet:background", p.color(QPalette::Base));
        m_defaultColors.insert("SheetPanel:background", p.color(QPalette::Window));
        m_defaultColors.insert("Timeline:background", p.color(QPalette::Window));
        m_defaultColors.insert("Timeline:text", p.color(QPalette::WindowText));
        m_defaultColors.insert("Track:cliptopoffset", p.color(QPalette::Dark));
        m_defaultColors.insert("Track:clipbottomoffset", p.color(QPalette::Dark));
        m_defaultColors.insert("Track:background", p.color(QPalette::Base));
        m_defaultColors.insert("Track:mousehover", p.color((QPalette::Highlight)));
        m_defaultColors.insert("TrackPanel:background", p.color(QPalette::Window));
        m_defaultColors.insert("TrackPanel:text", p.color(QPalette::WindowText));
        m_defaultColors.insert("TrackPanel:sliderborder", p.color(QPalette::WindowText));
        m_defaultColors.insert("TrackPanel:slider:background", p.color(QPalette::Button));
        m_defaultColors.insert("TrackPanel:slider:border", p.color(QPalette::Window));
        m_defaultColors.insert("TrackPanel:head:active", p.color(QPalette::Highlight));
        m_defaultColors.insert("TrackPanel:head:inactive", p.color(QPalette::Highlight));
        m_defaultColors.insert("TrackPanel:muteled", QColor(Qt::yellow));
        m_defaultColors.insert("TrackPanel:sololed", QColor(Qt::green));
        m_defaultColors.insert("TrackPanel:recled", QColor(Qt::red));
        m_defaultColors.insert("TrackPanel:led:inactive", p.color(QPalette::Button));
        m_defaultColors.insert("TrackPanel:trackseparation", p.color(QPalette::WindowText));
        m_defaultColors.insert("TrackPanel:led:margin:active", p.color(QPalette::Dark));
        m_defaultColors.insert("TrackPanel:led:margin:inactive", p.color(QPalette::Dark));
        m_defaultColors.insert("TrackPanel:led:font:active", p.color(QPalette::WindowText));
        m_defaultColors.insert("TrackPanel:led:font:inactive", p.color(QPalette::WindowText));
        m_defaultColors.insert("TrackPanel:bus:font", p.color(QPalette::WindowText));
        m_defaultColors.insert("TrackPanel:bus:background", p.color(QPalette::Button));
        m_defaultColors.insert("TrackPanel:bus:margin", p.color(QPalette::Dark));
        m_defaultColors.insert("BusTrack:background", p.color(QPalette::Background));
        m_defaultColors.insert("BusTrackPanel:background", p.color(QPalette::Background));
        m_defaultColors.insert("Workcursor:default", QColor(100, 50, 100, 180));
        m_defaultColors.insert("Marker:default", QColor(Qt::red));
        m_defaultColors.insert("Marker:blink", p.color(QPalette::Highlight));
        m_defaultColors.insert("Marker:end", QColor(Qt::blue));
        m_defaultColors.insert("Marker:blinkend", p.color(QPalette::Highlight));
}

void Themer::validate_loaded_theme()
{
        QStringList list;
        foreach(const QString& key, m_defaultColors.keys()) {
                if (!m_colors.contains(key)) {
                        QColor color = m_defaultColors.value(key);
                        // add the missing color from default colors
                        // so the theme editor will be able to save those too.
                        m_colors.insert(key, color);
                        list << tr("<color name=\"%1\"  red=\"%2\" green=\"%3\" blue=\"%4\"  alpha=\"%5\" />").
                                        arg(key).arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
                }
        }

        if (list.size()) {
                printf("\n");
                printf("Themer: the following entries are missing, please edit theme: %s\n", QS_C(m_themefile));

                foreach(QString string, list) {
                        printf("%s\n", QS_C(string));
                }
                printf("\nAnd adjust the color(s) to fit your theme, using the edit theme button in the Appearance config page!\n"
                       "The edited theme will be saved to ~/.traverso/themes/editedtheme.xml\n\n");
        }
}

QList<QString> Themer::get_colors()
{
        QList<QString> colors = m_colors.keys();
        // if the current theme misses some colors, add them here.
        foreach(QString color, m_defaultColors.keys()) {
                if (!colors.contains(color)) {
                        colors.append(color);
                }
        }

        return colors;
}

void Themer::set_new_theme_color(const QString &name, const QColor &color)
{
        m_colors.insert(name, color);
        emit themeLoaded();
}
