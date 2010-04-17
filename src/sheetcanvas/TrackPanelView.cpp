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
#include <QFont>
#include <QMenu>
#include <QAction>
#include <QStringList>

#include "AudioDevice.h"
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
#include "Sheet.h"
#include "SubGroup.h"
#include "Track.h"
#include "Interface.h"
#include "VUMeterView.h"
		
#include <Debugger.h>

#define MICRO_HEIGHT 35
#define SMALL_HEIGHT 65

const int LED_SPACING = 6;
const int LED_WIDTH = 16;
const int MUTE_X_POS = 130;
const int SOLO_X_POS = MUTE_X_POS + LED_SPACING + LED_WIDTH;
const int REC_X_POS = SOLO_X_POS + LED_SPACING + LED_WIDTH;
const int LED_Y_POS = 4;



TrackPanelView::TrackPanelView(TrackView* view)
        : ViewItem(0, view)
{
        PENTERCONS;

        m_trackView = view;
        m_viewPort = m_trackView->get_sheetview()->get_trackpanel_view_port();
        m_track = m_trackView->get_track();

        m_gainView = new TrackPanelGain(this, m_track);
        m_gainView->set_width(m_viewPort->width() - 20);

        m_panView = new TrackPanelPan(this, m_track);
        m_panView->set_width(m_viewPort->width() - 20);

        m_soloLed = new TrackPanelLed(this, m_track, "solo", "solo");
        m_muteLed = new TrackPanelLed(this, m_track, "mute", "mute");
        m_muteLed->set_bounding_rect(QRectF(0, 0, 15, 12));
        m_soloLed->set_bounding_rect(QRectF(0, 0, 15, 12));

        if (m_track->is_solo()) {
                m_soloLed->ison_changed(true);
        }
        if (m_track->is_muted()) {
                m_muteLed->ison_changed(true);
        }

        m_inBus = new TrackPanelBus(this, m_track, TrackPanelBus::BUSIN);
        m_outBus = new TrackPanelBus(this, m_track, TrackPanelBus::BUSOUT);

        m_viewPort->scene()->addItem(this);

        if (m_track == m_track->get_sheet()->get_master_out()) {
                m_vuMeterView = new VUMeterView(this, m_track->get_sheet()->get_master_out()->get_process_bus());
                m_vuMeterView->set_bounding_rect(QRectF(0, 0, 180, 10));
                m_vuMeterView->setPos(10, 20);
        }



        m_boundingRect = QRectF(0, 0, 200, m_track->get_height());

//        layout_panel_items();

        connect(m_track, SIGNAL(soloChanged(bool)), m_soloLed, SLOT(ison_changed(bool)));
        connect(m_track, SIGNAL(muteChanged(bool)), m_muteLed, SLOT(ison_changed(bool)));

        connect(m_track, SIGNAL(stateChanged()), this, SLOT(update_gain()));
        connect(m_track, SIGNAL(panChanged()), this, SLOT(update_pan()));

        connect(m_track, SIGNAL(busConfigurationChanged()), m_inBus, SLOT(bus_changed()));
        connect(m_track, SIGNAL(busConfigurationChanged()), m_outBus, SLOT(bus_changed()));

        connect(m_track, SIGNAL(stateChanged()), this, SLOT(update_name()));
        connect(m_track, SIGNAL(activeContextChanged()), this, SLOT(active_context_changed()));

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


        if (m_trackView->is_moving() || m_track->has_active_context()) {
                QColor color = themer()->get_color("Track:mousehover");
                color.setAlpha(30);
                painter->fillRect(boundingRect(), color);
        }


        int xstart = (int)option->exposedRect.x();
        int pixelcount = (int)option->exposedRect.width();

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
                title = fm.elidedText(title, Qt::ElideMiddle, 90);
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





AudioTrackPanelView::AudioTrackPanelView(AudioTrackView* trackView)
        : TrackPanelView(trackView)
{
	PENTERCONS;

        m_tv = trackView;
        m_recLed = new TrackPanelLed(this, m_track, "rec", "toggle_arm");
        m_recLed->set_bounding_rect(QRectF(0, 0, 15, 12));

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
        int height =  m_track->get_height();
	
	m_gainView->setPos(10, 39);
	m_panView->setPos(10, 54);
	
        m_inBus->setPos(10, 73);
        m_outBus->setPos(100, 73);
	
	if (height < SMALL_HEIGHT) {
		m_gainView->setPos(10, 20);
		m_panView->setPos(10, 36);
	} else {
                m_gainView->setPos(10, 39);
		m_panView->setPos(10, 54);
	}
	
        m_muteLed->setPos(MUTE_X_POS, LED_Y_POS);
        m_soloLed->setPos(SOLO_X_POS, LED_Y_POS);
        m_recLed->setPos(REC_X_POS, LED_Y_POS);

        if ((m_inBus->pos().y() + m_inBus->boundingRect().height()) >= height) {
                m_inBus->hide();
                m_outBus->hide();
	} else {
                m_inBus->show();
                m_outBus->show();
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





SubGroupPanelView::SubGroupPanelView(SubGroupView* view)
        : TrackPanelView(view)
{
        PENTERCONS;

        m_inBus->hide();
        m_panView->hide();
        m_muteLed->set_bounding_rect(QRectF(0, 0, 41, 14));
        m_soloLed->set_bounding_rect(QRectF(0, 0, 38, 14));

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
        int height =  m_track->get_height();

        m_gainView->setPos(10, 39);
        m_panView->setPos(10, 54);

        if (height < 50) {
                m_outBus->setPos(100, 0);
                m_gainView->setPos(10, 20);
                m_muteLed->hide();
                m_soloLed->hide();
        } else {
                m_outBus->setPos(100, 2);
                m_muteLed->setPos(100, 20);
                m_soloLed->setPos(148, 20);
                m_gainView->setPos(10, 40);
                m_muteLed->show();
                m_soloLed->show();
        }


        if ( (m_gainView->pos().y() + m_panView->boundingRect().height()) >= height) {
                m_gainView->hide();
        } else {
                m_gainView->show();
        }
}









TrackPanelGain::TrackPanelGain(TrackPanelView *parent, Track *track)
        : ViewItem(parent, 0)
        , m_track(track)
{
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
	
	painter->save();
	
	painter->setPen(themer()->get_color("TrackPanel:text"));
	painter->setFont(themer()->get_font("TrackPanel:fontscale:gain"));
	painter->drawText(0, height + 1, "GAIN");
	painter->drawRect(30, 0, sliderWidth, height);
	
	QColor color(cr,0,cb);
        if (has_active_context()) {
		color = color.light(140);
	}
	painter->fillRect(31, 1, sliderdbx, height-1, color);
	painter->drawText(sliderWidth + 35, height, sgain);
	
	painter->restore();
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
	
	int roundfactor = 20;
	
	painter->save();
	
	painter->setRenderHint(QPainter::Antialiasing);
	
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
}

void TrackPanelBus::paint(QPainter* painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	QColor color = themer()->get_color("TrackPanel:bus:background");
	int roundfactor = 15;
	
	painter->save();
	
	painter->setRenderHint(QPainter::Antialiasing);
	
        if (has_active_context()) {
		color = color.light(110);
	}
	 
	painter->setPen(themer()->get_color("TrackPanel:bus:margin"));
	painter->setBrush(color);
	painter->drawRoundRect(m_boundingRect, roundfactor, roundfactor);
	
	painter->setFont(themer()->get_font("TrackPanel:fontscale:led"));
	painter->setPen(themer()->get_color("TrackPanel:bus:font"));
	
        painter->drawText(m_boundingRect.adjusted(15, 0, 0, 0), Qt::AlignCenter, m_busName);
			
	painter->drawPixmap(3, 3, m_pix);
	
	painter->restore();
}

void TrackPanelBus::bus_changed()
{
	QFontMetrics fm(themer()->get_font("TrackPanel:bus"));
	prepareGeometryChange();
	
	if (m_type == BUSIN) {
                m_busName =  m_track-> get_bus_in_name();
		m_pix = find_pixmap(":/bus_in");
		m_boundingRect = m_pix.rect();
		m_boundingRect.setWidth(m_pix.rect().width() + fm.width(m_busName) + 10);
	} else {
                m_busName = m_track->get_bus_out_name();
		m_pix = find_pixmap(":/bus_out");
		m_boundingRect = m_pix.rect();
		m_boundingRect.setWidth(m_pix.rect().width() + fm.width(m_busName) + 10);
	}
	
	m_boundingRect.setHeight(m_boundingRect.height() + 6);
	
	update();
}


Command* TrackPanelBus::select_bus()
{
        QMenu menu;

        Sheet* sheet = m_track->get_sheet();
        SubGroup* masterOut = sheet->get_master_out();
        QAction* action;

        if (m_type == BUSOUT) {
                if (m_track->get_type() != Track::SUBGROUP) {
                        action = menu.addAction(tr("Sub Groups"));
                        action->setEnabled(false);
                        menu.addSeparator();
                }



                if (!(m_track == masterOut)) {
                        menu.addAction(masterOut->get_name());
                }

                QList<SubGroup*> subgroups = sheet->get_subgroups();
                if (!m_track->get_type() == Track::SUBGROUP && subgroups.size()) {
                        foreach(SubGroup* sub, subgroups) {
                                menu.addAction(sub->get_name());
                        }
                }
        }

        action = menu.addAction(tr("Hardware Buses"));
        action->setEnabled(false);

        menu.addSeparator();

        if (m_type == BUSIN) {
                foreach(QString busName, audiodevice().get_capture_buses_names()) {
                        menu.addAction(busName);
                }
        } else {

                foreach(QString busName, audiodevice().get_playback_buses_names()) {
                        menu.addAction(busName);
                }

        }

        menu.addAction(tr("More..."));

        action = menu.exec(QCursor::pos());

        if (action) {
                if (action->text() == tr("More...")) {
                        Interface::instance()->audio_io_dialog();
                        return 0;
                }

                if (m_type == BUSIN) {
                        m_track->set_input_bus(action->text());
                } else {
                        m_track->set_output_bus(action->text());
                }
        }

        return 0;
}
