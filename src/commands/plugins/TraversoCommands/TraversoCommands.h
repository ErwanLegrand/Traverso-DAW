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

#ifndef TRAVERSO_COMMANDS_H
#define TRAVERSO_COMMANDS_H


#include <CommandPlugin.h>

class TraversoCommands : public CommandPlugin
{
	Q_OBJECT
	Q_CLASSINFO("Gain", tr("Gain"))
	Q_CLASSINFO("ResetGain", tr("Gain: Reset"))
	Q_CLASSINFO("TrackPan", tr("Panorama"))
	Q_CLASSINFO("ResetTrackPan", tr("Panorama: Reset"))
	Q_CLASSINFO("ImportAudio", tr("Import audio"))
	Q_CLASSINFO("CopyClip", tr("Copy Clip"))
	Q_CLASSINFO("AddNewTrack", tr("New Track"))
	Q_CLASSINFO("RemoveClip", tr("Remove Clip"))
	Q_CLASSINFO("RemoveTrack", tr("Remove Track"))
	Q_CLASSINFO("AudioClipExternalProcessing", tr("External Processing"))
	Q_CLASSINFO("ClipSelectionSelect", tr("Select"))
	Q_CLASSINFO("ClipSelectionRemove", tr("Remove from Selection"))
	Q_CLASSINFO("ClipSelectionAdd", tr("Add to Selection"))
	Q_CLASSINFO("MoveClip", tr("Move Clip"))
	Q_CLASSINFO("DragEdge", tr("Drag Edge"))
	Q_CLASSINFO("MoveClipOrEdge", tr("Move Or Resize Clip"))
	Q_CLASSINFO("SplitClip", tr("Split"))
	Q_CLASSINFO("ArmTracks", tr("Arm Tracks"))

public:
	TraversoCommands();
	Command* create(QObject* obj, const QString& command, QVariantList arguments);
	
private:
	enum Commands {
		GainCommand,
  		TrackPanCommand,
    		ImportAudioCommand,
		AddNewTrackCommand,
  		RemoveClipCommand,
  		RemoveTrackCommand,
		AudioClipExternalProcessingCommand,
  		ClipSelectionCommand,
    		MoveClipCommand,
    		DragEdgeCommand,
    		MoveClipOrEdgeCommand,
      		SplitClipCommand,
		ArmTracksCommand
	};
};

#endif

//eof
