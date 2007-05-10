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

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QString>

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <cfloat>

#include "Project.h"
#include "Song.h"
#include "ProjectManager.h"
#include "Information.h"
#include "InputEngine.h"
#include "ResourcesManager.h"
#include "Export.h"
#include "AudioDevice.h"
#include "Config.h"
#include "ContextPointer.h"
#include "Utils.h"
#include <AddRemove.h>

#define PROJECT_FILE_VERSION 	1

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Project::Project(const QString& pTitle)
	: ContextItem(), title(pTitle)
{
	PENTERCONS;
	m_currentSongId = 0;
	engineer = "";

	rootDir = config().get_property("Project", "directory", "/directory/unknown/").toString() + "/" + title;
	sourcesDir = rootDir + "/audiosources";
	m_rate = audiodevice().get_sample_rate();
	m_bitDepth = audiodevice().get_bit_depth();

	m_asmanager = new ResourcesManager();
	m_hs = new QUndoStack(pm().get_undogroup());

	cpointer().add_contextitem(this);
}


Project::~Project()
{
	PENTERDES;
	cpointer().remove_contextitem(this);

	foreach(Song* song, m_songs) {
		song->schedule_for_deletion();
		song->disconnect_from_audiodevice();
	}

	delete m_asmanager;
	delete m_hs;
}


int Project::create(int songcount, int numtracks)
{
	PENTER;
	PMESG("Creating new project %s  NumSongs=%d", title.toAscii().data(), songcount);

	QDir dir;
	if (dir.mkdir(rootDir) < 0) {
		info().critical(tr("Cannot create dir %1").arg(rootDir));
		return -1;
	}

	if (dir.mkdir(sourcesDir) < 0) {
		info().critical(tr("Cannot create dir %1").arg(sourcesDir));
		return -1;
	}

	QString peaksDir = rootDir + "/peakfiles/";

	if (dir.mkdir(peaksDir) < 0) {
		info().critical(tr("Cannot create dir %1").arg(peaksDir));
		return -1;
	}

	for (int i=0; i< songcount; i++) {
		Song* song = new Song(this, numtracks);
		private_add_song(song);
	}

	if (m_songs.size()) {
		set_current_song(m_songs.first()->get_id());
	}
	
	m_id = create_id();
	m_importDir = QDir::homePath();

	info().information(tr("Created new Project %1").arg(title));
	return 1;
}


int Project::load(QString projectfile)
{
	PENTER;
	QDomDocument doc("Project");
	
	QFile file;
	QString filename;
	if (projectfile.isEmpty()) {
		filename = rootDir + "/project.tpf";
		file.setFileName(filename);
	} else {
		filename = projectfile;
		file.setFileName(filename);
	}

	if (!file.open(QIODevice::ReadOnly))
	{
		file.close();
		info().critical(tr("Project %1: Cannot open project.tpf file! (%2)")
			       .arg(title).arg(file.errorString()));
		
		QFile backup(filename + "~");
		backup.copy(filename);
		if (QFile::exists(filename.append("~"))) {
			return load(filename);
		} else {
			info().information(tr("No backup project file available, unable to restore project"));
			return -1;
		}
	}

	QString errorMsg;
	if (!doc.setContent(&file, &errorMsg))
	{
		info().critical(tr("Project %1: Failed to parse project.tpf file! (%2)")
				.arg(title).arg(errorMsg));
		
		file.remove();
		file.close();
		if (! filename.contains("~")) {
			// Trying to load backup!
			QFile backup(filename + "~");
			backup.copy(filename);
			if (QFile::exists(filename.append("~"))) {
				return load(filename);
			} else {
				info().information(tr("No backup project file available, unable to restore project"));
				return -1;
			}
		} else {
			return -1;
		}
	}
	
	if (filename.contains("~")) {
		info().information(tr("Project file restored from older version (%1)")
				.arg(QFileInfo(filename).lastModified().toString()));
	}

	file.close();

	QDomElement docElem = doc.documentElement();
	QDomNode propertiesNode = docElem.firstChildElement("Properties");
	QDomElement e = propertiesNode.toElement();
	
	if (e.attribute("projectfileversion", "-1").toInt() != PROJECT_FILE_VERSION) {
		info().warning(tr("Project File Version does not match, unable to load Project!"));
		return -1;
	}

	title = e.attribute( "title", "" );
	engineer = e.attribute( "engineer", "" );
	m_description = e.attribute( "description", "No description set");
	m_rate = e.attribute( "rate", "" ).toInt();
	m_bitDepth = e.attribute( "bitdepth", "" ).toInt();
	m_id = e.attribute("id", "0").toLongLong();
	if (m_id == 0) {
		m_id = create_id();
	}
	m_importDir = e.attribute("importdir", QDir::homePath()); 
	
	
	// Load all the AudioSources for this project
	
	QDomNode asmNode = docElem.firstChildElement("ResourcesManager");
	m_asmanager->set_state(asmNode);


	QDomNode songsNode = docElem.firstChildElement("Songs");
	QDomNode songNode = songsNode.firstChild();

	// Load all the Songs
	while(!songNode.isNull())
	{
		Song* song = new Song(this, songNode);
		Command::process_command(add_song(song, false));
		songNode = songNode.nextSibling();
	}

	qint64 id = e.attribute("currentSongId", "0" ).toLongLong();
	
	if ( id == 0) {
		if (m_songs.size()) {
			id = m_songs.first()->get_id();
		}
	}
			
	set_current_song(id);

	info().information( tr("Project %1 loaded").arg(title) );

	return 1;
}


int Project::save()
{
	PENTER;
	QDomDocument doc("Project");
	QString fileName = rootDir + "/project.tpf";
	
	QFile::remove(fileName + "~");
	QFile backup(fileName);
	backup.rename(fileName + "~");
	QFile::remove(fileName);
	
	QFile data( fileName );

	if (data.open( QIODevice::WriteOnly ) ) {
		get_state(doc);
		QTextStream stream(&data);
		doc.save(stream, 4);
		data.close();
		info().information( tr("Project %1 saved ").arg(title) );
	} else {
		info().critical( tr("Couldn't open Project properties file for writing! (%1)").arg(fileName) );
		return -1;
	}

	return 1;
}


QDomNode Project::get_state(QDomDocument doc, bool istemplate)
{
	PENTER;
	
	QDomElement projectNode = doc.createElement("Project");
	QDomElement properties = doc.createElement("Properties");

	properties.setAttribute("title", title);
	properties.setAttribute("engineer", engineer);
	properties.setAttribute("description", m_description);
	properties.setAttribute("currentSongId", m_currentSongId);
	properties.setAttribute("rate", m_rate);
	properties.setAttribute("bitdepth", m_bitDepth);
	properties.setAttribute("projectfileversion", PROJECT_FILE_VERSION);
	if (! istemplate) {
		properties.setAttribute("id", m_id);
	} else {
		properties.setAttribute("title", "Template Project File!!");
	}
	
	properties.setAttribute("importdir", m_importDir);
		
	projectNode.appendChild(properties);

	doc.appendChild(projectNode);

	// Get the AudioSources Node, and append
	if (! istemplate) {
		projectNode.appendChild(m_asmanager->get_state(doc));
	}

	// Get all the Songs
	QDomNode songsNode = doc.createElement("Songs");

	foreach(Song* song, m_songs) {
		songsNode.appendChild(song->get_state(doc, istemplate));
	}

	projectNode.appendChild(songsNode);
	
	return projectNode;
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
	foreach(Song* song, m_songs) {
		if(song->is_changed())
			return true;
	}
	return false;
}


Command* Project::add_song(Song* song, bool historable)
{
	PENTER;
	
	AddRemove* cmd;
	cmd = new AddRemove(this, song, historable, 0,
		"private_add_song(Song*)", "songAdded(Song*)",
       		"private_remove_song(Song*)", "songRemoved(Song*)",
       		tr("Song %1 added").arg(song->get_title()));
	
	cmd->set_instantanious(true);
	
	return cmd;
}


void Project::set_current_song(qint64 id)
{
	PENTER;
	
	if ( m_currentSongId == id) {
		return;
	}
	
	Song* newcurrent = 0;
	
	foreach(Song* song, m_songs) {
		if (song->get_id() == id) {
			newcurrent = song;
			break;
		}
	}
	
	if (!newcurrent) {
		info().information( tr("Song '%1' doesn't exist!").arg(id) );
		emit currentSongChanged(0);
		return;
	}

	m_currentSongId=id;
	
	emit currentSongChanged(newcurrent);
}


Song* Project::get_current_song() const
{
	Song* current = 0;
	
	foreach(Song* song, m_songs) {
		if (song->get_id() == m_currentSongId) {
			current = song;
			break;
		}
	}
	
	return current;
}


Song* Project::get_song(qint64 id) const
{
	Song* current = 0;
	
	foreach(Song* song, m_songs) {
		if (song->get_id() == id) {
			current = song;
			break;
		}
	}
	
	return current;
}


Command* Project::remove_song(Song* song, bool historable)
{
	AddRemove* cmd;
	cmd = new AddRemove(this, song, historable, 0,
		"private_remove_song(Song*)", "songRemoved(Song*)",
		"private_add_song(Song*)", "songAdded(Song*)",
		tr("Remove Song %1").arg(song->get_title()));
	
	cmd->set_instantanious(true);
	
	return cmd;
}


int Project::export_project(ExportSpecification* spec)
{
	spec->progress = 0;
	spec->running = true;
	spec->stop = false;
	spec->breakout = false;

	ExportThread* exportThread =  new ExportThread(this, spec);
	exportThread->start();

	return 0;
}

int Project::start_export(ExportSpecification* spec)
{
	PMESG("Starting export, rate is %d bitdepth is %d", spec->sample_rate, spec->data_width );

	spec->blocksize = 32768;

	spec->dataF = new audio_sample_t[spec->blocksize * spec->channels];

	overallExportProgress = renderedSongs = 0;
	songsToRender.clear();

	if (spec->allSongs) {
		foreach(Song* song, m_songs) {
			songsToRender.append(song);
		}
	} else {
		Song* song = get_current_song();
		if (!song) {
			return -1;
		}
		songsToRender.append(song);
	}

	QString cdrdaoImg = get_cdrdao_header(spec);

	foreach(Song* song, songsToRender) {
		PMESG("Starting export for song %lld", song->get_id());
		emit exportStartedForSong(song);
		spec->resumeTransport = false;
		spec->resumeTransportFrame = song->get_transport_frame();
		
		if (spec->normalize) {
			spec->peakvalue = 0.0;
			spec->renderpass = ExportSpecification::CALC_NORM_FACTOR;
			
			
			if (song->prepare_export(spec) < 0) {
				PERROR("Failed to prepare song for export");
				continue;
			}
			
			while(song->render(spec) > 0) {}
			
			spec->normvalue = 1.0 / spec->peakvalue;
			
			if (spec->peakvalue > (1.0 + FLT_EPSILON)) {
				info().critical(tr("Detected clipping in exported audio! (%1)")
						.arg(coefficient_to_dbstring(spec->peakvalue)));
			}
			
			if (!spec->breakout) {
				info().information(tr("calculated norm factor: %1").arg(coefficient_to_dbstring(spec->normvalue)));
			}
		}

		spec->renderpass = ExportSpecification::WRITE_TO_HARDDISK;
		
		if (song->prepare_export(spec) < 0) {
			PERROR("Failed to prepare song for export");
			continue;
		}
		
		cdrdaoImg += song->get_cdrdao_tracklist(spec);

		while(song->render(spec) > 0) {}
		
		song->set_transport_pos(spec->resumeTransportFrame);
		if (spec->resumeTransport) {
			Command* k;
			if (!QMetaObject::invokeMethod(song, "go",  Qt::QueuedConnection)) {
				printf("Invoking Song::go() failed\n");
			}
		}
		if (spec->breakout) {
			break;
		}
		renderedSongs++;
	}

	if (spec->writeToc) {
		PMESG("Writing cdrdao toc-file");

		QString name = spec->exportdir + "/";

		if (spec->allSongs) {
			// filename of the toc file is "project-name.toc"
			name += get_title() + ".toc";
		} else {
			// filename of the toc file is "song-name.toc"
			name += spec->basename + ".toc";
		}
		
		spec->tocFileName = name;

		QFile file(name);

		if (file.open(QFile::WriteOnly)) {
			QTextStream out(&file);
			out << cdrdaoImg;
		}
	}

	PMESG("Export Finished");

	spec->running = false;
	overallExportProgress = 0;
	delete spec->dataF;

	emit exportFinished();

	return 1;
}

QString Project::get_cdrdao_header(ExportSpecification* spec)
{
	QString output;

	output += "CD_DA\n\n";
	output += "CD_TEXT {\n";

	output += "  LANGUAGE_MAP {\n    0 : EN\n  }\n\n";

	output += "  LANGUAGE 0 {\n";
	output += "    TITLE \"" + get_title() +  "\"\n";
	output += "    PERFORMER \"\"\n";
	output += "    DISC_ID \"\"\n";
	output += "    UPC_EAN \"\"\n\n";

	output += "    ARRANGER \"\"\n";
	output += "    SONGWRITER \"\"\n";
	output += "    MESSAGE \"\"\n";
	output += "    GENRE \"\"\n  }\n}\n\n";

	return output;
}

Command* Project::select()
{
	int index = ie().collected_number();
	if (index <= m_songs.size() && index > 0) {
		set_current_song(m_songs.at(index - 1)->get_id());
	}
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

void Project::set_song_export_progress(int progress)
{
	overallExportProgress = (progress / songsToRender.count()) + 
			(renderedSongs * (100 / songsToRender.count()) );

	emit songExportProgressChanged(progress);
	emit overallExportProgressChanged(overallExportProgress);
}

QList<Song* > Project::get_songs( ) const
{
	return m_songs;
}

int Project::get_song_index(qint64 id) const
{
	for (int i=0; i<m_songs.size(); ++i) {
		if (m_songs.at(i)->get_id() == id) {
			return i + 1;
		}
	}
	
	return 0;
}


int Project::get_current_song_id( ) const
{
	return m_currentSongId;
}

int Project::get_num_songs( ) const
{
	return m_songs.size();
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

ResourcesManager * Project::get_audiosource_manager( ) const
{
	return m_asmanager;
}


void Project::private_add_song(Song * song)
{
	m_songs.append(song);
	song->connect_to_audiodevice();
}

void Project::private_remove_song(Song * song)
{
	m_songs.removeAll(song);
		
	qint64 newcurrent = 0;
		
	if (m_songs.size() > 0) {
		newcurrent = m_songs.last()->get_id();
	}
		
	set_current_song(newcurrent);

	song->disconnect_from_audiodevice();
}

QString Project::get_import_dir() const
{
	return m_importDir;
}

void Project::set_import_dir(const QString& dir)
{
	m_importDir = dir;
}

bool Project::is_save_to_close() const
{
	if (is_recording()) {
		QMessageBox::information( 0, 
				tr("Traverso - Information"), 
				tr("You're still recording, please stop recording first to be able to exit the application!"),
				QMessageBox::Ok);
		return false;
	}
	return true;
}

bool Project::is_recording() const
{
	foreach(Song* song, m_songs) {
		if (song->is_recording() && song->is_transporting()) {
			return true;
		}
	}
	return false;
}

//eof

