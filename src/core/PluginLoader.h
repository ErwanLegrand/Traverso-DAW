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
 
    $Id: PluginLoader.h,v 1.1 2006/04/20 14:51:40 r_sijrier Exp $
*/

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H


#include <qlibrary.h>
#include <QHash>
#include <qstring.h>
#include <qfile.h>
#include <qdir.h>

#include <libtraverso.h>

class PluginLoader
{
public :
        static int probe_plugins();
        // 		static QHash<AudioPlugin> pluginContainer;
};


#endif

