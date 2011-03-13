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

QList<TFunction*> TShortcut::getFunctionsForObject(const QString &objectName)
{
	return objects.values(objectName);
}

QString TFunction::getKeySequence()
{
	QString sequence;
	QStringList sequenceList;
	QString modifiersString;

	foreach(int modifier, modifierkeys) {
		if (modifier == Qt::Key_Alt) {
			modifiersString += "Alt+";
		} else if (modifier == Qt::Key_Control) {
			modifiersString += "Ctrl+";
		} else if (modifier == Qt::Key_Shift) {
			modifiersString += "Shift+";
		} else if (modifier == Qt::Key_Meta) {
			modifiersString += "Meta+";
		}
	}

	foreach(QString keyString, m_defaultKeyStrings)
	{

		sequenceList << (modifiersString + keyString);
	}

	sequence = sequenceList.join(" , ");

	TShortcutManager::makeShortcutKeyHumanReadable(sequence);

	return sequence;
}

QStringList TFunction::getKeyStrings() const
{
	return m_defaultKeyStrings;
}

void TFunction::addDefaultKeyString(const QString &keyString)
{
	m_defaultKeyStrings << keyString.split(";");
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

void TShortcutManager::addFunction(TFunction *function)
{
	if (m_functions.contains(function->commandName))
	{
		printf("There is already a function registered with command name %s\n", QS_C(function->commandName));
		return;
	}

	m_functions.insert(function->commandName, function);

	QStringList keyStrings = function->getKeyStrings();
	foreach(QString keyString, keyStrings) {
		TShortcut* shortcut = getShortcut(keyString);
		if (shortcut)
		{
			shortcut->objects.insertMulti(function->object, function);
		}
	}
}

TFunction* TShortcutManager::getFunction(const QString &functionName) const
{
	TFunction* function = m_functions.value(functionName, 0);
	if (!function)
	{
		printf("TShortcutManager::getFunction: Function %s not in database!!\n", functionName.toAscii().data());
	}

	return function;
}


QList<TFunction* > TShortcutManager::getFunctionsForMetaobject(const QMetaObject * metaObject) const
{
	QList<TFunction* > list;
	QString classname = metaObject->className();

	foreach(TFunction* function, m_functions)
	{
		if (function->object == classname)
		{
			list.append(function);
		}
	}

	return list;
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
			list << getFunctionsForMetaobject(mo);
			mo = mo->superClass();
		}

		contextitem = qobject_cast<ContextItem*>(item);
	}
	while (contextitem && (item = contextitem->get_context()) );


	return list;
}

TShortcut* TShortcutManager::getShortcut(const QString &keyString)
{
	int keyValue = -1;

	if (!t_KeyStringToKeyValue(keyValue, keyString)) {
	       info().warning(tr("Shortcut Manager: Loaded keymap has this unrecognized key: %1").arg(keyString));
	       return 0;
	}

	TShortcut* shortcut = m_shortcuts.value(keyValue, 0);

	if (!shortcut)
	{
		printf("Adding key %s to shortcut keys\n", keyString.toAscii().data());
		shortcut = new TShortcut(keyValue);
		m_shortcuts.insert(keyValue, shortcut);

	}

	return shortcut;
}

TShortcut* TShortcutManager::getShortcut(int key)
{
	TShortcut* shortcut = m_shortcuts.value(key, 0);
	return shortcut;
}

void TShortcutManager::loadFunctions()
{
	foreach(TFunction* function, m_functions.values())
	{
		delete function;
	}

	m_functions.clear();


	TFunction* function;

	function = new TFunction();
	function->object = "AudioTrack";
	function->slotsignature = "toggle_show_clip_volume_automation";
	function->description = tr("Show/Hide Clip Volume Automation");
	function->addDefaultKeyString("F2");
	function->commandName = "AudioTrack:ShowClipVolumeAutomation";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioTrack";
	function->slotsignature = "toggle_arm";
	function->description = tr("Record: On/Off");
	function->addDefaultKeyString("R");
	function->commandName = "AudioTrack:ToggleRecord";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioTrack";
	function->slotsignature = "silence_others";
	function->description = tr("Silence other tracks");
	function->addDefaultKeyString("A");
	function->commandName = "AudioTrack:SilenceOthers";
	addFunction(function);

	function = new TFunction();
	function->object = "FadeCurve";
	function->slotsignature = "set_mode";
	function->description = tr("Cycle Shape");
	function->addDefaultKeyString("H");
	function->commandName = "FadeCurve:CycleShape";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "show_context_menu";
	function->description = tr("Show Context Menu");
	function->addDefaultKeyString("MouseButtonRight");
	function->commandName = "TMainWindow:ShowContextMenu";
	addFunction(function);

	function = new TFunction();
	function->object = "HoldCommand";
	function->slotsignature = "show_context_menu";
	function->description = tr("Show Context Menu");
	function->addDefaultKeyString("MouseButtonRight");
	function->commandName = "HoldCommand:ShowContextMenu";
	addFunction(function);
}
