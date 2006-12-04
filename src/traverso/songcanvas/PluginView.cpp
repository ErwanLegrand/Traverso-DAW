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

$Id: PluginView.cpp,v 1.3 2006/12/04 19:24:54 r_sijrier Exp $
*/

#include "PluginView.h"

#include <QPainter>

#include <Track.h>
#include "TrackView.h"

#include "ColorManager.h"
#include <Plugin.h>

#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
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
	,  propertiesDialog(0)
{
	PENTERCONS;
	m_track = m_trackView->get_track();
	m_name = plugin->get_name();
	m_boundingRectangle = QRectF(0, 0, 100, 25);
	
}

PluginView::~PluginView( )
{
	PENTERDES2;
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
	if (propertiesDialog) {
		delete propertiesDialog;
	}
#endif
}

void PluginView::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QColor color;
	if (m_plugin->is_bypassed()) {
		color.setRgb(230, 0, 230, 80);
	} else {
		color.setRgb(230, 0, 230, 170);
	}

	QBrush brush(color);
	int xstart = 200 + m_index * 120;
	QRect rect(0, 0, 100, 25);
	painter->fillRect(rect, brush);
	painter->setPen(QColor(Qt::white));
	painter->drawText(rect, Qt::AlignCenter, m_name);
}


Command * PluginView::edit_properties( )
{
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
	if (! propertiesDialog) {
		propertiesDialog = new LV2PluginPropertiesDialog((LV2Plugin*) m_plugin);
		propertiesDialog->setWindowTitle(m_name);
	} 
	propertiesDialog->show();
#endif
	return (Command*) 0;
}

Command* PluginView::remove_item()
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
//eof
