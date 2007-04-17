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
 
    $Id: Themer.h,v 1.1 2007/04/17 11:51:20 r_sijrier Exp $
*/

#ifndef COLORMANAGER_H
#define COLORMANAGER_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QHash>
#include <QCursor>
#include <QString>
#include <QFileSystemWatcher>
#include <QVariant>
#include <QPalette>

class Themer : public QObject
{
	Q_OBJECT
public:
        void save();
        void load();
        
        void set_path_and_theme(const QString& path, const QString& theme);
	void use_builtin_theme(const QString& theme);
	void set_color_adjust_value(int value);
        
	QColor get_color(const QString& name) const;
        QFont get_font(const QString& fontname) const;
        QVariant get_property(const QString& propertyname, const QVariant& defaultValue=0) const;
	QPalette system_palette() const {return m_systempallete;}
	QStringList get_builtin_themes();
	QCursor get_cursor(const QString& name) const;

	static Themer* instance();
	
private:
        Themer();

        QHash<QString, QColor>	 m_colors;
	QHash<QString, QVariant> m_properties;
	QHash<QString, QFont>	m_fonts;
	QHash<QString, QCursor>	m_cursors;
	QFileSystemWatcher*	m_watcher;
        QString			m_themefile;
	int			m_coloradjust;
	QPalette 		m_systempallete;
	QString			m_currentTheme;

	static Themer* m_instance;
        
       
private slots:
	void reload_on_themefile_change(const QString&);
	
signals:
	void themeLoaded();
};

// use this function to get the Colormanager object
Themer* themer();

#endif

