/*
Copyright (C) 2007 Remon Sijrier 

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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class SettingsDialog : public QDialog
{
	Q_OBJECT
	
public:
	SettingsDialog(QWidget* parent=0);
	
	void show_page(const QString& page);
	
public slots:
	void changePage(QListWidgetItem *current, QListWidgetItem *previous);
	
private:
	void createIcons();
	
	QListWidget *contentsWidget;
	QStackedWidget *pagesWidget;
	
	bool m_saving;
	

private slots:
	void save_config();
	void restore_defaults_button_clicked();
	void external_change_to_settings();
};

#endif
