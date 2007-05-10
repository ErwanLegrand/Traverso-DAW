/*
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

#include "ExportWidget.h"
#include "ui_ExportWidget.h"

#include "libtraversocore.h"

#include <QFileDialog>
#include <QByteArray>
#include <QMessageBox>

#include "Export.h"
#include <AudioDevice.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



ExportWidget::ExportWidget( QWidget * parent )
	: QDialog(parent)
	, m_burnprocess(0)
	, m_exportSpec(0)
{
        setupUi(this);
	
	stopButton->hide();
	
	set_project(pm().get_project());

        //bitdepthComboBox->insertItem(0, "8");
        bitdepthComboBox->insertItem(0, "16");
        bitdepthComboBox->insertItem(1, "24");
        bitdepthComboBox->insertItem(2, "32");
        bitdepthComboBox->insertItem(3, "32 (float)");

        channelComboBox->insertItem(0, "Stereo");
        channelComboBox->insertItem(1, "Mono");

        sampleRateComboBox->insertItem(0, "8.000 Hz");
        sampleRateComboBox->insertItem(1, "11.025 Hz");
        sampleRateComboBox->insertItem(2, "22.050 Hz");
        sampleRateComboBox->insertItem(3, "44.100 Hz");
        sampleRateComboBox->insertItem(4, "48.000 Hz");
        sampleRateComboBox->insertItem(5, "88.200 Hz");
        sampleRateComboBox->insertItem(6, "96.000 Hz");

	audioTypeComboBox->insertItem(0, "WAV");
	audioTypeComboBox->insertItem(1, "AIFF");
	audioTypeComboBox->insertItem(2, "FLAC");
	audioTypeComboBox->insertItem(3, "CD image (cdrdao)");

	bitdepthComboBox->setCurrentIndex(0);

        switch(audiodevice().get_sample_rate()) {
        case		8000:
                sampleRateComboBox->setCurrentIndex(0);
                break;
        case		11025:
                sampleRateComboBox->setCurrentIndex(1);
                break;
        case		22050:
                sampleRateComboBox->setCurrentIndex(2);
                break;
        case		44100:
                sampleRateComboBox->setCurrentIndex(3);
                break;
        case		48000:
                sampleRateComboBox->setCurrentIndex(4);
                break;
        case		88200:
                sampleRateComboBox->setCurrentIndex(5);
                break;
        case		96000:
                sampleRateComboBox->setCurrentIndex(6);
                break;
        }

        show_settings_view();

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(on_exportStartButton_clicked()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(on_cancelButton_clicked()));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(hide()));
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));


	
	// CD Burning stuff....
	
	m_burnprocess = new QProcess(this);
	m_burnprocess->setProcessChannelMode(QProcess::MergedChannels);
	QStringList env = QProcess::systemEnvironment();
	env << "LC_ALL=C";
	m_burnprocess->setEnvironment(env);
	m_writingState = NO_STATE;
	
	refreshButton->setIcon(QIcon(find_pixmap(":/refresh-16")));
		
	connect(m_burnprocess, SIGNAL(readyReadStandardOutput()), this, SLOT(read_standard_output()));
	connect(m_burnprocess, SIGNAL(started()), this, SLOT(cdrdao_process_started()));
	connect(m_burnprocess, SIGNAL(finished(int, QProcess::ExitStatus)),
		this, SLOT(cdrdao_process_finished(int, QProcess::ExitStatus)));
	connect(startButton, SIGNAL(clicked()), this, SLOT(start_burn_process()));
	connect(stopButton, SIGNAL(clicked()), this, SLOT(stop_burn_process()));
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(query_devices()));
		
	query_devices();
}

ExportWidget::~ ExportWidget( )
{}


bool ExportWidget::is_save_to_export()
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
	
	return  true;
}

void ExportWidget::on_exportStartButton_clicked( )
{
	if (!is_save_to_export()) {
		return;
	}
	
	cdburningWidget->setEnabled(false);
	
	show_progress_view();
	
	
	switch (audioTypeComboBox->currentIndex()) {
        case	0:
                m_exportSpec->format = SF_FORMAT_WAV;
                m_exportSpec->extension = ".wav";
                break;
        case	1:
                m_exportSpec->format = SF_FORMAT_AIFF;
                m_exportSpec->extension = ".aiff";
                break;
        case	2:
		m_exportSpec->format = SF_FORMAT_FLAC;
		m_exportSpec->extension = ".flac";
                break;
        }

	switch (bitdepthComboBox->currentIndex()) {
	//case		0:
	//        m_exportSpec->data_width = 8;
	//        m_exportSpec->format |= SF_FORMAT_PCM_U8;
	//	break;
	case		0:
		m_exportSpec->data_width = 16;
		m_exportSpec->format |= SF_FORMAT_PCM_16;
		break;
	case		1:
		m_exportSpec->data_width = 24;
		m_exportSpec->format |= SF_FORMAT_PCM_24;
		break;
	case		2:
		m_exportSpec->data_width = 32;
		m_exportSpec->format |= SF_FORMAT_PCM_32;
		break;
	case		3:
		m_exportSpec->data_width = 1;	// 1 means float
		m_exportSpec->format |= SF_FORMAT_FLOAT;
		break;
	}

	switch (channelComboBox->currentIndex()) {
	case		0:
		m_exportSpec->channels = 2;
		break;
	case		1:
		m_exportSpec->channels = 1;
		break;
	}

	switch (sampleRateComboBox->currentIndex()) {
	case		0:
		m_exportSpec->sample_rate = 8000;
		break;
	case		1:
		m_exportSpec->sample_rate = 11025;
		break;
	case		2:
		m_exportSpec->sample_rate = 22050;
		break;
	case		3:
		m_exportSpec->sample_rate = 44100;
		break;
	case		4:
		m_exportSpec->sample_rate = 48000;
		break;
	case		5:
		m_exportSpec->sample_rate = 88200;
		break;
	case		6:
		m_exportSpec->sample_rate = 96000;
		break;
	}

        //TODO Make a ComboBox for this one too!
        m_exportSpec->dither_type = GDitherTri;

        //TODO Make a ComboBox for this one too!
        m_exportSpec->src_quality = SRC_SINC_MEDIUM_QUALITY; // SRC_SINC_BEST_QUALITY  SRC_SINC_FASTEST  SRC_ZERO_ORDER_HOLD  SRC_LINEAR

        if (allSongsButton->isChecked()) {
                m_exportSpec->allSongs = true;
	} else {
                m_exportSpec->allSongs = false;
	}
	
	QString name = m_exportSpec->exportdir + "/";
	QFileInfo fi(m_exportSpec->name);
	name += fi.completeBaseName() + ".toc";
	m_exportSpec->tocFileName = name;

	m_exportSpec->normalize = normalizeCheckBox->isChecked();
	m_exportSpec->isRecording = false;
	m_project->export_project(m_exportSpec);
}


void ExportWidget::on_cancelButton_clicked()
{
	hide();
}


void ExportWidget::on_exportStopButton_clicked( )
{
        show_settings_view();
        m_exportSpec->stop = true;
	m_exportSpec->breakout = true;
}


void ExportWidget::on_fileSelectButton_clicked( )
{
        if (!m_project) {
                info().information(tr("No project loaded, to export a project, load it first!"));
                return;
        }

        QString dirName = QFileDialog::getExistingDirectory(this, tr("Choose/create an export directory"), m_exportSpec->exportdir);

        if (!dirName.isEmpty())
                exportDirName->setText(dirName);

}


void ExportWidget::on_selectionSongButton_clicked( )
{
        show_settings_view();
}

void ExportWidget::on_allSongsButton_clicked( )
{
        show_settings_view();
}

void ExportWidget::on_currentSongButton_clicked( )
{
        show_settings_view();
}

void ExportWidget::update_song_progress( int progress )
{
        songProgressBar->setValue(progress);
}

void ExportWidget::update_overall_progress( int progress )
{
        overalProgressBar->setValue(progress);
}

void ExportWidget::render_finished( )
{
        songProgressBar->setValue(0);
        overalProgressBar->setValue(0);

        show_settings_view();
	
	cdburningWidget->setEnabled(true);
}

void ExportWidget::set_exporting_song( Song * song )
{
        QString name = tr("Progress of Song ") + 
			QString::number(m_project->get_song_index(song->get_id())) + ": " +
			song->get_title();

	currentProcessingSongName->setText(name);
}

void ExportWidget::show_progress_view( )
{
	buttonBox->setEnabled(false);
	stackedWidget->setCurrentIndex(1);
}

void ExportWidget::show_settings_view( )
{
	stackedWidget->setCurrentIndex(0);
	buttonBox->setEnabled(true);
}

void ExportWidget::set_project(Project * project)
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
		exportDirName->setText(m_exportSpec->exportdir);
		
		connect(m_project, SIGNAL(songExportProgressChanged(int)), this, SLOT(update_song_progress(int)));
		connect(m_project, SIGNAL(overallExportProgressChanged(int)), this, SLOT(update_overall_progress(int)));
		connect(m_project, SIGNAL(exportFinished()), this, SLOT(render_finished()));
		connect(m_project, SIGNAL(exportStartedForSong(Song*)), this, SLOT (set_exporting_song(Song*)));
	}
}




/****************************************************************/
/*			CD EXPORT 				*/
/****************************************************************/


void ExportWidget::query_devices()
{
	if ( ! (m_burnprocess->state() == QProcess::NotRunning) ) {
		printf("query_devices: burnprocess still running!\n");
		return;
	}
	cdDeviceComboBox->clear();
	printf("quering devices...\n");
	m_writingState = QUERY_DEVICE;
	QStringList args;
	args << "drive-info";
	m_burnprocess->start("cdrdao", args);
}

void ExportWidget::unlock_device()
{
	if ( ! (m_burnprocess->state() == QProcess::NotRunning) ) {
		return;
	}
	
	m_writingState = UNLOCK_DEVICE;
	int index = cdDeviceComboBox->currentIndex();
	if (index == -1) {
		return;
	}
		
	QString device = cdDeviceComboBox->itemData(index).toString();
	
	QStringList args;
	args  << "unlock" << "--device" << device;
	m_burnprocess->start("cdrdao", args);
}


void ExportWidget::stop_burn_process()
{
	PENTER;
	
	update_cdburn_status(tr("Aborting CD Burn process ..."), NORMAL_MESSAGE);
	
	m_burnprocess->terminate();
	
	m_writingState = ABORT_BURN;
	stopButton->setEnabled(false);
}


void ExportWidget::start_burn_process()
{
	PENTER;
	
	if(!is_save_to_export()) {
		return;
	}
	
	cd_render();
	closeButton->setEnabled(false);
}


void ExportWidget::cdrdao_process_started()
{
	PENTER;
	
	if (m_writingState == BURNING) {
		return;
		update_cdburn_status(tr("Starting burn process...."), NORMAL_MESSAGE);
		progressBar->setMaximum(0);
	}

}

void ExportWidget::cdrdao_process_finished(int exitcode, QProcess::ExitStatus exitstatus)
{
	if (exitstatus == QProcess::CrashExit) {
		update_cdburn_status(tr("CD Burn process failed!"), ERROR_MESSAGE);
	}
	
	if (m_writingState == ABORT_BURN) {
		update_cdburn_status(tr("CD Burn process stopped on user request."), NORMAL_MESSAGE);
		startButton->show();
		stopButton->hide();
		stopButton->setEnabled(true);
	}
	
	if (m_writingState == BURNING) {
		update_cdburn_status(tr("CD Writing process finished!"), NORMAL_MESSAGE);
		startButton->show();
		stopButton->hide();
	}
	
	if (exitstatus == QProcess::CrashExit || m_writingState == ABORT_BURN) {
		unlock_device();
	}
	
	progressBar->setMaximum(100);
	progressBar->setValue(0);
	
	closeButton->setEnabled(true);
	
	m_writingState = NO_STATE;
	exportWidget->setEnabled(true);
	optionsGroupBox->setEnabled(true);
}

void ExportWidget::cd_render()
{
	PENTER;
	
	exportWidget->setEnabled(false);
	optionsGroupBox->setEnabled(false);
	
	update_cdburn_status(tr("Rendering Song(s)"), NORMAL_MESSAGE);
	
	m_exportSpec->extension = ".wav";
	m_exportSpec->data_width = 16;
	m_exportSpec->format = SF_FORMAT_WAV;
	m_exportSpec->format |= SF_FORMAT_PCM_16;
	m_exportSpec->channels = 2;
	m_exportSpec->sample_rate = 44100;
	m_exportSpec->writeToc = true;
	m_exportSpec->dither_type = GDitherTri;
	m_exportSpec->src_quality = SRC_SINC_MEDIUM_QUALITY; // SRC_SINC_BEST_QUALITY  SRC_SINC_FASTEST  SRC_ZERO_ORDER_HOLD  SRC_LINEAR
	if (cdAllSongsButton->isChecked()) {
		m_exportSpec->allSongs = true;
	} else {
		m_exportSpec->allSongs = false;
	}
	QString name = m_exportSpec->exportdir + "/";
	QFileInfo fi(m_exportSpec->name);
	name += fi.completeBaseName() + ".toc";
	m_exportSpec->tocFileName = name;

	m_exportSpec->normalize = cdNormalizeCheckBox->isChecked();
	m_exportSpec->isRecording = false;
	
	connect(m_project, SIGNAL(overallExportProgressChanged(int)), this, SLOT(cd_export_progress(int)));
	connect(m_project, SIGNAL(exportFinished()), this, SLOT(cd_export_finished()));
	m_project->export_project(m_exportSpec);
}

void ExportWidget::write_to_cd()
{
	PENTER;
	if ( ! (m_burnprocess->state() == QProcess::NotRunning) ) {
		m_writingState = NO_STATE;
		info().critical(tr("Burn process is still running, cannot start it twice!!"));
		return;
	}
	
	m_writingState = BURNING;
	
	startButton->hide();
	stopButton->show();
	
	progressBar->setValue(0);
	
	int index = cdDeviceComboBox->currentIndex();
	if (index == -1) {
		QMessageBox::information( 0, tr("No Burn Device"), 
					  tr("No burn Device available!"),
					     QMessageBox::Ok);
		return;
	}
		
	QString device = cdDeviceComboBox->itemData(index).toString();
	
	QStringList arguments;
	arguments << "write" << "--device" << device << "-n" << "--eject";
	if (speedComboBox->currentIndex() != 0) {
		arguments << "--speed" << speedComboBox->currentText().remove("x");
	}
	if (simulateCheckBox->isChecked()) {
		arguments <<"--simulate";
	}
	
	arguments << m_exportSpec->tocFileName;
	m_burnprocess->start("cdrdao", arguments);
}

void ExportWidget::cd_export_finished()
{
	PENTER;
	disconnect(m_project, SIGNAL(overallExportProgressChanged(int)), this, SLOT(cd_export_progress(int)));
	disconnect(m_project, SIGNAL(exportFinished()), this, SLOT(cd_export_finished()));
	
	if (cdDiskExportOnlyCheckBox->isChecked()) {
		update_cdburn_status(tr("Export to disk finished!"), NORMAL_MESSAGE);
		exportWidget->setEnabled(true);
		optionsGroupBox->setEnabled(true);
		return;
	}
	
	write_to_cd();
}

void ExportWidget::cd_export_progress(int progress)
{
	progressBar->setValue(progress);
}

void ExportWidget::update_cdburn_status(const QString& message, int type)
{
	if (type == NORMAL_MESSAGE) {
		QPalette palette;
		palette.setColor(QPalette::WindowText, QColor(Qt::black));
		cdExportInformationLabel->setPalette(palette);
		cdExportInformationLabel->setText(message);
	}
	
	if (type == ERROR_MESSAGE) {
		QPalette palette;
		palette.setColor(QPalette::WindowText, QColor(Qt::red));
		cdExportInformationLabel->setPalette(palette);
		cdExportInformationLabel->setText(message);
	}
}

void ExportWidget::read_standard_output()
{
	Q_ASSERT(m_burnprocess);
	
	if (m_writingState == QUERY_DEVICE) {
		char buf[1024];
		while(m_burnprocess->readLine(buf, sizeof(buf)) != -1) {
			QString stdout = buf;
			if (stdout.contains("/dev/")) {
				QString deviceName;
				QStringList strlist = stdout.split(QRegExp("\\s+"));
				for (int i=1; i<strlist.size(); ++i) {
					QString token = strlist.at(i);
					if (!token.contains("Rev:")) {
						deviceName += token + " ";
					} else {
						break;
					}
				}
				QString device = strlist.at(0);
				device = device.remove(":");
				cdDeviceComboBox->addItem(deviceName, device);
			}
		}
		return;
	}
	
	
	QString stdout = m_burnprocess->readAllStandardOutput();
	printf("CD Writing: %s\n", QS_C(stdout));
	
	
	if (stdout.contains("Disk seems to be written")) {
		QMessageBox::information( 0, tr("Disc not empty"), 
					  tr("Please, insert an empty disc and hit enter"),
					     QMessageBox::Ok);
		m_burnprocess->write("enter");
		return;
	}
	
	if (stdout.contains("Inserted disk is not empty and not appendable.")) {
		QMessageBox::information( 0, tr("Disc not empty"),
					  tr("Inserted disk is not empty, and cannot append data to it!"),
					     QMessageBox::Ok);
		return;
	}
		
	
	if (stdout.contains("Unit not ready")) {
		update_cdburn_status(tr("Waiting for CD Writer... (no disk inserted?)"), NORMAL_MESSAGE);
		progressBar->setMaximum(0);
		return;
	}
		
		
	if (stdout.contains("%") && stdout.contains("(") && stdout.contains(")")) {
		QStringList strlist = stdout.split(" ");
		if (strlist.size() > 7) {
			int written = strlist.at(1).toInt();
			int total = strlist.at(3).toInt();
			if (total == 0) {
				progressBar->setValue(0);
			} else {
				if (progressBar->maximum() == 0) {
					progressBar->setMaximum(100);
				}
				int progress = (100 * written) / total;
				progressBar->setValue(progress);
			}
		}
		return;
	}
	
	if (stdout.contains("Writing track")) {
		QStringList strlist = stdout.split(" ");
		if (strlist.size() > 3) {
			QString text = strlist.at(0) + " " + strlist.at(1) + " " + strlist.at(2);
			update_cdburn_status(text, NORMAL_MESSAGE);
		}
		return;
	}	
	
}

void ExportWidget::closeEvent(QCloseEvent * event)
{
	if (m_writingState != NO_STATE || !buttonBox->isEnabled()) {
		event->setAccepted(false);
		return;
	}
	QDialog::closeEvent(event);
}
