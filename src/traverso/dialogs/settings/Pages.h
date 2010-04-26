/*
Copyright (C) 2007-2009 Remon Sijrier 

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

#ifndef CONFIG_PAGES_H
#define CONFIG_PAGES_H

#include <QWidget>

#include "ui_AudioDriverConfigPage.h"
#include "ui_KeyboardConfigPage.h"
#include "ui_BehaviorConfigPage.h"
#include "ui_RecordingConfigPage.h"
#include "ui_AppearenceConfigPage.h"
#include "ui_PerformanceConfigPage.h"
#include "ui_AlsaDevicesPage.h"
#include "ui_PaDriverPage.h"

class ThemeModifierDialog;

class AlsaDevicesPage : public QWidget, private Ui::AlsaDevicesPage
{
public:
	AlsaDevicesPage(QWidget* parent) : QWidget(parent) {
		setupUi(this);
	}
private:
	friend class AudioDriverConfigPage;
};


class PaDriverPage : public QWidget, private Ui::PaDriverPage
{
public:
	PaDriverPage(QWidget* parent) : QWidget(parent)	{
		setupUi(this);
	}

private:
	friend class AudioDriverConfigPage;
};



class ConfigPage : public QWidget
{
	Q_OBJECT
public:
	ConfigPage(QWidget* parent) : QWidget(parent) {};
	virtual void save_config() = 0;
	virtual void load_config() = 0;
	virtual void reset_default_config() = 0;
};



class AudioDriverConfigPage : public ConfigPage, private Ui::AudioDriverConfigPage
{
	Q_OBJECT
public:
	AudioDriverConfigPage(QWidget *parent = 0);
	
	void load_config();
    	void save_config();
	void reset_default_config();

private:
	QVBoxLayout* m_mainLayout;
	AudioDriverConfigPage* m_driverConfigPage;
	AlsaDevicesPage* m_alsadevices;
	PaDriverPage* m_portaudiodrivers;
	QList<int>	periodBufferSizesList;
	
private slots:
	void update_latency_combobox();
	void rate_combobox_index_changed(QString);
	void driver_combobox_index_changed(QString);
	void restart_driver_button_clicked();
};


class AppearenceConfigPage : public ConfigPage, private Ui::AppearenceConfigPage
{
	Q_OBJECT
public:
	AppearenceConfigPage(QWidget* parent = 0);
	
	void load_config();
	void save_config();
	void reset_default_config();

	
private:
	void update_theme_combobox(const QString& path);
	void create_connections();
	QString supportedIconSizes;
        ThemeModifierDialog* m_colorModifierDialog;

private slots:
	void dirselect_button_clicked();
	void style_index_changed(const QString& text);
	void theme_index_changed(const QString& theme);
	void use_selected_styles_pallet_checkbox_toggled(bool);
	void color_adjustbox_changed(int);
	void theme_option_changed();
        void edit_theme_button_clicked();
};


class BehaviorConfigPage : public ConfigPage, private Ui::BehaviorConfigPage
{
	Q_OBJECT
public:
	BehaviorConfigPage(QWidget* parent = 0);
	
	void load_config();
	void save_config();
	void reset_default_config();
	
private slots:
	void update_follow();
};


class KeyboardConfigPage : public ConfigPage, private Ui::KeyboardConfigPage
{
	Q_OBJECT
public:
	KeyboardConfigPage(QWidget* parent = 0);

	void load_config();
	void save_config();
	void reset_default_config();

private slots:
	void keymap_index_changed(const QString& keymap);
	void update_keymap_combo();
	void on_exportButton_clicked();
	void on_printButton_clicked();
};


class PerformanceConfigPage : public ConfigPage, private Ui::PerformanceConfigPage
{
public:
	PerformanceConfigPage(QWidget* parent = 0);
	
	void load_config();
	void save_config();
	void reset_default_config();
};


class RecordingConfigPage : public ConfigPage, private Ui::RecordingConfigPage
{
	Q_OBJECT
public:
	RecordingConfigPage(QWidget* parent = 0);
	
	void load_config();
	void save_config();
	void reset_default_config();

private slots:
	void encoding_index_changed(int index);
	void use_onthefly_resampling_checkbox_changed(int state);
};


#endif
