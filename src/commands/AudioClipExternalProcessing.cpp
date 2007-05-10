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

#include <QInputDialog>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

AudioClipExternalProcessing::AudioClipExternalProcessing(AudioClip* clip)
	: Command(clip, tr("Clip: External Processing"))
{
	m_clip = clip;
	m_track = m_clip->get_track();
}


AudioClipExternalProcessing::~AudioClipExternalProcessing()
{}


int AudioClipExternalProcessing::prepare_actions()
{
	bool ok;
	
	QString command = QInputDialog::getText(
				0, 
				tr("Clip Processing"),
				tr("Enter sox command"),
				QLineEdit::Normal,
     				"",
				&ok );
	
	if (! ok || command.isEmpty()) {
		// Nothing typed in, or used hit cancel
		printf("No input command, or cancel button clicked\n");
		return -1;
	}
	
	printf("returned command: %s\n", QS_C(command));
	
	
	m_processor = new QProcess(this);
	m_processor->setProcessChannelMode(QProcess::MergedChannels);
	
	ReadSource* rs = resources_manager()->get_readsource(m_clip->get_readsource_id());
/*	if (! rs) {
		// This should NOT be possible, but just in case....
		printf("resources manager didn't return a resource for the to be processed audioclip (%lld) !!!!\n",
		       		m_clip->get_id());
		return -1;
	}*/
	
	QString name = rs->get_name();
	
	QString infilename = rs->get_filename();
	QString outfilename = pm().get_project()->get_audiosources_dir() + name.remove(".wav").remove(".")
							.append("-").append(command.simplified()).append(".wav");
	
	printf("infilename is %s\n", QS_C(infilename));
	printf("outfilename is %s\n", QS_C(outfilename));
	
	QStringList arguments;
	arguments.append(infilename);
	arguments.append(outfilename);
	arguments += command.split(QRegExp("\\s+"));
	
	printf("Complete command is %s\n", QS_C( arguments.join(" ")));
	
	m_processor->start("sox", arguments);
	
	QString result;
	
	if (! m_processor->waitForFinished() ) {
		if (!result.isEmpty())
		result = m_processor->errorString();
		if (!result.isEmpty())
			printf("output: \n %s", QS_C(result));
		return -1;
	} else {
		result = m_processor->readAllStandardOutput();
		if (!result.isEmpty())
			printf("output: \n %s", QS_C(result));
		
		QString dir = pm().get_project()->get_audiosources_dir();
	
		ReadSource* source = resources_manager()->create_new_readsource(dir, name);
		if (!source) {
			printf("ResourcesManager didn't return a ReadSource, most likely sox didn't understand your command\n");
			return -1;
		}
		
		m_resultingclip = resources_manager()->new_audio_clip(name.remove(".wav"));
		// Clips live at project level, we have to set its Song, Track and ReadSource explicitely!!
		m_resultingclip->set_song(m_clip->get_song());
		m_resultingclip->set_track(m_clip->get_track());
		resources_manager()->set_source_for_clip(m_resultingclip, source);
		m_resultingclip->set_track_start_frame(m_clip->get_track_start_frame());
		// FIXME!!!!!!!!!!!!!!!!!!!!
		m_resultingclip->init_gain_envelope();
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



// eof

