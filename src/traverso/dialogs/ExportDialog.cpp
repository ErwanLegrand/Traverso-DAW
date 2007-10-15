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


RELAYTOOL_WAVPACK
RELAYTOOL_FLAC
RELAYTOOL_MP3LAME
RELAYTOOL_VORBISENC

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
	
	audioTypeComboBox->addItem("WAV", "wav");
	audioTypeComboBox->addItem("AIFF", "aiff");
	if (libFLAC_is_present) {
		audioTypeComboBox->addItem("FLAC", "flac");
	}
	if (libwavpack_is_present) {
		audioTypeComboBox->addItem("WAVPACK", "wavpack");
	}
	if (libmp3lame_is_present) {
		audioTypeComboBox->addItem("MP3", "mp3");
	}
	if (libvorbisenc_is_present) {
		audioTypeComboBox->addItem("OGG", "ogg");
	}
	
	bitdepthComboBox->setCurrentIndex(bitdepthComboBox->findData(16));
	channelComboBox->setCurrentIndex(channelComboBox->findData(2));
	
	int rateIndex = sampleRateComboBox->findData(audiodevice().get_sample_rate());
	if (rateIndex == -1) {
		rateIndex = 0;
	}
	sampleRateComboBox->setCurrentIndex(rateIndex);
	resampleQualityComboBox->setCurrentIndex(1);
	
	connect(closeButton, SIGNAL(clicked()), this, SLOT(hide()));
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(audioTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(audio_type_changed(int)));
	
	
	// Mp3 Options Setup
	mp3MethodComboBox->addItem("Constant Bitrate", "cbr");
	mp3MethodComboBox->addItem("Average Bitrate", "abr");
	mp3MethodComboBox->addItem("Variable Bitrate", "vbr-new");
	
	mp3MinBitrateComboBox->addItem("32 - recommended", "32");
	mp3MinBitrateComboBox->addItem("64", "64");
	mp3MinBitrateComboBox->addItem("96", "96");
	mp3MinBitrateComboBox->addItem("128", "128");
	mp3MinBitrateComboBox->addItem("160", "160");
	mp3MinBitrateComboBox->addItem("192", "192");
	mp3MinBitrateComboBox->addItem("256", "256");
	mp3MinBitrateComboBox->addItem("320", "320");
	
	mp3MaxBitrateComboBox->addItem("32", "32");
	mp3MaxBitrateComboBox->addItem("64", "64");
	mp3MaxBitrateComboBox->addItem("96", "96");
	mp3MaxBitrateComboBox->addItem("128", "128");
	mp3MaxBitrateComboBox->addItem("160", "160");
	mp3MaxBitrateComboBox->addItem("192", "192");
	mp3MaxBitrateComboBox->addItem("256", "256");
	mp3MaxBitrateComboBox->addItem("320", "320");
	
	mp3MethodComboBox->setCurrentIndex(mp3MethodComboBox->findData("vbr-new"));
	mp3MinBitrateComboBox->setCurrentIndex(mp3MinBitrateComboBox->findData("32"));
	mp3MaxBitrateComboBox->setCurrentIndex(mp3MaxBitrateComboBox->findData("192"));
	
	mp3OptionsGroupBox->hide();
	connect(mp3MethodComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(mp3_method_changed(int)));
	
	
	// Ogg Options Setup
	oggMethodComboBox->addItem("Constant Bitrate", "manual");
	oggMethodComboBox->addItem("Variable Bitrate", "vbr");
	
	oggBitrateComboBox->addItem("45", "45");
	oggBitrateComboBox->addItem("64", "64");
	oggBitrateComboBox->addItem("96", "96");
	oggBitrateComboBox->addItem("112", "112");
	oggBitrateComboBox->addItem("128", "128");
	oggBitrateComboBox->addItem("160", "160");
	oggBitrateComboBox->addItem("192", "192");
	oggBitrateComboBox->addItem("224", "224");
	oggBitrateComboBox->addItem("256", "256");
	oggBitrateComboBox->addItem("320", "320");
	oggBitrateComboBox->addItem("400", "400");
	
	oggMethodComboBox->setCurrentIndex(oggMethodComboBox->findData("vbr"));
	oggBitrateComboBox->setCurrentIndex(oggBitrateComboBox->findData("160"));
	ogg_method_changed(oggMethodComboBox->findData("vbr"));
	
	oggOptionsGroupBox->hide();
	connect(oggMethodComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ogg_method_changed(int)));
	
	
	// WavPack option
	wacpackGroupBox->hide();
	wavpackCompressionComboBox->addItem("Very high", "very_high");
	wavpackCompressionComboBox->addItem("High", "high");
	wavpackCompressionComboBox->addItem("Fast", "fast");
	
	audio_type_changed(0);
	
	setMaximumSize(400, 250);
}

ExportDialog::~ ExportDialog( )
{}


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
// 		extraEncodingGroupBox->show();
		mp3OptionsGroupBox->show();
		oggOptionsGroupBox->hide();
		wacpackGroupBox->hide();
	}
	else if (newType == "ogg") {
// 		extraEncodingGroupBox->show();
		oggOptionsGroupBox->show();
		mp3OptionsGroupBox->hide();
		wacpackGroupBox->hide();
	}
	else if (newType == "wavpack") {
// 		extraEncodingGroupBox->show();
		wacpackGroupBox->show();
		mp3OptionsGroupBox->hide();
		oggOptionsGroupBox->hide();
	}
	else {
// 		extraEncodingGroupBox->hide();
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
		oggBitrateComboBox->show();
		oggBitrateLabel->show();
		oggQualitySlider->hide();
		oggQualityLabel->hide();
	}
	else {
		// VBR
		oggQualitySlider->show();
		oggQualityLabel->show();
		oggBitrateComboBox->hide();
		oggBitrateLabel->hide();
	}
}


void ExportDialog::on_startButton_clicked( )
{
	if (!is_safe_to_export()) {
		return;
	}
	
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
		m_exportSpec->extraFormat["skip_wvx"] = wavpackUseAlmostLosslessCheckBox->isChecked() ? "true" : "false";
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
	m_exportSpec->src_quality = resampleQualityComboBox->currentIndex();
	
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
	songProgressBar->setValue(progress);
}

void ExportDialog::update_overall_progress( int progress )
{
// 	overalProgressBar->setValue(progress);
}

void ExportDialog::render_finished( )
{
	songProgressBar->setValue(0);
// 	overalProgressBar->setValue(0);
	startButton->show();
	closeButton->show();
	abortButton->hide();

// 	on_closeButton_clicked();
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
		
		connect(m_project, SIGNAL(songExportProgressChanged(int)), this, SLOT(update_song_progress(int)));
		connect(m_project, SIGNAL(overallExportProgressChanged(int)), this, SLOT(update_overall_progress(int)));
		connect(m_project, SIGNAL(exportFinished()), this, SLOT(render_finished()));
		connect(m_project, SIGNAL(exportStartedForSong(Song*)), this, SLOT (set_exporting_song(Song*)));
	}
}



void ExportDialog::closeEvent(QCloseEvent * event)
{
/*	if (m_writingState != NO_STATE || !buttonBox->isEnabled()) {
		event->setAccepted(false);
		return;
	}*/
	QDialog::closeEvent(event);
}

void ExportDialog::reject()
{
/*	if (m_writingState == NO_STATE && buttonBox->isEnabled()) {
		hide();
	}*/
}

void ExportDialog::disable_ui_interaction()
{
/*	closeButton->setEnabled(false);
	exportWidget->setEnabled(false);
	optionsGroupBox->setEnabled(false);
	burnGroupBox->setEnabled(false);
	startButton->hide();
	abortButton->show();*/
}

void ExportDialog::enable_ui_interaction()
{
/*	m_writingState = NO_STATE;
	exportWidget->setEnabled(true);
	optionsGroupBox->setEnabled(true);
	burnGroupBox->setDisabled(cdDiskExportOnlyCheckBox->isChecked());
	closeButton->setEnabled(true);
	startButton->show();
	abortButton->hide();
	abortButton->setEnabled(true);
	progressBar->setValue(0);*/
}
