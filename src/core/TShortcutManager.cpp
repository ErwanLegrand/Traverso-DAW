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

#include <QPluginLoader>
#include <QSettings>
#include <QCoreApplication>

#include "ContextItem.h"
#include "InputEngine.h"
#include "Information.h"
#include "Utils.h"
#include "TConfig.h"
#include "CommandPlugin.h"

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
	QStringList modifierStringList = getModifierKeys();
	foreach(QString key, modifierStringList)
	{
		modifiersString = key + "+";
	}

	if (modifierStringList.size())
	{
		modifiersString += " ";
	}

	foreach(QString keyString, m_keys)
	{

		sequenceList << (modifiersString + keyString);
	}

	sequence = sequenceList.join(" , ");

	TShortcutManager::makeShortcutKeyHumanReadable(sequence);

	return sequence;
}

QStringList TFunction::getModifierKeys()
{
	QStringList list;

	foreach(int modifier, modifierkeys) {
		if (modifier == Qt::Key_Alt) {
			list << "Alt";
		} else if (modifier == Qt::Key_Control) {
			list << "Ctrl";
		} else if (modifier == Qt::Key_Shift) {
			list << "Shift";
		} else if (modifier == Qt::Key_Meta) {
			list << "Meta";
		}
	}

	return list;
}

QStringList TFunction::getKeys() const
{
	return m_keys;
}

void TShortcutManager::makeShortcutKeyHumanReadable(QString& keyfact)
{
	keyfact.replace(QString("MOUSESCROLLVERTICALUP"), tr("Scroll Wheel"));
	keyfact.replace(QString("MOUSESCROLLVERTICALDOWN"), tr("Scroll Wheel"));
	keyfact.replace(QString("MOUSEBUTTONRIGHT"), tr("Right MB"));
	keyfact.replace(QString("MOUSEBUTTONLEFT"), tr("Left MB"));
	keyfact.replace(QString("MOUSEBUTTONMIDDLE"), tr("Center MB"));
	keyfact.replace(QString("UPARROW"), tr("Up Arrow"));
	keyfact.replace(QString("DOWNARROW"), tr("Down Arrow"));
	keyfact.replace(QString("LEFTARROW"), tr("Left Arrow"));
	keyfact.replace(QString("RIGHTARROW"), tr("Right Arrow"));
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
}

void TShortcutManager::addFunction(TFunction *function)
{
	if (m_functions.contains(function->commandName))
	{
		printf("There is already a function registered with command name %s\n", QS_C(function->commandName));
		return;
	}

	m_functions.insert(function->commandName, function);
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

CommandPlugin* TShortcutManager::getCommandPlugin(const QString &pluginName)
{
	return m_commandPlugins.value(pluginName);
}

void TShortcutManager::loadFunctions()
{
#if defined (STATIC_BUILD)
	Q_IMPORT_PLUGIN(tcp_traversocommands);
#endif

	foreach (QObject* obj, QPluginLoader::staticInstances()) {
		CommandPlugin* plug = qobject_cast<CommandPlugin*>(obj);
		if (plug)
		{
			m_commandPlugins.insert(plug->metaObject()->className(), plug);
		}
	}

	TFunction* function;

	function = new TFunction();
	function->object = "AudioTrack";
	function->slotsignature = "toggle_show_clip_volume_automation";
	function->description = tr("Show/Hide Clip Volume Automation");
	function->commandName = "AudioTrack_ShowClipVolumeAutomation";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioTrack";
	function->slotsignature = "toggle_arm";
	function->description = tr("Record: On/Off");
	function->commandName = "AudioTrack_ToggleRecord";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioTrack";
	function->slotsignature = "silence_others";
	function->description = tr("Silence other tracks");
	function->commandName = "AudioTrack_SilenceOthers";
	addFunction(function);

	function = new TFunction();
	function->object = "FadeCurve";
	function->slotsignature = "set_mode";
	function->description = tr("Cycle Shape");
	function->commandName = "FadeCurve_CycleShape";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "show_context_menu";
	function->description = tr("Show Context Menu");
	function->commandName = "TMainWindow_ShowContextMenu";
	addFunction(function);

	function = new TFunction();
	function->object = "HoldCommand";
	function->slotsignature = "show_context_menu";
	function->description = tr("Show Context Menu");
	function->commandName = "HoldCommand_ShowContextMenu";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_right";
	function->description = tr("Move Right");
	function->commandName = "MoveCommandRight";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_left";
	function->description = tr("Move Left");
	function->commandName = "MoveCommandLeft";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_up";
	function->description = tr("Move Up");
	function->commandName = "MoveCommandUp";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_down";
	function->description = tr("Move Down");
	function->commandName = "MoveCommandDown";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_faster";
	function->description = tr("Move Faster");
	function->commandName = "MoveCommandFaster";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_slower";
	function->description = tr("Move Slower");
	function->commandName = "MoveCommandSlower";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "quick_start";
	function->description = tr("Show Help");
	function->commandName = "ShowHelp";
	addFunction(function);

	function = new TFunction();
	function->object = "Track";
	function->slotsignature = "toggle_show_track_volume_automation";
	function->description = tr("Show/Hide Volume Automation");
	function->commandName = "Track_ShowVolumeAutomation";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "full_screen";
	function->description = tr("Full Screen");
	function->commandName = "MainWindow_ShowFullScreen";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "export_keymap";
	function->description = tr("Export keymap");
	function->commandName = "ExportShortcutMap";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackView";
	function->slotsignature = "add_new_plugin";
	function->description = tr("Add new Plugin");
	function->commandName = "Track_AddPlugin";
	addFunction(function);

	function = new TFunction();
	function->object = "TPanKnobView";
	function->slotsignature = "pan_left";
	function->description = tr("Pan to Left");
	function->commandName = "PanKnobPanLeft";
	addFunction(function);

	function = new TFunction();
	function->object = "TPanKnobView";
	function->slotsignature = "pan_right";
	function->description = tr("Pan to Right");
	function->commandName = "PanKnobPanRight";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "hzoom_out";
	function->description = tr("Zoom Horizontal Out");
	function->commandName = "ZoomCommandOut";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "hzoom_in";
	function->description = tr("Zoom Horizontal In");
	function->commandName = "ZoomCommandIn";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackPan";
	function->slotsignature = "pan_left";
	function->description = tr("Pan to Left");
	function->commandName = "TrackPanLeft";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackPan";
	function->slotsignature = "pan_right";
	function->description = tr("Pan to Right");
	function->commandName = "TrackPanRight";
	addFunction(function);

	function = new TFunction();
	function->object = "Gain";
	function->slotsignature = "increase_gain";
	function->description = tr("Increase");
	function->commandName = "GainIncrease";
	addFunction(function);

	function = new TFunction();
	function->object = "Gain";
	function->slotsignature = "decrease_gain";
	function->description = tr("Decrease");
	function->commandName = "GainDecrease";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveTrack";
	function->slotsignature = "move_up";
	function->slotsignature = tr("Move Up");
	function->commandName = "MoveTrackUp";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveTrack";
	function->slotsignature = "move_down";
	function->description = tr("Move Down");
	function->commandName = "MoveTrackDown";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "scroll_up";
	function->description =tr("Up");
	function->commandName = "ViewScrollUp";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "scroll_down";
	function->description = tr("Down");
	function->commandName = "ViewScrollDown";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "track_vzoom_out";
	function->description = tr("Track Vertical Zoom Out");
	function->commandName = "ZoomTrackVerticalOut";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "track_vzoom_in";
	function->description = tr("Track Vertical Zoom In");
	function->commandName = "ZoomTrackVerticalIn";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "to_upper_context_level";
	function->description = tr("One Layer Up");
	function->commandName = "NavigateToUpperContext";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "to_lower_context_level";
	function->description = tr("One Layer Down");
	function->commandName = "NavigateToLowerContext";
	addFunction(function);


}

void TShortcutManager::saveFunction(TFunction *function)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Traverso", "Shortcuts");

	QStringList groups = settings.childGroups();
	if (groups.contains("HoldCommand_ShowContextMenu"))
	{
		printf("settings contains %s\n", "HoldCommand_ShowContextMenu");
	}

	settings.beginGroup(function->commandName);
	settings.setValue("keys", function->getKeys().join(";"));
	settings.setValue("modifiers", function->getModifierKeys().join(";"));
	settings.endGroup();
}

void TShortcutManager::loadShortcuts()
{
	QSettings::setPath (QSettings::IniFormat, QSettings::UserScope, ":/");
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Traverso", "shortcuts");

	QStringList groups = settings.childGroups();
	foreach(TFunction* function, m_functions)
	{
		if (!groups.contains(function->commandName))
		{
			continue;
		}

		settings.beginGroup(function->commandName);
		QStringList keys = settings.value("keys").toString().toUpper().split(";", QString::SkipEmptyParts);
		QStringList modifiers = settings.value("modifiers").toString().toUpper().split(";", QString::SkipEmptyParts);
		QString autorepeatinterval = settings.value("autorepeatinterval").toString();
		QString autorepeatstartdelay = settings.value("autorepeatstartdelay").toString();
		settings.endGroup();

		function->m_keys << keys;

		foreach(QString string, modifiers)
		{
			int modifier;
			if (t_KeyStringToKeyValue(modifier, string))
			{
				function->modifierkeys << modifier;
			}
		}

		bool ok;
		int interval = autorepeatinterval.toInt(&ok);
		if (ok)
		{
			function->autorepeatInterval = interval;
		}

		int startdelay = autorepeatstartdelay.toInt(&ok);
		if (ok)
		{
			function->autorepeatStartDelay = startdelay;
		}

		foreach(QString keyString, function->getKeys()) {
			TShortcut* shortcut = getShortcut(keyString);
			if (shortcut)
			{
				shortcut->objects.insertMulti(function->object, function);
			}
		}
	}
}
