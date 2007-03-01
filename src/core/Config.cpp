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

$Id: Config.cpp,v 1.10 2007/03/01 16:06:44 r_sijrier Exp $
*/

#include "Config.h"
#include "../config.h"
#include "InputEngine.h"

#include <QSettings>
#include <QString>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const char* CONFIG_FILE_VERSION = "6";


Config& config()
{
	static Config conf;
	return conf;
}

Config::~ Config( )
{
	save();
}

void Config::load_configuration()
{
	QSettings settings;
	QStringList keys = settings.allKeys();
	
	foreach(QString key, keys) {
		m_configs.insert(key, settings.value(key));
	}
}


void Config::reset_settings( )
{
	QSettings settings;
	settings.clear();

	settings.setValue("ProgramVersion", VERSION);
	settings.setValue("ConfigFileVersion", CONFIG_FILE_VERSION);
	
	m_configs.clear();
	
	load_configuration();
}

void Config::check_and_load_configuration( )
{
	QSettings::setPath ( QSettings::NativeFormat, QSettings::UserScope, QDir::homePath () + "/.traverso" );
	
	load_configuration();
	
	init_input_engine();

	// Detect if the config file versions match, if not, there has been most likely 
	// a change, overwrite with the newest version...
	if (m_configs.value("ConfigFileVersion").toString() != CONFIG_FILE_VERSION) {
		reset_settings();
	}

	QString projects_path = m_configs.value("Project/directory").toString();

	QDir dir;
	if ( (projects_path.isEmpty()) || (!dir.exists(projects_path)) ) {
		if (projects_path.isEmpty())
			projects_path = QDir::homePath();

		QString newPath = QFileDialog::getExistingDirectory(0,
				tr("Choose an existing or create a new Project Directory"),
				projects_path );
		if (dir.exists(newPath)) {
			QMessageBox::information( 0, 
					tr("Traverso - Information"), 
					tr("Using existing Project directory: %1\n").arg(newPath), 
					"OK", 
					0 );
		} else if (!dir.mkpath(newPath)) {
			QMessageBox::warning( 0, 
					      tr("Traverso - Warning"), 
					      tr("Unable to create Project directory! \n") +
							      tr("Please check permission for this directory: %1").arg(newPath) );
			return;
		} else {
			QMessageBox::information( 0, 
					tr("Traverso - Information"), 
					tr("Created new Project directory for you here: %1\n").arg(newPath), 
					"OK", 
					0 );
		}
		set_property("Project", "directory", newPath);
	}
}


void Config::init_input_engine( )
{
	ie().init_map(":/keymap");
	ie().set_clear_time(config().get_property("CCE", "clearTime", 2000).toInt());
	ie().set_hold_sensitiveness(config().get_property("CCE", "holdTimeout", 200).toInt());
	ie().set_double_fact_interval(config().get_property("CCE", "doublefactTimeout", 200).toInt());
}



void Config::save( )
{
	QSettings settings;
	
	QHash<QString, QVariant>::const_iterator i = m_configs.constBegin();
	
	while (i != m_configs.constEnd()) {
		settings.setValue(i.key(), i.value());
		++i;
	}
}

QVariant Config::get_property( const QString & type, const QString & property, QVariant defaultValue )
{
	QVariant var = defaultValue;
	
	if (m_configs.contains(type + ("/") + property)) {
		var = m_configs.value(type + ("/") + property);
	} else {
		m_configs.insert(type + "/" + property, defaultValue);
	}
	
	return var;
}

void Config::set_property( const QString & type, const QString & property, QVariant newValue )
{
	m_configs.insert(type + "/" + property, newValue);
}


//eof
