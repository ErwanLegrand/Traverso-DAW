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

$Id: TrackView.cpp,v 1.17 2007/02/08 20:51:38 r_sijrier Exp $
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

#include <PluginSelectorDialog.h>

#include <Debugger.h>


TrackView::TrackView(SongView* sv, Track * track)
	: ViewItem(0, track)
{
	PENTERCONS;
	
	setZValue(sv->zValue() + 1);
	
	m_sv = sv;
	sv->scene()->addItem(this);
	
	load_theme_data();

	m_track = track;
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
	
	if (m_topborderwidth > 0) {
		QColor color = themer()->get_color("Track:cliptopoffset");
		painter->fillRect(xstart, 0, pixelcount, m_topborderwidth, color);
	}
	
	if (m_paintBackground) {
		QColor color = themer()->get_color("Track:background");
		painter->fillRect(xstart, m_topborderwidth, pixelcount, m_track->get_height() - m_bottomborderwidth, color);
	}
	
	if (m_bottomborderwidth > 0) {
		QColor color = themer()->get_color("Track:clipbottomoffset");
		painter->fillRect(xstart, m_track->get_height() - m_bottomborderwidth, pixelcount, m_bottomborderwidth, color);
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
	return m_topborderwidth + m_cliptopmargin;
}

void TrackView::move_to( int x, int y )
{
	setPos(0, y);
	m_panel->setPos(-202, y);
}

int TrackView::get_clipview_height( )
{
	return m_track->get_height() - (m_topborderwidth + m_bottomborderwidth + m_clipbottommargin + m_cliptopmargin);
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
	m_boundingRectangle = QRectF(0, 0, MAX_CANVAS_WIDTH, m_track->get_height());
	m_panel->calculate_bounding_rect();
	update();
}

void TrackView::load_theme_data()
{
	m_paintBackground = themer()->get_property("Track:paintbackground").toInt();
	m_topborderwidth = themer()->get_property("Track:topborderwidth").toInt();
	m_bottomborderwidth = themer()->get_property("Track:bottomborderwidth").toInt();
	
	m_cliptopmargin = themer()->get_property("Track:cliptopmargin").toInt();
	m_clipbottommargin = themer()->get_property("Track:clipbottommargin").toInt();

	update();
}

//eof

