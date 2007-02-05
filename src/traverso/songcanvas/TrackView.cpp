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

$Id: TrackView.cpp,v 1.13 2007/02/05 17:12:02 r_sijrier Exp $
*/

#include <QLineEdit>
#include <QInputDialog>
#include <QGraphicsScene>

#include "TrackView.h"
#include "AudioClipView.h"
#include "PluginChainView.h"
#include "Themer.h"
#include "TrackPanelViewPort.h"
#include "SongView.h"
#include "TrackPanelView.h"

#include <Song.h>
#include <Track.h>

#include <cmath>

#include <PluginSelectorDialog.h>

#include <Debugger.h>


TrackView::TrackView(SongView* sv, Track * track)
	: ViewItem(0, track)
{
	PENTERCONS;
	
	setZValue(sv->zValue() + 1);
	
	m_sv = sv;
	sv->scene()->addItem(this);
	
	reload_theme_data();

	m_track = track;
	m_clipViewYOfsset = 3;
	setFlags(ItemIsSelectable | ItemIsMovable);
// 	setAcceptsHoverEvents(true);

	m_panel = new TrackPanelView(m_sv->get_trackpanel_view_port(), this, m_track);
	calculate_bounding_rect();
	
	m_pluginChainView = new PluginChainView(this, m_track->get_plugin_chain());

	connect(m_track, SIGNAL(audioClipAdded(AudioClip*)), this, SLOT(add_new_audioclipview(AudioClip*)));
	connect(m_track, SIGNAL(audioClipRemoved(AudioClip*)), this, SLOT(remove_audioclipview(AudioClip*)));
	
	
	foreach(AudioClip* clip, m_track->get_cliplist()) {
		add_new_audioclipview(clip);
	}
}

TrackView:: ~ TrackView( )
{
}

void TrackView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(widget);
	
	int xstart = (int)option->exposedRect.x();
	int pixelcount = (int)option->exposedRect.width();
	
	if (m_cliptopoffset > 0) {
		QColor color = themer()->get_color("Track:cliptopoffset");
		painter->fillRect(xstart, 0, pixelcount, m_cliptopoffset, color);
	}
	
	if (m_paintBackground) {
		QColor color = themer()->get_color("Track:background");
		painter->fillRect(xstart, m_cliptopoffset, pixelcount, m_track->get_height() - m_clipbottomoffset, color);
	}
	
	if (m_clipbottomoffset > 0) {
		QColor color = themer()->get_color("Track:clipbottomoffset");
		painter->fillRect(xstart, m_track->get_height() - m_clipbottomoffset, pixelcount, m_clipbottomoffset, color);
	}
}

void TrackView::add_new_audioclipview( AudioClip * clip )
{
	PENTER;
	AudioClipView* clipView = new AudioClipView(m_sv, this, clip);
	m_clipViews.append(clipView);
}

void TrackView::remove_audioclipview( AudioClip * clip )
{
	PENTER;
	foreach(AudioClipView* view, m_clipViews) {
		if (view->get_clip() == clip) {
			printf("Removing clipview from track %d\n", m_track->get_id());
			scene()->removeItem(view);
			m_clipViews.removeAll(view);
			delete view;
			return;
		}
	}
}


// void TrackView::add_clip_view(AudioClipView* view)
// {
// 	view->setParent(this);
// 	m_clipViews.append(view);
// }

Track* TrackView::get_track( ) const
{
	return m_track;
}

int TrackView::get_clipview_y_offset( )
{
	return m_clipViewYOfsset;
}

void TrackView::move_to( int x, int y )
{
	setPos(0, y);
	m_panel->setPos(-202, y);
}

int TrackView::get_clipview_height( )
{
	return m_track->get_height() - (m_cliptopoffset + m_clipbottomoffset);
}

Command* TrackView::edit_properties( )
{
	bool ok;
	QString text = QInputDialog::getText(m_sv->get_trackpanel_view_port()->viewport(), tr("Set Track name"),
					tr("Enter new Track name"),
					QLineEdit::Normal, m_track->get_name(), &ok);
	if (ok && !text.isEmpty()) {
		m_track->set_name(text);
	}

	return (Command*) 0;
}

Command* TrackView::add_new_plugin( )
{
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
	if (PluginSelectorDialog::instance()->exec() == QDialog::Accepted) {
		Plugin* plugin = PluginSelectorDialog::instance()->get_selected_plugin();
		if (plugin) {
			return m_track->add_plugin(plugin);
		}
	}

#endif
	return 0;
}

int TrackView::get_height( )
{
	return m_height;
}

void TrackView::set_height( int height )
{
	m_height = height;
}

void TrackView::calculate_bounding_rect()
{
	m_boundingRectangle = QRectF(0, 0, pow(2, 30), m_track->get_height());
	m_panel->calculate_bounding_rect();
	update();
}

void TrackView::reload_theme_data()
{
	m_paintBackground = themer()->get_property("Track:paintbackground").toInt();
	m_cliptopoffset = themer()->get_property("Track:cliptopoffset").toInt();
	m_clipbottomoffset = themer()->get_property("Track:clipbottomoffset").toInt();

	update();
}

//eof

