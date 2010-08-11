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

#include "Config.h"
#include "../config.h"
#include "InputEngine.h"
#include "AudioDevice.h"
#include "Utils.h"

#include <QSettings>
#include <QString>
#include <QDir>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const char* CONFIG_FILE_VERSION = "8";


Config& config()
{
	static Config conf;
	return conf;
}

Config::~ Config( )
{
}

void Config::load_configuration()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Traverso-DAW", "Traverso");
	
	QStringList keys = settings.allKeys();
	
	foreach(const QString &key, keys) {
		m_configs.insert(key, settings.value(key));
	}
	
	set_audiodevice_driver_properties();
}

void Config::reset_settings( )
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Traverso-DAW", "Traverso");
	
	settings.clear();

        QString projectspath = QDir::homePath() + "/" + tr("TraversoProjects");
        QDir dir;
        dir.mkpath(projectspath);
        settings.setValue("Project/directory", projectspath);
	settings.setValue("ProgramVersion", VERSION);
	settings.setValue("ConfigFileVersion", CONFIG_FILE_VERSION);
	
	m_configs.clear();
	
	load_configuration();
}

void Config::check_and_load_configuration( )
{
        QSettings::setPath (QSettings::IniFormat, QSettings::UserScope, QDir::homePath () + "/.traverso");

        load_configuration();

	// Detect if the config file versions match, if not, there has been most likely 
	// a change, overwrite with the newest version...
	if (m_configs.value("ConfigFileVersion").toString() != CONFIG_FILE_VERSION) {
		reset_settings();
	}
}

#include <QPluginLoader>
#include "CommandPlugin.h"

void Config::init_input_engine( )
{
        foreach (QObject* obj, QPluginLoader::staticInstances()) {
                CommandPlugin* plug = qobject_cast<CommandPlugin*>(obj);
                if (plug) {
                        plug->create_menu_translations();
                }
        }

#if !defined (STATIC_BUILD)
        QDir pluginsDir("lib/commandplugins");
        foreach (const QString &fileName, pluginsDir.entryList(QDir::Files)) {
                QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
                CommandPlugin* plug = qobject_cast<CommandPlugin*>(loader.instance());
                if (plug) {
                        plug->create_menu_translations();
        }

#endif

	ie().init_map(config().get_property("CCE", "keymap", "default").toString());
        ie().set_hold_sensitiveness(config().get_property("CCE", "holdTimeout", 180).toInt());
        ie().set_double_fact_interval(config().get_property("CCE", "doublefactTimeout", 220).toInt());
}



void Config::save( )
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Traverso-DAW", "Traverso");
	
	QHash<QString, QVariant>::const_iterator i = m_configs.constBegin();
	
	while (i != m_configs.constEnd()) {
		settings.setValue(i.key(), i.value());
		++i;
	}
	
	set_audiodevice_driver_properties();
	
	emit configChanged();
}

QVariant Config::get_property( const QString & type, const QString & property, QVariant defaultValue )
{
	QVariant var = defaultValue;
	QString key = type + ("/") + property;
	
	if (m_configs.contains(key)) {
		var = m_configs.value(key);
	} else {
		m_configs.insert(key, defaultValue);
	}
	
	return var;
}

void Config::set_property( const QString & type, const QString & property, QVariant newValue )
{
	m_configs.insert(type + "/" + property, newValue);
}


void Config::set_audiodevice_driver_properties()
{
	QHash<QString, QVariant> hardwareconfigs;
	hardwareconfigs.insert("jackslave", get_property("Hardware", "jackslave", false));
	hardwareconfigs.insert("numberofperiods", get_property("Hardware", "numberofperiods", 3));
	
	audiodevice().set_driver_properties(hardwareconfigs);
}

