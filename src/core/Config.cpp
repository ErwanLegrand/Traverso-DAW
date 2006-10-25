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

$Id: Config.cpp,v 1.3 2006/10/25 14:52:38 r_sijrier Exp $
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

static const char* CONFIG_FILE_VERSION = "3";


Config& config()
{
	static Config conf;
	return conf;
}

void Config::load_configuration()
{
	QSettings settings;
	
	m_intConfigs.insert("WaveFormRectified", settings.value("WaveFormRectified", "0").toInt());
	m_intConfigs.insert("WaveFormMerged", settings.value("WaveFormMerged", "0").toInt());
	m_intConfigs.insert("trackCreationCount", settings.value("trackCreationCount", "0").toInt());
	m_intConfigs.insert("hzoomLevel", settings.value("hzoomLevel", "0").toInt());
	
	m_intConfigs.insert("Project/loadLastUsed", settings.value("Project/loadLastUsed", "0").toInt());
	
	m_intConfigs.insert("CCE/clearTime", settings.value("CCE/clearTime", "2000").toInt());
	m_intConfigs.insert("CCE/holdTimeout", settings.value("CCE/holdTimeout", "200").toInt());
	m_intConfigs.insert("CCE/doublefactTimeout", settings.value("CCE/doublefactTimeout", "200").toInt());
	
	m_intConfigs.insert("Hardware/samplerate", settings.value("Hardware/samplerate", "44100").toInt());
	m_intConfigs.insert("Hardware/bufferSize", settings.value("Hardware/bufferSize", "1024").toInt());
	m_intConfigs.insert("Hardware/PreBufferSize", settings.value("Hardware/PreBufferSize", "32768").toInt());
	
	m_stringConfigs.insert("ProgramVersion", settings.value("ProgramVersion", "0").toString());
	m_stringConfigs.insert("ProgramVersion", settings.value("ProgramVersion", "0").toString());
	m_stringConfigs.insert("ConfigFileVersion", settings.value("ConfigFileVersion", "0").toString());

	m_stringConfigs.insert("Project/current", settings.value("Project/current", "0").toString());
	m_stringConfigs.insert("Project/directory", settings.value("Project/directory", "0").toString());

	// Use Jack by default on mac os x, since thats the only supported driver there!
#ifdef MAC_OS_BUILD
	m_stringConfigs.insert("Hardware/", settings.value("Hardware/drivertype", "Jack").toString());
#else
	m_stringConfigs.insert("Hardware/drivertype", settings.value("Hardware/drivertype", "ALSA").toString());
#endif
}


void Config::reset_settings( )
{
	QSettings settings;
	settings.clear();

	settings.setValue("ProgramVersion", VERSION);
	settings.setValue("ConfigFileVersion", CONFIG_FILE_VERSION);
	settings.setValue("trackCreationCount", 6);
	settings.setValue("hzoomLevel", 2048);
	settings.setValue("WaveFormRectified", 0);

	settings.beginGroup("Project");
	settings.setValue("current", "Untitled");
	settings.setValue("loadLastUsed", 1);
	settings.setValue("directory", "");
	settings.endGroup();

	settings.beginGroup("CCE");
	settings.setValue("clearTime", 2000);
	settings.setValue("holdTimeout", 200);
	settings.setValue("doublefactTimeout", 200);
	settings.endGroup();

	settings.beginGroup("Hardware");
	settings.setValue("samplerate", 44100);
	settings.setValue("bufferSize", 1024);
	settings.setValue("PreBufferSize", 32768);
// Use Jack by default on mac os x, since thats the only supported driver there!
#ifdef MAC_OS_BUILD
	settings.setValue("drivertype", "Jack");
#else
	settings.setValue("drivertype", "ALSA");
#endif
	settings.endGroup();
	
	load_configuration();
}

void Config::check_and_load_configuration( )
{
#if QT_VERSION >= 0x040100
	QSettings::setPath ( QSettings::NativeFormat, QSettings::UserScope, QDir::homePath () + "/.traverso" );
#endif
	
	load_configuration();
	
	init_input_engine();

	// Detect if the config file versions match, if not, there has been most likely 
	// a change, overwrite with the newest version...
	if (m_stringConfigs.value("ConfigFileVersion") != CONFIG_FILE_VERSION) {
		reset_settings();
	}

	QString projects_path = m_stringConfigs.value("Project/directory");

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
		set_project_property("directory", newPath);
	}
}


void Config::init_input_engine( )
{
	ie().init_map(":/keymap");
	ie().set_clear_time(config().get_ie_int_property("clearTime"));
	ie().set_hold_sensitiveness(config().get_ie_int_property("holdTimeout"));
	ie().set_double_fact_interval(config().get_ie_int_property("doublefactTimeout"));
}


int Config::get_int_property(const QString& property, int defaultValue) const
{
	return m_intConfigs.value(property, defaultValue);
}

int Config::get_hardware_int_property( const QString& property, int defaultValue ) const
{
	return m_intConfigs.value(QString("Hardware/").append(property), defaultValue);
}

int Config::get_project_int_property( const QString & property, int defaultValue ) const
{
	return m_intConfigs.value(QString("Project/").append(property), defaultValue);
}

QString Config::get_project_string_property( const QString & property, const QString & defaultValue ) const
{
	return m_stringConfigs.value(QString("Project/").append(property), defaultValue);
}

int Config::get_ie_int_property( const QString & property, int defaultValue ) const
{
	return m_intConfigs.value(QString("CCE/").append(property), defaultValue);
}

QString Config::get_hardware_string_property( const QString & property, const QString & defaultValue ) const
{
	return m_stringConfigs.value(QString("Hardware/").append(property), defaultValue);
}


void Config::set_project_property( const QString & property, const QString& newValue )
{
	printf("setting property (%s) to %s\n", property.toAscii().data(), newValue.toAscii().data());
	QSettings settings;
	settings.setValue(QString("Project/").append(property), newValue);
}

void Config::set_hardware_property( const QString & property, int newValue )
{
	m_intConfigs.insert(QString("Hardware/").append(property), newValue);
}

void Config::set_hardware_property( const QString & property, const QString & newValue )
{
	m_stringConfigs.insert(QString("Hardware/").append(property), newValue);
}

void Config::save( )
{
	QSettings settings;
	
	QHash<QString, int>::const_iterator i = m_intConfigs.constBegin();
	
	while (i != m_intConfigs.constEnd()) {
		settings.setValue(i.key(), i.value());
		++i;
	}
 	

	QHash<QString, QString>::const_iterator j = m_stringConfigs.constBegin();
	
	while (j != m_stringConfigs.constEnd()) {
		settings.setValue(j.key(), j.value());
		++j;
	}

}



//eof
