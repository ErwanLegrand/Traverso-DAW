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

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>
#include <QStyleFactory>

#include "Pages.h"
#include "dialogs/ThemeModifierDialog.h"
#include <AudioDevice.h>
#if defined (ALSA_SUPPORT)
#include <AlsaDriver.h>
#endif
#include <Config.h>
#include <Utils.h>
#include <Themer.h>
#include <InputEngine.h>
#include "ContextPointer.h"
#include "TMainWindow.h"
#include <QDomDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>

#if defined (JACK_SUPPORT)
RELAYTOOL_JACK
#endif


/****************************************/
/*            AudioDriver               */
/****************************************/


AudioDriverConfigPage::AudioDriverConfigPage(QWidget *parent)
    : ConfigPage(parent)
{
	setupUi(this);
	periodBufferSizesList << 16 << 32 << 64 << 128 << 256 << 512 << 1024 << 2048 << 4096;
	
	m_mainLayout = qobject_cast<QVBoxLayout*>(layout());
	
	QStringList drivers = audiodevice().get_available_drivers();
	foreach(const QString &name, drivers) {
		driverCombo->addItem(name);
	}
	

	m_portaudiodrivers = new PaDriverPage(this);
	m_mainLayout->addWidget(m_portaudiodrivers);

	m_alsadevices = new AlsaDevicesPage(this);
	m_alsadevices->layout()->setMargin(0);
	m_mainLayout->addWidget(m_alsadevices);
	
	connect(driverCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(driver_combobox_index_changed(QString)));
	connect(restartDriverButton, SIGNAL(clicked()), this, SLOT(restart_driver_button_clicked()));
	connect(rateComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(rate_combobox_index_changed(QString)));
	
	load_config();
}

void AudioDriverConfigPage::save_config()
{
	config().set_property("Hardware", "samplerate", rateComboBox->currentText());
	int bufferindex = latencyComboBox->currentIndex();
	int buffersize = 1024;
	if (bufferindex >= 0) {
		buffersize = periodBufferSizesList.at(bufferindex);
	}
	config().set_property("Hardware", "buffersize", buffersize);
	
	config().set_property("Hardware", "drivertype", driverCombo->currentText());
	
	int playback=1, capture=1;
	if(duplexComboBox->currentIndex() == 1) {
		capture = 0;
	}
	
	if(duplexComboBox->currentIndex() == 2) {
		playback = 0;
	}
	
	config().set_property("Hardware", "capture", capture);
	config().set_property("Hardware", "playback", playback);

	
#if defined (ALSA_SUPPORT)
	int periods = m_alsadevices->periodsCombo->currentText().toInt();
	config().set_property("Hardware", "numberofperiods", periods);
	int index = m_alsadevices->devicesCombo->currentIndex();
	config().set_property("Hardware", "carddevice", m_alsadevices->devicesCombo->itemData(index));
	config().set_property("Hardware", "DitherShape", m_alsadevices->ditherShapeComboBox->currentText());
	
#endif

	
#if defined (PORTAUDIO_SUPPORT)
	int paindex = m_portaudiodrivers->driverCombo->currentIndex();
	config().set_property("Hardware", "pahostapi", m_portaudiodrivers->driverCombo->itemData(paindex));
#endif
	
	config().set_property("Hardware", "jackslave", jackTransportCheckBox->isChecked());
}

void AudioDriverConfigPage::reset_default_config()
{
	config().set_property("Hardware", "samplerate", 44100);
	config().set_property("Hardware", "buffersize", 512);
#if defined (ALSA_SUPPORT)
	config().set_property("Hardware", "drivertype", "ALSA");
        config().set_property("Hardware", "carddevice", "default");
	config().set_property("Hardware", "numberofperiods", 3);
	config().set_property("Hardware", "DitherShape", "None");
#elif defined (JACK_SUPPORT)
	if (libjack_is_present)
		config().set_property("Hardware", "drivertype", "Jack");
#else
	config().set_property("Hardware", "drivertype", "Null Driver");
#endif
	
#if defined (PORTAUDIO_SUPPORT)
#if defined (Q_WS_X11)
	config().set_property("Hardware", "pahostapi", "alsa");
#endif
#if defined (Q_WS_MAC)
	config().set_property("Hardware", "pahostapi", "coreaudio");
#endif
#if defined (Q_WS_WIN)
	config().set_property("Hardware", "pahostapi", "wmme");
#endif
#endif //end PORTAUDIO_SUPPORT
	
	config().set_property("Hardware", "capture", 1);
	config().set_property("Hardware", "playback", 1);
	
	config().set_property("Hardware", "jackslave", false);

	load_config();
}

void AudioDriverConfigPage::load_config( )
{
	int samplerate = config().get_property("Hardware", "samplerate", 44100).toInt();
	int buffersize = config().get_property("Hardware", "buffersize", 512).toInt();
#if defined (Q_WS_X11)
	QString driverType = config().get_property("Hardware", "drivertype", "ALSA").toString();
#else
	QString driverType = config().get_property("Hardware", "drivertype", "PortAudio").toString();
#endif	
	bool capture = config().get_property("Hardware", "capture", 1).toInt();
	bool playback = config().get_property("Hardware", "playback", 1).toInt();


	int driverTypeIndex = driverCombo->findText(driverType);
	if (driverTypeIndex >= 0) {
		driverCombo->setCurrentIndex(driverTypeIndex);
	}
	
	driver_combobox_index_changed(driverType);
	
	int buffersizeIndex = periodBufferSizesList.indexOf(buffersize);
	int samplerateIndex = rateComboBox->findText(QString::number(samplerate));
	
	rateComboBox->setCurrentIndex(samplerateIndex);
	latencyComboBox->setCurrentIndex(buffersizeIndex);
	
	
	if (capture && playback) {
		duplexComboBox->setCurrentIndex(0);
	} else if (playback) {
		duplexComboBox->setCurrentIndex(1);
	} else {
		duplexComboBox->setCurrentIndex(2);
	}
	
	int index;
	
#if defined (ALSA_SUPPORT)
	m_alsadevices->devicesCombo->clear();
	int periodsIndex = config().get_property("Hardware", "numberofperiods", 3).toInt();
	QString ditherShape = config().get_property("Hardware", "DitherShape", "None").toString();
	m_alsadevices->periodsCombo->setCurrentIndex(periodsIndex - 2);
	
	int shapeIndex = m_alsadevices->ditherShapeComboBox->findText(ditherShape);
	if (shapeIndex >=0) {
		m_alsadevices->ditherShapeComboBox->setCurrentIndex(shapeIndex);
	}
	
	// Always add the 'default' device
	m_alsadevices->devicesCombo->addItem(tr("System default"), "default");
	
	// Iterate over the maximum number of devices that can be in a system
	// according to alsa, and add them to the devices list.
	QString name;
	for (int i=0; i<6; ++i) {
		name = AlsaDriver::alsa_device_name(false, i);
		if (name != "") {
			QString card = "Card " + QString::number(i+1) + ":  ";
                        m_alsadevices->devicesCombo->addItem(card + name, name);
		}
	}
	
        QString defaultdevice =  config().get_property("Hardware", "carddevice", "default").toString();
	index = m_alsadevices->devicesCombo->findData(defaultdevice);
	if (index >= 0) {
		m_alsadevices->devicesCombo->setCurrentIndex(index);
	}
#endif
	
#if defined (PORTAUDIO_SUPPORT)
	m_portaudiodrivers->driverCombo->clear();
	QString defaulthostapi = "";

#if defined (Q_WS_X11)
	m_portaudiodrivers->driverCombo->addItem("ALSA", "alsa");
	m_portaudiodrivers->driverCombo->addItem("Jack", "jack");
	m_portaudiodrivers->driverCombo->addItem("OSS", "oss");
	defaulthostapi = "jack";
#endif

#if defined (Q_WS_MAC)
	m_portaudiodrivers->driverCombo->addItem("Core Audio", "coreaudio");
	m_portaudiodrivers->driverCombo->addItem("Jack", "jack");
	defaulthostapi = "coreaudio";
#endif

#if defined (Q_WS_WIN)
	m_portaudiodrivers->driverCombo->addItem("MME", "wmme");
	m_portaudiodrivers->driverCombo->addItem("Direct Sound", "directsound");
	m_portaudiodrivers->driverCombo->addItem("ASIO", "asio");
	defaulthostapi = "wmme";
#endif
	
	QString hostapi = config().get_property("Hardware", "pahostapi", defaulthostapi).toString();
	index = m_portaudiodrivers->driverCombo->findData(hostapi);
	if (index >= 0) {
		m_portaudiodrivers->driverCombo->setCurrentIndex(index);
	}
	
	update_latency_combobox();

#endif //end PORTAUDIO_SUPPORT

	bool usetransport = config().get_property("Hardware", "jackslave", false).toBool();
	jackTransportCheckBox->setChecked(usetransport);
}


void AudioDriverConfigPage::restart_driver_button_clicked()
{
        AudioDeviceSetup ads = audiodevice().get_device_setup();
	QString driver = driverCombo->currentText();
        ads.rate = rateComboBox->currentText().toInt();
        ads.bufferSize =  periodBufferSizesList.at(latencyComboBox->currentIndex());
	
        ads.playback = true;
        ads.capture = true;
	if(duplexComboBox->currentIndex() == 1) {
                ads.capture = false;
	}
	
	if(duplexComboBox->currentIndex() == 2) {
                ads.playback = false;
	}
	

        ads.cardDevice = "";
        ads.ditherShape = "None";


#if defined (ALSA_SUPPORT)
	int periods = m_alsadevices->periodsCombo->currentText().toInt();
        ads.ditherShape = m_alsadevices->ditherShapeComboBox->currentText();
	// The AlsaDriver retrieves it's periods number directly from config()
	// So there is no way to use the current selected one, other then
	// setting it now, and restoring it afterwards...
	int currentperiods = config().get_property("Hardware", "numberofperiods", 3).toInt();
	config().set_property("Hardware", "numberofperiods", periods);
	
	if (driver == "ALSA") {
		int index = m_alsadevices->devicesCombo->currentIndex();
                ads.cardDevice = m_alsadevices->devicesCombo->itemData(index).toString();
	}
#endif
	
#if defined (PORTAUDIO_SUPPORT)
	if (driver == "PortAudio") {
		int index = m_portaudiodrivers->driverCombo->currentIndex();
                ads.cardDevice = m_portaudiodrivers->driverCombo->itemData(index).toString();
	}
#endif
			
        ads.driverType = driver;
        audiodevice().set_parameters(ads);
	
#if defined (ALSA_SUPPORT)
	config().set_property("Hardware", "numberofperiods", currentperiods);
#endif
	
	config().set_property("Hardware", "jackslave", jackTransportCheckBox->isChecked());
}



void AudioDriverConfigPage::driver_combobox_index_changed(QString driver)
{
	m_mainLayout->removeWidget(m_alsadevices);
	m_mainLayout->removeWidget(m_portaudiodrivers);
	m_mainLayout->removeWidget(jackGroupBox);

	if (driver == "ALSA") {
		m_alsadevices->show();
		m_mainLayout->insertWidget(m_mainLayout->indexOf(driverConfigGroupBox) + 1, m_alsadevices);
	} else {
		m_alsadevices->hide();
		m_mainLayout->removeWidget(m_alsadevices);
	}

	if (driver == "PortAudio") {
		m_portaudiodrivers->show();
		m_mainLayout->insertWidget(m_mainLayout->indexOf(driverConfigGroupBox) + 1, m_portaudiodrivers);
	} else {
		m_portaudiodrivers->hide();
		m_mainLayout->removeWidget(m_portaudiodrivers);
	}
	
	if (driver == "Jack") {
		jackGroupBox->show();
		m_mainLayout->insertWidget(m_mainLayout->indexOf(driverConfigGroupBox) + 1, jackGroupBox);
	} else {
		jackGroupBox->hide();
		m_mainLayout->removeWidget(jackGroupBox);
	}
}


void AudioDriverConfigPage::update_latency_combobox( )
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

void AudioDriverConfigPage::rate_combobox_index_changed(QString )
{
	update_latency_combobox();
}



/****************************************/
/*            Appearance                */
/****************************************/


void AppearenceConfigPage::save_config()
{
	config().set_property("Themer", "themepath", themePathLineEdit->text());
        config().set_property("Themer", "currenttheme", themeSelecterCombo->currentText());
	config().set_property("Themer", "coloradjust", colorAdjustBox->value());
	config().set_property("Themer", "style", styleCombo->currentText());
	config().set_property("Themer", "usestylepallet", useStylePalletCheckBox->isChecked());
	config().set_property("Themer", "paintaudiorectified", rectifiedCheckBox->isChecked());
	config().set_property("Themer", "paintstereoaudioasmono", mergedCheckBox->isChecked());
	config().set_property("Themer", "drawdbgrid", dbGridCheckBox->isChecked());
	config().set_property("Themer", "paintwavewithoutline", paintAudioWithOutlineCheckBox->isChecked());
	config().set_property("Themer", "iconsize", iconSizeCombo->currentText());
	config().set_property("Themer", "toolbuttonstyle", toolbarStyleCombo->currentIndex());
	config().set_property("Themer", "supportediconsizes", supportedIconSizes);
	config().set_property("Themer", "transportconsolesize", transportConsoleCombo->currentText());
	config().set_property("Interface", "LanguageFile", languageComboBox->itemData(languageComboBox->currentIndex()));
        config().set_property("Themer", "VUOrientation", trackVUOrientationCheckBox->isChecked() ? Qt::Horizontal : Qt::Vertical);
}

void AppearenceConfigPage::load_config()
{
	QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon);
	pathSelectButton->setIcon(icon);
	QString themepath = config().get_property("Themer", "themepath",
				   QString(QDir::homePath()).append(".traverso/themes")).toString();
	
	
	QStringList keys = QStyleFactory::keys();
	keys.sort();
	foreach(const QString &key, keys) {
		styleCombo->addItem(key);
	}
	
	update_theme_combobox(themepath);
	
	
	// Hmm, there seems no way to get the name of the current
	// used style, using the classname minus Q and Style seems to do the trick.
	QString systemstyle = QString(QApplication::style()->metaObject()->className()).remove("Q").remove("Style");
	QString style = config().get_property("Themer", "style", systemstyle).toString();
        QString theme  = config().get_property("Themer", "currenttheme", "Traverso Light").toString();
	int coloradjust = config().get_property("Themer", "coloradjust", 100).toInt();
	bool usestylepallete = config().get_property("Themer", "usestylepallet", "").toBool();
	bool paintRectified = config().get_property("Themer", "paintaudiorectified", false).toBool();
	bool paintStereoAsMono = config().get_property("Themer", "paintstereoaudioasmono", false).toBool();
	bool paintWaveWithLines = config().get_property("Themer", "paintwavewithoutline", true).toBool();
	bool dbGrid = config().get_property("Themer", "drawdbgrid", false).toBool();
        Qt::Orientation orientation = (Qt::Orientation)config().get_property("Themer", "VUOrientation", Qt::Vertical).toInt();

	
	QString interfaceLanguage = config().get_property("Interface", "LanguageFile", "").toString();
	
	int index = styleCombo->findText(style);
	styleCombo->setCurrentIndex(index);
	index = themeSelecterCombo->findText(theme);
	themeSelecterCombo->setCurrentIndex(index);
	colorAdjustBox->setValue(coloradjust);
	useStylePalletCheckBox->setChecked(usestylepallete);
	themePathLineEdit->setText(themepath);
	rectifiedCheckBox->setChecked(paintRectified);
	mergedCheckBox->setChecked(paintStereoAsMono);
	dbGridCheckBox->setChecked(dbGrid);
	paintAudioWithOutlineCheckBox->setChecked(paintWaveWithLines);
        trackVUOrientationCheckBox->setChecked(orientation == Qt::Horizontal ? true : false);

	toolbarStyleCombo->clear();
	toolbarStyleCombo->addItem(tr("Icons only"));
	toolbarStyleCombo->addItem(tr("Text only"));
	toolbarStyleCombo->addItem(tr("Text beside Icons"));
	toolbarStyleCombo->addItem(tr("Text below Icons"));
	int tbstyle = config().get_property("Themer", "toolbuttonstyle", 0).toInt();
	toolbarStyleCombo->setCurrentIndex(tbstyle);

	// icon sizes of the toolbars
	QString iconsize = config().get_property("Themer", "iconsize", "22").toString();
	supportedIconSizes = config().get_property("Themer", "supportediconsizes", "16;22;32;48").toString();

	// if the list is empty, we should offer some default values. (The list can only be 
	// empty if someone deleted the values, but not the whole entry, in the config file.)
	if (supportedIconSizes.isEmpty()) {
		supportedIconSizes = "16;22;32;48";
	}

	QStringList iconSizesList = supportedIconSizes.split(";", QString::SkipEmptyParts);

	// check if the current icon size occurs in the list, if not, add it
	if (iconSizesList.lastIndexOf(iconsize) == -1) {
		iconSizesList << iconsize;
		iconSizesList.sort();
	}

	iconSizeCombo->clear();
	iconSizeCombo->addItems(iconSizesList);
	int iconsizeindex = iconSizeCombo->findText(iconsize);
	iconSizeCombo->setCurrentIndex(iconsizeindex);

	// and the same again for the icons size of the transport console
	QString trspsize = config().get_property("Themer", "transportconsolesize", "22").toString();
	iconSizesList = supportedIconSizes.split(";", QString::SkipEmptyParts);

	if (iconSizesList.lastIndexOf(iconsize) == -1) {
		iconSizesList << trspsize;
		iconSizesList.sort();
	}

	transportConsoleCombo->clear();
	transportConsoleCombo->addItems(iconSizesList);
	int trspsizeindex = iconSizeCombo->findText(trspsize);
	transportConsoleCombo->setCurrentIndex(trspsizeindex);
	
	
	int langIndex = languageComboBox->findData(interfaceLanguage);
	if (langIndex >= 0) {
		languageComboBox->setCurrentIndex(langIndex);
	}
}

void AppearenceConfigPage::reset_default_config()
{
	styleCombo->clear();
	
	config().set_property("Themer", "themepath", QString(QDir::homePath()).append("/.traverso/themes"));
        config().set_property("Themer", "currenttheme", "Traverso Light");
	config().set_property("Themer", "coloradjust", 100);
	QString systemstyle = QString(QApplication::style()->metaObject()->className()).remove("Q").remove("Style");
	config().set_property("Themer", "style", systemstyle);
	config().set_property("Themer", "usestylepallet", false);
	config().set_property("Themer", "paintaudiorectified", false);
	config().set_property("Themer", "paintstereoaudioasmono", false);
	config().set_property("Themer", "drawdbgrid", false);
	config().set_property("Themer", "paintwavewithoutline", true);
	config().set_property("Themer", "supportediconsizes", "16;22;32;48");
	config().set_property("Themer", "iconsize", "22");
	config().set_property("Themer", "toolbuttonstyle", 0);
	config().set_property("Interface", "LanguageFile", "");
        config().set_property("Themer", "VUOrientation", Qt::Vertical);

	load_config();
}


AppearenceConfigPage::AppearenceConfigPage(QWidget * parent)
	: ConfigPage(parent)
{
	setupUi(this);
	
	themeSelecterCombo->setInsertPolicy(QComboBox::InsertAlphabetically);
        m_colorModifierDialog = 0;
	
	languageComboBox->addItem(tr("Default Language"), "");
	foreach(const QString &lang, find_qm_files()) {
		languageComboBox->addItem(language_name_from_qm_file(lang), lang);
	}
	
	load_config();
	create_connections();
}

void AppearenceConfigPage::create_connections()
{
	connect(styleCombo, SIGNAL(currentIndexChanged(const QString)), this, SLOT(style_index_changed(const QString)));
	connect(themeSelecterCombo, SIGNAL(currentIndexChanged(const QString)), this, SLOT(theme_index_changed(const QString)));
	connect(useStylePalletCheckBox, SIGNAL(toggled(bool)), this, SLOT(use_selected_styles_pallet_checkbox_toggled(bool)));
	connect(pathSelectButton, SIGNAL(clicked()), this, SLOT(dirselect_button_clicked()));
	connect(colorAdjustBox, SIGNAL(valueChanged(int)), this, SLOT(color_adjustbox_changed(int)));
	connect(rectifiedCheckBox, SIGNAL(toggled(bool)), this, SLOT(theme_option_changed()));
	connect(mergedCheckBox, SIGNAL(toggled(bool)), this, SLOT(theme_option_changed()));
	connect(dbGridCheckBox, SIGNAL(toggled(bool)), this, SLOT(theme_option_changed()));
	connect(paintAudioWithOutlineCheckBox, SIGNAL(toggled(bool)), this, SLOT(theme_option_changed()));
        connect(trackVUOrientationCheckBox, SIGNAL(toggled(bool)), this, SLOT(theme_option_changed()));
        connect(editThemePushButton, SIGNAL(clicked()), this, SLOT(edit_theme_button_clicked()));
}

void AppearenceConfigPage::style_index_changed(const QString& text)
{
	QApplication::setStyle(text);
	QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon);
	pathSelectButton->setIcon(icon);
	use_selected_styles_pallet_checkbox_toggled(useStylePalletCheckBox->isChecked());
}

void AppearenceConfigPage::theme_index_changed(const QString & theme)
{
	int index = themeSelecterCombo->findText(theme);
	QString data = themeSelecterCombo->itemData(index).toString();
	QString path = config().get_property("Themer", "themepath", "").toString();
	
	if (data == "builtintheme") {
		themer()->use_builtin_theme(theme);
	} else {
                themer()->set_path_and_theme(path, theme + ".xml");
	}
}

void AppearenceConfigPage::use_selected_styles_pallet_checkbox_toggled(bool checked)
{
	if (checked) {
		QApplication::setPalette(QApplication::style()->standardPalette());
	} else {
		QApplication::setPalette(themer()->system_palette());
	}
}

void AppearenceConfigPage::color_adjustbox_changed(int value)
{
	themer()->set_color_adjust_value(value);
}

void AppearenceConfigPage::dirselect_button_clicked()
{
	QString path = themePathLineEdit->text();
	if (path.isEmpty()) {
		path = QDir::homePath();
	}
	QString dirName = QFileDialog::getExistingDirectory(this,
			tr("Select default project dir"), path);

	if (!dirName.isEmpty()) {
		themePathLineEdit->setText(dirName);
		update_theme_combobox(dirName);
	}
}

void AppearenceConfigPage::update_theme_combobox(const QString& path)
{
	themeSelecterCombo->clear();

        foreach(QString theme, themer()->get_builtin_themes()) {
                themeSelecterCombo->addItem(theme, "builtintheme");
	}
	
	QDir themedir(path);
        foreach (QString themeName, themedir.entryList(QDir::Files)) {
                themeName = themeName.remove(".xml");
                QString filename = path + "/" + themeName;
                if (QFile::exists(filename + ".xml") ) {
                        themeSelecterCombo->addItem(themeName);
		}
	}
	
}

void AppearenceConfigPage::theme_option_changed()
{
	config().set_property("Themer", "paintaudiorectified", rectifiedCheckBox->isChecked());
	config().set_property("Themer", "paintstereoaudioasmono", mergedCheckBox->isChecked());
	config().set_property("Themer", "drawdbgrid", dbGridCheckBox->isChecked());
	config().set_property("Themer", "paintwavewithoutline", paintAudioWithOutlineCheckBox->isChecked());
        config().set_property("Themer", "VUOrientation", trackVUOrientationCheckBox->isChecked() ? Qt::Horizontal : Qt::Vertical);
        themer()->load();
}

void AppearenceConfigPage::edit_theme_button_clicked()
{
        if (!m_colorModifierDialog) {
                m_colorModifierDialog = new ThemeModifierDialog(this);
        }
        m_colorModifierDialog->show();
}



/****************************************/
/*            Behavior                  */
/****************************************/

BehaviorConfigPage::BehaviorConfigPage(QWidget * parent)
	: ConfigPage(parent)
{
	setupUi(this);
	
	connect(&config(), SIGNAL(configChanged()), this, SLOT(update_follow()));
	
	load_config();
}


void BehaviorConfigPage::save_config()
{
	config().set_property("Sheet", "trackCreationCount", numberOfTrackSpinBox->value());
	config().set_property("PlayHead", "Follow", keepCursorVisibleCheckBox->isChecked());
	config().set_property("PlayHead", "Scrollmode", scrollModeComboBox->currentIndex());
	config().set_property("AudioClip", "SyncDuringDrag", resyncAudioCheckBox->isChecked());
	config().set_property("AudioClip", "LockByDefault", lockClipsCheckBox->isChecked());

	QString oncloseaction;
	if (saveRadioButton->isChecked()) {
		config().set_property("Project", "onclose", "save");
	} else if (askRadioButton->isChecked()) {
		config().set_property("Project", "onclose", "ask");
	} else {
		config().set_property("Project", "onclose", "dontsave");
	}
}

void BehaviorConfigPage::load_config()
{
	QString oncloseaction = config().get_property("Project", "onclose", "save").toString();
	int defaultNumTracks = config().get_property("Sheet", "trackCreationCount", 6).toInt();
	int scrollMode = config().get_property("PlayHead", "Scrollmode", 2).toInt();
	bool resyncAudio = config().get_property("AudioClip", "SyncDuringDrag", false).toBool();
	bool lockClips = config().get_property("AudioClip", "LockByDefault", false).toBool();
	
	numberOfTrackSpinBox->setValue(defaultNumTracks);
	scrollModeComboBox->setCurrentIndex(scrollMode);
	resyncAudioCheckBox->setChecked(resyncAudio);
	lockClipsCheckBox->setChecked(lockClips);
	
	if (oncloseaction == "save") {
		saveRadioButton->setChecked(true);
	} else if (oncloseaction == "ask") {
		askRadioButton->setChecked(true);
	} else {
		neverRadioButton->setChecked(true);
	}
	
	update_follow();

}


void BehaviorConfigPage::update_follow()
{
	bool keepCursorVisible = config().get_property("PlayHead", "Follow", true).toBool();
	keepCursorVisibleCheckBox->setChecked(keepCursorVisible);
	scrollModeComboBox->setEnabled(keepCursorVisible);
}

void BehaviorConfigPage::reset_default_config()
{
	config().set_property("Project", "onclose", "save");
	config().set_property("Sheet", "trackCreationCount", 6);
	config().set_property("PlayHead", "Follow", 0);
	config().set_property("PlayHead", "Scrollmode", 2);
	config().set_property("AudioClip", "SyncDuringDrag", false);
	config().set_property("AudioClip", "LockByDefault", false);
	
	load_config();
}




/****************************************/
/*            Keyboard                  */
/****************************************/


KeyboardConfigPage::KeyboardConfigPage(QWidget * parent)
	: ConfigPage(parent)
{
	setupUi(this);
	connect(keymapComboBox, SIGNAL(currentIndexChanged(const QString)),
		this, SLOT(keymap_index_changed(const QString)));
	
	load_config();
	
	update_keymap_combo();
}

void KeyboardConfigPage::load_config()
{
        int doubleFactTimeout = config().get_property("CCE", "doublefactTimeout", 220).toInt();
        int holdTimeout = config().get_property("CCE", "holdTimeout", 180).toInt();
        int jogByPassDistance = config().get_property("CCE", "jobbypassdistance", 70).toInt();
        int mouseClickTakesOverKeyboardNavigation = config().get_property("CCE", "mouseclicktakesoverkeyboardnavigation", false).toBool();
	
	doubleFactTimeoutSpinBox->setValue(doubleFactTimeout);
	holdTimeoutSpinBox->setValue(holdTimeout);
        mouseTreshHoldSpinBox->setValue(jogByPassDistance);

        if (mouseClickTakesOverKeyboardNavigation) {
                leftMouseClickRadioButton->setChecked(true);
        } else {
                mouseMoveRadioButton->setChecked(true);
        }
	
	QString defaultkeymap = config().get_property("CCE", "keymap", "default").toString();
	int index = keymapComboBox->findText(defaultkeymap);
	if (index >= 0) {
		keymapComboBox->setCurrentIndex(index);
	}
}

void KeyboardConfigPage::save_config()
{
	QString currentkeymap = config().get_property("CCE", "keymap", "default").toString();
	QString newkeymap = keymapComboBox->currentText();
	
	config().set_property("CCE", "doublefactTimeout", doubleFactTimeoutSpinBox->value());
	config().set_property("CCE", "holdTimeout", holdTimeoutSpinBox->value());
	config().set_property("CCE", "keymap", newkeymap);
        config().set_property("CCE", "jobbypassdistance", mouseTreshHoldSpinBox->value());
        config().set_property("CCE", "mouseclicktakesoverkeyboardnavigation", leftMouseClickRadioButton->isChecked());

	ie().set_double_fact_interval(doubleFactTimeoutSpinBox->value());
	ie().set_hold_sensitiveness(holdTimeoutSpinBox->value());
        cpointer().set_jog_bypass_distance(mouseTreshHoldSpinBox->value());
        cpointer().set_left_mouse_click_bypasses_jog(leftMouseClickRadioButton->isChecked());

	if (currentkeymap != newkeymap) {
		ie().init_map(newkeymap);
	}
}

void KeyboardConfigPage::reset_default_config()
{
        config().set_property("CCE", "doublefactTimeout", 220);
        config().set_property("CCE", "holdTimeout", 180);
	config().set_property("CCE", "keymap", "default");
        config().set_property("CCE", "jobbypassdistance", 70);
        config().set_property("CCE", "mouseclicktakesoverkeyboardnavigation", false);
        load_config();
}


void KeyboardConfigPage::keymap_index_changed(const QString& keymap)
{
	QString filename = ":/keymaps/" + keymap + ".xml";
	if ( ! QFile::exists(filename)) {
		filename = QDir::homePath() + "/.traverso/keymaps/" + keymap + ".xml";
	}
	
	QDomDocument doc("keymap");
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
		return;
	if (!doc.setContent(&file)) {
		file.close();
		return;
	}
	file.close();

	QDomElement root = doc.documentElement();
	QDomNode mapinfo = root.firstChildElement("KeymapInfo");
	QDomElement e = mapinfo.toElement();
	QString description = e.attribute("description", tr("No description set for this keymap"));
	
	descriptionTextEdit->setHtml(description);
}

void KeyboardConfigPage::update_keymap_combo()
{
	keymapComboBox->clear();
	
	QDir keymapdir(":/keymaps");
	foreach (QString filename, keymapdir.entryList(QDir::Files)) {
		keymapComboBox->insertItem(0, filename.remove(".xml"));
	}
	
	keymapdir.setPath(QDir::homePath() + "/.traverso/keymaps");
	foreach (QString filename, keymapdir.entryList(QDir::Files)) {
		keymapComboBox->insertItem(0, filename.remove(".xml"));
	}
}

void KeyboardConfigPage::on_exportButton_clicked()
{
	TMainWindow::instance()->export_keymap();
	QMessageBox::information( TMainWindow::instance(), tr("KeyMap Export"), 
		     tr("The exported keymap can be found here:\n\n %1").arg(QDir::homePath() + "/traversokeymap.html"),
		     QMessageBox::Ok);
}

void KeyboardConfigPage::on_printButton_clicked()
{
	QString kmap;
	TMainWindow::instance()->get_keymap(kmap);

	QPrinter printer(QPrinter::ScreenResolution);
	QPrintDialog printDialog(&printer, TMainWindow::instance());
	if (printDialog.exec() == QDialog::Accepted) {
                QTextDocument doc;
                doc.setHtml(kmap);
                doc.print(&printer);
	}
}




PerformanceConfigPage::PerformanceConfigPage(QWidget* parent)
	: ConfigPage(parent)
{
	delete layout();
	setupUi(this);
	
	load_config();


	// don't show it for now, it's not making sense with current opengl support
	useOpenGLCheckBox->hide();
#if defined (QT_OPENGL_SUPPORT)
	useOpenGLCheckBox->setEnabled(true);
#else
	useOpenGLCheckBox->setEnabled(false);
#endif
	QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
	reloadWarningLabel->setPixmap(icon.pixmap(22, 22));
}


void PerformanceConfigPage::load_config()
{
	int jogUpdateInterval = config().get_property("CCE", "jogupdateinterval", 28).toInt();
	bool useOpenGL = config().get_property("Interface", "OpenGL", false).toBool();
	
	jogUpdateIntervalSpinBox->setValue(1000 / jogUpdateInterval);
	useOpenGLCheckBox->setChecked(useOpenGL);	


	double buffertime = config().get_property("Hardware", "readbuffersize", 1.0).toDouble();
	bufferTimeSpinBox->setValue(buffertime);
}

void PerformanceConfigPage::save_config()
{
	config().set_property("Interface", "OpenGL", useOpenGLCheckBox->isChecked());
	config().set_property("CCE", "jogupdateinterval", 1000 / jogUpdateIntervalSpinBox->value());	
	double buffertime = bufferTimeSpinBox->value();
	config().set_property("Hardware", "readbuffersize", buffertime);
}

void PerformanceConfigPage::reset_default_config()
{
	config().set_property("CCE", "jogupdateinterval", 28);
	config().set_property("Interface", "OpenGL", false);
	config().set_property("Hardware", "readbuffersize", 1.0);
	load_config();
}


/****************************************/
/*            Recording                 */
/****************************************/

RecordingConfigPage::RecordingConfigPage(QWidget * parent)
	: ConfigPage(parent)
{
	setupUi(this);
	
	encodingComboBox->addItem("WAV", "wav");
	encodingComboBox->addItem("WavPack", "wavpack");
	encodingComboBox->addItem("WAV64", "w64");
	wavpackCompressionComboBox->addItem("Very high", "very_high");
	wavpackCompressionComboBox->addItem("High", "high");
	wavpackCompressionComboBox->addItem("Fast", "fast");
	
	connect(encodingComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(encoding_index_changed(int)));
	connect(useResamplingCheckBox, SIGNAL(stateChanged(int)), 
		this, SLOT(use_onthefly_resampling_checkbox_changed(int)));
	
	load_config();
}

void RecordingConfigPage::load_config()
{
	bool useResampling = config().get_property("Conversion", "DynamicResampling", true).toBool();
	if (useResampling) {
		use_onthefly_resampling_checkbox_changed(Qt::Checked);
	} else {
		use_onthefly_resampling_checkbox_changed(Qt::Unchecked);
	}
	
	QString recordFormat = config().get_property("Recording", "FileFormat", "wav").toString();
	if (recordFormat == "wavpack") {
		encoding_index_changed(1);
	} else if (recordFormat == "w64") {
		encoding_index_changed(2);
	} else {
		encoding_index_changed(0);
	}
	
	QString wavpackcompression = config().get_property("Recording", "WavpackCompressionType", "fast").toString();
	if (wavpackcompression == "very_high") {
		wavpackCompressionComboBox->setCurrentIndex(0);
	} else if (wavpackcompression == "high") {
		wavpackCompressionComboBox->setCurrentIndex(1);
	} else {
		wavpackCompressionComboBox->setCurrentIndex(2);
	}		
	
	int index = config().get_property("Conversion", "RTResamplingConverterType", DEFAULT_RESAMPLE_QUALITY).toInt();
	ontheflyResampleComboBox->setCurrentIndex(index);
	
	index = config().get_property("Conversion", "ExportResamplingConverterType", 1).toInt();
	exportDefaultResampleQualityComboBox->setCurrentIndex(index);
}

void RecordingConfigPage::save_config()
{
	config().set_property("Conversion", "DynamicResampling", useResamplingCheckBox->isChecked());
	config().set_property("Conversion", "RTResamplingConverterType", ontheflyResampleComboBox->currentIndex());
	config().set_property("Conversion", "ExportResamplingConverterType", exportDefaultResampleQualityComboBox->currentIndex());
	config().set_property("Recording", "FileFormat", encodingComboBox->itemData(encodingComboBox->currentIndex()).toString());
	config().set_property("Recording", "WavpackCompressionType", wavpackCompressionComboBox->itemData(wavpackCompressionComboBox->currentIndex()).toString());
	QString skipwvx = wavpackUseAlmostLosslessCheckBox->isChecked() ? "true" : "false";
	config().set_property("Recording", "WavpackSkipWVX", skipwvx);
}

void RecordingConfigPage::reset_default_config()
{
	config().set_property("Conversion", "DynamicResampling", true);
	config().set_property("Conversion", "RTResamplingConverterType", DEFAULT_RESAMPLE_QUALITY);
	config().set_property("Conversion", "ExportResamplingConverterType", 1);
	config().set_property("Recording", "FileFormat", "wav");
	config().set_property("Recording", "WavpackCompressionType", "fast");
	config().set_property("Recording", "WavpackSkipWVX", "false");
	
	load_config();
}


void RecordingConfigPage::encoding_index_changed(int index)
{
	encodingComboBox->setCurrentIndex(index);
	if (index != 1) {
		wacpackGroupBox->hide();
	} else {
		wacpackGroupBox->show();
	}
}

void RecordingConfigPage::use_onthefly_resampling_checkbox_changed(int state)
{
	if (state == Qt::Checked) {
		useResamplingCheckBox->setChecked(true);
		ontheflyResampleComboBox->setEnabled(true);
	} else {
		useResamplingCheckBox->setChecked(false);
		ontheflyResampleComboBox->setEnabled(false);
	}
}

