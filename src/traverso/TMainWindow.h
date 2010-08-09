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

#ifndef TMAIN_WINDOW_H
#define TMAIN_WINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QModelIndex>
#include <QTimer>
#include <QGridLayout>

class Sheet;
class TSession;
class AudioTrack;
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
class QCompleter;
class QTreeView;
class QLineEdit;
class QStandardItemModel;

class ResourcesWidget;
class ResourcesInfoWidget;
class SheetWidget;
class CorrelationMeterWidget;
class SpectralMeterWidget;
class TransportConsoleWidget;
class SettingsDialog;
class ProjectManagerDialog;
class OpenProjectDialog;
class InfoToolBar;
class SysInfoToolBar;
class InsertSilenceDialog;
class MarkerDialog;
class NewSheetDialog;
class NewTrackDialog;
class NewProjectDialog;
class Ui_QuickStartDialog;
class RestoreProjectBackupDialog;
class ProgressToolBar;
class WelcomeWidget;
class TSessionTabWidget;
struct MenuData;

class TTabWidget : public QTabWidget
{
public:
        TTabWidget(QWidget* parent) : QTabWidget(parent) {}

        QTabBar* get_tab_bar() {return tabBar();}
};

class AbstractVUMeterLevel
{
public:
        AbstractVUMeterLevel() {}
        virtual ~AbstractVUMeterLevel() {}

        virtual void update_peak() = 0;
        virtual void reset_peak_hold_value() = 0;
};

class TMainWindow : public QMainWindow
{
	Q_OBJECT
	Q_CLASSINFO("show_export_widget", tr("Show Export Dialog"))
	Q_CLASSINFO("show_context_menu", tr("Show Context Menu"))
	Q_CLASSINFO("about_traverso", tr("About Traverso"))
	Q_CLASSINFO("show_project_manager_dialog", tr("Show Project Management Dialog"))
	Q_CLASSINFO("full_screen", tr("Full Screen"))
	Q_CLASSINFO("export_keymap", tr("Export keymap"))
	Q_CLASSINFO("start_transport", tr("Play"))
        Q_CLASSINFO("set_recordable_and_start_transport", tr("Record"));
        Q_CLASSINFO("sheet_audio_io_dialog", tr("Show Audio I/O dialog"));
        Q_CLASSINFO("show_track_finder", tr("Activate Track Finder"));
        Q_CLASSINFO("audio_io_dialog", tr("Show Audio I/O Dialog"));
        Q_CLASSINFO("browse_to_first_track_in_active_sheet", tr("Browse to first Track in current View"))
        Q_CLASSINFO("browse_to_last_track_in_active_sheet", tr("Browse to last Track in current View"))

public :
        TMainWindow();
        ~TMainWindow();

        static TMainWindow* instance();
	
	void select_fade_in_shape();
	void select_fade_out_shape();
	void set_insertsilence_track(AudioTrack* track);
	
        void register_vumeter_level(AbstractVUMeterLevel* level);
        void unregister_vumeter_level(AbstractVUMeterLevel* level);
        int get_vulevel_update_frequency() const {return m_vuLevelUpdateFrequency;}

        QLineEdit* get_track_finder() const {return m_trackFinder;}

protected:
        void timerEvent(QTimerEvent *event);
	void keyPressEvent ( QKeyEvent* e);
	void keyReleaseEvent ( QKeyEvent* e);
	void closeEvent ( QCloseEvent * event );
	QSize sizeHint () const;
	void changeEvent(QEvent *event);
	bool eventFilter(QObject *obj, QEvent *ev);

private:
        QGridLayout*            m_mainLayout;
        QStackedWidget*         m_centerAreaWidget;
        int                     m_previousCenterAreaWidgetIndex;
        int                     m_vuLevelUpdateFrequency;
        QHash<TSession*, SheetWidget* > m_sheetWidgets;
        QHash<TSession*, TSessionTabWidget* > m_sessionTabWidgets;
        SheetWidget*		m_currentSheetWidget;
	QHash<QString, QMenu*>	m_contextMenus;
        QHash<QString, QString> m_contextMenuDict;
	ExportDialog*		m_exportDialog;
	CDWritingDialog*	m_cdWritingDialog;
        QUndoView*		m_historyWidget;
        QDockWidget* 		m_historyDW;
        QDockWidget*		m_busMonitorDW;
        QDockWidget*		m_audioSourcesDW;
        ResourcesWidget* 	m_audiosourcesview;
        QDockWidget*		m_correlationMeterDW;
        CorrelationMeterWidget*	m_correlationMeter;
        TransportConsoleWidget*	m_transportConsole;
        QDockWidget*		m_spectralMeterDW;
        SpectralMeterWidget*	m_spectralMeter;
	SettingsDialog*		m_settingsdialog;
	ProjectManagerDialog*	m_projectManagerDialog;
	OpenProjectDialog*	m_openProjectDialog;
	InsertSilenceDialog*	m_insertSilenceDialog;
	SysInfoToolBar* 	m_sysinfo;
	ProgressToolBar*	m_progressBar;
	NewSheetDialog*		m_newSheetDialog;
	NewTrackDialog*		m_newTrackDialog;
	NewProjectDialog*	m_newProjectDialog;
        WelcomeWidget*          m_welcomeWidget;
	QDialog*		m_quickStart;
	RestoreProjectBackupDialog* m_restoreProjectBackupDialog;
	Project*		m_project;
	bool			m_isFollowing;

	BusMonitor* 		busMonitor;
        QToolBar*               m_mainMenuToolBar;
        QMenuBar*               m_mainMenuBar;
	QToolBar*		m_projectToolBar;
	QToolBar*		m_editToolBar;
        QToolBar*               m_sessionTabsToolbar;
	QAction*		m_snapAction;
	QAction*		m_followAction;
	QAction*		m_effectAction;
	QMenu*			m_encodingMenu;
	QMenu*			m_resampleQualityMenu;
        QList<QAction*>         m_projectMenuToolbarActions;
        QLineEdit*              m_trackFinder;
        QCompleter*             m_trackFinderCompleter;
        QStandardItemModel*     m_trackFinderModel;
        QTreeView*              m_trackFinderTreeView;

        QList<AbstractVUMeterLevel*> m_vuLevels;
        QBasicTimer                  m_vuLevelUpdateTimer;
        QBasicTimer                  m_vuLevelPeakholdTimer;

	
	void create_menus();
        void set_project_actions_enabled(bool enable);
	void save_config_and_emit_message(const QString& message);
        void track_finder_show_initial_text();

        static TMainWindow* m_instance;
	
	QMenu* create_context_menu(QObject* item, QList<MenuData >* list = 0);
	QMenu* create_fade_selector_menu(const QString& fadeTypeName);

        void update_vu_levels_peak();
        void reset_vu_levels_peak_hold_value();


public slots :
	void set_project(Project* project);
        void show_session(TSession* sheet);
	void show_settings_dialog();
	void show_settings_dialog_sound_system_page();
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
        Command* show_fft_meter_only();
	Command* about_traverso();
	Command* quick_start();
	Command* export_keymap();
	Command* get_keymap(QString &);
	Command* show_export_widget();
	Command* show_cd_writing_dialog();
	Command* show_context_menu();
	Command* show_open_project_dialog();
	Command* show_project_manager_dialog();
	Command* show_restore_project_backup_dialog(QString projectdir);
	Command* show_insertsilence_dialog();
	Command* show_marker_dialog();
	Command* show_newsheet_dialog();
	Command* show_newtrack_dialog();
	Command* show_newproject_dialog();
        Command* show_add_child_session_dialog();
        Command* show_welcome_page();
        Command* show_current_sheet();
        Command* start_transport();
	Command* set_recordable_and_start_transport();
        Command* show_track_finder();
        Command* browse_to_first_track_in_active_sheet();
        Command* browse_to_last_track_in_active_sheet();
	
private slots:
        void add_sheetwidget(Sheet* session);
        void remove_sheetwidget(Sheet*);
        void add_session(TSession* session);
        void remove_session(TSession*);
        void project_dir_change_detected();
	void project_load_failed(QString project, QString reason);
        void project_load_finished();
	void project_file_mismatch(QString rootdir, QString projectname);
	void snap_state_changed(bool state);
	void update_snap_state();
	void effect_state_changed(bool state);
	void update_effects_state();
	void follow_state_changed(bool state);
	void update_follow_state();
	void update_temp_follow_state(bool state);
        void track_finder_model_index_changed(const QModelIndex& index);
        void track_finder_return_pressed();
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
