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

#include <QLineEdit>
#include <QInputDialog>
#include <QGraphicsScene>

#include "AudioTrackView.h"
#include "AudioClipView.h"
#include "PluginChainView.h"
#include "Themer.h"
#include "TrackPanelViewPort.h"
#include "SheetView.h"
#include "TrackPanelView.h"
#include "TMainWindow.h"

#include <Sheet.h>
#include <AudioTrack.h>
#include "TTrackLaneView.h"
#include <AudioClip.h>
#include <Utils.h>
#include "CurveView.h"
#include "PluginChain.h"

#include <PluginSelectorDialog.h>

#include <Debugger.h>

AudioTrackView::AudioTrackView(SheetView* sv, AudioTrack * track)
        : TrackView(sv, track)
{
	PENTERCONS;
	
        m_track = track;
	load_theme_data();

        m_panel = new AudioTrackPanelView(this);

        calculate_bounding_rect();

	connect(m_track, SIGNAL(audioClipAdded(AudioClip*)), this, SLOT(add_new_audioclipview(AudioClip*)));
	connect(m_track, SIGNAL(audioClipRemoved(AudioClip*)), this, SLOT(remove_audioclipview(AudioClip*)));
	
        foreach(AudioClip* clip, m_track->get_cliplist()) {
                add_new_audioclipview(clip);
        }

	automation_visibility_changed();
}

void AudioTrackView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(widget);

        TrackView::paint(painter, option, widget);

// 	printf("TrackView:: PAINT :: exposed rect is: x=%f, y=%f, w=%f, h=%f\n", option->exposedRect.x(), option->exposedRect.y(), option->exposedRect.width(), option->exposedRect.height());
	
	int xstart = (int)option->exposedRect.x();
	int pixelcount = (int)option->exposedRect.width();
	
	if (m_paintBackground) {
		QColor color = themer()->get_color("Track:background");
                painter->fillRect(xstart, m_topborderwidth, pixelcount+1, m_sv->get_track_height(m_track) - m_bottomborderwidth, color);
	}
}

void AudioTrackView::add_new_audioclipview( AudioClip * clip )
{
	PENTER;
	AudioClipView* clipView = new AudioClipView(m_sv, this, clip);
	m_clipViews.append(clipView);
}

void AudioTrackView::remove_audioclipview( AudioClip * clip )
{
	PENTER;
	foreach(AudioClipView* view, m_clipViews) {
		if (view->get_clip() == clip) {
			m_clipViews.removeAll(view);
			scene()->removeItem(view);
			delete view;
			return;
		}
	}
}

int AudioTrackView::get_height( )
{
	return m_sv->get_track_height(m_track);
}


void AudioTrackView::load_theme_data()
{
	m_paintBackground = themer()->get_property("Track:paintbackground").toInt();
	m_topborderwidth = themer()->get_property("Track:topborderwidth").toInt();
	m_bottomborderwidth = themer()->get_property("Track:bottomborderwidth").toInt();
}


TCommand* AudioTrackView::insert_silence()
{
	TMainWindow::instance()->show_insertsilence_dialog();
        TMainWindow::instance()->set_insertsilence_track((AudioTrack*)m_track);
	return 0; 
}

void AudioTrackView::to_front(AudioClipView * view)
{
	foreach(AudioClipView* clipview, m_clipViews) {
		clipview->setZValue(zValue() + 1);
	}
	
	view->setZValue(zValue() + 2);
}

AudioClipView* AudioTrackView::get_nearest_audioclip_view(TimeRef location) const
{
        PENTER;
        if (!m_clipViews.size()) {
                return (AudioClipView*) 0;
        }

        AudioClipView* nearestClipView = 0;
        TimeRef shortestDistance(LONG_LONG_MAX);

        foreach(AudioClipView* clipview, m_clipViews) {
                AudioClip* clip = clipview->get_clip();

                // check if location is in the clipviews start/end range
                // if so, we found the 'nearest' clipview, so return it.
                if (clip->get_track_start_location() < location &&
                    clip->get_track_end_location() > location) {
                        return clipview;
                }

                // this clip is left of of location.
                if (clip->get_track_end_location() < location) {
                        TimeRef diff = location - clip->get_track_end_location();
                        if (diff < shortestDistance) {
                                shortestDistance = diff;
                                nearestClipView = clipview;
                        }
                }
                // this clip is right of location
                if (clip->get_track_start_location() > location) {
                        TimeRef diff = clip->get_track_start_location() - location;
                        if (diff < shortestDistance) {
                                shortestDistance = diff;
                                nearestClipView = clipview;
                        }
                }
        }

        return nearestClipView;
}

TCommand* AudioTrackView::show_track_gain_curve()
{
        if (m_curveView->isVisible()) {
                m_curveView->hide();
        } else {
		m_curveView->show();
        }

        return 0;
}

void AudioTrackView::automation_visibility_changed()
{
	if (m_track->show_clip_volume_automation()) {
		foreach(AudioClipView* acView, m_clipViews) {
			acView->get_gain_curve_view()->show();
		}
	} else {
		foreach(AudioClipView* acView, m_clipViews) {
			acView->get_gain_curve_view()->hide();
		}
	}

	TrackView::automation_visibility_changed();
}
