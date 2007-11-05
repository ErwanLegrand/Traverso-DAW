/**
    Copyright (C) 2005-2007 Remon Sijrier 
 
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

#include "ExportDialog.h"
#include "ui_ExportDialog.h"

#include "libtraversocore.h"

#include <QFileDialog>
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>

#include "Export.h"
#include "Config.h"
#include <AudioDevice.h>


RELAYTOOL_WAVPACK;
RELAYTOOL_FLAC;
RELAYTOOL_MP3LAME;
RELAYTOOL_VORBISENC;

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



ExportDialog::ExportDialog( QWidget * parent )
	: QDialog(parent)
	, m_exportSpec(0)
{
        setupUi(this);

	abortButton->hide();
	QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon);
	fileSelectButton->setIcon(icon);
	
	set_project(pm().get_project());
	
	//bitdepthComboBox->addItem("8", 8);
	bitdepthComboBox->addItem("16", 16);
	bitdepthComboBox->addItem("24", 24);
	bitdepthComboBox->addItem("32", 32);
	bitdepthComboBox->addItem("32 (float)", 1);
	
	channelComboBox->addItem("Mono", 1);
	channelComboBox->addItem("Stereo", 2);
	
	sampleRateComboBox->addItem("8.000 Hz", 8000);
	sampleRateComboBox->addItem("11.025 Hz", 11025);
	sampleRateComboBox->addItem("22.050 Hz", 22050);
	sampleRateComboBox->addItem("44.100 Hz", 44100);
	sampleRateComboBox->addItem("48.000 Hz", 48000);
	sampleRateComboBox->addItem("88.200 Hz", 88200);
	sampleRateComboBox->addItem("96.000 Hz", 96000);
	
	resampleQualityComboBox->addItem("Best", 0); // Best
	resampleQualityComboBox->addItem("High", 1); // Medium
	resampleQualityComboBox->addItem("Medium", 2); // Fastest
	resampleQualityComboBox->addItem("Fast", 4); // Linear (Should we use ZERO_HOLD(3) instead?)
	
	audioTypeComboBox->addItem("WAV", "wav");
	audioTypeComboBox->addItem("AIFF", "aiff");
	if (libFLAC_is_present) {
		audioTypeComboBox->addItem("FLAC", "flac");
	}
	if (libwavpack_is_present) {
		audioTypeComboBox->addItem("WAVPACK", "wavpack");
	}
#if defined MP3_ENCODE_SUPPORT
	if (libmp3lame_is_present) {
		audioTypeComboBox->addItem("MP3", "mp3");
	}
#endif
	if (libvorbisenc_is_present) {
		audioTypeComboBox->addItem("OGG", "ogg");
	}
	
	channelComboBox->setCurrentIndex(channelComboBox->findData(2));
	
	int rateIndex = sampleRateComboBox->findData(audiodevice().get_sample_rate());
	sampleRateComboBox->setCurrentIndex(rateIndex >= 0 ? rateIndex : 3);
	
	connect(closeButton, SIGNAL(clicked()), this, SLOT(hide()));
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(audioTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(audio_type_changed(int)));
	
	QString option;
	int index;
	bool checked;
	
	// Mp3 Options Setup
	mp3MethodComboBox->addItem("Constant Bitrate", "cbr");
	mp3MethodComboBox->addItem("Average Bitrate", "abr");
	mp3MethodComboBox->addItem("Variable Bitrate", "vbr-new");
	
	mp3MinBitrateComboBox->addItem("32 Kbps - recommended", "32");
	mp3MinBitrateComboBox->addItem("64 Kbps", "64");
	mp3MinBitrateComboBox->addItem("96 Kbps", "96");
	mp3MinBitrateComboBox->addItem("128 Kbps", "128");
	mp3MinBitrateComboBox->addItem("160 Kbps", "160");
	mp3MinBitrateComboBox->addItem("192 Kbps", "192");
	mp3MinBitrateComboBox->addItem("256 Kbps", "256");
	mp3MinBitrateComboBox->addItem("320 Kbps", "320");
	
	mp3MaxBitrateComboBox->addItem("32 Kbps", "32");
	mp3MaxBitrateComboBox->addItem("64 Kbps", "64");
	mp3MaxBitrateComboBox->addItem("96 Kbps", "96");
	mp3MaxBitrateComboBox->addItem("128 Kbps", "128");
	mp3MaxBitrateComboBox->addItem("160 Kbps", "160");
	mp3MaxBitrateComboBox->addItem("192 Kbps", "192");
	mp3MaxBitrateComboBox->addItem("256 Kbps", "256");
	mp3MaxBitrateComboBox->addItem("320 Kbps", "320");
	
	// First set to VBR, so that if we default to something else, it will trigger mp3_method_changed()
	index = mp3MethodComboBox->findData("vbr-new");
	mp3MethodComboBox->setCurrentIndex(index >=0 ? index : 0);
	connect(mp3MethodComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(mp3_method_changed(int)));
	
	option = config().get_property("ExportDialog", "mp3MethodComboBox", "vbr-new").toString();
	index = mp3MethodComboBox->findData(option);
	mp3MethodComboBox->setCurrentIndex(index >=0 ? index : 0);
	option = config().get_property("ExportDialog", "mp3MinBitrateComboBox", "32").toString();
	index = mp3MinBitrateComboBox->findData(option);
	mp3MinBitrateComboBox->setCurrentIndex(index >=0 ? index : 0);
	option = config().get_property("ExportDialog", "mp3MaxBitrateComboBox", "192").toString();
	index = mp3MaxBitrateComboBox->findData(option);
	mp3MaxBitrateComboBox->setCurrentIndex(index >=0 ? index : 0);
	
	mp3OptionsGroupBox->hide();
	
	
	// Ogg Options Setup
	oggMethodComboBox->addItem("Constant Bitrate", "manual");
	oggMethodComboBox->addItem("Variable Bitrate", "vbr");
	
	oggBitrateComboBox->addItem("45 Kbps", "45");
	oggBitrateComboBox->addItem("64 Kbps", "64");
	oggBitrateComboBox->addItem("96 Kbps", "96");
	oggBitrateComboBox->addItem("112 Kbps", "112");
	oggBitrateComboBox->addItem("128 Kbps", "128");
	oggBitrateComboBox->addItem("160 Kbps", "160");
	oggBitrateComboBox->addItem("192 Kbps", "192");
	oggBitrateComboBox->addItem("224 Kbps", "224");
	oggBitrateComboBox->addItem("256 Kbps", "256");
	oggBitrateComboBox->addItem("320 Kbps", "320");
	oggBitrateComboBox->addItem("400 Kbps", "400");
	
	// First set to VBR, so that if we default to something else, it will trigger ogg_method_changed()
	index = oggMethodComboBox->findData("vbr");
	oggMethodComboBox->setCurrentIndex(index >=0 ? index : 0);
	connect(oggMethodComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ogg_method_changed(int)));
	
	option = config().get_property("ExportDialog", "oggMethodComboBox", "vbr").toString();
	index = oggMethodComboBox->findData(option);
	oggMethodComboBox->setCurrentIndex(index >=0 ? index : 0);
	ogg_method_changed(index >=0 ? index : 0);
	option = config().get_property("ExportDialog", "oggBitrateComboBox", "160").toString();
	index = oggBitrateComboBox->findData(option);
	oggBitrateComboBox->setCurrentIndex(index >= 0 ? index : 0);
	
	oggOptionsGroupBox->hide();
	
	
	// WavPack option
	wacpackGroupBox->hide();
	wavpackCompressionComboBox->addItem("Very high", "very_high");
	wavpackCompressionComboBox->addItem("High", "high");
	wavpackCompressionComboBox->addItem("Fast", "fast");
	
	option = config().get_property("ExportDialog", "wavpackCompressionComboBox", "very_high").toString();
	index = wavpackCompressionComboBox->findData(option);
	wavpackCompressionComboBox->setCurrentIndex(index >= 0 ? index : 0);
	checked = config().get_property("ExportDialog", "skipWVXCheckBox", "false").toBool();
	skipWVXCheckBox->setChecked(checked);

	
	option = config().get_property("ExportDialog", "audioTypeComboBox", "wav").toString();
	index = audioTypeComboBox->findData(option);
	audioTypeComboBox->setCurrentIndex(index >= 0 ? index : 0);
	
	checked = config().get_property("ExportDialog", "normalizeCheckBox", "false").toBool();
	normalizeCheckBox->setChecked(checked);
	
	index = config().get_property("ExportDialog", "resampleQualityComboBox", "1").toInt();
	index = resampleQualityComboBox->findData(index);
	resampleQualityComboBox->setCurrentIndex(index >= 0 ? index : 1);
	
	option = config().get_property("ExportDialog", "bitdepthComboBox", "16").toString();
	index = bitdepthComboBox->findData(option);
	bitdepthComboBox->setCurrentIndex(index >= 0 ? index : 0);
	setMaximumSize(400, 250);
}


ExportDialog::~ ExportDialog( )
{
	config().set_property("ExportDialog", "mp3MethodComboBox", mp3MethodComboBox->itemData(mp3MethodComboBox->currentIndex()).toString());
	config().set_property("ExportDialog", "mp3MinBitrateComboBox", mp3MinBitrateComboBox->itemData(mp3MinBitrateComboBox->currentIndex()).toString());
	config().set_property("ExportDialog", "mp3MaxBitrateComboBox", mp3MaxBitrateComboBox->itemData(mp3MaxBitrateComboBox->currentIndex()).toString());
	config().set_property("ExportDialog", "oggMethodComboBox", oggMethodComboBox->itemData(oggMethodComboBox->currentIndex()).toString());
	config().set_property("ExportDialog", "oggBitrateComboBox", oggBitrateComboBox->itemData(oggBitrateComboBox->currentIndex()).toString());
	config().set_property("ExportDialog", "wavpackCompressionComboBox", wavpackCompressionComboBox->itemData(wavpackCompressionComboBox->currentIndex()).toString());
	config().set_property("ExportDialog", "audioTypeComboBox", audioTypeComboBox->itemData(audioTypeComboBox->currentIndex()).toString());
	config().set_property("ExportDialog", "normalizeCheckBox", normalizeCheckBox->isChecked());
	config().set_property("ExportDialog", "skipWVXCheckBox", skipWVXCheckBox->isChecked());
	config().set_property("ExportDialog", "resampleQualityComboBox", resampleQualityComboBox->itemData(resampleQualityComboBox->currentIndex()).toString());
	config().set_property("ExportDialog", "bitdepthComboBox", bitdepthComboBox->itemData(bitdepthComboBox->currentIndex()).toString());
}


bool ExportDialog::is_safe_to_export()
{
	PENTER;
	if (m_project->is_recording()) {
		info().warning(tr("Export during recording is not supported!"));
		return false;
	}
	
	QDir exportDir;
	QString dirName = exportDirName->text();
	
	if (!dirName.isEmpty() && !exportDir.exists(dirName)) {
		if (!exportDir.mkpath(dirName)) {
			info().warning(tr("Unable to create export directory! Please check permissions for this directory: %1").arg(dirName));
			return false;
		}
	}
	
	return true;
}


void ExportDialog::audio_type_changed(int index)
{
	QString newType = audioTypeComboBox->itemData(index).toString();
	
	if (newType == "mp3") {
		oggOptionsGroupBox->hide();
		wacpackGroupBox->hide();
		mp3OptionsGroupBox->show();
	}
	else if (newType == "ogg") {
		mp3OptionsGroupBox->hide();
		wacpackGroupBox->hide();
		oggOptionsGroupBox->show();
	}
	else if (newType == "wavpack") {
		mp3OptionsGroupBox->hide();
		oggOptionsGroupBox->hide();
		wacpackGroupBox->show();
	}
	else {
		mp3OptionsGroupBox->hide();
		wacpackGroupBox->hide();
		oggOptionsGroupBox->hide();
	}
	
	if (newType == "mp3" || newType == "ogg" || newType == "flac") {
		bitdepthComboBox->setCurrentIndex(bitdepthComboBox->findData(16));
		bitdepthComboBox->setDisabled(true);
	}
	else {
		bitdepthComboBox->setEnabled(true);
	}
}


void ExportDialog::mp3_method_changed(int index)
{
	QString method = mp3MethodComboBox->itemData(index).toString();
	
	if (method == "cbr") {
		mp3MinBitrateComboBox->hide();
		mp3MinBitrateLabel->hide();
		mp3MaxBitrateLabel->setText(tr("Bitrate"));
	}
	else if (method == "abr") {
		mp3MinBitrateComboBox->hide();
		mp3MinBitrateLabel->hide();
		mp3MaxBitrateLabel->setText(tr("Average Bitrate"));
	}
	else {
// 		VBR new or VBR old
		mp3MinBitrateComboBox->show();
		mp3MinBitrateLabel->show();
		mp3MaxBitrateLabel->setText(tr("Maximum Bitrate"));
	}
}


void ExportDialog::ogg_method_changed(int index)
{
	QString method = oggMethodComboBox->itemData(index).toString();
	
	if (method == "manual") {
		oggQualitySlider->hide();
		oggQualityLabel->hide();
		oggBitrateComboBox->show();
		oggBitrateLabel->show();
	}
	else {
		// VBR
		oggBitrateComboBox->hide();
		oggBitrateLabel->hide();
		oggQualitySlider->show();
		oggQualityLabel->show();
	}
}


void ExportDialog::on_startButton_clicked( )
{
	if (!is_safe_to_export()) {
		return;
	}
	
	connect(m_project, SIGNAL(songExportProgressChanged(int)), this, SLOT(update_song_progress(int)));
	connect(m_project, SIGNAL(overallExportProgressChanged(int)), this, SLOT(update_overall_progress(int)));
	connect(m_project, SIGNAL(exportFinished()), this, SLOT(render_finished()));
	connect(m_project, SIGNAL(exportStartedForSong(Song*)), this, SLOT (set_exporting_song(Song*)));
	
	// clear extraformats, it might be different now from previous runs!
	m_exportSpec->extraFormat.clear();
	
	
	QString audioType = audioTypeComboBox->itemData(audioTypeComboBox->currentIndex()).toString();
	if (audioType == "wav") {
		m_exportSpec->writerType = "sndfile";
		m_exportSpec->extraFormat["filetype"] = "wav";
	}
	else if (audioType == "aiff") {
		m_exportSpec->writerType = "sndfile";
		m_exportSpec->extraFormat["filetype"] = "aiff";
	}
	else if (audioType == "flac") {
		m_exportSpec->writerType = "flac";
	}
	else if (audioType == "wavpack") {
		m_exportSpec->writerType = "wavpack";
		m_exportSpec->extraFormat["quality"] = wavpackCompressionComboBox->itemData(wavpackCompressionComboBox->currentIndex()).toString();
		m_exportSpec->extraFormat["skip_wvx"] = skipWVXCheckBox->isChecked() ? "true" : "false";
	}
	else if (audioType == "mp3") {
		m_exportSpec->writerType = "lame";
		m_exportSpec->extraFormat["method"] = mp3MethodComboBox->itemData(mp3MethodComboBox->currentIndex()).toString();
		m_exportSpec->extraFormat["minBitrate"] = mp3MinBitrateComboBox->itemData(mp3MinBitrateComboBox->currentIndex()).toString();
		m_exportSpec->extraFormat["maxBitrate"] = mp3MaxBitrateComboBox->itemData(mp3MaxBitrateComboBox->currentIndex()).toString();
		m_exportSpec->extraFormat["quality"] = QString::number(mp3QualitySlider->value());
	}
	else if (audioType == "ogg") {
		m_exportSpec->writerType = "vorbis";
		m_exportSpec->extraFormat["mode"] = oggMethodComboBox->itemData(oggMethodComboBox->currentIndex()).toString();
		if (m_exportSpec->extraFormat["mode"] == "manual") {
			m_exportSpec->extraFormat["bitrateNominal"] = oggBitrateComboBox->itemData(oggBitrateComboBox->currentIndex()).toString();
			m_exportSpec->extraFormat["bitrateUpper"] = oggBitrateComboBox->itemData(oggBitrateComboBox->currentIndex()).toString();
		}
		else {
			m_exportSpec->extraFormat["vbrQuality"] = QString::number(oggQualitySlider->value());
		}
	}
	
	m_exportSpec->data_width = bitdepthComboBox->itemData(bitdepthComboBox->currentIndex()).toInt();
	m_exportSpec->channels = channelComboBox->itemData(channelComboBox->currentIndex()).toInt();
	m_exportSpec->sample_rate = sampleRateComboBox->itemData(sampleRateComboBox->currentIndex()).toInt();
	m_exportSpec->src_quality = resampleQualityComboBox->itemData(resampleQualityComboBox->currentIndex()).toInt();
	
	//TODO Make a ComboBox for this one too!
	m_exportSpec->dither_type = GDitherTri;
	
	
	if (allSongsButton->isChecked()) {
                m_exportSpec->allSongs = true;
	} else {
                m_exportSpec->allSongs = false;
	}
	
	m_exportSpec->exportdir = exportDirName->text();
	if (m_exportSpec->exportdir.size() > 1 && (m_exportSpec->exportdir.at(m_exportSpec->exportdir.size()-1).decomposition() != "/")) {
		m_exportSpec->exportdir = m_exportSpec->exportdir.append("/");
	}
	QString name = m_exportSpec->exportdir;
	QFileInfo fi(m_exportSpec->name);
	name += fi.completeBaseName() + ".toc";
	m_exportSpec->tocFileName = name;

	m_exportSpec->normalize = normalizeCheckBox->isChecked();
	m_exportSpec->isRecording = false;
	m_project->export_project(m_exportSpec);
	
	startButton->hide();
	closeButton->hide();
	abortButton->show();
}


void ExportDialog::on_closeButton_clicked()
{
	hide();
}


void ExportDialog::on_abortButton_clicked( )
{
	m_exportSpec->stop = true;
	m_exportSpec->breakout = true;
}


void ExportDialog::on_fileSelectButton_clicked( )
{
	if (!m_project) {
		info().information(tr("No project loaded, to export a project, load it first!"));
		return;
	}
	
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Choose/create an export directory"), m_exportSpec->exportdir);
	
	if (!dirName.isEmpty()) {
		exportDirName->setText(dirName);
	}
}


void ExportDialog::update_song_progress( int progress )
{
}

void ExportDialog::update_overall_progress( int progress )
{
	progressBar->setValue(progress);
}

void ExportDialog::render_finished( )
{
	disconnect(m_project, SIGNAL(songExportProgressChanged(int)), this, SLOT(update_song_progress(int)));
	disconnect(m_project, SIGNAL(overallExportProgressChanged(int)), this, SLOT(update_overall_progress(int)));
	disconnect(m_project, SIGNAL(exportFinished()), this, SLOT(render_finished()));
	disconnect(m_project, SIGNAL(exportStartedForSong(Song*)), this, SLOT (set_exporting_song(Song*)));
	
	startButton->show();
	closeButton->show();
	abortButton->hide();
	progressBar->setValue(0);
}

void ExportDialog::set_exporting_song( Song * song )
{
	QString name = tr("Progress of Sheet ") + 
		QString::number(m_project->get_song_index(song->get_id())) + ": " +
		song->get_title();
	
	currentProcessingSongName->setText(name);
}

void ExportDialog::set_project(Project * project)
{
	m_project = project;
	if (! m_project) {
		info().information(tr("No project loaded, to export a project, load it first!"));
		setEnabled(false);
		if (m_exportSpec) {
			delete m_exportSpec;
			m_exportSpec = 0;
		}
	} else {
		setEnabled(true);
		if (m_exportSpec) {
			delete m_exportSpec;
			m_exportSpec = 0;
		}
		m_exportSpec = new ExportSpecification;
		m_exportSpec->exportdir = m_project->get_root_dir() + "/Export/";
		m_exportSpec->renderfinished = false;
		exportDirName->setText(m_exportSpec->exportdir);
	}
}



void ExportDialog::closeEvent(QCloseEvent * event)
{
	if (closeButton->isHidden()) {
		event->setAccepted(false);
		return;
	}
	QDialog::closeEvent(event);
}

void ExportDialog::reject()
{
	if (closeButton->isHidden()) {
		return;
	}
	hide();
}

