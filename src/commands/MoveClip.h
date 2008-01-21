/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: MoveClip.h,v 1.22 2008/01/21 16:22:11 r_sijrier Exp $
*/

#ifndef MOVECLIPACTION_H
#define MOVECLIPACTION_H

#include "Command.h"

#include <QPoint>
#include <defines.h>

class AudioClip;
class Sheet;
class Track;
class SheetView;
class TrackView;
class AudioClipView;

class MoveClip : public Command
{
	Q_OBJECT
	Q_CLASSINFO("next_snap_pos", tr("To next snap position"));
	Q_CLASSINFO("prev_snap_pos", tr("To previous snap position"));
	
public :
	MoveClip(AudioClipView* clipView, QString type);
        ~MoveClip();

        int begin_hold();
        int finish_hold();
        int prepare_actions();
        int do_action();
        int undo_action();
	void cancel_action();
        int jog();
	
private :
	Sheet* 		m_sheet;
	AudioClip* 	m_clip;
        TimeRef 	m_originalTrackStartLocation;
        TimeRef 	m_posDiff;
        TimeRef 	m_oldOppositeEdge;
        Track* 		m_originTrack;
        Track* 		m_targetTrack;
	QString		m_actionType;
	
	struct Data {
		int 		origXPos;
		int 		hScrollbarValue;
		TimeRef		xoffset;
		AudioClip* 	newclip;
		SheetView* 	sv;
		AudioClipView*	view;
		TrackView*	origTrackView;
		QPoint		origPos;
		TimeRef 	origTrackStartLocation;
		TimeRef 	origTrackEndLocation;
		bool 		resync;
		bool		bypassjog;
		QPoint		jogBypassPos;
	};
			

	Data* d;

	void init_data(bool isCopy=false);
	void calculate_snap_diff(TimeRef& leftlocation, TimeRef rightlocation);

	
public slots:
	void next_snap_pos(bool autorepeat);
	void prev_snap_pos(bool autorepeat);
        void move_to_start(bool autorepeat);
        void move_to_end(bool autorepeat);
	
private slots:
	void audioclip_added(AudioClip* clip);
};

#endif
