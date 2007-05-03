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
#include <QAbstractItemModel>
#include <QStringListModel>
#include <QByteArray>

#include "Export.h"
#include <AudioDevice.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



ExportWidget::ExportWidget( QWidget * parent )
	: QDialog(parent)
{
        setupUi(this);
	
	m_layout = new QHBoxLayout(this);
	m_layout->setMargin(0);
	setLayout(m_layout);

        m_project = pm().get_project();

        if (!m_project) {
                info().information(tr("No project loaded, to export a project, load it first!"));
        } else {
                spec = new ExportSpecification;
                spec->exportdir = m_project->get_root_dir() + "/Export/";
                exportDirName->setText(spec->exportdir);
		
		connect(m_project, SIGNAL(songExportProgressChanged(int)), this, SLOT(update_song_progress(int)));
                connect(m_project, SIGNAL(overallExportProgressChanged(int)), this, SLOT(update_overall_progress(int)));
                connect(m_project, SIGNAL(exportFinished()), this, SLOT(render_finished()));
                connect(m_project, SIGNAL(exportStartedForSong(Song*)), this, SLOT (set_exporting_song(Song*)));
        }

        bitdepthComboBox->insertItem(0, "16");
        bitdepthComboBox->insertItem(1, "24");
        bitdepthComboBox->insertItem(2, "32");
        bitdepthComboBox->insertItem(3, "32 (float)");

        channelComboBox->insertItem(0, "Stereo");
        channelComboBox->insertItem(1, "Mono");

        sampleRateComboBox->insertItem(0, "22.050 Hz");
        sampleRateComboBox->insertItem(1, "44.100 Hz");
        sampleRateComboBox->insertItem(2, "48.000 Hz");
        sampleRateComboBox->insertItem(3, "88.200 Hz");
        sampleRateComboBox->insertItem(4, "96.000 Hz");

	audioTypeComboBox->insertItem(0, "WAV");
	audioTypeComboBox->insertItem(1, "AIFF");
	audioTypeComboBox->insertItem(2, "FLAC");
	audioTypeComboBox->insertItem(3, "CD image (cdrdao)");


        switch(audiodevice().get_sample_rate()) {
        case		22050:
                sampleRateComboBox->setCurrentIndex(0);
                break;
        case		44100:
                sampleRateComboBox->setCurrentIndex(1);
                break;
        case		48000:
                sampleRateComboBox->setCurrentIndex(2);
                break;
        case		88200:
                sampleRateComboBox->setCurrentIndex(3);
                break;
        case		96000:
                sampleRateComboBox->setCurrentIndex(4);
                break;
        }

        show_settings_view();

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(on_exportStartButton_clicked()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(on_cancelButton_clicked()));
}

ExportWidget::~ ExportWidget( )
{}

void ExportWidget::on_exportStartButton_clicked( )
{
	if (m_project->is_recording()) {
		info().warning(tr("Export during recording is not supported!"));
		return;
	}
	
        QDir exportDir;
        QString dirName = exportDirName->text();

        if (!dirName.isEmpty() && !exportDir.exists(dirName)) {
                if (!exportDir.mkpath(dirName)) {
                        info().warning(tr("Unable to create export directory! Please check permissions for this directory: %1").arg(dirName));
                        return;
                }
        }

	
	show_progress_view();
	
	
	switch (audioTypeComboBox->currentIndex()) {
        case	0:
                spec->format = SF_FORMAT_WAV;
                spec->extension = ".wav";
                break;
        case	1:
                spec->format = SF_FORMAT_AIFF;
                spec->extension = ".aiff";
                break;
        case	2:
		spec->format = SF_FORMAT_FLAC;
		spec->extension = ".flac";
                break;
        case	3:
		spec->format = SF_FORMAT_WAV;
		spec->extension = ".wav";
		break;
        }

	// check if CD-format is required
	if (audioTypeComboBox->currentIndex() == 3) {

                spec->data_width = 16;
	        spec->format |= SF_FORMAT_PCM_16;
                spec->channels = 2;
		spec->sample_rate = 44100;
		spec->writeToc = true;

	} else {

   	     switch (bitdepthComboBox->currentIndex()) {
	        case		0:
        	        spec->data_width = 16;
	                spec->format |= SF_FORMAT_PCM_16;
                	break;
        	case		1:
	                spec->data_width = 24;
                	spec->format |= SF_FORMAT_PCM_24;
        	        break;
	        case		2:
        	        spec->data_width = 32;
	                spec->format |= SF_FORMAT_PCM_32;
                	break;
        	case		3:
	                spec->data_width = 1;	// 1 means float
                	spec->format |= SF_FORMAT_FLOAT;
        	        break;
	        }

	        switch (channelComboBox->currentIndex()) {
        	case		0:
	                spec->channels = 2;
                	break;
        	case		1:
	                spec->channels = 1;
                	break;
        	}
	
        	switch (sampleRateComboBox->currentIndex()) {
	        case		0:
                	spec->sample_rate = 22050;
        	        break;
	        case		1:
                	spec->sample_rate = 44100;
        	        break;
	        case		2:
                	spec->sample_rate = 48000;
        	        break;
	        case		3:
                	spec->sample_rate = 88200;
        	        break;
	        case		4:
                	spec->sample_rate = 96000;
        	        break;
	        }
	}

        //TODO Make a ComboBox for this one too!
        spec->dither_type = GDitherTri;

        //TODO Make a ComboBox for this one too!
        spec->src_quality = SRC_SINC_MEDIUM_QUALITY; // SRC_SINC_BEST_QUALITY  SRC_SINC_FASTEST  SRC_ZERO_ORDER_HOLD  SRC_LINEAR

        if (allSongsButton->isChecked()) {
                spec->allSongs = true;
	} else {
                spec->allSongs = false;
	}
	
	spec->isRecording = false;

        m_project->export_project(spec);
}


void ExportWidget::on_cancelButton_clicked()
{
	hide();
}


void ExportWidget::on_exportStopButton_clicked( )
{
        show_settings_view();
        spec->stop = true;
}


void ExportWidget::on_fileSelectButton_clicked( )
{
        if (!m_project) {
                info().information(tr("No project loaded, to export a project, load it first!"));
                return;
        }

        QString dirName = QFileDialog::getExistingDirectory(this, tr("Choose/create an export directory"), spec->exportdir);

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

        if (!spec->stop) {
                hide();
	}

        show_settings_view();
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
	optionsWidget->hide();
	progressWidget->show();
	m_layout->removeWidget(optionsWidget);
	m_layout->addWidget(progressWidget);
	resize(370, 150);
}

void ExportWidget::show_settings_view( )
{
	m_layout->removeWidget(progressWidget);
	m_layout->addWidget(optionsWidget);
	optionsWidget->show();
	progressWidget->hide();
	resize(370, 300);
}



//eof

