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

*/

#include "PluginView.h"

#include <QPainter>

#include <Track.h>
#include "TrackView.h"

#include "Themer.h"
#include <Plugin.h>
#include <Utils.h>

#if defined (LV2_SUPPORT)
#include <LV2PluginPropertiesDialog.h>
#endif

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PluginView::PluginView(TrackView* parent, Plugin* plugin, int index)
	: ViewItem(parent, plugin)
	, m_trackView(parent)
	,  m_plugin(plugin)
	,  m_index(index)
{
	PENTERCONS;
	
#if defined (LV2_SUPPORT)
	propertiesDialog = 0;
#endif

	setZValue(parent->zValue() + 2);
	
	m_track = m_trackView->get_track();
	m_name = plugin->get_name();
	m_boundingRect = QRectF(0, 0, 100, 25);
	
	setAcceptsHoverEvents(true);
	setCursor(themer()->get_cursor("Plugin"));
}

PluginView::~PluginView( )
{
	PENTERDES2;
#if defined (LV2_SUPPORT)
	if (propertiesDialog) {
		delete propertiesDialog;
	}
#endif
}

void PluginView::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	QColor color;
	if (m_plugin->is_bypassed()) {
		color.setRgb(230, 0, 230, 80);
	} else {
		color.setRgb(230, 0, 230, 170);
	}

	int height, width;
	if (option->state & QStyle::State_MouseOver) {
		height = 21;
		width = 101;
		color = color.light(120);
	} else {
		height = 20;
		width = 100;
	}
	
	QBrush brush(color);
	QRect rect(0, 0, width, height); 
	painter->fillRect(rect, brush);
	painter->setPen(QColor(Qt::white));
	painter->drawText(rect, Qt::AlignCenter, m_name);
	
	connect(m_plugin, SIGNAL(bypassChanged()), this, SLOT(repaint()));
}


Command * PluginView::edit_properties( )
{
#if defined (LV2_SUPPORT)
	if (! propertiesDialog) {
		propertiesDialog = new LV2PluginPropertiesDialog((LV2Plugin*) m_plugin);
		propertiesDialog->setWindowTitle(m_name);
	} 
	propertiesDialog->show();
#endif
	return (Command*) 0;
}

Command* PluginView::remove_plugin()
{
	return m_track->remove_plugin(m_plugin);
}

Plugin * PluginView::get_plugin( )
{
	return m_plugin;
}

void PluginView::set_index(int index)
{
	m_index = index;
}

void PluginView::repaint( )
{
	update();
}

//eof
