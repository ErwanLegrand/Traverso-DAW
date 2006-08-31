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

$Id: Interface.h,v 1.3 2006/08/31 12:39:09 r_sijrier Exp $
*/

#ifndef INTERFACE_H
#define INTERFACE_H

#include <QResizeEvent>
#include <QMainWindow>
#include <QWidget>
#include "libtraversocore.h"

class Help;
class Song;
class Project;
class BusMonitor;
class InfoBox;
class ViewPort;
class SongView;
class OverViewWidget;

class QLabel;
class ManagerWidget;
class ExportWidget;
class QStackedWidget;
class BorderLayout;
class QHBoxLayout;


class Interface : public QMainWindow
{
	Q_OBJECT

public :
	static const int MINIMUM_INTERFACE_WIDTH = 750;
	static const int MINIMUM_INTERFACE_HEIGHT = 500;

	Interface();
	~Interface();

	void busmonitor_dock();
	void busmonitor_undock();
	void create();

	bool is_busmonitor_docked();


public slots :
	void set_project(Project* project);
	void set_songview(Song* song);
	void create_songview(Song* song);

	Command* set_manager_widget();
	Command* set_songview_widget();
	Command* full_screen();
	Command* about_traverso();
	Command* show_export_widget();

protected:
	void resizeEvent(QResizeEvent* e);
	void keyPressEvent ( QKeyEvent* e);
	void keyReleaseEvent ( QKeyEvent* e);
	void wheelEvent ( QWheelEvent* e );

private:
	QStackedWidget* 		centerAreaWidget;
	QList<SongView* > 		songViewList;
	QList<ViewPort* > 		currentProjectViewPortList;
	SongView* 			currentSongView;
	ManagerWidget* 			managerWidget;
	ExportWidget*			exportWidget;
	OverViewWidget* 		overView;

	bool 				managerWidgetCreated;
	bool 				isBusMonitorDocked;

	InfoBox*			infoBox;
	BusMonitor* 			busMonitor;
	QWidget* 			busMonitorWindow;
	Help* 				helpWindow;

	BorderLayout* 			mainVBoxLayout;
	QHBoxLayout* 			topPanelWidgetLayout;
	QHBoxLayout* 			statusAreaWidgetLayout;
	QHBoxLayout* 			centerWidgetLayout;
	QWidget* 			topPanelWidget;
	QWidget* 			statusAreaWidget;
	
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
	
	void create_menus();
	void create_menu_actions();

};

#endif


// eof
