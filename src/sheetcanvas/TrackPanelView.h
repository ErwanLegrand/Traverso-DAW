/*
Copyright (C) 2005-2010 Remon Sijrier

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

#ifndef TRACK_PANEL_VIEW_H
#define TRACK_PANEL_VIEW_H

#include "ViewItem.h"

class Track;
class AudioTrack;
class TrackView;
class TrackPanelView;
class AudioTrackView;
class TrackPanelViewPort;
class PanelLed;
class AudioTrackPanelView;
class SubGroupView;
class VUMeterView;

class TrackPanelGain : public ViewItem
{
	Q_OBJECT

public:
        TrackPanelGain(TrackPanelView* parent, Track* track);
	TrackPanelGain(){}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void set_width(int width);
        void load_theme_data();

public slots:
	Command* gain_increment();
	Command* gain_decrement();
	
private:
        Track* m_track;
        QLinearGradient	m_gradient2D;
};

class TrackPanelPan : public ViewItem
{
	Q_OBJECT
	
public:
        TrackPanelPan(TrackPanelView* parent, Track* track);
	TrackPanelPan(){}
	

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void set_width(int width);
        void load_theme_data();

public slots:
	Command* pan_left();
	Command* pan_right();

private:
        Track* m_track;
        QLinearGradient	m_gradient2D;
};



class TrackPanelLed : public ViewItem
{
	Q_OBJECT
public:
        TrackPanelLed(TrackPanelView* view, Track* track, const QString& name, const QString& toggleslot);
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void set_bounding_rect(QRectF rect);

private:
        Track*  m_track;
        QString m_name;
	QString m_toggleslot;
        bool    m_isOn;

public slots:
        void ison_changed(bool isOn);
	
	Command* toggle();
};

class TrackPanelBus : public ViewItem
{
	Q_OBJECT
public:
        TrackPanelBus(TrackPanelView* view, Track* track, int busType);
	TrackPanelBus(){}
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	enum { BUSIN, BUSOUT };

private:
        Track*	m_track;
        int	m_type;
	QString m_busName;
	QPixmap m_pix;

public slots:
        void bus_changed();
};


class TrackPanelView : public ViewItem
{
	Q_OBJECT

public:
        TrackPanelView(TrackView* trackView);
        ~TrackPanelView();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void calculate_bounding_rect();
	
        TrackView* get_track_view() const {return m_trackView;}

protected:
        Track*                  m_track;
        TrackView*              m_trackView;
	TrackPanelViewPort*	m_viewPort;
	TrackPanelGain*		m_gainView;
	TrackPanelPan*		m_panView;
	
        TrackPanelLed*          m_muteLed;
        TrackPanelLed*          m_soloLed;
	
        TrackPanelBus*          m_inBus;
        TrackPanelBus*  	m_outBus;

        VUMeterView*            m_vuMeterView;

	void draw_panel_name(QPainter* painter);
        virtual void layout_panel_items();

private slots:
        void update_gain();
	void update_pan();
        void update_name();
        void theme_config_changed();
        void active_context_changed() {update();}
};



class AudioTrackPanelView : public TrackPanelView
{
        Q_OBJECT

public:
        AudioTrackPanelView(AudioTrackView* trackView);
        ~AudioTrackPanelView();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
        void layout_panel_items();


private:
        AudioTrackView*	m_tv;
        TrackPanelLed*  m_recLed;
};


class SubGroupPanelView : public TrackPanelView
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
 
