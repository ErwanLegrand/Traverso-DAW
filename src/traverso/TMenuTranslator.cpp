/*
Copyright (C) 2010 Remon Sijrier

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

*/

#include "TMenuTranslator.h"

#include "libtraversocore.h"
#include "libtraversosheetcanvas.h"
#include "commands.h"
#include "TMainWindow.h"
#include "InputEngine.h"
#include "TShortcutManager.h"
#include <QMenu>
#include <QPalette>

TMenuTranslator* TMenuTranslator::m_instance = 0;

TMenuTranslator* TMenuTranslator::instance()
{
	if (m_instance == 0) {
		m_instance = new TMenuTranslator();
	}

	return m_instance;
}

void TMenuTranslator::add_meta_object(const QMetaObject* mo)
{
	QString classname = QString(mo->className()).remove("View");
	QList<const QMetaObject*> list = m_objects.value(classname);
	list.append(mo);
	m_objects.insert(classname, list);
}

QList<const QMetaObject*> TMenuTranslator::get_metaobjects_for_class(const QString &className)
{
	return m_objects.value(className);
}

TMenuTranslator::TMenuTranslator()
{
	add_meta_object(&Sheet::staticMetaObject);
	add_meta_object(&SheetView::staticMetaObject);
	add_meta_object(&AudioTrack::staticMetaObject);
	add_meta_object(&AudioTrackView::staticMetaObject);
	add_meta_object(&TBusTrack::staticMetaObject);
	add_meta_object(&TBusTrackView::staticMetaObject);
	add_meta_object(&AudioClip::staticMetaObject);
	add_meta_object(&AudioClipView::staticMetaObject);
	add_meta_object(&Curve::staticMetaObject);
	add_meta_object(&CurveView::staticMetaObject);
	add_meta_object(&TimeLine::staticMetaObject);
	add_meta_object(&TimeLineView::staticMetaObject);
	add_meta_object(&Plugin::staticMetaObject);
	add_meta_object(&PluginView::staticMetaObject);
	add_meta_object(&FadeCurve::staticMetaObject);
	add_meta_object(&FadeCurveView::staticMetaObject);
	add_meta_object(&TMainWindow::staticMetaObject);
	add_meta_object(&Project::staticMetaObject);
	add_meta_object(&ProjectManager::staticMetaObject);
	add_meta_object(&Gain::staticMetaObject);
	add_meta_object(&MoveTrack::staticMetaObject);
	add_meta_object(&MoveClip::staticMetaObject);
	add_meta_object(&MoveCurveNode::staticMetaObject);
	add_meta_object(&Zoom::staticMetaObject);
	add_meta_object(&TrackPan::staticMetaObject);
	add_meta_object(&MoveMarker::staticMetaObject);
	add_meta_object(&WorkCursorMove::staticMetaObject);
	add_meta_object(&PlayHeadMove::staticMetaObject);
	add_meta_object(&MoveEdge::staticMetaObject);
	add_meta_object(&CropClip::staticMetaObject);
	add_meta_object(&FadeRange::staticMetaObject);
	add_meta_object(&Shuttle::staticMetaObject);
	add_meta_object(&SplitClip::staticMetaObject);
	add_meta_object(&TPanKnobView::staticMetaObject);

	add_entry("ArrowKeyBrowser", tr("Arrow Key Browser"));
	add_entry("ArmTracks", tr("Arm Tracks"));
	add_entry("CropClip", tr("Cut Clip (Magnetic)"));
	add_entry("Fade", tr("Fade"));
	add_entry("Gain", tr("Gain"));
	add_entry("MoveClip", tr("Move Clip"));
	add_entry("MoveCurveNode", tr("Move Node"));
	add_entry("MoveEdge", tr("Move Clip Edge"));
	add_entry("MoveMarker", tr("Move Marker"));
	add_entry("MoveTrack", tr("Move Track"));
	add_entry("PlayHeadMove", tr("Move Play Head"));
	add_entry("Shuttle", tr("Shuttle"));
	add_entry("SplitClip", tr("Split Clip"));
	add_entry("TrackPan", tr("Track Panorama"));
	add_entry("TPanKnob", tr("Pan Knob"));
	add_entry("TPanKnobView", tr("Pan Knob"));

	add_entry("WorkCursorMove", tr("Move Work Cursor"));
	add_entry("Zoom", tr("Zoom"));

	add_entry("AudioClip",tr("Audio Clip"));
	add_entry("AudioTrack", tr("Audio Track"));
	add_entry("Curve",tr("Curve"));
	add_entry("CurveNode",tr("Curve Node"));
	add_entry("FadeCurve",tr("Fade Curve"));
	add_entry("FadeRange", tr("Fade Length"));
	add_entry("FadeBend", tr("Bend Factor"));
	add_entry("FadeStrength", tr("Strength Factor"));
	add_entry("Marker",tr("Marker"));
	add_entry("Sheet",tr("Sheet"));
	add_entry("TBusTrack",tr("Bus Track"));
	add_entry("TimeLine",tr("Time Line"));
	add_entry("TBusTrackPanel", tr("Bus Track"));
	add_entry("TMainWindow", tr("Main Window"));
	add_entry("Project", tr("Project"));
	add_entry("ProjectManager", tr("Project Manager"));
	add_entry("TrackPanelGain", tr("Gain"));
	add_entry("TrackPanelPan", tr("Panorama"));
	add_entry("TrackPanelLed", tr("Track Panel Button"));
	add_entry("TrackPanelBus", tr("Routing Indicator"));
	add_entry("VUMeterLevel", tr("VU Level"));
	add_entry("VUMeter", tr("VU Level"));
	add_entry("AudioTrackPanel",tr("Audio Track"));
	add_entry("Plugin",tr("Plugin"));
	add_entry("PlayHead", tr("Play Head"));
	add_entry("PositionIndicator", tr("Position Indicator"));
	add_entry("WorkCursor", tr("Work Cursor"));
	add_entry("CorrelationMeter", tr("Correlation Meter"));
	add_entry("SpectralMeter", tr("Spectral Analyzer"));
}

void TMenuTranslator::add_entry(const QString &signature, const QString &translation)
{
	m_dict.insert(signature, translation);
}


QString TMenuTranslator::get_translation_for(const QString &entry)
{
	QString key = entry;
	key = key.remove("View");
	if (!m_dict.contains(key)) {
		return QString("TMenuTranslator: %1 not found!").arg(key);
	}
	return m_dict.value(key);
}

QString TMenuTranslator::createHtmlForMetaObects(QList<const QMetaObject *> metas, QObject* object)
{
	if (!metas.size()) {
		return "";
	}


	// FIXME ?
	QString holdKeyFact = "";// ie().keyfacts_for_hold_command(metas.first()->className()).join(" , ");


	QString name = get_translation_for(QString(metas.first()->className()));

	QColor bgcolor = themer()->get_color("ResourcesBin:alternaterowcolor");
	QString html = QString("<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
	      "<style type=\"text/css\">\n"
	      "table {font-size: 11px;}\n"
	      ".object {background-color: %1; font-size: 11px;}\n"
	      ".description {background-color: %2; font-size: 11px; font-weight: bold;}\n"
	      "</style>\n"
	      "</head>\n<body>\n").arg(bgcolor.darker(105).name()).arg(bgcolor.darker(103).name());

	if (object && object->inherits("PCommand")) {
		PCommand* pc = static_cast<PCommand*>(object);
		html += "<table><tr class=\"object\">\n<td width=220 align=\"center\">" + pc->text() + "</td></tr>\n";
	} else {
		html += "<table><tr class=\"object\">\n<td colspan=\"2\" align=\"center\"><b>" + name + "</b><font style=\"font-size: 11px;\">&nbsp;&nbsp;&nbsp;" + holdKeyFact + "</font></td></tr>\n";
		html += "<tr><td width=110 class=\"description\">" +tr("Description") + "</td><td width=110 class=\"description\">" + tr("Shortcut") + "</td></tr>\n";
	}

	QStringList result;
	int j=0;
	QString submenuhtml;
	QList<QMenu* > menulist;
	QList<TFunction* > list;

	foreach(const QMetaObject* mo, metas) {
		while (mo) {
			list << tShortCutManager().getFunctionsForMetaobject(mo);
			mo = mo->superClass();
		}
	}

	QMenu* menu = TMainWindow::instance()->create_context_menu(0, &list);
	if (menu) {
		menulist.append(menu);
		foreach(QAction* action, menu->actions()) {
			if (action->menu()) {
				menulist.append(action->menu());
			}
		}
	}

	QStringList submenushtml;

	for (int i=0; i<menulist.size(); ++i) {
		QMenu* somemenu = menulist.at(i);
		submenuhtml = "";
		if (i>0) {
			submenuhtml = "<tr class=\"object\">\n<td colspan=\"2\" align=\"center\">"
				      "<font style=\"font-size: 11px;\"><b>" + somemenu->menuAction()->text() + "</b></font></td></tr>\n";
		}
		foreach(QAction* action, somemenu->actions()) {
			QStringList strings = action->data().toStringList();
			if (strings.size() >= 2) {
				QString keyfact = strings.at(0);

				keyfact.replace(QString("Up Arrow"), QString("&uarr;"));
				keyfact.replace(QString("Down Arrow"), QString("&darr;"));
				keyfact.replace(QString("Left Arrow"), QString("&larr;"));
				keyfact.replace(QString("Right Arrow"), QString("&rarr;"));
				keyfact.replace(QString("-"), QString("&#45;"));
				keyfact.replace(QString("+"), QString("&#43;"));
				keyfact.replace(QString(" , "), QString("<br />"));


				QString alternatingColor;
				if ((j % 2) == 1) {
					alternatingColor = QString("bgcolor=\"%1\"").arg(themer()->get_color("ResourcesBin:alternaterowcolor").name());
				} else {
					alternatingColor = QString("bgcolor=\"%1\"").arg(TMainWindow::instance()->palette().color(QPalette::Base).name());
				}
				j += 1;

				if (i>0) {
					submenuhtml += QString("<tr %1><td>").arg(alternatingColor) + strings.at(1) + "</td><td>" + keyfact + "</td></tr>\n";
				} else {
					result += QString("<tr %1><td>").arg(alternatingColor) + strings.at(1) + "</td><td>" + keyfact + "</td></tr>\n";
				}
			}
		}
		if (!submenuhtml.isEmpty()) {
			submenushtml.append(submenuhtml);
		}
	}

	foreach(QString html, submenushtml) {
		result += html;
	}

	foreach(QMenu* menu, menulist) {
		delete menu;
	}

	result.removeDuplicates();
	html += result.join("");
	html += "</table>\n";
	html += "</body>\n</html>";

	return html;
}

bool TMenuTranslator::classInherits(const QString& className, const QString &inherited)
{
	QList<const QMetaObject*> metas = TMenuTranslator::instance()->get_metaobjects_for_class(className);

	foreach(const QMetaObject* mo, metas) {
		while (mo) {
			if (mo->className() == inherited)
			{
				return true;
			}
			mo = mo->superClass();
		}
	}

	return false;
}
