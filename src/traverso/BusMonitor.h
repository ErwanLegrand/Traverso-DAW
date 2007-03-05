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

$Id: BusMonitor.h,v 1.4 2007/03/05 20:51:23 r_sijrier Exp $
*/

#ifndef BUSMONITOR_H
#define BUSMONITOR_H

#include <QWidget>
#include <QList>
#include <QHBoxLayout>

class VUMeter;
class Interface;

class BusMonitor :  public QWidget
{
	Q_OBJECT

public:
	BusMonitor(QWidget* parent, Interface* iface);
	~BusMonitor();

protected:
	void resizeEvent( QResizeEvent* e);
	QSize sizeHint () const;
	QSize minimumSizeHint () const;
	
private:
	QHBoxLayout* 		layout;
	QHBoxLayout* 		vumeterLayout;
	QWidget* 			vumeterLayoutWidget;
	QList<VUMeter* >		inMeters;
	QList<VUMeter* >		outMeters;
	Interface*				m_interface;

private slots:
	void create_vu_meters();
};

#endif
