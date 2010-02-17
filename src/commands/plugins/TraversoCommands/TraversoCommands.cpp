/*
Copyright (C) 2007-2008 Remon Sijrier 

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

#include "TraversoCommands.h"

#include <libtraversocore.h>
#include <libtraversosheetcanvas.h>
#include <commands.h>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
 *	\class TraversoCommands
	\brief The Traverso CommandPlugin class which 'implements' many of the default Commands
	
	With this plugin, the InputEngine is able to dispatch key actions by directly 
	asking this Plugin for the needed Command object.

	Dispatching key events to a specific object to create a Command object is accomplished by creating 
	entries in the keymap.xml file, which has the following syntax:

	\code 
	<Object objectname="" mousehint="" modes="" sortorder="" pluginname="" commandname="" arguments="" />
	\endcode 
	
	\code 
	objectname: 	The class name of the object that we want to dispatch the key action on
	mousehint 	Used to set the mouse cursor. Use "LR" for horizontal suggestion, "UD" for vertical suggestion, or "RLUD" for omnidirectional suggestion.
	modes:		Modes for which this command can be used, like All, Edit or Effects. Modes are to be defined in the keymap itself!
	sortorder:	The order of where the key action should show up in the ContextMenu of 'objectname'
	plugniname:	The plugin to ask the Command for, the only one available right now is TraversoCommands
	commandname:	The Command to be created, which by convention tries to be the same name as the actuall class implementing the Command
	arguments:	One, or more values (seperated by semicolons), each Command explains which (list of) values it can understand
	\endcode 


	Available Command classes, their required Object (to operate on) and arguments are:


	\code 
	MoveClip: Move or Copy-move an AudioClip.
	
	objectname:	AudioClipView
	arguments:	First entry, either one of the following: "move", "copy", "move_to_end", "move_to_start"
			Second (optional) entry: Move vertical only: "true" for vertical moving only, "false" for omnidirectional moving (the default if none is given)
	commandname:	MoveClip
	\endcode 


	\code 
	Gain: Change (jog), or reset the Gain of audio processing items
	objectname:	SheetView, TrackView, AudioClipView
	arguments:	No argument: Move Mouse vertically. Argument "horizontal": Move Mouse horizontally
	commandname:	Gain
	\endcode 
	
	\code 
	ResetGain: Set Gain value to a pre-defined value (by means of the supplied argument).
	
	objectname:	SheetView, TrackView, AudioClipView
	arguments:	a floating point value, eg: "0.5"
	commandname:	ResetGain
	\endcode 
	
	TODO document all remaining Commands
	\code 
	?: ?
	
	Objects(s):	
	arguments:	
	commandname:	
	\endcode 

 */


TraversoCommands::TraversoCommands()
{
	m_dict.insert("Gain", GainCommand);
	m_dict.insert("ResetGain", GainCommand);
	m_dict.insert("TrackPan", TrackPanCommand);
	m_dict.insert("ResetTrackPan", TrackPanCommand);
	m_dict.insert("ImportAudio", ImportAudioCommand);
	m_dict.insert("InsertSilence", InsertSilenceCommand);
	m_dict.insert("AddNewTrack", AddNewTrackCommand);
	m_dict.insert("RemoveClip", RemoveClipCommand);
        m_dict.insert("RemoveProcessingData", RemoveProcessingDataCommand);
	m_dict.insert("AudioClipExternalProcessing", AudioClipExternalProcessingCommand);
	m_dict.insert("ClipSelectionSelect", ClipSelectionCommand);
	m_dict.insert("ClipSelectionSelectAll", ClipSelectionCommand);
	m_dict.insert("ClipSelectionAdd", ClipSelectionCommand);
	m_dict.insert("ClipSelectionRemove", ClipSelectionCommand);
	m_dict.insert("MoveClip", MoveClipCommand);
	m_dict.insert("FoldSheet", MoveClipCommand);
	m_dict.insert("FoldTrack", MoveClipCommand);
	m_dict.insert("FoldMarkers", MoveClipCommand);
	m_dict.insert("DragEdge", DragEdgeCommand);
	m_dict.insert("MoveClipOrEdge", MoveClipOrEdgeCommand);
	m_dict.insert("CopyClip", MoveClipCommand);
        m_dict.insert("SplitClip", SplitClipCommand);
        m_dict.insert("CropClip", CropClipCommand);
        m_dict.insert("ArmTracks", ArmTracksCommand);
	m_dict.insert("VZoomIn", ZoomCommand);
	m_dict.insert("VZoomOut", ZoomCommand);
	m_dict.insert("HZoomIn", ZoomCommand);
	m_dict.insert("HZoomOut", ZoomCommand);
	m_dict.insert("HJogZoom", ZoomCommand);
	m_dict.insert("VJogZoom", ZoomCommand);
	m_dict.insert("JogZoom", ZoomCommand);
	m_dict.insert("ScrollRightHold", ScrollCommand);
	m_dict.insert("ScrollLeftHold", ScrollCommand);
	m_dict.insert("ScrollUpHold", ScrollCommand);
	m_dict.insert("ScrollDownHold", ScrollCommand);
        m_dict.insert("Shuttle", ShuttleCommand);
}

Command* TraversoCommands::create(QObject* obj, const QString& command, QVariantList arguments)
{
	switch (m_dict.value(command)) {
		case GainCommand:
		{
			ContextItem* item = qobject_cast<ContextItem*>(obj);
			SheetView* sheetview = 0;
                        printf("context item classname is %s\n", item->metaObject()->className());
			
			if (item->metaObject()->className() == QString("TrackPanelGain")) {
				item = item->get_context();
			} else if (item->metaObject()->className() == QString("SheetPanelGain")) {
				item = item->get_context();
			} else if (AudioClipView* view = qobject_cast<AudioClipView*>(item)) {
				sheetview = view->get_sheetview();
				item = view->get_context();
			} else if (TrackView* view = qobject_cast<TrackView*>(item)) {
				sheetview = view->get_sheetview();
				item = view->get_context();
                        } else if (SubGroupView* view = qobject_cast<SubGroupView*>(item)) {
                                sheetview = view->get_sheetview();
                                item = view->get_context();
                        } else if (SheetView* view = qobject_cast<SheetView*>(item)) {
				sheetview = view;
				item = view->get_context();
			}
			
			// ugly hack to avoid assigning a sheetview when the 
			// mouse cursor is above the trackpanel....
			if (cpointer().scene_x() < 0) {
                                sheetview = 0;
			}
			
			if (!item) {
				PERROR("TraversoCommands: Supplied QObject was not a ContextItem, "
					"GainCommand only works with ContextItem objects!!");
				return 0;
			}
			return new Gain(item, sheetview, arguments);
		}
		
		case TrackPanCommand:
		{
			Track* track = qobject_cast<Track*>(obj);
			if (! track) {
				TrackPanelPan* tpp = qobject_cast<TrackPanelPan*>(obj);
				if (! tpp ) {
					PERROR("TraversoCommands: Supplied QObject was not a Track! "
						"TrackPanCommand needs a Track as argument");
					return 0;
				}
				track = (Track*)(tpp->get_context());
			}
			return new TrackPan(track, arguments);
		}
		
		case ImportAudioCommand:
		{
			Track* track = qobject_cast<Track*>(obj);
			if (! track) {
				PERROR("TraversoCommands: Supplied QObject was not a Track! "
					"ImportAudioCommand needs a Track as argument");
				return 0;
			}
			return new Import(track, TimeRef());
		}
		
		case InsertSilenceCommand:
		{
			Track* track = qobject_cast<Track*>(obj);
			if (! track) {
				PERROR("TraversoCommands: Supplied QObject was not a Track! "
					"ImportAudioCommand needs a Track as argument");
				return 0;
			}
			TimeRef length(10*UNIVERSAL_SAMPLE_RATE);
			return new Import(track, length, true);
		}
		
		case AddNewTrackCommand:
		{
			Sheet* sheet = qobject_cast<Sheet*>(obj);
			if (!sheet) {
				PERROR("TraversoCommands: Supplied QObject was not a Sheet! "
					"AddNewTrackCommand needs a Sheet as argument");
				return 0;
			}
                        return sheet->add_processing_data(new Track(sheet, "Unnamed", Track::INITIAL_HEIGHT));
		}
		
		case RemoveClipCommand:
		{
			AudioClip* clip = qobject_cast<AudioClip*>(obj);
			if (!clip) {
				PERROR("TraversoCommands: Supplied QObject was not a Clip! "
					"RemoveClipCommand needs a Clip as argument");
				return 0;
			}
			return new AddRemoveClip(clip, AddRemoveClip::REMOVE);
		}
		
                case RemoveProcessingDataCommand:
		{
                        ProcessingData* pd = qobject_cast<ProcessingData*>(obj);
                        if (!pd) {
                                PERROR("TraversoCommands: Supplied QObject was not a ProcessingData! "
                                        "RemoveProcessingDataCommand needs a ProcessingData as argument");
				return 0;
			}
                        return pd->get_sheet()->remove_processing_data(pd);
		}
		
		case AudioClipExternalProcessingCommand:
		{
			AudioClip* clip = qobject_cast<AudioClip*>(obj);
			if (!clip) {
				PERROR("TraversoCommands: Supplied QObject was not an AudioClip! "
					"AudioClipExternalProcessingCommand needs an AudioClip as argument");
				return 0;
			}
			return new AudioClipExternalProcessing(clip);
		}
		
		case ClipSelectionCommand:
		{
			Sheet* sheet = qobject_cast<Sheet*>(obj);
			if (sheet) {
				QString action;
				if (arguments.size()) {
					action = arguments.at(0).toString();
					if (action == "select_all_clips") {
						return sheet->get_audioclip_manager()->select_all_clips();
					}
				}
			}
			AudioClip* clip = qobject_cast<AudioClip*>(obj);
			if (!clip) {
				PERROR("TraversoCommands: Supplied QObject was not an AudioClip! "
					"ClipSelectionCommand needs an AudioClip as argument");
				return 0;
			}
			return new ClipSelection(clip, arguments);
		}
		
		case MoveClipCommand:
		{
			ViewItem* view = qobject_cast<ViewItem*>(obj);
			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an AudioClipView! "
					"MoveClipCommand needs an AudioClipView as argument");
				return 0;
			}

			return new MoveClip(view, arguments);
		}
		
		case DragEdgeCommand:
		{
			AudioClipView* view = qobject_cast<AudioClipView*>(obj);
			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an AudioClipView! "
					"DragEdgeCommand needs an AudioClipView as argument");
				return 0;
			}
			
			int x = (int) (cpointer().on_first_input_event_scene_x() - view->scenePos().x());
			
			if (x < (view->boundingRect().width() / 2)) {
				return new MoveEdge(view, view->get_sheetview(), "set_left_edge");
			} else {
				return new MoveEdge(view, view->get_sheetview(), "set_right_edge");
			}
		}

		// The existence of this is doubtfull. Using [ E ] is so much easier
		// then trying to mimic 'if near to edge, drag edge' features.
		case MoveClipOrEdgeCommand:
		{
			AudioClipView* view = qobject_cast<AudioClipView*>(obj);

			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an AudioClipView! "
					"MoveClipOrEdgeCommand needs an AudioClipView as argument");
				return 0;
			}
			
			int x = (int) (cpointer().on_first_input_event_scene_x() - view->scenePos().x());
			
			int edge_width = 0;
			if (arguments.size() == 2) {
				edge_width = arguments[0].toInt();
			}
			
			if (x < edge_width) {
				return new MoveEdge(view, view->get_sheetview(), "set_left_edge");
			} else if (x > (view->boundingRect().width() - edge_width)) {
				return new MoveEdge(view, view->get_sheetview(), "set_right_edge");
			}
			
			return new MoveClip(view, QVariantList() << "move");
		}

		case SplitClipCommand:
		{
			AudioClipView* view = qobject_cast<AudioClipView*>(obj);
			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an AudioClipView! "
					"SplitClipCommand needs an AudioClipView as argument");
				return 0;
			}
			return new SplitClip(view);
		}
		
                case CropClipCommand:
                {
                        AudioClipView* view = qobject_cast<AudioClipView*>(obj);
                        if (!view) {
                                PERROR("TraversoCommands: Supplied QObject was not an AudioClipView! "
                                        "CropClipCommand needs an AudioClipView as argument");
                                return 0;
                        }
                        return new Crop(view);
                }

                case ArmTracksCommand:
		{
			SheetView* view = qobject_cast<SheetView*>(obj);
			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an SheetView! "
						"ArmTracksCommand needs an SheetView as argument");
				return 0;
			}
			return new ArmTracks(view);
		}
		
		case ZoomCommand:
		{
			SheetView* view = qobject_cast<SheetView*>(obj);
			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an SheetView! "
                                                "ZoomCommand needs an SheetView as argument");
				return 0;
			}
			return new Zoom(view, arguments);
		}

                case ScrollCommand:
		{
			SheetView* view = qobject_cast<SheetView*>(obj);
			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an SheetView! "
                                                "ScrollCommand needs an SheetView as argument");
				return 0;
			}
			return new Scroll(view, arguments);
		}

                case ShuttleCommand:
                {
                        SheetView* view = qobject_cast<SheetView*>(obj);
                        if (!view) {
                                PERROR("TraversoCommands: Supplied QObject was not an SheetView! "
                                                "ShuttleCommand needs an SheetView as argument");
                                return 0;
                        }
                        return new Shuttle(view);
                }

	}
	
	return 0;
}


Q_EXPORT_PLUGIN2(tcp_traversocommands, TraversoCommands)

// eof
