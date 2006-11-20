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

$Id: Interface.h,v 1.9 2006/11/20 16:37:57 n_doebelin Exp $
*/

#ifndef INTERFACE_H
#define INTERFACE_H

#include <QtGui>

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
class ManagerWidget;
class ExportWidget;
class QStackedWidget;
class QHBoxLayout;
class QVBoxLayout;
class QTreeView;
class QuickDriverConfigWidget;
class ResourcesInfoWidget;
class DriverInfoWidget;
class HDDSpaceInfoWidget;
class SongWidget;
class MultiMeterWidget;

class Interface : public QMainWindow
{
	Q_OBJECT

public :
	Interface();
	~Interface();

protected:
	void keyPressEvent ( QKeyEvent* e);
	void keyReleaseEvent ( QKeyEvent* e);
	void wheelEvent ( QWheelEvent* e );
	

private:
	QStackedWidget* 	centerAreaWidget;
	QHash<Song*, SongWidget* > m_songWidgets;
	SongWidget*		currentSongWidget;
	QList<ViewPort* > 	currentProjectViewPortList;
	QHash<QString, QMenu*>	m_contextMenus;
	ManagerWidget* 		managerWidget;
	ExportWidget*		exportWidget;
	OverViewWidget* 	overView;
	QUndoView*		historyWidget;
	QDockWidget* 		historyDW;
	QDockWidget*		busMonitorDW;
	QDockWidget*		AudioSourcesDW;
	QTreeView* 		audiosourcesview;
	QuickDriverConfigWidget* driverConfigWidget;
	QDockWidget*		multiMeterDW;
	MultiMeterWidget*	multiMeter;

	BusMonitor* 		busMonitor;
	Help* 			helpWindow;

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
	QToolButton*		openGlButton;
	
	ResourcesInfoWidget*	resourcesInfo;
	DriverInfoWidget*	driverInfo;
	HDDSpaceInfoWidget*	hddInfo;
	
	void create_menus();
	
	QMenu* create_context_menu(ContextItem* item);
	QMenu* create_fade_selector_menu(const QString& fadeTypeName);

public slots :
	void set_project(Project* project);
	void show_song(Song* song);
	void process_context_menu_action(QAction* action);
	void set_bus_in(QAction* action);
	void set_bus_out(QAction* action);
	void set_fade_in_shape(QAction* action);
	void set_fade_out_shape(QAction* action);
	void show_driver_config_widget();
	void toggle_OpenGL();

	Command* set_manager_widget();
	Command* show_song_widget();
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
