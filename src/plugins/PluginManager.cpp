/*Copyright (C) 2006-2007 Remon Sijrier

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


#include "PluginManager.h"
#include "Plugin.h"
#include "CorrelationMeter.h"
#include "SpectralMeter.h"

#if defined (LV2_SUPPORT)
#include <LV2Plugin.h>
#endif

PluginManager* PluginManager::m_instance = 0;


PluginManager::PluginManager()
{
	init();
}


PluginManager::~PluginManager()
{
#if defined (LV2_SUPPORT)
	slv2_world_free(m_slv2World);
#endif
}


PluginManager* PluginManager::instance()
{
	if (m_instance == 0) {
		m_instance = new PluginManager;
	}

	return m_instance;
}


void PluginManager::init()
{
#if defined (LV2_SUPPORT)
// LV2 part:
	m_slv2World = slv2_world_new();
	slv2_world_load_all(m_slv2World);
	m_slv2Plugins = slv2_world_get_all_plugins(m_slv2World);
#endif
}


Plugin* PluginManager::get_plugin(const  QDomNode node )
{
	QDomElement e = node.toElement();
	QString type = e.attribute( "type", "");

	Plugin* plugin = 0;

#if defined (LV2_SUPPORT)
	if (type == "LV2Plugin") {
		plugin = new LV2Plugin();
	}
#endif
	
// Well, this looks a little ehm, ugly hehe
// I'll investigate sometime in the future to make 
// a Plugin a _real_ plugin, by using the Qt Plugin
// framework. (loading it as a shared library object...)

	if (type == "CorrelationMeterPlugin") {
		plugin = new CorrelationMeter();
	}

	if (type == "SpectralMeterPlugin") {
		plugin = new SpectralMeter();
	}
	
	if (plugin) {
		if (plugin->set_state(node) > 0) {
			return plugin;
		} else {
			delete plugin;
			plugin = 0;
		}
	}

	return plugin;
}

#if defined (LV2_SUPPORT)

SLV2Plugins PluginManager::get_slv2_plugin_list()
{
	return m_slv2Plugins;
}
#endif

//eof
