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
 
*/

#ifndef RESOURCES_INFO_WIDGET_H
#define RESOURCES_INFO_WIDGET_H

#include <QPushButton>
#include <QComboBox>
#include <QToolBar>
#include <QTimer>
#include <QLabel>
#include <QFrame>
#include <QProgressBar>
#include <QMenu>

class Project;
class Song;
class QuickDriverConfigWidget;
class MessageWidget;
class SystemValueBar;


class InfoWidget : public QFrame
{
	Q_OBJECT

public:
	InfoWidget(QWidget* parent = 0);

protected:
	Song*		m_song;
	Project*	m_project;
	Qt::Orientation	m_orientation;
	
	virtual QSize sizeHint() const {return QSize(size());}
	virtual void set_orientation(Qt::Orientation orientation);

private:
	friend class InfoToolBar;

protected slots:
	void set_song(Song* );
	void set_project(Project* );
};



class SystemResources : public InfoWidget
{
        Q_OBJECT

public:
        SystemResources(QWidget* parent = 0);

protected:
	void set_orientation(Qt::Orientation orientation);
	QSize sizeHint () const;
	
private:
	QTimer	m_updateTimer;
	SystemValueBar*	m_readBufferStatus;
	SystemValueBar*	m_writeBufferStatus;
	SystemValueBar*	m_cpuUsage;
	QPushButton*	m_icon;

	friend class SysInfoToolBar;
	
private slots:
        void update_status();
};



class DriverInfo : public InfoWidget
{
        Q_OBJECT

public:
        DriverInfo(QWidget* parent = 0);
        ~DriverInfo() {};

protected:
	void set_orientation(Qt::Orientation orientation);
	QSize sizeHint () const;

private:
        QTimer		updateTimer;
	QPushButton*	m_driver;
	QLabel*		m_rateBitdepth;
	QLabel* 	m_latency;
	QLabel*		m_xruns;
        int		xrunCount;
	QuickDriverConfigWidget* driverConfigWidget;
	
	void draw_information();

private slots:
        void update_driver_info();
        void update_xrun_info();
	void show_driver_config_widget();
};


class HDDSpaceInfo : public InfoWidget
{
	Q_OBJECT
public:
	HDDSpaceInfo(QWidget* parent  = 0);
	~HDDSpaceInfo(){};

protected:
	QSize sizeHint() const;	
	
private:
	QTimer	updateTimer;
	QPushButton* m_button;

private slots:
	void set_song(Song* );

private slots:
	void update_status();
	void song_started();
	void song_stopped();
};


class SongSelector : public InfoWidget
{
	Q_OBJECT

public:
	SongSelector(QWidget* parent=0);

protected:
	QSize sizeHint() const;	

private:
	QComboBox* m_box;

protected slots:
	void set_project(Project* project);
	
private slots:
	void update_songs();
	void change_index_to(Song* song);
	void index_changed(int index);
		
};


class PlayHeadInfo : public InfoWidget
{
	Q_OBJECT

public:
	PlayHeadInfo(QWidget* parent = 0);
	~PlayHeadInfo() {};

protected:
	void paintEvent( QPaintEvent* e);
	QSize sizeHint() const;	
	
private:
	QTimer m_updateTimer;

protected slots:
	void set_project(Project* );
	void set_song(Song* );
	
private slots:
	void start_smpte_update_timer();
	void stop_smpte_update_timer();
};



class SongInfo : public InfoWidget
{
public:
	SongInfo(QWidget* parent);
	
protected:
	void set_orientation(Qt::Orientation orientation);
	QSize sizeHint() const;	
	
private:
	PlayHeadInfo* 	m_playhead;
	SongSelector* 	m_selector;
	QComboBox*	m_snap;
	QPushButton*	m_addNew;
	QPushButton*	m_record;
};


class InfoToolBar : public QToolBar
{
	Q_OBJECT

public:
	InfoToolBar(QWidget* parent);

private:
	QList<InfoWidget* > m_widgets;
	DriverInfo* driverInfo;
	SongInfo*	m_songinfo;
	
private slots:
	void orientation_changed(Qt::Orientation orientation);
};


class SysInfoToolBar : public QToolBar
{
	Q_OBJECT

public:
	SysInfoToolBar(QWidget* parent);

private:
	SystemResources* resourcesInfo;
	HDDSpaceInfo* hddInfo;
	MessageWidget* message;
	
private slots:
	void orientation_changed(Qt::Orientation orientation);
};


class SystemValueBar : public QWidget
{
	Q_OBJECT
			
public: 
	SystemValueBar(QWidget* parent);

	void set_value(float value);
	void set_range(float min, float max);
	void set_text(const QString& text);
	void set_int_rounding(bool rounding);
	void add_range_color(float x0, float x1, QColor color);

protected:
	void paintEvent( QPaintEvent* e);
	QSize sizeHint () const;
	

private:
	struct RangeColor {
		float x0;
		float x1;
		QColor color;
	};
		
	QList<RangeColor> m_rangecolors;
	QString m_text;
	float m_min;
	float m_max;
	float m_current;
	bool m_introunding;
};

#endif

//eof





