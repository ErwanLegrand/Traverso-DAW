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

$Id: BusMonitor.h,v 1.1 2008/05/24 17:41:02 r_sijrier Exp $
*/

#ifndef BUSMONITOR_H
#define BUSMONITOR_H

#include <QWidget>
#include <QList>

class VUMeter;
class Project;
class Sheet;
class QMenu;
class QHBoxLayout;

class BusMonitor :  public QWidget
{
	Q_OBJECT

public:
	BusMonitor(QWidget* parent);
	~BusMonitor();

protected:
	void enterEvent ( QEvent * );
	void mousePressEvent ( QMouseEvent * e );
	void keyPressEvent ( QKeyEvent* e);
	QSize sizeHint () const;
	QSize minimumSizeHint () const;
	
private:
        Sheet*                  m_sheet;
        VUMeter*                m_masterOutMeter;
	QList<VUMeter* >	inMeters;
	QList<VUMeter* >	outMeters;
        QMenu*                  m_menu;
        QHBoxLayout*            m_layout;
	
	void show_menu();

private slots:
	void create_vu_meters();
	void set_project(Project* project);
        void set_sheet(Sheet* sheet);
	void reset_vu_meters();
};

#endif
