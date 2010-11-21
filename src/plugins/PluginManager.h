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


#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#if defined (LV2_SUPPORT)
#include <slv2/slv2.h>
#include <redland.h>
#endif

#include <QDomDocument>

class Plugin;

class PluginManager
{

public:
	~PluginManager();

	static PluginManager* instance();

	Plugin* get_plugin(const QDomNode node);

#if defined (LV2_SUPPORT)
	SLV2Plugins get_slv2_plugin_list();
	SLV2World get_slv2_world() {return m_slv2World;}
	Plugin* create_lv2_plugin(const QString& uri);
#endif

private:
	PluginManager();

	static PluginManager* m_instance;
#if defined (LV2_SUPPORT)
	SLV2World 	m_slv2World;
	SLV2Plugins	m_slv2Plugins;
#endif
	void init();
};

#endif

//eof
