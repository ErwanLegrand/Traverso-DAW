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

#include <QPixmap>
#include <QByteArray>
#include <QDesktopWidget>

#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const int INFOBAR_HEIGH_VER_ORIENTATION = 30;
static const int INFOBAR_HEIGH_HOR_ORIENTATION = 28;
static const int SYS_INFOBAR_HEIGHT_HOR_ORIENTATION = 24;


SystemResources::SystemResources(QWidget * parent)
	: InfoWidget(parent)
{
	
	m_writeBufferStatus = new SystemValueBar(this);
	m_readBufferStatus = new SystemValueBar(this);
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
	
	m_readBufferStatus->set_range(0, 100);
	m_readBufferStatus->add_range_color(0, 40, QColor(255, 0, 0));
	m_readBufferStatus->add_range_color(40, 60, QColor(255, 255, 0));
	m_readBufferStatus->add_range_color(60, 100, QColor(227, 254, 227));
	
	m_cpuUsage->set_range(0, 100);
	m_cpuUsage->set_int_rounding(false);
	m_cpuUsage->setMinimumWidth(90);
	m_cpuUsage->add_range_color(0, 60, QColor(227, 254, 227));
	m_cpuUsage->add_range_color(60, 75, QColor(255, 255, 0));
	m_cpuUsage->add_range_color(75, 100, QColor(255, 0, 0));
	
	m_readBufferStatus->set_text("R");
	m_writeBufferStatus->set_text("W");
	m_cpuUsage->set_text("CPU");
	
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
		foreach(Song* song, m_project->get_songs() ) {
			bufReadStatus = std::min(song->get_diskio()->get_read_buffers_fill_status(), bufReadStatus);
			bufWriteStatus = std::min(song->get_diskio()->get_write_buffers_fill_status(), bufWriteStatus);
	// 		time += song->get_diskio()->get_cpu_time();
		}
	}

	
	m_readBufferStatus->set_value(bufReadStatus);
	m_writeBufferStatus->set_value(bufWriteStatus);
	m_cpuUsage->set_value(time);
}


void SystemResources::set_orientation(Qt::Orientation orientation)
{
	QLayout* lay = layout();
	
	if (lay) delete lay;
	
	InfoWidget::set_orientation(orientation);
	
	if (m_orientation == Qt::Horizontal) {
		QHBoxLayout* lay = new QHBoxLayout(this);
		lay->addSpacing(6);
		lay->addWidget(m_icon);
		lay->addWidget(m_readBufferStatus);
		lay->addWidget(m_writeBufferStatus);
		lay->addWidget(m_cpuUsage);
		lay->setMargin(0);
		lay->addSpacing(6);
		setLayout(lay);
// 		setFrameStyle(QFrame::StyledPanel);
		setFrameStyle(QFrame::NoFrame);
	} else {
		QVBoxLayout* lay = new QVBoxLayout(this);
		lay->addWidget(m_readBufferStatus);
		lay->addWidget(m_writeBufferStatus);
		lay->addWidget(m_cpuUsage);
		lay->setMargin(0);
		setFrameStyle(QFrame::StyledPanel);
		setLayout(lay);
	}
}

QSize SystemResources::sizeHint() const
{
	if (m_orientation == Qt::Horizontal) {
		return QSize(250, SYS_INFOBAR_HEIGHT_HOR_ORIENTATION);
	}
	
	return QSize(100, SYS_INFOBAR_HEIGHT_HOR_ORIENTATION);
}




DriverInfo::DriverInfo( QWidget * parent )
	: InfoWidget(parent)
{
	m_driver = new QPushButton();
	m_driver->setIcon(find_pixmap(":/driver"));
	m_driver->setToolTip(tr("Click to configure audiodevice"));
	m_driver->setFlat(true);
	m_driver->setFocusPolicy(Qt::NoFocus);
	m_latency = new QLabel(this);
	m_xruns = new QLabel(this);
	m_rateBitdepth = new QLabel(this);
	
	driverConfigWidget = 0;
	
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
	QString latency = QString::number( ( (float)  (audiodevice().get_buffer_size() * 2) / audiodevice().get_sample_rate() ) * 1000, 'f', 2 ).append(" ms ");
	
	QByteArray xruns = "";
	if (xrunCount) {
		xruns = QByteArray::number(xrunCount).prepend(" xruns ");
	}
	
	m_driver->setText(audiodevice().get_driver_type());
	m_latency->setText(" @ " + latency);
	m_rateBitdepth->setText(QString::number(audiodevice().get_sample_rate()) + 
			"/" + 
			QString::number(audiodevice().get_bit_depth()));
	m_xruns->setText(xruns);
}

void DriverInfo::update_xrun_info( )
{
	set_orientation(Qt::Horizontal);
	xrunCount++;
	draw_information();
}

void DriverInfo::set_orientation(Qt::Orientation orientation)
{
	QLayout* lay = layout();
	
	if (lay) delete lay;
	
	InfoWidget::set_orientation(orientation);
	
	if (m_orientation == Qt::Horizontal) {
		QHBoxLayout* lay = new QHBoxLayout(this);
		
		lay->addWidget(m_driver);
		lay->addWidget(m_rateBitdepth);
		lay->addWidget(m_latency);
		if (xrunCount > 0) {
			lay->addWidget(m_xruns);
		}
		
		lay->setMargin(0);
		setLayout(lay);
// 		setFrameStyle(QFrame::StyledPanel);
		setFrameStyle(QFrame::NoFrame);
	} else {
		QVBoxLayout* lay = new QVBoxLayout(this);
		
		lay->addWidget(m_driver);
		lay->addWidget(m_rateBitdepth);
		lay->addWidget(m_latency);
		if (xrunCount > 0) {
			lay->addWidget(m_xruns);
		}
		
		lay->setMargin(0);
		setFrameStyle(QFrame::StyledPanel);
		setLayout(lay);
	}
}

QSize DriverInfo::sizeHint() const
{
	if (m_orientation == Qt::Horizontal) {
		return QSize(250, INFOBAR_HEIGH_HOR_ORIENTATION);
	}
	
	return QSize(90, INFOBAR_HEIGH_VER_ORIENTATION);
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


void HDDSpaceInfo::set_song(Song* song)
{
	m_song = song;
	
	if (! m_song) {
		updateTimer.start(20000);
		return;
	}
	
	update_status();
	
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(song_stopped()));
	connect(m_song, SIGNAL(transferStarted()), this, SLOT(song_started()));
}

void HDDSpaceInfo::song_started()
{
	updateTimer.start(3000);
	m_button->setEnabled(true);
}

void HDDSpaceInfo::song_stopped()
{
	updateTimer.start(20000);
	m_button->setEnabled(false);
}



void HDDSpaceInfo::update_status( )
{
#if HAVE_SYS_VFS_H
	Project* project = pm().get_project();

	if (!project) {
		m_button->setText("No Info");
		return;
	}
	
	struct statfs fs;
	statfs(project->get_root_dir().toAscii().data(), &fs);
	double space = floor (fs.f_bavail * (fs.f_bsize / 1048576.0));

	QString s;
	if (space > 9216) {
		s.setNum((space/1024), 'f', 2);
		s.append(" GB");
	} else {
		s.setNum(space, 'f', 0);
		s.append(" MB");
	}
	
	m_button->setText(s);

#endif
}

QSize HDDSpaceInfo::sizeHint() const
{
	if (m_orientation == Qt::Horizontal)
		return QSize(110, SYS_INFOBAR_HEIGHT_HOR_ORIENTATION);
	
	return QSize(100, SYS_INFOBAR_HEIGHT_HOR_ORIENTATION);
}



SongSelector::SongSelector(QWidget* parent)
	: InfoWidget(parent)
{
	setToolTip(tr("Select Song to be displayed"));
	setFrameStyle(QFrame::NoFrame);
	
	m_box = new QComboBox;
	
	QHBoxLayout* lay = new QHBoxLayout;
	lay->setMargin(0);
	lay->addWidget(m_box);
	setLayout(lay);
	
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(m_box, SIGNAL(activated(int)), this, SLOT(index_changed(int)));
}

void SongSelector::set_project(Project * project)
{
	if ( ! project) {
		m_project = project;
		m_box->clear();
		return;
	}
	
	m_project = project;
	
	connect(m_project, SIGNAL(songAdded(Song*)), this, SLOT(song_added(Song*)));
	connect(m_project, SIGNAL(songRemoved(Song*)), this, SLOT(song_removed(Song*)));
	connect(m_project, SIGNAL(currentSongChanged(Song*)), this, SLOT(change_index_to(Song*)));
	
	update_songs();
}

void SongSelector::update_songs()
{
	m_box->clear();
	foreach(Song* song, m_project->get_songs()) {
		m_box->addItem(QString::number(m_project->get_song_index(song->get_id())) +
			" " + song->get_title(),
		       song->get_id());
	}
}

void SongSelector::song_added(Song * song)
{
	connect(song, SIGNAL(propertieChanged()), this, SLOT(update_songs()));
	update_songs();
}

void SongSelector::song_removed(Song * song)
{
	disconnect(song, SIGNAL(propertieChanged()), this, SLOT(update_songs()));
	update_songs();
}

QSize SongSelector::sizeHint() const
{
	if (m_orientation == Qt::Horizontal)
		return QSize(140, INFOBAR_HEIGH_HOR_ORIENTATION);
	
	return QSize(100, INFOBAR_HEIGH_VER_ORIENTATION);
}

void SongSelector::index_changed(int index)
{
	qint64 id = m_box->itemData(index).toLongLong();
	
	m_project->set_current_song(id);
}

void SongSelector::change_index_to(Song* song)
{
	if (!song) {
		return;
	}
	
	int index = m_box->findData(song->get_id());
	
// 	Q_ASSERT(index != -1);
	
	m_box->setCurrentIndex(index);
}



PlayHeadInfo::PlayHeadInfo(QWidget* parent)
	: InfoWidget(parent)
{
	setAutoFillBackground(false);
	
	setToolTip(tr("Playhead position<br /><br />Click to change Playhead behavior"));
	
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
}


void PlayHeadInfo::set_project(Project* project )
{
	if (! project) {
		stop_smpte_update_timer();
	}
	
	InfoWidget::set_project(project);
}

void PlayHeadInfo::set_song(Song* song)
{
	m_song = song;
	
	if (!m_song) {
		stop_smpte_update_timer();
		return;
	}
	
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(stop_smpte_update_timer()));
	connect(m_song, SIGNAL(transferStarted()), this, SLOT(start_smpte_update_timer()));
	
	update();
}

void PlayHeadInfo::paintEvent(QPaintEvent* )
{
	QPainter painter(this);
	QString currentsmpte;
	
	if (!m_song) {
		currentsmpte = "0:00:0";
	} else {
		currentsmpte = frame_to_smpte(m_song->get_transport_frame(), m_song->get_rate());
	}
	
	int bgcolor = 60;
	int fc = 170;
	QColor background = QColor(bgcolor, bgcolor, bgcolor);
	QColor fontcolor = QColor(fc, fc, fc);
	
	if (m_song && m_song->is_transporting()) {
		background = QColor(40, 40, 40);
		fontcolor = QColor(bgcolor, bgcolor, bgcolor);
	}
	
	painter.setFont(QFont("Bitstream Vera Sans", 13));
	painter.setPen(fontcolor);
	
/*	painter.setBrush(background);
	painter.setRenderHints(QPainter::Antialiasing);
	painter.drawRoundRect(0, 0, width(), height(), 20);*/
	
	painter.drawText(QRect(0, 4, width() - 6, height() - 6), Qt::AlignCenter, currentsmpte);
}

void PlayHeadInfo::start_smpte_update_timer( )
{
	m_updateTimer.start(150);
}

void PlayHeadInfo::stop_smpte_update_timer( )
{
	m_updateTimer.stop();
	update();
}

QSize PlayHeadInfo::sizeHint() const
{
	if (m_orientation == Qt::Horizontal)
		return QSize(140, INFOBAR_HEIGH_HOR_ORIENTATION);
	
	return QSize(100, INFOBAR_HEIGH_VER_ORIENTATION);
}





InfoToolBar::InfoToolBar(QWidget * parent)
	: QToolBar(parent)
{
	setObjectName(tr("Main Toolbar"));
	
	driverInfo = new DriverInfo(this);
	m_widgets.append(driverInfo);
	
	m_songinfo = new SongInfo(this);
	m_widgets.append(m_songinfo);
	

	connect(this, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(orientation_changed(Qt::Orientation)));
	
	orientation_changed(orientation());
}


void InfoToolBar::orientation_changed(Qt::Orientation orientation)
{
	clear();
	
	QAction* action;
	if (orientation == Qt::Horizontal) {
		action = addWidget(driverInfo);
		action->setVisible(true);
		addSeparator();
		action = addWidget(m_songinfo);
		action->setVisible(true);
		addSeparator();
	} else {
		action = addWidget(m_songinfo);
		action->setVisible(true);
		action = addWidget(driverInfo);
		action->setVisible(true);
	}
	
	foreach(InfoWidget* widget, m_widgets) {
		widget->set_orientation(orientation);
	}
}



InfoWidget::InfoWidget(QWidget* parent)
	: QFrame(parent)
	, m_song(0)
	, m_project(0)
{
	QPalette pallet;
	pallet.setColor(QPalette::Background, QColor("#FFF4FF"));
	setPalette(pallet);
	setAutoFillBackground(true);
	
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	
	setFocusPolicy(Qt::NoFocus);
}


void InfoWidget::set_project(Project* project )
{
	if (project) {
		m_project = project;
		connect(m_project, SIGNAL(currentSongChanged(Song*)), this, SLOT(set_song(Song*)));
	} else {
		m_project = project;
		set_song(0);
	}
}

void InfoWidget::set_song(Song* song)
{
	m_song = song;
}

void InfoWidget::set_orientation(Qt::Orientation orientation)
{
	m_orientation = orientation;
}

SongInfo::SongInfo(QWidget * parent)
	: InfoWidget(parent)
{
	m_selector = new SongSelector(this);
	m_playhead = new PlayHeadInfo(this);
	m_snap = new QComboBox(this);
	m_snap->addItem("Snap: On");
	m_snap->addItem("Snap: Off");
	m_snap->setFocusPolicy(Qt::NoFocus);
	
	m_addNew = new QPushButton(tr("Add new..."), this);
	m_addNew->setFocusPolicy(Qt::NoFocus);
	m_addNew->setToolTip(tr("Create new Song or Track"));
	m_addNew->setMaximumHeight(22);
	
	m_record = new QPushButton(tr("Record"));
	m_record->setFocusPolicy(Qt::NoFocus);
	m_record->setEnabled(false);
	m_record->setIcon(find_pixmap(":/redled-16"));
	m_record->setMaximumHeight(22);
	
	QMenu* menu = new QMenu;
	menu->addAction("Track");
	menu->addAction("Song");
	menu->addAction("Song using template");
	m_addNew->setMenu(menu);
	
	connect(m_snap, SIGNAL(activated(int)), this, SLOT(snap_combo_index_changed(int)));
}

void SongInfo::set_orientation(Qt::Orientation orientation)
{
	QLayout* lay = layout();
	
	if (lay) delete lay;
	
	InfoWidget::set_orientation(orientation);
	
	if (m_orientation == Qt::Horizontal) {
		QHBoxLayout* lay = new QHBoxLayout(this);
		
		lay->addWidget(m_playhead);
		lay->addWidget(m_record);
		lay->addWidget(m_snap);
		lay->addWidget(m_selector);
		lay->addWidget(m_addNew);
		
		setLayout(lay);
		lay->setMargin(0);
		
		setFrameStyle(QFrame::NoFrame);
	} else {
		QVBoxLayout* lay = new QVBoxLayout(this);
		
		lay->addWidget(m_playhead);
		lay->addWidget(m_record);
		lay->addWidget(m_snap);
		lay->addWidget(m_addNew);
		lay->addWidget(m_selector);
		
		lay->setMargin(0);
		setFrameStyle(QFrame::StyledPanel);
		setLayout(lay);
	}
}

void SongInfo::set_song(Song* song)
{
	m_song = song;
	
	if (m_song) {
		connect(m_song, SIGNAL(snapChanged()), this, SLOT(song_snap_changed()));
		song_snap_changed();
	} else {
		m_snap->setEnabled(false);
	}
}

void SongInfo::song_snap_changed()
{
	if (m_song->is_snap_on()) {
		m_snap->setCurrentIndex(0);
	} else {
		m_snap->setCurrentIndex(1);
	}
}

void SongInfo::snap_combo_index_changed(int index)
{
	if (! m_song) {
		return;
	}
	
	if (index == 0) {
		m_song->set_snapping(true);
	} else {
		m_song->set_snapping(false);
	}
}

QSize SongInfo::sizeHint() const
{
	if (m_orientation == Qt::Horizontal) {
		return QSize(500, INFOBAR_HEIGH_HOR_ORIENTATION);
	}
	
	return QSize(90, INFOBAR_HEIGH_VER_ORIENTATION * 5 - 6);
	
}

SysInfoToolBar::SysInfoToolBar(QWidget * parent)
	: QToolBar(parent)
{
	setObjectName(tr("System Information"));
	message = new MessageWidget(this);
	resourcesInfo = new SystemResources(this);
	hddInfo = new HDDSpaceInfo(this);
	
	setMovable(false);
	
	connect(this, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(orientation_changed(Qt::Orientation)));
	
	orientation_changed(orientation());
}

void SysInfoToolBar::orientation_changed(Qt::Orientation orientation)
{
	clear();
	
	QAction* action;
	
	action = addWidget(message);
	action->setVisible(true);
	addSeparator();
	action = addWidget(resourcesInfo);
	resourcesInfo->set_orientation(orientation);
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
	painter.setFont(QFont("Bitstream Vera Sans", 8));
	
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
	return QSize(70, 25);
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

