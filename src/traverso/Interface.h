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

#ifndef INTERFACE_H
#define INTERFACE_H

#include <QMainWindow>
#include <QHash>

class Help;
class Song;
class Track;
class Project;
class BusMonitor;
class InfoBox;
class ViewPort;
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
class ResourcesWidget;
class ResourcesInfoWidget;
class DriverInfoWidget;
class HDDSpaceInfoWidget;
class SongWidget;
class CorrelationMeterWidget;
class SpectralMeterWidget;
class SettingsDialog;
class ProjectManagerDialog;
class OpenProjectDialog;
class InfoToolBar;
class SysInfoToolBar;
class CDTextDialog;
class MarkerDialog;
class BusSelectorDialog;
class NewSongDialog;
class NewTrackDialog;
class NewProjectDialog;

class Interface : public QMainWindow
{
	Q_OBJECT

public :
	Interface();
	~Interface();

	static Interface* instance();
	
	void select_fade_in_shape();
	void select_fade_out_shape();
	void show_busselector(Track* track);
	
protected:
	void keyPressEvent ( QKeyEvent* e);
	void keyReleaseEvent ( QKeyEvent* e);
	void closeEvent ( QCloseEvent * event );
	QSize sizeHint () const;

private:
	QStackedWidget* 	centerAreaWidget;
	QHash<Song*, SongWidget* > m_songWidgets;
	SongWidget*		currentSongWidget;
	QList<ViewPort* > 	currentProjectViewPortList;
	QHash<QString, QMenu*>	m_contextMenus;
	ExportWidget*		exportWidget;
	QUndoView*		historyWidget;
	QDockWidget* 		historyDW;
	QDockWidget*		busMonitorDW;
	QDockWidget*		AudioSourcesDW;
	ResourcesWidget* 		audiosourcesview;
	QDockWidget*		correlationMeterDW;
	CorrelationMeterWidget*	correlationMeter;
	QDockWidget*		spectralMeterDW;
	SpectralMeterWidget*	spectralMeter;
	SettingsDialog*		m_settingsdialog;
	ProjectManagerDialog*	m_projectManagerDialog;
	OpenProjectDialog*	m_openProjectDialog;
	CDTextDialog*		m_cdTextDialog;
	MarkerDialog*		m_markerDialog;
	InfoToolBar* 		m_infoBar;
	SysInfoToolBar* 	m_sysinfo;
	BusSelectorDialog*	m_busSelector;
	NewSongDialog*		m_newSongDialog;
	NewTrackDialog*		m_newTrackDialog;
	NewProjectDialog*	m_newProjectDialog;


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
	
	static Interface* m_instance;
	
	QMenu* create_context_menu(QObject* item);
	QMenu* create_fade_selector_menu(const QString& fadeTypeName);

public slots :
	void set_project(Project* project);
	void show_song(Song* song);
	void show_settings_dialog();
	void process_context_menu_action(QAction* action);
	void set_fade_in_shape(QAction* action);
	void set_fade_out_shape(QAction* action);
	void update_opengl();
	void import_audio();
	void insert_silence();

	Command* show_song_widget();
	Command* full_screen();
	Command* about_traverso();
	Command* show_export_widget();
	Command* show_context_menu();
	Command* show_open_project_dialog();
	Command* show_project_manager_dialog();
	Command* show_cdtext_dialog();
	Command* show_marker_dialog();
	Command* show_newsong_dialog();
	Command* show_newtrack_dialog();
	Command* show_newproject_dialog();
	
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
