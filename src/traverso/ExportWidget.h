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

#ifndef EXPORTWIDGET_H
#define EXPORTWIDGET_H

#include "ui_ExportWidget.h"

#include <QDialog>
#include <QProcess>

class Project;
class Song;
struct ExportSpecification;

class ExportWidget : public QDialog, protected Ui::ExportWidget
{
        Q_OBJECT

public:
        ExportWidget(QWidget* parent = 0);
        ~ExportWidget();
	
	void set_was_closed();
	
protected:
	void closeEvent(QCloseEvent* event);

private:
        Project* m_project;
	QProcess* m_burnprocess;
	ExportSpecification* 	m_exportSpec;
	
        void show_progress_view();
        void show_settings_view();
	
	bool is_save_to_export();
	void cd_render();
	void write_to_cd();
	void disable_ui_interaction();
	void enable_ui_interaction();
	
	void update_cdburn_status(const QString& message, int type);
	void unlock_device();
	
	enum {
		NO_STATE,
  		RENDER,
  		BURNING,
    		ABORT_BURN,
  		QUERY_DEVICE,
    		UNLOCK_DEVICE,
		NORMAL_MESSAGE,
    		ERROR_MESSAGE
	};
	
	int m_writingState;
	bool m_wasClosed;
	QString get_device(int index);

private slots:
	void set_project(Project* project);
	void update_song_progress(int progress);
        void update_overall_progress(int progress);
        void render_finished();
        void set_exporting_song(Song* song);

        void on_fileSelectButton_clicked();
        void on_exportStartButton_clicked();
        void on_exportStopButton_clicked();
	void on_cancelButton_clicked();
	void export_only_changed(int state);

	void start_burn_process();
	void stop_burn_process();
	void read_standard_output();
	void cdrdao_process_started();
	void cdrdao_process_finished(int exitcode, QProcess::ExitStatus exitstatus);
	void cd_export_finished();
	void cd_export_progress(int progress);
	void query_devices();
	
	void reject();
};

#endif

//eof


