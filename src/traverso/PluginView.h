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

    $Id: PluginView.h,v 1.3 2006/10/18 12:08:56 r_sijrier Exp $
*/

#ifndef PLUGIN_VIEW_H
#define PLUGIN_VIEW_H


#include "ViewItem.h"
#include <QString>

#include <PluginChain.h>
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
#include "LV2PluginPropertiesDialog.h"
#endif

class TrackView;
class Plugin;
class Track;

class PluginView : public ViewItem
{
        Q_OBJECT

public:
        PluginView(ViewPort* vp, TrackView* view, Plugin* plugin, int index);
        ~PluginView();

        Plugin* get_plugin();
        void set_index(int index);

        QRect draw(QPainter& p);

private:
        TrackView* 	m_trackView;
        Track*		m_track;
        Plugin*		m_plugin;

        int 		m_index;
        QString		m_name;

#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
        LV2PluginPropertiesDialog*	propertiesDialog;
#endif

public slots:
        void schedule_for_repaint();

	Command* edit_properties();
        Command* remove_item();
};

#endif

//eof

