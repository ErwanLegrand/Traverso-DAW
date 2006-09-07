/*
Copyright (C) 2005-2006 Remon Sijrier 

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

$Id: Import.cpp,v 1.9 2006/09/07 09:36:52 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include <QFileDialog>
#include <ReadSource.h>
#include "AudioClipList.h"

#include "Import.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Import::Import(Track* track)
		: Command(track, tr("Import Audio File"))
{
	m_track = track;
}


Import::Import(Track* track, const QString& fileName)
		: Command(track, tr("Import Audio File"))
{
	m_track = track;
	m_fileName = fileName;
}

Import::~Import()
{}

int Import::prepare_actions()
{
	PENTER;
	if (m_fileName.isEmpty()) {
		m_fileName = QFileDialog::getOpenFileName(0,
				tr("Import audio source"),
				getenv("HOME"),
				tr("All files (*);;Audio files (*.wav *.flac)"));
	}

	if (m_fileName.isEmpty()) {
		PWARN("FileName is empty!");
		return 0;
	}

	int splitpoint = m_fileName.lastIndexOf("/") + 1;
	int length = m_fileName.length();

	QString dir = m_fileName.left(splitpoint - 1);
	QString name = m_fileName.right(length - splitpoint);

	Project* project = pm().get_project();
	if (!project) {
		PWARN("No project loaded, can't import an AudioSource without a Project");
		return 0;
	}

	ReadSource* source = new ReadSource(0, dir, name);

	if (source->init() < 0) {
		PWARN("AudioSource init failed");
		delete source;
		return 0;
	}


	m_clip = new AudioClip(m_track, 0, name);

	int channels = source->get_channel_count();
	
	delete source;
	
	
	ReadSource* existingSource;

	for (int channel=0; channel < channels; channel++) {
		
		existingSource = project->get_audiosource_manager()->get_readsource( m_fileName, channel);
		
		if ( existingSource ) {
			m_clip->add_audio_source(existingSource, channel);
		} else {
			
			ReadSource* newSource = project->get_audiosource_manager()->new_readsource(dir, name, channel, 0, 0);

			if ( ! newSource) {
				PERROR("Failed to initialize ReadSource %s for channel %d", m_fileName.toAscii().data(), channel);
				return -1;
			}

			m_clip->add_audio_source(newSource, channel);
		}
	}

	if (AudioClip* lastClip = m_track->get_cliplist().get_last()) {
		m_clip->set_track_start_frame( lastClip->get_track_end_frame() + 1);
	}
	
	return 1;
}

int Import::do_action()
{
	PENTER;
	
	m_track->add_clip(m_clip);
	
	return 1;
}


int Import::undo_action()
{
	PENTER;
		
	m_track->remove_clip(m_clip);
	
	return 1;
}


// eof


