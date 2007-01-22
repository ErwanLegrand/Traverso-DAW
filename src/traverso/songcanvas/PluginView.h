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

    $Id: PluginView.h,v 1.5 2007/01/22 20:27:06 r_sijrier Exp $
*/

#ifndef PLUGIN_VIEW_H
#define PLUGIN_VIEW_H


#include "ViewItem.h"
#include <QString>

#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
class LV2PluginPropertiesDialog;
#endif

class TrackView;
class Plugin;
class Track;

class PluginView : public ViewItem
{
        Q_OBJECT
	Q_CLASSINFO("edit_properties", tr("Settings..."))
	Q_CLASSINFO("remove_plugin", tr("Remove"))

public:
        PluginView(TrackView* view, Plugin* plugin, int index);
        ~PluginView();

	enum {Type = UserType + 5};
        
	Plugin* get_plugin();
        void set_index(int index);

        void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	int type() const;

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
	Command* edit_properties();
        Command* remove_plugin();
        
private slots:
	void repaint();
};

inline int PluginView::type() const {return Type;}

#endif

//eof

