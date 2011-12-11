/*
Copyright (C) 2007-2010 Remon Sijrier

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

class TFunction;

class TraversoCommands : public CommandPlugin
{
	Q_OBJECT

public:
	TraversoCommands();
	TCommand* create(QObject* obj, const QString& command, QVariantList arguments);

private:
	enum Commands {
		GainCommand,
		TrackPanCommand,
		ImportAudioCommand,
		InsertSilenceCommand,
		AddNewAudioTrackCommand,
		RemoveClipCommand,
		RemoveTrackCommand,
		RemoveClipNodeCommmand,
		RemovePluginCommand,
		AudioClipExternalProcessingCommand,
		ClipSelectionCommand,
		MoveClipCommand,
		MoveTrackCommand,
		MoveClipOrEdgeCommand,
		SplitClipCommand,
		CropClipCommand,
		ArmTracksCommand,
		ZoomCommand,
		ScrollCommand,
		ShuttleCommand,
		NormalizeClipCommand,
		ArrowKeyBrowserCommand,
		WorkCursorMoveCommand,
		MoveEdgeCommand,
		MoveCurveNodesCommand,
		MoveMarkerCommand
	};

private:
	void addFunction(TFunction* function, int command);
};

#endif

//eof
