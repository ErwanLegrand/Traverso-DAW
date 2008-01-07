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

#include <libtraversocore.h>

#include <QFileDialog>
#include "ReadSource.h"
#include "Import.h"
#include "Utils.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Import::Import(const QString& fileName)
	: Command("")
{
	m_fileName = fileName;
	m_clip = 0;
	m_track = 0;
	m_silent = false;
	m_hasPosition = false;
	m_initialLength = TimeRef();
}


Import::Import(Track* track, const TimeRef& length, bool silent)
	: Command(track, "")
{
	m_track = track;
	m_clip = 0;
	m_silent = silent;
	m_hasPosition = false;
	m_initialLength = length;
	
	if (!m_silent) {
		setText(tr("Import Audio File"));
	} else {
		setText(tr("Insert Silence"));
	}
}


Import::Import(Track* track, const QString& fileName)
	: Command(track, tr("Import Audio File"))
{
	m_track = track;
	m_clip = 0;
	m_fileName = fileName;
	m_silent = false;
	m_hasPosition = false;
	m_initialLength = TimeRef();
}

Import::Import(Track* track, const QString& fileName, const TimeRef& position)
	: Command(track, tr("Import Audio File"))
{
	m_track = track;
	m_clip = 0;
	m_fileName = fileName;
	m_silent = false;
	m_initialLength = TimeRef();
	m_hasPosition = true;
	m_position = position;
}

Import::~Import()
{}

int Import::prepare_actions()
{
	PENTER;
	if (m_silent) {
		m_source = resources_manager()->get_silent_readsource();
		m_name = tr("Silence");
		m_fileName = tr("Silence");
		create_audioclip();
	} else if (m_fileName.isEmpty()) {
		m_fileName = QFileDialog::getOpenFileName(0,
				tr("Import audio source"),
				pm().get_project()->get_import_dir(),
				tr("All files (*);;Audio files (*.wav *.flac *.ogg *.mp3 *.wv *.w64)"));
		
		int splitpoint = m_fileName.lastIndexOf("/") + 1;
		QString dir = m_fileName.left(splitpoint - 1);
		
		if (m_fileName.isEmpty()) {
			PWARN("Import:: FileName is empty!");
			return -1;
		}
		
		pm().get_project()->set_import_dir(dir);
		
		if (create_readsource() == -1) {
			return -1;
		}
		create_audioclip();
	}
	
	return 1;
}

int Import::create_readsource()
{
	int splitpoint = m_fileName.lastIndexOf("/") + 1;
	int length = m_fileName.length();

	QString dir = m_fileName.left(splitpoint - 1) + "/";
	m_name = m_fileName.right(length - splitpoint);
	
	m_source = resources_manager()->import_source(dir, m_name);
	if (! m_source) {
		PERROR("Can't import audiofile %s", QS_C(m_fileName));
		return -1;
	}
	
	return 1;
}

void Import::create_audioclip()
{
	Q_ASSERT(m_track);
	m_clip = resources_manager()->new_audio_clip(m_name);
	resources_manager()->set_source_for_clip(m_clip, m_source);
	m_clip->set_song(m_track->get_song());
	m_clip->set_track(m_track);
	
	TimeRef startLocation;
	if (!m_hasPosition) {
		if (!m_track->get_cliplist().isEmpty()) {
			AudioClip* lastClip = m_track->get_cliplist().last();
			startLocation = lastClip->get_track_end_location();
		}
	} else {
		startLocation = m_position;
	}
	m_clip->set_track_start_location(startLocation);
	
	if (m_initialLength > qint64(0)) {
		m_clip->set_right_edge(m_initialLength + startLocation);
	}
}

void Import::set_track(Track * track)
{
	m_track = track;
}


void Import::set_position(const TimeRef& position)
{
	m_hasPosition = true;
	m_position = position;
}


int Import::do_action()
{
	PENTER;
	
	if (! m_clip) {
		create_audioclip();
	}
	
	Command::process_command(m_track->add_clip(m_clip, false));
	
	return 1;
}


int Import::undo_action()
{
	PENTER;
	Command::process_command(m_track->remove_clip(m_clip, false));
	return 1;
}


// eof


