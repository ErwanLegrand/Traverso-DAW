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
#include "FileHelpers.h"

#define PROJECT_FILE_VERSION 	3

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Project::Project(const QString& title)
	: ContextItem(), m_title(title)
{
	PENTERCONS;
	m_currentSongId = 0;
	m_exportThread = 0;
	engineer = "";

	m_useResampling = config().get_property("Conversion", "DynamicResampling", true).toBool();
	m_rootDir = config().get_property("Project", "directory", "/directory/unknown/").toString() + "/" + m_title;
	m_sourcesDir = m_rootDir + "/audiosources";
	m_rate = audiodevice().get_sample_rate();
	m_bitDepth = audiodevice().get_bit_depth();

	m_resourcesManager = new ResourcesManager(this);
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

	delete m_hs;
}


int Project::create(int songcount, int numtracks)
{
	PENTER;
	PMESG("Creating new project %s  NumSongs=%d", QS_C(m_title), songcount);

	QDir dir;
	if (dir.mkdir(m_rootDir) < 0) {
		info().critical(tr("Cannot create dir %1").arg(m_rootDir));
		return -1;
	}
	
	if (create_peakfiles_dir() < 0) {
		return -1;
	}
	
	if (create_audiosources_dir() < 0) {
		return -1;
	}
	
	if (pm().create_projectfilebackup_dir(m_rootDir) < 0) {
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

	info().information(tr("Created new Project %1").arg(m_title));
	return 1;
}

int Project::create_audiosources_dir()
{
	QDir dir;
	if (dir.mkdir(m_sourcesDir) < 0) {
		info().critical(tr("Cannot create dir %1").arg(m_sourcesDir));
		return -1;
	}
	
	return 1;

}

int Project::create_peakfiles_dir()
{
	QDir dir;
	QString peaksDir = m_rootDir + "/peakfiles/";

	if (dir.mkdir(peaksDir) < 0) {
		info().critical(tr("Cannot create dir %1").arg(peaksDir));
		return -1;
	}
	
	return 1;
}

int Project::load(QString projectfile)
{
	PENTER;
	QDomDocument doc("Project");
	
	QFile file;
	QString filename;
	
	if (projectfile.isEmpty()) {
		filename = m_rootDir + "/project.tpf";
		file.setFileName(filename);
	} else {
		filename = projectfile;
		file.setFileName(filename);
	}

	if (!file.open(QIODevice::ReadOnly)) {
		m_errorString = tr("Project %1: Cannot open project.tpf file! (Reason: %2)").arg(m_title).arg(file.errorString());
		info().critical(m_errorString);
		return PROJECT_FILE_COULD_NOT_BE_OPENED;
	}
	
	// Check if important directories still exist!
	QDir dir;
	if (!dir.exists(m_rootDir + "/peakfiles")) {
		create_peakfiles_dir();
	}
	if (!dir.exists(m_rootDir + "/audiosources")) {
		create_audiosources_dir();
	}

	
	// Start setting and parsing the content of the xml file
	QString errorMsg;
	if (!doc.setContent(&file, &errorMsg)) {
		m_errorString = tr("Project %1: Failed to parse project.tpf file! (Reason: %2)").arg(m_title).arg(errorMsg);
		info().critical(m_errorString);
		return SETTING_XML_CONTENT_FAILED;
	}
	
	QDomElement docElem = doc.documentElement();
	QDomNode propertiesNode = docElem.firstChildElement("Properties");
	QDomElement e = propertiesNode.toElement();
	
	if (e.attribute("projectfileversion", "-1").toInt() != PROJECT_FILE_VERSION) {
		m_errorString = tr("Project File Version does not match, unable to load Project!");
		info().warning(m_errorString);
		return PROJECT_FILE_VERSION_MISMATCH;
	}

	m_title = e.attribute( "title", "" );
	engineer = e.attribute( "engineer", "" );
	m_description = e.attribute( "description", "No description set");
	m_discid = e.attribute( "discId", "" );
	m_upcEan = e.attribute( "upc_ean", "" );
	m_genre = e.attribute( "genre", "" ).toInt();
	m_performer = e.attribute( "performer", "" );
	m_arranger = e.attribute( "arranger", "" );
	m_songwriter = e.attribute( "songwriter", "" );
	m_message = e.attribute( "message", "" );
	m_rate = e.attribute( "rate", "" ).toInt();
	m_bitDepth = e.attribute( "bitdepth", "" ).toInt();
	m_id = e.attribute("id", "0").toLongLong();
	if (m_id == 0) {
		m_id = create_id();
	}
	m_importDir = e.attribute("importdir", QDir::homePath()); 
	
	
	// Load all the AudioSources for this project
	QDomNode asmNode = docElem.firstChildElement("ResourcesManager");
	m_resourcesManager->set_state(asmNode);


	QDomNode songsNode = docElem.firstChildElement("Sheets");
	QDomNode songNode = songsNode.firstChild();

	// Load all the Songs
	while(!songNode.isNull())
	{
		Song* song = new Song(this, songNode);
		Command::process_command(add_song(song, false));
		songNode = songNode.nextSibling();
	}

	qint64 id = e.attribute("currentsheetid", "0" ).toLongLong();
	
	if ( id == 0) {
		if (m_songs.size()) {
			id = m_songs.first()->get_id();
		}
	}
			
	set_current_song(id);

	info().information( tr("Project %1 loaded").arg(m_title) );
	
	emit projectLoadFinished();

	return 1;
}


int Project::save()
{
	PENTER;
	QDomDocument doc("Project");
	QString fileName = m_rootDir + "/project.tpf";
	
	QFile data( fileName );

	if (!data.open( QIODevice::WriteOnly ) ) {
		QString errorstring = FileHelper::fileerror_to_string(data.error());
		info().critical( tr("Couldn't open Project properties file for writing! (File %1. Reason: %2)").arg(fileName).arg(errorstring) );
		return -1;
	}
	
	get_state(doc);
	QTextStream stream(&data);
	doc.save(stream, 4);
	data.close();
	info().information( tr("Project %1 saved ").arg(m_title) );
	
	pm().start_incremental_backup(m_title);

	return 1;
}


QDomNode Project::get_state(QDomDocument doc, bool istemplate)
{
	PENTER;
	
	QDomElement projectNode = doc.createElement("Project");
	QDomElement properties = doc.createElement("Properties");

	properties.setAttribute("title", m_title);
	properties.setAttribute("engineer", engineer);
	properties.setAttribute("description", m_description);
	properties.setAttribute("discId", m_discid );
	properties.setAttribute("upc_ean", m_upcEan);
	properties.setAttribute("genre", QString::number(m_genre));
	properties.setAttribute("performer", m_performer);
	properties.setAttribute("arranger", m_arranger);
	properties.setAttribute("songwriter", m_songwriter);
	properties.setAttribute("message", m_message);
	properties.setAttribute("currentsheetid", m_currentSongId);
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
		projectNode.appendChild(m_resourcesManager->get_state(doc));
	}

	// Get all the Songs
	QDomNode songsNode = doc.createElement("Sheets");

	foreach(Song* song, m_songs) {
		songsNode.appendChild(song->get_state(doc, istemplate));
	}

	projectNode.appendChild(songsNode);
	
	return projectNode;
}


void Project::set_title(const QString& title)
{
	if (title == m_title) {
		// do nothing if the title is the same as the current one
		return;
	}
	
	if (pm().project_exists(title)) {
		info().critical(tr("Project with title '%1' allready exists, not setting new title!").arg(title));
		return;
	}
	
	QString newrootdir = config().get_property("Project", "directory", "/directory/unknown/").toString() + "/" + title;
	
	QDir dir(m_rootDir);
	
	if ( ! dir.exists() ) {
		info().critical(tr("Project directory %1 no longer exists, did you rename it? " 
				"Shame on you! Please undo that, and come back later to rename your Project...").arg(m_rootDir));
		return;
	}
	
	m_title = title;
	
	save();
	
	if (pm().rename_project_dir(m_rootDir, newrootdir) < 0 ) {
		return;
	}
	
	QMessageBox::information( 0, 
			tr("Traverso - Information"), 
			tr("Project title changed, Project will to be reloaded to ensure proper operation"),
			QMessageBox::Ok);
	
	pm().load_renamed_project(m_title);
}


void Project::set_engineer(const QString& pEngineer)
{
	engineer=pEngineer;
}

void Project::set_description(const QString& des)
{
	m_description = des;
}

void Project::set_discid(const QString& pId)
{
	m_discid = pId;
}

void Project::set_performer(const QString& pPerformer)
{
	m_performer = pPerformer;
}

void Project::set_arranger(const QString& pArranger)
{
	m_arranger = pArranger;
}

void Project::set_songwriter(const QString& pSongwriter)
{
	m_songwriter = pSongwriter;
}

void Project::set_message(const QString& pMessage)
{
	m_message = pMessage;
}

void Project::set_upc_ean(const QString& pUpc)
{
	m_upcEan = pUpc;
}

void Project::set_genre(int pGenre)
{
	m_genre = pGenre;
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
       		tr("Sheet %1 added").arg(song->get_title()));
	
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
		info().information( tr("Sheet '%1' doesn't exist!").arg(id) );
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
		tr("Remove Sheet %1").arg(song->get_title()));
	
	cmd->set_instantanious(true);
	
	return cmd;
}


int Project::export_project(ExportSpecification* spec)
{
	if (!m_exportThread) {
		m_exportThread = new ExportThread(this, spec);
	}
	
	if (m_exportThread->isRunning()) {
		info().warning(tr("Export already in progress, cannot start it twice!"));
		return -1;
	}
	
	spec->progress = 0;
	spec->running = true;
	spec->stop = false;
	spec->breakout = false;

	m_exportThread->start();

	return 0;
}

int Project::start_export(ExportSpecification* spec)
{
	PMESG("Starting export, rate is %d bitdepth is %d", spec->sample_rate, spec->data_width );

	spec->blocksize = 32768;

	spec->dataF = new audio_sample_t[spec->blocksize * spec->channels];
	audio_sample_t* readbuffer = new audio_sample_t[spec->blocksize * spec->channels];

	overallExportProgress = renderedSongs = 0;
	songsToRender.clear();

	if (spec->allSongs) {
		foreach(Song* song, m_songs) {
			songsToRender.append(song);
		}
	} else {
		Song* song = get_current_song();
		if (song) {
			songsToRender.append(song);
		}
	}

	foreach(Song* song, songsToRender) {
		PMESG("Starting export for song %lld", song->get_id());
		emit exportStartedForSong(song);
		spec->resumeTransport = false;
		spec->resumeTransportLocation = song->get_transport_location();
		song->readbuffer = readbuffer;
		
		if (spec->normalize) {
			spec->peakvalue = 0.0;
			spec->renderpass = ExportSpecification::CALC_NORM_FACTOR;
			
			
			if (song->prepare_export(spec) < 0) {
				PERROR("Failed to prepare song for export");
				continue;
			}
			
			while(song->render(spec) > 0) {}
			
			spec->normvalue = (1.0 - FLT_EPSILON) / spec->peakvalue;
			
			if (spec->peakvalue > 1.0) {
				info().critical(tr("Detected clipping in exported audio! (%1)")
						.arg(coefficient_to_dbstring(spec->peakvalue)));
			}
			
			if (!spec->breakout) {
				info().information(tr("calculated norm factor: %1").arg(coefficient_to_dbstring(spec->normvalue)));
			}
		}

		spec->renderpass = ExportSpecification::WRITE_TO_HARDDISK;
		
		if (song->prepare_export(spec) < 0) {
			PERROR("Failed to prepare sheet for export");
			break;
		}
		
		while(song->render(spec) > 0) {}
		
		if (!QMetaObject::invokeMethod(song, "set_transport_pos",  Qt::QueuedConnection, Q_ARG(TimeRef, spec->resumeTransportLocation))) {
			printf("Invoking Song::set_transport_pos() failed\n");
		}
		if (spec->resumeTransport) {
			if (!QMetaObject::invokeMethod(song, "start_transport",  Qt::QueuedConnection)) {
				printf("Invoking Song::start_transport() failed\n");
			}
		}
		if (spec->breakout) {
			break;
		}
		renderedSongs++;
	}

	PMESG("Export Finished");

	spec->running = false;
	overallExportProgress = 0;
	
	delete [] spec->dataF;
	delete [] readbuffer;
	spec->dataF = 0;

	emit exportFinished();

	return 1;
}

int Project::create_cdrdao_toc(ExportSpecification* spec)
{
	QList<Song* > songs;
	QString filename = spec->exportdir;
	
	if (spec->allSongs) {
		foreach(Song* song, m_songs) {
			songs.append(song);
		}
		// filename of the toc file is "project-name.toc"
		filename += get_title() + ".toc";
	} else {
		Song* song = get_current_song();
		if (!song) {
			return -1;
		}
		songs.append(song);
	}
	
	QString output;

	output += "CD_DA\n\n";
	output += "CD_TEXT {\n";

	output += "  LANGUAGE_MAP {\n    0 : EN\n  }\n\n";

	output += "  LANGUAGE 0 {\n";
	output += "    TITLE \"" + get_title() +  "\"\n";
	output += "    PERFORMER \"" + get_performer() + "\"\n";
	output += "    DISC_ID \"" + get_discid() + "\"\n";
	output += "    UPC_EAN \"" + get_upc_ean() + "\"\n\n";

	output += "    ARRANGER \"" + get_arranger() + "\"\n";
	output += "    SONGWRITER \"" + get_songwriter() + "\"\n";
	output += "    MESSAGE \"" + get_message() + "\"\n";
	output += "    GENRE \"" + QString::number(get_genre()) + "\"\n  }\n}\n\n";

	
	bool pregap = true;
	spec->renderpass = ExportSpecification::CREATE_CDRDAO_TOC;
	
	foreach(Song* song, songs) {
		if (song->prepare_export(spec) < 0) {
			return -1;
		}
		output += song->get_cdrdao_tracklist(spec, pregap);
		pregap = false; // only add the pregap at the first song
	}
	

	if (spec->writeToc) {
		if (!spec->allSongs) {
			// filename of the toc file is "song-name.toc"
			filename += spec->basename + ".toc";
		}
		
		spec->tocFileName = filename;

		QFile file(filename);

		if (file.open(QFile::WriteOnly)) {
			printf("Saving cdrdao toc-file to %s\n", QS_C(spec->tocFileName));
			QTextStream out(&file);
			out << output;
			file.close();
		}
	}
	
	spec->cdrdaoToc = output;
	
	return 1;
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
	// FIXME: Projects should eventually just use the universal samplerate
	if (m_useResampling) {
		return audiodevice().get_sample_rate();
	}
	
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
	return m_title;
}

QString Project::get_engineer( ) const
{
	return engineer;
}

QString Project::get_description() const
{
	return m_description;
}

QString Project::get_discid() const
{
	return m_discid;
}

QString Project::get_performer() const
{
	return m_performer;
}

QString Project::get_arranger() const
{
	return m_arranger;
}

QString Project::get_songwriter() const
{
	return m_songwriter;
}

QString Project::get_message() const
{
	return m_message;
}

QString Project::get_upc_ean() const
{
	return m_upcEan;
}

int Project::get_genre()
{
	return m_genre;
}

QString Project::get_root_dir( ) const
{
	return m_rootDir;
}

QString Project::get_audiosources_dir() const
{
	return m_rootDir + "/audiosources/";
}

ResourcesManager * Project::get_audiosource_manager( ) const
{
	return m_resourcesManager;
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
		if (song->is_recording() && song->is_transport_rolling()) {
			return true;
		}
	}
	return false;
}

