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

class Song;
class Track;
class Project;
class BusMonitor;
class InfoBox;
class ViewPort;
class ContextItem;
class Command;

class QLabel;
class ExportDialog;
class CDWritingDialog;
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
class InsertSilenceDialog;
class MarkerDialog;
class BusSelectorDialog;
class NewSongDialog;
class NewTrackDialog;
class NewProjectDialog;
class Ui_QuickStartDialog;
class RestoreProjectBackupDialog;
struct MenuData;

class Interface : public QMainWindow
{
	Q_OBJECT
	Q_CLASSINFO("show_export_widget", tr("Show Export Dialog"))
	Q_CLASSINFO("show_context_menu", tr("Show Context Menu"))
	Q_CLASSINFO("about_traverso", tr("About Traverso"))
	Q_CLASSINFO("show_project_manager_dialog", tr("Show Project Management Dialog"))
	Q_CLASSINFO("full_screen", tr("Full Screen"))
	Q_CLASSINFO("export_keymap", tr("Export keymap"))

public :
	Interface();
	~Interface();

	static Interface* instance();
	
	void select_fade_in_shape();
	void select_fade_out_shape();
	void show_busselector(Track* track);
	void set_insertsilence_track(Track* track);
	
protected:
	void keyPressEvent ( QKeyEvent* e);
	void keyReleaseEvent ( QKeyEvent* e);
	void closeEvent ( QCloseEvent * event );
	QSize sizeHint () const;
	void changeEvent(QEvent *event);

private:
	QStackedWidget* 	centerAreaWidget;
	QHash<Song*, SongWidget* > m_songWidgets;
	SongWidget*		currentSongWidget;
	QList<ViewPort* > 	currentProjectViewPortList;
	QHash<QString, QMenu*>	m_contextMenus;
	ExportDialog*		m_exportDialog;
	CDWritingDialog*	m_cdWritingDialog;
	QUndoView*		historyWidget;
	QDockWidget* 		historyDW;
	QDockWidget*		busMonitorDW;
	QDockWidget*		AudioSourcesDW;
	ResourcesWidget* 	audiosourcesview;
	QDockWidget*		correlationMeterDW;
	CorrelationMeterWidget*	correlationMeter;
	QDockWidget*		spectralMeterDW;
	SpectralMeterWidget*	spectralMeter;
	SettingsDialog*		m_settingsdialog;
	ProjectManagerDialog*	m_projectManagerDialog;
	OpenProjectDialog*	m_openProjectDialog;
	InsertSilenceDialog*	m_insertSilenceDialog;
	MarkerDialog*		m_markerDialog;
	InfoToolBar* 		m_infoBar;
	SysInfoToolBar* 	m_sysinfo;
	BusSelectorDialog*	m_busSelector;
	NewSongDialog*		m_newSongDialog;
	NewTrackDialog*		m_newTrackDialog;
	NewProjectDialog*	m_newProjectDialog;
	QDialog*		m_quickStart;
	RestoreProjectBackupDialog* m_restoreProjectBackupDialog;


	BusMonitor* 		busMonitor;
	QToolBar* 		mainToolBar;
	QToolButton*		openGlButton;
	QAction*		m_projectSaveAction;
	QAction*		m_projectSongManagerAction;
	QAction*		m_projectExportAction;
	QAction*		m_songMenuAction;
	QMenu*			m_encodingMenu;
	QMenu*			m_resampleQualityMenu;
	
	ResourcesInfoWidget*	resourcesInfo;
	DriverInfoWidget*	driverInfo;
	HDDSpaceInfoWidget*	hddInfo;
	
	void create_menus();
	void save_config_and_emit_message(const QString& message);
	
	static Interface* m_instance;
	
	QMenu* create_context_menu(QObject* item, QList<MenuData >* list = 0);
	QMenu* create_fade_selector_menu(const QString& fadeTypeName);

public slots :
	void set_project(Project* project);
	void show_song(Song* song);
	void show_settings_dialog();
	void open_help_browser();
	void process_context_menu_action(QAction* action);
	void set_fade_in_shape(QAction* action);
	void set_fade_out_shape(QAction* action);
	void config_changed();
	void import_audio();
	void show_restore_project_backup_dialog();
	void change_recording_format_to_wav();
	void change_recording_format_to_wav64();
	void change_recording_format_to_wavpack();
	void change_resample_quality_to_best();
	void change_resample_quality_to_high();
	void change_resample_quality_to_medium();
	void change_resample_quality_to_fast();

	Command* full_screen();
	Command* about_traverso();
	Command* quick_start();
	Command* export_keymap();
	Command* show_export_widget();
	Command* show_cd_writing_dialog();
	Command* show_context_menu();
	Command* show_open_project_dialog();
	Command* show_project_manager_dialog();
	Command* show_restore_project_backup_dialog(QString projectdir);
	Command* show_insertsilence_dialog();
	Command* show_marker_dialog();
	Command* show_newsong_dialog();
	Command* show_newtrack_dialog();
	Command* show_newproject_dialog();
	
private slots:
	void delete_songwidget(Song*);
	void project_dir_change_detected();
	void project_load_failed(QString project, QString reason);
	void project_file_mismatch(QString rootdir, QString projectname);
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
