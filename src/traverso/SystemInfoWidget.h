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
 
    $Id: SystemInfoWidget.h,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#ifndef SYSTEMINFOWIDGET_H
#define SYSTEMINFOWIDGET_H

#include "ui_SystemInfoWidget.h"
#include <QWidget>
#include <QTimer>

class SystemInfoWidget : public QWidget, protected Ui::SystemInfoWidget
{
        Q_OBJECT

public:
        SystemInfoWidget(QWidget* parent = 0);
        ~SystemInfoWidget();

private:
        QTimer		sytemResourcesTimer;
        QTimer		cpuUsageTimer;
        int			xrunCount;

public slots:
        void update_system_resources();
        void update_driver_info();
        void update_xrun_info();
        void update_cpu_usage();
};

#endif

//eof





