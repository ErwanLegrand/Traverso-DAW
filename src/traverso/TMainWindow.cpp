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

#include "../config.h"

#include <libtraversocore.h>
#include "libtraversosheetcanvas.h"
#include "commands.h"

#include "AudioChannel.h"
#include <AudioDevice.h>

#include <QDockWidget>
#include <QUndoView>
#include <QFile>
#include <QDir>
#include <QMenuBar>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTextStream>
#include <QFileDialog>
#include <QFileInfo>
#include <QTabBar>
#include <QCompleter>
#include <QStandardItemModel>

#include "TMainWindow.h"
#include "ProjectManager.h"
#include "ViewPort.h"
#include "FadeCurve.h"
#include "Config.h"
#include "Plugin.h"
#include "Import.h"
#include "TimeLine.h"
#include "AudioFileCopyConvert.h"

#include "../sheetcanvas/SheetWidget.h"

#include "ui_QuickStart.h"

#include "widgets/BusMonitor.h"
#include "widgets/InfoWidgets.h"
#include "widgets/ResourcesWidget.h"
#include "widgets/CorrelationMeterWidget.h"
#include "widgets/SpectralMeterWidget.h"
#include "widgets/TransportConsoleWidget.h"
#include "widgets/WelcomeWidget.h"
#include "widgets/TSessionTabWidget.h"

#include "dialogs/settings/SettingsDialog.h"
#include "dialogs/project/ProjectManagerDialog.h"
#include "dialogs/project/OpenProjectDialog.h"
#include "dialogs/project/NewProjectDialog.h"
#include "dialogs/project/NewSheetDialog.h"
#include "dialogs/project/NewTrackDialog.h"
#include "dialogs/MarkerDialog.h"
#include "dialogs/InsertSilenceDialog.h"
#include "dialogs/RestoreProjectBackupDialog.h"
#include "dialogs/ProjectConverterDialog.h"
#include "dialogs/ExportDialog.h"
#include "dialogs/CDWritingDialog.h"
#include "dialogs/project/ImportClipsDialog.h"
#include "dialogs/TTrackSelector.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


class HistoryWidget : public QUndoView
{
public:
	HistoryWidget(QUndoGroup* group, QWidget* parent)
		: QUndoView(group, parent) 
	{
	}
	
protected:
	QSize sizeHint() const {
		return QSize(120, 140);
	}
	QSize minimumSizeHint() const
	{
		return QSize(90, 90);
	}
};


TMainWindow* TMainWindow::m_instance = 0;

TMainWindow* TMainWindow::instance()
{
	if (m_instance == 0) {
                m_instance = new TMainWindow();
	}

	return m_instance;
}

TMainWindow::TMainWindow()
        : QMainWindow()
{
	PENTERCONS;

        m_instance = this;

	setWindowTitle("Traverso");
	setMinimumSize(400, 300);
	setWindowIcon(QPixmap (":/windowicon") );

        // Track finder and related
        m_trackFinder = new QLineEdit(this);
        m_trackFinder->setMinimumWidth(100);
        m_trackFinder->installEventFilter(this);
        m_trackFinderCompleter = new QCompleter;
        m_trackFinder->setCompleter(m_trackFinderCompleter);
        m_trackFinderModel = new QStandardItemModel();
        m_trackFinderCompleter->setModel(m_trackFinderModel);
        m_trackFinderCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_trackFinder->setCompleter(m_trackFinderCompleter);
        connect(m_trackFinderCompleter, SIGNAL(activated(const QModelIndex&)),
                this, SLOT(track_finder_model_index_changed(const QModelIndex&)));
        connect(m_trackFinder, SIGNAL(returnPressed()), this, SLOT(track_finder_return_pressed()));

        m_trackFinderTreeView = new QTreeView;
        m_trackFinderTreeView->setMinimumWidth(250);
        m_trackFinderCompleter->setPopup(m_trackFinderTreeView);
        m_trackFinderTreeView->setRootIsDecorated(false);
        m_trackFinderTreeView->header()->hide();
        m_trackFinderTreeView->header()->setStretchLastSection(false);

        // CenterAreaWidget
//        QWidget* mainWidget = new QWidget(this);
//        m_mainLayout = new QGridLayout(this);
//        mainWidget->setLayout(m_mainLayout);
//        setCentralWidget(mainWidget);
        m_centerAreaWidget = new QStackedWidget(this);
//        m_mainLayout->addWidget(m_centerAreaWidget, 0, 0);
        setCentralWidget(m_centerAreaWidget);

	// HistoryView 
        m_historyDW = new QDockWidget(tr("History"), this);
        m_historyDW->setObjectName("HistoryDockWidget");
        m_historyWidget = new HistoryWidget(pm().get_undogroup(), m_historyDW);
        m_historyWidget->setFocusPolicy(Qt::NoFocus);
        m_historyDW->setWidget(m_historyWidget);
        addDockWidget(Qt::RightDockWidgetArea, m_historyDW);
	
	// AudioSources View
        m_audioSourcesDW = new QDockWidget(tr("Resources Bin"), this);
        m_audioSourcesDW->setObjectName("AudioSourcesDockWidget");
        m_audiosourcesview = new ResourcesWidget(m_audioSourcesDW);
        m_audiosourcesview->setFocusPolicy(Qt::NoFocus);
        m_audioSourcesDW->setWidget(m_audiosourcesview);
        addDockWidget(Qt::TopDockWidgetArea, m_audioSourcesDW);
        m_audioSourcesDW->hide();
	
	// Meter Widgets
        m_correlationMeterDW = new QDockWidget(tr("Correlation Meter"), this);
        m_correlationMeterDW->setObjectName("CorrelationMeterDockWidget");
        m_correlationMeter = new CorrelationMeterWidget(m_correlationMeterDW);
        m_correlationMeter->setFocusPolicy(Qt::NoFocus);
        m_correlationMeterDW->setWidget(m_correlationMeter);
        addDockWidget(Qt::TopDockWidgetArea, m_correlationMeterDW);
        m_correlationMeterDW->hide();

        m_spectralMeterDW = new QDockWidget(tr("FFT Spectrum"), this);
        m_spectralMeterDW->setObjectName("SpectralMeterDockWidget");
        m_spectralMeter = new SpectralMeterWidget(m_spectralMeterDW);
        m_spectralMeter->setFocusPolicy(Qt::NoFocus);
        m_spectralMeterDW->setWidget(m_spectralMeter);
        addDockWidget(Qt::TopDockWidgetArea, m_spectralMeterDW);
        m_spectralMeterDW->hide();

	// BusMonitor
        m_busMonitorDW = new QDockWidget(tr("VU Meters"), this);
        m_busMonitorDW->setObjectName(tr("VU Meters"));

        busMonitor = new BusMonitor(m_busMonitorDW);
        m_busMonitorDW->setWidget(busMonitor);
        addDockWidget(Qt::RightDockWidgetArea, m_busMonitorDW);
	
	m_sysinfo = new SysInfoToolBar(this);
	m_sysinfo->setObjectName("System Info Toolbar");
	addToolBar(Qt::BottomToolBarArea, m_sysinfo);
	
	m_progressBar = new ProgressToolBar(this);
	m_progressBar->setObjectName("Progress Toolbar");
	addToolBar(Qt::BottomToolBarArea, m_progressBar);
	m_progressBar->hide();


        m_mainMenuToolBar = new QToolBar(this);
        m_mainMenuToolBar->setObjectName("MainToolBar");
        m_mainMenuToolBar->toggleViewAction()->setText("Main Tool Bar");
        m_mainMenuToolBar->setStyleSheet("margin-top: 0px; margin-bottom: 0px;");
        m_mainMenuToolBar->setMovable(false);

        m_mainMenuBar = new QMenuBar(m_mainMenuToolBar);
        QLabel* webAddressLabel = new QLabel(m_mainMenuToolBar);
        webAddressLabel->setText("<a href=\"http://traverso-daw.org/Welcome\">traverso-daw.org</a>");
        webAddressLabel->setOpenExternalLinks(true);

        m_mainMenuToolBar->addWidget(m_mainMenuBar);
        m_mainMenuToolBar->addWidget(webAddressLabel);
        addToolBar(Qt::TopToolBarArea, m_mainMenuToolBar);
        addToolBarBreak();


	m_projectToolBar = new QToolBar(this);
	m_projectToolBar->setObjectName("Project Toolbar");
        addToolBar(Qt::TopToolBarArea, m_projectToolBar);
	
	m_editToolBar = new QToolBar(this);
	m_editToolBar->setObjectName("Edit Toolbar");
        addToolBar(Qt::TopToolBarArea, m_editToolBar);

        m_transportConsole = new TransportConsoleWidget(this);
        m_transportConsole->setObjectName("Transport Console");
#if defined (Q_WS_MAC)
	addToolBar(Qt::BottomToolBarArea, transportConsole);
#else
        addToolBar(Qt::TopToolBarArea, m_transportConsole);
#endif

	if (config().get_property("Themer", "textundericons", false).toBool()) {
		m_projectToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		m_editToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	}

	int iconsize = config().get_property("Themer", "iconsize", "22").toInt();
	m_projectToolBar->setIconSize(QSize(iconsize, iconsize));
	m_editToolBar->setIconSize(QSize(iconsize, iconsize));


        addToolBarBreak();

        m_sessionTabsToolbar = new QToolBar(this);
        m_sessionTabsToolbar->setObjectName("Sheet Tabs");
        m_sessionTabsToolbar->toggleViewAction()->setText("Sheet Tabs");
        addToolBar(Qt::TopToolBarArea, m_sessionTabsToolbar);
        if (m_sessionTabsToolbar->layout()) {
                m_sessionTabsToolbar->layout()->setSpacing(6);
        }

        m_welcomeWidget = new WelcomeWidget(this);
        m_welcomeWidget->show();
        m_centerAreaWidget->addWidget(m_welcomeWidget/*, tr("&0: Welcome")*/);
        m_welcomeWidget->setFocus(Qt::MouseFocusReason);

	// Some default values.
        m_project = 0;
        m_previousCenterAreaWidgetIndex = 0;
        m_currentSheetWidget = 0;
	m_exportDialog = 0;
	m_cdWritingDialog = 0;
	m_settingsdialog = 0;
	m_projectManagerDialog = 0;
	m_openProjectDialog = 0;
	m_newProjectDialog = 0;
	m_insertSilenceDialog = 0;
	m_newSheetDialog = 0;
	m_newTrackDialog = 0;
	m_quickStart = 0;
	m_restoreProjectBackupDialog = 0;
        m_vuLevelUpdateFrequency = 40;
	
	create_menus();
	
	/** Read in the Interface settings and apply them
	 */
	resize(config().get_property("Interface", "size", QSize(900, 600)).toSize());
	move(config().get_property("Interface", "pos", QPoint(200, 200)).toPoint());
	restoreState(config().get_property("Interface", "windowstate", "").toByteArray());

	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(&pm(), SIGNAL(unsupportedProjectDirChangeDetected()), this, SLOT(project_dir_change_detected()));	
	connect(&pm(), SIGNAL(projectLoadFailed(QString,QString)), this, SLOT(project_load_failed(QString,QString)));
	connect(&pm(), SIGNAL(projectFileVersionMismatch(QString,QString)), this, SLOT(project_file_mismatch(QString,QString)), Qt::QueuedConnection);

	cpointer().add_contextitem(this);

	connect(&config(), SIGNAL(configChanged()), this, SLOT(config_changed()));
	connect(&config(), SIGNAL(configChanged()), this, SLOT(update_follow_state()));
	update_follow_state();

	setUnifiedTitleAndToolBarOnMac(true);

        m_vuLevelUpdateTimer.start(m_vuLevelUpdateFrequency, this);
        m_vuLevelPeakholdTimer.start(1000, this);
}

TMainWindow::~TMainWindow()
{
	PENTERDES;
	
	if (m_exportDialog) {
		delete m_exportDialog;
	}
	
	config().set_property("Interface", "size", size());
	config().set_property("Interface", "fullScreen", isFullScreen());
	config().set_property("Interface", "pos", pos());
	config().set_property("Interface", "windowstate", saveState());
}


void TMainWindow::set_project(Project* project)
{
        PENTER;
	
        foreach(SheetWidget* sw, m_sheetWidgets) {
                remove_session(sw->get_sheet());
        }

        m_project = project;

        m_trackFinderModel->clear();
        track_finder_show_initial_text();

        if ( m_project ) {
                connect(m_project, SIGNAL(projectLoadFinished()), this, SLOT(project_load_finished()));

		setWindowTitle(project->get_title() + " - Traverso");
                set_project_actions_enabled(true);

	} else {
                m_welcomeWidget->setFocus(Qt::MouseFocusReason);
                setWindowTitle("Traverso");
                set_project_actions_enabled(false);
                show_welcome_page();
	}
}

void TMainWindow::project_load_finished()
{
        PENTER;
        if (!m_project) {
                return;
        }

        connect(m_project, SIGNAL(currentSessionChanged(TSession*)), this, SLOT(show_session(TSession*)));
        connect(m_project, SIGNAL(sheetAdded(Sheet*)), this, SLOT(add_sheetwidget(Sheet*)));
        connect(m_project, SIGNAL(sheetRemoved(Sheet*)), this, SLOT(remove_sheetwidget(Sheet*)));

        add_session(m_project);
        foreach(TSession* session, m_project->get_child_sessions()) {
                add_session(session);
        }

        foreach(Sheet* sheet, m_project->get_sheets()) {
                add_session(sheet);
                foreach(TSession* session, sheet->get_child_sessions()) {
                        add_session(session);
                }
        }

        show_session(m_project->get_current_session());
}

void TMainWindow::remove_sheetwidget(Sheet* sheet)
{
        remove_session(sheet);
}

void TMainWindow::add_sheetwidget(Sheet* sheet)
{
        add_session(sheet);
}

void TMainWindow::add_session(TSession *session)
{
        if ( ! session->is_child_session()) {
                TSessionTabWidget* tabWidget = new TSessionTabWidget(m_sessionTabsToolbar, session);
                m_sessionTabsToolbar->addWidget(tabWidget);
                m_sessionTabWidgets.insert(session, tabWidget);
        }

        SheetWidget* sheetWidget = new SheetWidget(session, m_centerAreaWidget);
        m_sheetWidgets.insert(session, sheetWidget);
        m_centerAreaWidget->addWidget(sheetWidget);

        connect(session, SIGNAL(transportStopped()), this, SLOT(update_follow_state()));
        connect(session, SIGNAL(modeChanged()), this, SLOT(update_effects_state()));
        connect(session, SIGNAL(tempFollowChanged(bool)), this, SLOT(update_temp_follow_state(bool)));

        Sheet* sheet = qobject_cast<Sheet*>(session);
        if (sheet) {
                connect(session, SIGNAL(snapChanged()), this, SLOT(update_snap_state()));
                connect(session, SIGNAL(sessionAdded(TSession*)), this, SLOT(add_session(TSession*)));
                connect(session, SIGNAL(sessionRemoved(TSession*)), this, SLOT(remove_session(TSession*)));
        }
}

void TMainWindow::remove_session(TSession* session)
{
        SheetWidget* sw = m_sheetWidgets.value(session);
        if (sw) {
                m_sheetWidgets.remove(session);
                m_centerAreaWidget->removeWidget(sw);
                if (m_currentSheetWidget == sw) {
                        m_currentSheetWidget = 0;
                }
                delete sw;

                TSessionTabWidget* tabWidget = m_sessionTabWidgets.take(session);
                if (tabWidget) {
                        delete tabWidget;
                }
        }
}


void TMainWindow::show_session(TSession* session)
{
	PENTER;
	
	SheetWidget* sheetWidget = 0;
	
        if (!session) {
		m_snapAction->setEnabled(false);
		m_effectAction->setEnabled(false);
		m_followAction->setEnabled(false);

                return;

	} else {
                sheetWidget = m_sheetWidgets.value(session);
                if (!sheetWidget) {
                        return;
                }

		update_snap_state();
		update_effects_state();
		m_snapAction->setEnabled(true);
		m_effectAction->setEnabled(true);
		m_followAction->setEnabled(true);
	}

        if (m_currentSheetWidget && m_project && m_project->sheets_are_track_folder()) {
                sheetWidget->get_sheet()->set_hzoom(m_currentSheetWidget->get_sheet()->get_hzoom());
                sheetWidget->get_sheetview()->set_hscrollbar_value(m_currentSheetWidget->get_sheetview()->hscrollbar_value());
        }

        if (m_currentSheetWidget && cpointer().keyboard_only_input()) {
                ClipsViewPort* currentClipViewPort = m_currentSheetWidget->get_sheetview()->get_clips_viewport();
                ClipsViewPort* nextClipViewPort = sheetWidget->get_sheetview()->get_clips_viewport();
                nextClipViewPort->set_edit_point_position(currentClipViewPort->get_hold_cursor_pos());
        }

        m_currentSheetWidget = sheetWidget;
        m_currentSheetWidget->setFocus(Qt::MouseFocusReason);

        m_centerAreaWidget->setCurrentWidget(m_currentSheetWidget);

        if (session) {
                pm().get_undogroup()->setActiveStack(session->get_history_stack());
//                setWindowTitle(m_project->get_title() + ": Sheet " + session->get_name() + " - Traverso");
        }
}

Command* TMainWindow::about_traverso()
{
	PENTER;
	QString text(tr("Traverso %1 (built with Qt %2)\n\n" 
			"A multitrack audio recording and editing program.\n\n"
			"Look in the Help menu for more info.\n\n"
			"Traverso is brought to you by R. Sijrier and others,\n"
			"including all the people from the Free Software world\n"
			"who contributed the important technologies on which\n"
			"Traverso is based (Gcc, Qt, Xorg, Linux, and so on)" ).arg(VERSION).arg(QT_VERSION_STR));
	QMessageBox::about ( this, tr("About Traverso"), text);

	return (Command*) 0;
}

Command* TMainWindow::quick_start()
{
	PENTER;
	
	if (m_quickStart == 0) {
		m_quickStart = new QDialog();
		Ui_QuickStartDialog *qsd = new Ui_QuickStartDialog();
		qsd->setupUi(m_quickStart);
	}
	m_quickStart->show();
	
	return (Command*) 0;
}

Command* TMainWindow::full_screen()
{
	if (isFullScreen())
		showNormal();
	else
		showFullScreen();
	return (Command*) 0;
}

Command* TMainWindow::show_fft_meter_only()
{
        if (m_centerAreaWidget->isHidden()) {
                m_centerAreaWidget->show();
                restoreState(m_windowState);
        } else {
                m_windowState = saveState();
                m_spectralMeterDW->show();

                m_correlationMeterDW->hide();
                m_historyDW->hide();
                m_audioSourcesDW->hide();
                m_projectToolBar->hide();
                m_editToolBar->hide();
                m_sessionTabsToolbar->hide();
                m_transportConsole->hide();
                m_centerAreaWidget->hide();
        }
        return 0;
}

void TMainWindow::timerEvent(QTimerEvent *event)
{
        if (event->timerId() == m_vuLevelUpdateTimer.timerId()) {
                update_vu_levels_peak();
        }
        if (event->timerId() == m_vuLevelPeakholdTimer.timerId()) {
                reset_vu_levels_peak_hold_value();
        }
}

void TMainWindow::keyPressEvent( QKeyEvent * e)
{
        if (m_trackFinder->hasFocus()) {
                if (e->key() == Qt::Key_Escape) {
                        track_finder_show_initial_text();
                        if (m_currentSheetWidget) {
                                m_currentSheetWidget->setFocus();
                        }
                }
                return;
        }
	ie().catch_key_press(e);
	e->ignore();
}

void TMainWindow::keyReleaseEvent( QKeyEvent * e)
{
	ie().catch_key_release(e);
	e->ignore();
}

bool TMainWindow::eventFilter(QObject * obj, QEvent * event)
{
        if (event->type() == QEvent::MouseButtonPress && obj == m_trackFinder) {
                show_track_finder();
                return true;
        }

	QMenu* menu = qobject_cast<QMenu*>(obj);
	
	// If the installed filter was for a QMenu, we need to
	// delegate key releases to the InputEngine, e.g. a hold 
	// action would never finish if we release the hold key 
	// on the open Menu, resulting in weird behavior!
	if (menu) {
		if (event->type() == QEvent::KeyRelease) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
			ie().catch_key_release(keyEvent);
			return true;
		} else if (event->type() == QEvent::MouseMove) {
                        // FIXME: Seems no longer to be the case??
			// Also send mouse move events to the current viewport
			// so in case we close the Menu, and _do_not_move_the_mouse
			// and perform an action, it could be delegated to the wrong ViewItem!
			// Obviously we don't want to send this event when the InputEngine is still
			// in holding mode, to avoid jog() being called for the active HoldCommand!
//			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
//                        ViewPort* vp = static_cast<ViewPort*>(cpointer().get_viewport());
//			if (vp && !ie().is_holding()) {
//				vp->mouseMoveEvent(mouseEvent);
//			}
                } else {
			return false;
		}
	}
	
	return false;
}


void TMainWindow::changeEvent(QEvent *event)
{
	switch (event->type()) {
		case QEvent::ActivationChange:
		case QEvent::WindowStateChange:
			// clean up the ie after Alt-Tab
			// if problems remain, maybe ie().reset() will help...
                        ie().abort_current_hold_actions();
		default:
			break;
	}
	
	// pass the event on to the parent class
	QMainWindow::changeEvent(event);
}

Command * TMainWindow::show_export_widget( )
{
	if (m_cdWritingDialog && !m_cdWritingDialog->isHidden()) {
		return 0;
	}
	
	if (! m_exportDialog) {
		m_exportDialog = new ExportDialog(this);
	}
	
	if (m_exportDialog->isHidden()) {
		m_exportDialog->show();
	}
	
	return (Command*) 0;
}

Command * TMainWindow::show_cd_writing_dialog( )
{
	if (m_exportDialog && !m_exportDialog->isHidden()) {
		return 0;
	}
	
	if (! m_cdWritingDialog) {
		m_cdWritingDialog = new CDWritingDialog(this);
	}
	
	if (m_cdWritingDialog->isHidden()) {
		m_cdWritingDialog->show();
	}
	
	return (Command*) 0;
}

void TMainWindow::create_menus( )
{
	QAction* action;
        QList<QKeySequence> list;

        QMenu* menu = m_mainMenuBar->addMenu(tr("&File"));
        menu->installEventFilter(this);

        action = menu->addAction(tr("&New..."));
        action->setIcon(find_pixmap(":/new"));
        action->setShortcuts(QKeySequence::New);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(show_newproject_dialog()));

        action = menu->addAction(tr("&Open..."));
        action->setIcon(QIcon(":/open"));
        action->setShortcuts(QKeySequence::Open);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(show_welcome_page()));

        menu->addSeparator();

        action = menu->addAction(tr("&Save"));
        m_projectMenuToolbarActions.append(action);
        action->setShortcuts(QKeySequence::Save);
        action->setIcon(QIcon(":/save"));
        connect(action, SIGNAL(triggered(bool)), &pm(), SLOT(save_project()));

        menu->addSeparator();

        action = menu->addAction(tr("&Close Project"));
        m_projectMenuToolbarActions.append(action);
        action->setShortcuts(QKeySequence::Cut);
        action->setIcon(QIcon(":/exit"));
        connect(action, SIGNAL(triggered(bool)), &pm(), SLOT(close_current_project()));

        menu->addSeparator();

        action = menu->addAction(tr("&Manage Project..."));
        m_projectMenuToolbarActions.append(action);
        list.clear();
	list.append(QKeySequence("F4"));
	action->setShortcuts(list);
	action->setIcon(QIcon(":/projectmanager"));
	menu->addAction(action);
        m_projectToolBar->addAction(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(show_project_manager_dialog()));

	action = menu->addAction(tr("&Export..."));
        m_projectMenuToolbarActions.append(action);
        list.clear();
	list.append(QKeySequence("F9"));
	action->setShortcuts(list);
	action->setIcon(QIcon(":/export"));
	m_projectToolBar->addAction(action);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(show_export_widget()));
	
	action = menu->addAction(tr("&CD Writing..."));
        m_projectMenuToolbarActions.append(action);
        list.clear();
	list.append(QKeySequence("F8"));
	action->setShortcuts(list);
	action->setIcon(QIcon(":/write-cd"));
	m_projectToolBar->addAction(action);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(show_cd_writing_dialog()));
	
	action = menu->addAction(tr("&Restore Backup..."));
        m_projectMenuToolbarActions.append(action);
        list.clear();
	list.append(QKeySequence("F10"));
	action->setShortcuts(list);
	action->setIcon(QIcon(":/restore"));
	m_projectToolBar->addAction(action);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(show_restore_project_backup_dialog()));

        menu->addSeparator();

        action = menu->addAction(tr("&Quit"));
        list.clear();
        list.append(QKeySequence("CTRL+Q"));
        action->setShortcuts(list);
        action->setIcon(QIcon(":/exit"));
        connect(action, SIGNAL(triggered( bool )), &pm(), SLOT(exit()));


        menu = m_mainMenuBar->addMenu(tr("&Edit"));
        menu->installEventFilter(this);

        action = menu->addAction(tr("Undo"));
        m_projectMenuToolbarActions.append(action);
        action->setIcon(QIcon(":/undo"));
	action->setShortcuts(QKeySequence::Undo);
	m_editToolBar->addAction(action);
	connect(action, SIGNAL(triggered( bool )), &pm(), SLOT(undo()));

	action = menu->addAction(tr("Redo"));
        m_projectMenuToolbarActions.append(action);
        action->setIcon(QIcon(":/redo"));
	action->setShortcuts(QKeySequence::Redo);
	m_editToolBar->addAction(action);
	connect(action, SIGNAL(triggered( bool )), &pm(), SLOT(redo()));	

	menu->addSeparator();
	m_editToolBar->addSeparator();

	action = menu->addAction(tr("Import &Audio..."));
        m_projectMenuToolbarActions.append(action);
        action->setIcon(QIcon(":/import-audio"));
	m_editToolBar->addAction(action);
	connect(action, SIGNAL(triggered()), this, SLOT(import_audio()));

	action = menu->addAction(tr("Insert Si&lence..."));
        m_projectMenuToolbarActions.append(action);
        action->setIcon(QIcon(":/import-silence"));
	m_editToolBar->addAction(action);
	connect(action, SIGNAL(triggered()), this, SLOT(show_insertsilence_dialog()));

	menu->addSeparator();
	m_editToolBar->addSeparator();

	m_snapAction = menu->addAction(tr("&Snap"));
        m_projectMenuToolbarActions.append(m_snapAction);
        m_snapAction->setIcon(QIcon(":/snap"));
	m_snapAction->setCheckable(true);
	m_snapAction->setToolTip(tr("Snap items to edges of other items while dragging."));
	m_editToolBar->addAction(m_snapAction);
	connect(m_snapAction, SIGNAL(triggered(bool)), this, SLOT(snap_state_changed(bool)));

        m_followAction = menu->addAction(tr("S&croll Playback"));
        m_projectMenuToolbarActions.append(m_followAction);
	m_followAction->setIcon(QIcon(":/follow"));
	m_followAction->setCheckable(true);
	m_followAction->setToolTip(tr("Keep play cursor in view while playing or recording."));
	m_editToolBar->addAction(m_followAction);
	connect(m_followAction, SIGNAL(triggered(bool)), this, SLOT(follow_state_changed(bool)));

        m_effectAction = menu->addAction(tr("&Show Effects"));
        m_projectMenuToolbarActions.append(m_effectAction);
        m_effectAction->setIcon(QIcon(":/effects"));
	m_effectAction->setCheckable(true);
	m_effectAction->setToolTip(tr("Show effect plugins and automation curves on tracks"));
	m_editToolBar->addAction(m_effectAction);
	connect(m_effectAction, SIGNAL(triggered(bool)), this, SLOT(effect_state_changed(bool)));

        menu = m_mainMenuBar->addMenu(tr("Vi&ew"));
        menu->installEventFilter(this);

        menu->addAction(m_historyDW->toggleViewAction());
        menu->addAction(m_busMonitorDW->toggleViewAction());
        menu->addAction(m_audioSourcesDW->toggleViewAction());

        action = menu->addAction(tr("Marker Editor..."));
        m_projectMenuToolbarActions.append(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(show_marker_dialog()));

        action = menu->addAction(tr("Toggle Full Screen"));
        connect(action, SIGNAL(triggered()), this, SLOT(full_screen()));

        action = menu->addAction(tr("Toggle FFT Only"));
        connect(action, SIGNAL(triggered()), this, SLOT(show_fft_meter_only()));

        menu->addSeparator();
	
        menu->addAction(m_correlationMeterDW->toggleViewAction());
        menu->addAction(m_spectralMeterDW->toggleViewAction());

	menu->addSeparator();
        action = menu->addAction(tr("ToolBars"));
        action->setEnabled(false);
	menu->addSeparator();
	
        menu->addAction(m_transportConsole->toggleViewAction());
        m_transportConsole->toggleViewAction()->setText(tr("Transport Console"));

	// if unifiedTitleAndToolBarOnMac == true we don't want the main toolbars
	// to be hidden. thus only add the menu entries on systems != OS X
#if !defined (Q_WS_MAC)
	menu->addAction(m_projectToolBar->toggleViewAction());
	m_projectToolBar->toggleViewAction()->setText(tr("Project"));

	menu->addAction(m_editToolBar->toggleViewAction());
	m_editToolBar->toggleViewAction()->setText(tr("Edit"));
#endif

        menu->addAction(m_mainMenuToolBar->toggleViewAction());
        menu->addAction(m_sessionTabsToolbar->toggleViewAction());

        menu->addAction(m_sysinfo->toggleViewAction());
	m_sysinfo->toggleViewAction()->setText(tr("System Information"));

	menu->addSeparator();
	
        menu = m_mainMenuBar->addMenu(tr("&Settings"));
        menu->installEventFilter(this);
	
        m_encodingMenu = menu->addMenu(tr("&Recording File Format"));
	
        action = m_encodingMenu->addAction("WAVE");
	action->setData("wav");
	connect(action, SIGNAL(triggered(bool)), this, SLOT(change_recording_format_to_wav()));
        action = m_encodingMenu->addAction("WavPack");
	action->setData("wavpack");
	connect(action, SIGNAL(triggered( bool )), this, SLOT(change_recording_format_to_wavpack()));
        action = m_encodingMenu->addAction("WAVE-64");
	action->setData("w64");
	connect(action, SIGNAL(triggered( bool )), this, SLOT(change_recording_format_to_wav64()));
	
        m_resampleQualityMenu = menu->addMenu(tr("&Resample Quality"));
        action = m_resampleQualityMenu->addAction(tr("Best"));
	action->setData(0);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(change_resample_quality_to_best()));
        action = m_resampleQualityMenu->addAction(tr("High"));
	action->setData(1);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(change_resample_quality_to_high()));
        action = m_resampleQualityMenu->addAction(tr("Medium"));
	action->setData(2);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(change_resample_quality_to_medium()));
        action = m_resampleQualityMenu->addAction(tr("Fast"));
	action->setData(3);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(change_resample_quality_to_fast()));

	// fake a config changed 'signal-slot' action, to set the encoding menu icons
	config_changed();
	
	menu->addSeparator();
	
	action = menu->addAction(tr("&Preferences..."));
	connect(action, SIGNAL(triggered( bool )), this, SLOT(show_settings_dialog()));
	
	
        menu = m_mainMenuBar->addMenu(tr("&Help"));
        menu->installEventFilter(this);

	action = menu->addAction(tr("&Getting Started"));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(quick_start()));
	
	action = menu->addAction(tr("&User Manual"));
	action->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(open_help_browser()));
	
	action = menu->addAction(tr("&About Traverso"));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(about_traverso()));

        set_project_actions_enabled(false);

        m_contextMenuDict.insert("AudioClip",tr("Audio Clip"));
        m_contextMenuDict.insert("AudioTrack", tr("Audio Track"));
        m_contextMenuDict.insert("Curve",tr("Curve"));
        m_contextMenuDict.insert("CurveNode",tr("Curve Node"));
        m_contextMenuDict.insert("FadeCurve",tr("Fade Curve"));
        m_contextMenuDict.insert("Marker",tr("Marker"));
        m_contextMenuDict.insert("Sheet",tr("Sheet"));
        m_contextMenuDict.insert("TBusTrack",tr("Bus Track"));
        m_contextMenuDict.insert("TimeLine",tr("Time Line"));
        m_contextMenuDict.insert("TBusTrackPanel", tr("Bus Track"));
        m_contextMenuDict.insert("AudioTrackPanel",tr("Audio Track"));
        m_contextMenuDict.insert("Plugin",tr("Plugin"));
}

void TMainWindow::set_project_actions_enabled(bool enable)
{
        foreach(QAction* action, m_projectMenuToolbarActions) {
                action->setEnabled(enable);
        }

}


void TMainWindow::process_context_menu_action( QAction * action )
{
	QStringList strings = action->data().toStringList();
	QString name = strings.first();
	ie().broadcast_action_from_contextmenu(name);
}

Command * TMainWindow::show_context_menu( )
{
	QList<QObject* > items;
	
	// In case of a holding action, show the menu for the holding command!
	// If not, show the menu for the topmost context item, and it's 
	// siblings as submenus
	if (ie().is_holding()) {
		Command* holding = ie().get_holding_command();
		if (holding) {
			items.append(holding);
		}
	} else {
		items = cpointer().get_context_items();
		
		// Filter out classes that don't need to show up in the menu
		foreach(QObject* item, items) {
			QString className = item->metaObject()->className();
			if ( ( ! className.contains("View")) || className.contains("ViewPort") ) {
				items.removeAll(item);
			}
		}
	}
	
	if (items.isEmpty()) {
		printf("Interface:: No items under mouse to show context menu for!\n");
		return 0;
	}
	
	// 'Store' the contextitems under the mouse cursor, so the InputEngine
	// dispatches the 'keyfact' from the menu to the 'pointed' objects!
	cpointer().set_contextmenu_items(cpointer().get_context_items());

	QMenu* toplevelmenu = 0;
	QAction* action = 0;
			
	for (int i=0; i<items.size(); ++i) {
		QObject* item = items.at(i);
		
		QString className = item->metaObject()->className();
		
		if (i==0) {
			toplevelmenu = m_contextMenus.value(className);
		
			if ( ! toplevelmenu ) {
				printf("Interface: No menu for %s, creating new one\n", QS_C(className));
				toplevelmenu = create_context_menu(item);
				if (! toplevelmenu ) {
					if (items.size() > 1) {
                                                toplevelmenu = new QMenu(this);
					} else {
						return 0;
					}
						
				}
				m_contextMenus.insert(className, toplevelmenu);
				connect(toplevelmenu, SIGNAL(triggered(QAction*)), this, SLOT(process_context_menu_action(QAction*)));
			} else {
				break;
			}
		} else {
			// Create submenus
			toplevelmenu->addSeparator();
			QMenu* menu = create_context_menu(item);
			if (! menu) {
				continue;
			}
			action = toplevelmenu->insertMenu(action, menu);
                        QString name = m_contextMenuDict.value(className.remove("View"));

                        action->setText(name);
		}
	}

	// It's impossible there is NO toplevelmenu, but oh well...
	if (toplevelmenu) {
                // using toplevelmenu->exec() continues on the event loop
                // when the hold action finishes, input engine clears hold items
                // which could be referenced again in inputengine, causing a segfault.
                // so showing it will be sufficient. In fact, using exec() is
                // considered bad practice due this very issue.
                toplevelmenu->move(QCursor::pos());
                toplevelmenu->show();
        }
	
	return 0;
}

QString create_keyfact_string(QString& keyfact, QList<int> modifiers) 
{
	QString modifierkey = "";
	foreach(int key, modifiers) {
		if (keyfact.contains("+)")) continue;
		if (key == Qt::Key_Alt) {
			modifierkey += "ALT+";
		} else if (key == Qt::Key_Control) {
			modifierkey += "CTRL+";
                } else if (key == Qt::Key_Shift) {
                        modifierkey += "SHIFT+";
		} else {
			QKeySequence seq(key);
			modifierkey += seq.toString() + " +";
		}
	}
	if (!modifierkey.isEmpty()) {
		modifierkey.prepend("(");
		modifierkey.append(")");
	}
	return modifierkey + " " + keyfact;
}

Command * TMainWindow::export_keymap()
{
	QTextStream out;
	QFile data(QDir::homePath() + "/traversokeymap.html");
	if (data.open(QFile::WriteOnly | QFile::Truncate)) {
		out.setDevice(&data);
	} else {
		return 0;
	}

	QString str;
	(Command *) get_keymap(str);
	out << str;

	data.close();
	return 0;
}

Command * TMainWindow::get_keymap(QString &str)
{
	
	QMap<QString, QList<const QMetaObject*> > objects;
	
	QList<const QMetaObject*> sheetlist; sheetlist << &Sheet::staticMetaObject; sheetlist << &SheetView::staticMetaObject;
        QList<const QMetaObject*> audiotracklist; audiotracklist << &AudioTrack::staticMetaObject; audiotracklist << &AudioTrackView::staticMetaObject;
        QList<const QMetaObject*> bustracklist; bustracklist << &TBusTrack::staticMetaObject; bustracklist << &TBusTrackView::staticMetaObject;
        QList<const QMetaObject*> cliplist; cliplist << &AudioClip::staticMetaObject; cliplist << &AudioClipView::staticMetaObject;
	QList<const QMetaObject*> curvelist; curvelist << &Curve::staticMetaObject; curvelist << &CurveView::staticMetaObject;
	QList<const QMetaObject*> timelinelist; timelinelist << &TimeLine::staticMetaObject; timelinelist << &TimeLineView::staticMetaObject;
	QList<const QMetaObject*> pluginlist; pluginlist << &Plugin::staticMetaObject; pluginlist << &PluginView::staticMetaObject;
	QList<const QMetaObject*> fadelist; fadelist << &FadeCurve::staticMetaObject; fadelist << &FadeView::staticMetaObject;
        QList<const QMetaObject*> interfacelist; interfacelist << &TMainWindow::staticMetaObject;
	QList<const QMetaObject*> pmlist; pmlist << &ProjectManager::staticMetaObject;
        QList<const QMetaObject*> gainlist; gainlist << &Gain::staticMetaObject;
        QList<const QMetaObject*> movetracklist; movetracklist << &MoveTrack::staticMetaObject;
        QList<const QMetaObject*> movecliplist; movecliplist << &MoveClip::staticMetaObject;
        QList<const QMetaObject*> movecurvenodelist; movecurvenodelist << &MoveCurveNode::staticMetaObject;
        QList<const QMetaObject*> zoomlist; zoomlist << &Zoom::staticMetaObject;
        QList<const QMetaObject*> trackpanlist; trackpanlist << &TrackPan::staticMetaObject;
        QList<const QMetaObject*> croplist; croplist << &Crop::staticMetaObject;
        QList<const QMetaObject*> movemarkerlist; movemarkerlist << &MoveMarker::staticMetaObject;
        QList<const QMetaObject*> moveworkcursorlist; moveworkcursorlist << &WorkCursorMove::staticMetaObject;
        QList<const QMetaObject*> moveplaycursorlist; moveplaycursorlist << &PlayHeadMove::staticMetaObject;
        QList<const QMetaObject*> moveclipedgelist; moveclipedgelist << &MoveEdge::staticMetaObject;

	objects.insert("Sheet", sheetlist);
        objects.insert("Audio Track", audiotracklist);
        objects.insert("Bus Track", bustracklist);
        objects.insert("AudioClip", cliplist);
	objects.insert("Curve", curvelist);
	objects.insert("TimeLine", timelinelist);
	objects.insert("Plugin", pluginlist);
	objects.insert("Fade", fadelist);
	objects.insert("Interface", interfacelist);
        objects.insert("ProjectManager", pmlist);
        objects.insert("Gain", gainlist);
        objects.insert("Move Track", movetracklist);
        objects.insert("Move AudioClip", movecliplist);
        objects.insert("Move Clip Edge", moveclipedgelist);
        objects.insert("Move Marker", movemarkerlist);
        objects.insert("Move Curve Node", movecurvenodelist);
        objects.insert("Move Work Cursor", moveworkcursorlist);
        objects.insert("Move Play Cursor", moveplaycursorlist);
        objects.insert("Zoom", zoomlist);
        objects.insert("Track Pan", trackpanlist);
        objects.insert("Magnetic Cut", croplist);

	
        str = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
              "<style type=\"text/css\">\n"
              "H1 {text-align: left; font-size: 20px;}\n"
              "table {font-size: 12px; border: solid; border-width: 1px; width: 600px;}\n"
              ".object {background-color: #ccc; font-size: 16px; font-weight: bold;}\n"
              ".description {background-color: #ddd; width: 300px; padding: 2px; font-size: 12px; font-weight: bold;}\n"
              "</style>\n"
              "</head>\n<body>\n<h1>Traverso keymap: " + config().get_property("CCE", "keymap", "default").toString() + "</h1>\n";
	
	foreach(QList<const QMetaObject* > objectlist, objects.values()) {
		QString name = objects.key(objectlist);
		
                str += "<table><tr class=\"object\">\n<td colspan=\"2\">" + name + "</td></tr>\n";
                str += "<tr><td class=\"description\">" +tr("Description") + "</td><td class=\"description\">" + tr("Key Sequence") + "</td></tr>\n";
		
		QStringList result;
		
		foreach(const QMetaObject* mo, objectlist) {
                        while (mo) {
                                QList<MenuData > list;
                                
                                ie().create_menudata_for_metaobject(mo, list);
                        
                                QList<QMenu* > menulist;
                                QMenu* menu = create_context_menu(0, &list);
                                if (menu) {
                                        menulist.append(menu);
                                        foreach(QAction* action, menu->actions()) {
                                                if (action->menu()) {
                                                        menulist.append(action->menu());
                                                }
                                        }
                                        for (int i=0; i<menulist.size(); ++i) {
                                                QMenu* somemenu = menulist.at(i);
                                                foreach(QAction* action, somemenu->actions()) {
                                                        QStringList strings = action->data().toStringList();
                                                        if (strings.size() >= 3) {
                                                                QString submenuname = "";
                                                                if (i > 0) {
                                                                        submenuname = somemenu->menuAction()->text() + "&#160;&#160;&#160;&#160;";
                                                                }
                                                                QString keyfact = strings.at(2);
                                                                keyfact.replace("<", "&lt;");
                                                                                
                                                                result += QString("<tr><td>") + submenuname + strings.at(1) + "</td><td>" + keyfact + "</td></tr>\n";
                                                        }
                                                }
                                        }
                                        delete menu;
                                }
                                mo = mo->superClass();
                        }
		}
		result.sort();
                result.removeDuplicates();
		str += result.join("");
                str += "</table>\n<p></p><p></p>\n";
	}
	
        str += "</body>\n</html>";
	
	return 0;
}

QMenu* TMainWindow::create_context_menu(QObject* item, QList<MenuData >* menulist)
{
	QList<MenuData > list;
	if (item) {
		 list = ie().create_menudata_for( item );
	} else {
		list = *menulist;
	}
	
	if (list.size() == 0) {
		// Empty menu!
		return 0;
	}
	
	qSort(list.begin(), list.end(), MenuData::smaller);

	
	QString name;
	if (item) {
                name = m_contextMenuDict.value(QString(item->metaObject()->className()).remove("View"));
	}

	QMenu* menu = new QMenu(this);
	menu->installEventFilter(this);
	
        QAction* menuAction = menu->addAction(name);
	QFont font(themer()->get_font("ContextMenu:fontscale:actions"));
	font.setBold(true);
	menuAction->setFont(font);
	menuAction->setEnabled(false);
	menu->addSeparator();
	menu->setFont(themer()->get_font("ContextMenu:fontscale:actions"));
	
	QMap<QString, QList<MenuData>* > submenus;
	
	for (int i=0; i<list.size(); ++i) {
		MenuData data = list.at(i);
		
		// Merge entries with equal actions, but different key facts.
		for (int j=i+1; j<list.size(); ++j) {
			if (list.at(j).description == data.description && list.at(j).submenu == data.submenu) {
				QString mergestring = list.at(j).keysequence;
				data.keysequence = create_keyfact_string(data.keysequence, data.modifierkeys) +
						" ,  " +
						create_keyfact_string(mergestring, list.at(j).modifierkeys);
				list.removeAt(j);
			}
		}
		
		// If this MenuData item is a submenu, add to the 
		// list of submenus, which will be processed lateron
		// Else, add the MenuData item as action in the Menu
		if ( ! data.submenu.isEmpty() ) {
			QList<MenuData>* list;
			if ( ! submenus.contains(data.submenu)) {
				submenus.insert(data.submenu, new QList<MenuData>());
			}
			list = submenus.value(data.submenu);
			list->append(data);
		} else {
			QString keyfact = create_keyfact_string(data.keysequence, data.modifierkeys);
			QString text = QString(data.description + "  " + keyfact);
			QAction* action = new QAction(this);
			action->setText(text);
			QStringList strings;
			strings << data.iedata << data.description << keyfact;
			action->setData(strings);
			menu->addAction(action);
		}
	}
	
	// For all submenus, create the Menu, and add
	// actions, a little code duplication here, adding action to the 
	// menu is also done ~10 lines up ...
	QList<QString> keys = submenus.keys();
	foreach(const QString &key, keys) {
		QList<MenuData>* list = submenus.value(key);
		
		qSort(list->begin(), list->end(), MenuData::smaller);

                QMenu* subMenu = new QMenu(this);
                subMenu->setFont(themer()->get_font("ContextMenu:fontscale:actions"));
		
		QFont font(themer()->get_font("ContextMenu:fontscale:actions"));
		font.setBold(true);
                subMenu->menuAction()->setFont(font);
		
                QAction* action = menu->insertMenu(0, subMenu);
                action->setText(key);
		foreach(MenuData data, *list) {
                        QAction* action = new QAction(subMenu);
			QString keyfact = create_keyfact_string(data.keysequence, data.modifierkeys);
			QString text = QString(data.description + "  " + keyfact);
			action->setText(text);
			QStringList strings;
			strings << data.iedata << data.description << keyfact;
			action->setData(strings);
                        subMenu->addAction(action);
		}
		
		delete list;
	}
	
	return menu;
}

void TMainWindow::set_insertsilence_track(AudioTrack* track)
{
	if (m_insertSilenceDialog) {
		m_insertSilenceDialog->setTrack(track);
	}
}

void TMainWindow::select_fade_in_shape( )
{
	QMenu* menu = m_contextMenus.value("fadeInSelector");
	
	if (!menu) {
		menu = create_fade_selector_menu("fadeInSelector");
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(set_fade_in_shape(QAction*)));
	}
		
	
	menu->exec(QCursor::pos());
}

void TMainWindow::select_fade_out_shape( )
{
	QMenu* menu = m_contextMenus.value("fadeOutSelector");
	
	if (!menu) {
		menu = create_fade_selector_menu("fadeOutSelector");
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(set_fade_out_shape(QAction*)));
	}
		
	
	menu->exec(QCursor::pos());
}


void TMainWindow::set_fade_in_shape( QAction * action )
{
	QList<QObject* > items = cpointer().get_context_items();
	foreach(QObject* obj, items) {
		AudioClipView* acv = qobject_cast<AudioClipView*>(obj);
		if (acv) {
			if (! acv->get_clip()->get_fade_in() ) {
				acv->get_clip()->set_fade_in(1);
			}
			acv->get_clip()->get_fade_in()->set_shape(action->data().toString());
			break;
		}
	}
}

void TMainWindow::set_fade_out_shape( QAction * action )
{
	QList<QObject* > items = cpointer().get_context_items();
	foreach(QObject* obj, items) {
		AudioClipView* acv = qobject_cast<AudioClipView*>(obj);
		if (acv) {
			if (! acv->get_clip()->get_fade_out() ) {
				acv->get_clip()->set_fade_out(1);
			}
			acv->get_clip()->get_fade_out()->set_shape(action->data().toString());
			break;
		}
	}
}


QMenu* TMainWindow::create_fade_selector_menu(const QString& fadeTypeName)
{
        QMenu* menu = new QMenu(this);
	
	foreach(QString name, FadeCurve::defaultShapes) {
		QAction* action = menu->addAction(name);
		action->setData(name);
	}
	
	m_contextMenus.insert(fadeTypeName, menu);
	
	return menu;
}

void TMainWindow::config_changed()
{
/*	bool toggled = config().get_property("Interface", "OpenGL", false).toBool();

	foreach(SheetWidget* widget, m_sheetWidgets) {
		widget->set_use_opengl(toggled);
	}*/
	
	QString encoding = config().get_property("Recording", "FileFormat", "wav").toString();
	QList<QAction* > actions = m_encodingMenu->actions();
	
	foreach(QAction* action, actions) {
		if (action->data().toString() == encoding) {
			action->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
		} else {
			action->setIcon(QIcon());
		}
	}
	
	int quality = config().get_property("Conversion", "RTResamplingConverterType", DEFAULT_RESAMPLE_QUALITY).toInt();
	actions = m_resampleQualityMenu->actions();
	
	bool useResampling = config().get_property("Conversion", "DynamicResampling", true).toBool();
	if (useResampling) {
		m_resampleQualityMenu->setEnabled(true);
	} else {
		m_resampleQualityMenu->setEnabled(false);
	}

	
	foreach(QAction* action, actions) {
		if (action->data().toInt() == quality) {
			action->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
		} else {
			action->setIcon(QIcon());
		}
	}

	switch (config().get_property("Themer", "toolbuttonstyle", 0).toInt()) {
		case 0:
			m_projectToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
			m_editToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
			break;

		case 1:
			m_projectToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
			m_editToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
			break;

		case 2:
			m_projectToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
			m_editToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
			break;

		case 3:
			m_projectToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
			m_editToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
			break;
	}

	int iconsize = config().get_property("Themer", "iconsize", "22").toInt();
	m_projectToolBar->setIconSize(QSize(iconsize, iconsize));
	m_editToolBar->setIconSize(QSize(iconsize, iconsize));

	int transportconsolesize = config().get_property("Themer", "transportconsolesize", "22").toInt();
        m_transportConsole->setIconSize(QSize(transportconsolesize, transportconsolesize));
        m_transportConsole->resize(m_transportConsole->sizeHint());
}

void TMainWindow::import_audio()
{
        Project* project = pm().get_project();
        if (!project) {
                return;
        }

        Sheet* sheet = qobject_cast<Sheet*>(m_currentSheetWidget->get_sheet());
        if (!sheet || !sheet->get_numtracks()) {
		return;
	}

        QStringList files = QFileDialog::getOpenFileNames(this, tr("Open Audio Files"),
                        project->get_import_dir(),
			tr("Audio files (*.wav *.flac *.ogg *.mp3 *.wv *.w64)"));

        if (files.isEmpty()) {
                return;
        }

        QList<AudioTrack*> tracks = sheet->get_audio_tracks();
	AudioTrack*	track = tracks.first();
	bool markers = false;

	ImportClipsDialog *importClips = new ImportClipsDialog(this);

	importClips->set_tracks(tracks);

	if (importClips->exec() == QDialog::Accepted) {
		if (!importClips->has_tracks()) {
			delete importClips;
			return;
		}

		track = importClips->get_selected_track();
		markers = importClips->get_add_markers();
	}

	// append the clips to the selected track
	TimeRef position = TimeRef();
	if (!track->get_cliplist().isEmpty()) {
		position = (track->get_cliplist().last())->get_track_end_location();
	}

        TimeLine* tl = sheet->get_timeline();
	int n = tl->get_markers().size() + 1;
	if (tl->has_end_marker()) {
		n -= 1;
	}

	while(!files.isEmpty()) {
		QString file = files.takeFirst();
		Import* import = new Import(file);
		import->set_track(track);
		import->set_position(position);

		QFileInfo fi(file);
		Marker* m = new Marker(tl, position);
		m->set_description(QString(tr("%1: %2")).arg(n).arg(fi.baseName()));

		if (import->create_readsource() != -1) {
			position += import->readsource()->get_length();
			Command::process_command(import);
			Command::process_command(tl->add_marker(m, true));
		}
		++n;
	}

	if (tl->has_end_marker()) {
		Marker* m = tl->get_end_marker();
		m->set_when(position);
	} else {
		Marker* m = new Marker(tl, position, Marker::ENDMARKER);
		Command::process_command(tl->add_marker(m, true));
	}

	delete importClips;
}

DigitalClock::DigitalClock(QWidget *parent)
	: QLCDNumber(parent)
{
	setSegmentStyle(Outline);
	setFrameStyle(QFrame::StyledPanel);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
	timer->start(1000);
	
	showTime();

	setWindowTitle(tr("Digital Clock"));
	resize(150, 60);
}

void DigitalClock::showTime()
{
	QTime time = QTime::currentTime();
	QString text = time.toString("hh:mm");
	if ((time.second() % 2) == 0)
		text[2] = ' ';
	display(text);
}


void TMainWindow::show_settings_dialog()
{
	if (!m_settingsdialog) {
		m_settingsdialog = new SettingsDialog(this);
	}
	
	m_settingsdialog->show();
}

void TMainWindow::show_settings_dialog_sound_system_page()
{
	show_settings_dialog();
	m_settingsdialog->show_page("Sound System");
}


void TMainWindow::closeEvent(QCloseEvent * event)
{
	event->ignore();
	pm().exit();
}

Command* TMainWindow::show_project_manager_dialog()
{
	if (! m_projectManagerDialog) {
		m_projectManagerDialog = new ProjectManagerDialog(this);
	}
	m_projectManagerDialog->show();
	return 0;
}

Command* TMainWindow::show_open_project_dialog()
{
	if (!m_openProjectDialog) {
		m_openProjectDialog = new OpenProjectDialog(this);
	}
	m_openProjectDialog->show();
	return 0;
}

Command * TMainWindow::show_newproject_dialog()
{
	if (! m_newProjectDialog ) {
		m_newProjectDialog = new NewProjectDialog(this);
                AudioFileCopyConvert* converter = m_newProjectDialog->get_converter();
                connect(converter, SIGNAL(taskStarted(QString)), m_progressBar, SLOT(set_label(QString)));
                connect(converter, SIGNAL(progress(int)), m_progressBar, SLOT(set_progress(int)));
                connect(m_newProjectDialog, SIGNAL(numberOfFiles(int)), m_progressBar, SLOT(set_num_files(int)));
	}
	m_newProjectDialog->show();
	return 0;
}

Command * TMainWindow::show_insertsilence_dialog()
{
	if (! m_insertSilenceDialog) {
		m_insertSilenceDialog = new InsertSilenceDialog(this);
	}
	
	m_insertSilenceDialog->setTrack(0);
	m_insertSilenceDialog->focusInput();
	m_insertSilenceDialog->show();
	
	return 0;
}


Command * TMainWindow::show_marker_dialog()
{
        MarkerDialog* markerDialog = new MarkerDialog(this);

        markerDialog->exec();
        delete markerDialog;

	return 0;
}

Command* TMainWindow::show_add_child_session_dialog()
{
        if (!m_project) {
                return 0;
        }

        Sheet* activeSheet = m_project->get_active_sheet();
        TSession* activeSession = m_project->get_current_session();
        TSession* parentSession = 0;

        if (activeSession->is_project_session()) {
                parentSession = activeSession;
        } else if (activeSheet) {
                parentSession = activeSheet;
        } else {
                info().information(tr("No Sheet active to add child view to"));
                return 0;
        }

        TSession* session = new TSession(parentSession);

        TTrackSelector selector(this, parentSession, session);
        if (selector.exec() == QDialog::Accepted) {
                parentSession->add_child_session(session);
                m_project->set_current_session(session->get_id());
        } else {
                delete session;
        }

        return 0;
}


QSize TMainWindow::sizeHint() const
{
	return QSize(800, 600);
}

Command* TMainWindow::show_newsheet_dialog()
{
	if (! m_newSheetDialog) {
		m_newSheetDialog = new NewSheetDialog(this);
	}
	
	m_newSheetDialog->show();
	
	return 0;
}

Command* TMainWindow::show_newtrack_dialog()
{
        if (!m_project) {
                return 0;
        }

        TSession* activeSession = m_project->get_current_session();
        Sheet* sheet = qobject_cast<Sheet*>(activeSession);
        Project* project = qobject_cast<Project*>(activeSession);

        if (sheet || project) {
                if (! m_newTrackDialog) {
                        m_newTrackDialog = new NewTrackDialog(this);
                }

                m_newTrackDialog->show();
        } else {
                if (!activeSession) {
                        return 0;
                }

                if (activeSession->get_parent_session()) {
                        TTrackSelector selector(this, activeSession->get_parent_session(), activeSession);
                        selector.exec();
                        return 0;
                }
        }

	
	return 0;
}


void TMainWindow::open_help_browser()
{
	info().information(tr("Opening User Manual in external browser!"));
	QDesktopServices::openUrl(QUrl("http://traverso-daw.org/UserManual"));
}

void TMainWindow::project_dir_change_detected()
{
	QMessageBox::critical(this, tr("Traverso - Important"), 
			      tr("A Project directory changed outside of Traverso. \n\n"
			      "This is NOT supported! Please undo this change now!\n\n"
			      "If you want to rename a Project, use the Project Manager instead!"),
	   			QMessageBox::Ok);
}

Command * TMainWindow::show_restore_project_backup_dialog(QString projectname)
{
	if (! m_restoreProjectBackupDialog) {
		m_restoreProjectBackupDialog = new RestoreProjectBackupDialog(this);
	}
	
	m_restoreProjectBackupDialog->set_project_name(projectname);
	m_restoreProjectBackupDialog->show();
	
	
	return 0;
}

void TMainWindow::show_restore_project_backup_dialog()
{
	Project* project = pm().get_project();
	
	if (! project ) {
		return;
	}
	
	show_restore_project_backup_dialog(project->get_title());
}

void TMainWindow::project_load_failed(QString project, QString reason)
{
	QMessageBox::critical(	this, tr("Traverso - Project load failed"), 
				tr("The requested Project `%1` \ncould not be loaded for the following reason:\n\n'%2'"
			      	"\n\nYou will now be given a list of available backups (if any) \n"
				"to restore the Project from.").arg(project).arg(reason),
				QMessageBox::Ok);
	
	show_restore_project_backup_dialog(project);
}



void TMainWindow::project_file_mismatch(QString rootdir, QString projectname)
{
	ProjectConverterDialog dialog(this);
	dialog.set_project(rootdir, projectname);
	dialog.exec();
}

void TMainWindow::change_recording_format_to_wav()
{
	config().set_property("Recording", "FileFormat", "wav");
	save_config_and_emit_message(tr("Changed encoding for recording to %1").arg("WAVE"));
	config().save();
}

void TMainWindow::change_recording_format_to_wav64()
{
	config().set_property("Recording", "FileFormat", "w64");
	save_config_and_emit_message(tr("Changed encoding for recording to %1").arg("WAVE-64"));
}

void TMainWindow::change_recording_format_to_wavpack()
{
	config().set_property("Recording", "FileFormat", "wavpack");
	save_config_and_emit_message(tr("Changed encoding for recording to %1").arg("WavPack"));
}

void TMainWindow::change_resample_quality_to_best()
{
	config().set_property("Conversion", "RTResamplingConverterType", 0);
	save_config_and_emit_message(tr("Changed resample quality to: %1").arg("Best"));
}

void TMainWindow::change_resample_quality_to_high()
{
	config().set_property("Conversion", "RTResamplingConverterType", 1);
	save_config_and_emit_message(tr("Changed resample quality to: %1").arg("High"));
}

void TMainWindow::change_resample_quality_to_medium()
{
	config().set_property("Conversion", "RTResamplingConverterType", 2);
	save_config_and_emit_message(tr("Changed resample quality to: %1").arg("Medium"));
}

void TMainWindow::change_resample_quality_to_fast()
{
	config().set_property("Conversion", "RTResamplingConverterType", 3);
	save_config_and_emit_message(tr("Changed resample quality to: %1").arg("Fast"));
}

void TMainWindow::save_config_and_emit_message(const QString & message)
{
	info().information(message);
	config().save();
}



Command * TMainWindow::start_transport()
{
	Project* project = pm().get_project();
	if (project) {
                TSession* session = project->get_current_session();
                if (session) {
                        return session->start_transport();
		}
	}
	
	return 0;
}

Command * TMainWindow::set_recordable_and_start_transport()
{
	if (m_project) {
                Sheet* sheet = m_project->get_active_sheet();
		if (sheet) {
			return sheet->set_recordable_and_start_transport();
		}
	}
	
	return 0;
}

// snapping is a global property and should be stored in each sheet
void TMainWindow::snap_state_changed(bool state)
{
	if (m_project) {
		QList<Sheet* > sheetlist = m_project->get_sheets();
		foreach( Sheet* sheet, sheetlist) {
			sheet->set_snapping(state);
		}
	}
}

void TMainWindow::update_snap_state()
{
	if (m_project) {
                bool snapping = m_project->get_current_session()->is_snap_on();
		m_snapAction->setChecked(snapping);
	}
}

// scrolling is a global property but should not be stored in the sheets
void TMainWindow::update_follow_state()
{
	m_isFollowing = config().get_property("PlayHead", "Follow", true).toBool();
	m_followAction->setChecked(m_isFollowing);
}

void TMainWindow::update_temp_follow_state(bool state)
{
        if (m_project->get_current_session()->is_transport_rolling() && m_isFollowing) {
		m_followAction->setChecked(state);
	}
}

void TMainWindow::follow_state_changed(bool state)
{
        Sheet* sheet = qobject_cast<Sheet*>(m_project->get_current_session());

	if (!sheet) {
		return;
	}
	
	if (!sheet->is_transport_rolling() || !m_isFollowing) {
		m_isFollowing = state;
		config().set_property("PlayHead", "Follow", state);
		config().save();
		if (sheet->is_transport_rolling()) {
			sheet->set_temp_follow_state(state);
		}
	} else {
		sheet->set_temp_follow_state(state);
	}
}

// the view mode is a sheet property and should be stored in the sheet
void TMainWindow::effect_state_changed(bool state)
{
        TSession* session = m_project->get_current_session();

	if (state) {
                session->set_effects_mode();
	} else {
                session->set_editing_mode();
	}
}

void TMainWindow::update_effects_state()
{
        TSession* session = m_project->get_current_session();

        if (!session) {
		return;
	}
	
        if (session->get_mode() == Sheet::EDIT) {
		m_effectAction->setChecked(false);
	} else {
		m_effectAction->setChecked(true);
	}
}

Command* TMainWindow::show_welcome_page()
{
        m_previousCenterAreaWidgetIndex = m_centerAreaWidget->currentIndex();
        m_centerAreaWidget->setCurrentIndex(0);

        return 0;
}

Command* TMainWindow::show_current_sheet()
{
        m_centerAreaWidget->setCurrentIndex(m_previousCenterAreaWidgetIndex);
        return 0;
}


Command* TMainWindow::show_track_finder()
{
        if (!m_project) {
                return 0;
        }

        m_trackFinder->setStyleSheet("color: blue;"
                                 "background-color: yellow;"
                                 "selection-color: yellow;"
                                 "selection-background-color: blue;");

        m_trackFinder->setText("Type to locate");
        m_trackFinder->selectAll();

        m_trackFinderModel->clear();

        QList<Sheet*> sheets = m_project->get_sheets();

        foreach(Sheet* sheet, sheets) {
                QList<Track*> tracks = sheet->get_tracks();
                tracks.append(sheet->get_master_out());
                tracks.append(m_project->get_master_out());
                foreach(Track* track, tracks) {
                        QStandardItem* sItem = new QStandardItem(track->get_name());
                        sItem->setData(track->get_id(), Qt::UserRole);
                        QList<QStandardItem*> items;
                        items.append(sItem);
                        sItem = new QStandardItem(sheet->get_name());
                        items.append(sItem);
                        m_trackFinderModel->appendRow(items);
                }
        }

        m_trackFinderTreeView->header()->setResizeMode(0, QHeaderView::Stretch);
        m_trackFinderTreeView->header()->setResizeMode(1, QHeaderView::ResizeToContents);

        m_trackFinder->setFocus();

        return 0;
}


void TMainWindow::track_finder_model_index_changed(const QModelIndex& index)
{
        qlonglong id = index.data(Qt::UserRole).toLongLong();

        foreach(SheetWidget* sw, m_sheetWidgets) {
                Sheet* sheet = qobject_cast<Sheet*>(sw->get_sheet());
                if (!sheet) return;
                Track* track = sheet->get_track(id);
                if (track) {
                        show_session(sheet);
                        sw->setFocus();
                        sw->get_sheetview()->browse_to_track(track);
                        break;
                }
        }
        track_finder_show_initial_text();
}

void TMainWindow::track_finder_return_pressed()
{
        if (!m_project) {
                return;
        }

        QString name = m_trackFinder->text();
        QList<QStandardItem*> items = m_trackFinderModel->findItems(name, Qt::MatchStartsWith);
        if (items.size()) {
                track_finder_model_index_changed(m_trackFinderModel->indexFromItem(items.at(0)));
                return;
        }
}


void TMainWindow::track_finder_show_initial_text()
{
        m_trackFinder->setStyleSheet("color: gray; background-color: white");
        m_trackFinder->setText(tr("Track Finder"));
}


Command* TMainWindow::browse_to_first_track_in_active_sheet()
{
        if (m_currentSheetWidget) {
                SheetView* sv = m_currentSheetWidget->get_sheetview();
                QList<TrackView*> tracks = sv->get_track_views();
                if (tracks.size()) {
                        sv->browse_to_track(tracks.first()->get_track());
                }
        }

        return 0;
}

Command* TMainWindow::browse_to_last_track_in_active_sheet()
{
        if (m_currentSheetWidget) {
                SheetView* sv = m_currentSheetWidget->get_sheetview();
                QList<TrackView*> tracks = sv->get_track_views();
                if (tracks.size()) {
                        sv->browse_to_track(tracks.last()->get_track());
                }
        }

        return 0;
}

void TMainWindow::register_vumeter_level(AbstractVUMeterLevel *level)
{
        m_vuLevels.append(level);
}

void TMainWindow::unregister_vumeter_level(AbstractVUMeterLevel *level)
{
        m_vuLevels.removeAll(level);
}

void TMainWindow::update_vu_levels_peak()
{
        if (!m_project) {
                return;
        }

        for(int i=0; i<m_vuLevels.size(); i++) {
                m_vuLevels.at(i)->update_peak();
        }

        QList<Track*> tracks = m_project->get_sheet_tracks();
        tracks.append(m_project->get_tracks());
        tracks.append(m_project->get_master_out());
        for(int i = 0; i< tracks.size(); i++) {
                VUMonitors monitors = tracks.at(i)->get_vumonitors();
                for (int j=0; j<monitors.size(); ++j) {
                        monitors.at(j)->set_read();
                }
        }
}


void TMainWindow::reset_vu_levels_peak_hold_value()
{
        for(int i=0; i<m_vuLevels.size(); i++) {
                m_vuLevels.at(i)->reset_peak_hold_value();
        }
}
