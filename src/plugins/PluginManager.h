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

$Id: PluginManager.h,v 1.1 2006/07/31 13:24:46 r_sijrier Exp $
*/


#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <slv2/slv2.h>

#include <QDomDocument>

class Plugin;

class PluginManager
{

public:
	~PluginManager();
	
	static PluginManager* instance();
	
	Plugin* get_plugin(const QDomNode node);
	
	SLV2List get_slv2_plugin_list();
	
private:
	PluginManager();
	
	static PluginManager* m_instance;	
	SLV2List	slv2PluginList;
	
	void init();
};

#endif

//eof

