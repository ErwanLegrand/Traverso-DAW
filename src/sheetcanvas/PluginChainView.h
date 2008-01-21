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

#ifndef PLUGIN_CHAIN_VIEW_H
#define PLUGIN_CHAIN_VIEW_H


#include "ViewItem.h"
#include <QString>

class SheetView;
class Plugin;
class PluginChain;
class PluginView;

class PluginChainView : public ViewItem
{
        Q_OBJECT

public:
        PluginChainView(SheetView* sv, ViewItem* parent, PluginChain* chain);
        ~PluginChainView();

        void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void calculate_bounding_rect();

private:
	PluginChain* m_pluginchain;
        QList<PluginView* >	m_pluginViews;

public slots:
        void add_new_pluginview(Plugin* plugin);
        void remove_pluginview(Plugin* plugin);
	void scrollbar_value_changed(int value);
	
private slots:
	void set_view_mode();

};

#endif

//eof

