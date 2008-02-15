/*
    Copyright (C) 2008 Remon Sijrier
 
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

#ifndef METER_WIDGET_H
#define METER_WIDGET_H

#include <QWidget>
#include <QTimer>

#include <ViewPort.h>
#include <ViewItem.h>

class MeterView;
class Sheet;
class Project;
class Plugin;

class MeterWidget : public ViewPort
{

public:
	MeterWidget(QWidget* parent, MeterView* item);
	~MeterWidget();

	void get_pointed_context_items(QList<ContextItem* > &list);

protected:
	void resizeEvent( QResizeEvent* e);
	void hideEvent ( QHideEvent * event );
	void showEvent ( QShowEvent * event );
	QSize minimumSizeHint () const;
	QSize sizeHint () const;
	MeterView* m_item;
};

class MeterView : public ViewItem
{
	Q_OBJECT

public:
	MeterView(MeterWidget* widget);
	~MeterView();

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};
	virtual void resize();
	void hide_event();
	void show_event();

	void		set_sheet( Sheet* );
	
protected:
	MeterWidget* 	m_widget;
	Plugin*		m_meter;
	QTimer		timer;
	QTimer		m_delayTimer;
	Project*	m_project;
	Sheet*		m_sheet;

public slots:
	void		set_project( Project* );
	
private slots:
	virtual void	update_data() {};
	void		transport_started();
	void		transport_stopped();
	void		delay_timeout();
};

#endif

 
