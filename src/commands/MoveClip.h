/*
    Copyright (C) 2005-2008 Remon Sijrier 
 
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

#ifndef MOVECLIPACTION_H
#define MOVECLIPACTION_H

#include "MoveCommand.h"

#include <QPoint>
#include <defines.h>
#include "AudioClipGroup.h"
#include "Marker.h"

class AudioClip;
class TSession;
class AudioTrack;
class SheetView;
class ViewItem;
class Zoom;

typedef struct {
	Marker*	marker;
	TimeRef origin;
} MarkerAndOrigin;

class MoveClip : public MoveCommand
{
	Q_OBJECT
	Q_CLASSINFO("next_snap_pos", tr("To next snap position"));
	Q_CLASSINFO("prev_snap_pos", tr("To previous snap position"));
	Q_CLASSINFO("start_zoom", tr("Jog Zoom"));
	Q_CLASSINFO("move_up", tr("Move Up"));
	Q_CLASSINFO("move_down", tr("Move Down"));
	Q_CLASSINFO("move_left", tr("Move Left"));
	Q_CLASSINFO("move_right", tr("Move Right"));
	Q_CLASSINFO("toggle_vertical_only", tr("Toggle Vertical Only"));
	
public :
	MoveClip(ViewItem* view, QVariantList args);
        ~MoveClip();

        int begin_hold();
        int finish_hold();
        int prepare_actions();
        int do_action();
        int undo_action();
	void cancel_action();
        int jog();

	void set_cursor_shape(int useX, int useY);
        void set_jog_bypassed(bool bypassed);
	
private :
	enum ActionType {
		MOVE,
		COPY,
		FOLD_TRACK,
		FOLD_SHEET,
		FOLD_MARKERS,
		MOVE_TO_START,
		MOVE_TO_END
	};
	
        TSession*	m_session;
	AudioClipGroup  m_group;
	QList<MarkerAndOrigin>	m_markers;
        TimeRef 	m_trackStartLocation;
        TimeRef 	m_posDiff;
	ActionType	m_actionType;
	int		m_origTrackIndex;
	int		m_newTrackIndex;
	
	struct Data {
		SheetView* 	sv;
		int 		sceneXStartPos;
		int		pointedTrackIndex;
		bool		verticalOnly;
		Zoom*		zoom;
                TimeRef         relativeWorkCursorPos;
	};
	
	Data* d;

	void do_prev_next_snap(TimeRef trackStartLocation, TimeRef trackEndLocation);
	void do_move();
	
public slots:
	void next_snap_pos(bool autorepeat);
	void prev_snap_pos(bool autorepeat);
        void move_to_start(bool autorepeat);
        void move_to_end(bool autorepeat);
	void move_up(bool autorepeat);;
	void move_down(bool autorepeat);
	void move_left(bool autorepeat);
	void move_right(bool autorepeat);
	void start_zoom(bool autorepeat);
	void toggle_vertical_only(bool autorepeat);
};

#endif
