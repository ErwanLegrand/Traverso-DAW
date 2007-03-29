/*
Copyright (C) 2007 Remon Sijrier 

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
#include <libtraversosongcanvas.h>
#include <commands.h>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

TraversoCommands::TraversoCommands()
{
	m_dict.insert("Gain", GainCommand);
	m_dict.insert("ResetGain", GainCommand);
	m_dict.insert("TrackPan", TrackPanCommand);
	m_dict.insert("ImportAudio", ImportAudioCommand);
	m_dict.insert("AddNewTrack", AddNewTrackCommand);
	m_dict.insert("RemoveTrack", RemoveTrackCommand);
	m_dict.insert("AudioClipExternalProcessing", AudioClipExternalProcessingCommand);
	m_dict.insert("ClipSelectionSelect", ClipSelectionCommand);
	m_dict.insert("ClipSelectionAdd", ClipSelectionCommand);
	m_dict.insert("ClipSelectionRemove", ClipSelectionCommand);
	m_dict.insert("MoveClip", MoveClipCommand);
	m_dict.insert("DragEdge", DragEdgeCommand);
	m_dict.insert("MoveClipOrEdge", MoveClipOrEdgeCommand);
	m_dict.insert("CopyClip", MoveClipCommand);
	m_dict.insert("SplitClip", SplitClipCommand);
	m_dict.insert("ArmTracks", ArmTracksCommand);
}

Command* TraversoCommands::create(QObject* obj, const QString& command, QVariantList arguments)
{
	switch (m_dict.value(command)) {
		case GainCommand:
		{
			ContextItem* item = qobject_cast<ContextItem*>(obj);
			if (!item) {
				PERROR("TraversoCommands: Supplied QObject was not aContextItem, "
					"GainCommand only works with ContextItem objects!!");
				return 0;
			}
			return new Gain(item, arguments);
		}
		
		case TrackPanCommand:
		{
			Track* track = qobject_cast<Track*>(obj);
			if (! track) {
				PERROR("TraversoCommands: Supplied QObject was not a Track! "
					"TrackPanCommand needs a Track as argument");
				return 0;
			}
			return new TrackPan(track);
		}
		
		case ImportAudioCommand:
		{
			Track* track = qobject_cast<Track*>(obj);
			if (! track) {
				PERROR("TraversoCommands: Supplied QObject was not a Track! "
					"ImportAudioCommand needs a Track as argument");
				return 0;
			}
			return new Import(track);
		}
		
		case AddNewTrackCommand:
		{
			Song* song = qobject_cast<Song*>(obj);
			if (!song) {
				PERROR("TraversoCommands: Supplied QObject was not a Song! "
					"AddNewTrackCommand needs a Song as argument");
				return 0;
			}
			return song->add_track(new Track(song, "Unnamed", Track::INITIAL_HEIGHT));
		}
		
		case RemoveTrackCommand:
		{
			Track* track = qobject_cast<Track*>(obj);
			if (!track) {
				PERROR("TraversoCommands: Supplied QObject was not a Track! "
					"RemoveTrackCommand needs a Track as argument");
				return 0;
			}
			return track->get_song()->remove_track(track);
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
			AudioClipView* view = qobject_cast<AudioClipView*>(obj);
			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an AudioClipView! "
					"MoveClipCommand needs an AudioClipView as argument");
				return 0;
			}

			QString type;
			if (arguments.size()) {
				type = arguments.at(0).toString();
			} else {
				type = "move";  // Default Action
			}
			return new MoveClip(view, type);
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
			bool anchorAudio = arguments[0].toBool();

			if (x < (view->boundingRect().width() / 2)) {
				if (anchorAudio) {
					return new MoveClip(view, "anchored_left_edge_move");
				} else {
					return new MoveEdge(view, view->get_songview(), "set_left_edge");
				}
			} else {
				if (anchorAudio) {
					return new MoveClip(view, "anchored_right_edge_move");
				} else {
					return new MoveEdge(view, view->get_songview(), "set_right_edge");
				}
			}
		}

		case MoveClipOrEdgeCommand:
		{
			AudioClipView* view = qobject_cast<AudioClipView*>(obj);

			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an AudioClipView! "
					"MoveClipOrEdgeCommand needs an AudioClipView as argument");
				return 0;
			}

			int x = (int) (cpointer().on_first_input_event_scene_x() - view->scenePos().x());
			int edge_width = arguments[0].toInt();
			bool anchorAudio = arguments[1].toBool();

			if (x < edge_width) {
				if (anchorAudio) {
					return new MoveClip(view, "anchored_left_edge_move");
				} else {
					return new MoveEdge(view, view->get_songview(), "set_left_edge");
				}
			} else if (x > (view->boundingRect().width() - edge_width)) {
				if (anchorAudio) {
					return new MoveClip(view, "anchored_right_edge_move");
				} else {
					return new MoveEdge(view, view->get_songview(), "set_right_edge");
				}
			}
			return new MoveClip(view, "move");
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
		
		case ArmTracksCommand:
		{
			SongView* view = qobject_cast<SongView*>(obj);
			if (!view) {
				PERROR("TraversoCommands: Supplied QObject was not an SongView! "
						"ArmTracksCommand needs an SongView as argument");
				return 0;
			}
			return new ArmTracks(view);
		}
	}
	
	return 0;
}


Q_EXPORT_PLUGIN2(tcp_traversocommands, TraversoCommands)


// eof
