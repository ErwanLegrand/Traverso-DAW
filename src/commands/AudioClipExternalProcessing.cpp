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

#include "AudioClipExternalProcessing.h"

#include <AudioClip.h>
#include <AudioClipView.h>
#include <Track.h>
#include <InputEngine.h>
#include <ReadSource.h>
#include <ProjectManager.h>
#include <Project.h>
#include <ResourcesManager.h>
#include <Utils.h>
#include "Interface.h"
#include "Export.h"
#include "WriteSource.h"

#include <QThread>
#include <QFile>
#include <QApplication>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

class MergeThread : public QThread
{
public:
	MergeThread(ReadSource* source, QString outFileName) {
		m_outFileName = outFileName;
		m_readsource = source;
	}

	void run() {
		uint buffersize = 16384;
		audio_sample_t readbuffer[buffersize];
		audio_sample_t* mixdown = new audio_sample_t[2 * buffersize];
	
		ExportSpecification* spec = new ExportSpecification();
		spec->start_frame = 0;
		spec->end_frame = m_readsource->get_nframes();
		spec->total_frames = spec->end_frame;
		spec->pos = 0;
		spec->isRecording = false;
		spec->extension = "wav";
	
		spec->exportdir = pm().get_project()->get_root_dir() + "/audiosources/";
		spec->format = SF_FORMAT_WAV;
		spec->data_width = 16;
		spec->format |= SF_FORMAT_PCM_16;
		spec->channels = 2;
		spec->sample_rate = m_readsource->get_rate();
		spec->blocksize = buffersize;
		spec->name = m_outFileName;
		spec->dataF = new audio_sample_t[buffersize * 2];
	
		WriteSource* writesource = new WriteSource(spec);
	
		do {
			nframes_t this_nframes = std::min((nframes_t)(spec->end_frame - spec->pos), buffersize);
			nframes_t nframes = this_nframes;
		
			memset (spec->dataF, 0, sizeof (spec->dataF[0]) * nframes * spec->channels);
		
			for (int chan=0; chan < 2; ++chan) {
			
				m_readsource->file_read(chan, mixdown, spec->pos, nframes, readbuffer);
			
				for (uint x = 0; x < nframes; ++x) {
					spec->dataF[chan+(x*spec->channels)] = mixdown[x];
				}
			}
		
			writesource->process(buffersize);
		
			spec->pos += nframes;
			
		} while (spec->pos != spec->total_frames);
		
		writesource->finish_export();
		delete writesource;
		delete [] spec->dataF;
		delete spec;
		delete [] mixdown;
	}

private:
	QString m_outFileName;
	ReadSource* m_readsource;
};


AudioClipExternalProcessing::AudioClipExternalProcessing(AudioClip* clip)
	: Command(clip, tr("Clip: External Processing"))
{
	m_clip = clip;
	m_resultingclip = 0;
	m_track = m_clip->get_track();
}


AudioClipExternalProcessing::~AudioClipExternalProcessing()
{}


int AudioClipExternalProcessing::prepare_actions()
{
	ExternalProcessingDialog epdialog(Interface::instance(), this);
	
	epdialog.exec();
	
	if (! m_resultingclip) {
		return -1;
	}
	
	return 1;
}


int AudioClipExternalProcessing::do_action()
{
	PENTER;
	Command::process_command(m_track->remove_clip(m_clip, false));
	Command::process_command(m_track->add_clip(m_resultingclip, false));
	
	return 1;
}

int AudioClipExternalProcessing::undo_action()
{
	PENTER;
	Command::process_command(m_track->remove_clip(m_resultingclip, false));
	Command::process_command(m_track->add_clip(m_clip, false));
	return 1;
}



/************************************************************************/
/*				DIALOG 					*/
/************************************************************************/


ExternalProcessingDialog::ExternalProcessingDialog(QWidget * parent, AudioClipExternalProcessing* acep)
	: QDialog(parent)
{
	setupUi(this);
	m_acep = acep;
	m_queryOptions = false;
	m_merger = 0;
	
	m_processor = new QProcess(this);
	m_processor->setProcessChannelMode(QProcess::MergedChannels);
	
	command_lineedit_text_changed("sox");
	
	connect(m_processor, SIGNAL(readyReadStandardOutput()), this, SLOT(read_standard_output()));
	connect(m_processor, SIGNAL(started()), this, SLOT(process_started()));
	connect(m_processor, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(process_finished(int, QProcess::ExitStatus)));
	connect(m_processor, SIGNAL(error( QProcess::ProcessError)), this, SLOT(process_error(QProcess::ProcessError)));
	connect(argsComboBox, SIGNAL(activated(const QString&)), this, SLOT(arg_combo_index_changed(const QString&)));
	connect(programLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(command_lineedit_text_changed(const QString&)));
	connect(startButton, SIGNAL(clicked()), this, SLOT(prepare_for_external_processing()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

ExternalProcessingDialog::~ ExternalProcessingDialog()
{
	delete m_processor;
}


void ExternalProcessingDialog::prepare_for_external_processing()
{
	m_commandargs = argumentsLineEdit->text();
	
	if (m_commandargs.isEmpty()) {
		statusText->setText(tr("You have to supply an argument before starting the external process!"));
		return;
	}
	
	ReadSource* rs = resources_manager()->get_readsource(m_acep->m_clip->get_readsource_id());
	
	//This should NOT be possible, but just in case....
	if (! rs) {
		printf("ExternalProcessing:: resources manager did NOT return a resource for the to be processed audioclip (%lld) !!!!\n", m_acep->m_clip->get_id());
		return;
	}
	
	m_filename = rs->get_name();
	m_newClipName= rs->get_short_name().remove(".wav") + "-" + m_commandargs.simplified();
	
	m_infilename = rs->get_filename();
	// remove the extension and any dots that might confuse the external program, append the 
	// new name and again the extension.
	m_outfilename = pm().get_project()->get_audiosources_dir() + 
			m_filename.remove(".wav").remove(".").append("-").append(m_commandargs.simplified()).append(".wav");
	
	
	if (rs->get_channel_count() == 2 && rs->get_file_count() == 2) {
		m_merger = new MergeThread(rs, "merged.wav");
		connect(m_merger, SIGNAL(finished()), this, SLOT(start_external_processing()));
		m_merger->start();
		statusText->setHtml(tr("Preparing audio data to a format that can be used by <b>%1</b>, this can take a while for large files!").arg(m_program));
		progressBar->setMaximum(0);
	} else {	
		start_external_processing();
	}
}

void ExternalProcessingDialog::start_external_processing()
{
	m_arguments.clear();
	
	// On mac os x (and perhaps windows) the full path is given, so we check if the path contains sox!
	if (m_program.contains("sox")) {
		m_arguments.append("-S");
	}
	
	if (m_merger) {
		progressBar->setMaximum(100);
		m_arguments.append(pm().get_project()->get_audiosources_dir() + "merged.wav");
		delete m_merger;
	} else {
		m_arguments.append(m_infilename);
	}
	
	m_arguments.append(m_outfilename);
	m_arguments += m_commandargs.split(QRegExp("\\s+"));
	
	m_processor->start(m_program, m_arguments);
}

void ExternalProcessingDialog::read_standard_output()
{
	if (m_queryOptions) {
		QString result = m_processor->readAllStandardOutput();
		// On mac os x (and perhaps windows) the full path is given, so we check if the path contains sox!
		if (m_program.contains("sox")) {
			QStringList list = result.split("\n");
			foreach(QString string, list) {
				if (string.contains("Supported effects:") || string.contains("effect:") || string.contains("SUPPORTED EFFECTS:")) {
					result = string.remove("Supported effects:").remove("effect:").remove("SUPPORTED EFFECTS:");
					QStringList options = string.split(QRegExp("\\s+"));
					foreach(QString string, options) {
						if (!string.isEmpty())
							argsComboBox->addItem(string);
					}
				}
			}
		}
		return;
	}
	
	QString result = m_processor->readAllStandardOutput();
	
	if (result.contains("%")) {
		QStringList tokens = result.split(QRegExp("\\s+"));
		foreach(QString token, tokens) {
			if (token.contains("%")) {
				token = token.remove("%)");
				bool ok;
				int number = (int)token.toDouble(&ok);
				if (ok && number > progressBar->value()) {
					progressBar->setValue(number);
				}
				return;
			}
		}
	}
	
	statusText->append(result);
}

void ExternalProcessingDialog::process_started()
{
	statusText->clear();
}

void ExternalProcessingDialog::process_finished(int exitcode, QProcess::ExitStatus exitstatus)
{
	Q_UNUSED(exitcode);
	Q_UNUSED(exitstatus);
	
	if (m_queryOptions) {
		m_queryOptions = false;
		return;
	}
	
	if (exitstatus == QProcess::CrashExit) {
		statusText->setHtml(tr("Program <b>%1</b> crashed!").arg(m_program));
		return;
	}
	
	QString dir = pm().get_project()->get_audiosources_dir();
	
	// In case we used the merger, remove the file...
	QFile::remove(dir + "/merged.wav");
	
	
	QString result = m_processor->readAllStandardOutput();
	// print anything on command line we didn't catch
	printf("output: \n %s", QS_C(result));
		
	ReadSource* source = resources_manager()->import_source(dir, m_filename);
	if (!source) {
		printf("ResourcesManager didn't return a ReadSource, most likely sox didn't understand your command\n");
		return rejected();
	}
		
	m_acep->m_resultingclip = resources_manager()->new_audio_clip(m_newClipName);
	resources_manager()->set_source_for_clip(m_acep->m_resultingclip, source);
	// Clips live at project level, we have to set its Song, Track and ReadSource explicitely!!
	m_acep->m_resultingclip->set_song(m_acep->m_clip->get_song());
	m_acep->m_resultingclip->set_track(m_acep->m_clip->get_track());
	m_acep->m_resultingclip->set_track_start_frame(m_acep->m_clip->get_track_start_frame());
	
	close();
}

void ExternalProcessingDialog::query_options()
{
	m_queryOptions = true;
	argsComboBox->clear();
	m_processor->start(m_program, QStringList() << "-h");
}

void ExternalProcessingDialog::arg_combo_index_changed(const QString & text)
{
	argumentsLineEdit->setText(text);	
}

void ExternalProcessingDialog::command_lineedit_text_changed(const QString & text)
{
	m_program = text.simplified();
	if (m_program == "sox") {
		#if defined (Q_WS_MAC)
			m_program = qApp->applicationDirPath() + "/sox";
		#endif

		query_options();
		argsComboBox->show();
		argsComboBox->setToolTip(tr("Available arguments for the sox program"));
		return;
	}
	
	argsComboBox->hide();
}

void ExternalProcessingDialog::process_error(QProcess::ProcessError error)
{
	if (error == QProcess::FailedToStart) {
		statusText->setHtml(tr("Program <b>%1</b> not installed, or insufficient permissions to run!").arg(m_program));
	}
}


