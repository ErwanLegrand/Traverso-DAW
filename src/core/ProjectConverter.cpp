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
	m_merger = 0;
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
	
	return 1;
}

void ProjectConverter::set_project(const QString & rootdir, const QString & name)
{
	m_readsources.clear();
	m_filesToMerge = 0;
	m_filesMerged = 0;
	if (m_merger) {
		delete m_merger;
	}
	m_document.clear();
	
	
	m_projectfileversion = -1;
	
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
	connect(m_merger, SIGNAL(processingStopped()), this, SLOT(processing_stopped()));
	
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
		bool wasrecording = readsourceelement.attribute("wasrecording", 0).toInt();

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
			
		} else {
			// The version 2 of the project file didn't have the full name
			// including the channel number, it was added by ReadSource so 
			// we have to do it now since newer version of ReadSource no longer add
			// the channel number + extension to the readsource name.
			if (!name.contains(".wav") && wasrecording && channelcount == 1) {
				readsourceelement.setAttribute("name", name + "-ch0.wav");
			}
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
		
		QDomElement fadeInNode = clipsNode.firstChildElement("FadeIn");
		if (!fadeInNode.isNull()) {
			QDomElement e = fadeInNode.toElement();
			QString rangestring = e.attribute("range", "1");
			double range;
			if (rangestring == "nan" || rangestring == "inf") {
				printf("FadeCurve::set_state: stored range was not a number!\n");
				range = 1;
			} else {
				range = rangestring.toDouble();
			}
			if (range > 1.0) {
				e.setAttribute("range", TimeRef(nframes_t(range), rate).universal_frame());
			}
		}
		QDomElement fadeOutNode = clipsNode.firstChildElement("FadeOut");
		if (!fadeOutNode.isNull()) {
			QDomElement e = fadeOutNode.toElement();
			QString rangestring = e.attribute("range", "1");
			double range;
			if (rangestring == "nan" || rangestring == "inf") {
				printf("FadeCurve::set_state: stored range was not a number!\n");
				range = 1;
			} else {
				range = rangestring.toDouble();
			}
			if (range > 1.0)
				e.setAttribute("range", TimeRef(nframes_t(range), rate).universal_frame());
		}
			
		QDomNode pluginChainNode = clipsNode.firstChildElement("PluginChain");
		if (!pluginChainNode.isNull()) {
			QDomNode pluginsNode = pluginChainNode.firstChild();
			QDomNode pluginNode = pluginsNode.firstChild();

			while(!pluginNode.isNull()) {
				if (pluginNode.toElement().attribute( "type", "") == "GainEnvelope") {
					QDomElement controlPortsNode = pluginNode.firstChildElement("ControlPorts");
					if (!controlPortsNode.isNull()) {
						QDomNode portNode = controlPortsNode.firstChild();
		
						if(!portNode.isNull()) {
							QDomElement curveNode = portNode.firstChildElement("PortAutomation");
							if (!curveNode.isNull()) {
								QDomElement e = curveNode.toElement();
								QStringList nodesList = e.attribute( "nodes", "" ).split(";");
								QStringList newNodesList;
								for (int i=0; i<nodesList.size(); ++i) {
									QStringList whenValueList = nodesList.at(i).split(",");
									double when = whenValueList.at(0).toDouble();
									double value = whenValueList.at(1).toDouble();
									newNodesList << QString::number(TimeRef(nframes_t(when), rate).universal_frame(), 'g', 24).append(",").append(QString::number(value));
									e.setAttribute("nodes",  newNodesList.join(";"));
								}
							}
						}
					}
				}
				pluginNode = pluginNode.nextSibling();
			}
		}
		
		clipsNode = clipsNode.nextSibling();
	}
		
	QDomNode sheetsNode = docElem.firstChildElement("Sheets");
	QDomNode sheetNode = sheetsNode.firstChild();

		// Load all the Sheets
	while(!sheetNode.isNull())
	{
		QDomNode tracksNode = sheetNode.firstChildElement("Tracks");
		QDomNode trackNode = tracksNode.firstChild();

		while(!trackNode.isNull()) {
			QDomElement trackelement = trackNode.toElement();
			trackelement.setAttribute("OutBus", "Master Out");
			trackNode = trackNode.nextSibling();
		}
		sheetNode = sheetNode.nextSibling();
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
	
	// Exit the FileMerger event loop
	m_merger->exit(0);	
	
	if ( ! m_merger->wait(1000) ) {
		qWarning("FileMerger:: Still running after 1 second wait, terminating!");
		m_merger->terminate();
	}
	
	delete m_merger;
	
	save_converted_document();
	
	emit conversionFinished();
}

void ProjectConverter::stop_conversion()
{
	if (m_merger && m_merger->isRunning()) {
		m_merger->stop_merging();
	}
}

void ProjectConverter::processing_stopped()
{
	emit message(tr("Conversion stopped on user request, you can continue to use this Project with Traverso <= 0.41.0, or reopen it with this version of Traverso and start the conversion again"));
}

