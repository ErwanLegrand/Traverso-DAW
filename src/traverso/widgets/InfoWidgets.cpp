/*
Copyright (C) 2005-2008 Remon Sijrier 

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

#include "InfoWidgets.h"

#include "AudioDevice.h"
#include "TConfig.h"
#include "DiskIO.h"
#include "TMainWindow.h"
#include "InputEngine.h"
#include "MessageWidget.h" 
#include "Project.h"
#include "ProjectManager.h"
#include "Sheet.h"
#include "Themer.h"
#include "AudioTrack.h"
#include "Utils.h"
#include "Mixer.h"

#include <QPainter>
#include <QLineEdit>
#include <QByteArray>
#include <QPalette>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QAction>


#if defined (Q_WS_WIN)
#include <windows.h>
#elif defined (Q_WS_MAC)
#include <sys/param.h>
#include <sys/mount.h>
#else
#if defined(HAVE_SYS_VFS_H)
#include <sys/vfs.h>
#endif
#endif


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

#if defined (Q_WS_MAC)
static const int SONG_TOOLBAR_HEIGHT = 27;
#else
static const int SONG_TOOLBAR_HEIGHT = 24;
#endif


SystemResources::SystemResources(QWidget * parent)
	: InfoWidget(parent)
{
	
	m_writeBufferStatus = new SystemValueBar(this);
	m_readBufferStatus = new SystemValueBar(this);
	m_readBufferStatus->setToolTip(tr("Read Buffer Status"));
	m_writeBufferStatus->setToolTip(tr("Write Buffer Status"));
	m_cpuUsage = new SystemValueBar(this);
	m_icon = new QPushButton();
	m_icon->setIcon(find_pixmap(":/memorysmall"));
	m_icon->setFlat(true);
	m_icon->setMaximumWidth(20);
	m_icon->setFocusPolicy(Qt::NoFocus);

        m_collectedNumber = new QLabel(this);
        m_collectedNumber->setFocusPolicy(Qt::NoFocus);
        collected_number_changed();
	
	m_writeBufferStatus->set_range(0, 100);
	m_writeBufferStatus->add_range_color(0, 40, QColor(255, 0, 0));
	m_writeBufferStatus->add_range_color(40, 60, QColor(255, 255, 0));
	m_writeBufferStatus->add_range_color(60, 100, QColor(227, 254, 227));
	m_writeBufferStatus->setMinimumWidth(60);
	
	m_readBufferStatus->set_range(0, 100);
	m_readBufferStatus->add_range_color(0, 40, QColor(255, 0, 0));
	m_readBufferStatus->add_range_color(40, 60, QColor(255, 255, 0));
	m_readBufferStatus->add_range_color(60, 100, QColor(227, 254, 227));
	m_readBufferStatus->setMinimumWidth(60);
	
	m_cpuUsage->set_range(0, 100);
	m_cpuUsage->set_int_rounding(false);
	m_cpuUsage->setMinimumWidth(90);
	m_cpuUsage->add_range_color(0, 60, QColor(227, 254, 227));
	m_cpuUsage->add_range_color(60, 75, QColor(255, 255, 0));
	m_cpuUsage->add_range_color(75, 100, QColor(255, 0, 0));
	
        m_readBufferStatus->set_text("R");
	m_writeBufferStatus->set_text("W");
        m_cpuUsage->set_text("DSP");
	
        QHBoxLayout* lay = new QHBoxLayout(this);
	lay->addSpacing(6);
	lay->addWidget(m_readBufferStatus);
	lay->addWidget(m_icon);
	lay->addWidget(m_writeBufferStatus);
	lay->addWidget(m_cpuUsage);
        lay->addWidget(TMainWindow::instance()->get_track_finder());
        lay->addWidget(m_collectedNumber);
	lay->setMargin(0);
	lay->addSpacing(6);
	setLayout(lay);
	setFrameStyle(QFrame::NoFrame);
	
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(update_status()));
	
	update_status();
	
        m_updateTimer.start(700);

        connect(&ie(), SIGNAL(collectedNumberChanged()), this, SLOT(collected_number_changed()));
}

void SystemResources::update_status( )
{
	float time = audiodevice().get_cpu_time();
        float ioCPUTime = 0;
	int bufReadStatus = 100;
	int bufWriteStatus = 100;
	
	if (m_project) {
		foreach(Sheet* sheet, m_project->get_sheets() ) {
			bufReadStatus = std::min(sheet->get_diskio()->get_read_buffers_fill_status(), bufReadStatus);
			bufWriteStatus = std::min(sheet->get_diskio()->get_write_buffers_fill_status(), bufWriteStatus);
                        ioCPUTime += sheet->get_diskio()->get_cpu_time();
		}
	}

	
	m_readBufferStatus->set_value(bufReadStatus);
	m_writeBufferStatus->set_value(bufWriteStatus);
	m_cpuUsage->set_value(time);
}


QSize SystemResources::sizeHint() const
{
	return QSize(250, SONG_TOOLBAR_HEIGHT);
}

void SystemResources::collected_number_changed()
{
        QString number = ie().get_collected_number();
        if (!number.isEmpty() || !number.isNull()) {
                m_collectedNumber->setText(tr("Num. Input: ") + number);
        }
}



DriverInfo::DriverInfo( QWidget * parent )
	: InfoWidget(parent)
{
	m_driver = new QPushButton();
	m_driver->setIcon(find_pixmap(":/driver"));
	m_driver->setToolTip(tr("Change Audio Device settings"));
	m_driver->setFlat(true);
	m_driver->setFocusPolicy(Qt::NoFocus);
	
        QHBoxLayout* lay = new QHBoxLayout(this);
	lay->addWidget(m_driver);
	lay->setMargin(0);
	setLayout(lay);
	
	setFrameStyle(QFrame::NoFrame);
	
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(update_driver_info()));
	connect(&audiodevice(), SIGNAL(bufferUnderRun()), this, SLOT(update_xrun_info()));
	connect(m_driver, SIGNAL(clicked( bool )), this, SLOT(show_driver_config_dialog()));
	
	update_driver_info();
}

void DriverInfo::update_driver_info( )
{
	xrunCount = 0;
	draw_information();
}

void DriverInfo::draw_information( )
{
	QString text;
	QString latency = QString::number( ( (float)  (audiodevice().get_buffer_size() * 2) / audiodevice().get_sample_rate() ) * 1000, 'f', 2 ).append(" ms ");
	
	QByteArray xruns;
	if (xrunCount) {
		xruns = QByteArray::number(xrunCount).prepend(" xruns ");
	}
	
        text = audiodevice().get_driver_information() + "   " +
			QString::number(audiodevice().get_sample_rate()) + 
			"/" + 
			QString::number(audiodevice().get_bit_depth()) +
			" @ " + latency +
			xruns;
	
	m_driver->setText(text);
	updateGeometry();
}

void DriverInfo::update_xrun_info( )
{
	xrunCount++;
	draw_information();
}

QSize DriverInfo::sizeHint() const
{
	return QSize(m_driver->width(), SONG_TOOLBAR_HEIGHT);
}

void DriverInfo::enterEvent(QEvent * event)
{
	m_driver->setFlat(false);
}

void DriverInfo::leaveEvent(QEvent * event)
{
	m_driver->setFlat(true);
}


void DriverInfo::show_driver_config_dialog( )
{
        TMainWindow::instance()->show_settings_dialog_sound_system_page();
}



HDDSpaceInfo::HDDSpaceInfo(QWidget* parent )
	: InfoWidget(parent)
{
	m_button = new QPushButton;
	m_button->setIcon(find_pixmap(":/harddrivesmall"));
	m_button->setFlat(true);
	m_button->setFocusPolicy(Qt::NoFocus);
	m_button->setEnabled(false);
	
        QHBoxLayout* lay = new QHBoxLayout;
	lay->setMargin(0);
	lay->addWidget(m_button);
	setLayout(lay);
	
	setFrameStyle(QFrame::NoFrame);
	
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update_status()));
	
	update_status();
	updateTimer.start(20000);
}


void HDDSpaceInfo::set_session(TSession* session)
{
        m_session = session;
	
        if (! m_session) {
		updateTimer.start(20000);
		return;
	}
	
	update_status();
	
        connect(m_session, SIGNAL(transportStopped()), this, SLOT(sheet_stopped()));
        connect(m_session, SIGNAL(transportStarted()), this, SLOT(sheet_started()));
}

void HDDSpaceInfo::sheet_started()
{
	updateTimer.start(5000);
	m_button->setEnabled(true);
	update_status();
}

void HDDSpaceInfo::sheet_stopped()
{
	updateTimer.start(60000);
	m_button->setEnabled(false);
	update_status();
}



void HDDSpaceInfo::update_status( )
{
	if (!m_project) {
		m_button->setText("No Info");
		return;
	}
	
	double space = 0.0;
	
#if defined (Q_WS_WIN)
	__int64 freebytestocaller, totalbytes, freebytes; 
	if (! GetDiskFreeSpaceEx ((const CHAR*)(QS_C(m_project->get_root_dir())),
					(PULARGE_INTEGER)&freebytestocaller,
					(PULARGE_INTEGER)&totalbytes,
					(PULARGE_INTEGER)&freebytes)) 
	{
// 		info().warning("HHDSpaceInfo: " + QString().sprintf("error: %lu", GetLastError()));
		m_button->setText("No Info");
		return;
	}
	
	space =  double(freebytestocaller / (1 << 20));
#else

#if !defined(HAVE_SYS_VFS_H)
	m_button->setText("No Info");
	return;
#else
	
	struct statfs fs;
	statfs(QS_C(m_project->get_root_dir()), &fs);
	space = floor (fs.f_bavail * (fs.f_bsize / 1048576.0));
#endif
#endif

	QList<Sheet*> recordingSheets;
	foreach(Sheet* sheet, m_project->get_sheets()) {
		if (sheet->is_recording() && sheet->any_audio_track_armed()) {
			recordingSheets.append(sheet);
		}
	}
	
	QString text;
	
	if (recordingSheets.size()) {
		int recChannelCount = 0;
		foreach(Sheet* sheet, recordingSheets) {
                        foreach(AudioTrack* track, sheet->get_audio_tracks()) {
				if (track->armed()) {
                                        // FIXME !!!!!!!
                                        recChannelCount = 2;
//					recChannelCount += track->capture_left_channel() ? 1 : 0;
//					recChannelCount += track->capture_right_channel() ? 1 : 0;
				}
			}
		}
		
		uint rate = audiodevice().get_sample_rate();
		double availabletime = (double(UNIVERSAL_SAMPLE_RATE) / rate) * space * 1048576.0;
		availabletime /= double(sizeof(float) * recChannelCount);
 		
		QString recordFormat = config().get_property("Recording", "FileFormat", "wav").toString();
		// I think a compression ratio of 40 % with wavpack is a safe estimation
		// and 50% with skipwvx...
		if (recordFormat == "wavpack") {
			QString skipwvx = config().get_property("Recording", "WavpackSkipWVX", "false").toString();
			if (skipwvx == "true") {
				availabletime = qint64(availabletime / 0.5);
			} else {
				availabletime = qint64(availabletime / 0.6);
			}
		}
		
		TimeRef time(availabletime);
		text = timeref_to_hms(time);
		if (text < "00:30:00") {
			QPalette pal;
			pal.setColor(QPalette::ButtonText, QColor(Qt::red));
			m_button->setPalette(pal);
		}
	} else {
		if (space > 9216) {
			text.setNum((space/1024), 'f', 2);
			text.append(" GB");
		} else {
			text.setNum(space, 'f', 0);
			text.append(" MB");
		}
	}
	
	m_button->setText(text);
}

QSize HDDSpaceInfo::sizeHint() const
{
	return QSize(90, SONG_TOOLBAR_HEIGHT);
}






InfoWidget::InfoWidget(QWidget* parent)
	: QFrame(parent)
        , m_session(0)
	, m_project(0)
{
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	
	setFocusPolicy(Qt::NoFocus);
}


void InfoWidget::set_project(Project* project )
{
	m_project = project;
	if (m_project) {
                connect(m_project, SIGNAL(currentSessionChanged(TSession*)), this, SLOT(set_session(TSession*)));
	} else {
                set_session(0);
	}
}

void InfoWidget::set_session(TSession* session)
{
        m_session = session;
}


SysInfoToolBar::SysInfoToolBar(QWidget * parent)
	: QToolBar(parent)
{
	setObjectName(tr("System Information"));
        message = new MessageWidget(this);
	resourcesInfo = new SystemResources(this);
	hddInfo = new HDDSpaceInfo(this);
	driverInfo = new DriverInfo(this);
	
        setMovable(false);
	
	QAction* action;
	
	action = addWidget(driverInfo);
	action->setVisible(true);
	addSeparator();
	action = addWidget(resourcesInfo);
	action->setVisible(true);
	addSeparator();
	action = addWidget(hddInfo);
	action->setVisible(true);
	addSeparator();
        action = addWidget(message);
        action->setVisible(true);
}

SystemValueBar::SystemValueBar(QWidget * parent)
	: QWidget(parent)
{
	m_current = m_min = m_max = 0;
	m_text = "";
	m_introunding = true;
}

void SystemValueBar::set_value(float value)
{
	if (m_current == value) {
		return;
	}
	
	m_current = value;
	
        if (m_current > m_max) {
                m_current  = m_max;
        }
	
        if (m_current < m_min) {
                m_current = m_min;
        }
	
	update();
}

void SystemValueBar::set_range(float min, float max)
{
	m_min = min;
	m_max = max;
	update();
}

void SystemValueBar::set_text(const QString & text)
{
	m_text = text;
}

void SystemValueBar::paintEvent(QPaintEvent* )
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing);
	
	QColor color = QColor(227, 254, 227);
	
	for (int i=0; i<m_rangecolors.size(); ++i) {
		RangeColor range = m_rangecolors.at(i);
		if (m_current <= range.x1 && m_current >= range.x0) {
			color = range.color;
			break;
		}
	}
	
	QRect rect = QRect(0, (height() - 15) / 2, width(), 15);
	painter.drawRect(rect);
	
	painter.setBrush(color);
	painter.setPen(Qt::NoPen);
	float scalefactor = width() / m_max;
	rect = QRect(1, (height() - 15) / 2 + 1, width() - 2 - (int)(scalefactor* (m_max - m_current)), 13);
	painter.drawRect(rect);
	
	painter.setPen(Qt::black);
	painter.setFont(themer()->get_font("InfoWidget:fontscale:values"));
	
	if (m_introunding) {
		painter.drawText(0, 0, width(), height(), Qt::AlignCenter, 
				m_text + " " + QString::number((int)m_current).append("%"));
	} else {
		painter.drawText(0, 0, width(), height(), Qt::AlignCenter, 
				 m_text + " " + QString::number(m_current, 'f', 2).append("%"));
	}
}

QSize SystemValueBar::sizeHint() const
{
	return QSize(60, 25);
}

void SystemValueBar::set_int_rounding(bool rounding)
{
	m_introunding = rounding;
}

void SystemValueBar::add_range_color(float x0, float x1, QColor color)
{
	RangeColor range;
	range.x0 = x0;
	range.x1 = x1;
	range.color = color;
	m_rangecolors.append(range);
}

ProgressToolBar::ProgressToolBar(QWidget* parent)
	: QToolBar(tr("Progress Toolbar"), parent)
{
	m_progressBar = new QProgressBar(this);
	m_progressBar->setMinimumWidth(800);
	addWidget(m_progressBar);
	m_progressBar->setEnabled(false);
	filecount = 1;
	filenum = 1;

	QString style = "QProgressBar {border: 2px solid grey;border-radius: 5px; height: 10px; width 300px; text-align: center;}" 
"QProgressBar::chunk {background-color: qlineargradient(x1: 0, y1: 0, x2: 1.0, y2: 1.0,stop: 0 white, stop: 1 navy);}";

        m_progressBar->setStyleSheet(style);
}

ProgressToolBar::~ProgressToolBar()
{
}

void ProgressToolBar::set_progress(int i)
{
	if (i == m_progressBar->maximum()) {
		if (filenum == filecount) {
			hide();
			m_progressBar->reset();
			m_progressBar->setEnabled(false);
			return;
		} else {
			++filenum;
		}
	}

	if (!m_progressBar->isEnabled()) {
		m_progressBar->setEnabled(true);
		show();
	}

	m_progressBar->setValue(i);
}

void ProgressToolBar::set_label(QString s)
{
	Q_UNUSED(s);
	m_progressBar->setFormat(tr("Importing file %1 of %2: %p%").arg(filenum).arg(filecount));
}

void ProgressToolBar::set_num_files(int i)
{
	filecount = i;
	filenum = 1;
}

//eof
