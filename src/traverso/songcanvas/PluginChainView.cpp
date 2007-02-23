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

    $Id: PluginChainView.cpp,v 1.8 2007/02/23 13:52:38 r_sijrier Exp $
*/

#include "PluginChainView.h"

#include <QGraphicsScene>

#include "TrackView.h"
#include "PluginView.h"
#include "Themer.h"
#include <PluginChain.h>
#include <Plugin.h>

#include <Track.h>

#if defined (LV2_SUPPORT)
#include <LV2Plugin.h>
#endif
#include <PluginChain.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


PluginChainView::PluginChainView(TrackView* parent, PluginChain* chain)
	: ViewItem(parent, parent)
	, m_trackView(parent)
{
	PENTERCONS;
	
	setZValue(parent->zValue() + 1);
	
	m_trackView->scene()->addItem(this);
	m_boundingRect = QRectF(0, 0, 0, 44);
	
	QList<Plugin* >* pluginList = chain->get_plugin_list();
	for (int i=0; i<pluginList->size(); ++i) {
		Plugin* plugin = pluginList->at(i);
		add_new_pluginview(plugin);
	}
	
	connect(chain, SIGNAL(pluginAdded(Plugin*)), this, SLOT(add_new_pluginview(Plugin*)));
	connect(chain, SIGNAL(pluginRemoved(Plugin*)), this, SLOT(remove_pluginview(Plugin*)));
	
}

PluginChainView::~PluginChainView( )
{
        PENTERDES2;
}

void PluginChainView::add_new_pluginview( Plugin * plugin )
{
	PluginView* view = new PluginView(m_trackView, plugin, m_pluginViews.size());
	m_pluginViews.append(view);
	scene()->addItem(view);
	view->setPos( ( m_pluginViews.size() -1 )* 120, m_boundingRect.height() - view->boundingRect().height());
}

void PluginChainView::remove_pluginview( Plugin * plugin )
{
	foreach(PluginView* view, m_pluginViews) {
		if (view->get_plugin() == plugin) {
			printf("Removing pluginview\n");
			m_pluginViews.removeAll(view);
			delete view;
		}
	}

	for (int i=0; i<m_pluginViews.size(); ++i) {
		m_pluginViews.at(i)->set_index(i);
	}
	
	m_trackView->update();
}

void PluginChainView::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(painter);
	Q_UNUSED(option);
	Q_UNUSED(widget);
}

//eof

