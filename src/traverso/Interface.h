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

$Id: Interface.h,v 1.17 2007/02/28 17:25:07 r_sijrier Exp $
*/

#ifndef INTERFACE_H
#define INTERFACE_H

#include <QMainWindow>
#include <QHash>

class Help;
class Song;
class Project;
class BusMonitor;
class InfoBox;
class ViewPort;
class OverViewWidget;
class ContextItem;
class Command;

class QLabel;
class ExportWidget;
class QStackedWidget;
class QHBoxLayout;
class QVBoxLayout;
class QUndoView;
class QDockWidget;
class QToolBar;
class QToolButton;
class QTreeView;
class QuickDriverConfigWidget;
class ResourcesInfoWidget;
class DriverInfoWidget;
class HDDSpaceInfoWidget;
class SongWidget;
class CorrelationMeterWidget;
class SpectralMeterWidget;
class SettingsDialog;
class ProjectManagerDialog;
class SongManagerDialog;

class Interface : public QMainWindow
{
	Q_OBJECT

public :
	Interface();
	~Interface();

protected:
	void keyPressEvent ( QKeyEvent* e);
	void keyReleaseEvent ( QKeyEvent* e);
	void closeEvent ( QCloseEvent * event );
// 	void wheelEvent ( QWheelEvent* e );
	

private:
	QStackedWidget* 	centerAreaWidget;
	QHash<Song*, SongWidget* > m_songWidgets;
	SongWidget*		currentSongWidget;
	QList<ViewPort* > 	currentProjectViewPortList;
	QHash<QString, QMenu*>	m_contextMenus;
	ExportWidget*		exportWidget;
	OverViewWidget* 	overView;
	QUndoView*		historyWidget;
	QDockWidget* 		historyDW;
	QDockWidget*		busMonitorDW;
	QDockWidget*		AudioSourcesDW;
	QTreeView* 		audiosourcesview;
	QuickDriverConfigWidget* driverConfigWidget;
	QDockWidget*		correlationMeterDW;
	CorrelationMeterWidget*	correlationMeter;
	QDockWidget*		spectralMeterDW;
	SpectralMeterWidget*	spectralMeter;
	SettingsDialog*		m_settingsdialog;
	ProjectManagerDialog*	m_projectManagerDialog;
	SongManagerDialog*	m_songManagerDialog;

	BusMonitor* 		busMonitor;
	Help* 			helpWindow;

	QToolBar* 		mainToolBar;
	QToolButton*		openGlButton;
	QAction*		m_projectSaveAction;
	QAction*		m_projectSongManagerAction;
	QAction*		m_projectExportAction;
	
	ResourcesInfoWidget*	resourcesInfo;
	DriverInfoWidget*	driverInfo;
	HDDSpaceInfoWidget*	hddInfo;
	
	void create_menus();
	
	QMenu* create_context_menu(QObject* item);
	QMenu* create_fade_selector_menu(const QString& fadeTypeName);

public slots :
	void set_project(Project* project);
	void show_song(Song* song);
	void show_settings_dialog();
	void process_context_menu_action(QAction* action);
	void set_bus_in(QAction* action);
	void set_bus_out(QAction* action);
	void set_fade_in_shape(QAction* action);
	void set_fade_out_shape(QAction* action);
	void show_driver_config_widget();
	void toggle_OpenGL(bool);
	void show_project_manager_dialog();
	void show_song_manager_dialog();

	Command* show_song_widget();
	Command* full_screen();
	Command* about_traverso();
	Command* show_export_widget();
	Command* show_context_menu();
	Command* select_bus_in();
	Command* select_bus_out();
	Command* select_fade_in_shape();
	Command* select_fade_out_shape();
	
private slots:
	void delete_songwidget(Song*);
};


#include <QLCDNumber>

class DigitalClock : public QLCDNumber
{
	Q_OBJECT

	public:
		DigitalClock(QWidget *parent = 0);

	private slots:
		void showTime();
};

#endif

// eof
