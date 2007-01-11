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
 
    $Id: SongWidget.cpp,v 1.4 2007/01/11 20:11:26 r_sijrier Exp $
*/

		
#include "SongWidget.h"
#include "TrackPanelViewPort.h"
#include "ClipsViewPort.h"
#include "TimeLineViewPort.h"
#include "SongView.h"

#include <Song.h>
#include <QtOpenGL>

#include <Debugger.h>


SongWidget::SongWidget(Song* song, QWidget* parent)
	: QFrame(parent)
{
	scene = new QGraphicsScene(this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);

	m_trackPanel = new TrackPanelViewPort(scene, this);
	m_clipsViewPort = new ClipsViewPort(scene, this);
	m_timeLine = new TimeLineViewPort(scene, this, m_clipsViewPort);
	
	m_clipsViewPort->setScene(scene);
	
	m_mainLayout = new QGridLayout(this);
	m_mainLayout->addWidget(new QWidget(this), 0, 0);
	m_mainLayout->addWidget(m_timeLine, 0, 1);
	m_mainLayout->addWidget(m_trackPanel, 1, 0);
	m_mainLayout->addWidget(m_clipsViewPort, 1, 1);
	
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(0);
	
	setLayout(m_mainLayout);
	
	m_songView = new SongView(m_clipsViewPort, m_trackPanel, m_timeLine, song);
	m_timeLine->set_songview(m_songView);
	
	connect(m_clipsViewPort->horizontalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_timeLine->horizontalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(m_timeLine->horizontalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_clipsViewPort->horizontalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(m_clipsViewPort->verticalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_trackPanel->verticalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(m_trackPanel->verticalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_clipsViewPort->verticalScrollBar(), 
		SLOT(setValue(int)));
	
	
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
}

void SongWidget::set_use_opengl( bool useOpenGL )
{
	m_clipsViewPort->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
	m_trackPanel->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
}


//eof
