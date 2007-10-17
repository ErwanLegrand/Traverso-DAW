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

$Id: TrackPanelView.cpp,v 1.30 2007/10/17 14:46:18 r_sijrier Exp $
*/

#include <QGraphicsScene>
#include <QFont>
#include "TrackPanelView.h"
#include "TrackView.h"
#include "SongView.h"
#include <Themer.h>
#include "TrackPanelViewPort.h"
#include <Track.h>
#include "TrackPanelView.h"
#include <Utils.h>
#include <Mixer.h>
#include <Gain.h>
#include <TrackPan.h>
		
#include <Debugger.h>

#define MICRO_HEIGHT 35
#define SMALL_HEIGHT 65

TrackPanelView::TrackPanelView(TrackView* trackView)
	: ViewItem(0, trackView)
	, m_tv(trackView)
{
	PENTERCONS;
	
	m_viewPort = m_tv->get_songview()->get_trackpanel_view_port();
	m_track = m_tv->get_track();
	
	m_gainView = new TrackPanelGain(this, m_track);
	m_gainView->set_width(m_viewPort->width() - 20);
	
	m_panView = new TrackPanelPan(this, m_track);
	m_panView->set_width(m_viewPort->width() - 20);
	
	recLed = new TrackPanelLed(this, "rec", "toggle_arm");
	soloLed = new TrackPanelLed(this, "solo", "solo");
	muteLed = new TrackPanelLed(this, "mute", "mute");
	
	if (m_track->armed()) {
		recLed->ison_changed(true);
	}
	if (m_track->is_solo()) {
		soloLed->ison_changed(true);
	}
	if (m_track->is_muted()) {
		muteLed->ison_changed(true);
	}
	
	inBus = new TrackPanelBus(this, m_track, TrackPanelBus::BUSIN);
	outBus = new TrackPanelBus(this, m_track, TrackPanelBus::BUSOUT);
	
	m_viewPort->scene()->addItem(this);

	
	m_boundingRect = QRectF(0, 0, 200, m_track->get_height());
	
	layout_panel_items();
	
	connect(m_track, SIGNAL(armedChanged(bool)), recLed, SLOT(ison_changed(bool)));
	connect(m_track, SIGNAL(soloChanged(bool)), soloLed, SLOT(ison_changed(bool)));
	connect(m_track, SIGNAL(muteChanged(bool)), muteLed, SLOT(ison_changed(bool)));
	
	connect(m_track, SIGNAL(gainChanged()), this, SLOT(update_gain()));
	connect(m_track, SIGNAL(panChanged()), this, SLOT(update_pan()));
	
	connect(m_track, SIGNAL(inBusChanged()), inBus, SLOT(bus_changed()));
	connect(m_track, SIGNAL(outBusChanged()), outBus, SLOT(bus_changed()));
	
	connect(m_track, SIGNAL(stateChanged()), this, SLOT(update_track_name()));
	
	
// 	setFlags(ItemIsSelectable | ItemIsMovable);
// 	setAcceptsHoverEvents(true);
	setCursor(themer()->get_cursor("Track"));
}

TrackPanelView::~TrackPanelView( )
{
	PENTERDES;
}


void TrackPanelView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(widget);
	Q_UNUSED(option);
	
	int xstart = (int)option->exposedRect.x();
	int pixelcount = (int)option->exposedRect.width();
	
	if (m_tv->m_topborderwidth > 0) {
		QColor color = themer()->get_color("Track:cliptopoffset");
		painter->fillRect(xstart, 0, pixelcount, m_tv->m_topborderwidth, color);
	}
	
	if (m_tv->m_paintBackground) {
		QColor color = themer()->get_color("Track:background");
		painter->fillRect(xstart, m_tv->m_topborderwidth, pixelcount, m_track->get_height() - m_tv->m_bottomborderwidth, color);
	}
	
	if (m_tv->m_bottomborderwidth > 0) {
		QColor color = themer()->get_color("Track:clipbottomoffset");
		painter->fillRect(xstart, m_track->get_height() - m_tv->m_bottomborderwidth, pixelcount, m_tv->m_bottomborderwidth, color);
	}

	// Track / track panel seperator is painted in TrackPanelViewPort... not the best place perhaps ?
	painter->fillRect(m_viewPort->width() - 3, 0, 3, m_track->get_height() - 1, themer()->get_color("TrackPanel:trackseparation"));
	
	if (xstart < 180) {
		draw_panel_track_name(painter);
	}
}

void TrackPanelView::update_track_name()
{
	update();
}

void TrackPanelView::update_gain()
{
	m_gainView->update();
}


void TrackPanelView::update_pan()
{
	m_panView->update();
}


void TrackPanelView::draw_panel_track_name(QPainter* painter)
{
	QString title = QString::number(m_track->get_sort_index() + 1) + "  " + m_track->get_name();
	
	if (m_track->get_height() < SMALL_HEIGHT) {
		QFontMetrics fm(themer()->get_font("TrackPanel:fontscale:name"));
		title = fm.elidedText(title, Qt::ElideMiddle, 90);
	}
	
	painter->setPen(themer()->get_color("TrackPanel:text"));
	painter->setFont(themer()->get_font("TrackPanel:fontscale:name"));
	painter->drawText(4, 12, title);
}


void TrackPanelView::calculate_bounding_rect()
{
	prepareGeometryChange();
	m_boundingRect = QRectF(0, 0, 200, m_track->get_height());
	layout_panel_items();
}

void TrackPanelView::layout_panel_items()
{
	int height =  m_track->get_height();
	
	m_gainView->setPos(10, 39);
	m_panView->setPos(10, 54);
	
	inBus->setPos(10, 73);
	outBus->setPos(100, 73);
	
	if (height < SMALL_HEIGHT) {
		m_gainView->setPos(10, 20);
		m_panView->setPos(10, 36);
	} else {
		muteLed->setPos(70, 19);
		soloLed->setPos(118, 19);
		recLed->setPos(162, 19);
		muteLed->set_bounding_rect(QRectF(0, 0, 41, 14)); 
		soloLed->set_bounding_rect(QRectF(0, 0, 38, 14)); 
		recLed->set_bounding_rect(QRectF(0, 0, 30, 14)); 
		
		m_gainView->setPos(10, 39);
		m_panView->setPos(10, 54);
	}
	
	if (height < SMALL_HEIGHT) {
		muteLed->setPos(90, 1.5);
		soloLed->setPos(132, 1.5);
		recLed->setPos(166, 1.5);
		muteLed->set_bounding_rect(QRectF(0, 0, 39, 12)); 
		soloLed->set_bounding_rect(QRectF(0, 0, 31, 12)); 
		recLed->set_bounding_rect(QRectF(0, 0, 27, 12)); 
	}
	
	if ((inBus->pos().y() + inBus->boundingRect().height()) >= height) {
		inBus->hide();
		outBus->hide();
	} else {
		inBus->show();
		outBus->show();
	}
	
	if ( (m_panView->pos().y() + m_panView->boundingRect().height()) >= height) {
		m_panView->hide();
	} else {
		m_panView->show();
	}
	
	if ( (m_gainView->pos().y() + m_panView->boundingRect().height()) >= height) {
		m_gainView->hide();
	} else {
		m_gainView->show();
	}
}


TrackPanelGain::TrackPanelGain(TrackPanelView* parent, Track * track)
	: ViewItem(parent, track)
	, m_track(track)
{
	setAcceptsHoverEvents(true);

}

void TrackPanelGain::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);
	const int height = 9;

	int sliderWidth = (int)m_boundingRect.width() - 75;
	float gain = m_track->get_gain();
	QString sgain = coefficient_to_dbstring(gain);
	float db = coefficient_to_dB(gain);

	if (db < -60) {
		db = -60;
	}
	int sliderdbx =  (int) (sliderWidth - (sliderWidth*0.3)) - (int) ( ( (-1 * db) / 60 ) * sliderWidth);
	if (sliderdbx < 0) {
		sliderdbx = 0;
	}
	if (db > 0) {
		sliderdbx =  (int)(sliderWidth*0.7) + (int) ( ( db / 6 ) * (sliderWidth*0.3));
	}

	int cr = (gain >= 1 ? 30 + (int)(100 * gain) : (int)(50 * gain));
	int cb = ( gain < 1 ? 150 + (int)(50 * gain) : abs((int)(10 * gain)) );
	
	painter->setPen(themer()->get_color("TrackPanel:text"));
	painter->setFont(themer()->get_font("TrackPanel:fontscale:gain"));
	painter->drawText(0, height + 1, "GAIN");
	painter->drawRect(30, 0, sliderWidth, height);
	
	bool mousehover = (option->state & QStyle::State_MouseOver);
	QColor color(cr,0,cb);
	if (mousehover) {
		color = color.light(140);
	}
	painter->fillRect(31, 1, sliderdbx, height-1, color);
	painter->drawText(sliderWidth + 35, height, sgain);
}

void TrackPanelGain::set_width(int width)
{
	m_boundingRect = QRectF(0, 0, width, 9);
}

Command* TrackPanelGain::gain_increment()
{
	m_track->set_gain(m_track->get_gain() + 0.05);
	return 0;
}

Command* TrackPanelGain::gain_decrement()
{
	m_track->set_gain(m_track->get_gain() - 0.05);
	return 0;
}

TrackPanelPan::TrackPanelPan(TrackPanelView* parent, Track * track)
	: ViewItem(parent, track)
	, m_track(track)
{
	setAcceptsHoverEvents(true);
}

void TrackPanelPan::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);

	bool mousehover = (option->state & QStyle::State_MouseOver);
	
	QColor color = themer()->get_color("TrackPanel:slider:background");
	if (mousehover) {
		color = color.light(110);
	}
	
	const int PAN_H = 9;

	int sliderWidth = (int)m_boundingRect.width() - 75;
	float v;
	//	int y;
	QString s, span;
	painter->setPen(themer()->get_color("TrackPanel:text"));
	painter->setFont(themer()->get_font("TrackPanel:fontscale:pan"));

	painter->drawText(0, PAN_H + 1, "PAN");

	v = m_track->get_pan();
	span = QByteArray::number(v,'f',1);
	s = ( v > 0 ? QString("+") + span :  span );
	painter->fillRect(30, 0, sliderWidth, PAN_H, color);
	painter->drawRect(30, 0, sliderWidth, PAN_H);
	int pm= 31 + sliderWidth/2;
	int z = abs((int)(v*(sliderWidth/2)));
	int c = abs((int)(255*v));
	if (v>=0) {
		painter->fillRect(pm, 1, z, PAN_H-1, QColor(c,0,0));
	} else {
		painter->fillRect(pm-z, 1, z, PAN_H-1, QColor(c,0,0));
	}
	painter->drawText(30 + sliderWidth + 10, PAN_H + 1, s);
}

void TrackPanelPan::set_width(int width)
{
	m_boundingRect = QRectF(0, 0, width, 9);
}

Command* TrackPanelPan::pan_left()
{
	m_track->set_pan(m_track->get_pan() - 0.05);
	return 0;
}

Command* TrackPanelPan::pan_right()
{
	m_track->set_pan(m_track->get_pan() + 0.05);
	return 0;
}



TrackPanelLed::TrackPanelLed(TrackPanelView* parent, const QString& name, const QString& toggleslot)
	: ViewItem(parent, 0)
	, m_name(name)
	, m_toggleslot(toggleslot)
	, m_isOn(false)
{
	m_track = parent->get_track();
	setAcceptsHoverEvents(true);
}

void TrackPanelLed::paint(QPainter* painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);
	
	bool mousehover = (option->state & QStyle::State_MouseOver);
	int roundfactor = 20;
	
	painter->setRenderHint(QPainter::Antialiasing);
	
	if (m_isOn) {
		QColor color = themer()->get_color("TrackPanel:" + m_name + "led");
		if (mousehover) {
			color = color.light(110);
		}
		
		painter->setPen(themer()->get_color("TrackPanel:led:margin:active"));
		painter->setBrush(color);
		painter->drawRoundRect(m_boundingRect, roundfactor, roundfactor);
		
		painter->setFont(themer()->get_font("TrackPanel:fontscale:led"));
		painter->setPen(themer()->get_color("TrackPanel:led:font:active"));
		
		painter->drawText(m_boundingRect, Qt::AlignCenter, m_name);
	} else {
		QColor color = themer()->get_color("TrackPanel:led:inactive");
		if (mousehover) {
			color = color.light(110);
		}
		
		painter->setPen(themer()->get_color("TrackPanel:led:margin:inactive"));
		painter->setBrush(color);
		painter->drawRoundRect(m_boundingRect, roundfactor, roundfactor);
		
		painter->setFont(themer()->get_font("TrackPanel:fontscale:led"));
		painter->setPen(themer()->get_color("TrackPanel:led:font:inactive"));
		
		painter->drawText(m_boundingRect, Qt::AlignCenter, m_name);
	}
	
}

void TrackPanelLed::set_bounding_rect(QRectF rect)
{
	prepareGeometryChange();
	m_boundingRect = rect;
}


void TrackPanelLed::ison_changed(bool isOn)
{
        m_isOn = isOn;
	update();
}

Command * TrackPanelLed::toggle()
{
	Command* com;
	QMetaObject::invokeMethod(m_track, QS_C(m_toggleslot), Qt::DirectConnection, Q_RETURN_ARG(Command*, com));
	return 0;
}



TrackPanelBus::TrackPanelBus(TrackPanelView* parent, Track* track, int type)
	: ViewItem(parent, 0)
	, m_track(track)
	, m_type(type)
{
	bus_changed();
	setAcceptsHoverEvents(true);
}

void TrackPanelBus::paint(QPainter* painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	QColor color = themer()->get_color("TrackPanel:bus:background");
	int roundfactor = 15;
	
	painter->setRenderHint(QPainter::Antialiasing);
	
	if (option->state & QStyle::State_MouseOver) {
		color = color.light(110);
	}
	 
	painter->setPen(themer()->get_color("TrackPanel:bus:margin"));
	painter->setBrush(color);
	painter->drawRoundRect(m_boundingRect, roundfactor, roundfactor);
	
	painter->setFont(themer()->get_font("TrackPanel:fontscale:led"));
	painter->setPen(themer()->get_color("TrackPanel:bus:font"));
	
	QString leftright = "";
	
	if ((m_type == BUSIN) && ! (m_track->capture_left_channel() && m_track->capture_right_channel()) ) {
		if (m_track->capture_left_channel()) {
			leftright = " L";
		} else {
			leftright = " R";
		}
	}
	
	painter->drawText(m_boundingRect.adjusted(15, 0, 0, 0), Qt::AlignCenter, m_busName + leftright);
			
	painter->drawPixmap(3, 3, m_pix);
}

void TrackPanelBus::bus_changed()
{
	QFontMetrics fm(themer()->get_font("TrackPanel:bus"));
	prepareGeometryChange();
	
	if (m_type == BUSIN) {
		m_busName =  m_track->get_bus_in();
		m_pix = find_pixmap(":/bus_in");
		m_boundingRect = m_pix.rect();
		m_boundingRect.setWidth(m_pix.rect().width() + fm.width(m_busName) + 10);
	} else {
		m_busName = m_track->get_bus_out();
		m_pix = find_pixmap(":/bus_out");
		m_boundingRect = m_pix.rect();
		m_boundingRect.setWidth(m_pix.rect().width() + fm.width(m_busName) + 10);
	}
	
	m_boundingRect.setHeight(m_boundingRect.height() + 6);
	
	update();
}



//eof
