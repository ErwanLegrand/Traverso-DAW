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

#include "ProjectManagerDialog.h"

#include "libtraversocore.h"
#include <QStringList>
#include <QInputDialog>
#include <QHeaderView>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <dialogs/project/NewSongDialog.h>
#include <Interface.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ProjectManagerDialog::ProjectManagerDialog( QWidget * parent )
	: QDialog(parent)
{
	setupUi(this);

	treeSongWidget->setColumnCount(3);
	treeSongWidget->header()->resizeSection(0, 160);
	treeSongWidget->header()->resizeSection(1, 55);
	treeSongWidget->header()->resizeSection(2, 70);
	QStringList stringList;
	stringList << "Song Name" << "Tracks" << "Length";
	treeSongWidget->setHeaderLabels(stringList);
	
	set_project(pm().get_project());
	
	undoButton->setIcon(QIcon(find_pixmap(":/undo-16")));
	redoButton->setIcon(QIcon(find_pixmap(":/redo-16")));
	
	connect(treeSongWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(songitem_clicked(QTreeWidgetItem*,int)));
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
}

ProjectManagerDialog::~ProjectManagerDialog()
{}

void ProjectManagerDialog::set_project(Project* project)
{
	m_project = project;
	
	if (m_project) {
		connect(m_project, SIGNAL(songAdded(Song*)), this, SLOT(update_song_list()));
		connect(m_project, SIGNAL(songRemoved(Song*)), this, SLOT(update_song_list()));
		connect(m_project->get_history_stack(), SIGNAL(redoTextChanged ( const QString &)),
			this, SLOT(redo_text_changed(const QString&)));
		connect(m_project->get_history_stack(), SIGNAL(undoTextChanged ( const QString &)),
			this, SLOT(undo_text_changed(const QString&)));
		setWindowTitle("Manage Project - " + m_project->get_title());
		descriptionTextEdit->setText(m_project->get_description());
	} else {
		setWindowTitle("Manage Project - No Project loaded!");
		treeSongWidget->clear();
		descriptionTextEdit->clear();
	}
	
	update_song_list();
}


void ProjectManagerDialog::update_song_list( )
{
	if ( ! m_project) {
		printf("ProjectManagerDialog:: no project ?\n");
		return;
	}
	
	treeSongWidget->clear();
	foreach(Song* song, m_project->get_songs()) {

		QString songNr = QString::number(m_project->get_song_index(song->get_id()));
		QString songName = "Song " + songNr + " - " + song->get_title();
		QString numberOfTracks = QString::number(song->get_numtracks());
		QString songLength = frame_to_smpte(song->get_last_frame(), song->get_rate());
		QString songStatus = song->is_changed()?"UnSaved":"Saved";
		QString songSpaceAllocated = "Unknown";

		QTreeWidgetItem* item = new QTreeWidgetItem(treeSongWidget);
		item->setTextAlignment(1, Qt::AlignHCenter);
		item->setTextAlignment(2, Qt::AlignHCenter);
		item->setText(0, songName);
		item->setText(1, numberOfTracks);
		item->setText(2, songLength);
		
		item->setData(0, Qt::UserRole, song->get_id());
	}
}

void ProjectManagerDialog::songitem_clicked( QTreeWidgetItem* item, int)
{
	if (!item) {
		return;
	}

	Song* song;

	qint64 id = item->data(0, Qt::UserRole).toLongLong();
	song = m_project->get_song(id);

	Q_ASSERT(song);
		
	selectedSongName->setText(song->get_title());
	
	m_project->set_current_song(song->get_id());
}

void ProjectManagerDialog::on_renameSongButton_clicked( )
{
	if( ! m_project) {
		return;
	}
	
	QTreeWidgetItem* item = treeSongWidget->currentItem();
	
	if ( ! item) {
		return;
	}
	
	qint64 id = item->data(0, Qt::UserRole).toLongLong();
	Song* song = m_project->get_song(id);
	
	Q_ASSERT(song);
	
	QString newtitle = selectedSongName->text();
	
	if (newtitle.isEmpty()) {
		info().information(tr("No new Song name was supplied!"));
		return;
	}
	
	song->set_title(newtitle);

	update_song_list();
}

void ProjectManagerDialog::on_deleteSongButton_clicked( )
{
	QTreeWidgetItem* item = treeSongWidget->currentItem();
	
	if ( ! item ) {
		return;
	}
	
	qint64 id = item->data(0, Qt::UserRole).toLongLong();
	
	Command::process_command(m_project->remove_song(m_project->get_song(id)));
}

void ProjectManagerDialog::on_createSongButton_clicked( )
{
	Interface::instance()->show_newsong_dialog();
}

void ProjectManagerDialog::redo_text_changed(const QString & text)
{
	redoButton->setText(text);
}

void ProjectManagerDialog::undo_text_changed(const QString & text)
{
	undoButton->setText(text);
}

void ProjectManagerDialog::on_undoButton_clicked()
{
	if (! m_project ) {
		return;
	}
	
	m_project->get_history_stack()->undo();
}

void ProjectManagerDialog::on_redoButton_clicked()
{
	if (! m_project ) {
		return;
	}
	
	m_project->get_history_stack()->redo();
}

void ProjectManagerDialog::on_songsExportButton_clicked()
{
	Interface::instance()->show_export_widget();
}

void ProjectManagerDialog::on_exportTemplateButton_clicked()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("Save Template"),
					     tr("Enter Template name"),
					     QLineEdit::Normal, "", &ok);
	if (! ok || text.isEmpty()) {
		return;
	}
	
	QString fileName = QDir::homePath() + "/.traverso/ProjectTemplates/";
	
	QDir dir;
	if (! dir.exists(fileName)) {
		if (! dir.mkdir(fileName)) {
			info().critical( tr("Couldn't open file %1 for writing!").arg(fileName));
			return;
		}
	}
	
	fileName.append(text + ".tpt");
	
	QDomDocument doc(text);
	
	if (QFile::exists(fileName)) {
		QMessageBox::StandardButton button = QMessageBox::question(this,
				tr("Traverso - Information"),
				tr("Template with name %1 allready exists!\n Do you want to overwrite it?").arg(fileName),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No);
				
		if (button == QMessageBox::No) {
			return;
		}
	}
	
	QFile file(fileName);

	if (file.open( QIODevice::WriteOnly ) ) {
		m_project->get_state(doc, true);
		QTextStream stream(&file);
		doc.save(stream, 4);
		file.close();
		info().information(tr("Saved Project Template: %1").arg(text));
	} else {
		info().critical( tr("Couldn't open file %1 for writing!").arg(fileName));
	}
	
}

void ProjectManagerDialog::accept()
{
	if ( ! m_project ) {
		hide();
		return;
	}
	
	m_project->set_description(descriptionTextEdit->toPlainText());
	
	hide();
}

void ProjectManagerDialog::reject()
{
	if ( ! m_project ) {
		hide();
		return;
	}
	
	descriptionTextEdit->setText(m_project->get_description());
	hide();
}

//eof

