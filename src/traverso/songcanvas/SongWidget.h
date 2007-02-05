/*
    Copyright (C) 2006 Remon Sijrier 
 
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
 
    $Id: SongWidget.h,v 1.5 2007/02/05 17:12:02 r_sijrier Exp $
*/

#ifndef SONG_WIDGET_H
#define SONG_WIDGET_H

#include <QFrame>

class QGridLayout;
class QGraphicsScene;
		
class TrackPanelViewPort;
class TimeLineViewPort;
class ClipsViewPort;

class Project;
class Song;
class Command;
class SongView;

class SongWidget : public QFrame
{
	Q_OBJECT
public:
	SongWidget(Song* song, QWidget* parent=0);
	~SongWidget();
	
	SongView* m_songView;
	
	void set_use_opengl(bool useOpenGL);
	
private:
	QGridLayout*		m_mainLayout;
	TrackPanelViewPort*	m_trackPanel;
	TimeLineViewPort*	m_timeLine;
	ClipsViewPort*		m_clipsViewPort;
	QGraphicsScene* 	m_scene;

private slots:
	void reload_theme_data();
};


#endif

//eof
