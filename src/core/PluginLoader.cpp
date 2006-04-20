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
 
    $Id: PluginLoader.cpp,v 1.1 2006/04/20 14:51:40 r_sijrier Exp $
*/

#include "../config.h"
#include <PluginLoader.h>
#include <QStringList>

// QHash<AudioPlugin> PluginLoader::pluginContainer = QHash<AudioPlugin>();
// typedef AudioPlugin* (*PluginConstructor)() ;

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

int PluginLoader::probe_plugins()
{
        PENTER;
        int totalPlugins = 0;
        QDir d( RESOURCES_DIR "plugins/");
        printf("Probing plugins ...\n");
        QStringList list;
        list << "*.so";
        d.setNameFilters(list);
        d.setFilter(QDir::Files);
        d.setSorting(QDir::Name);

        // read the list of files
        /*	const QFileInfoList *list = d.entryInfoList();
        	QFileInfoListIterator it( *list );
        	QFileInfo *fi;
        	int numPluginsLoaded=0;*/
        //FIXME Make it work with qt4
        /*	while ( (fi = it.current()) != 0 )
        		{
        		QString pluginName= fi->fileName();
        		printf( "Found %s. Loading it ( plugin will say hello )\n", pluginName.ascii() );
        		++it;
        		QString fn = QString( RESOURCES_DIR ) + "plugins/" + pluginName;
        		QLibrary* pluginFactory = new QLibrary(fn);
        		if (!pluginFactory->load())
        			{
        			printf("Cannot load %s. Maybe it is not a shared object ?!\n",pluginFactory->library().ascii());
        			continue;
        			}
         
        		PluginConstructor constructor = (PluginConstructor) pluginFactory->resolve( "construct" );
        		AudioPlugin* plugin;
        		if ( constructor ) 
        			{
        			plugin = constructor();
        			if (plugin)
        				{
        				printf("\t");
        				plugin->hello();
        				}
        			}
        		else
        			{
        			printf("No extern constructor defined\n");
        			continue;
        			}
        		pluginContainer.insert(pluginName,plugin);
        		totalPlugins++;
        		}*/
        printf("%d plugin(s) found.\n",totalPlugins);
        return 1;
}

//eof

