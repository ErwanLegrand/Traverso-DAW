/*Copyright (C) 2006 Remon Sijrier

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

$Id: PluginManager.cpp,v 1.3 2006/11/27 20:54:44 r_sijrier Exp $

slv2 url: http://codeson.net/svn/libslv2/
*/


#include "PluginManager.h"
#include "Plugin.h"
#include "CorrelationMeter.h"
#include "SpectralMeter.h"

#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
#include <LV2Plugin.h>
#endif

PluginManager* PluginManager::m_instance = 0;


PluginManager::PluginManager()
{
	init();
}


PluginManager::~PluginManager()
{
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
	slv2_list_free(slv2PluginList);
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

#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
// LV2 part:
	slv2PluginList  = slv2_list_new();
	slv2_list_load_all(slv2PluginList);
#endif
}


Plugin* PluginManager::get_plugin(const  QDomNode node )
{
	QDomElement e = node.toElement();
	QString type = e.attribute( "type", "");

	Plugin* plugin = 0;

#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
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
	
	
	if (plugin->set_state(node) > 0) {
		return plugin;
	} else {
		delete plugin;
		plugin = 0;
	}

	return plugin;
}

#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)

SLV2List PluginManager::get_slv2_plugin_list()
{
	return slv2PluginList;
}
#endif

//eof
