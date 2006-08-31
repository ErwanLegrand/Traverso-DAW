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

$Id: Interface.cpp,v 1.11 2006/08/31 12:39:09 r_sijrier Exp $
*/

#include "../config.h"

#include <libtraversocore.h>

#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QtGui>

#include "Interface.h"
#include "BusMonitor.h"
#include "ProjectManager.h"
#include "InfoBox.h"
#include "BorderLayout.h"
#include "SongView.h"
#include "ViewPort.h"
#include "OverViewWidget.h"
#include "Help.h"

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

	setMinimumSize(MINIMUM_INTERFACE_WIDTH, MINIMUM_INTERFACE_HEIGHT);
	setWindowIcon(QPixmap (":/traverso") );

	//         setMaximumWidth(1024);
	//         setMaximumHeight(768);

	exportWidget = 0;

	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project* )), this, SLOT(set_project(Project* )));

	cpointer().add_contextitem(this);
};

Interface::~Interface()
{
	PENTERDES;
	QSettings settings;
	settings.beginGroup("Interface");
	settings.setValue("size", size());
	settings.setValue("fullScreen", isFullScreen());
	settings.setValue("pos", pos());
	settings.endGroup();
	
	delete helpWindow;
}

void Interface::create()
{
	QWidget* centralWidget = new QWidget;
	setCentralWidget(centralWidget);
	
	setWindowTitle("Traverso");

	helpWindow = new Help(this);

	mainVBoxLayout = new BorderLayout(centralWidget, 0, 0);

	topPanelWidget = new QWidget(centralWidget);
	topPanelWidgetLayout = new QHBoxLayout(topPanelWidget);
	topPanelWidget->setLayout(topPanelWidgetLayout);
	topPanelWidget->setMinimumHeight(TOPPANEL_FIXED_HEIGHT);
	topPanelWidget->setMaximumHeight(TOPPANEL_FIXED_HEIGHT);

	infoBox = new InfoBox(topPanelWidget);
	topPanelWidgetLayout->insertWidget( 0, infoBox, 4);

	busMonitor = new BusMonitor(topPanelWidget, this);
	topPanelWidgetLayout->insertWidget( 1, busMonitor);
	busMonitor->show();

	isBusMonitorDocked = true;
	// 	busMonitorWindow = new QWidget( (QWidget*) 0);
	// 	busMonitorWindow->hide();


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

	QWidget* buttonWidget = new QWidget(statusAreaWidget);
	buttonWidget->setMinimumWidth(180);
	buttonWidget->setMinimumHeight(18);
	buttonWidget->setMaximumHeight (18);
	statusAreaWidgetLayout->addWidget(buttonWidget);
	
	QPushButton* helpbutton = new QPushButton("Help", buttonWidget);
	helpbutton->setMaximumHeight (18);
	helpbutton->setFocusPolicy(Qt::NoFocus);
	connect(helpbutton, SIGNAL(clicked()), helpWindow, SLOT(show_help()));


	overView = new OverViewWidget(statusAreaWidget);
	statusAreaWidgetLayout->addWidget(overView, 4);

	topPanelWidgetLayout->setMargin(3);
	centerWidgetLayout->setMargin(3);
	statusAreaWidgetLayout->setMargin(3);

	mainVBoxLayout->addWidget(topPanelWidget, BorderLayout::North);
	mainVBoxLayout->addWidget(widget, BorderLayout::Center);
	mainVBoxLayout->addWidget(statusAreaWidget, BorderLayout::South);
	
	centralWidget->setLayout(mainVBoxLayout);

	managerWidgetCreated = false;

	/** Read in the Interface settings and apply them
	*/
	QSettings settings;
	settings.beginGroup("Interface");
	resize(settings.value("size", QSize(400, 400)).toSize());
	move(settings.value("pos", QPoint(200, 200)).toPoint());
	settings.endGroup();
	
	create_menu_actions();
	create_menus();

// 	show();
}


void Interface::set_project(Project* project)
{
	PENTER;
	
	if ( project ) {
		connect(project, SIGNAL(currentSongChanged(Song* )), this, SLOT(set_songview(Song* )));
		connect(project, SIGNAL(currentSongChanged(Song* )), overView, SLOT(set_song(Song* )));
		connect(project, SIGNAL(newSongCreated(Song* )), this, SLOT(create_songview(Song* )));
		
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
			centerAreaWidget->setCurrentWidget(sv->get_viewport());
			currentSongView = sv;
			
			// NB If I don't set this explicitely, Qt doesn't return the
			// focus to this widget after for example a popup of a QMenu :-(
			sv->get_viewport()->setFocus();
			
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

void Interface::busmonitor_dock()
{
	PENTER2;
	/*	busMonitor->setParent(wid);
		//Reset busMonitor maximum height to fit busMonitor in widLayout
		busMonitor->setMinimumHeight(MINIMUM_BUS_MONITOR_HEIGHT);
		widLayout->insertWidget( 2, busMonitor);
		busMonitorWindow->hide();
		isBusMonitorDocked = true;*/
}

void Interface::busmonitor_undock()
{
	PENTER2;
	/*	busMonitor->setParent(busMonitorWindow->parentWidget());
		//Make the floating busMonitor a bit heigher by default
		busMonitor->setMinimumHeight(MINIMUM_FLOATING_BUS_MONITOR_HEIGHT);
		widLayout->removeWidget(busMonitor);
		isBusMonitorDocked = false;*/
}

bool Interface::is_busmonitor_docked()
{
	return isBusMonitorDocked;
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
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(saveAction);
	fileMenu->addAction(exitAction);
	
	menuBar()->addSeparator();
	
	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(projManViewAction);
	viewMenu->addAction(editViewAction);
	viewMenu->addAction(curveViewAction);
	viewMenu->addAction(settingsViewAction);
	
	menuBar()->addSeparator();
	
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(handBookAction);
	helpMenu->addAction(aboutTraversoAction);

}

void Interface::create_menu_actions( )
{
	saveAction = new QAction(tr("&Save"), this);
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
}


// eof
