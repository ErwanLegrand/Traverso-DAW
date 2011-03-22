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

	foreach(int modifier, getModifierKeys()) {
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

	if (getModifierKeys().size())
	{
		modifiersString += " ";
	}

	foreach(QString keyString, getKeys())
	{

		sequenceList << (modifiersString + keyString);
	}

	sequence = sequenceList.join(" , ");

	TShortcutManager::makeShortcutKeyHumanReadable(sequence);

	return sequence;
}

QList<int> TFunction::getModifierKeys()
{
	if (m_inheritedFunction)
	{
		return m_inheritedFunction->getModifierKeys();
	}

	return m_modifierkeys;
}

QString TFunction::getSlotSignature() const
{
	if (m_inheritedFunction)
	{
		return m_inheritedFunction->getSlotSignature();
	}

	return slotsignature;
}

QString TFunction::getDescription() const
{
	if (m_inheritedFunction)
	{
		return m_inheritedFunction->getDescription();
	}

	return m_description;
}

void TFunction::setDescription(const QString& description)
{
	m_description = description;
}

QStringList TFunction::getKeys() const
{
	if (m_inheritedFunction)
	{
		return m_inheritedFunction->getKeys();
	}

	return m_keys;
}

void TFunction::setInheritedFunction(TFunction *inherited)
{
	m_inheritedFunction = inherited;
}

int TFunction::getAutoRepeatInterval() const
{
	if (m_inheritedFunction)
	{
		return m_inheritedFunction->getAutoRepeatInterval();
	}

	return m_autorepeatInterval;
}

int TFunction::getAutoRepeatStartDelay() const
{
	if (m_inheritedFunction)
	{
		return m_inheritedFunction->getAutoRepeatStartDelay();
	}

	return m_autorepeatStartDelay;
}

void TShortcutManager::makeShortcutKeyHumanReadable(QString& keyfact)
{
	keyfact.replace(QString("MOUSESCROLLVERTICALUP"), tr("Scroll Up"));
	keyfact.replace(QString("MOUSESCROLLVERTICALDOWN"), tr("Scroll Down"));
	keyfact.replace(QString("MOUSEBUTTONRIGHT"), tr("Right Button"));
	keyfact.replace(QString("MOUSEBUTTONLEFT"), tr("Left Button"));
	keyfact.replace(QString("MOUSEBUTTONMIDDLE"), tr("Center Button"));
	keyfact.replace(QString("UPARROW"), tr("Up Arrow"));
	keyfact.replace(QString("DOWNARROW"), tr("Down Arrow"));
	keyfact.replace(QString("LEFTARROW"), tr("Left Arrow"));
	keyfact.replace(QString("RIGHTARROW"), tr("Right Arrow"));
	keyfact.replace(QString("DELETE"), tr("Delete"));
	keyfact.replace(QString("MINUS"), QString("-"));
	keyfact.replace(QString("PLUS"), QString("+"));
	keyfact.replace(QString("PAGEDOWN"), tr("Page Down"));
	keyfact.replace(QString("PAGEUP"), tr("Page Up"));
	keyfact.replace(QString("ESC"), tr("Esc"));
	keyfact.replace(QString("NUMERICAL"), tr("0, 1, ... 9"));
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
	function->m_description = tr("Remove");
	function->commandName = "Delete";
	addFunction(function);

	function = new TFunction();
	function->slotsignature = "toggle_bypass";
	function->setDescription(tr("Toggle Bypass"));
	function->commandName = "ToggleBypass";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "toggle_snap_on_off";
	function->setDescription(tr("Toggle Snap on/off"));
	function->commandName = "MoveCommandToggleSnap";
	addFunction(function);

	function = new TFunction();
	function->slotsignature = "edit_properties";
	function->setDescription(tr("Edit Properties"));
	function->commandName = "EditProperties";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioTrack";
	function->slotsignature = "toggle_show_clip_volume_automation";
	function->m_description = tr("Show/Hide Clip Volume Automation");
	function->commandName = "AudioTrack_ShowClipVolumeAutomation";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioTrack";
	function->slotsignature = "toggle_arm";
	function->m_description = tr("Record: On/Off");
	function->commandName = "AudioTrack_ToggleRecord";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioTrack";
	function->slotsignature = "silence_others";
	function->m_description = tr("Silence other tracks");
	function->commandName = "AudioTrack_SilenceOthers";
	addFunction(function);

	function = new TFunction();
	function->object = "FadeCurve";
	function->slotsignature = "set_mode";
	function->m_description = tr("Cycle Shape");
	function->commandName = "FadeCurve_CycleShape";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_right";
	function->m_description = tr("Move Right");
	function->setUsesAutoRepeat(true);
	function->commandName = "MoveCommandRight";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_left";
	function->m_description = tr("Move Left");
	function->setUsesAutoRepeat(true);
	function->commandName = "MoveCommandLeft";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_up";
	function->m_description = tr("Move Up");
	function->setUsesAutoRepeat(true);
	function->commandName = "MoveCommandUp";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_down";
	function->m_description = tr("Move Down");
	function->setUsesAutoRepeat(true);
	function->commandName = "MoveCommandDown";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_faster";
	function->m_description = tr("Move Faster");
	function->setUsesAutoRepeat(true);
	function->commandName = "MoveCommandFaster";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "move_slower";
	function->m_description = tr("Move Slower");
	function->setUsesAutoRepeat(true);
	function->commandName = "MoveCommandSlower";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "quick_start";
	function->m_description = tr("Show Help");
	function->commandName = "ShowHelp";
	addFunction(function);

	function = new TFunction();
	function->object = "Track";
	function->slotsignature = "toggle_show_track_volume_automation";
	function->m_description = tr("Show/Hide Volume Automation");
	function->commandName = "Track_ShowVolumeAutomation";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "full_screen";
	function->m_description = tr("Full Screen");
	function->commandName = "MainWindow_ShowFullScreen";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "export_keymap";
	function->m_description = tr("Export keymap");
	function->commandName = "ExportShortcutMap";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackView";
	function->slotsignature = "add_new_plugin";
	function->m_description = tr("Add new Plugin");
	function->commandName = "Track_AddPlugin";
	addFunction(function);

	function = new TFunction();
	function->object = "TPanKnobView";
	function->slotsignature = "pan_left";
	function->m_description = tr("Pan to Left");
	function->commandName = "PanKnobPanLeft";
	addFunction(function);

	function = new TFunction();
	function->object = "TPanKnobView";
	function->slotsignature = "pan_right";
	function->m_description = tr("Pan to Right");
	function->commandName = "PanKnobPanRight";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "hzoom_out";
	function->m_description = tr("Out");
	function->commandName = "ZoomOut";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "hzoom_in";
	function->m_description = tr("In");
	function->commandName = "ZoomIn";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackPan";
	function->slotsignature = "pan_left";
	function->m_description = tr("Pan to Left");
	function->setUsesAutoRepeat(true);
	function->commandName = "TrackPanLeft";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackPan";
	function->slotsignature = "pan_right";
	function->m_description = tr("Pan to Right");
	function->setUsesAutoRepeat(true);
	function->commandName = "TrackPanRight";
	addFunction(function);

	function = new TFunction();
	function->object = "Gain";
	function->slotsignature = "increase_gain";
	function->m_description = tr("Increase");
	function->setUsesAutoRepeat(true);
	function->commandName = "GainIncrease";
	addFunction(function);

	function = new TFunction();
	function->object = "Gain";
	function->slotsignature = "decrease_gain";
	function->m_description = tr("Decrease");
	function->setUsesAutoRepeat(true);
	function->commandName = "GainDecrease";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveTrack";
	function->slotsignature = "move_up";
	function->m_description = tr("Move Up");
	function->setUsesAutoRepeat(true);
	function->commandName = "MoveTrackUp";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveTrack";
	function->slotsignature = "move_down";
	function->m_description = tr("Move Down");
	function->setUsesAutoRepeat(true);
	function->commandName = "MoveTrackDown";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "scroll_up";
	function->m_description =tr("Up");
	function->commandName = "ViewScrollUp";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "scroll_down";
	function->m_description = tr("Down");
	function->commandName = "ViewScrollDown";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "track_vzoom_out";
	function->m_description = tr("Track Vertical Zoom Out");
	function->setUsesAutoRepeat(true);
	function->commandName = "ZoomTrackVerticalOut";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "track_vzoom_in";
	function->m_description = tr("Track Vertical Zoom In");
	function->setUsesAutoRepeat(true);
	function->commandName = "ZoomTrackVerticalIn";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "to_upper_context_level";
	function->m_description = tr("One Layer Up");
	function->commandName = "NavigateToUpperContext";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "to_lower_context_level";
	function->m_description = tr("One Layer Down");
	function->commandName = "NavigateToLowerContext";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioClipView";
	function->slotsignature = "fade_range";
	function->m_description = tr("Closest: Adjust Length");
	function->commandName = "AudioClip_FadeLength";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "start_transport";
	function->m_description = tr("Play (Start/Stop)");
	function->commandName = "PlayStartStop";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "set_recordable_and_start_transport";
	function->m_description = tr("Start Recording");
	function->commandName = "SetRecordingPlayStart";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackView";
	function->inherits = "EditProperties";
	function->commandName = "EditTrackProperties";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->inherits = "EditProperties";
	function->commandName = "EditSongProperties";
	addFunction(function);

	function = new TFunction();
	function->object = "PluginView";
	function->inherits = "EditProperties";
	function->commandName = "EditPluginProperties";
	addFunction(function);

	function = new TFunction();
	function->object = "SpectralMeterView";
	function->inherits = "EditProperties";
	function->commandName = "EditSpectralMeterProperties";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioClipView";
	function->inherits = "EditProperties";
	function->commandName = "EditAudioClipProperties";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "toggle_expand_all_tracks";
	function->m_description = tr("Expand/Collapse Tracks");
	function->commandName = "ZoomToggleExpandAllTracks";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackPanelLed";
	function->slotsignature = "toggle";
	function->m_description = tr("Toggle On/Off");
	function->commandName = "PanelLedToggle";
	addFunction(function);

	function = new TFunction();
	function->object = "CurveView";
	function->slotsignature = "add_node";
	function->m_description = tr("New Node");
	function->commandName = "AddCurveNode";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCommand";
	function->slotsignature = "numerical_input";
	function->m_description = tr("Moving Speed");
	function->commandName = "MoveCommandSpeed";
	addFunction(function);

	function = new TFunction();
	function->object = "TCommand";
	function->m_description = tr("Abort");
	function->commandName = "AbortHoldCommand";
	addFunction(function);

	function = new TFunction();
	function->object = "Gain";
	function->slotsignature = "numerical_input";
	function->m_description = tr("Input dB value");
	function->commandName = "GainNumericalInput";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "numerical_input";
	function->m_description = tr("Track Height");
	function->commandName = "ZoomNumericalInput";
	addFunction(function);

	function = new TFunction();
	function->object = "FadeCurveView";
	function->slotsignature = "select_fade_shape";
	function->m_description = tr("Select Preset");
	function->commandName = "FadeSelectPreset";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "show_newtrack_dialog";
	function->m_description = tr("New Track Dialog");
	function->commandName = "ShowNewTrackDialog";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "browse_to_time_line";
	function->m_description = tr("To Timeline");
	function->commandName = "NavigateToTimeLine";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackPanelGain";
	function->slotsignature = "gain_decrement";
	function->m_description = tr("Decrease");
	function->commandName = "TrackPanelGainDecrement";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackPanelGain";
	function->slotsignature = "gain_increment";
	function->m_description = tr("Increase");
	function->commandName = "TrackPanelGainIncrement";
	addFunction(function);

	function = new TFunction();
	function->object = "CropClip";
	function->slotsignature = "adjust_left";
	function->m_description = tr("Adjust Left");
	function->setUsesAutoRepeat(true);
	function->commandName = "CropClipAdjustLeft";
	addFunction(function);

	function = new TFunction();
	function->object = "CropClip";
	function->slotsignature = "adjust_right";
	function->m_description = tr("Adjust Right");
	function->setUsesAutoRepeat(true);
	function->commandName = "CropClipAdjustRight";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "touch";
	function->m_description = tr("Set");
	function->commandName = "WorkCursorTouch";
	addFunction(function);

	function = new TFunction();
	function->object = "TimeLineView";
	function->slotsignature = "playhead_to_marker";
	function->m_description = tr("Playhead to Marker");
	function->commandName = "TimeLinePlayheadToMarker";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioClipView";
	function->slotsignature = "set_audio_file";
	function->m_description = tr("Reset Audio File");
	function->commandName = "AudioClipSetAudioFile";
	addFunction(function);

	function = new TFunction();
	function->object = "FadeCurve";
	function->slotsignature = "toggle_raster";
	function->m_description = tr("Toggle Raster");
	function->commandName = "FadeCurveToggleRaster";
	addFunction(function);

	function = new TFunction();
	function->object = "TrackPan";
	function->slotsignature = "reset_pan";
	function->m_description = tr("Reset");
	function->commandName = "TrackPanReset";
	addFunction(function);

	function = new TFunction();
	function->object = "SpectralMeterView";
	function->slotsignature = "reset";
	function->m_description = tr("Reset average curve");
	function->commandName = "SpectralMeterResetAverageCurve";
	addFunction(function);

	function = new TFunction();
	function->object = "AudioClip";
	function->slotsignature = "lock";
	function->m_description = tr("Lock");
	function->commandName = "AudioClipLock";
	addFunction(function);

	function = new TFunction();
	function->object = "CorrelationMeterView";
	function->slotsignature = "set_mode";
	function->m_description = tr("Toggle display range");
	function->commandName = "CorrelationMeterToggleDisplayRange";
	addFunction(function);

	function = new TFunction();
	function->object = "SpectralMeterView";
	function->slotsignature = "set_mode";
	function->m_description = tr("Toggle average curve");
	function->commandName = "SpectralMeterToggleDisplayRange";
	addFunction(function);

	function = new TFunction();
	function->object = "TimeLineView";
	function->slotsignature = "add_marker";
	function->m_description = tr("Add Marker");
	function->commandName = "TimeLineAddMarker";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "add_marker";
	function->m_description = tr("Add Marker");
	function->commandName = "SheetAddMarker";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "add_marker_at_playhead";
	function->m_description = tr("Add Marker at Playhead");
	function->commandName = "SheetAddMarkerAtPlayhead";
	addFunction(function);

	function = new TFunction();
	function->object = "TimeLineView";
	function->slotsignature = "add_marker_at_playhead";
	function->m_description = tr("Add Marker at Playhead");
	function->commandName = "TimeLineAddMarkerAtPlayhead";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "add_marker_at_work_cursor";
	function->m_description = tr("Add Marker at Work Cursor");
	function->commandName = "SheetAddMarkerAtWorkCursor";
	addFunction(function);

	function = new TFunction();
	function->object = "TimeLineView";
	function->slotsignature = "add_marker_at_work_cursor";
	function->m_description = tr("Add Marker at Work Cursor");
	function->commandName = "TimeLineAddMarkerAtWorkCursor";
	addFunction(function);

	function = new TFunction();
	function->object = "FadeCurve";
	function->inherits = "ToggleBypass";
	function->commandName = "FadeCurveToggleBypass";
	addFunction(function);

	function = new TFunction();
	function->object = "Plugin";
	function->inherits = "ToggleBypass";
	function->commandName = "PluginToggleBypass";
	addFunction(function);

	function = new TFunction();
	function->object = "FadeCurveView";
	function->slotsignature = "bend";
	function->setDescription(tr("Adjust Bend"));
	function->useY = true;
	function->commandName = "FadeCurveBend";
	addFunction(function);

	function = new TFunction();
	function->object = "TimeLineView";
	function->slotsignature = "TMainWindow::show_marker_dialog";
	function->setDescription(tr("Edit Markers"));
	function->commandName = "TimeLineShowMarkerDialog";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "show_context_menu";
	function->setDescription(tr("Context Menu"));
	function->commandName = "ShowContextMenu";
	addFunction(function);

	function = new TFunction();
	function->object = "HoldCommand";
	function->slotsignature = "TMainWindow::show_context_menu";
	function->setDescription(tr("Context Menu"));
	function->commandName = "HoldCommandShowContextMenu";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "show_project_manager_dialog";
	function->setDescription(tr("Show Project Management Dialog"));
	function->commandName = "MainWindowShowProjectManagementDialog";
	addFunction(function);

	function = new TFunction();
	function->object = "ProcessingData";
	function->slotsignature = "mute";
	function->setDescription(tr("Mute"));
	function->commandName = "Mute";
	addFunction(function);

	function = new TFunction();
	function->object = "Track";
	function->slotsignature = "solo";
	function->setDescription(tr("Solo"));
	function->commandName = "Solo";
	addFunction(function);

	function = new TFunction();
	function->object = "CurveView";
	function->slotsignature = "toggle_select_all_nodes";
	function->setDescription(tr("Select All Nodes"));
	function->commandName = "CurveSelectAllNodes";
	addFunction(function);

	function = new TFunction();
	function->object = "ProjectManager";
	function->slotsignature = "save_project";
	function->setDescription(tr("Save Project"));
	function->commandName = "ProjectSave";
	addFunction(function);

	function = new TFunction();
	function->object = "CurveView";
	function->slotsignature = "select_lazy_selected_node";
	function->setDescription(tr("Select Node"));
	function->commandName = "CurveSelectNode";
	addFunction(function);

	function = new TFunction();
	function->object = "FadeCurveView";
	function->slotsignature = "strength";
	function->setDescription(tr("Adjust Strength"));
	function->setUsesAutoRepeat(true);
	function->commandName = "FadeCurveStrenght";
	function->useX = true;
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "goto_end";
	function->setDescription(tr("To end"));
	function->commandName = "WorkCursorToEnd";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "play_to_end";
	function->setDescription(tr("To end"));
	function->commandName = "PlayHeadToEnd";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveTrack";
	function->slotsignature = "to_bottom";
	function->setDescription(tr("To Bottom"));
	function->commandName = "MoveTrackToBottom";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "goto_begin";
	function->setDescription(tr("To start"));
	function->commandName = "WorkCursorToStart";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "play_to_begin";
	function->setDescription(tr("To start"));
	function->commandName = "PlayHeadToStart";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveTrack";
	function->slotsignature = "to_top";
	function->setDescription(tr("To Top"));
	function->commandName = "MoveTrackToTop";
	addFunction(function);

	function = new TFunction();
	function->object = "PlayHeadMove";
	function->slotsignature = "move_to_work_cursor";
	function->setDescription(tr("To Work Cursor"));
	function->commandName = "PlayHeadMoveToWorkCursor";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "touch_play_cursor";
	function->setDescription(tr("Set"));
	function->commandName = "SheetSetPlayPosition";
	addFunction(function);

	function = new TFunction();
	function->object = "SheetView";
	function->slotsignature = "center_playhead";
	function->setDescription(tr("Center"));
	function->commandName = "SheetCenterPlayhead";
	addFunction(function);

	function = new TFunction();
	function->object = "Zoom";
	function->slotsignature = "toggle_vertical_horizontal_jog_zoom";
	function->setDescription(tr("Toggle Vertical / Horizontal"));
	function->commandName = "ZoomToggleVerticalHorizontal";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveClip";
	function->slotsignature = "toggle_vertical_only";
	function->setDescription(tr("Toggle Vertical Only"));
	function->commandName = "MoveClipToggleVerticalOnly";
	addFunction(function);

	function = new TFunction();
	function->object = "MoveCurveNode";
	function->slotsignature = "toggle_vertical_only";
	function->setDescription(tr("Toggle Vertical Only"));
	function->commandName = "MoveCurveNodeToggleVerticalOnly";
	addFunction(function);

	function = new TFunction();
	function->object = "WorkCursorMove";
	function->slotsignature = "move_to_play_cursor";
	function->setDescription(tr("To Playhead"));
	function->commandName = "WorkCursorMoveToPlayhead";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "show_track_finder";
	function->setDescription(tr("Activate Track Finder"));
	function->commandName = "MainWindowActivateTrackFinder";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "browse_to_first_track_in_active_sheet";
	function->setDescription(tr("Browse to first Track in current View"));
	function->commandName = "MainWindowNavigateToFirstTrack";
	addFunction(function);

	function = new TFunction();
	function->object = "TMainWindow";
	function->slotsignature = "browse_to_last_track_in_active_sheet";
	function->setDescription(tr("Browse to last Track in current View"));
	function->commandName = "MainWindowNavigateToLastTrack";
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
	settings.endGroup();
}

void TShortcutManager::loadShortcuts()
{
	QSettings settings(":/Traverso/shortcuts.ini", QSettings::IniFormat);

	QStringList groups = settings.childGroups();
	QList<TFunction*> functionsThatInherit;

	foreach(TFunction* function, m_functions)
	{
		if (!groups.contains(function->commandName))
		{
			continue;
		}

		settings.beginGroup(function->commandName);
		QString keyString = settings.value("keys").toString();
		QStringList keys = keyString.toUpper().split(";", QString::SkipEmptyParts);
		QStringList modifiers = settings.value("modifiers").toString().toUpper().split(";", QString::SkipEmptyParts);
		QString autorepeatinterval = settings.value("autorepeatinterval").toString();
		QString autorepeatstartdelay = settings.value("autorepeatstartdelay").toString();
		QString submenu = settings.value("submenu").toString();
		QString sortorder = settings.value("sortorder").toString();
		settings.endGroup();

		function->submenu = submenu;

		foreach(QString string, modifiers)
		{
			int modifier;
			if (t_KeyStringToKeyValue(modifier, string))
			{
				function->m_modifierkeys << modifier;
			}
		}

		bool ok;
		int interval = autorepeatinterval.toInt(&ok);
		if (ok)
		{
			function->m_autorepeatInterval = interval;
		}

		int startdelay = autorepeatstartdelay.toInt(&ok);
		if (ok)
		{
			function->m_autorepeatStartDelay = startdelay;
		}

		int order = sortorder.toInt(&ok);
		if (ok)
		{
			function->sortorder = order;
		}

		function->m_keys << keys;

		if (!function->inherits.isEmpty())
		{
			functionsThatInherit.append(function);
		}
		else
		{
			foreach(QString key, function->getKeys())
			{
				TShortcut* shortcut = getShortcut(key);
				if (shortcut)
				{
					shortcut->objects.insertMulti(function->object, function);
				}
			}
		}
	}

	foreach(TFunction* function, functionsThatInherit)
	{
		TFunction* inheritedFunction = getFunction(function->inherits);
		if (inheritedFunction)
		{
			function->setInheritedFunction(inheritedFunction);
			foreach(QString key, function->getKeys())
			{
				TShortcut* shortcut = getShortcut(key);
				if (shortcut)
				{
					shortcut->objects.insertMulti(function->object, function);
				}
			}
		}
	}
}

void TShortcutManager::modifyFunctionKeys(TFunction *function, QStringList keys)
{
	function->m_keys.clear();

	foreach(TShortcut* shortcut, m_shortcuts)
	{
		shortcut->objects.remove(function->object, function);
	}

	foreach(QString key, keys) {
		function->m_keys << key;
		TShortcut* shortcut = getShortcut(key);
		if (shortcut)
		{
			shortcut->objects.insertMulti(function->object, function);
		}
	}
}
