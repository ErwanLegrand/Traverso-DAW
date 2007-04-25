/*
Copyright (C) 2006-2007 Remon Sijrier 

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

#ifndef PLUGIN_SELECTOR_DIALOG_H
#define PLUGIN_SELECTOR_DIALOG_H

#include "ui_PluginSelectorDialog.h"
#include <QDialog>

class Plugin;

class PluginSelectorDialog : public QDialog, protected Ui::PluginSelectorDialog
{
	Q_OBJECT

public:
	
	static PluginSelectorDialog* instance();
	
	Plugin* get_selected_plugin();

private:
	PluginSelectorDialog(QWidget* parent = 0);
	~PluginSelectorDialog();
	
	static PluginSelectorDialog* m_instance;
	
	Plugin* m_plugin;

private slots:
	void on_okButton_clicked();
	void on_cancelButton_clicked();
	void plugin_double_clicked();
	
};

#endif

//eof


