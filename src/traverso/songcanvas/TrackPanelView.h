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

$Id: TrackPanelView.h,v 1.2 2007/01/11 20:11:26 r_sijrier Exp $
*/

#ifndef TRACK_PANEL_VIEW_H
#define TRACK_PANEL_VIEW_H

#include "ViewItem.h"

class Track;
class TrackView;
class TrackPanelViewPort;
class PanelLed;
class TrackPanelView;

class TrackPanelGain : public QGraphicsItem
{
public:
	TrackPanelGain(TrackPanelView* parent, Track* track);
	TrackPanelGain(){}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QRectF boundingRect() const {return m_boundingRect;}
	void set_width(int width);

private:
	QRectF	m_boundingRect;
	Track* m_track;
};

class TrackPanelPan : public QGraphicsItem
{
public:
	TrackPanelPan(TrackPanelView* parent, Track* track);
	TrackPanelPan(){}
	

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QRectF boundingRect() const {return m_boundingRect;}
	void set_width(int width);

private:
	QRectF	m_boundingRect;
	Track* m_track;
};



class TrackPanelLed : public ViewItem
{
	Q_OBJECT
public:
	TrackPanelLed(TrackPanelView* view, char* on, char* off);
	TrackPanelLed(){}
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
        QString onType;
        QString offType;
        bool m_isOn;

public slots:
        void ison_changed(bool isOn);
};

class TrackPanelBus : public ViewItem
{
	Q_OBJECT
public:
	TrackPanelBus(TrackPanelView* view, Track* track, char* busType);
	TrackPanelBus(){}
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
	Track*	m_track;
        QString m_type;
	QString m_busName;

public slots:
        void bus_changed();
};


class TrackPanelView : public ViewItem
{
	Q_OBJECT

public:
	TrackPanelView(TrackPanelViewPort* view, TrackView* trackView, Track* track);
	~TrackPanelView(){};

	enum {Type = UserType + 7};
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void calculate_bounding_rect();
	int type() const;
	
private:
	Track*			m_track;
	TrackView*		m_trackView;
	TrackPanelViewPort*	m_viewPort;
	TrackPanelGain*		m_gainView;
	TrackPanelPan*		m_panView;
	
	TrackPanelLed* muteLed;
	TrackPanelLed* soloLed;
	TrackPanelLed* recLed;
	
	TrackPanelBus*	inBus;
	TrackPanelBus*	outBus;
	
	QPixmap panelPixmap;

	void draw_panel_track_name(QPainter* painter);
	void draw_panel_head();

private slots:
	void update_gain();
	void update_pan();
};

inline int TrackPanelView::type() const {return Type;}


#endif

//eof
 
