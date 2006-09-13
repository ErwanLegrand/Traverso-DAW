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

$Id: Import.cpp,v 1.10 2006/09/13 12:51:07 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include <QFileDialog>
#include <ReadSource.h>
#include "AudioClipList.h"

#include "Import.h"
#include "Utils.h"

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

	QString dir = m_fileName.left(splitpoint - 1) + "/";
	QString name = m_fileName.right(length - splitpoint);

	Project* project = pm().get_project();
	if (!project) {
		PWARN("No project loaded, can't import an AudioSource without a Project");
		return 0;
	}

	ReadSource* source = project->get_audiosource_manager()->get_readsource(m_fileName);
	
	if (! source ) {
		PMESG("AudioSource not found in acm, requesting new one");
		source = project->get_audiosource_manager()->new_readsource(dir, name, -1, 0, 0);
		if (! source) {
			PERROR("Can't import audiofile %s", QS_C(m_fileName));
		}
	}

	m_clip = project->get_audiosource_manager()->new_audio_clip(name);
	m_clip->set_song(m_track->get_song());
	m_clip->set_track(m_track);
	m_clip->set_track_start_frame(0);
	m_clip->set_audio_source(source);

	if (AudioClip* lastClip = m_track->get_cliplist().get_last()) {
		m_clip->set_track_start_frame( lastClip->get_track_end_frame() + 1);
	}
	
	return 1;
}

int Import::do_action()
{
	PENTER;
	
	ie().process_command(m_track->add_clip(m_clip, false));
	
	return 1;
}


int Import::undo_action()
{
	PENTER;
		
	ie().process_command(m_track->remove_clip(m_clip, false));
	
	return 1;
}


// eof


