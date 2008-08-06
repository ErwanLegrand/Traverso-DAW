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

#ifndef PAGES_H
#define PAGES_H

#include <QWidget>

#include "ui_DriverConfigPage.h"
#include "ui_KeyboardConfigPage.h"
#include "ui_BehaviorConfigPage.h"
#include "ui_RecordingConfigPage.h"
#include "ui_ThemeConfigPage.h"
#include "ui_PerformanceConfigPage.h"

#if defined (ALSA_SUPPORT)
#include "ui_AlsaDevicesPage.h"
#endif

#if defined (PORTAUDIO_SUPPORT)
#include "ui_PaDriverPage.h"
#endif

class PerformanceConfigPage : public QWidget, private Ui::PerformanceConfigPage
{
public:
	PerformanceConfigPage(QWidget* parent = 0);

private:
	friend class PerformancePage;
};

class DriverConfigPage : public QWidget, private Ui::DriverConfigPage
{
	Q_OBJECT

public:
	DriverConfigPage(QWidget* parent = 0);

private:
	QList<int>	periodBufferSizesList;
	friend class AudioDriverPage;

private slots:
	void update_latency_combobox();
	void rate_combobox_index_changed(QString);
};


#if defined (ALSA_SUPPORT)
class AlsaDevicesPage : public QWidget, private Ui::AlsaDevicesPage
{
public:
	AlsaDevicesPage(QWidget* parent = 0);
private:
	friend class AudioDriverPage;
};
#endif


#if defined (PORTAUDIO_SUPPORT)
class PaDriverPage : public QWidget, private Ui::PaDriverPage
{
public:
	PaDriverPage(QWidget* parent = 0);
private:
	friend class AudioDriverPage;
};
#endif


class KeyboardConfigPage : public QWidget, private Ui::KeyboardConfigPage
{
	Q_OBJECT
public:
	KeyboardConfigPage(QWidget* parent = 0);
private:
	friend class KeyboardPage;
	
private slots:
	void keymap_index_changed(const QString& keymap);
	void update_keymap_combo();
	void on_exportButton_clicked();
	void on_printButton_clicked();
};

class BehaviorConfigPage : public QWidget, private Ui::BehaviorConfigPage
{
	Q_OBJECT
public:
	BehaviorConfigPage(QWidget* parent = 0);
private:
	friend class BehaviorPage;
};


class RecordingConfigPage : public QWidget, private Ui::RecordingConfigPage
{
	Q_OBJECT
public:
	RecordingConfigPage(QWidget* parent = 0);
private:
	friend class RecordingPage;
private slots:
	void encoding_index_changed(int index);
	void use_onthefly_resampling_checkbox_changed(int state);
};


class ThemeConfigPage : public QWidget, private Ui::ThemeConfigPage
{
	Q_OBJECT
public:
	ThemeConfigPage(QWidget* parent = 0);
	
private:
	void update_theme_combobox(const QString& path);
	void create_connections();
	friend class AppearancePage;

private slots:
	void dirselect_button_clicked();
	void style_index_changed(const QString& text);
	void theme_index_changed(const QString& theme);
	void use_selected_styles_pallet_checkbox_toggled(bool);
	void color_adjustbox_changed(int);
	void theme_option_changed();
};




class ConfigPage : public QWidget
{
	Q_OBJECT
public:
	ConfigPage(QWidget* parent);
	virtual void save_config() = 0;
	virtual void load_config() = 0;
	virtual void reset_default_config() = 0;
protected:
	QVBoxLayout* mainLayout;
};


class AudioDriverPage : public ConfigPage
{
	Q_OBJECT
public:
	AudioDriverPage(QWidget *parent = 0);
    	void save_config();
	void reset_default_config();

private:
	QVBoxLayout* m_mainLayout;
	QComboBox* m_driverCombo;
	DriverConfigPage* m_driverConfigPage;
#if defined (ALSA_SUPPORT)
	AlsaDevicesPage* m_alsadevices;
#endif
	
#if defined (PORTAUDIO_SUPPORT)
	PaDriverPage* m_portaudiodrivers;
#endif
	
	void load_config();
	
private slots:
	void driver_combobox_index_changed(QString);
	void restart_driver_button_clicked();
};


class RecordingPage : public ConfigPage
{
public:
	RecordingPage(QWidget *parent = 0);
	void load_config();
	void save_config();
	void reset_default_config();
private:
	RecordingConfigPage* m_config;
};


class KeyboardPage : public ConfigPage
{
public:
	KeyboardPage(QWidget* parent=0);
	void load_config();
	void save_config();
	void reset_default_config();

private:
	KeyboardConfigPage* m_configpage;
};


class AppearancePage : public ConfigPage
{
public:
	AppearancePage(QWidget *parent = 0);
	void load_config();
	void save_config();
	void reset_default_config();

private:
	QString supportedIconSizes;
	ThemeConfigPage* m_themepage;
};


class BehaviorPage : public ConfigPage
{
	Q_OBJECT
public:
	BehaviorPage(QWidget *parent = 0);
	void load_config();
	void save_config();
	void reset_default_config();
	
private slots:
	void update_follow();

private:
	BehaviorConfigPage* m_configpage;
};

class PerformancePage : public ConfigPage
{
public:
	PerformancePage(QWidget *parent = 0);
	void load_config();
	void save_config();
	void reset_default_config();
	
private:
	PerformanceConfigPage* m_configpage;
};

#endif
