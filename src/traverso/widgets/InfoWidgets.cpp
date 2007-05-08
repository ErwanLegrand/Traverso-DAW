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

#if defined (WIN_BUILD)
#include <Windows.h>
#elif defined (OSX_BUILD)
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
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
	lay->addWidget(m_icon);
	lay->addWidget(m_readBufferStatus);
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
		foreach(Song* song, m_project->get_songs() ) {
			bufReadStatus = std::min(song->get_diskio()->get_read_buffers_fill_status(), bufReadStatus);
			bufWriteStatus = std::min(song->get_diskio()->get_write_buffers_fill_status(), bufWriteStatus);
			time += song->get_diskio()->get_cpu_time();
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
	m_driver->setToolTip(tr("Click to configure audiodevice"));
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
}

void DriverInfo::update_xrun_info( )
{
	xrunCount++;
	draw_information();
}

QSize DriverInfo::sizeHint() const
{
	return QSize(150, SONG_TOOLBAR_HEIGHT);
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
	updateTimer.start(5000);
	m_button->setEnabled(true);
	update_status();
}

void HDDSpaceInfo::song_stopped()
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
	
#if defined (WIN_BUILD)
	__int64 freebytestocaller, totalbytes, freebytes; 
	if (! GetDiskFreeSpaceEx ((const WCHAR*)(m_project->get_root_dir().toUtf8().data()),
					(PULARGE_INTEGER)&freebytestocaller,
					(PULARGE_INTEGER)&totalbytes,
					(PULARGE_INTEGER)&freebytes)) 
	{
// 		info().warning("HHDSpaceInfo: " + QString().sprintf("error: %lu", GetLastError()));
		m_button->setText("No Info");
		return;
	}
	
	double space =  double(freebytestocaller / (1 << 20));
#else
	struct statfs fs;
	statfs(m_project->get_root_dir().toUtf8().data(), &fs);
	double space = floor (fs.f_bavail * (fs.f_bsize / 1048576.0));
#endif

	QList<Song*> recordingSongs;
	foreach(Song* song, m_project->get_songs()) {
		if (song->is_recording() && song->any_track_armed()) {
			recordingSongs.append(song);
		}
	}
	
	QString text;
	
	if (recordingSongs.size()) {
		int recChannelCount = 0;
		foreach(Song* song, recordingSongs) {
			foreach(Track* track, song->get_tracks()) {
				if (track->armed()) {
					recChannelCount += track->capture_left_channel() ? 1 : 0;
					recChannelCount += track->capture_right_channel() ? 1 : 0;
				}
			}
		}
		
		uint rate = audiodevice().get_sample_rate();
		double frames = ( (space * 1048576) / (sizeof(float) * recChannelCount));
		text = frame_to_hms(frames, rate);
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
	return QSize(70, SONG_TOOLBAR_HEIGHT);
}



SongSelector::SongSelector(QWidget* parent)
	: InfoWidget(parent)
{
	setToolTip(tr("Select Song to be displayed"));
	setFrameStyle(QFrame::NoFrame);
	
	m_box = new QComboBox;
	m_box->setMinimumWidth(140);
	m_box->setFocusPolicy(Qt::NoFocus);
	
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
		m_box->addItem("Song " +
			QString::number(m_project->get_song_index(song->get_id())) +
			": " + song->get_title(),
		       song->get_id());
	}
}

void SongSelector::song_added(Song * song)
{
	connect(song, SIGNAL(propertyChanged()), this, SLOT(update_songs()));
	update_songs();
}

void SongSelector::song_removed(Song * song)
{
	disconnect(song, SIGNAL(propertyChanged()), this, SLOT(update_songs()));
	update_songs();
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
	m_box->setCurrentIndex(index);
}



PlayHeadInfo::PlayHeadInfo(QWidget* parent)
	: InfoWidget(parent)
{
	setAutoFillBackground(false);
	setToolTip(tr("Start/stop playback. You should use the SpaceBar! ;-)"));
	setMinimumWidth(110);
	create_background();
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
}


void PlayHeadInfo::set_project(Project* project )
{
	if (! project) {
		stop_song_update_timer();
	}
	
	InfoWidget::set_project(project);
}

void PlayHeadInfo::set_song(Song* song)
{
	m_song = song;
	
	if (!m_song) {
		stop_song_update_timer();
		m_playpixmap = find_pixmap(":/playstart");
		return;
	}
	
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(stop_song_update_timer()));
	connect(m_song, SIGNAL(transferStarted()), this, SLOT(start_song_update_timer()));
	connect(m_song, SIGNAL(transportPosSet()), this, SLOT(update()));
	
	
	if (m_song->is_transporting()) {
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
	
	if (!m_song) {
		currentTime = "0:00.0";
	} else {
		currentTime = frame_to_ms_2(m_song->get_transport_frame(), m_song->get_rate());
	}
	
	int fc = 170;
	QColor fontcolor = QColor(fc, fc, fc);
	
	if (m_song && m_song->is_transporting()) {
		fc = 60;
		fontcolor = QColor(fc, fc, fc);
	}
	
	painter.setFont(themer()->get_font("Playhead:fontscale:info"));
	painter.setPen(fontcolor);
	
	painter.drawPixmap(0, 0, m_background);
	painter.drawPixmap(8, (height() - m_playpixmap.height()) / 2, m_playpixmap);
	painter.drawText(QRect(12, 4, width() - 6, height() - 6), Qt::AlignCenter, currentTime);
}

void PlayHeadInfo::start_song_update_timer( )
{
	m_playpixmap = find_pixmap(":/playstop");
	m_updateTimer.start(150);
}

void PlayHeadInfo::stop_song_update_timer( )
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
	if (! m_song) {
		return;
	}
	
	if (event->button() == Qt::LeftButton) {
		m_song->go();
	}
}




InfoToolBar::InfoToolBar(QWidget * parent)
	: QToolBar(parent)
{
	setObjectName(tr("Main Toolbar"));
	
	setMovable(false);
	
	m_songinfo = new SongInfo(this);
	
	QAction* action = addWidget(m_songinfo);
	action->setVisible(true);
}


InfoWidget::InfoWidget(QWidget* parent)
	: QFrame(parent)
	, m_song(0)
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
		connect(m_project, SIGNAL(currentSongChanged(Song*)), this, SLOT(set_song(Song*)));
	} else {
		set_song(0);
	}
}

void InfoWidget::set_song(Song* song)
{
	m_song = song;
}


SongInfo::SongInfo(QWidget * parent)
	: InfoWidget(parent)
{
	m_selector = new SongSelector(this);
	m_playhead = new PlayHeadInfo(this);

	m_snap = new QToolButton(this);
	m_snapAct = new QAction(tr("&Snap"), this);
	m_snapAct->setCheckable(true);
	m_snapAct->setToolTip(tr("Snap items to edges of other items while dragging."));
	m_snap->setDefaultAction(m_snapAct);
	m_snap->setFocusPolicy(Qt::NoFocus);

	m_follow = new QToolButton(this);
	m_followAct = new QAction(tr("S&croll Playback"), this);
	m_followAct->setCheckable(true);
	m_followAct->setToolTip(tr("Keep play cursor in view while playing or recording."));
	m_follow->setDefaultAction(m_followAct);
	m_follow->setFocusPolicy(Qt::NoFocus);
	
	m_mode = new QComboBox(this);
	m_mode->addItem("Mode: Edit");
	m_mode->addItem("Mode: Effects");
	m_mode->setFocusPolicy(Qt::NoFocus);
	
	m_record = new QToolButton(this);
	m_recAction = new QAction(tr("Record"), this);
	m_recAction->setCheckable(true);
	m_recAction->setToolTip(tr("Toggle recording state on/off"));
	m_record->setDefaultAction(m_recAction);
	m_record->setFocusPolicy(Qt::NoFocus);
	m_record->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	
	
	QToolButton* undobutton = new QToolButton(this);
	QAction* action = new QAction(tr("Undo"), this);
	action->setIcon(QIcon(find_pixmap(":/undo-16")));
	action->setShortcuts(QKeySequence::Undo);
	undobutton->setDefaultAction(action);
	undobutton->setFocusPolicy(Qt::NoFocus);
	undobutton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	undobutton->setText(tr("Undo"));
	connect(action, SIGNAL(triggered( bool )), &pm(), SLOT(undo()));
	
	QToolButton* redobutton = new QToolButton(this);
	action = new QAction(tr("Redo"), this);
	action->setIcon(QIcon(find_pixmap(":/redo-16")));
	action->setShortcuts(QKeySequence::Redo);
	redobutton->setDefaultAction(action);
	redobutton->setFocusPolicy(Qt::NoFocus);
	redobutton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	redobutton->setText(tr("Redo"));
	connect(action, SIGNAL(triggered( bool )), &pm(), SLOT(redo()));

	QHBoxLayout* lay = new QHBoxLayout(this);
		
	lay->addWidget(undobutton);
	lay->addWidget(redobutton);
	lay->addStretch(1);
	lay->addWidget(m_snap);
	lay->addWidget(m_follow);
	lay->addWidget(m_record);
	lay->addStretch(1);
	lay->addWidget(m_playhead);
	lay->addStretch(5);
	lay->addWidget(m_mode);
	lay->addWidget(m_selector);
		
	setLayout(lay);
	lay->setMargin(0);
		
	setFrameStyle(QFrame::NoFrame);
	setMaximumHeight(SONG_TOOLBAR_HEIGHT);
	
	connect(m_snapAct, SIGNAL(triggered(bool)), this, SLOT(snap_state_changed(bool)));
	connect(m_followAct, SIGNAL(triggered(bool)), this, SLOT(follow_state_changed(bool)));
	connect(&config(), SIGNAL(configChanged()), this, SLOT(update_follow_state()));
	connect(m_mode, SIGNAL(currentIndexChanged(int)), this, SLOT(mode_index_changed(int)));
	connect(m_recAction, SIGNAL(triggered(bool)), this, SLOT(recording_button_state_changed(bool)));
	
	update_follow_state();
}

void SongInfo::set_song(Song* song)
{
	m_song = song;
	
	if (m_song) {
		connect(m_song, SIGNAL(snapChanged()), this, SLOT(update_snap_state()));
		connect(m_song, SIGNAL(modeChanged()), this, SLOT(update_mode_state()));
		connect(m_song, SIGNAL(recordingStateChanged()), this, SLOT(update_recording_state()));
		update_snap_state();
		update_mode_state();
		update_recording_state();
		m_snapAct->setEnabled(true);
		m_mode->setEnabled(true);
		m_record->setEnabled(true);
		m_follow->setEnabled(true);
	} else {
		m_snapAct->setEnabled(false);
		m_mode->setEnabled(false);
		m_record->setEnabled(false);
		m_follow->setEnabled(false);
	}
}

void SongInfo::update_snap_state()
{
	m_snapAct->setChecked(m_song->is_snap_on());
}

void SongInfo::update_mode_state()
{
	if (m_song->get_mode() == Song::EDIT) {
		m_mode->setCurrentIndex(0);
	} else {
		m_mode->setCurrentIndex(1);
	}
}

void SongInfo::snap_state_changed(bool state)
{
	if (! m_song) {
		return;
	}
	m_song->set_snapping(state);
}

void SongInfo::update_follow_state()
{
	m_followAct->setChecked(config().get_property("PlayHead", "Follow", true).toBool());
}

void SongInfo::follow_state_changed(bool state)
{
	config().set_property("PlayHead", "Follow", state);
	config().save();
}

void SongInfo::mode_index_changed(int index)
{
	if (index == 0) {
		m_song->set_editing_mode();
	} else {
		m_song->set_effects_mode();
	}
}

void SongInfo::recording_button_state_changed(bool state)
{
	m_song->set_recording(state);
	if (state) {
		m_recAction->setIcon(find_pixmap(":/redled-16"));
	} else {
		m_recAction->setIcon(find_pixmap(":/redledinactive-16"));
	}
}

void SongInfo::update_recording_state()
{
	if (m_song->is_recording()) {
		m_recAction->setChecked(true);
		m_recAction->setIcon(find_pixmap(":/redled-16"));
	} else {
		m_recAction->setChecked(false);
		m_recAction->setIcon(find_pixmap(":/redledinactive-16"));
	}
}


QSize SongInfo::sizeHint() const
{
	return QSize(400, SONG_TOOLBAR_HEIGHT);
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

