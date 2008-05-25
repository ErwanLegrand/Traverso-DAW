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

#ifndef RESOURCES_INFO_WIDGET_H
#define RESOURCES_INFO_WIDGET_H

#include <QToolBar>
#include <QTimer>
#include <QFrame>
#include <QProgressBar>

class Project;
class Sheet;
class MessageWidget;
class SystemValueBar;

class QPushButton;


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
	
	void draw_information();

private slots:
        void update_driver_info();
        void update_xrun_info();
	void show_driver_config_dialog();
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

class ProgressToolBar : public QToolBar
{
	Q_OBJECT

public:
	ProgressToolBar(QWidget* parent);
	~ProgressToolBar();

public slots:
	void set_progress(int);
	void set_label(QString);
	void set_num_files(int);

private:
	QProgressBar*	m_progressBar;
	int		filecount;
	int		filenum;
};

#endif

//eof

