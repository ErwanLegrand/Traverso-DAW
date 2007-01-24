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

$Id: PluginChain.h,v 1.6 2007/01/24 21:19:37 r_sijrier Exp $
*/


#ifndef PLUGIN_CHAIN_H
#define PLUGIN_CHAIN_H

#include <ContextItem.h>
#include <QList>
#include <QDomNode>

class Plugin;
class Song;

class PluginChain : public ContextItem
{
	Q_OBJECT
	
public:
	PluginChain(ContextItem* parent, Song* song);
	~PluginChain();
	
	QDomNode get_state(QDomDocument doc);
	int set_state(const QDomNode & node );
	
	Command* add_plugin(Plugin* plugin, bool historable=true);
	Command* remove_plugin(Plugin* plugin);
	
	QList<Plugin* >* get_plugin_list() {return &m_pluginList;}
	
private:
	QList<Plugin* >	m_pluginList;
	Song*		m_song;
	
signals:
	void pluginAdded(Plugin* plugin);
	void pluginRemoved(Plugin* plugin);

private slots:
	void private_add_plugin(Plugin* plugin);
	void private_remove_plugin(Plugin* plugin);


};

#endif

//eof
