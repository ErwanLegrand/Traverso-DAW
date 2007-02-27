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

#include <QtGui>

#include "Pages.h"
#include <AudioDevice.h>
#if defined (ALSA_SUPPORT)
#include <AlsaDriver.h>
#endif
#include <Config.h>
#include <Utils.h>
#include <Themer.h>



/****************************************/
/*            AudioDriver               */
/****************************************/


AudioDriverPage::AudioDriverPage(QWidget *parent)
    : ConfigPage(parent)
{
	// AudioDriverPage manages it's own layout
	// perhaps, should be done like the other pages as well
	// anyhow, the created layout should be deleted before
	// the new one is set.
	delete layout();
	
	
	QGroupBox* selectionGroup = new QGroupBox(tr("Driver Selection"));
	m_driverConfigPage = new DriverConfigPage(this);
	
	QLabel* driverLabel = new QLabel(tr("Driver:"));
	m_driverCombo = new QComboBox;
	
	QStringList drivers = audiodevice().get_available_drivers();
	foreach(QString name, drivers) {
		m_driverCombo->addItem(name);
	}
	
	QHBoxLayout* driverLayout = new QHBoxLayout;
	driverLayout->addWidget(driverLabel);
	driverLayout->addWidget(m_driverCombo);
	
	QVBoxLayout* selectionLayout = new QVBoxLayout;
	selectionLayout->addLayout(driverLayout);
	selectionGroup->setLayout(selectionLayout);

	QPushButton* restartDriverButton = new QPushButton(tr("Restart Driver"));
	QHBoxLayout* restartDriverButtonLayout = new QHBoxLayout;
	restartDriverButtonLayout->addStretch(1);
	restartDriverButtonLayout->addWidget(restartDriverButton);
	
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(selectionGroup);
	mainLayout->addWidget(m_driverConfigPage);
#if defined (ALSA_SUPPORT)
	m_alsadevices = new AlsaDevicesPage(this);
	mainLayout->addWidget(m_alsadevices);
#endif
	mainLayout->addLayout(restartDriverButtonLayout);
	mainLayout->addStretch(1);
	mainLayout->setSpacing(12);
	setLayout(mainLayout);
	
	connect(m_driverCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(driver_combobox_index_changed(QString)));
	connect(restartDriverButton, SIGNAL(clicked()), this, SLOT(restart_driver_button_clicked()));
	
	load_config();
}

void AudioDriverPage::save_config()
{
	config().set_property("Hardware", "samplerate", m_driverConfigPage->rateComboBox->currentText());
	config().set_property("Hardware", "bufferSize", 
	       m_driverConfigPage->periodBufferSizesList.at(m_driverConfigPage->latencyComboBox->currentIndex()));
	
	config().set_property("Hardware", "drivertype", m_driverCombo->currentText());
	
	int playback=1, capture=1;
	if(m_driverConfigPage->duplexComboBox->currentIndex() == 1) {
		capture = 0;
	}
	
	if(m_driverConfigPage->duplexComboBox->currentIndex() == 2) {
		playback = 0;
	}
	
	config().set_property("Hardware", "capture", capture);
	config().set_property("Hardware", "playback", playback);

	
#if defined (ALSA_SUPPORT)
	int periods = m_alsadevices->periodsCombo->currentText().toInt();
	config().set_property("Hardware", "NumberOfPeriods", periods);
	int index = m_alsadevices->devicesCombo->currentIndex();
	config().set_property("Hardware", "carddevice", m_alsadevices->devicesCombo->itemData(index));
	
#endif

}

void AudioDriverPage::reset_default_config()
{
	config().set_property("Hardware", "samplerate", 44100);
	config().set_property("Hardware", "bufferSize", 1024);
#if defined (ALSA_SUPPORT)
	config().set_property("Hardware", "drivertype", "ALSA");
	config().set_property("Hardware", "carddevice", "hw:0");
	config().set_property("Hardware", "NumberOfPeriods", 2);
#elif defined (JACK_SUPPORT)
	config().set_property("Hardware", "drivertype", "Jack");
#else
	config().set_property("Hardware", "drivertype", "Null Driver");
	
#endif
	config().set_property("Hardware", "capture", 1);
	config().set_property("Hardware", "playback", 1);
	
	load_config();
}

void AudioDriverPage::load_config( )
{
	int samplerate = config().get_property("Hardware", "samplerate", 44100).toInt();
	int buffersize = config().get_property("Hardware", "buffersize", 1024).toInt();
	QString driverType = config().get_property("Hardware", "drivertype", "ALSA").toString();
	bool capture = config().get_property("Hardware", "capture", 1).toInt();
	bool playback = config().get_property("Hardware", "playback", 1).toInt();


	int driverTypeIndex = m_driverCombo->findText(driverType);
	if (driverTypeIndex >= 0) {
		m_driverCombo->setCurrentIndex(driverTypeIndex);
	}
	
	driver_combobox_index_changed(driverType);
	
	int buffersizeIndex = m_driverConfigPage->periodBufferSizesList.indexOf(buffersize);
	int samplerateIndex = m_driverConfigPage->rateComboBox->findText(QString::number(samplerate));
	
	m_driverConfigPage->rateComboBox->setCurrentIndex(samplerateIndex);
	m_driverConfigPage->latencyComboBox->setCurrentIndex(buffersizeIndex);
	
	
	if (capture && playback) {
		m_driverConfigPage->duplexComboBox->setCurrentIndex(0);
	} else if (playback) {
		m_driverConfigPage->duplexComboBox->setCurrentIndex(1);
	} else {
		m_driverConfigPage->duplexComboBox->setCurrentIndex(2);
	}
	

#if defined (ALSA_SUPPORT)
	int periodsIndex = config().get_property("Hardware", "NumberOfPeriods", 1).toInt();
	m_alsadevices->periodsCombo->setCurrentIndex(periodsIndex - 2);
	
	QString name;
	for (int i=0; i<6; ++i) {
		name = AlsaDriver::alsa_device_name(false, i);
		if (name != "") {
			QString card = "Card " + QString::number(i+1) + ":  ";
			QString device = "hw:" + QString::number(i);
			m_alsadevices->devicesCombo->addItem(card + name, device);
		}
	}
	
	QString defaultdevice =  config().get_property("Hardware", "carddevice", "hw:0").toString();
	int index = m_alsadevices->devicesCombo->findData(defaultdevice);
	if (index >= 0) {
		m_alsadevices->devicesCombo->setCurrentIndex(index);
	}
#endif
}


void AudioDriverPage::restart_driver_button_clicked()
{
	QString driver = m_driverCombo->currentText();
	int rate = m_driverConfigPage->rateComboBox->currentText().toInt();
	int buffersize =  m_driverConfigPage->periodBufferSizesList.at(m_driverConfigPage->latencyComboBox->currentIndex());
	
	int playback=1, capture=1;
	if(m_driverConfigPage->duplexComboBox->currentIndex() == 1) {
		capture = 0;
	}
	
	if(m_driverConfigPage->duplexComboBox->currentIndex() == 2) {
		playback = 0;
	}
	
	int periods = m_alsadevices->periodsCombo->currentText().toInt();
	
	// The AlsaDriver retrieves it's periods number directly from config()
	// So there is no way to use the current selected one, other then
	// setting it now, and restoring it afterwards...
	int currentperiods = config().get_property("Hardware", "NumberOfPeriods", 2).toInt();
	config().set_property("Hardware", "NumberOfPeriods", periods);
	
	QString cardDevice = "";

#if defined (ALSA_SUPPORT)
	config().get_property("Hardware", "carddevice", "hw:0").toString();
	int index = m_alsadevices->devicesCombo->currentIndex();
	cardDevice = m_alsadevices->devicesCombo->itemData(index).toString();
#endif
		
	audiodevice().set_parameters(rate, buffersize, driver, capture, playback, cardDevice);
	
	config().set_property("Hardware", "NumberOfPeriods", currentperiods);
}



void AudioDriverPage::driver_combobox_index_changed(QString driver)
{
	if (driver == "ALSA" || driver == "Null Driver") {
		m_driverConfigPage->setEnabled(true);
	} else {
		m_driverConfigPage->setEnabled(false);
	}
	
#if defined (ALSA_SUPPORT)
	if (driver == "ALSA") {
		m_alsadevices->show();
	} else {
		m_alsadevices->hide();
	}
#endif
}


DriverConfigPage::DriverConfigPage( QWidget * parent )
	: QWidget(parent)
{
	setupUi(this);
	periodBufferSizesList << 16 << 32 << 64 << 128 << 256 << 512 << 1024 << 2048 << 4096;
	
	connect(rateComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(rate_combobox_index_changed(QString)));
}


void DriverConfigPage::update_latency_combobox( )
{
	latencyComboBox->clear();
	int rate = rateComboBox->currentText().toInt();
	int buffersize = audiodevice().get_buffer_size();
	
	for (int i=0; i<periodBufferSizesList.size(); ++i) {
		QString latency = QString::number( ((float)(periodBufferSizesList.at(i)) / rate) * 1000 * 2, 'f', 2);
		latencyComboBox->addItem(latency);
	}
	
	int index = periodBufferSizesList.indexOf(buffersize);
	latencyComboBox->setCurrentIndex(index);
}

void DriverConfigPage::rate_combobox_index_changed(QString )
{
	update_latency_combobox();
}

#if defined (ALSA_SUPPORT)
AlsaDevicesPage::AlsaDevicesPage(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
}
#endif



ConfigPage::ConfigPage(QWidget * parent)
	: QWidget(parent)
{
	mainLayout = new QVBoxLayout;
	setLayout(mainLayout);
}



/****************************************/
/*            Behavior                  */
/****************************************/


BehaviorPage::BehaviorPage(QWidget *parent)
	: ConfigPage(parent)
{
	m_configpage = new BehaviorConfigPage(this);
	mainLayout->addWidget(m_configpage);
	mainLayout->addStretch(1);
	load_config();
}


void BehaviorPage::save_config()
{
	config().set_property("Project", "DefaultDirectory", m_configpage->projectDirLineEdit->text());
	config().set_property("Project", "loadLastUsed", m_configpage->loadLastProjectCheckBox->isChecked());
	config().set_property("Song", "trackCreationCount", m_configpage->numberOfTrackSpinBox->value());
	QString oncloseaction;
	if (m_configpage->saveRadioButton->isChecked()) {
		config().set_property("Project", "onclose", "save");
	} else if (m_configpage->askRadioButton->isChecked()) {
		config().set_property("Project", "onclose", "ask");
	} else {
		config().set_property("Project", "onclose", "dontsave");
	}
}

void BehaviorPage::load_config()
{
	QString dir = config().get_property("Project", "DefaultDirectory", getenv("HOME")).toString();
	bool loadLastUsedProject = config().get_property("Project", "loadLastUsed", 1).toBool();
	QString oncloseaction = config().get_property("Project", "onclose", "save").toString();
	int defaultNumTracks = config().get_property("Song", "trackCreationCount", 6).toInt();
	
	m_configpage->projectDirLineEdit->setText(dir);
	m_configpage->loadLastProjectCheckBox->setChecked(loadLastUsedProject);
	m_configpage->numberOfTrackSpinBox->setValue(defaultNumTracks);
	
	if (oncloseaction == "save") {
		m_configpage->saveRadioButton->setChecked(true);
	} else if (oncloseaction == "ask") {
		m_configpage->askRadioButton->setChecked(true);
	} else {
		m_configpage->neverRadioButton->setChecked(true);
	}
}


void BehaviorPage::reset_default_config()
{
	config().set_property("Project", "DefaultDirectory", getenv("HOME"));
	config().set_property("Project", "loadLastUsed", true);
	config().set_property("Project", "onclose", "save");
	config().set_property("Song", "trackCreationCount", 6);
	
	load_config();
}


BehaviorConfigPage::BehaviorConfigPage(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
	QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon);
	selectButton->setIcon(icon);
	
	connect(selectButton, SIGNAL(clicked()), this, SLOT(dirselect_button_clicked()));
}

void BehaviorConfigPage::dirselect_button_clicked()
{
	QString dirName = QFileDialog::getExistingDirectory(this, 
				tr("Select default project dir"), 
				projectDirLineEdit->text());
	
	if (!dirName.isEmpty()) {
		projectDirLineEdit->setText(dirName);
	}
}





/****************************************/
/*            Appearance                */
/****************************************/

AppearancePage::AppearancePage(QWidget *parent)
	: ConfigPage(parent)
{
	m_themepage = new ThemeConfigPage(this);
	mainLayout->addWidget(m_themepage);
	mainLayout->addStretch(1);
	
	load_config();
	m_themepage->create_connections();
}

void AppearancePage::save_config()
{
	QString path = m_themepage->themePathLineEdit->text();
	
	config().set_property("Themer", "themepath", path);
	config().set_property("Themer", "currenttheme", m_themepage->themeSelecterCombo->currentText());
	config().set_property("Themer", "coloradjust", m_themepage->colorAdjustBox->value());
	config().set_property("Themer", "style", m_themepage->styleCombo->currentText());
	config().set_property("Themer", "usestylepallet", m_themepage->useStylePalletCheckBox->isChecked());
}

void AppearancePage::load_config()
{
	QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon);
	m_themepage->pathSelectButton->setIcon(icon);
	QString themepath = config().get_property("Themer", "themepath",
				   QString(getenv("HOME")).append(".traverso/themes")).toString();
	
	
	foreach(QString key, QStyleFactory::keys()) {
		m_themepage->styleCombo->addItem(key);
	}
	
	
	m_themepage->update_theme_combobox(themepath);
	
	
	// Hmm, there seems no way to get the name of the current
	// used style, using the classname minus Q and Style seems to do the trick.
	QString systemstyle = QString(QApplication::style()->metaObject()->className()).remove("Q").remove("Style");
	QString style = config().get_property("Themer", "style", systemstyle).toString();
	QString theme  = config().get_property("Themer", "currenttheme", "TraversoLight").toString();
	int coloradjust = config().get_property("Themer", "coloradjust", 100).toInt();
	bool usestylepallete = config().get_property("Themer", "usestylepallet", "").toBool();
	
	int index = m_themepage->styleCombo->findText(style);
	m_themepage->styleCombo->setCurrentIndex(index);
	index = m_themepage->themeSelecterCombo->findText(theme);
	m_themepage->themeSelecterCombo->setCurrentIndex(index);
	m_themepage->colorAdjustBox->setValue(coloradjust);
	m_themepage->useStylePalletCheckBox->setChecked(usestylepallete);
	m_themepage->themePathLineEdit->setText(themepath);
	
}

void AppearancePage::reset_default_config()
{
	m_themepage->styleCombo->clear();
	
	config().set_property("Themer", "themepath", QString(getenv("HOME")).append("/.traverso/themes"));
	config().set_property("Themer", "currenttheme", "TraversoLight");
	config().set_property("Themer", "coloradjust", 100);
	QString systemstyle = QString(QApplication::style()->metaObject()->className()).remove("Q").remove("Style");
	config().set_property("Themer", "style", systemstyle);
	config().set_property("Themer", "usestylepallet", false);
	
	load_config();
}


ThemeConfigPage::ThemeConfigPage(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
}

void ThemeConfigPage::create_connections()
{
	connect(styleCombo, SIGNAL(currentIndexChanged(const QString)), this, SLOT(style_index_changed(const QString)));
	connect(themeSelecterCombo, SIGNAL(currentIndexChanged(const QString)), this, SLOT(theme_index_changed(const QString)));
	connect(useStylePalletCheckBox, SIGNAL(toggled(bool)), this, SLOT(use_selected_styles_pallet_checkbox_toggled(bool)));
	connect(colorAdjustBox, SIGNAL(valueChanged(int)), this, SLOT(color_adjustbox_changed(int)));
	connect(pathSelectButton, SIGNAL(clicked()), this, SLOT(dirselect_button_clicked()));
}

void ThemeConfigPage::style_index_changed(const QString& text)
{
	QApplication::setStyle(text);
	QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon);
	pathSelectButton->setIcon(icon);
	use_selected_styles_pallet_checkbox_toggled(useStylePalletCheckBox->isChecked());
}

void ThemeConfigPage::theme_index_changed(const QString & theme)
{
	int index = themeSelecterCombo->findText(theme);
	QString data = themeSelecterCombo->itemData(index).toString();
	QString path = config().get_property("Themer", "themepath", "").toString();
	
	if (data == "builtintheme") {
		themer()->use_builtin_theme(theme);
	} else {
		themer()->set_path_and_theme(path, theme);
	}
}

void ThemeConfigPage::use_selected_styles_pallet_checkbox_toggled(bool checked)
{
	if (checked) {
		QApplication::setPalette(QApplication::style()->standardPalette());
	} else {
		QApplication::setPalette(themer()->system_palette());
	}
}

void ThemeConfigPage::color_adjustbox_changed(int value)
{
	themer()->set_color_adjust_value(value);
}

void ThemeConfigPage::dirselect_button_clicked()
{
	QString dirName = QFileDialog::getExistingDirectory(this, 
			tr("Select default project dir"), 
			   themePathLineEdit->text());
	
	if (!dirName.isEmpty()) {
		themePathLineEdit->setText(dirName);
		update_theme_combobox(dirName);
	}
}

void ThemeConfigPage::update_theme_combobox(const QString& path)
{
	themeSelecterCombo->clear();
	
	foreach(QString key, themer()->get_builtin_themes()) {
		themeSelecterCombo->addItem(key, "builtintheme");
	}
	
	QDir themedir(path);
	foreach (QString dirName, themedir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
		QString filename = path + "/" + dirName + "/" + "traversotheme.xml";
		if (QFile::exists(filename) ) {
			themeSelecterCombo->addItem(dirName);
		}
	}
	
}



/****************************************/
/*            Keyboard                  */
/****************************************/

KeyboardPage::KeyboardPage(QWidget * parent)
	: ConfigPage(parent)
{
	m_configpage = new KeyboardConfigPage(this);
	mainLayout->addWidget(m_configpage);
	mainLayout->addStretch(1);
	
	load_config();
}

void KeyboardPage::load_config()
{
	int doubleFactTimeout = config().get_property("CCE", "doublefactTimeout", 200).toInt();
	int holdTimeout = config().get_property("CCE", "holdTimeout", 200).toInt();
	
	m_configpage->doubleFactTimeoutSpinBox->setValue(doubleFactTimeout);
	m_configpage->holdTimeoutSpinBox->setValue(holdTimeout);
}

void KeyboardPage::save_config()
{
	config().set_property("CCE", "doublefactTimeout", m_configpage->doubleFactTimeoutSpinBox->value());
	config().set_property("CCE", "holdTimeout", m_configpage->holdTimeoutSpinBox->value());
}

void KeyboardPage::reset_default_config()
{
	config().set_property("CCE", "doublefactTimeout", 200);
	config().set_property("CCE", "holdTimeout", 200);
	
	load_config();
}

KeyboardConfigPage::KeyboardConfigPage(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
}



/****************************************/
/*            DiskIO                    */
/****************************************/


DiskIOPage::DiskIOPage(QWidget * parent)
	: ConfigPage(parent)
{
	m_config = new MemoryConfigPage(this);
	mainLayout->addWidget(m_config);
	mainLayout->addStretch(5);
	
	load_config();
}

void DiskIOPage::load_config()
{
	double buffertime = config().get_property("Hardware", "PreBufferSize", 1.0).toDouble();
	m_config->bufferTimeSpinBox->setValue(buffertime);
}

void DiskIOPage::save_config()
{
	double buffertime = m_config->bufferTimeSpinBox->value();
	config().set_property("Hardware", "PreBufferSize", buffertime);
}

void DiskIOPage::reset_default_config()
{
	config().set_property("Hardware", "PreBufferSize", 1.0);
	load_config();
}

MemoryConfigPage::MemoryConfigPage(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
	QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
	reloadWarningLabel->setPixmap(icon.pixmap(22, 22));
}

//eof
