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
 
    $Id: CorrelationMeterWidget.h,v 1.3 2007/01/18 21:39:37 n_doebelin Exp $
*/

#ifndef CORRELATIONMETERWIDGET_H
#define CORRELATIONMETERWIDGET_H

#include <QWidget>
#include <QTimer>

class CorrelationMeter;
class Song;
class Project;
class QLinearGradient;
class QColor;

class CorrelationMeterWidget : public QWidget
{
	Q_OBJECT

public:
        CorrelationMeterWidget(QWidget* parent);

protected:
        void resizeEvent( QResizeEvent* e);
        void paintEvent( QPaintEvent* e);

private:
	QTimer		timer;
	float		coeff;
	float		direction;
	CorrelationMeter*	m_meter;
	QLinearGradient	gradPhase;
	QColor		bgColor, fgColor,
			hgColor, dtColor,
			gcColor;


private slots:
	void		set_project( Project* );
	void		set_song( Song* );
	void		update_data();

};

#endif

