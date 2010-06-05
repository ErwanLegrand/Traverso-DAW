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
#include "Config.h"
#include "TrackPanelView.h"
#include "AudioTrackView.h"
#include "SubGroupView.h"
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
#include "SubGroup.h"
#include "Track.h"
#include "TSend.h"
#include "Interface.h"
#include "VUMeterView.h"
		
#include <Debugger.h>

#define MICRO_HEIGHT 35
#define SMALL_HEIGHT 65

const int LED_SPACING = 5;
const int LED_WIDTH = 17;
const int LED_HEIGHT = 12;
const int MUTE_X_POS = 120;
const int SOLO_X_POS = MUTE_X_POS + LED_SPACING + LED_WIDTH;
const int REC_X_POS = SOLO_X_POS + LED_SPACING + LED_WIDTH;
const int LED_Y_POS = 3;
const int VU_WIDTH = 8;
const int GAIN_X_POS = 10;
const int GAIN_Y_POS = 23;
const int PAN_X_POS = 10;
const int PAN_Y_POS = GAIN_Y_POS + 13;
const int INPUT_OUTPUT_BUTTON_Y_POS = PAN_Y_POS + 30;


TrackPanelView::TrackPanelView(TrackView* view)
        : ViewItem(0, view)
{
        PENTERCONS;

        m_trackView = view;
        m_viewPort = m_trackView->get_sheetview()->get_trackpanel_view_port();
        m_track = m_trackView->get_track();

        m_gainView = new TrackPanelGain(this, m_track);
        m_gainView->set_width(m_viewPort->width() - 30);

        m_panView = new TrackPanelPan(this, m_track);
        m_panView->set_width(m_viewPort->width() - 30);

        m_soloLed = new TrackPanelLed(this, m_track, "solo", "solo");
        m_muteLed = new TrackPanelLed(this, m_track, "mute", "mute");
        m_muteLed->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));
        m_soloLed->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));

        if (m_track->is_solo()) {
                m_soloLed->ison_changed(true);
        }
        if (m_track->is_muted()) {
                m_muteLed->ison_changed(true);
        }

        m_vuMeterView = new VUMeterView(this, m_track);

        m_inBus = new TrackPanelBus(this, m_track, TrackPanelBus::BUSIN);
        m_outBus = new TrackPanelBus(this, m_track, TrackPanelBus::BUSOUT);

        m_viewPort->scene()->addItem(this);

        m_boundingRect = QRectF(0, 0, 200, m_track->get_height());


        connect(m_track, SIGNAL(soloChanged(bool)), m_soloLed, SLOT(ison_changed(bool)));
        connect(m_track, SIGNAL(muteChanged(bool)), m_muteLed, SLOT(ison_changed(bool)));

        connect(m_track, SIGNAL(stateChanged()), this, SLOT(update_gain()));
        connect(m_track, SIGNAL(panChanged()), this, SLOT(update_pan()));

        connect(pm().get_project(), SIGNAL(trackRoutingChanged()), m_inBus, SLOT(bus_changed()));
        connect(pm().get_project(), SIGNAL(trackRoutingChanged()), m_outBus, SLOT(bus_changed()));

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
                painter->fillRect(xstart, m_track->get_height() - m_trackView->m_bottomborderwidth, pixelcount, m_trackView->m_bottomborderwidth, color);
        }

        // Track / track panel seperator is painted in TrackPanelViewPort... not the best place perhaps ?
        painter->fillRect(m_viewPort->width() - 3, 0, 3, m_track->get_height() - 1, themer()->get_color("TrackPanel:trackseparation"));

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
        m_gainView->update();
}


void TrackPanelView::update_pan()
{
        m_panView->update();
}


void TrackPanelView::draw_panel_name(QPainter* painter)
{
        QString title = QString::number(m_track->get_sort_index() + 1) + "  " + m_track->get_name();

        if (m_track->get_height() < SMALL_HEIGHT) {
                QFontMetrics fm(themer()->get_font("TrackPanel:fontscale:name"));
                title = fm.elidedText(title, Qt::ElideMiddle, 110);
        }

        painter->save();
        painter->setPen(themer()->get_color("TrackPanel:text"));
        painter->setFont(themer()->get_font("TrackPanel:fontscale:name"));
        painter->drawText(4, 12, title);
        painter->restore();
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
        int adjust = 0;

        Qt::Orientation orientation = (Qt::Orientation)config().get_property("Themer", "VUOrientation", Qt::Vertical).toInt();
        if (orientation == Qt::Vertical) {
                m_vuMeterView->set_bounding_rect(QRectF(0, 0, VU_WIDTH, height - 4));
                m_vuMeterView->setPos(m_boundingRect.width() - VU_WIDTH - 5, 2);
        } else {
                adjust = 14;
                m_vuMeterView->set_bounding_rect(QRectF(0, 0, 170, VU_WIDTH));
                m_vuMeterView->setPos(10, GAIN_Y_POS);
        }


        m_gainView->setPos(GAIN_X_POS, GAIN_Y_POS + adjust);
        m_panView->setPos(PAN_X_POS, PAN_Y_POS + adjust);

        m_inBus->setPos(4, INPUT_OUTPUT_BUTTON_Y_POS);
        m_outBus->setPos(96, INPUT_OUTPUT_BUTTON_Y_POS);

        m_soloLed->setPos(SOLO_X_POS, LED_Y_POS);
        m_muteLed->setPos(MUTE_X_POS, LED_Y_POS);

        if ((m_outBus->pos().y() + m_outBus->boundingRect().height()) >= height) {
                m_outBus->hide();
                m_inBus->hide();
        } else {
                m_outBus->show();
                m_inBus->show();
        }

        if ( (m_panView->pos().y() + m_panView->boundingRect().height()) >= height) {
                m_panView->hide();
        } else {
                m_panView->show();
        }

        if ( (m_gainView->pos().y() + m_gainView->boundingRect().height()) >= height) {
                m_gainView->hide();
        } else {
                m_gainView->show();
        }

        if ( (m_vuMeterView->pos().y() + m_vuMeterView->boundingRect().height()) >= height) {
                m_vuMeterView->hide();
        } else {
                m_vuMeterView->show();
        }

}

void TrackPanelView::theme_config_changed()
{
        m_vuMeterView->update_orientation();
        layout_panel_items();
        m_gainView->load_theme_data();
        m_panView->update();
        m_soloLed->update();
        m_muteLed->update();
        m_inBus->update();
        m_outBus->update();
}



AudioTrackPanelView::AudioTrackPanelView(AudioTrackView* trackView)
        : TrackPanelView(trackView)
{
	PENTERCONS;

        m_tv = trackView;
        m_recLed = new TrackPanelLed(this, m_track, "rec", "toggle_arm");
        m_recLed->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));

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
        m_recLed->setPos(REC_X_POS, LED_Y_POS);
}


SubGroupPanelView::SubGroupPanelView(SubGroupView* view)
        : TrackPanelView(view)
{
        PENTERCONS;

        m_panView->hide();
        m_muteLed->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));
        m_soloLed->set_bounding_rect(QRectF(0, 0, LED_WIDTH, LED_HEIGHT));

}

SubGroupPanelView::~SubGroupPanelView( )
{
        PENTERDES;
}


void SubGroupPanelView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
        Q_UNUSED(widget);
        Q_UNUSED(option);

        int xstart = (int)option->exposedRect.x();
        int pixelcount = (int)option->exposedRect.width();

        QColor color = themer()->get_color("SubGroup:background");
        painter->fillRect(xstart, m_trackView->m_topborderwidth, pixelcount, m_track->get_height() - m_trackView->m_bottomborderwidth, color);

        TrackPanelView::paint(painter, option, widget);
}


void SubGroupPanelView::layout_panel_items()
{
        TrackPanelView::layout_panel_items();
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

TrackPanelPan::TrackPanelPan(TrackPanelView *parent, Track *track)
        : ViewItem(parent, 0)
        , m_track(track)
{
}

void TrackPanelPan::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);

	QColor color = themer()->get_color("TrackPanel:slider:background");
        if (has_active_context()) {
		color = color.light(110);
	}
	
        const int PAN_H = 6;

	int sliderWidth = (int)m_boundingRect.width() - 75;
	float v;
	//	int y;
	QString s, span;
	painter->setPen(themer()->get_color("TrackPanel:text"));
	painter->setFont(themer()->get_font("TrackPanel:fontscale:pan"));

        painter->drawText(0, PAN_H + 1, "Pan");

        v = m_track->get_pan();
        span = QByteArray::number(v, 'f', 2);
	s = ( v > 0 ? QString("+") + span :  span );
        painter->fillRect(30, 0, sliderWidth, PAN_H, color);
        int pm= 31 + sliderWidth/2;
        int z = abs((int)(v*(sliderWidth/2)));

        if (v>=0) {
                painter->fillRect(pm, 1, z, PAN_H-1, m_gradient2D);
        } else {
                painter->fillRect(pm-z, 1, z, PAN_H-1, m_gradient2D);
        }
        painter->drawText(30 + sliderWidth + 10, PAN_H + 1, s);

        painter->setPen(themer()->get_color("TrackPanel:slider:border"));
        painter->drawRect(30, 0, sliderWidth, PAN_H);
}

void TrackPanelPan::set_width(int width)
{
	m_boundingRect = QRectF(0, 0, width, 9);
        load_theme_data();
}

void TrackPanelPan::load_theme_data()
{
        m_gradient2D.setColorAt(0.0, themer()->get_color("PanSlider:-1"));
        m_gradient2D.setColorAt(0.5, themer()->get_color("PanSlider:0"));
        m_gradient2D.setColorAt(1.0, themer()->get_color("PanSlider:1"));
        m_gradient2D.setStart(QPointF(m_boundingRect.width() - 40, 0));
        m_gradient2D.setFinalStop(31, 0);
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



TrackPanelLed::TrackPanelLed(TrackPanelView* view, Track *track, const QString& name, const QString& toggleslot)
        : ViewItem(view, 0)
	, m_name(name)
	, m_toggleslot(toggleslot)
	, m_isOn(false)
{
        m_track = track;
}

void TrackPanelLed::paint(QPainter* painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);
	
        int roundfactor = 10;
	
	painter->save();
	
//	painter->setRenderHint(QPainter::Antialiasing);
	
	if (m_isOn) {
		QColor color = themer()->get_color("TrackPanel:" + m_name + "led");
                if (has_active_context()) {
			color = color.light(110);
		}
		
		painter->setPen(themer()->get_color("TrackPanel:led:margin:active"));
		painter->setBrush(color);
		painter->drawRoundRect(m_boundingRect, roundfactor, roundfactor);
		
		painter->setFont(themer()->get_font("TrackPanel:fontscale:led"));
                painter->setPen(themer()->get_color("TrackPanel:led:font:active"));
		
                QString shortString = m_name.left(1).toUpper();
                painter->drawText(m_boundingRect, Qt::AlignCenter, shortString);
	} else {
		QColor color = themer()->get_color("TrackPanel:led:inactive");
                if (has_active_context()) {
			color = color.light(110);
		}
		
		painter->setPen(themer()->get_color("TrackPanel:led:margin:inactive"));
		painter->setBrush(color);
		painter->drawRoundRect(m_boundingRect, roundfactor, roundfactor);
		
		painter->setFont(themer()->get_font("TrackPanel:fontscale:led"));
		painter->setPen(themer()->get_color("TrackPanel:led:font:inactive"));
		
                QString shortString = m_name.left(1).toUpper();
                painter->drawText(m_boundingRect, Qt::AlignCenter, shortString);
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

Command * TrackPanelLed::toggle()
{
	Command* com;
        QMetaObject::invokeMethod(m_track, QS_C(m_toggleslot), Qt::DirectConnection, Q_RETURN_ARG(Command*, com));
	return 0;
}



TrackPanelBus::TrackPanelBus(TrackPanelView *view, Track *track, int busType)
        : ViewItem(view, 0)
        , m_track(track)
        , m_type(busType)
{
	bus_changed();
        m_boundingRect = QRectF(0, 0, 84, 13);
}

void TrackPanelBus::paint(QPainter* painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	QColor color = themer()->get_color("TrackPanel:bus:background");
	
	painter->save();
	
        if (has_active_context()) {
		color = color.light(110);
	}
	 
	painter->setPen(themer()->get_color("TrackPanel:bus:margin"));
	painter->setBrush(color);
        painter->drawRect(m_boundingRect);
	
        painter->setFont(themer()->get_font("TrackPanel:fontscale:bus"));
	painter->setPen(themer()->get_color("TrackPanel:bus:font"));
	
			
        if (m_type == BUSIN) {
                painter->drawPixmap(3, 3, m_pix);
                painter->drawText(m_boundingRect.adjusted(20, 0, 0, 0), Qt::AlignVCenter, m_busName);
        } else {
                painter->drawPixmap(m_boundingRect.width() - (m_pix.width() + 3), 3, m_pix);
                painter->drawText(m_boundingRect.adjusted(3, 0, 0, 0), Qt::AlignVCenter, m_busName);
        }
	
	painter->restore();
}

void TrackPanelBus::bus_changed()
{
	QFontMetrics fm(themer()->get_font("TrackPanel:bus"));
	prepareGeometryChange();
        int maxbuswidth = 70;
	
	if (m_type == BUSIN) {
                if (m_track->get_type() == Track::SUBGROUP) {
                        QList<TSend*> sends  = pm().get_project()->get_inputs_for_subgroup(qobject_cast<SubGroup*>(m_track));
                        if (sends.size() == 0) {
                                m_busName = "No Input";
                        } else if (sends.size() == 1) {
                                m_busName = sends.first()->get_from_name();
                        } else {
                                m_busName = "Multi" + QString("  (%1)").arg(sends.size());
                        }

                } else {
                        m_busName =  m_track-> get_bus_in_name();
                }

                int stringwidth = fm.width(m_busName) + 10;
                if (stringwidth > maxbuswidth) {
                        stringwidth = maxbuswidth;
                }
                m_pix = find_pixmap(":/bus_in");
	} else {
                QList<TSend*> sends  = m_track->get_post_sends();
                if (sends.size() == 0) {
                        m_busName = "No Outputs";
                } else if (sends.size() == 1) {
                        m_busName = sends.first()->get_name();
                } else {
                        m_busName = "Multi" + QString("  (%1)").arg(sends.size());
                }

                int stringwidth = fm.width(m_busName) + 10;
                if (stringwidth > maxbuswidth) {
                        stringwidth = maxbuswidth;
                }
                m_pix = find_pixmap(":/bus_out");
	}
	
	update();
}

