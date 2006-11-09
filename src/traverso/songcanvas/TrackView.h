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

$Id: TrackView.h,v 1.2 2006/11/09 15:45:42 r_sijrier Exp $
*/

#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

#include "ViewItem.h"
#include "Track.h"

class Track;
class AudioClipView;
class TrackPanelView;

class TrackView : public ViewItem
{
	Q_OBJECT

public:
	TrackView(SongView* sv, Track* track);
	~TrackView();
	
	enum {Type = UserType + 2};
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	Track* get_track() const;
	
	int get_clipview_y_offset();
	int get_clipview_height();
	void move_to(int x, int y);
	
// 	void add_clip_view(AudioClipView* view);
	int type() const;
	
private:
	Track*			m_track;
	QList<AudioClipView* >	m_clipViews;
	TrackPanelView*		m_panel;
	int			m_clipViewYOfsset;

public slots:
	Command* edit_properties();
	Command* add_new_plugin();	

private slots:
	void add_new_audioclipview(AudioClip* clip);
	void remove_audioclipview(AudioClip* clip);
};


inline int TrackView::type() const {return Type;}

#endif

//eof
