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

#include "TrackView.h"
#include "PluginChainView.h"

#include <Themer.h>
#include <Plugin.h>
#include <PluginChain.h>
#include <Track.h>
#include <Utils.h>

#if defined (LV2_SUPPORT)
#include <LV2PluginPropertiesDialog.h>
#endif

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PluginView::PluginView(PluginChainView* parent, PluginChain* chain, Plugin* plugin, int index)
	: ViewItem(parent, plugin)
	, m_pluginchain(chain)
	, m_plugin(plugin)
	, m_index(index)
{
	PENTERCONS;
	
#if defined (LV2_SUPPORT)
	propertiesDialog = 0;
#endif

	setZValue(parent->zValue() + 2);
	
	m_name = plugin->get_name();
	
	QFontMetrics fm(themer()->get_font("Plugin:fontscale:name"));
	int fontwidth = fm.width(m_name);
	
	m_boundingRect = QRectF(0, 0, fontwidth + 8, 25);
	
	setAcceptsHoverEvents(true);
	setCursor(themer()->get_cursor("Plugin"));
	
	connect(m_plugin, SIGNAL(bypassChanged()), this, SLOT(repaint()));
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
		color = themer()->get_color("Plugin:background:bypassed");
	} else {
		color = themer()->get_color("Plugin:background");
	}

	int height, width;
	if (option->state & QStyle::State_MouseOver) {
		height = 21;
		width = m_boundingRect.width() + 1;
		color = color.light(120);
	} else {
		height = 20;
		width = m_boundingRect.width();
	}
	
	QBrush brush(color);
	QRect rect(0, 0, width, height); 
	painter->fillRect(rect, brush);
	painter->setPen(themer()->get_color("Plugin:text"));
	painter->setFont(themer()->get_font("Plugin:fontscale:name"));
	painter->drawText(rect, Qt::AlignCenter, m_name);
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
	return m_pluginchain->remove_plugin(m_plugin);
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
