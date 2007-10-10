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

#include "ProjectConverter.h"

#include <QTextStream>
#include <QFile>

#include "FileHelpers.h"
#include "AudioFileMerger.h"
#include "ReadSource.h"
#include "Utils.h"
#include "defines.h"


ProjectConverter::ProjectConverter() 
{
	m_projectfileversion = -1;
	connect(this, SIGNAL(conversionFinished()), this, SLOT(conversion_finished()));
}

int ProjectConverter::start()
{

	switch(m_projectfileversion) {
		case 2: {
			start_conversion_from_version_2_to_3();
			break;
		}
		default: {
			emit message(tr("Project file with version %1 cannot be converted, only files with version 2 can!").arg(m_projectfileversion));
			return -1;
		}
	}
}

void ProjectConverter::set_project(const QString & rootdir, const QString & name)
{
	m_rootdir = rootdir;
	m_projectname = name;

	QDomDocument doc("Project");
	
	QString filename(m_rootdir + "/project.tpf");
	QFile file(filename);
		
	if (!file.open(QIODevice::ReadOnly)) {
		printf("filename '%s' could not be opened!\n", QS_C(filename));
		return;
	}
		
	// Start setting and parsing the content of the xml file
	QString errorMsg;
	if (!doc.setContent(&file, &errorMsg)) {
		QString error = tr("Project %1: Failed to parse project.tpf file! (Reason: %2)").arg(m_projectname).arg(errorMsg);
		printf("%s\n", QS_C(error));
		return;
	}
	
	QDomElement docElem = doc.documentElement();
	QDomNode propertiesNode = docElem.firstChildElement("Properties");
	QDomElement projectelement = propertiesNode.toElement();
	
	m_projectfileversion = projectelement.attribute("projectfileversion", "-1").toInt();
	
	m_document = doc.cloneNode(true).toDocument();
}

int ProjectConverter::start_conversion_from_version_2_to_3()
{
	emit message(tr("Starting to convert Project from version 2 to version 3"));
	
	m_merger = new AudioFileMerger;
	m_filesToMerge = m_filesMerged = 0;
	connect(m_merger, SIGNAL(progress(int)), this, SIGNAL(progress(int)));
	connect(m_merger, SIGNAL(taskStarted(QString)), this, SLOT(file_merge_started(QString)));
	connect(m_merger, SIGNAL(taskFinished(QString)), this, SLOT(file_merge_finished(QString)));
	
	QDomElement docElem = m_document.documentElement();
	QDomNode propertiesNode = docElem.firstChildElement("Properties");
	QDomElement projectelement = propertiesNode.toElement();
	
	QString version = projectelement.attribute("projectfileversion", "-1");
	int projectrate = projectelement.attribute("rate", "44100").toInt();
	version = QString::number(3);
	projectelement.setAttribute("projectfileversion", version);

		
	// Load all the AudioSources for this project
	QDomNode asmNode = docElem.firstChildElement("ResourcesManager");
	QDomNode sourcesNode = asmNode.firstChildElement("AudioSources").firstChild();
	
	while(!sourcesNode.isNull()) {
		QDomElement readsourceelement = sourcesNode.toElement();
		int channelcount = readsourceelement.attribute("channelcount", "0").toInt();
		int filecount = readsourceelement.attribute("filecount", "0").toInt();
		qint64 id = readsourceelement.attribute("id", "").toLongLong();
		QString dir = readsourceelement.attribute("dir", "" );
		QString name = readsourceelement.attribute("name", "No name supplied?" );

		if (filecount == 2 && channelcount == 2 && dir == (m_rootdir + "/audiosources/")) {
			ReadSource* readsource0 = new ReadSource(dir, name + "-ch0.wav");
			ReadSource* readsource1 = new ReadSource(dir, name + "-ch1.wav");
			readsource0->ref();
			readsource0->init();
			readsource1->ref();
			readsource1->init();
			
			m_readsources.insertMulti(id, readsource0);
			m_readsources.insertMulti(id, readsource1);
			
			m_filesToMerge++;
			m_merger->enqueue_task(readsource0, readsource1, dir, name);
			
			readsourceelement.setAttribute("name", name + ".wav");
			readsourceelement.setAttribute("length", readsource0->get_length().universal_frame());
			readsourceelement.setAttribute("rate", readsource0->get_rate());
			
		}
			
			
		sourcesNode = sourcesNode.nextSibling();
	}
		
		
	QDomNode clipsNode = asmNode.firstChildElement("AudioClips").firstChild();
	
	while(!clipsNode.isNull()) {
		QDomElement clipelement = clipsNode.toElement();
		qint64 readsourceid = clipelement.attribute("source", "").toLongLong();
		nframes_t length = clipelement.attribute( "length", "0" ).toUInt();
		nframes_t sourceStartFrame = clipelement.attribute( "sourcestart", "" ).toUInt();
		nframes_t trackStart = clipelement.attribute( "trackstart", "" ).toUInt();
		ReadSource* source = m_readsources.value(readsourceid);
		int rate = projectrate;
		if (source) {
			rate = source->get_rate();
		}
			
		clipelement.setAttribute("trackstart", TimeRef(trackStart, rate).universal_frame());
		clipelement.setAttribute("sourcestart", TimeRef(sourceStartFrame, rate).universal_frame());
		clipelement.setAttribute("length", TimeRef(length, rate).universal_frame());
			
		clipsNode = clipsNode.nextSibling();
	}
		
	QDomNode songsNode = docElem.firstChildElement("Sheets");
	QDomNode songNode = songsNode.firstChild();

		// Load all the Songs
	while(!songNode.isNull())
	{
		QDomNode tracksNode = songNode.firstChildElement("Tracks");
		QDomNode trackNode = tracksNode.firstChild();

		while(!trackNode.isNull()) {
			QDomElement trackelement = trackNode.toElement();
			trackelement.setAttribute("OutBus", "Master Out");
			trackNode = trackNode.nextSibling();
		}
		songNode = songNode.nextSibling();
	}
	
	emit message(tr("Converting project.tpf file..... Done!"));
	
	if (!m_filesToMerge) {
		finish_2_3_conversion();
	} else {
		emit message(tr("<b>Need to convert %1 files</b>").arg(m_filesToMerge));
	}
	
	return 1;
}

int ProjectConverter::save_converted_document()
{
	QString filename(m_rootdir + "/project.tpf");
	
	QFile backup(filename);
	backup.copy(m_rootdir + "/projectv" + QString::number(m_projectfileversion) + "backup.tpf");
	
	QFile savefile( filename );

	if (!savefile.open( QIODevice::WriteOnly ) ) {
		QString errorstring = FileHelper::fileerror_to_string(savefile.error());
		printf("%s\n", QS_C(tr("Couldn't open Project properties file for writing! (File %1. Reason: %2)").arg(filename).arg(errorstring)));
		return -1;
	}
	
	QTextStream stream(&savefile);
	m_document.save(stream, 4);
	printf("%s\n", QS_C(tr("Project %1 converted").arg(m_projectname)));
	emit message(tr("Saving converted project.tpf file.... Done!"));
	
	return 1;
}

void ProjectConverter::conversion_finished()
{
	emit message(tr("Conversion finished succesfully"));
}

QString ProjectConverter::get_conversion_description()
{
	switch(m_projectfileversion) {
		case 2 : {
			QFile file(":/project_conversion_description_2_3");
			file.open(QIODevice::ReadOnly);
			return file.readAll();
		}
	}
	return tr("No conversion description available!");
}

void ProjectConverter::file_merge_started(QString file)
{
	emit fileMergeStarted(file + "   (" + QString::number(m_filesMerged + 1) + "/" + QString::number(m_filesToMerge) + ")");
}

void ProjectConverter::file_merge_finished(QString file)
{
	emit fileMergeFinished(file);
	
	m_filesMerged++;
	
	if ((m_filesToMerge - m_filesMerged) == 0) {
		finish_2_3_conversion();
	}
}

void ProjectConverter::finish_2_3_conversion()
{
	foreach(ReadSource* source, m_readsources) {
		delete source;
	}
	
	if ( ! m_merger->wait(1000) ) {
		qWarning("FileMerger:: Still running after 1 second wait, terminating!");
		m_merger->terminate();
	}
	
	delete m_merger;
	
	save_converted_document();
	
	emit conversionFinished();
}

