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

$Id: PluginChain.cpp,v 1.5 2006/11/08 14:47:35 r_sijrier Exp $
*/
 
#include "PluginChain.h"

#include "Plugin.h"
#include "PluginManager.h"
#include <Tsar.h>
#include <InputEngine.h>
#include <Song.h>
#include <AddRemoveItemCommand.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PluginChain::PluginChain(ContextItem* parent, Song* song)
	: ContextItem(parent), m_song(song)
{
	m_hs = parent->get_history_stack();
}

PluginChain::~ PluginChain( )
{
	PENTERDES;
	foreach(Plugin* plugin, m_pluginList) {
		delete plugin;
	}
}


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
		ie().process_command(add_plugin(plugin, false));
		pluginNode = pluginNode.nextSibling();
	}
	
	return 1;
}


Command* PluginChain::add_plugin(Plugin * plugin, bool historable)
{
	plugin->set_history_stack(get_history_stack());
	
	return new AddRemoveItemCommand( this, plugin, historable, m_song,
						"private_add_plugin(Plugin*)", "pluginAdded(Plugin*)",
						"private_remove_plugin(Plugin*)", "pluginRemoved(Plugin*)",
				       tr("Add Plugin (%1)").arg(plugin->get_name()));
}


Command* PluginChain::remove_plugin(Plugin* plugin)
{
	return new AddRemoveItemCommand( this, plugin, true, m_song,
					 "private_remove_plugin(Plugin*)", "pluginRemoved(Plugin*)",
					 "private_add_plugin(Plugin*)", "pluginAdded(Plugin*)",
				       tr("Remove Plugin (%1)").arg(plugin->get_name()));
}


void PluginChain::private_add_plugin( Plugin * plugin )
{
	m_pluginList.append(plugin);
}


void PluginChain::private_remove_plugin( Plugin * plugin )
{
	int index = m_pluginList.indexOf(plugin);
	
	if (index >=0 ) {
		m_pluginList.removeAt(index);
	} else {
// 		QCritical("Plugin not found in list, this is invalid plugin remove!!!!!");
	}
}

//eof
