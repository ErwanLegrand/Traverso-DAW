/*
    Copyright (C) 2005-2007 Remon Sijrier 
 
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

#ifndef TIME_LINE_VIEW_H
#define TIME_LINE_VIEW_H

#include "ViewItem.h"

#include <QTimer>

class SheetView;
class TimeLine;
class MarkerView;
class Marker;


class TimeLineView : public ViewItem
{
        Q_OBJECT
	Q_CLASSINFO("add_marker", tr("Add Marker"))
        Q_CLASSINFO("add_marker_at_playhead", tr("Add Marker at Playhead"))
        Q_CLASSINFO("add_marker_at_work_cursor", tr("Add Marker at Work Cursor"))
        Q_CLASSINFO("remove_marker", tr("Remove Marker"))
	Q_CLASSINFO("drag_marker", tr("Drag Marker"))
	Q_CLASSINFO("clear_markers", tr("Clear all Markers"))
	Q_CLASSINFO("playhead_to_marker", tr("Playhead to Marker"))

public:
        TimeLineView(SheetView* view);
        ~TimeLineView();
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void calculate_bounding_rect();
	void load_theme_data();
        void mouse_hover_move_event();
        QList<MarkerView*> get_marker_views() const { return m_markerViews;}

private:
	QList<MarkerView* > m_markerViews;
	TimeLine* 	m_timeline;
	MarkerView* 	m_blinkingMarker;
	QColor		m_blinkColor;

	QHash<nframes_t, QString>	m_zooms;	

	Command* add_marker_at(const TimeRef when);
        void update_softselected_marker(QPointF pos);
	
	
public slots:
        void hzoom_changed();
	
public slots:
	Command* add_marker();
        Command* add_marker_at_playhead();
        Command* add_marker_at_work_cursor();
        Command* remove_marker();
	Command* drag_marker();
	Command* clear_markers();
	Command* playhead_to_marker();

private slots:
	void add_new_marker_view(Marker* marker);
	void remove_marker_view(Marker* marker);
        void active_context_changed();

};

#endif

//eof
