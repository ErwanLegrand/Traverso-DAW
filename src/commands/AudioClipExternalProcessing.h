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

#ifndef AUDIOCLIP_EXTERNAL_PROCESSING_H
#define AUDIOCLIP_EXTERNAL_PROCESSING_H

#include <QProcess>
#include <Command.h>
#include "ui_ExternalProcessing.h"
#include <QDialog>

class AudioClip;
class Track;
class AudioClipExternalProcessing;

class ExternalProcessingDialog : public QDialog, protected Ui::ExternalProcessing
{
	Q_OBJECT
	
public:
	ExternalProcessingDialog(QWidget* parent, AudioClipExternalProcessing* acep);
	~ExternalProcessingDialog();


private:
	AudioClipExternalProcessing* m_acep;
	QProcess* m_processor;
	QString m_filename;
	QString m_program;
	bool m_queryOptions;
	QStringList m_arguments;
	QString m_commandargs;
	QString m_infilename;
	QString m_outfilename;
	QString m_newClipName;
	
	void query_options();

private slots:
	void read_standard_output();
	void prepare_for_external_processing();
	void process_started();
	void process_finished(int exitcode, QProcess::ExitStatus exitstatus);
	void arg_combo_index_changed ( const QString & text );
	void start_external_processing();
	void command_lineedit_text_changed(const QString & text);
	void process_error(QProcess::ProcessError error);
};


class AudioClipExternalProcessing : public Command
{
public :
	AudioClipExternalProcessing(AudioClip* object);
	~AudioClipExternalProcessing();

	int prepare_actions();
	int do_action();
	int undo_action();

private :
	Track* m_track;
	AudioClip* m_clip;
	AudioClip* m_resultingclip;
	QProcess* m_processor;
	
	friend class ExternalProcessingDialog;
	
};

#endif

//eof
