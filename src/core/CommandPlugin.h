/*
Copyright (C) 2007 Remon Sijrier 

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


#ifndef COMMAND_PLUGIN_H
#define COMMAND_PLUGIN_H

#include <QtPlugin>
#include <Command.h>
#include <QStringList>
#include <QHash>
#include <QVariant>

class CommandInterface
{
public:
	virtual ~CommandInterface() {}
	virtual Command* create(QObject* obj, const QString& command, QVariantList arguments) = 0;
};

Q_DECLARE_INTERFACE(CommandInterface, "org.traversodaw.Command.CommandInterface/1.0");


/**
 * \class CommandPlugin
 * \brief An abstract class to create new Command's which can be loaded dynamically
 *	
 */

class CommandPlugin : public QObject, public CommandInterface
{
	Q_OBJECT
	Q_INTERFACES(CommandInterface);

public:
	virtual ~CommandPlugin() {}
	virtual Command* create(QObject* obj, const QString& command, QVariantList arguments) = 0;
	
	virtual QStringList commands() const {
		return QStringList(m_dict.keys());
	}
	
	virtual bool implements(const QString& command) const {
		return m_dict.contains(command);
	}
	
protected:
	QHash<QString, int> m_dict;
};

#endif

//eof
