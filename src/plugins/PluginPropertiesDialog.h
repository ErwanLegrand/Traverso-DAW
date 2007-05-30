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


#ifndef LV2_PLUGIN_PROPERTIES_DIALOG_H
#define LV2_PLUGIN_PROPERTIES_DIALOG_H

#include <QDialog>

class Plugin;
class PluginSlider;
class QPushButton;

class PluginPropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	PluginPropertiesDialog(QWidget* parent, Plugin* plugin);
	~PluginPropertiesDialog(){};


private:
	Plugin*	m_plugin;
	QList<PluginSlider*> m_sliders;
	QPushButton* m_bypassButton;
	
private slots:
	void bypass_button_clicked();
	void reset_button_clicked();
};

#endif

//eof
