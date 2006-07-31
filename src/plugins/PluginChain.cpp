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

$Id: PluginChain.cpp,v 1.1 2006/07/31 13:24:46 r_sijrier Exp $
*/
 
#include "PluginChain.h"

#include "Plugin.h"
#include "PluginManager.h"
#include <Tsar.h>

QDomNode PluginChain::get_state(QDomDocument doc)
{
	QDomNode pluginsNode = doc.createElement("Plugins");
	
	foreach(Plugin* plugin, m_pluginList) {
		pluginsNode.appendChild(plugin->get_state(doc));
	}
	
	return pluginsNode;
}

int PluginChain::set_state( const QDomNode & node )
{
	QDomNode pluginsNode = node.firstChildElement("Plugins");
	QDomNode pluginNode = pluginsNode.firstChild();
	
	while(!pluginNode.isNull()) {
		Plugin* plugin = PluginManager::instance()->get_plugin(pluginNode);
		insert_plugin(plugin);
		pluginNode = pluginNode.nextSibling();
	}
	
	return 1;
}

void PluginChain::insert_plugin( Plugin * plugin, int index )
{
	THREAD_SAVE_CALL_EMIT_SIGNAL(this, plugin, private_add_plugin(Plugin*), pluginAdded(Plugin*));
}

int PluginChain::remove_plugin(Plugin* plugin)
{
	THREAD_SAVE_CALL_EMIT_SIGNAL(this, plugin, private_remove_plugin(Plugin*), pluginRemoved(Plugin*));
	return 1;
}

void PluginChain::private_add_plugin( Plugin * plugin )
{
	m_pluginList.append(plugin);
}

void PluginChain::private_remove_plugin( Plugin * plugin )
{
	int index = m_pluginList.indexOf(plugin);
	
	if (index >=0 ) {
		m_pluginList.takeAt(index);
	} else {
// 		QCritical("Plugin not found in list, this is invalid plugin remove!!!!!");
	}
}

//eof
