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

class SongView;
class TimeLine;
class MarkerView;
class Marker;

class TimeLineView : public ViewItem
{
        Q_OBJECT
	Q_CLASSINFO("add_marker", tr("Add Marker"))
	Q_CLASSINFO("remove_marker", tr("Remove Marker"))
	Q_CLASSINFO("drag_marker", tr("Drag Marker"))

public:
        TimeLineView(SongView* view);
        ~TimeLineView();
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void calculate_bounding_rect();
	void load_theme_data();

protected:
	void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
	void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
	void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );

private:
	QList<MarkerView* > m_markerViews;
	TimeLine* 	m_timeline;
	MarkerView* 	m_blinkingMarker;
	QTimer		m_blinkTimer;
	QColor		m_blinkColor;
	int		m_samplerate;

	void update_softselected_marker(QPoint pos);
	
	
public slots:
        void hzoom_changed();
	
public slots:
	Command* add_marker();
	Command* remove_marker();
	Command* drag_marker();
	
private slots:
	void add_new_marker_view(Marker* marker);
	void remove_marker_view(Marker* marker);
};

#endif

//eof
