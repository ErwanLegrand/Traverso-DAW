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
 
    $Id: SongWidget.cpp,v 1.11 2007/04/03 21:25:25 benjie Exp $
*/

		
#include "SongWidget.h"
#include "TrackPanelViewPort.h"
#include "ClipsViewPort.h"
#include "TimeLineViewPort.h"
#include "SongView.h"
#include "ViewItem.h"
#include "Themer.h"
#include "Config.h"

#include <Song.h>
#include <QtOpenGL>

#include <Debugger.h>


SongWidget::SongWidget(Song* song, QWidget* parent)
	: QFrame(parent)
	, m_song(song)
{
	m_scene = new QGraphicsScene();
	m_vScrollBar = new QScrollBar(this);
	m_hScrollBar = new QScrollBar(this);
	m_hScrollBar->setOrientation(Qt::Horizontal);

	m_trackPanel = new TrackPanelViewPort(m_scene, this);
	m_clipsViewPort = new ClipsViewPort(m_scene, this);
	m_timeLine = new TimeLineViewPort(m_scene, this, m_clipsViewPort);
	
	m_mainLayout = new QGridLayout(this);
	m_mainLayout->addWidget(new QWidget(this), 0, 0);
	m_mainLayout->addWidget(m_timeLine, 0, 1);
	m_mainLayout->addWidget(m_trackPanel, 1, 0);
	m_mainLayout->addWidget(m_clipsViewPort, 1, 1);
	m_mainLayout->addWidget(m_hScrollBar, 2, 1);
	m_mainLayout->addWidget(m_vScrollBar, 1, 2);
	
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(0);

	setLayout(m_mainLayout);
	
	m_sv = new SongView(this, m_clipsViewPort, m_trackPanel, m_timeLine, song);
	m_timeLine->set_songview(m_sv);
	
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
	
	connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
	
	m_usingOpenGL  = false;
	set_use_opengl(config().get_property("Interface", "OpenGL", false).toBool());

	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
}


SongWidget::~ SongWidget()
{
	delete m_trackPanel;
	delete m_clipsViewPort;
	delete m_timeLine;
	delete m_scene;
}


QSize SongWidget::minimumSizeHint() const
{
	return QSize(400, 200);
}

QSize SongWidget::sizeHint() const
{
	return QSize(700, 600);
}

void SongWidget::set_use_opengl( bool useOpenGL )
{
	if (useOpenGL != m_usingOpenGL) {
		m_clipsViewPort->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
		m_trackPanel->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
	}
	m_usingOpenGL = useOpenGL;
}


void SongWidget::load_theme_data()
{
	QList<QGraphicsItem*> list = m_scene->items();
	
	for (int i = 0; i < list.size(); ++i) {
		ViewItem* item = qgraphicsitem_cast<ViewItem*>(list.at(i));
		if (item) {
			item->load_theme_data();
		}
	}
	
}

Song * SongWidget::get_song() const
{
	return m_song;
}

SongView * SongWidget::get_songview() const
{
	return m_sv;
}

//eof

