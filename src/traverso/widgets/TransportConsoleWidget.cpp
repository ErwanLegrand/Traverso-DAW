/*
    Copyright (C) 2008 Nicola Doebelin

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

#include "TransportConsoleWidget.h"
#include "libtraversocore.h"
#include "Themer.h"

#include <QPixmap>
#include <QGridLayout>
#include <QToolButton>
#include <QAction>
#include <QSize>
#include <QFrame>
#include <QApplication>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const int HEIGHT_THRESHOLD = 90;
static const float FONT_HEIGHT = 0.8;

TransportConsoleWidget::TransportConsoleWidget(QWidget* parent)
	: QToolBar(parent)
{
	setEnabled(false);

	m_timeLabel = new QLineEdit(this);
	m_timeLabel->setReadOnly(true);
	m_timeLabel->setAlignment(Qt::AlignCenter);
	m_timeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_timeLabel->setFont(themer()->get_font("Playhead:fontscale:info"));

	m_toStartAction = addAction(QIcon(":/skipleft"), tr("Skip to Start"), this, SLOT(to_start()));
	m_toLeftAction = addAction(QIcon(":/seekleft"), tr("Previous Snap Position"), this, SLOT(to_left()));
	m_recAction = addAction(QIcon(":/record"), tr("Record"), this, SLOT(rec_toggled()));
	m_playAction = addAction(QIcon(":/playstart"), tr("Play / Stop"), this, SLOT(play_toggled()));
	m_toRightAction = addAction(QIcon(":/seekright"), tr("Skip to End"), this, SLOT(to_right()));
	m_toEndAction = addAction(QIcon(":/skipright"), tr("Next Snap Position"), this, SLOT(to_end()));

	addWidget(m_timeLabel);

	m_recAction->setCheckable(true);
	m_playAction->setCheckable(true);

	m_lastSnapPosition = TimeRef();
	m_skipTimer.setSingleShot(true);

	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(update_label()));
}


void TransportConsoleWidget::set_project(Project* project)
{
	m_project = project;
	if (m_project) {
		connect(m_project, SIGNAL(currentSheetChanged(Sheet*)), this, SLOT(set_sheet(Sheet*)));
	} else {
		m_updateTimer.stop();
		set_sheet(0);
	}
}

void TransportConsoleWidget::set_sheet(Sheet* sheet)
{
	m_sheet = sheet;

	if (!m_sheet)
	{
		m_updateTimer.stop();
		setEnabled(false);
		return;
	}

	setEnabled(true);

	connect(m_sheet, SIGNAL(recordingStateChanged()), this, SLOT(update_recording_state()));
	connect(m_sheet, SIGNAL(transferStarted()), this, SLOT(transfer_started()));
	connect(m_sheet, SIGNAL(transferStopped()), this, SLOT(transfer_stopped()));
	connect(m_sheet, SIGNAL(transportPosSet()), this, SLOT(update_label()));

	update_label();
}

void TransportConsoleWidget::to_start()
{
	m_sheet->set_transport_pos((TimeRef)0.0);
	m_sheet->set_work_at((TimeRef)0.0);
}

// the timer is used to allow 'hopping' to the left from snap position to snap position
// even during playback.
void TransportConsoleWidget::to_left()
{
	TimeRef p = m_sheet->get_transport_location();

	if (m_skipTimer.isActive()) 
	{
		p = m_lastSnapPosition;
	}

	SnapList* slist = m_sheet->get_snap_list();
	TimeRef newpos = slist->prev_snap_pos(p);
	m_sheet->set_transport_pos(newpos);

	m_lastSnapPosition = newpos;
	m_skipTimer.start(500);
}

void TransportConsoleWidget::rec_toggled()
{
	m_sheet->set_recordable();
}

void TransportConsoleWidget::play_toggled()
{
	m_sheet->start_transport();
}

void TransportConsoleWidget::to_end()
{
	// stop the transport, no need to play any further than the end of the sheet
	if (m_sheet->is_transport_rolling())
	{
		m_sheet->start_transport();
	}
	m_sheet->set_transport_pos(m_sheet->get_last_location());
}

void TransportConsoleWidget::to_right()
{
	SnapList* slist = m_sheet->get_snap_list();
	TimeRef p = m_sheet->get_transport_location();
	TimeRef newpos = slist->next_snap_pos(p);
	m_sheet->set_transport_pos(newpos);
}

void TransportConsoleWidget::transfer_started()
{
    // use an odd number for the update interval, because
	// a round number (e.g. 100) lets the last digit stay
	// the same most of the time, but not always, which 
	// looks jerky
	m_updateTimer.start(123);
	m_playAction->setChecked(true);
	m_playAction->setIcon(QIcon(":/playstop"));
	m_recAction->setEnabled(false);

	// this is needed when the record button is pressed, but no track is armed.
	// uncheck the rec button in that case
	if (!m_sheet->is_recording()) {
		m_recAction->setChecked(false);
	}
}

void TransportConsoleWidget::transfer_stopped()
{
	m_updateTimer.stop();
	m_playAction->setChecked(false);
	m_playAction->setIcon(QIcon(":/playstart"));
	m_recAction->setEnabled(true);
}

void TransportConsoleWidget::update_recording_state()
{
	if (!m_sheet)
	{
		return;
	}

	if (m_sheet->is_recording()) {
		QString recordFormat = config().get_property("Recording", "FileFormat", "wav").toString();
		int count = 0;
		foreach(Track* track, m_sheet->get_tracks()) {
			if (track->armed()) {
				count++;
			}
		}
		info().information(tr("Recording to %1 Tracks, encoding format: %2").arg(count).arg(recordFormat));
		m_recAction->setChecked(true);
	} else {
		m_recAction->setChecked(false);
	}
}

void TransportConsoleWidget::update_label()
{
	QString currentTime;
	
	if (!m_sheet) {
		currentTime = "0:00.0";
	} else {
		currentTime = timeref_to_ms_2(m_sheet->get_transport_location());
	}
	m_timeLabel->setText(currentTime);
}

//eof

