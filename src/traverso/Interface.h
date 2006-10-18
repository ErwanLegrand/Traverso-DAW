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

$Id: Interface.h,v 1.6 2006/10/18 12:08:56 r_sijrier Exp $
*/

#ifndef INTERFACE_H
#define INTERFACE_H

#include <QResizeEvent>
#include <QMainWindow>
#include <QWidget>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QToolBar>
#include <QHash>

class Help;
class Song;
class Project;
class BusMonitor;
class InfoBox;
class ViewPort;
class SongView;
class OverViewWidget;
class ContextItem;
class Command;

class QLabel;
class ManagerWidget;
class ExportWidget;
class QStackedWidget;
class BorderLayout;
class QHBoxLayout;
class HistoryWidget;
class QTreeView;
class QuickDriverConfigWidget;
class ResourcesInfoWidget;
class DriverInfoWidget;
class HDDSpaceInfoWidget;

class Interface : public QMainWindow
{
	Q_OBJECT

public :
	Interface();
	~Interface();

protected:
	void resizeEvent(QResizeEvent* e);
	void keyPressEvent ( QKeyEvent* e);
	void keyReleaseEvent ( QKeyEvent* e);
	void wheelEvent ( QWheelEvent* e );

private:
	QStackedWidget* 	centerAreaWidget;
	QList<SongView* > 	songViewList;
	QList<ViewPort* > 	currentProjectViewPortList;
	QHash<QString, QMenu*>	m_contextMenus;
	SongView* 		currentSongView;
	ManagerWidget* 		managerWidget;
	ExportWidget*		exportWidget;
	OverViewWidget* 	overView;
	HistoryWidget*		historyWidget;
	QDockWidget* 		hvdw;
	QDockWidget*		tpdw;
	QDockWidget*		asdw;
	QTreeView* 		audiosourcesview;
	QuickDriverConfigWidget* driverConfigWidget;
	 
	bool 			managerWidgetCreated;

	InfoBox*		infoBox;
	BusMonitor* 		busMonitor;
	Help* 			helpWindow;

	BorderLayout* 		mainVBoxLayout;
	QHBoxLayout* 		topPanelWidgetLayout;
	QHBoxLayout* 		statusAreaWidgetLayout;
	QHBoxLayout* 		centerWidgetLayout;
	QWidget* 		topPanelWidget;
	QWidget* 		statusAreaWidget;
	
	QMenu*			fileMenu;
	QMenu*			helpMenu;
	QMenu*			viewMenu;
	
	QAction*		exitAction;
	QAction*		saveAction;
	QAction*		aboutTraversoAction;
	QAction*		handBookAction;
	
	QAction*		editViewAction;
	QAction*		curveViewAction;
	QAction*		projManViewAction;
	QAction*		settingsViewAction;
	
	QToolBar* 		mainToolBar;
	
	ResourcesInfoWidget*	resourcesInfo;
	DriverInfoWidget*	driverInfo;
	HDDSpaceInfoWidget*	hddInfo;
	
	void create();
	void create_menus();
	
	QMenu* create_context_menu(ContextItem* item);
	QMenu* create_fade_selector_menu(const QString& fadeTypeName);

public slots :
	void set_project(Project* project);
	void set_songview(Song* song);
	void create_songview(Song* song);
	void process_context_menu_action(QAction* action);
	void set_bus_in(QAction* action);
	void set_bus_out(QAction* action);
	void set_fade_in_shape(QAction* action);
	void set_fade_out_shape(QAction* action);
	void show_driver_config_widget();

	Command* set_manager_widget();
	Command* set_songview_widget();
	Command* full_screen();
	Command* about_traverso();
	Command* show_export_widget();
	Command* show_context_menu();
	Command* select_bus_in();
	Command* select_bus_out();
	Command* select_fade_in_shape();
	Command* select_fade_out_shape();
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
