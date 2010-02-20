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
#include <Interface.h>

#include <Sheet.h>
#include <AudioTrack.h>
#include <AudioClip.h>
#include <Utils.h>

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
		painter->fillRect(xstart, m_topborderwidth, pixelcount+1, m_track->get_height() - m_bottomborderwidth, color);
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

int AudioTrackView::get_childview_y_offset() const
{
	return m_topborderwidth + m_cliptopmargin;
}

int AudioTrackView::get_height( )
{
	return m_track->get_height() - (m_topborderwidth + m_bottomborderwidth + m_clipbottommargin + m_cliptopmargin);
}


void AudioTrackView::load_theme_data()
{
	m_paintBackground = themer()->get_property("Track:paintbackground").toInt();
	m_topborderwidth = themer()->get_property("Track:topborderwidth").toInt();
	m_bottomborderwidth = themer()->get_property("Track:bottomborderwidth").toInt();
	
	m_cliptopmargin = themer()->get_property("Track:cliptopmargin").toInt();
	m_clipbottommargin = themer()->get_property("Track:clipbottommargin").toInt();
}


Command* AudioTrackView::insert_silence()
{
	Interface::instance()->show_insertsilence_dialog();
        Interface::instance()->set_insertsilence_track((AudioTrack*)m_track);
	return 0; 
}

void AudioTrackView::to_front(AudioClipView * view)
{
	foreach(AudioClipView* clipview, m_clipViews) {
		clipview->setZValue(zValue() + 1);
	}
	
	view->setZValue(zValue() + 2);
}

