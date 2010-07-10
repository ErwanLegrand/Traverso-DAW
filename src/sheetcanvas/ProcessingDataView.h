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

#ifndef PROCESSING_DATA_VIEW_H
#define PROCESSING_DATA_VIEW_H

#include "ViewItem.h"

class ProcessingData;
class AudioClip;
class Track;
class PDPanelView;
class PluginChainView;

class ProcessingDataView : public ViewItem
{
        Q_OBJECT
        Q_CLASSINFO("edit_properties", tr("Edit properties"))
        Q_CLASSINFO("add_new_plugin", tr("Add new Plugin"))
        Q_CLASSINFO("select_bus", tr("Select Bus"))

public:
        ProcessingDataView(SheetView* sv, ProcessingData* pd);
        ~ProcessingDataView();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        ProcessingData* get_processing_data() const {return m_pd;}
        PDPanelView* get_panel_view() const {return m_panel;}

        virtual int get_childview_y_offset() const;
        void move_to(int x, int y);
        int get_height();
        void set_height(int height);

        void calculate_bounding_rect();
        void load_theme_data();

protected:
        ProcessingData*		m_pd;
        PDPanelView*		m_panel;
        PluginChainView*	m_pluginChainView;
        int			m_height;
        int			m_paintBackground;
        int			m_cliptopmargin;
        int			m_clipbottommargin;
        int			m_topborderwidth;
        int			m_bottomborderwidth;

public slots:
        Command* edit_properties();
        Command* add_new_plugin();
        Command* select_bus();

private slots:
};


#endif

//eof
