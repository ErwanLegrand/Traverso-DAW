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
 
    $Id: PluginView.cpp,v 1.2 2006/07/31 14:58:50 r_sijrier Exp $
*/

#include "PluginView.h"

#include <QPainter>

#include "TrackView.h"
#include "ColorManager.h"
#include <LV2Plugin.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


PluginView::PluginView( ViewPort * vp, TrackView* parent, Plugin* plugin, int index)
                : ViewItem(vp, parent, plugin), m_trackView(parent), 
                  m_plugin(plugin), m_index(index)
{
	zOrder = 10;
	m_track = m_trackView->get_track();
	m_name = plugin->get_name();
	propertiesDialog = new LV2PluginPropertiesDialog((LV2Plugin*) plugin);
	propertiesDialog->setWindowTitle(m_name);
	
	init_context_menu( this );
	m_type = PLUGINVIEW;
	
	connect(m_plugin, SIGNAL(bypassChanged()), m_trackView, SLOT(repaint_all_clips()));
	
	schedule_for_repaint();
}

PluginView::~PluginView( )
{
        PENTERDES2;
}

QRect PluginView::draw( QPainter & p )
{
	QColor color;
	if (m_plugin->is_bypassed()) {
		color.setRgb(230, 0, 230, 80);
	} else {
		color.setRgb(230, 0, 230, 170);
	}
	
	QBrush brush(color);
	int xstart = 200 + m_index * 120;
	QRect rect(xstart, m_track->get_baseY() + m_track->get_height() - 30, 100, 25);
	p.fillRect(rect, brush);
	p.setPen(QColor(Qt::white));
	p.drawText(rect, Qt::AlignCenter, m_name);
        
        return rect;
}


void PluginView::schedule_for_repaint( )
{
	int xstart = 200 + m_index * 120;
	
	set_geometry(xstart, m_track->get_baseY() + m_track->get_height() - 30, 100, 25);
        m_vp->schedule_for_repaint(this);
}

Command * PluginView::edit_properties( )
{
	propertiesDialog->show();
	
	return (Command*) 0;
}

Command* PluginView::remove_item()
{
	m_track->remove_plugin(m_plugin);
	
	return (Command*) 0;
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
 
 
