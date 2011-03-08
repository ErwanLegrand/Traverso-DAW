/*
Copyright (C) 2011 Remon Sijrier

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

#include "TShortcutManager.h"

#include "ContextItem.h"
#include "InputEngine.h"

#include "Debugger.h"


QString TShortcutData::getKeySequence()
{
	QString sequence;

	foreach(int modifier, modifierkeys) {
		if (modifier == Qt::Key_Alt) {
			sequence += "Alt+";
		} else if (modifier == Qt::Key_Control) {
			sequence += "Ctrl+";
		} else if (modifier == Qt::Key_Shift) {
			sequence += "Shift+";
		} else if (modifier == Qt::Key_Meta) {
			sequence += "Meta+";
		}
	}

	sequence += keyString;

	TShortcutManager::makeShortcutKeyHumanReadable(sequence);

	return sequence;
}

void TShortcutManager::makeShortcutKeyHumanReadable(QString& keyfact)
{
	keyfact.replace(QString("MouseScrollVerticalUp"), tr("Scroll Wheel"));
	keyfact.replace(QString("MouseScrollVerticalDown"), tr("Scroll Wheel"));
	keyfact.replace(QString("MouseButtonRight"), tr("Right MB"));
	keyfact.replace(QString("MouseButtonLeft"), tr("Left MB"));
	keyfact.replace(QString("MouseButtonMiddle"), tr("Center MB"));
	keyfact.replace(QString("UARROW"), tr("Up Arrow"));
	keyfact.replace(QString("DARROW"), tr("Down Arrow"));
	keyfact.replace(QString("LARROW"), tr("Left Arrow"));
	keyfact.replace(QString("RARROW"), tr("Right Arrow"));
	keyfact.replace(QString("DELETE"), tr("Delete"));
	keyfact.replace(QString("MINUS"), QString("-"));
	keyfact.replace(QString("PLUS"), QString("+"));
	keyfact.replace(QString("PAGEDOWN"), tr("Page Down"));
	keyfact.replace(QString("PAGEUP"), tr("Page Up"));
}


TShortcutManager& tShortCutManager()
{
	static TShortcutManager manager;
	return manager;
}

TShortcutManager::TShortcutManager()
{
	loadDefaultShortcuts();
}

void TShortcutManager::addFunctionShortcut(const QString &function, TShortcutData *data)
{
	m_dict.insert(function, data);
}

TShortcutData* TShortcutManager::getFunctionshortcut(const QString &function) const
{
	TShortcutData* data = m_dict.value(function, 0);
	if (!data)
	{
		printf("TShortcutManager::getFunctionshortcut: Function %s not in database!!\n", function.toAscii().data());
	}

	return data;
}


void TShortcutManager::getShortcutDataForMetaobject(const QMetaObject * mo, QList<TShortcutData* > & list) const
{
	const char* classname = mo->className();
	QList<IEAction*> ieActions = ie().get_ie_actions();

	for (int i=0; i<ieActions.size(); i++) {
		IEAction* ieaction = ieActions.at(i);

		QList<TShortcutData*> datalist;
		datalist.append(ieaction->objects.value(classname));
		datalist.append(ieaction->objectUsingModifierKeys.values(classname));

		foreach(TShortcutData* data, datalist)
		{
			if (!data) continue;

			if (data->slotsignature == "numerical_input") {
				data->keyString =  "0, 1, 2 ... 9";
			} else {
				data->keyString = ieaction->keyString;
			}

			list.append(data);
		}
	}
}

QList< TShortcutData* > TShortcutManager::getShortcutDataFor(QObject* item)
{
	QList<TShortcutData* > list;
	ContextItem* contextitem;

	do {
		const QMetaObject* mo = item->metaObject();

		// traverse upwards till no more superclasses are found
		// this supports inheritance on contextitems.
		while (mo) {
			getShortcutDataForMetaobject(mo, list);
			mo = mo->superClass();
		}

		contextitem = qobject_cast<ContextItem*>(item);
	}
	while (contextitem && (item = contextitem->get_context()) );


	return list;
}

void TShortcutManager::loadDefaultShortcuts()
{
	foreach(TShortcutData* data, m_dict.values())
	{
		delete data;
	}

	m_dict.clear();


	TShortcutData* data;

	data = new TShortcutData();
	data->classname = "AudioTrack";
	data->slotsignature = "toggle_show_clip_volume_automation";
	data->sortorder = 53;
	data->description = tr("Show/Hide Clip Volume Automation");

	addFunctionShortcut("AudioTrack::ShowClipVolumeAutomation", data);


//	data = new TShortcutData();
//	data->classname = "";
//	data->slotsignature = "";
//	data->sortorder = 0;
//	data->description = ;

//	addFunctionShortcut("", data);


//	data = new TShortcutData();
//	data->classname = "";
//	data->slotsignature = "";
//	data->sortorder = 0;
//	data->description = ;

//	addFunctionShortcut("", data);

}

