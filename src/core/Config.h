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

$Id: Config.h,v 1.7 2007/01/20 18:46:20 r_sijrier Exp $
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QHash>
#include <QVariant>

class Config : public QObject
{
	Q_OBJECT
public:
	QVariant get_property(const QString& type, const QString& property, QVariant defaultValue);
	void set_property(const QString& type, const QString& property, QVariant newValue);
	
	void check_and_load_configuration();
	void reset_settings( );
	
	void init_input_engine();

private:
	Config() {}
	~Config();

	Config(const Config&);

	void save();

	// allow this function to create one instance
	friend Config& config();
	
	void load_configuration();
	
	QHash<QString, QVariant>	m_configs;

};


// use this function to access the settings
Config& config();


#endif
