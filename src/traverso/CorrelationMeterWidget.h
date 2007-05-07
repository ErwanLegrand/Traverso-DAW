/*
    Copyright (C) 2005-2006 Nicola Doebelin
 
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

#ifndef CORRELATIONMETERWIDGET_H
#define CORRELATIONMETERWIDGET_H

#include <QWidget>
#include <QTimer>

#include <ViewPort.h>
#include <ViewItem.h>

class CorrelationMeter;
class CorrelationMeterView;
class Song;
class Project;
class Command;
class QLinearGradient;
class QColor;


class CorrelationMeterWidget : public ViewPort
{

public:
	CorrelationMeterWidget(QWidget* parent);
	~CorrelationMeterWidget();

	void get_pointed_context_items(QList<ContextItem* > &list);

protected:
        void resizeEvent( QResizeEvent* e);
	void hideEvent ( QHideEvent * event );
	void showEvent ( QShowEvent * event );
	QSize minimumSizeHint () const;
	QSize sizeHint () const;

private:
	CorrelationMeterView* m_item;

};

class CorrelationMeterView : public ViewItem
{
	Q_OBJECT

	Q_CLASSINFO("set_mode", tr("Toggle display range"))

public:
        CorrelationMeterView(CorrelationMeterWidget* widget);
	~CorrelationMeterView();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void resize();
	void hide_event();
	void show_event();

private:
	CorrelationMeterWidget* m_widget;
	QTimer		timer;
	float		coeff;
	float		direction;
	CorrelationMeter*	m_meter;
	Song*		m_song;
	QLinearGradient	gradPhase;
	QColor		bgColor, fgColor,
			hgColor, dtColor;
	int		range;

	void save_configuration();
	void load_configuration();

private slots:
	void		set_project( Project* );
	void		set_song( Song* );
	void		update_data();

public slots:
	Command*	set_mode();

};

#endif

