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

$Id: SongInfoWidget.cpp,v 1.9 2006/10/18 12:08:56 r_sijrier Exp $
*/

#include "SongInfoWidget.h"
#include "ui_SongInfoWidget.h"

#include "libtraversocore.h"
#include "ColorManager.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
//#include "Debugger.h"

SongInfoWidget::SongInfoWidget( QWidget * parent )
		: QWidget(parent)
{
	setupUi(this);

	QPalette palette;
	palette.setColor(QPalette::Background, cm().get("INFO_WIDGET_BACKGROUND"));
	setPalette(palette);
	setAutoFillBackground(true);

	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(&smpteTimer, SIGNAL(timeout()), this, SLOT(update_smpte()));
}

SongInfoWidget::~ SongInfoWidget( )
{}

void SongInfoWidget::set_project(Project* project )
{
	if (project) {
		m_project = project;
		connect(m_project, SIGNAL(currentSongChanged(Song*)), this, SLOT(set_song(Song*)));
	} else {
		smpteLabel->setText("-");
		zoomLabel->setText("-");
		snapStatusLabel->setText("-");
		masterGainLabel->setText("-");
		songNameLabel->setText("-");
		// Just in case the timer is still running, there is _no_ Song anymore!
		stop_smpte_update_timer();
	}
}

void SongInfoWidget::set_song(Song* song)
{
	m_song = song;
	connect(m_song, SIGNAL(hzoomChanged()), this, SLOT(update_zoom()));
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(stop_smpte_update_timer()));
	connect(m_song, SIGNAL(transferStarted()), this, SLOT(start_smpte_update_timer()));
	connect(m_song, SIGNAL(cursorPosChanged()), this, SLOT(update_smpte()));
	connect(m_song, SIGNAL(propertieChanged()), this, SLOT (update_properties()));
	connect(m_song, SIGNAL(masterGainChanged()), this, SLOT(update_master_gain()));
	
	update_zoom();
	update_smpte();
	update_properties();
	update_master_gain();
}

void SongInfoWidget::start_smpte_update_timer( )
{
	smpteTimer.start(150);
}

void SongInfoWidget::stop_smpte_update_timer( )
{
	smpteTimer.stop();
}

void SongInfoWidget::update_smpte( )
{
	nframes_t bpos;
	if (!m_song->is_transporting())
		bpos = m_song->xpos_to_frame(cpointer().clip_area_x());
	else
		bpos = m_song->get_transport_frame();

	smpteLabel->setText( frame_to_smpte(bpos, m_song->get_rate()) );
}

void SongInfoWidget::update_zoom( )
{
	QByteArray zoomLevel = "1:" + QByteArray::number(Peak::zoomStep[m_song->get_hzoom()]);
	zoomLabel->setText(zoomLevel);
}

void SongInfoWidget::update_properties( )
{
	QString name = m_song->get_title();
	name.prepend(QString::number(m_song->get_id()) + " - " );
	songNameLabel->setText(name);

	QString snaptext = m_song->is_snap_on() ? "ON" : "OFF";
	snapStatusLabel->setText(snaptext);
}

void SongInfoWidget::update_master_gain( )
{
	masterGainLabel->setText(coefficient_to_dbstring(m_song->get_gain()));
}

//eof
