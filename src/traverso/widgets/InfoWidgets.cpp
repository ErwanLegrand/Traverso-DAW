/*
Copyright (C) 2005-2007 Remon Sijrier 

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

#include "libtraversocore.h"
#include "Themer.h"
#include "DiskIO.h"
#include <AudioDevice.h>
#include <Utils.h>
#include "QuickDriverConfigWidget.h"
#include "MessageWidget.h" 
#include <Interface.h>

#include <QPixmap>
#include <QByteArray>
#include <QDesktopWidget>
#include <QPalette>

#if defined (Q_WS_WIN)
#include <Windows.h>
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
	m_cpuUsage->set_text("CPU");
	
	QHBoxLayout* lay = new QHBoxLayout(this);
	lay->addSpacing(6);
	lay->addWidget(m_readBufferStatus);
	lay->addWidget(m_icon);
	lay->addWidget(m_writeBufferStatus);
	lay->addWidget(m_cpuUsage);
	lay->setMargin(0);
	lay->addSpacing(6);
	setLayout(lay);
	setFrameStyle(QFrame::NoFrame);
	
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(update_status()));
	
	update_status();
	
	m_updateTimer.start(1200);
}

void SystemResources::update_status( )
{
	float time = audiodevice().get_cpu_time();
	int bufReadStatus = 100;
	int bufWriteStatus = 100;
	
	if (m_project) {
		foreach(Sheet* sheet, m_project->get_sheets() ) {
			bufReadStatus = std::min(sheet->get_diskio()->get_read_buffers_fill_status(), bufReadStatus);
			bufWriteStatus = std::min(sheet->get_diskio()->get_write_buffers_fill_status(), bufWriteStatus);
			time += sheet->get_diskio()->get_cpu_time();
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



DriverInfo::DriverInfo( QWidget * parent )
	: InfoWidget(parent)
{
	m_driver = new QPushButton();
	m_driver->setIcon(find_pixmap(":/driver"));
	m_driver->setToolTip(tr("Change Audio Device settings"));
	m_driver->setFlat(true);
	m_driver->setFocusPolicy(Qt::NoFocus);
	
	driverConfigWidget = 0;
	
	QHBoxLayout* lay = new QHBoxLayout(this);
	lay->addWidget(m_driver);
	lay->setMargin(0);
	setLayout(lay);
	
	setFrameStyle(QFrame::NoFrame);
	
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(update_driver_info()));
	connect(&audiodevice(), SIGNAL(bufferUnderRun()), this, SLOT(update_xrun_info()));
	connect(m_driver, SIGNAL(clicked( bool )), this, SLOT(show_driver_config_widget()));
	
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
	
	text = audiodevice().get_driver_type() + "   " +
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


void DriverInfo::show_driver_config_widget( )
{
	if (! driverConfigWidget) {
		driverConfigWidget = new QuickDriverConfigWidget(m_driver);
	}
	
	
	QRect rect = QApplication::desktop()->screenGeometry();
	QPoint pos = QCursor::pos();
	
	if ( (pos.y() + driverConfigWidget->height() + 30) > rect.height()) {
		pos.setY(pos.y() - driverConfigWidget->height());
	}
	
	if ( (pos.x() + driverConfigWidget->width() + 20) > rect.width()) {
		pos.setX(pos.x() - driverConfigWidget->width());
	}
	
	driverConfigWidget->move(pos);
	driverConfigWidget->show();
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


void HDDSpaceInfo::set_sheet(Sheet* sheet)
{
	m_sheet = sheet;
	
	if (! m_sheet) {
		updateTimer.start(20000);
		return;
	}
	
	update_status();
	
	connect(m_sheet, SIGNAL(transferStopped()), this, SLOT(sheet_stopped()));
	connect(m_sheet, SIGNAL(transferStarted()), this, SLOT(sheet_started()));
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
	if (! GetDiskFreeSpaceEx ((const WCHAR*)(QS_C(m_project->get_root_dir())),
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
		if (sheet->is_recording() && sheet->any_track_armed()) {
			recordingSheets.append(sheet);
		}
	}
	
	QString text;
	
	if (recordingSheets.size()) {
		int recChannelCount = 0;
		foreach(Sheet* sheet, recordingSheets) {
			foreach(Track* track, sheet->get_tracks()) {
				if (track->armed()) {
					recChannelCount += track->capture_left_channel() ? 1 : 0;
					recChannelCount += track->capture_right_channel() ? 1 : 0;
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




PlayHeadInfo::PlayHeadInfo(QWidget* parent)
	: InfoWidget(parent)
{
	setAutoFillBackground(false);
	setToolTip(tr("Start/stop playback. You should use the SpaceBar! ;-)"));
	setMinimumWidth(160);
	create_background();
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
}


void PlayHeadInfo::set_project(Project* project )
{
	if (! project) {
		stop_sheet_update_timer();
	}
	
	InfoWidget::set_project(project);
}

void PlayHeadInfo::set_sheet(Sheet* sheet)
{
	m_sheet = sheet;
	
	if (!m_sheet) {
		stop_sheet_update_timer();
		m_playpixmap = find_pixmap(":/playstart");
		return;
	}
	
	connect(m_sheet, SIGNAL(transferStopped()), this, SLOT(stop_sheet_update_timer()));
	connect(m_sheet, SIGNAL(transferStarted()), this, SLOT(start_sheet_update_timer()));
	connect(m_sheet, SIGNAL(transportPosSet()), this, SLOT(update()));
	
	
	if (m_sheet->is_transport_rolling()) {
		m_playpixmap = find_pixmap(":/playstop");
	} else {
		m_playpixmap = find_pixmap(":/playstart");
	}
	
	update();
}

void PlayHeadInfo::paintEvent(QPaintEvent* )
{
	QPainter painter(this);
	QString currentTime;
	
	if (!m_sheet) {
		currentTime = "0:00.0";
	} else {
		currentTime = timeref_to_ms_2(m_sheet->get_transport_location());
	}
	
	int fc = 170;
	QColor fontcolor = QColor(fc, fc, fc);
	
	if (m_sheet && m_sheet->is_transport_rolling()) {
		fc = 60;
		fontcolor = QColor(fc, fc, fc);
	}
	
	painter.setFont(themer()->get_font("Playhead:fontscale:info"));
	painter.setPen(fontcolor);
	
	painter.drawPixmap(0, 0, m_background);
	painter.drawPixmap(8, (height() - m_playpixmap.height()) / 2, m_playpixmap);
	painter.drawText(QRect(30, 4, width(), height() - 6), Qt::AlignCenter, currentTime);
}

void PlayHeadInfo::start_sheet_update_timer( )
{
	m_playpixmap = find_pixmap(":/playstop");
	m_updateTimer.start(150);
}

void PlayHeadInfo::stop_sheet_update_timer( )
{
	m_updateTimer.stop();
	m_playpixmap = find_pixmap(":/playstart");
	update();
}

QSize PlayHeadInfo::sizeHint() const
{
	return QSize(120, SONG_TOOLBAR_HEIGHT);
}

void PlayHeadInfo::resizeEvent(QResizeEvent * e)
{
	Q_UNUSED(e);
	create_background();
}


void PlayHeadInfo::create_background()
{
	m_background = QPixmap(size());
	QPainter painter(&m_background);
	painter.setRenderHints(QPainter::Antialiasing);
	m_background.fill(palette().background().color());
	int round = 12;
	painter.setPen(QColor(50, 50, 50));
	painter.setBrush(QColor(250, 251, 255));
	painter.drawRoundRect(m_background.rect(), round, round);
}

void PlayHeadInfo::mousePressEvent(QMouseEvent * event)
{
	if (! m_sheet) {
		return;
	}
	
	if (event->button() == Qt::LeftButton) {
		m_sheet->start_transport();
	}
}




InfoWidget::InfoWidget(QWidget* parent)
	: QFrame(parent)
	, m_sheet(0)
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
		connect(m_project, SIGNAL(currentSheetChanged(Sheet*)), this, SLOT(set_sheet(Sheet*)));
	} else {
		set_sheet(0);
	}
}

void InfoWidget::set_sheet(Sheet* sheet)
{
	m_sheet = sheet;
}



InfoToolBar::InfoToolBar(QWidget * parent)
	: QToolBar(parent)
{
	setObjectName(tr("Main Toolbar"));
	
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(&config(), SIGNAL(configChanged()), this, SLOT(update_follow_state()));

	m_sheetselectbox = new QComboBox(this);
	m_sheetselectbox->setMinimumWidth(140);
	m_sheetselectbox->setToolTip(tr("Select Sheet to be displayed"));
	connect(m_sheetselectbox, SIGNAL(activated(int)), this, SLOT(sheet_selector_index_changed(int)));

	m_playhead = new PlayHeadInfo(this);

	m_recAct = new QAction(tr("Record"), this);
	m_recAct->setToolTip(tr("Set Sheet Recordable. <br /><br />Hit Spacebar afterwards to start recording!"));
	connect(m_recAct, SIGNAL(triggered(bool)), this, SLOT(recording_action_clicked()));
	
	// the order in which the actions are added determines the order of appearance in the toolbar
	addAction(m_recAct);
	addWidget(m_playhead);
	addWidget(m_sheetselectbox);
}


void InfoToolBar::set_project(Project * project)
{
	m_project = project;
	
	if (!project) {
		m_sheetselectbox->clear();
		set_sheet(0);
		return;
	}
	
	connect(m_project, SIGNAL(sheetAdded(Sheet*)), this, SLOT(sheet_selector_sheet_added(Sheet*)));
	connect(m_project, SIGNAL(sheetRemoved(Sheet*)), this, SLOT(sheet_selector_sheet_removed(Sheet*)));
	connect(m_project, SIGNAL(currentSheetChanged(Sheet*)), this, SLOT(sheet_selector_change_index_to(Sheet*)));
	connect(m_project, SIGNAL(currentSheetChanged(Sheet*)), this, SLOT(set_sheet(Sheet*)));
	connect(m_project, SIGNAL(projectLoadFinished()), this, SLOT(project_load_finished()));
	
	sheet_selector_update_sheets();
}

void InfoToolBar::project_load_finished()
{
	sheet_selector_change_index_to(m_project->get_current_sheet());
}

void InfoToolBar::set_sheet(Sheet* sheet)
{
	m_sheet = sheet;
	
	if (m_sheet) {
		connect(m_sheet, SIGNAL(recordingStateChanged()), this, SLOT(update_recording_state()));
		m_recAct->setEnabled(true);
	} else {
		m_recAct->setEnabled(false);
	}
}



void InfoToolBar::recording_action_clicked()
{
	m_sheet->set_recordable();
}

void InfoToolBar::update_recording_state()
{
	if (m_sheet->is_recording()) {
		m_recAct->setIcon(find_pixmap(":/redled-16"));
		QString recordFormat = config().get_property("Recording", "FileFormat", "wav").toString();
		int count = 0;
		foreach(Track* track, m_sheet->get_tracks()) {
			if (track->armed()) {
				count++;
			}
		}
		info().information(tr("Recording to %1 Tracks, encoding format: %2").arg(count).arg(recordFormat));
	} else {
		m_recAct->setIcon(find_pixmap(":/redledinactive-16"));
	}
}


void InfoToolBar::sheet_selector_update_sheets()
{
	m_sheetselectbox->clear();
	foreach(Sheet* sheet, m_project->get_sheets()) {
		m_sheetselectbox->addItem("Sheet " +
		QString::number(m_project->get_sheet_index(sheet->get_id())) +
		": " + sheet->get_title(),
		sheet->get_id());
	}

	if (m_project->get_current_sheet()) {
		int i = m_project->get_sheet_index((m_project->get_current_sheet())->get_id()) - 1;
		m_sheetselectbox->setCurrentIndex(i);
	}
}

void InfoToolBar::sheet_selector_sheet_added(Sheet * sheet)
{
	connect(sheet, SIGNAL(propertyChanged()), this, SLOT(sheet_selector_update_sheets()));
	sheet_selector_update_sheets();
}

void InfoToolBar::sheet_selector_sheet_removed(Sheet * sheet)
{
	disconnect(sheet, SIGNAL(propertyChanged()), this, SLOT(sheet_selector_update_sheets()));
	sheet_selector_update_sheets();
}

void InfoToolBar::sheet_selector_index_changed(int index)
{
	qint64 id = m_sheetselectbox->itemData(index).toLongLong();
	
	m_project->set_current_sheet(id);
}

void InfoToolBar::sheet_selector_change_index_to(Sheet* sheet)
{
	if (!sheet) {
		return;
	}
	
	int index = m_sheetselectbox->findData(sheet->get_id());
	if (index >= 0) {
		m_sheetselectbox->setCurrentIndex(index);
	}
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
	action = addWidget(message);
	action->setVisible(true);
	addSeparator();
	action = addWidget(resourcesInfo);
	action->setVisible(true);
	addSeparator();
	action = addWidget(hddInfo);
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

//eof
