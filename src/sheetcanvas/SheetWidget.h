/*
    Copyright (C) 2006-2007 Remon Sijrier 
 
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
 
    $Id: SheetWidget.h,v 1.2 2009/02/23 20:12:58 r_sijrier Exp $
*/

#ifndef SONG_WIDGET_H
#define SONG_WIDGET_H

#include <QFrame>
#include "ViewItem.h"

class QGridLayout;
class QGraphicsScene;
class QScrollBar;
		
class TrackPanelViewPort;
class TimeLineViewPort;
class ClipsViewPort;
class SheetPanelViewPort;

class Project;
class Sheet;
class Command;
class SheetView;

class SheetPanelGain : public ViewItem
{
	Q_OBJECT
public:
	SheetPanelGain(ViewItem* parent, Sheet* sheet);
	SheetPanelGain(){}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
	Command* gain_increment();
	Command* gain_decrement();

private slots:
	void update_gain() {update();}

private:
	Sheet* m_sheet;
};

class SheetPanelView : public ViewItem
{
	Q_OBJECT
public:
	SheetPanelView(QGraphicsScene* scene, Sheet* sheet);
	~SheetPanelView() {}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
	SheetPanelGain* m_gainview;
	Sheet* m_sheet;
};

class SheetWidget : public QFrame
{
	Q_OBJECT
public:
	SheetWidget(Sheet* sheet, QWidget* parent=0);
	~SheetWidget();
	
// 	void set_use_opengl(bool useOpenGL);
	Sheet* get_sheet() const;
	SheetView* get_sheetview() const;
	
protected:
	QSize minimumSizeHint () const;
	QSize sizeHint () const;

private:
	SheetView* 		m_sv;
	Sheet*			m_sheet;
	QGridLayout*		m_mainLayout;
	TrackPanelViewPort*	m_trackPanel;
	TimeLineViewPort*	m_timeLine;
	ClipsViewPort*		m_clipsViewPort;
	SheetPanelViewPort*	m_sheetPanelVP;
	QGraphicsScene* 	m_scene;
	QScrollBar*		m_vScrollBar;
	QScrollBar*		m_hScrollBar;
// 	bool			m_usingOpenGL;
	
	friend class SheetView;

private slots:
	void load_theme_data();
};


#endif

//eof
