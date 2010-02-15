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

$Id: TrackPanelView.h,v 1.1 2008/01/21 16:17:30 r_sijrier Exp $
*/

#ifndef TRACK_PANEL_VIEW_H
#define TRACK_PANEL_VIEW_H

#include "ViewItem.h"

class ProcessingData;
class Track;
class ProcessingDataView;
class PDPanelView;
class TrackView;
class TrackPanelViewPort;
class PanelLed;
class TrackPanelView;
class SubGroupView;

class TrackPanelGain : public ViewItem
{
	Q_OBJECT

public:
        TrackPanelGain(PDPanelView* parent, ProcessingData* track);
	TrackPanelGain(){}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void set_width(int width);

public slots:
	Command* gain_increment();
	Command* gain_decrement();
	
private:
        ProcessingData* m_pd;
};

class TrackPanelPan : public ViewItem
{
	Q_OBJECT
	
public:
        TrackPanelPan(PDPanelView* parent, ProcessingData* track);
	TrackPanelPan(){}
	

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void set_width(int width);

public slots:
	Command* pan_left();
	Command* pan_right();

private:
        ProcessingData* m_pd;
};



class TrackPanelLed : public ViewItem
{
	Q_OBJECT
public:
        TrackPanelLed(PDPanelView* view, ProcessingData* pd, const QString& name, const QString& toggleslot);
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void set_bounding_rect(QRectF rect);

private:
        ProcessingData* m_pd;
        QString m_name;
	QString m_toggleslot;
	bool m_isOn;

public slots:
        void ison_changed(bool isOn);
	
	Command* toggle();
};

class TrackPanelBus : public ViewItem
{
	Q_OBJECT
public:
        TrackPanelBus(PDPanelView* view, ProcessingData* track, int busType);
	TrackPanelBus(){}
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	enum { BUSIN, BUSOUT };

private:
        ProcessingData*	m_pd;
        int	m_type;
	QString m_busName;
	QPixmap m_pix;

public slots:
        void bus_changed();

        Command* select_bus();
};


class PDPanelView : public ViewItem
{
	Q_OBJECT

public:
        PDPanelView(ProcessingDataView* trackView);
        ~PDPanelView();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void calculate_bounding_rect();
	
	
protected:
        ProcessingData*		m_pd;
        ProcessingDataView*	m_processingDataView;
	TrackPanelViewPort*	m_viewPort;
	TrackPanelGain*		m_gainView;
	TrackPanelPan*		m_panView;
	
        TrackPanelLed*          m_muteLed;
        TrackPanelLed*          m_soloLed;
	
        TrackPanelBus*          m_inBus;
        TrackPanelBus*  	m_outBus;

	void draw_panel_name(QPainter* painter);
        virtual void layout_panel_items() = 0;

private slots:
	void update_gain();
	void update_pan();
        void update_name();
};



class TrackPanelView : public PDPanelView
{
        Q_OBJECT

public:
        TrackPanelView(TrackView* trackView);
        ~TrackPanelView();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        Track* get_track() const {return (Track*)m_pd;}

protected:
        void layout_panel_items();


private:
        TrackView*	m_tv;
        TrackPanelLed*  m_recLed;
};


class SubGroupPanelView : public PDPanelView
{
        Q_OBJECT

public:
        SubGroupPanelView(SubGroupView* trackView);
        ~SubGroupPanelView();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


protected:
        void layout_panel_items();


private:
};


#endif

//eof
 
