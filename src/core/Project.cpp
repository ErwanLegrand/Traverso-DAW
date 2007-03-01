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

*/

#include <QFile>
#include <QDir>
#include <QTextStream>

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#include "Project.h"
#include "Song.h"
#include "ProjectManager.h"
#include "Information.h"
#include "AudioSourceManager.h"
#include "Export.h"
#include "AudioDevice.h"
#include "Config.h"
#include "ContextPointer.h"
#include "Utils.h"

#define PROJECT_FILE_VERSION 	1

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Project::Project(const QString& pTitle)
	: ContextItem(), title(pTitle)
{
	PENTERCONS;
	currentSongId = 1;
	engineer = "";

	rootDir = config().get_property("Project", "directory", "/directory/unknown").toString() + title;
	sourcesDir = rootDir + "/audiosources";
	m_rate = audiodevice().get_sample_rate();
	m_bitDepth = audiodevice().get_bit_depth();

	asmanager = new AudioSourceManager();

	cpointer().add_contextitem(this);
}


Project::~Project()
{
	PENTERDES;
	cpointer().remove_contextitem(this);

	foreach(Song* song, songList) {
		 song->disconnect_from_audiodevice_and_delete();
	}

	delete asmanager;
}


int Project::create(int pNumSongs)
{
	PENTER;
	PMESG("Creating new project %s  NumSongs=%d", title.toAscii().data(), pNumSongs);

	QDir dir;
	if (dir.mkdir(rootDir) < 0) {
		PERROR("Cannot create dir %s", rootDir.toAscii().data());
		return -1;
	}

	if (dir.mkdir(sourcesDir) < 0) {
		PERROR("Cannot create dir %s", sourcesDir.toAscii().data());
		return -1;
	}

	QString peaksDir = rootDir + "/peakfiles/";

	if (dir.mkdir(peaksDir) < 0) {
		PERROR("Cannot create dir %s", peaksDir.toAscii().data());
		return -1;
	}

	for (int i=0; i< pNumSongs; i++) {
		Song* song = new Song(this, i+1);
		songList.insert(i+1, song);
		emit songAdded();
	}

	set_current_song( 1 );
	
	m_id = create_id();

	save();
	info().information("New project created");
	return 1;
}


int Project::load() // try to load the project by its title
{
	PENTER;
	QDomDocument doc("Project");
	QFile file(rootDir + "/project.traverso");

	if (!file.open(QIODevice::ReadOnly))
	{
		file.close();
		PWARN("Cannot open project properties file");
		return -1;
	}

	QString errorMsg;
	if (!doc.setContent(&file, &errorMsg))
	{
		file.close();
		PWARN("Cannot set content of XML file (%s)", errorMsg.toAscii().data());
		return -1;
	}

	file.close();

	QDomElement docElem = doc.documentElement();
	QDomNode propertiesNode = docElem.firstChildElement("Properties");
	QDomElement e = propertiesNode.toElement();
	
	if (e.attribute("projectfileversion", "").toInt() != PROJECT_FILE_VERSION) {
		PERROR("Project File Version does not match, cannot load project :-(");
		info().warning("Project File Version does not match, cannot load project :-(");
		return -1;
	}

	title = e.attribute( "title", "" );
	engineer = e.attribute( "engineer", "" );
	m_description = e.attribute( "description", "No description set");
	currentSongId = e.attribute( "currentSongId", "" ).toInt();
	m_rate = e.attribute( "rate", "" ).toInt();
	m_bitDepth = e.attribute( "bitdepth", "" ).toInt();
	m_id = e.attribute("id", "0").toLongLong();
	if (m_id == 0) m_id = create_id();
	
	
	
	// Load all the AudioSources for this project
	
	QDomNode asmNode = docElem.firstChildElement("AudioSourcesManager");
	asmanager->set_state(asmNode);


	QDomNode songsNode = docElem.firstChildElement("Songs");
	QDomNode songNode = songsNode.firstChild();

	// Load all the Songs
	while(!songNode.isNull())
	{
		Song* song = new Song(this, songNode);
		int id = songNode.toElement().attribute( "id", "" ).toInt();
		songList.insert(id, song);
		emit songAdded();
		songNode = songNode.nextSibling();
	}

	set_current_song(currentSongId);

	info().information( tr("Project loaded (%1)").arg(title) );

	return 1;
}


int Project::save()
{
	PENTER;
	QDomDocument doc("Project");
	QString fileName = rootDir + "/project.traverso";
	QFile data( fileName );

	if (data.open( QIODevice::WriteOnly ) ) {
		QDomElement projectNode = doc.createElement("Project");
		QDomElement properties = doc.createElement("Properties");

		properties.setAttribute("title", title);
		properties.setAttribute("engineer", engineer);
		properties.setAttribute("description", m_description);
		properties.setAttribute("currentSongId", currentSongId);
		properties.setAttribute("rate", m_rate);
		properties.setAttribute("bitdepth", m_bitDepth);
		properties.setAttribute("projectfileversion", PROJECT_FILE_VERSION);
		properties.setAttribute("id", m_id);
		projectNode.appendChild(properties);

		doc.appendChild(projectNode);

		// Get the AudioSources Node, and append
		projectNode.appendChild(asmanager->get_state( doc ));

		// Get all the Songs
		QDomNode songsNode = doc.createElement("Songs");

		foreach(Song* song, songList)
		songsNode.appendChild(song->get_state(doc));

		projectNode.appendChild(songsNode);

		QTextStream stream(&data);
		doc.save(stream, 4);
		data.close();

		info().information( tr("Project saved (%1)").arg(title) );

	} else {
		info().critical( tr("Could not open project properties file for writing! (%1)").arg(fileName) );
		return -1;
	}

	return 1;
}

void Project::set_title(const QString& pTitle)
{
	title = pTitle;
}


void Project::set_engineer(const QString& pEngineer)
{
	engineer=pEngineer;
}

void Project::set_description(const QString& des)
{
	m_description = des;
}

bool Project::has_changed()
{
	foreach(Song* song, songList) {
		if(song->is_changed())
			return true;
	}
	return false;
}


Song* Project::add_song()
{
	PENTER;
	Song* song = new Song(this, songList.size()+1);

	songList.insert(song->get_id(), song);
	set_current_song(song->get_id());
	currentSongId = song->get_id();
	emit songAdded();
	return song;
}


void Project::set_current_song(int id)
{
	PENTER;
	Song* song = songList.value(id);
	if (!song) {
		info().information( tr("Song doesn't exist! ( Song %1)").arg(id) );
		return;
	}

	currentSongId=id;
	emit currentSongChanged(song);

}


Song* Project::get_current_song() const
{
	return songList.value(currentSongId);
}


Song* Project::get_song(int id) const
{
	return songList.value(id);
}


int Project::remove_song(int key)
{
	Song* song = songList.take(key);
	if (song) {
		if (song->get_id() == key)
			set_current_song(songList.size());

		emit songRemoved();

		song->disconnect_from_audiodevice_and_delete();

	} else {
		return -1;
	}
	return 1;
}


int Project::export_project(ExportSpecification* spec)
{
	spec->progress = 0;
	spec->running = true;
	spec->stop = false;

	ExportThread* exportThread =  new ExportThread(this, spec);
	exportThread->start();

	return 0;
}

int Project::start_export(ExportSpecification* spec)
{
	PMESG("Starting export, rate is %d bitdepth is %d", spec->sample_rate, spec->data_width );

	spec->blocksize = audiodevice().get_buffer_size();

	spec->dataF = new audio_sample_t[spec->blocksize * spec->channels];

	overallExportProgress = renderedSongs = 0;
	songsToRender.clear();

	if (spec->allSongs) {
		foreach(Song* song, songList)
		songsToRender.append(song);
	} else {
		Song* song = get_current_song();
		if (!song)
			return -1;
		songsToRender.append(song);
	}


	foreach(Song* song, songsToRender) {
		PMESG("Starting export for song %d", song->get_id());
		emit exportStartedForSong(song);

		if (song->prepare_export(spec) < 0) {
			PERROR("Failed to prepare song for export");
			continue;
		}

		while(song->render(spec) > 0) {}

		renderedSongs++;
	}

	PMESG("Export Finished");

	spec->running = false;
	overallExportProgress = 0;
	delete spec->dataF;

	emit exportFinished();

	return 1;
}

Command* Project::select()
{
	set_current_song(ie().collected_number());
	return (Command*) 0;
}

int Project::get_rate( ) const
{
	return m_rate;
}

int Project::get_bitdepth( ) const
{
	return m_bitDepth;
}

QStringList Project::get_songs( ) const
{
	QStringList list;
	foreach(Song* song, songList) {
		list.append(song->get_title());
	}
	return list;
}

void Project::set_song_export_progress(int progress)
{
	overallExportProgress = (progress / songsToRender.count()) + (renderedSongs * (100 / songsToRender.count()) );

	emit songExportProgressChanged(progress);
	emit overallExportProgressChanged(overallExportProgress);
}

QHash< int, Song * > Project::get_song_list( ) const
{
	return songList;
}


qint64 Project::get_id() const
{
	return m_id;
}

int Project::get_current_song_id( ) const
{
	return currentSongId;
}

int Project::get_num_songs( ) const
{
	return songList.size();
}

QString Project::get_title( ) const
{
	return title;
}

QString Project::get_engineer( ) const
{
	return engineer;
}

QString Project::get_description() const
{
	return m_description;
}

QString Project::get_root_dir( ) const
{
	return rootDir;
}

QString Project::get_audiosources_dir() const
{
	return rootDir + "/audiosources/";
}

AudioSourceManager * Project::get_audiosource_manager( ) const
{
	return asmanager;
}

//eof

