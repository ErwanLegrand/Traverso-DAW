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
#include "Sheet.h"
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

/**	\class Project
	\brief Project restores and saves the state of a Traverso Project
	
	A Project can have as much Sheet's as one likes. A Project with one Sheet acts like a 'Session'
	where the Sheet can be turned into a CD with various Tracks using Marker 's
	When a Project has multiple Sheet's, each Sheet will be a CD Track, this can be usefull if each
	Track on a CD is independend of the other CD Tracks.
 
 */


Project::Project(const QString& title)
	: ContextItem(), m_title(title)
{
	PENTERCONS;
	m_currentSheetId = 0;
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

	foreach(Sheet* sheet, m_sheets) {
		sheet->schedule_for_deletion();
		sheet->disconnect_from_audiodevice();
	}

	delete m_hs;
}


int Project::create(int sheetcount, int numtracks)
{
	PENTER;
	PMESG("Creating new project %s  NumSheets=%d", QS_C(m_title), sheetcount);

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
	
	for (int i=0; i< sheetcount; i++) {
		Sheet* sheet = new Sheet(this, numtracks);
		m_sheets.append(sheet);
		sheet->connect_to_audiodevice();
	}

	if (m_sheets.size()) {
		set_current_sheet(m_sheets.first()->get_id());
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




        prepare_audio_device(doc);

	
	// Load all the AudioSources for this project
	QDomNode asmNode = docElem.firstChildElement("ResourcesManager");
	m_resourcesManager->set_state(asmNode);


	QDomNode sheetsNode = docElem.firstChildElement("Sheets");
	QDomNode sheetNode = sheetsNode.firstChild();

	// Load all the Sheets
	while(!sheetNode.isNull())
	{
		Sheet* sheet = new Sheet(this, sheetNode);
		Command::process_command(add_sheet(sheet, false));
		sheetNode = sheetNode.nextSibling();
	}

	qint64 id = e.attribute("currentsheetid", "0" ).toLongLong();
	
	if ( id == 0) {
		if (m_sheets.size()) {
			id = m_sheets.first()->get_id();
		}
	}
			
	set_current_sheet(id);

	info().information( tr("Project %1 loaded").arg(m_title) );
	
	emit projectLoadFinished();

	return 1;
}


int Project::save(bool autosave)
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
	
	if (!autosave) {
		info().information( tr("Project %1 saved ").arg(m_title) );
	}
	
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
	properties.setAttribute("currentsheetid", m_currentSheetId);
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


        QDomElement audioIO = doc.createElement("AudioIO");
        QDomElement systemConfig = doc.createElement("SystemConfig");

        systemConfig.setAttribute("device", audiodevice().get_device_longname());
        systemConfig.setAttribute("driver", audiodevice().get_driver_type());
        audioIO.appendChild(systemConfig);

        QDomElement channelsElement = doc.createElement("Channels");
        QList<ChannelConfig> channelConfigs = audiodevice().get_channel_configuration();
        foreach(ChannelConfig conf, channelConfigs) {
                QDomElement chanElement = doc.createElement("Channel");
                chanElement.setAttribute("name", conf.name);
                chanElement.setAttribute("type", conf.type);
                channelsElement.appendChild(chanElement);
        }
        systemConfig.appendChild(channelsElement);

        QDomElement busesElement = doc.createElement("Buses");
        QList<BusConfig> busConfig = audiodevice().get_bus_configuration();
        foreach(BusConfig conf, busConfig) {
                QDomElement bus = doc.createElement("Bus");
                bus.setAttribute("name", conf.name);
                bus.setAttribute("channels", conf.channelNames.join(";"));
                bus.setAttribute("type", conf.type);
                bus.setAttribute("channelcount", conf.channelcount);
                busesElement.appendChild(bus);
        }
        systemConfig.appendChild(busesElement);

        projectNode.appendChild(audioIO);


	doc.appendChild(projectNode);

	// Get the AudioSources Node, and append
	if (! istemplate) {
		projectNode.appendChild(m_resourcesManager->get_state(doc));
	}

	// Get all the Sheets
	QDomNode sheetsNode = doc.createElement("Sheets");

	foreach(Sheet* sheet, m_sheets) {
		sheetsNode.appendChild(sheet->get_state(doc, istemplate));
	}

	projectNode.appendChild(sheetsNode);
	
	return projectNode;
}

void Project::prepare_audio_device(QDomDocument doc)
{
        AudioDeviceSetup ads;

        QDomNode audioIO = doc.documentElement().firstChildElement("AudioIO");
        QDomNode systemConfigNode = audioIO.firstChildElement("SystemConfig");
        QDomElement e = systemConfigNode.toElement();
        ads.driverType = e.attribute("driver", "Null Driver");

        QDomNode channelsConfigNode = systemConfigNode.firstChildElement("Channels");
        QDomNode channelNode = channelsConfigNode.firstChild();

        QList<ChannelConfig> channelConfigs;

        while (!channelNode.isNull()) {
                ChannelConfig conf;
                QDomElement e = channelNode.toElement();
                conf.name = e.attribute("name", "");
                conf.type = e.attribute("type", "");
                channelConfigs.append(conf);
                channelNode = channelNode.nextSibling();
        }


        QDomNode busesConfigNode = systemConfigNode.firstChildElement("Buses");
        QDomNode busNode = busesConfigNode.firstChild();

        QList<BusConfig> busConfigs;

        while (!busNode.isNull()) {
                BusConfig conf;
                QDomElement e = busNode.toElement();
                conf.name = e.attribute("name", "");
                conf.channelNames = e.attribute("channels", "").split(";");
                conf.type = e.attribute("type", "");
                conf.channelcount = e.attribute("channelcount", "2").toInt();
                busConfigs.append(conf);
                busNode = busNode.nextSibling();
        }


        ads.rate = config().get_property("Hardware", "samplerate", 44100).toInt();
        ads.bufferSize = config().get_property("Hardware", "buffersize", 512).toInt();
#if defined (Q_WS_X11)
        ads.driverType = config().get_property("Hardware", "drivertype", "ALSA").toString();
#else
        ads.driverType = config().get_property("Hardware", "drivertype", "PortAudio").toString();
#endif
        ads.cardDevice = config().get_property("Hardware", "carddevice", "default").toString();
        ads.ditherShape = config().get_property("Hardware", "DitherShape", "None").toString();
        ads.capture = config().get_property("Hardware", "capture", 1).toInt();
        ads.playback = config().get_property("Hardware", "playback", 1).toInt();

        if (ads.bufferSize == 0) {
                qWarning("BufferSize read from Settings is 0 !!!");
                ads.bufferSize = 1024;
        }
        if (ads.rate == 0) {
                qWarning("Samplerate read from Settings is 0 !!!");
                ads.rate = 44100;
        }
        if (ads.driverType.isEmpty()) {
                qWarning("Driver type read from Settings is an empty String !!!");
                ads.driverType = "ALSA";
        }

#if defined (ALSA_SUPPORT)
        if (ads.driverType == "ALSA") {
                ads.cardDevice = config().get_property("Hardware", "carddevice", "default").toString();
        }
#endif

#if defined (PORTAUDIO_SUPPORT)
        if (ads.driverType == "PortAudio") {
#if defined (Q_WS_X11)
                ads.cardDevice = config().get_property("Hardware", "pahostapi", "alsa").toString();
#elif defined (Q_WS_MAC)
                ads.cardDevice = config().get_property("Hardware", "pahostapi", "coreaudio").toString();
#elif defined (Q_WS_WIN)
                ads.cardDevice = config().get_property("Hardware", "pahostapi", "wmme").toString();
#endif
        }
#endif // end PORTAUDIO_SUPPORT

        ads.busConfigs = busConfigs;
        ads.channelConfigs = channelConfigs;

        audiodevice().set_parameters(ads);
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

void Project::set_songwriter(const QString& sw)
{
	m_songwriter = sw;
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
	foreach(Sheet* sheet, m_sheets) {
		if(sheet->is_changed())
			return true;
	}
	return false;
}


Command* Project::add_sheet(Sheet* sheet, bool historable)
{
	PENTER;
	
	AddRemove* cmd;
	cmd = new AddRemove(this, sheet, historable, 0,
		"private_add_sheet(Sheet*)", "sheetAdded(Sheet*)",
       		"private_remove_sheet(Sheet*)", "sheetRemoved(Sheet*)",
       		tr("Sheet %1 added").arg(sheet->get_title()));
	
	cmd->set_instantanious(true);
	
	return cmd;
}


void Project::set_current_sheet(qint64 id)
{
	PENTER;
	
	if (m_currentSheetId == id) {
		return;
	}
	
	Sheet* newcurrent = 0;
	
	foreach(Sheet* sheet, m_sheets) {
		if (sheet->get_id() == id) {
			newcurrent = sheet;
			break;
		}
	}
	
	if (!newcurrent) {
		info().information( tr("Sheet '%1' doesn't exist!").arg(id) );
		emit currentSheetChanged(0);
		return;
	}

	m_currentSheetId=id;
	
	emit currentSheetChanged(newcurrent);
}


Sheet* Project::get_current_sheet() const
{
	Sheet* current = 0;
	
	foreach(Sheet* sheet, m_sheets) {
		if (sheet->get_id() == m_currentSheetId) {
			current = sheet;
			break;
		}
	}
	
	return current;
}


Sheet* Project::get_sheet(qint64 id) const
{
	Sheet* current = 0;
	
	foreach(Sheet* sheet, m_sheets) {
		if (sheet->get_id() == id) {
			current = sheet;
			break;
		}
	}
	
	return current;
}


Command* Project::remove_sheet(Sheet* sheet, bool historable)
{
	AddRemove* cmd;
	cmd = new AddRemove(this, sheet, historable, 0,
		"private_remove_sheet(Sheet*)", "sheetRemoved(Sheet*)",
		"private_add_sheet(Sheet*)", "sheetAdded(Sheet*)",
		tr("Remove Sheet %1").arg(sheet->get_title()));
	
	cmd->set_instantanious(true);
	
	return cmd;
}

/* call this function to initiate the export or cd-writing */
int Project::export_project(ExportSpecification* spec)
{
	PENTER;
	
	if (!m_exportThread) {
		m_exportThread = new ExportThread(this);
	}
	
	if (m_exportThread->isRunning()) {
		info().warning(tr("Export already in progress, cannot start it twice!"));
		return -1;
	}
	
	QDir dir(spec->exportdir);
	if (!spec->exportdir.isEmpty() && !dir.exists()) {
		if (!dir.mkdir(spec->exportdir)) {
			info().warning(tr("Unable to create export directory! Please check permissions for this directory: %1").arg(spec->exportdir));
			return -1;
		}
	}
	
	spec->progress = 0;
	spec->running = true;
	spec->stop = false;
	spec->breakout = false;

	m_exportThread->set_specification(spec);

        // this will start the thread by executing ExportThread::run(),
        // which calls Project::start_export()
	m_exportThread->start();

	return 0;
}

int Project::start_export(ExportSpecification* spec)
{
	PMESG("Starting export, rate is %d bitdepth is %d", spec->sample_rate, spec->data_width );

	spec->blocksize = 32768;

	spec->dataF = new audio_sample_t[spec->blocksize * spec->channels];
	audio_sample_t* readbuffer = new audio_sample_t[spec->blocksize * spec->channels];

	overallExportProgress = renderedSheets = 0;
	sheetsToRender.clear();

        // determine which sheets to export, store them in sheetsToRender
	if (spec->allSheets) {
		foreach(Sheet* sheet, m_sheets) {
			sheetsToRender.append(sheet);
		}
	} else {
		Sheet* sheet = get_current_sheet();
		if (sheet) {
			sheetsToRender.append(sheet);
		}
	}

        // process each sheet in the list sheetsToRender. here we set the renderpass mode,
        // and then call Sheet::repare_export() and Sheet::render(), which do the actual
        // processing.
	foreach(Sheet* sheet, sheetsToRender) {
		PMESG("Starting export for sheet %lld", sheet->get_id());
		emit exportStartedForSheet(sheet);
		spec->resumeTransport = false;
                spec->resumeTransportLocation = sheet->get_transport_location();
		sheet->readbuffer = readbuffer;
		
		if (spec->normalize) {
                        // start one render pass in mode "CALC_NORM_FACTOR"
			spec->peakvalue = 0.0;
			spec->renderpass = ExportSpecification::CALC_NORM_FACTOR;
			
			
			if (sheet->prepare_export(spec) < 0) {
				PERROR("Failed to prepare sheet for export");
				continue;
			}
			
                        sheet->start_export(spec);
			
			spec->normvalue = (1.0 - FLT_EPSILON) / spec->peakvalue;
			
			if (spec->peakvalue > 1.0) {
				info().critical(tr("Detected clipping in exported audio! (%1)")
						.arg(coefficient_to_dbstring(spec->peakvalue)));
			}
			
			if (!spec->breakout) {
				info().information(tr("calculated norm factor: %1").arg(coefficient_to_dbstring(spec->normvalue)));
			}
		}

                // start the real render pass in mode "WRITE_TO_HARDDISK"
		spec->renderpass = ExportSpecification::WRITE_TO_HARDDISK;
		
                // first call Sheet::prepare_export()...
		if (sheet->prepare_export(spec) < 0) {
			PERROR("Failed to prepare sheet for export");
			break;
		}
		
                // ... then start the render process and wait until it's finished
                sheet->start_export(spec);
		
		if (!QMetaObject::invokeMethod(sheet, "set_transport_pos",  Qt::QueuedConnection, Q_ARG(TimeRef, spec->resumeTransportLocation))) {
			printf("Invoking Sheet::set_transport_pos() failed\n");
		}
		if (spec->resumeTransport) {
			if (!QMetaObject::invokeMethod(sheet, "start_transport",  Qt::QueuedConnection)) {
				printf("Invoking Sheet::start_transport() failed\n");
			}
		}
		if (spec->breakout) {
			break;
		}
		renderedSheets++;
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

/* returns the total time of the data that will be written to CD */
TimeRef Project::get_cd_totaltime(ExportSpecification* spec)
{
        TimeRef totalTime = TimeRef();

        spec->renderpass = ExportSpecification::CREATE_CDRDAO_TOC;

        if (spec->allSheets) {
                foreach(Sheet* sheet, m_sheets) {
                        sheet->prepare_export(spec);
                        totalTime += spec->totalTime;
                }
        } else {
                Sheet* sheet = get_current_sheet();
                sheet->prepare_export(spec);
                totalTime += spec->totalTime;
        }

        return totalTime;
}

int Project::create_cdrdao_toc(ExportSpecification* spec)
{
	QList<Sheet* > sheets;
	QString filename = spec->exportdir;
	
	if (spec->allSheets) {
		foreach(Sheet* sheet, m_sheets) {
			sheets.append(sheet);
		}

                // filename of the toc file is "project-name.toc"
		filename += get_title() + ".toc";
	} else {
		Sheet* sheet = get_current_sheet();
                if (!sheet) {
			return -1;
		}
		sheets.append(sheet);

                // filename of the toc file is "sheet-name.toc"
                filename += spec->basename + ".toc";
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
	
	foreach(Sheet* sheet, sheets) {
		if (sheet->prepare_export(spec) < 0) {
			return -1;
		}
		output += sheet->get_cdrdao_tracklist(spec, pregap);
		pregap = false; // only add the pregap at the first sheet
	}
	

	if (spec->writeToc) {
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
	if (index <= m_sheets.size() && index > 0) {
		set_current_sheet(m_sheets.at(index - 1)->get_id());
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

void Project::set_sheet_export_progress(int progress)
{
	overallExportProgress = (progress / sheetsToRender.count()) + 
			(renderedSheets * (100 / sheetsToRender.count()) );

	emit sheetExportProgressChanged(progress);
	emit overallExportProgressChanged(overallExportProgress);
}

void Project::set_export_message(QString message)
{
        emit exportMessage(message);
}


QList<Sheet* > Project::get_sheets( ) const
{
	return m_sheets;
}

int Project::get_sheet_index(qint64 id) const
{
	for (int i=0; i<m_sheets.size(); ++i) {
		if (m_sheets.at(i)->get_id() == id) {
			return i + 1;
		}
	}
	
	return 0;
}


int Project::get_current_sheet_id( ) const
{
	return m_currentSheetId;
}

int Project::get_num_sheets( ) const
{
	return m_sheets.size();
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


void Project::private_add_sheet(Sheet * sheet)
{
	PENTER;
	m_sheets.append(sheet);
	sheet->connect_to_audiodevice();
	
	set_current_sheet(sheet->get_id());
}

void Project::private_remove_sheet(Sheet * sheet)
{
	PENTER;
	m_sheets.removeAll(sheet);
	
	if (m_sheets.isEmpty()) {
		m_currentSheetId = -1;
	}
		
	qint64 newcurrent = 0;
		
	if (m_sheets.size() > 0) {
		newcurrent = m_sheets.last()->get_id();
	}
		
	set_current_sheet(newcurrent);

	sheet->disconnect_from_audiodevice();
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
	foreach(Sheet* sheet, m_sheets) {
		if (sheet->is_recording() && sheet->is_transport_rolling()) {
			return true;
		}
	}
	return false;
}

