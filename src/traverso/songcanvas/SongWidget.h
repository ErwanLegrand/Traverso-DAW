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
 
    $Id: SongWidget.h,v 1.13 2007/11/05 15:49:32 r_sijrier Exp $
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
class SongPanelViewPort;

class Project;
class Song;
class Command;
class SongView;

class SongPanelGain : public ViewItem
{
	Q_OBJECT
public:
	SongPanelGain(ViewItem* parent, Song* song);
	SongPanelGain(){}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
	Command* gain_increment();
	Command* gain_decrement();

private slots:
	void update_gain() {update();}

private:
	Song* m_song;
};

class SongPanelView : public ViewItem
{
	Q_OBJECT
public:
	SongPanelView(QGraphicsScene* scene, Song* song);
	~SongPanelView() {}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
	SongPanelGain* m_gainview;
	Song* m_song;
};

class SongWidget : public QFrame
{
	Q_OBJECT
public:
	SongWidget(Song* song, QWidget* parent=0);
	~SongWidget();
	
	void set_use_opengl(bool useOpenGL);
	Song* get_song() const;
	SongView* get_songview() const;
	
protected:
	QSize minimumSizeHint () const;
	QSize sizeHint () const;

private:
	SongView* 		m_sv;
	Song*			m_song;
	QGridLayout*		m_mainLayout;
	TrackPanelViewPort*	m_trackPanel;
	TimeLineViewPort*	m_timeLine;
	ClipsViewPort*		m_clipsViewPort;
	SongPanelViewPort*	m_songPanelVP;
	QGraphicsScene* 	m_scene;
	QScrollBar*		m_vScrollBar;
	QScrollBar*		m_hScrollBar;
	bool			m_usingOpenGL;
	
	friend class SongView;

private slots:
	void load_theme_data();
};


#endif

//eof
