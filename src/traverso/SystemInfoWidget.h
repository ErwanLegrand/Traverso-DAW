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
 
    $Id: SystemInfoWidget.h,v 1.3 2007/03/05 20:51:24 r_sijrier Exp $
*/

#ifndef RESOURCES_INFO_WIDGET_H
#define RESOURCES_INFO_WIDGET_H

#include <QPushButton>
#include <QTimer>

class ResourcesInfoWidget : public QWidget
{
        Q_OBJECT

public:
        ResourcesInfoWidget(QWidget* parent = 0);
        ~ResourcesInfoWidget() {};

protected:
	void paintEvent( QPaintEvent* e);
	QSize sizeHint () const;
	
private:
        QTimer		updateTimer;
	QString		m_info;

private slots:
        void update_resources_status();
};



class DriverInfoWidget : public QPushButton
{
        Q_OBJECT

public:
        DriverInfoWidget(QWidget* parent = 0);
        ~DriverInfoWidget() {};

private:
        QTimer		updateTimer;
        int		xrunCount;
	void draw_information();

private slots:
        void update_driver_info();
        void update_xrun_info();
};


class HDDSpaceInfoWidget : public QPushButton
{
	Q_OBJECT
public:
	HDDSpaceInfoWidget(QWidget* parent  = 0);
	~HDDSpaceInfoWidget(){};
private:
	QTimer	updateTimer;
private slots:
	void update_harddisk_space_info();
};
#endif

//eof





