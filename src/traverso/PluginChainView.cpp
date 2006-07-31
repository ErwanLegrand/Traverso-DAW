/*
    Copyright (C) 2006 Remon Sijrier 
 
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
 
    $Id: PluginChainView.cpp,v 1.2 2006/07/31 14:58:50 r_sijrier Exp $
*/

#include "PluginChainView.h"

#include <QPainter>

#include "TrackView.h"
#include "PluginView.h"
#include "ColorManager.h"
#include <LV2Plugin.h>
#include <PluginChain.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


PluginChainView::PluginChainView( ViewPort * vp, TrackView* parent)
                : ViewItem(vp, parent), m_trackView(parent)
{
	zOrder = 10;
	
	connect(m_trackView->get_track()->get_plugin_chain(), SIGNAL(pluginAdded(Plugin*)), this, SLOT(add_new_pluginview(Plugin*)));
	connect(m_trackView->get_track()->get_plugin_chain(), SIGNAL(pluginRemoved(Plugin*)), this, SLOT(remove_pluginview(Plugin*)));
}

PluginChainView::~PluginChainView( )
{
        PENTERDES2;
}

QRect PluginChainView::draw( QPainter & p )
{
       return QRect();
}


void PluginChainView::schedule_for_repaint( )
{
	foreach(PluginView* view, m_pluginViews) {
 		view->schedule_for_repaint();
	}
}

void PluginChainView::add_new_pluginview( Plugin * plugin )
{
	PluginView* view = new PluginView(m_vp, m_trackView, plugin, m_pluginViews.size());
	m_pluginViews.append(view);
	schedule_for_repaint();
}

void PluginChainView::remove_pluginview( Plugin * plugin )
{
	foreach(PluginView* view, m_pluginViews) {
		if (view->get_plugin() == plugin) {
			printf("Removing pluginview\n");
			m_pluginViews.removeAll(view);
			m_vp->unregister_viewitem(view);
			delete view;
		}
	}
	
	for (int i=0; i<m_pluginViews.size(); ++i) {
		m_pluginViews.at(i)->set_index(i);
	}
	
	m_trackView->repaint_all_clips();
	schedule_for_repaint();
}

//eof
 
