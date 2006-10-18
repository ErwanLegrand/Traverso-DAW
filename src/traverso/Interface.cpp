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

$Id: Interface.cpp,v 1.14 2006/10/18 12:08:56 r_sijrier Exp $
*/

#include "../config.h"

#include <libtraversocore.h>

#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QtGui>
#include <QTreeView>

#include "Interface.h"
#include "BusMonitor.h"
#include "ProjectManager.h"
#include "InfoBox.h"
#include "BorderLayout.h"
#include "AudioClipView.h"
#include "SongView.h"
#include "TrackView.h"
#include "ViewPort.h"
#include "OverViewWidget.h"
#include "Help.h"
#include "HistoryWidget.h"
#include "AudioSourcesTreeWidget.h"
#include <FadeCurve.h>
#include "QuickDriverConfigWidget.h"
#include "SystemInfoWidget.h"

#include "ManagerWidget.h"
#include "ExportWidget.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const int TOPPANEL_FIXED_HEIGHT = 140;
static const int STATUS_CONTAINER_FIXED_HEIGHT = 20;
static const int MINIMUM_BUS_MONITOR_WIDTH = 80;
static const int MINIMUM_BUS_MONITOR_HEIGHT = 90;
static const int MINIMUM_FLOATING_BUS_MONITOR_HEIGHT = 120;


Interface::Interface()
	: QMainWindow( 0 )
{
	PENTERCONS;

	setMinimumSize(150, 100);
	
	setWindowIcon(QPixmap (":/windowicon") );

	//         setMaximumWidth(1024);
	//         setMaximumHeight(768);

	exportWidget = 0;

	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));

	cpointer().add_contextitem(this);
	
	create();
}

Interface::~Interface()
{
	PENTERDES;
	QSettings settings;
	settings.beginGroup("Interface");
	settings.setValue("size", size());
	settings.setValue("fullScreen", isFullScreen());
	settings.setValue("pos", pos());
	settings.setValue("windowstate", saveState());
	settings.endGroup();
	
	delete helpWindow;
}

void Interface::create()
{
	driverConfigWidget = 0;
	
	QWidget* centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);
	
	setWindowTitle("Traverso");

	helpWindow = new Help(this);

	mainVBoxLayout = new BorderLayout(centralWidget, 0, 0);

	tpdw = new QDockWidget("Master VU", this);
	tpdw->setObjectName("Master VU");
// 	tpdw->setFeatures(QDockWidget::NoDockWidgetFeatures);
// 	topPanelWidget = new QWidget(tpdw);
// 	topPanelWidgetLayout = new QHBoxLayout(topPanelWidget);
// 	topPanelWidget->setLayout(topPanelWidgetLayout);
// 	topPanelWidget->setMinimumHeight(TOPPANEL_FIXED_HEIGHT);
// 	topPanelWidget->setMaximumHeight(TOPPANEL_FIXED_HEIGHT);
// 	tpdw->setWidget(topPanelWidget);
// 
// 	infoBox = new InfoBox(topPanelWidget);
// 	topPanelWidgetLayout->insertWidget( 0, infoBox, 4);

	busMonitor = new BusMonitor(tpdw, this);
// 	topPanelWidgetLayout->insertWidget( 1, busMonitor);
// 	busMonitor->show();
	tpdw->setWidget(busMonitor);
	addDockWidget(Qt::TopDockWidgetArea, tpdw);


	// 2nd Area (MIDDLE) the current project
	QWidget* widget = new QWidget(centralWidget);
	centerWidgetLayout = new QHBoxLayout(widget);
	widget->setLayout(centerWidgetLayout);
	centerAreaWidget = new QStackedWidget(widget);
	centerWidgetLayout->addWidget(centerAreaWidget);


	// 3rd Area (BOTTOM) : Status stuff
	statusAreaWidget = new QWidget(centralWidget);
	statusAreaWidgetLayout = new QHBoxLayout;
	statusAreaWidget->setLayout(statusAreaWidgetLayout);

	
	overView = new OverViewWidget(statusAreaWidget);
	statusAreaWidgetLayout->addWidget(overView, 4);

// 	topPanelWidgetLayout->setMargin(0);
	centerWidgetLayout->setMargin(0);
	statusAreaWidgetLayout->setMargin(3);

// 	mainVBoxLayout->addWidget(topPanelWidget, BorderLayout::North);
	mainVBoxLayout->addWidget(widget, BorderLayout::Center);
	mainVBoxLayout->addWidget(statusAreaWidget, BorderLayout::South);
	
	centralWidget->setLayout(mainVBoxLayout);

	managerWidgetCreated = false;

	
	hvdw = new QDockWidget(tr("History"), this);
	hvdw->setObjectName("HistoryDockWidget");
	historyWidget = new HistoryWidget(hvdw);
	historyWidget->setFocusPolicy(Qt::NoFocus);
	hvdw->setWidget(historyWidget);
	addDockWidget(Qt::RightDockWidgetArea, hvdw);
	
	asdw = new QDockWidget(tr("AudioSources"), this);
	asdw->setObjectName("AudioSourcesDockWidget");
	audiosourcesview = new QTreeView(asdw);
	audiosourcesview->setFocusPolicy(Qt::NoFocus);
// 	audiosourcesview->setAnimated(true);
	TreeModel* model = new TreeModel("hoi, test");
	audiosourcesview->setModel(model);
	asdw->setWidget(audiosourcesview);
	addDockWidget(Qt::RightDockWidgetArea, asdw);
	

	create_menus();

	
	/** Read in the Interface settings and apply them
	 */
	QSettings settings;
	settings.beginGroup("Interface");
	resize(settings.value("size", QSize(400, 400)).toSize());
	move(settings.value("pos", QPoint(200, 200)).toPoint());
	restoreState(settings.value("windowstate", "").toByteArray());
	settings.endGroup();
}


void Interface::set_project(Project* project)
{
	PENTER;
	
	if ( project ) {
		connect(project, SIGNAL(currentSongChanged(Song*)), this, SLOT(set_songview(Song*)));
		connect(project, SIGNAL(currentSongChanged(Song*)), overView, SLOT(set_song(Song*)));
		connect(project, SIGNAL(newSongCreated(Song*)), this, SLOT(create_songview(Song*)));
		
	}
	
	songViewList.clear();
	
	// OK, a new Project is created. Remove and delete all the ViewPorts related to this project
	while ( ! currentProjectViewPortList.isEmpty()) {
		ViewPort* view = currentProjectViewPortList.takeFirst();
		centerAreaWidget->removeWidget(view);
		delete view;
	}
	
	currentSongView = 0;
}

void Interface::set_songview(Song* song)
{
	PENTER;
	SongView* sv;
	for (int i=0; i<songViewList.size(); ++i) {
		sv = songViewList.at(i);
		if(sv->get_song() == song) {
			PMESG("Setting new songView");
			UndoGroup::instance()->set_active_stack(song->get_history_stack());
			centerAreaWidget->setCurrentWidget(sv->get_viewport());
			currentSongView = sv;
			break;
		}
	}
}

Command* Interface::set_manager_widget()
{
	if (!managerWidgetCreated) {
		managerWidget = new ManagerWidget(centerAreaWidget);
		managerWidgetCreated = true;
		centerAreaWidget->addWidget(managerWidget);
	}
	centerAreaWidget->setCurrentWidget(managerWidget);
	return (Command*) 0;
}

Command* Interface::set_songview_widget()
{
	if (currentSongView) {
		centerAreaWidget->setCurrentWidget(currentSongView->get_viewport());
		
		// In some circumstances the focus of the keyboard is still on the 
		// project manager widget :-( So we "steal" the focus explicitely!
		currentSongView->get_viewport()->setFocus();
	}
	
	return (Command*) 0;
}

void Interface::create_songview(Song* song)
{
	PENTER;
	ViewPort* vp = new ViewPort(centerAreaWidget);
	centerAreaWidget->addWidget(vp);
	SongView* sv = new SongView(song, vp);
	songViewList.append(sv);
	currentProjectViewPortList.append(vp);
}

void Interface::resizeEvent(QResizeEvent* )
{
	PENTER2;
}


Command* Interface::about_traverso()
{
	PENTER;
	QString text(tr("Traverso %1, making use of Qt %2\n\n" 
			"Traverso, a Multitrack audio recording and editing program.\n\n "
			"Traverso uses a very powerfull interface concept, which makes recording\n"
			"and editing audio much quicker and a pleasure to do!\n"
			"See for more info the Help file\n\n"
			"Traverso is brought to you by the author, R. Sijrier, and all the people from Free Software world\n"
			"who made important technologies on which Traverso is based (Gcc, Qt, Xorg, Linux, and so on)").arg(VERSION).arg(QT_VERSION_STR));
	QMessageBox::about ( this, tr("About Traverso"), text);
	
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
	if (!e->isAutoRepeat())
		ie().catch_press(e);
	e->ignore();
}

void Interface::keyReleaseEvent( QKeyEvent * e)
{
	if (!e->isAutoRepeat())
		ie().catch_release(e);
	e->ignore();
}

void Interface::wheelEvent( QWheelEvent * e )
{
	ie().catch_scroll(e);
	e->ignore();
}

Command * Interface::show_export_widget( )
{
	if (! exportWidget)
		exportWidget = new ExportWidget(this);
	exportWidget->show();
	return (Command*) 0;
}

void Interface::create_menus( )
{
	saveAction =  new QAction(tr("&Save"), this);
	saveAction->setIcon(QIcon("/usr/share/icons/crystalsvg/22x22/actions/filesave.png"));
	connect(saveAction, SIGNAL(triggered()), &pm(), SLOT(save_project()));
	
	exitAction = new QAction(tr("&Quit"), this);
	exitAction->setIcon(QIcon("/usr/share/icons/crystalsvg/22x22/actions/exit.png"));
	connect(exitAction, SIGNAL(triggered()), &pm(), SLOT(exit()));
	
	editViewAction = new QAction(tr("&Edit View"), this);
	connect(editViewAction, SIGNAL(triggered()), this, SLOT(set_songview_widget()));
	
	curveViewAction = new QAction(tr("&Curve View"), this);
	connect(curveViewAction, SIGNAL(triggered()), this, SLOT(set_songview_widget()));
	
	projManViewAction = new QAction(tr("&Project Management"), this);
	connect(projManViewAction, SIGNAL(triggered()), this, SLOT(set_manager_widget()));
	
	settingsViewAction = new QAction(tr("&Settings"), this);
	connect(settingsViewAction, SIGNAL(triggered()), this, SLOT(set_manager_widget()));
	
	
	handBookAction = new QAction(tr("&HandBook"), this);
	handBookAction->setIcon(QIcon("/usr/share/icons/crystalsvg/22x22/actions/help.png"));
	connect(handBookAction, SIGNAL(triggered()), helpWindow, SLOT(show_help()));
	
	aboutTraversoAction = new QAction(tr("&About Traverso"), this);
	connect(aboutTraversoAction,  SIGNAL(triggered()), this, SLOT(about_traverso()));
	
	
/*	fileMenu = menuBar()->addMenu(tr("&File"));
	viewMenu = menuBar()->addMenu(tr("&Views"));
	helpMenu = menuBar()->addMenu(tr("&Help"));
	
	
	
	fileMenu->addAction(exitAction);
	
	QAction* driverConfigAction = menuBar()->addAction(tr("Driver"));
// 	driverConfigAction->setIcon(QPixmap(":/driver"));
	driverConfigAction->setText(audiodevice().get_device_name());
	connect(driverConfigAction, SIGNAL(triggered()), this, SLOT(show_driver_config_widget()));
	*/
	
	
	fileMenu = new QMenu("File");
	fileMenu->addAction(saveAction);
	fileMenu->addAction(exitAction);
	
	viewMenu = new QMenu("Views");
	QAction* mvAction = viewMenu->addAction(tr("Manager View"));
	connect(mvAction, SIGNAL(triggered()), this, SLOT(set_manager_widget()));

	QAction* svAction = viewMenu->addAction(tr("Song View"));
	connect(svAction, SIGNAL(triggered()), this, SLOT(set_songview_widget()));
	viewMenu->addSeparator();
	viewMenu->addAction(hvdw->toggleViewAction());
	viewMenu->addAction(tpdw->toggleViewAction());
	viewMenu->addAction(asdw->toggleViewAction());
	
	helpMenu = new QMenu("Help");
	helpMenu->addAction(handBookAction);
	helpMenu->addAction(aboutTraversoAction);
	
	
	mainToolBar = addToolBar(tr("MainToolBar"));
	mainToolBar->setMovable(false);
	mainToolBar->setObjectName("MainToolBar");
	mainToolBar->setFocusPolicy(Qt::NoFocus);

	
	QPushButton* tbutton = new QPushButton(mainToolBar);
	QFontMetrics fm = tbutton->fontMetrics();
	QString text = tr("File");
// 	utton->setPopupMode(QToolButton::InstantPopup);
	tbutton->setText(text);
	tbutton->setMaximumWidth(fm.width(text) + 32);
	tbutton->setFlat(true);
	tbutton->setMenu(fileMenu);
	mainToolBar->addWidget(tbutton);
// 	tbutton->setFocusPolicy(Qt::NoFocus);
	
	tbutton = new QPushButton(mainToolBar);
// 	tbutton->setPopupMode(QToolButton::InstantPopup);
	text = tr("Views");
	tbutton->setText(text);
	tbutton->setMaximumWidth(fm.width(text) + 32);
	tbutton->setFlat(true);
	tbutton->setMenu(viewMenu);
	mainToolBar->addWidget(tbutton);
// 	tbutton->setFocusPolicy(Qt::NoFocus);
	
	
	QToolButton* button = new QToolButton(mainToolBar);
	button->setIcon(find_pixmap(":/projectmanagement-22"));
	button->setMinimumWidth(44);
	button->setToolTip("Show Manager View");
	mainToolBar->addWidget(button);
	button->setFocusPolicy(Qt::NoFocus);
	connect(button, SIGNAL(clicked()), this, SLOT(set_manager_widget()));
	

	button = new QToolButton(mainToolBar);
	button->setIcon(find_pixmap(":/songview-22"));
	button->setMinimumWidth(44);
	button->setToolTip("Show Song View");
	mainToolBar->addWidget(button);
	button->setFocusPolicy(Qt::NoFocus);
	connect(button, SIGNAL(clicked()), this, SLOT(set_songview_widget()));
	
	
	mainToolBar->addSeparator();
	
	driverInfo = new DriverInfoWidget(mainToolBar);
	driverInfo->setFlat(true);
	driverInfo->setFocusPolicy(Qt::NoFocus);
	
	mainToolBar->addWidget(driverInfo);
	connect(driverInfo, SIGNAL(clicked()), this, SLOT(show_driver_config_widget()));
	
	
	mainToolBar->addSeparator();
	
	resourcesInfo = new ResourcesInfoWidget(mainToolBar);
	resourcesInfo->setFlat(true);
	mainToolBar->addWidget(resourcesInfo);
	
	
	mainToolBar->addSeparator();
	
	hddInfo = new HDDSpaceInfoWidget(mainToolBar);
	hddInfo->setFlat(true);
	mainToolBar->addWidget(hddInfo);
	
	mainToolBar->addSeparator();
	
	tbutton = new QPushButton(mainToolBar);
// 	tbutton->setPopupMode(QToolButton::InstantPopup);
	text = tr("Help");
	tbutton->setText(text);
	tbutton->setMaximumWidth(fm.width(text) + 32);
	tbutton->setFlat(true);
	tbutton->setMenu(helpMenu);
	mainToolBar->addWidget(tbutton);
	tbutton->setFocusPolicy(Qt::NoFocus);
	
/*	DigitalClock* clock = new DigitalClock();
	mainToolBar->addWidget(clock);*/
	
}

void Interface::process_context_menu_action( QAction * action )
{
	QString name = (action->data()).toString();
	ie().broadcast_action_from_contextmenu(name);
}

Command * Interface::show_context_menu( )
{
	QList<QObject* > items = cpointer().get_context_items();
	if (items.isEmpty()) {
		printf("cpointer() returned empty list\n");
		return 0;
	}
	
	ViewItem* viewitem = qobject_cast<ViewItem*>(items.at(0));
	
	if (! viewitem) {
		printf("cpointer() first returned item is NOT a ViewItem\n");
		return 0;
	}
	
	QString className = viewitem->metaObject()->className();
	
	QMenu* menu = m_contextMenus.value(className);
	
	if ( ! menu ) {
		printf("No menu for %s, creating new one\n", QS_C(className));
		menu = create_context_menu(viewitem);
		m_contextMenus.insert(className, menu);
	}
	
	menu->exec(QCursor::pos());
	
	return 0;
}

QMenu* Interface::create_context_menu( ContextItem * item )
{
	printf("entering create_context_menu\n");
	QMenu* menu = new QMenu();
	
	
	connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(process_context_menu_action(QAction*)));
	
	QList<IEAction* > actionLst = ie().get_contextitem_actionlist( item );
	
	qSort(actionLst.begin(), actionLst.end(), IEAction::smaller);

	foreach(IEAction* ieaction, actionLst) {
		QString text = QString(ieaction->keySequence + "  " + ieaction->name);
		QAction* action = new QAction(this);
		action->setText(text);
		action->setData(ieaction->name);
		menu->addAction(action);
	}
	
	return menu;
}


Command* Interface::select_bus_in()
{
	PENTER;
	QMenu* menu = m_contextMenus.value("busInMenu");
	
	if (!menu) {
		menu = new QMenu();
		m_contextMenus.insert("busInMenu", menu);
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(set_bus_in(QAction*)));
	}
		
	
	menu->clear();
	
	QStringList names = audiodevice().get_capture_buses_names();
	
	foreach(QString name, names) {
		menu->addAction(name);
	}
	
	menu->exec(QCursor::pos());
	
	return (Command*) 0;
}


Command* Interface::select_bus_out()
{
	QMenu* menu = m_contextMenus.value("busOutMenu");
	
	if (!menu) {
		menu = new QMenu();
		m_contextMenus.insert("busOutMenu", menu);
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(set_bus_out(QAction*)));
	}
		
	
	menu->clear();
	
	QStringList names = audiodevice().get_playback_buses_names();
	
	foreach(QString name, names) {
		menu->addAction(name);
	}
	
	menu->exec(QCursor::pos());
	
	return (Command*) 0;
}

void Interface::set_bus_in( QAction* action )
{
	PENTER;
	QList<QObject* > items = cpointer().get_context_items();
	foreach(QObject* obj, items) {
		TrackView* tv = qobject_cast<TrackView*>(obj);
		if (tv) {
			tv->get_track()->set_bus_in(action->text().toAscii());
			break;
		}
	}
}

void Interface::set_bus_out( QAction* action )
{
	PENTER;
	QList<QObject* > items = cpointer().get_context_items();
	foreach(QObject* obj, items) {
		TrackView* tv = qobject_cast<TrackView*>(obj);
		if (tv) {
			tv->get_track()->set_bus_out(action->text().toAscii());
			break;
		}
	}
}

Command * Interface::select_fade_in_shape( )
{
	QMenu* menu = m_contextMenus.value("fadeInSelector");
	
	if (!menu) {
		menu = create_fade_selector_menu("fadeInSelector");
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(set_fade_in_shape(QAction*)));
	}
		
	
	menu->exec(QCursor::pos());
	
	return (Command*) 0;
}

Command * Interface::select_fade_out_shape( )
{
	QMenu* menu = m_contextMenus.value("fadeOutSelector");
	
	if (!menu) {
		menu = create_fade_selector_menu("fadeOutSelector");
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(set_fade_out_shape(QAction*)));
	}
		
	
	menu->exec(QCursor::pos());
	
	return (Command*) 0;
}


void Interface::set_fade_in_shape( QAction * action )
{
	QList<QObject* > items = cpointer().get_context_items();
	foreach(QObject* obj, items) {
		AudioClipView* acv = qobject_cast<AudioClipView*>(obj);
		if (acv) {
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

void Interface::show_driver_config_widget( )
{
	if (! driverConfigWidget) {
		driverConfigWidget = new QuickDriverConfigWidget(this);
	}
	
	// Hmmm, very weak positioning code imho. Can't find how to do it any better right now :(
	driverConfigWidget->move(driverInfo->x() + x() + 2, geometry().y() + mainToolBar->height() - 2);
	driverConfigWidget->show();
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


// eof
