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
#include "libtraversosongcanvas.h"
#include <AudioDevice.h> 

#include <QDockWidget>
#include <QUndoView>
#include <QFile>
#include <QDir>
#include <QMenuBar>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTextStream>

#include "Interface.h"
#include "BusMonitor.h"
#include "ProjectManager.h"
#include "ViewPort.h"
#include "FadeCurve.h"
#include "Config.h"
#include "Plugin.h"
#include "ExportWidget.h"
#include "CorrelationMeterWidget.h"
#include "SpectralMeterWidget.h"
		
#include "Import.h"

#include "songcanvas/SongWidget.h"

#include "ui_QuickStart.h"

#include "widgets/InfoWidgets.h"
#include "widgets/ResourcesWidget.h"

#include "dialogs/settings/SettingsDialog.h"
#include "dialogs/project/ProjectManagerDialog.h"
#include "dialogs/project/OpenProjectDialog.h"
#include "dialogs/project/NewProjectDialog.h"
#include "dialogs/project/NewSongDialog.h"
#include "dialogs/project/NewTrackDialog.h"
#include "dialogs/MarkerDialog.h"
#include "dialogs/BusSelectorDialog.h"
#include "dialogs/InsertSilenceDialog.h"


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


Interface* Interface::m_instance = 0;

Interface* Interface::instance()
{
	if (m_instance == 0) {
		m_instance = new Interface();
	}

	return m_instance;
}

Interface::Interface()
	: QMainWindow( 0 )
{
	PENTERCONS;
	setWindowTitle("Traverso");
	setMinimumSize(400, 300);
	setWindowIcon(QPixmap (":/windowicon") );
	//         setMaximumWidth(1024);
	//         setMaximumHeight(768);

	// CenterAreaWidget
	centerAreaWidget = new QStackedWidget(this);
	setCentralWidget(centerAreaWidget);
	
	// HistoryView 
	historyDW = new QDockWidget(tr("History"), this);
	historyDW->setObjectName("HistoryDockWidget");
	historyWidget = new HistoryWidget(pm().get_undogroup(), historyDW);
	historyWidget->setFocusPolicy(Qt::NoFocus);
	historyDW->setWidget(historyWidget);
	addDockWidget(Qt::RightDockWidgetArea, historyDW);
	
	// AudioSources View
	AudioSourcesDW = new QDockWidget(tr("Resources Bin"), this);
	AudioSourcesDW->setObjectName("AudioSourcesDockWidget");
	audiosourcesview = new ResourcesWidget(AudioSourcesDW);
	audiosourcesview->setFocusPolicy(Qt::NoFocus);
	AudioSourcesDW->setWidget(audiosourcesview);
	addDockWidget(Qt::TopDockWidgetArea, AudioSourcesDW);
	AudioSourcesDW->hide();
	
	// Meter Widgets
	correlationMeterDW = new QDockWidget(tr("Correlation Meter"), this);
	correlationMeterDW->setObjectName("CorrelationMeterDockWidget");
	correlationMeter = new CorrelationMeterWidget(correlationMeterDW);
	correlationMeter->setFocusPolicy(Qt::NoFocus);
	correlationMeterDW->setWidget(correlationMeter);
	addDockWidget(Qt::TopDockWidgetArea, correlationMeterDW);
	correlationMeterDW->hide();

	spectralMeterDW = new QDockWidget(tr("FFT Spectrum"), this);
	spectralMeterDW->setObjectName("SpectralMeterDockWidget");
	spectralMeter = new SpectralMeterWidget(spectralMeterDW);
	spectralMeter->setFocusPolicy(Qt::NoFocus);
	spectralMeterDW->setWidget(spectralMeter);
	addDockWidget(Qt::TopDockWidgetArea, spectralMeterDW);
	spectralMeterDW->hide();

	// BusMonitor
	busMonitorDW = new QDockWidget("VU Meters", this);
	busMonitorDW->setObjectName("VU Meters");

	busMonitor = new BusMonitor(busMonitorDW);
	busMonitorDW->setWidget(busMonitor);
	addDockWidget(Qt::RightDockWidgetArea, busMonitorDW);
	
	m_infoBar = new InfoToolBar(this);
	addToolBar(m_infoBar);
	
	m_sysinfo = new SysInfoToolBar(this);
	addToolBar(Qt::BottomToolBarArea, m_sysinfo);
	
	
	// Some default values.
	currentSongWidget = 0;
	exportWidget = 0;
	m_settingsdialog = 0;
	m_projectManagerDialog = 0;
	m_openProjectDialog = 0;
	m_newProjectDialog = 0;
	m_insertSilenceDialog = 0;
	m_markerDialog = 0;
	m_busSelector = 0;
	m_newSongDialog = 0;
	m_newTrackDialog = 0;
	m_quickStart = 0;
	
	create_menus();
	
	/** Read in the Interface settings and apply them
	 */
	resize(config().get_property("Interface", "size", QSize(900, 600)).toSize());
	move(config().get_property("Interface", "pos", QPoint(200, 200)).toPoint());
	restoreState(config().get_property("Interface", "windowstate", "").toByteArray());

	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(&pm(), SIGNAL(aboutToDelete(Song*)), this, SLOT(delete_songwidget(Song*)));
	connect(&pm(), SIGNAL(unsupportedProjectDirChangeDetected()), this, SLOT(project_dir_change_detected()));	

	cpointer().add_contextitem(this);

	connect(&config(), SIGNAL(configChanged()), this, SLOT(update_opengl()));
}

Interface::~Interface()
{
	PENTERDES;
	config().set_property("Interface", "size", size());
	config().set_property("Interface", "fullScreen", isFullScreen());
	config().set_property("Interface", "pos", pos());
	config().set_property("Interface", "windowstate", saveState());
}


void Interface::set_project(Project* project)
{
	PENTER;
	
	if ( project ) {
		connect(project, SIGNAL(currentSongChanged(Song*)), this, SLOT(show_song(Song*)));
		setWindowTitle(project->get_title() + " - Traverso");
		m_projectSaveAction->setEnabled(true);
		m_projectSongManagerAction->setEnabled(true);
		m_projectExportAction->setEnabled(true);
		m_songMenuAction->setEnabled(true);

		// the project's songs will be deleted _after_
		// the project has been deleted, which will happen after this
		// function returns. When the songs have been disconnected from the
		// audiodevice, delete_songwidget(Song* song) is called for all the songs
		// in the project. Meanwhile, disable updates of the SongWidgets (and implicitily
		// all their childrens) to avoid the (unlikely) situation of a paint event that 
		// refers to data that was part of the then deleted project!
		// The reason to not delete the SongWidgets right now is that the newly loaded project
		// now will be able to create and show it's songcanvas first, which improves the 
		// users experience a lot!
		foreach(SongWidget* sw, m_songWidgets) {
			sw->setUpdatesEnabled(false);
		}
	} else {
		m_projectSaveAction->setEnabled(false);
		m_projectSongManagerAction->setEnabled(false);
		m_projectExportAction->setEnabled(false);
		m_songMenuAction->setEnabled(false);
		setWindowTitle("Traverso");
		// No project loaded, the currently  loaded project will be deleted after this
		// function returns, if the songcanvas is still painting (due playback e.g.) we
		// could get a crash due canvas items refering to data that was managed by the project.
		// so let's delete the SongWidgets before the project is deleted!
		foreach(SongWidget* sw, m_songWidgets) {
			delete_songwidget(sw->get_song());
		}
	}
}

void Interface::delete_songwidget(Song* song)
{
	SongWidget* sw = m_songWidgets.value(song);
	if (sw) {
		m_songWidgets.remove(song);
		centerAreaWidget->removeWidget(sw);
		delete sw;
	}
}


void Interface::show_song(Song* song)
{
	PENTER;
	if (!song) {
		return;
	}
	
	SongWidget* songWidget = m_songWidgets.value(song);
	
	if (!songWidget) {
		songWidget = new SongWidget(song, centerAreaWidget);
		centerAreaWidget->addWidget(songWidget);
		m_songWidgets.insert(song, songWidget);
	}
	currentSongWidget = songWidget;
	centerAreaWidget->setCurrentIndex(centerAreaWidget->indexOf(songWidget));
	songWidget->setFocus();
	pm().get_undogroup()->setActiveStack(song->get_history_stack());
}


Command* Interface::about_traverso()
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

Command* Interface::quick_start()
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

Command* Interface::full_screen()
{
	if (isFullScreen())
		showNormal();
	else
		showFullScreen();
	return (Command*) 0;
}

void Interface::keyPressEvent( QKeyEvent * e)
{
	ie().catch_key_press(e);
	e->ignore();
}

void Interface::keyReleaseEvent( QKeyEvent * e)
{
	ie().catch_key_release(e);
	e->ignore();
}

Command * Interface::show_export_widget( )
{
	if (! exportWidget) {
		exportWidget = new ExportWidget(this);
	}
	if (exportWidget->isHidden()) {
		exportWidget->set_was_closed();
		exportWidget->show();
	}
	return (Command*) 0;
}

void Interface::create_menus( )
{
	QMenu* menu;
	QAction* action;
	 
	menu = menuBar()->addMenu(tr("&Project"));
	
	action = menu->addAction(tr("&New..."));
	action->setIcon(find_pixmap(":/new-16"));
	action->setShortcuts(QKeySequence::New);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(show_newproject_dialog()));
	
	action = menu->addAction(tr("&Open..."));
	action->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
	action->setShortcuts(QKeySequence::Open);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(show_open_project_dialog()));
	
	menu->addSeparator();
	
	action = menu->addAction(tr("&Save"));
	action->setShortcuts(QKeySequence::Save);
	m_projectSaveAction = action;
	action->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	connect(action, SIGNAL(triggered(bool)), &pm(), SLOT(save_project()));
	
	action = menu->addAction(tr("&Manage Project..."));
	QList<QKeySequence> list;
	list.append(QKeySequence("F4"));
	action->setShortcuts(list);
	action->setIcon(QIcon(find_pixmap(":/songmanager-16")));
	m_projectSongManagerAction = action;
	connect(action, SIGNAL(triggered(bool)), this, SLOT(show_project_manager_dialog()));
	
	action = menu->addAction(tr("&Export..."));
	list.clear();
	list.append(QKeySequence("F9"));
	action->setShortcuts(list);
	action->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
	m_projectExportAction = action;
	connect(action, SIGNAL(triggered(bool)), this, SLOT(show_export_widget()));
	
	menu->addSeparator();
	
	action = menu->addAction(tr("&Quit"));
	list.clear();
	list.append(QKeySequence("CTRL+Q"));
	action->setShortcuts(list);
	action->setIcon(QIcon(find_pixmap(":/exit-16")));
	connect(action, SIGNAL(triggered( bool )), &pm(), SLOT(exit()));
	
	
	menu = menuBar()->addMenu(tr("&Sheet"));
	m_songMenuAction = menu->menuAction();
	
	action = menu->addAction(tr("New &Track(s)..."));
	connect(action, SIGNAL(triggered()), this, SLOT(show_newtrack_dialog()));
	action = menu->addAction(tr("New &Sheet(s)..."));
	connect(action, SIGNAL(triggered()), this, SLOT(show_newsong_dialog()));

	menu->addSeparator();
	
	action = menu->addAction(tr("Marker Editor..."));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(show_marker_dialog()));
	
	menu->addSeparator();

	action = menu->addAction(tr("Import &Audio..."));
	connect(action, SIGNAL(triggered()), this, SLOT(import_audio()));
	action = menu->addAction(tr("Insert Si&lence..."));
	connect(action, SIGNAL(triggered()), this, SLOT(show_insertsilence_dialog()));
	
	
	menu = menuBar()->addMenu(tr("&View"));

	menu->addAction(historyDW->toggleViewAction());
	menu->addAction(busMonitorDW->toggleViewAction());
	menu->addAction(AudioSourcesDW->toggleViewAction());
	
	menu->addSeparator();
	
	menu->addAction(correlationMeterDW->toggleViewAction());
	menu->addAction(spectralMeterDW->toggleViewAction());
	
	menu->addSeparator();
	
	menu->addAction(m_infoBar->toggleViewAction());
	m_infoBar->toggleViewAction()->setText(tr("Sheet Toolbar"));
	menu->addAction(m_sysinfo->toggleViewAction());
	m_sysinfo->toggleViewAction()->setText(tr("System Information"));
	
	
	menu = menuBar()->addMenu(tr("Se&ttings"));
	menu->addAction(m_infoBar->get_snap_action());
	menu->addAction(m_infoBar->get_follow_action());
	
	menu->addSeparator();

	action = menu->addAction(tr("&Preferences..."));
	connect(action, SIGNAL(triggered( bool )), this, SLOT(show_settings_dialog()));
	
	
	menu = menuBar()->addMenu("&Help");
	action = menu->addAction(tr("&Getting Started"));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(quick_start()));
	
	action = menu->addAction(tr("&User Manual"));
	action->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(open_help_browser()));
	
	action = menu->addAction(tr("&About Traverso"));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(about_traverso()));
}

void Interface::process_context_menu_action( QAction * action )
{
	QStringList strings = action->data().toStringList();
	QString name = strings.first();
	ie().broadcast_action_from_contextmenu(name);
}

Command * Interface::show_context_menu( )
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

	QMenu* toplevelmenu;
			
	for (int i=0; i<items.size(); ++i) {
		QObject* item = items.at(i);
		
		QString className = item->metaObject()->className();
		
		if (i==0) {
			toplevelmenu = m_contextMenus.value(className);
		
			if ( ! toplevelmenu ) {
				printf("No menu for %s, creating new one\n", QS_C(className));
				toplevelmenu = create_context_menu(item);
				if (! toplevelmenu ) {
					if (items.size() > 1) {
						toplevelmenu = new QMenu();
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
			QAction* action = toplevelmenu->insertMenu(action, menu);
			QString name = className.remove("View");

			if (name == "Song") name = "Sheet"; // FIXME!!!

			action->setText(name);
		}
	}
	
	toplevelmenu->exec(QCursor::pos());
	
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

Command * Interface::export_keymap()
{
	QTextStream out;
	QFile data(QDir::homePath() + "/traversokeymap.html");
	if (data.open(QFile::WriteOnly | QFile::Truncate)) {
		out.setDevice(&data);
	} else {
		return 0;
	}
	
	QMap<QString, QList<const QMetaObject*> > objects;
	
	QList<const QMetaObject*> songlist; songlist << &Song::staticMetaObject; songlist << &SongView::staticMetaObject;
	QList<const QMetaObject*> tracklist; tracklist << &Track::staticMetaObject; tracklist << &TrackView::staticMetaObject;
	QList<const QMetaObject*> cliplist; cliplist << &AudioClip::staticMetaObject; cliplist << &AudioClipView::staticMetaObject;
	QList<const QMetaObject*> curvelist; curvelist << &Curve::staticMetaObject; curvelist << &CurveView::staticMetaObject;
	QList<const QMetaObject*> timelinelist; timelinelist << &TimeLine::staticMetaObject; timelinelist << &TimeLineView::staticMetaObject;
	QList<const QMetaObject*> markerlist; markerlist << &Marker::staticMetaObject; markerlist << &MarkerView::staticMetaObject;
	QList<const QMetaObject*> pluginlist; pluginlist << &Plugin::staticMetaObject; pluginlist << &PluginView::staticMetaObject;
	QList<const QMetaObject*> fadelist; fadelist << &FadeCurve::staticMetaObject; fadelist << &FadeView::staticMetaObject;
	QList<const QMetaObject*> interfacelist; interfacelist << &Interface::staticMetaObject;
	QList<const QMetaObject*> pmlist; pmlist << &ProjectManager::staticMetaObject;
		
	objects.insert("Sheet", songlist);
	objects.insert("Track", tracklist);
	objects.insert("AudioClip", cliplist);
	objects.insert("Curve", curvelist);
	objects.insert("TimeLine", timelinelist);
	objects.insert("Marker", markerlist);
	objects.insert("Plugin", pluginlist);
	objects.insert("Fade", fadelist);
	objects.insert("Interface", interfacelist);
	objects.insert("ProjectManager", pmlist);
	
	
	out << "<html><body><h1>Traverso keymap: " << config().get_property("CCE", "keymap", "default").toString() << "</h1>";
	
	foreach(QList<const QMetaObject* > objectlist, objects.values()) {
		QString name = objects.key(objectlist);
		
		out << "<h3>" << name << "</h3>";
		out << "<table><tr><td width=220>" << tr("<b>Description</b>") << "</td><td>" << tr("<b>Key Sequence</b>") << "</td></tr>";
		
		QStringList result;
		
		foreach(const QMetaObject* mo, objectlist) {
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
									
							result += QString("<tr><td>") + submenuname + strings.at(1) + "</td><td>" + keyfact + "</td></tr>";
						}
					}
				}
				delete menu;
			}
		}
		result.sort();
		out << result.join("");
		out << "</table></br></br>";
	}
	
	out << "</body></html>";
	
	data.close();
	
	return 0;
}

QMenu* Interface::create_context_menu(QObject* item, QList<MenuData >* menulist)
{
	QMenu* menu = new QMenu(this);
	
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
		 name = QString(item->metaObject()->className()).remove("View").remove("Panel");
	} else {
		name = "noname";
	}

	if (name == "Song") name = "Sheet"; // FIXME!!!

	QAction* menuAction = menu->addAction(name);
	QFont font(themer()->get_font("ContextMenu:fontscale:actions"));
	font.setBold(true);
	menuAction->setFont(font);
	menuAction->setEnabled(false);
	menu->addSeparator();
	menu->setFont(themer()->get_font("ContextMenu:fontscale:actions"));
	
	QHash<QString, QList<MenuData>* > submenus;
	
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
	foreach(QString key, keys) {
		QList<MenuData>* list = submenus.value(key);
		
		qSort(list->begin(), list->end(), MenuData::smaller);

		QMenu* sub = new QMenu(this);
		sub->setFont(themer()->get_font("ContextMenu:fontscale:actions"));
		
		QFont font(themer()->get_font("ContextMenu:fontscale:actions"));
		font.setBold(true);
		sub->menuAction()->setFont(font);
		
		QAction* action = menu->insertMenu(0, sub);
		action->setText(key);
		foreach(MenuData data, *list) {
			QAction* action = new QAction(sub);
			QString keyfact = create_keyfact_string(data.keysequence, data.modifierkeys);
			QString text = QString(data.description + "  " + keyfact);
			action->setText(text);
			QStringList strings;
			strings << data.iedata << data.description << keyfact;
			action->setData(strings);
			sub->addAction(action);
		}
		
		delete list;
	}
	
	return menu;
}

void Interface::show_busselector(Track* track)
{
	if (! m_busSelector) {
		m_busSelector = new BusSelectorDialog(this);
	}
	m_busSelector->set_current_track(track);	
	m_busSelector->show();
}

void Interface::set_insertsilence_track(Track* track)
{
	if (m_insertSilenceDialog) {
		m_insertSilenceDialog->setTrack(track);
	}
}

void Interface::select_fade_in_shape( )
{
	QMenu* menu = m_contextMenus.value("fadeInSelector");
	
	if (!menu) {
		menu = create_fade_selector_menu("fadeInSelector");
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(set_fade_in_shape(QAction*)));
	}
		
	
	menu->exec(QCursor::pos());
}

void Interface::select_fade_out_shape( )
{
	QMenu* menu = m_contextMenus.value("fadeOutSelector");
	
	if (!menu) {
		menu = create_fade_selector_menu("fadeOutSelector");
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(set_fade_out_shape(QAction*)));
	}
		
	
	menu->exec(QCursor::pos());
}


void Interface::set_fade_in_shape( QAction * action )
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

void Interface::set_fade_out_shape( QAction * action )
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


QMenu* Interface::create_fade_selector_menu(const QString& fadeTypeName)
{
	QMenu* menu = new QMenu();
	
	foreach(QString name, FadeCurve::defaultShapes) {
		QAction* action = menu->addAction(name);
		action->setData(name);
	}
	
	m_contextMenus.insert(fadeTypeName, menu);
	
	return menu;
}

void Interface::update_opengl()
{
	bool toggled = config().get_property("Interface", "OpenGL", false).toBool();

	foreach(SongWidget* widget, m_songWidgets) {
		widget->set_use_opengl(toggled);
	}
}

void Interface::import_audio()
{
	if (currentSongWidget->get_song()->get_numtracks() > 0) {
		QList<Track* > tracks = currentSongWidget->get_song()->get_tracks();
		Track*	shortestTrack = tracks.at(0);

		for (int i=1; i<tracks.size(); i++) {
			if (tracks.at(i)->get_cliplist().get_last() && tracks.at(i)->get_cliplist().get_last()->get_track_end_frame() > shortestTrack->get_cliplist().get_last()->get_track_end_frame()) {
				shortestTrack = tracks.at(i);
			}
		}

		Import* cmd = new Import(shortestTrack);
		Command::process_command(cmd);
	}
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


void Interface::show_settings_dialog()
{
	if (!m_settingsdialog) {
		m_settingsdialog = new SettingsDialog(this);
	}
	
	m_settingsdialog->show();
}


void Interface::closeEvent(QCloseEvent * event)
{
	event->ignore();
	pm().exit();
}

Command* Interface::show_project_manager_dialog()
{
	if (! m_projectManagerDialog) {
		m_projectManagerDialog = new ProjectManagerDialog(this);
	}
	m_projectManagerDialog->show();
	return 0;
}

Command* Interface::show_open_project_dialog()
{
	if (!m_openProjectDialog) {
		m_openProjectDialog = new OpenProjectDialog(this);
	}
	m_openProjectDialog->show();
	return 0;
}

Command * Interface::show_newproject_dialog()
{
	if (! m_newProjectDialog ) {
		m_newProjectDialog = new NewProjectDialog(this);
	}
	m_newProjectDialog->show();
	return 0;
}

Command * Interface::show_insertsilence_dialog()
{
	if (! m_insertSilenceDialog) {
		m_insertSilenceDialog = new InsertSilenceDialog(this);
	}
	
	m_insertSilenceDialog->setTrack(0);
	m_insertSilenceDialog->focusInput();
	m_insertSilenceDialog->show();
	
	return 0;
}


Command * Interface::show_marker_dialog()
{
	if (! m_markerDialog ) {
		m_markerDialog = new MarkerDialog(this);
	}
	m_markerDialog->show();
	
	return 0;
}

QSize Interface::sizeHint() const
{
	return QSize(800, 600);
}

Command* Interface::show_newsong_dialog()
{
	if (! m_newSongDialog) {
		m_newSongDialog = new NewSongDialog(this);
	}
	
	m_newSongDialog->show();
	
	return 0;
}

Command* Interface::show_newtrack_dialog()
{
	if (! m_newTrackDialog) {
		m_newTrackDialog = new NewTrackDialog(this);
	}
	
	m_newTrackDialog->show();
	
	return 0;
}


void Interface::open_help_browser()
{
	info().information(tr("Opening User Manual in external browser!"));
	QDesktopServices::openUrl(QUrl("http://traverso-daw.org/UserManual"));
}

void Interface::project_dir_change_detected()
{
	QMessageBox::critical(this, tr("Traverso - Important"), 
			      tr("A Project directory changed outside of Traverso. \n\n"
			      "This is NOT supported! Please undo this change now!\n\n"
			      "If you want to rename a Project, use the Project Manager instead!"),
	   			QMessageBox::Ok);
}

