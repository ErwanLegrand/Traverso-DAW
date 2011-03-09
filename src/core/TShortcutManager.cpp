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
#include "Information.h"
#include "Utils.h"

#include "Debugger.h"


QString TFunction::getKeySequence()
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
	loadFunctions();
}

void TShortcutManager::addFunction(const QString &function, TFunction *data)
{
	m_functions.insert(function, data);
}

TFunction* TShortcutManager::getFunctionshortcut(const QString &function) const
{
	TFunction* data = m_functions.value(function, 0);
	if (!data)
	{
		printf("TShortcutManager::getFunctionshortcut: Function %s not in database!!\n", function.toAscii().data());
	}

	return data;
}


void TShortcutManager::getFunctionsForMetaobject(const QMetaObject * mo, QList<TFunction* > & list) const
{
	const char* classname = mo->className();
	QList<TShortcutKey*> ieActions = ie().get_ie_actions();

	for (int i=0; i<ieActions.size(); i++) {
		TShortcutKey* ieaction = ieActions.at(i);

		QList<TFunction*> datalist;
		datalist.append(ieaction->objects.value(classname));

		foreach(TFunction* data, datalist)
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

QList< TFunction* > TShortcutManager::getFunctionsFor(QObject* item)
{
	QList<TFunction* > list;
	ContextItem* contextitem;

	do {
		const QMetaObject* mo = item->metaObject();

		// traverse upwards till no more superclasses are found
		// this supports inheritance on contextitems.
		while (mo) {
			getFunctionsForMetaobject(mo, list);
			mo = mo->superClass();
		}

		contextitem = qobject_cast<ContextItem*>(item);
	}
	while (contextitem && (item = contextitem->get_context()) );


	return list;
}

TShortcutKey* TShortcutManager::getShortcutFor(const QString &keyString)
{
	int keyValue = -1;

	if (!t_KeyStringToKeyValue(keyValue, keyString)) {
	       info().warning(tr("Shortcut Manager: Loaded keymap has this unrecognized key: %1").arg(keyString));
	       return 0;
	}

	TShortcutKey* shortcutKey = m_shortcutKeys.value(keyValue, 0);

	if (!shortcutKey)
	{
		printf("Adding key %s to shortcut keys\n", keyString.toAscii().data());
		shortcutKey = new TShortcutKey(keyValue);
		shortcutKey->keyString = keyString;
		m_shortcutKeys.insert(keyValue, shortcutKey);

	}

	return shortcutKey;
}

void TShortcutManager::loadFunctions()
{
	foreach(TFunction* data, m_functions.values())
	{
		delete data;
	}

	m_functions.clear();


	TFunction* function;

	function = new TFunction();
	function->classname = "AudioTrack";
	function->slotsignature = "toggle_show_clip_volume_automation";
	function->sortorder = 53;
	function->description = tr("Show/Hide Clip Volume Automation");
	addFunction("AudioTrack::ShowClipVolumeAutomation", function);

	function = new TFunction();
	function->classname = "AudioTrack";
	function->slotsignature = "toggle_arm";
	function->sortorder = 4;
	function->description = tr("Record: On/Off");
	addFunction("AudioTrack::ToggleRecord", function);

	function = new TFunction();
	function->classname = "AudioTrack";
	function->slotsignature = "silence_others";
	function->sortorder = 0;
	function->description = tr("Silence other tracks");
	addFunction("AudioTrack::SilenceOthers", function);

//	function = new TFunction();
//	function->classname = "";
//	function->slotsignature = "";
//	function->sortorder = ;
//	function->description = ;
//	addFunction("", function);


}

