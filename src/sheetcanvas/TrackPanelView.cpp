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

#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QMenu>
#include <QAction>
#include <QStringList>

#include "AudioBus.h"
#include "AudioDevice.h"
#include "TConfig.h"
#include "TrackPanelView.h"
#include "TTrackLaneView.h"
#include "AudioTrackView.h"
#include "TBusTrackView.h"
#include "SheetView.h"
#include <Themer.h>
#include "TrackPanelViewPort.h"
#include <AudioTrack.h>
#include "TrackPanelView.h"
#include <Utils.h>
#include <Mixer.h>
#include <Gain.h>
#include <TrackPan.h>
#include "Project.h"
#include "ProjectManager.h"
#include "Sheet.h"
#include "TBusTrack.h"
#include "Track.h"
#include "TMainWindow.h"
#include "VUMeterView.h"
#include "TKnobView.h"
		
#include <Debugger.h>

#define MICRO_HEIGHT 35
#define SMALL_HEIGHT 65

const int LED_WIDTH = 16;
const int LED_HEIGHT = 16;
const int LED_Y_POS = 24;
const int VU_WIDTH = 8;
const int GAIN_Y_POS = 48;
const int INDENT = 10;


TrackPanelView::TrackPanelView(TrackView* view)
        : ViewItem(0, view)
{
        PENTERCONS;

        m_sv = view->get_sheetview();

        m_trackView = view;
	m_viewPort = m_sv->get_trackpanel_view_port();
        m_track = m_trackView->get_track();

	m_infoLed = new TrackPanelLed(this, view, "I", "edit_properties");
	m_soloLed = new TrackPanelLed(this, m_track, "S", "solo");
	m_muteLed = new TrackPanelLed(this, m_track, "M", "mute");
	m_preLedButton = new TrackPanelLed(this, m_track, "P", "toggle_presend");
	m_panKnob = new TPanKnobView(this, m_track);

	m_infoLed->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));
	m_muteLed->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));
	m_soloLed->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));
	m_preLedButton->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));

	m_ledViews.insert(10, m_muteLed);
	m_ledViews.insert(11, m_soloLed);
	m_ledViews.insert(1, m_infoLed);

        if (m_track->is_solo()) {
                m_soloLed->ison_changed(true);
        }
        if (m_track->is_muted()) {
                m_muteLed->ison_changed(true);
        }
	if (m_track->presend_on()) {
		m_preLedButton->ison_changed(true);
	}

        m_vuMeterView = new VUMeterView(this, m_track);

        m_viewPort->scene()->addItem(this);

        connect(m_track, SIGNAL(soloChanged(bool)), m_soloLed, SLOT(ison_changed(bool)));
        connect(m_track, SIGNAL(muteChanged(bool)), m_muteLed, SLOT(ison_changed(bool)));
	connect(m_track, SIGNAL(preSendChanged(bool)), m_preLedButton, SLOT(ison_changed(bool)));

        connect(m_track, SIGNAL(stateChanged()), this, SLOT(update_gain()));
        connect(m_track, SIGNAL(panChanged()), this, SLOT(update_pan()));

        connect(m_track, SIGNAL(stateChanged()), this, SLOT(update_name()));
        connect(m_track, SIGNAL(activeContextChanged()), this, SLOT(active_context_changed()));

        connect(themer(), SIGNAL(themeLoaded()), this, SLOT(theme_config_changed()));

        setCursor(themer()->get_cursor("Track"));
}

TrackPanelView::~TrackPanelView( )
{
        PENTERDES;
}


void TrackPanelView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
        Q_UNUSED(widget);

        int xstart = (int)option->exposedRect.x();
        int pixelcount = (int)option->exposedRect.width();

        QRectF rect= option->exposedRect;
        // detect if only vu's are displayed, if so, do nothing.
        if (rect.left() > 180) {
//                printf("height, width: %f, %f\n", rect.height(), rect.width());
                return;
        }

        PENTER3;


        if (m_trackView->is_moving() || m_track->has_active_context()) {
                QColor color = themer()->get_color("Track:mousehover");
                painter->fillRect(boundingRect(), color);
        }



        if (m_trackView->m_topborderwidth > 0) {
                QColor color = themer()->get_color("Track:cliptopoffset");
                painter->fillRect(xstart, 0, pixelcount, m_trackView->m_topborderwidth, color);
        }

        if (m_trackView->m_bottomborderwidth > 0) {
                QColor color = themer()->get_color("Track:clipbottomoffset");
		painter->fillRect(xstart, m_trackView->get_total_height() - m_trackView->m_bottomborderwidth, pixelcount, m_trackView->m_bottomborderwidth, color);
        }

	painter->fillRect(m_viewPort->width() - 3, 0, 3, m_trackView->get_total_height() - 1, themer()->get_color("TrackPanel:trackseparation"));

        if (xstart < 180) {
                draw_panel_name(painter);
        }
}

void TrackPanelView::update_name()
{
        update();
}

void TrackPanelView::update_gain()
{
//        m_gainView->update();
}


void TrackPanelView::update_pan()
{
//        m_panView->update();
}


void TrackPanelView::draw_panel_name(QPainter* painter)
{
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);

	QColor color = themer()->get_color("TrackPanel:header:background");
	painter->setPen(color.darker(180));
	painter->setBrush(color);
	int corner = 5;
	painter->drawRoundedRect(INDENT, 3, 175, 15, corner, corner);

        painter->setPen(themer()->get_color("TrackPanel:text"));
        painter->setFont(themer()->get_font("TrackPanel:fontscale:name"));
	painter->drawText(INDENT + 8, 14, m_track->get_name());

	painter->restore();
}


void TrackPanelView::calculate_bounding_rect()
{
        prepareGeometryChange();
	m_boundingRect = QRectF(0, 0, 200, m_trackView->get_total_height());
        layout_panel_items();
}

void TrackPanelView::layout_panel_items()
{
        int height =  m_sv->get_track_height(m_track);
        int adjust = 0;

        Qt::Orientation orientation = (Qt::Orientation)config().get_property("Themer", "VUOrientation", Qt::Vertical).toInt();
        if (orientation == Qt::Vertical) {
                m_vuMeterView->set_bounding_rect(QRectF(0, 0, VU_WIDTH, height - 4));
                m_vuMeterView->setPos(m_boundingRect.width() - VU_WIDTH - 5, 2);
        } else {
                adjust = 14;
		m_vuMeterView->set_bounding_rect(QRectF(0, 0, 140, 16));
		m_vuMeterView->setPos(10, GAIN_Y_POS);
        }

	m_panKnob->setPos(165, GAIN_Y_POS - 2);


	int ledViewXPos = INDENT;
	int ledSpacing = 8;
	foreach(ViewItem* ledView, m_ledViews) {
		ledView->setPos(ledViewXPos, LED_Y_POS);
		ledViewXPos += ledView->boundingRect().width() + ledSpacing;
	}

	int preLedXPos = 135;
	m_preLedButton->setPos(preLedXPos, LED_Y_POS);

        if ( (m_vuMeterView->pos().y() + m_vuMeterView->boundingRect().height()) >= height) {
                m_vuMeterView->hide();
		m_panKnob->hide();
        } else {
                m_vuMeterView->show();
		m_panKnob->show();
        }
}

void TrackPanelView::theme_config_changed()
{
        m_vuMeterView->update_orientation();
        layout_panel_items();
	foreach(ViewItem* ledViews, m_ledViews) {
		ledViews->update();
	}
}


AudioTrackPanelView::AudioTrackPanelView(AudioTrackView* trackView)
        : TrackPanelView(trackView)
{
	PENTERCONS;

        m_tv = trackView;
	m_recLed = new TrackPanelLed(this, m_track, "R", "toggle_arm");
        m_recLed->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));

	m_ledViews.insert(12, m_recLed);

        if (m_tv->get_track()->armed()) {
                m_recLed->ison_changed(true);
        }

        connect(m_tv->get_track(), SIGNAL(armedChanged(bool)), m_recLed, SLOT(ison_changed(bool)));
}

AudioTrackPanelView::~AudioTrackPanelView( )
{
	PENTERDES;
}


void AudioTrackPanelView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
        TrackPanelView::paint(painter, option, widget);
}

void AudioTrackPanelView::layout_panel_items()
{
        TrackPanelView::layout_panel_items();
}


TBusTrackPanelView::TBusTrackPanelView(TBusTrackView* view)
        : TrackPanelView(view)
{
        PENTERCONS;

//        m_panView->hide();
}

TBusTrackPanelView::~TBusTrackPanelView( )
{
        PENTERDES;
}


void TBusTrackPanelView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
        Q_UNUSED(widget);
        Q_UNUSED(option);

        int xstart = (int)option->exposedRect.x();
        int pixelcount = (int)option->exposedRect.width();

        QColor color = themer()->get_color("BusTrack:background");
        painter->fillRect(xstart, m_trackView->m_topborderwidth, pixelcount, m_sv->get_track_height(m_track) - m_trackView->m_bottomborderwidth, color);

        TrackPanelView::paint(painter, option, widget);
}


void TBusTrackPanelView::layout_panel_items()
{
        TrackPanelView::layout_panel_items();
}


TTrackLanePanelView::TTrackLanePanelView(TTrackLaneView* laneView)
	: ViewItem(laneView, laneView)
{
	PENTERCONS;
	m_laneView = laneView;
	m_ignoreContext = true;
	setZValue(laneView->zValue() + 200);
}

TTrackLanePanelView::~TTrackLanePanelView( )
{
	PENTERDES;
}

#include "CurveView.h"

void TTrackLanePanelView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(widget);
	Q_UNUSED(option);

//	int xstart = (int)option->exposedRect.x();
//	int pixelcount = (int)option->exposedRect.width();

	QColor color = themer()->get_color("BusTrack:background");
	painter->fillRect(m_boundingRect, color);

	painter->setPen(themer()->get_color("TrackPanel:text"));
	painter->setFont(themer()->get_font("TrackPanel:fontscale:name"));
	painter->drawText(20, 20, m_parentViewItem->get_name());


	painter->setPen(QColor(100, 100, 100, 100));
	int xpos = 50;
	int halfFontHeight = 5;
	int height = m_laneView->get_height() - CurveView::BORDER_MARGIN;
	int topBorderMargin = CurveView::BORDER_MARGIN / 2;

	int y = (1.0 - dB_to_scale_factor(0.0f)) * height + topBorderMargin;
	int textY = y + halfFontHeight;
	painter->drawLine(m_boundingRect.width() - 4, y, m_boundingRect.width(), y);
	painter->drawText(m_boundingRect.width() - xpos, textY, "  0 dB");

	y = (1.0 - dB_to_scale_factor(-6.0f)) * height + topBorderMargin;
	textY = y + halfFontHeight;
	painter->drawLine(m_boundingRect.width() - 4, y, m_boundingRect.width(), y);
	painter->drawText(m_boundingRect.width() - xpos, textY, " - 6 dB");

	y = (1.0 - dB_to_scale_factor(-24.0f)) * height + topBorderMargin;
	textY = y + halfFontHeight;
	painter->drawLine(m_boundingRect.width() - 4, y, m_boundingRect.width(), y);
	painter->drawText(m_boundingRect.width() - xpos, textY, " -24 dB");
}

void TTrackLanePanelView::calculate_bounding_rect()
{
	prepareGeometryChange();
	m_boundingRect = QRectF(0, 0, 175, m_laneView->get_height());
}


TrackPanelGain::TrackPanelGain(TrackPanelView *parent, Track *track)
        : ViewItem(parent, 0)
        , m_track(track)
{
}

void TrackPanelGain::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);
        const int height = 6;

        QColor color = themer()->get_color("TrackPanel:slider:background");
        if (has_active_context()) {
                color = color.light(110);
        }


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
	
	painter->save();
	
	painter->setPen(themer()->get_color("TrackPanel:text"));
	painter->setFont(themer()->get_font("TrackPanel:fontscale:gain"));
        painter->drawText(0, height + 1, "Gain");
        painter->fillRect(30, 0, sliderWidth, height, color);

        color = QColor(cr,0,cb);
        if (has_active_context()) {
		color = color.light(140);
	}

        painter->fillRect(31, 1, sliderdbx, height-1, m_gradient2D);
	painter->drawText(sliderWidth + 35, height, sgain);
	
        painter->setPen(themer()->get_color("TrackPanel:slider:border"));
        painter->drawRect(30, 0, sliderWidth, height);

        painter->restore();
}

void TrackPanelGain::set_width(int width)
{
	m_boundingRect = QRectF(0, 0, width, 9);
        load_theme_data();
}

void TrackPanelGain::load_theme_data()
{
        float zeroDB = 1.0 - 100.0/115.0;  // 0 dB position
        float msixDB = 1.0 -  80.0/115.0;  // -6 dB position
        float smooth = themer()->get_property("GainSlider:smoothfactor", 0.05).toDouble();

        m_gradient2D.setColorAt(0.0,           themer()->get_color("GainSlider:6db"));
        m_gradient2D.setColorAt(zeroDB-smooth, themer()->get_color("GainSlider:6db"));
        m_gradient2D.setColorAt(zeroDB+smooth, themer()->get_color("GainSlider:0db"));
        m_gradient2D.setColorAt(msixDB-smooth, themer()->get_color("GainSlider:0db"));
        m_gradient2D.setColorAt(msixDB+smooth, themer()->get_color("GainSlider:-6db"));
        m_gradient2D.setColorAt(1.0,           themer()->get_color("GainSlider:-60db"));


        m_gradient2D.setStart(QPointF(m_boundingRect.width() - 40, 0));
        m_gradient2D.setFinalStop(31, 0);

}

TCommand* TrackPanelGain::gain_increment()
{
        m_track->set_gain(m_track->get_gain() + 0.05);
	return 0;
}

TCommand* TrackPanelGain::gain_decrement()
{
        m_track->set_gain(m_track->get_gain() - 0.05);
	return 0;
}


TrackPanelLed::TrackPanelLed(TrackPanelView* view, QObject *obj, const QString& name, const QString& toggleslot)
        : ViewItem(view, 0)
	, m_name(name)
	, m_toggleslot(toggleslot)
	, m_isOn(false)
{
	m_object = obj;
}

void TrackPanelLed::paint(QPainter* painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);

	painter->save();
	
	painter->setRenderHint(QPainter::Antialiasing);
	
	if (m_isOn) {
		QColor background = themer()->get_color("TrackPanel:led:inactive");
		QColor color = themer()->get_color("TrackPanel:" + m_name + "led");
                if (has_active_context()) {
			color = color.light(110);
		}
		
		painter->setPen(color);
		painter->setBrush(background);
		painter->drawEllipse(m_boundingRect);

		painter->setFont(themer()->get_font("TrackPanel:fontscale:led"));
		
                QString shortString = m_name.left(1).toUpper();
                painter->drawText(m_boundingRect, Qt::AlignCenter, shortString);
	} else {
		QColor color = themer()->get_color("TrackPanel:led:inactive");
                if (has_active_context()) {
			color = color.light(110);
		}
		
		painter->setPen(themer()->get_color("TrackPanel:led:margin:inactive"));
		painter->setBrush(color);
		painter->drawEllipse(m_boundingRect);
		
		painter->setFont(themer()->get_font("TrackPanel:fontscale:led"));
		painter->setPen(themer()->get_color("TrackPanel:led:font:inactive"));

		if (m_name == "I") {
			painter->setPen(themer()->get_color("TrackPanel:led:margin:inactive"));
			painter->setBrush(QColor(255, 255, 255, 50));
			painter->drawEllipse(m_boundingRect);
			painter->setPen(themer()->get_color("TrackPanel:led:font:inactive"));
			painter->drawText(m_boundingRect, Qt::AlignCenter, m_name);
		} else {
			painter->drawText(m_boundingRect, Qt::AlignCenter, m_name);
		}
	}
	
	painter->restore();
	
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

TCommand * TrackPanelLed::toggle()
{
	TCommand* com;
	QMetaObject::invokeMethod(m_object, QS_C(m_toggleslot), Qt::DirectConnection, Q_RETURN_ARG(TCommand*, com));
	Q_ASSERT(!com);

	return 0;
}
