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

$Id: TrackView.cpp,v 1.9 2007/01/22 15:12:08 r_sijrier Exp $
*/

#include <QLineEdit>
#include <QInputDialog>
#include <QGraphicsScene>

#include "TrackView.h"
#include "AudioClipView.h"
#include "PluginChainView.h"
#include "ColorManager.h"
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
	m_sv = sv;
	sv->scene()->addItem(this);

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

	setZValue(5);
}

TrackView:: ~ TrackView( )
{
}

void TrackView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(widget);
	int xstart = (int)option->exposedRect.x();
	int pixelcount = (int)option->exposedRect.width();
	
	QColor color = cm().get("TRACK_BG");
// 	color.setAlpha(100);
	
	if (option->state & QStyle::State_MouseOver) {
		color = color.light(110);
	}
	
	QColor color11 = QColor(186, 202, 231);
	QColor color22 = QColor(235, 243, 255);
	QColor color33 = QColor(240, 246, 255);
	
	QColor color1 = color11.light(110);
	QColor color2 = color22.light(103);
	QColor color3 = color33.light(104);

	QLinearGradient grad1(QPointF(0, 0), QPointF(0, m_track->get_height()));
	grad1.setColorAt(0.0, color1);
	grad1.setColorAt(0.01, color2);
	grad1.setColorAt(0.5, color3);
	grad1.setColorAt(0.96, color2);
	grad1.setColorAt(1.0, color1);
	
// 	painter->fillRect(xstart, 0, pixelcount, m_track->get_height(), grad1);

	painter->fillRect(xstart, 0, pixelcount, 3, color1);
	painter->fillRect(xstart, 3, pixelcount, m_track->get_height()-5, color2);
	painter->fillRect(xstart, m_track->get_height() - 5, pixelcount, 5, color1);
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
	return m_track->get_height() - 8;
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


//eof
