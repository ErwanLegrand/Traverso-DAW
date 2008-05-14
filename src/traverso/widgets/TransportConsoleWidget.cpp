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
#include "defines.h"

#include <QPixmap>
#include <QGridLayout>
#include <QToolButton>
#include <QAction>
#include <QSize>
#include <QFrame>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const int HEIGHT_THRESHOLD = 90;

TransportConsoleWidget::TransportConsoleWidget(QWidget* parent)
	: QWidget(parent)
{
	setEnabled(false);

	m_layout = new QGridLayout(this);
	m_label = new QLabel(this);
	m_label->setAlignment(Qt::AlignCenter);
	m_label->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	m_label->setMinimumWidth(80);
	m_label->setScaledContents(true);
	m_label->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	QToolButton* buttonToStart = new QToolButton(this);
	QToolButton* buttonToLeft = new QToolButton(this);
	QToolButton* buttonRec = new QToolButton(this);
	QToolButton* buttonPlay = new QToolButton(this);
	QToolButton* buttonToRight = new QToolButton(this);
	QToolButton* buttonToEnd = new QToolButton(this);

	buttonToStart->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
	buttonToLeft->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
	buttonRec->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
	buttonPlay->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
	buttonToRight->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
	buttonToEnd->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

	m_toStartAction = new QAction(this);
	m_toLeftAction = new QAction(this);
	m_recAction = new QAction(this);
	m_playAction = new QAction(this);
	m_toEndAction = new QAction(this);
	m_toRightAction = new QAction(this);

	m_recAction->setCheckable(true);
	m_playAction->setCheckable(true);

	m_toStartAction->setIcon(QIcon(":/skipleft"));
	m_toLeftAction->setIcon(QIcon(":/seekleft"));
	m_recAction->setIcon(QIcon(":/record"));
	m_playAction->setIcon(QIcon(":/playstart"));
	m_toEndAction->setIcon(QIcon(":/skipright"));
	m_toRightAction->setIcon(QIcon(":/seekright"));

	connect(m_toStartAction, SIGNAL(triggered()), this, SLOT(to_start()));
	connect(m_toLeftAction, SIGNAL(triggered()), this, SLOT(to_left()));
	connect(m_recAction, SIGNAL(triggered()), this, SLOT(rec_toggled()));
	connect(m_playAction, SIGNAL(triggered()), this, SLOT(play_toggled()));
	connect(m_toEndAction, SIGNAL(triggered()), this, SLOT(to_end()));
	connect(m_toRightAction, SIGNAL(triggered()), this, SLOT(to_right()));

	buttonToStart->setDefaultAction(m_toStartAction);
	buttonToLeft->setDefaultAction(m_toLeftAction);
	buttonRec->setDefaultAction(m_recAction);
	buttonPlay->setDefaultAction(m_playAction);
	buttonToEnd->setDefaultAction(m_toEndAction);
	buttonToRight->setDefaultAction(m_toRightAction);

	m_layout->addWidget(m_label,    0, 0, 1, 6);
	m_layout->addWidget(buttonToStart, 1, 0, 1, 1);
	m_layout->addWidget(buttonToLeft,  1, 1, 1, 1);
	m_layout->addWidget(buttonRec,     1, 2, 1, 1);
	m_layout->addWidget(buttonPlay,    1, 3, 1, 1);
	m_layout->addWidget(buttonToRight, 1, 4, 1, 1);
	m_layout->addWidget(buttonToEnd,   1, 5, 1, 1);

	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(update_label()));
	place_label();
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

void TransportConsoleWidget::to_left()
{
	SnapList* slist = m_sheet->get_snap_list();
	TimeRef p = m_sheet->get_transport_location();
	TimeRef newpos = slist->prev_snap_pos(p);
	m_sheet->set_transport_pos(newpos);
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
	m_updateTimer.start(100);
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

void TransportConsoleWidget::resizeEvent(QResizeEvent * e)
{
	// scale the font in the text label
	QFont font = m_label->font();
	int fs = 0.6 * m_label->height();
	font.setPixelSize(fs);
	m_label->setFont(font);

	// position the text label depending on the widget size
	if ((e->oldSize().height() >= HEIGHT_THRESHOLD) && (e->size().height() < HEIGHT_THRESHOLD))
	{
		place_label();
	}

	if ((e->oldSize().height() < HEIGHT_THRESHOLD) && (e->size().height() >= HEIGHT_THRESHOLD))
	{
		place_label();
	}
}

void TransportConsoleWidget::place_label()
{
	if (height() < HEIGHT_THRESHOLD)
	{
		m_layout->removeWidget(m_label);
		m_layout->addWidget(m_label, 1, 6, 1, 1);
	} else {
		m_layout->removeWidget(m_label);
		m_layout->addWidget(m_label, 0, 0, 1, 6);
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
	m_label->setText(currentTime);
}

//eof

