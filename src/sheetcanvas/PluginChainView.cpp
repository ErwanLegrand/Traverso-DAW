/*
    Copyright (C) 2006-2007 Remon Sijrier

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

#include "PluginChainView.h"

#include <QScrollBar>

#include "TrackView.h"
#include "SheetView.h"
#include "ClipsViewPort.h"
#include "PluginView.h"
#include "Themer.h"
#include <PluginChain.h>
#include <Plugin.h>

#include <Track.h>
#include "Sheet.h"

#if defined (LV2_SUPPORT)
#include <LV2Plugin.h>
#endif
#include <PluginChain.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


PluginChainView::PluginChainView(SheetView* sv, ViewItem* parent, PluginChain* chain)
	: ViewItem(parent, parent)
	, m_pluginchain(chain)
{
	PENTERCONS;
	
        setZValue(parent->zValue() + 10);
	m_sv = sv;
	calculate_bounding_rect();
	
	
	foreach(Plugin* plugin, chain->get_plugin_list()) {
		add_new_pluginview(plugin);
	}
	
	connect(chain, SIGNAL(pluginAdded(Plugin*)), this, SLOT(add_new_pluginview(Plugin*)));
	connect(chain, SIGNAL(pluginRemoved(Plugin*)), this, SLOT(remove_pluginview(Plugin*)));
	connect(m_sv->get_clips_viewport()->horizontalScrollBar(), SIGNAL(valueChanged(int)),
		this, SLOT(scrollbar_value_changed(int)));
	connect(m_sv->get_sheet(), SIGNAL(modeChanged()), this, SLOT(set_view_mode()));
	
	set_view_mode();
}

PluginChainView::~PluginChainView( )
{
        PENTERDES2;
}

void PluginChainView::add_new_pluginview( Plugin * plugin )
{
	PluginView* view = new PluginView(this, m_pluginchain, plugin, m_pluginViews.size());
	
	int x = 6;
	foreach(PluginView* view, m_pluginViews) {
		x += (int)view->boundingRect().width() + 6;
	}
	
	view->setPos(x, m_boundingRect.height() - view->boundingRect().height());
	
	m_pluginViews.append(view);
	
	show();
}

void PluginChainView::remove_pluginview( Plugin * plugin )
{
	foreach(PluginView* view, m_pluginViews) {
		if (view->get_plugin() == plugin) {
			m_pluginViews.removeAll(view);
			delete view;
		}
	}

	for (int i=0; i<m_pluginViews.size(); ++i) {
		m_pluginViews.at(i)->set_index(i);
	}
	
	int x = 6;
	foreach(PluginView* view, m_pluginViews) {
		view->setPos(x, m_boundingRect.height() - view->boundingRect().height());
		x += (int)view->boundingRect().width() + 6;
	}
	
	if (!m_pluginViews.size()) {
		hide();
	}
	
	m_parentViewItem->update();
}

void PluginChainView::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(painter);
	Q_UNUSED(option);
	Q_UNUSED(widget);
}

void PluginChainView::scrollbar_value_changed(int value)
{
	setPos(value, y());
}

void PluginChainView::set_view_mode()
{
	if (m_sv->get_sheet()->get_mode() == Sheet::EFFECTS) {
		show();
	} else {
		hide();
	}
}

void PluginChainView::calculate_bounding_rect()
{
	int y = (int)(m_parentViewItem->boundingRect().height());
	m_boundingRect = QRectF(0, 0, 0, y);
	setPos(pos().x(), - 2);
	ViewItem::calculate_bounding_rect();
}

