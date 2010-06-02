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

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QList>
#include <QDomNode>
#include "ContextItem.h"
#include "APILinkedList.h"

#include "defines.h"

class AudioBus;
class Sheet;
class ResourcesManager;
struct ExportSpecification;
class ExportThread;
class AudioDeviceClient;
class SubGroup;

class Project : public ContextItem
{
	Q_OBJECT

public :
	~Project();

        int process(nframes_t nframes);
        // jackd only feature
        int transport_control(transport_state_t state);

        AudioBus* get_playback_bus(const QString& name) const;
        AudioBus* get_capture_bus(const QString& name) const;

        AudioBus* get_bus(qint64 id);
        qint64 get_bus_id_for(const QString& busName);

        QStringList get_playback_buses_names( ) const;
        QStringList get_capture_buses_names( ) const;

        QList<AudioBus*> get_playback_buses() const {return m_hardwareBuses;}


	// Get functions
	int get_current_sheet_id() const;
	int get_num_sheets() const;
	int get_rate() const;
	int get_bitdepth() const;

        QStringList get_input_buses_for(SubGroup* subGroup);
	
	ResourcesManager* get_audiosource_manager() const;
        SubGroup* get_master_out() const {return m_masterOut;}
	QString get_title() const;
	QString get_engineer() const;
	QString get_description() const;
	QString get_discid() const;
	QString get_performer() const;
	QString get_arranger() const;
	QString get_songwriter() const;
	QString get_message() const;
	QString get_upc_ean() const;
	int get_genre();
	QString get_root_dir() const;
	QString get_audiosources_dir() const;
	QString get_import_dir() const;
	QString get_error_string() const {return m_errorString;}
	QList<Sheet* > get_sheets() const;
	Sheet* get_current_sheet() const ;
	Sheet* get_sheet(qint64 id) const;
        int get_sheet_index(qint64 id);
        int get_keyboard_arrow_key_navigation_speed() const {return m_keyboardArrowNavigationSpeed;}
	QDomNode get_state(QDomDocument doc, bool istemplate=false);


	// Set functions
	void set_title(const QString& title);
	void set_engineer(const QString& pEngineer);
	void set_description(const QString& des);
	void set_discid(const QString& pId);
	void set_performer(const QString& pPerformer);
	void set_arranger(const QString& pArranger);
	void set_songwriter(const QString& sw);
	void set_message(const QString& pMessage);
	void set_upc_ean(const QString& pUPC);
	void set_genre(int pGenre);
	void set_sheet_export_progress(int pogress);
        void set_export_message(QString message);
	void set_current_sheet(qint64 id);
	void set_import_dir(const QString& dir);
        void set_sheets_are_tracks_folder(bool isFolder);
        void set_work_at(TimeRef worklocation);
        void set_keyboard_arrow_key_navigation_speed(int speed) {m_keyboardArrowNavigationSpeed = speed;}
        int save_from_template_to_project_file(const QString& file, const QString& projectName);

	
	Command* add_sheet(Sheet* sheet, bool historable=true);
	Command* remove_sheet(Sheet* sheet, bool historable=true);
	
	bool has_changed();
	bool is_save_to_close() const;
	bool is_recording() const;
        bool sheets_are_track_folder() const {return m_sheetsAreTrackFolder;}
	
	int save(bool autosave=false);
	int load(QString projectfile = "");
	int export_project(ExportSpecification* spec);
	int start_export(ExportSpecification* spec);
	int create_cdrdao_toc(ExportSpecification* spec);
        TimeRef get_cd_totaltime(ExportSpecification*);

	enum {
		SETTING_XML_CONTENT_FAILED = -1,
  		PROJECT_FILE_COULD_NOT_BE_OPENED = -2,
    		PROJECT_FILE_VERSION_MISMATCH = -3
	};

        void connect_to_audio_device();
        int disconnect_from_audio_device();


public slots:
	Command* select();

private:
	Project(const QString& title);
	
        QList<Sheet*>           m_sheets;
        APILinkedList           m_RtSheets;
	ResourcesManager* 	m_resourcesManager;
        ExportThread*           m_exportThread;
        AudioDeviceClient*	m_audiodeviceClient;
        SubGroup*               m_masterOut;

        QList<AudioBus* >       m_hardwareBuses;



	QString 	m_title;
	QString 	m_rootDir;
	QString 	m_sourcesDir;
	QString 	engineer;
	QString		m_description;
	QString		m_importDir;
	QString		m_discid;
	int		m_genre;
	QString		m_upcEan;
	QString		m_performer;
	QString		m_arranger;
	QString		m_songwriter;
	QString		m_message;
	QString		m_errorString;

	int		m_rate;
	int		m_bitDepth;
        int             m_keyboardArrowNavigationSpeed;
	bool		m_useResampling;
        bool            m_sheetsAreTrackFolder;

	int		overallExportProgress;
	int 		renderedSheets;
	QList<Sheet* > 	sheetsToRender;

	qint64 		m_currentSheetId;
	
	int create(int sheetcount, int numtracks);
	int create_audiosources_dir();
	int create_peakfiles_dir();

        void prepare_audio_device(QDomDocument doc);
        void set_current_sheet(Sheet* sheet);
	
	friend class ProjectManager;

private slots:
	void private_add_sheet(Sheet* sheet);
	void private_remove_sheet(Sheet* sheet);
        void sheet_removed(Sheet* sheet);
        void sheet_added(Sheet* sheet);

signals:
	void currentSheetChanged(Sheet* );
        void privateSheetAdded(Sheet*);
	void sheetAdded(Sheet*);
        void privateSheetRemoved(Sheet*);
        void sheetRemoved(Sheet*);
        void sheetExportProgressChanged(int );
	void overallExportProgressChanged(int );
	void exportFinished();
	void exportStartedForSheet(Sheet* );
	void projectLoadFinished();
        void exportMessage(QString);
};

#endif
