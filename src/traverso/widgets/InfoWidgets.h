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

#ifndef RESOURCES_INFO_WIDGET_H
#define RESOURCES_INFO_WIDGET_H

#include <QPushButton>
#include <QComboBox>
#include <QAction>
#include <QToolBar>
#include <QToolButton>
#include <QTimer>
#include <QLabel>
#include <QFrame>
#include <QProgressBar>
#include <QMenu>

class Project;
class Sheet;
class QuickDriverConfigWidget;
class MessageWidget;
class SystemValueBar;


class InfoWidget : public QFrame
{
	Q_OBJECT

public:
	InfoWidget(QWidget* parent = 0);

protected:
	Sheet*		m_sheet;
	Project*	m_project;
	
	virtual QSize sizeHint() const {return QSize(size());}

private:
	friend class InfoToolBar;

protected slots:
	virtual void set_sheet(Sheet* );
	virtual void set_project(Project* );
};



class SystemResources : public InfoWidget
{
        Q_OBJECT

public:
        SystemResources(QWidget* parent = 0);

protected:
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
	QSize sizeHint () const;
	void enterEvent ( QEvent * event );
	void leaveEvent ( QEvent * event );

private:
        QTimer		updateTimer;
	QPushButton*	m_driver;
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
	void set_sheet(Sheet* );

private slots:
	void update_status();
	void sheet_started();
	void sheet_stopped();
};


class PlayHeadInfo : public InfoWidget
{
	Q_OBJECT

public:
	PlayHeadInfo(QWidget* parent = 0);
	~PlayHeadInfo() {};

protected:
	void mousePressEvent ( QMouseEvent * event );
	void paintEvent( QPaintEvent* e);
	void resizeEvent( QResizeEvent * e );
	QSize sizeHint() const;	
	
private:
	QTimer m_updateTimer;
	QPixmap m_playpixmap;
	QPixmap m_background;
	
	void create_background();

protected slots:
	void set_project(Project* );
	void set_sheet(Sheet* );
	
private slots:
	void start_sheet_update_timer();
	void stop_sheet_update_timer();
};



class SheetInfo : public InfoWidget
{
	Q_OBJECT
public:
	SheetInfo(QWidget* parent);
	QAction *get_snap_action() {return m_snapAct;};
	QAction *get_follow_action() {return m_followAct;};
	
protected:
	QSize sizeHint() const;	
	
protected slots:
	void set_project(Project* project);
	void set_sheet(Sheet* );
	
private slots:
	void update_snap_state();
	void snap_state_changed(bool state);
	void update_follow_state();
	void update_temp_follow_state(bool state);
	void update_effects_state();
	void follow_state_changed(bool state);
	void effect_button_clicked();
	void recording_button_clicked();
	void update_recording_state();
	void sheet_selector_sheet_added(Sheet* sheet);
	void sheet_selector_sheet_removed(Sheet* sheet);
	void sheet_selector_update_sheets();
	void sheet_selector_change_index_to(Sheet* sheet);
	void sheet_selector_index_changed(int index);
	void project_load_finished();
	
private:
	PlayHeadInfo* 	m_playhead;
	QToolButton*	m_snap;
	QToolButton*	m_effectButton;
	QComboBox* 	m_sheetselectbox;
	QAction*	m_snapAct;
	QToolButton*	m_follow;
	QAction*	m_followAct;
	QAction*	m_recAction;
	QToolButton*	m_record;
	bool		m_isFollowing;
};


class InfoToolBar : public QToolBar
{
public:
	InfoToolBar(QWidget* parent);
	QAction *get_snap_action() {return m_sheetinfo->get_snap_action();};
	QAction *get_follow_action() {return m_sheetinfo->get_follow_action();};

private:
	SheetInfo*	m_sheetinfo;
};


class SysInfoToolBar : public QToolBar
{
public:
	SysInfoToolBar(QWidget* parent);

private:
	SystemResources* resourcesInfo;
	HDDSpaceInfo* hddInfo;
	MessageWidget* message;
	DriverInfo* driverInfo;
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

