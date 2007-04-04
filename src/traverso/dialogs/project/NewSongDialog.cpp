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


#include "NewSongDialog.h"

#include <libtraversocore.h>

NewSongDialog::NewSongDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
	
	trackCountSpinBox->setValue(config().get_property("Song", "trackCreationCount", 4).toInt());
	
	set_project(pm().get_project());
	
	use_template_checkbox_state_changed(Qt::Unchecked);
	
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(useTemplateCheckBox, SIGNAL(stateChanged (int)), this, SLOT(use_template_checkbox_state_changed(int)));
}

void NewSongDialog::accept()
{
	if (! m_project) {
		info().information(tr("I can't create a new Song if there is no Project loaded!!"));
		return;
	}
	
	int count = countSpinBox->value();
	int trackcount = trackCountSpinBox->value();
	QString title = titleLineEdit->text();
	
	if (title.isEmpty()) {
		title = "Untitled";
	}
	
	int index = templateComboBox->currentIndex();
	bool usetemplate = false;
	QDomNode node;
	if (useTemplateCheckBox->isChecked() && index >= 0) {
		usetemplate = true;
		Song* templatesong = m_project->get_song(templateComboBox->itemData(index).toLongLong());
		Q_ASSERT(templatesong);
		QDomDocument doc("Song");
		node = templatesong->get_state(doc, usetemplate);
	}
	
	for (int i=0; i<count; ++i) {
		Song* song;
		if (usetemplate) {
			song = new Song(m_project);
			song->set_state(node);
		} else {
			song = new Song(m_project, trackcount);
		}
		song->set_title(title);
		Command::process_command(m_project->add_song(song));
	}
		
	hide();
}

void NewSongDialog::set_project(Project * project)
{
	m_project = project;
	
	if (! m_project) {
		templateComboBox->clear();
		return;
	}
	
	connect(m_project, SIGNAL(songAdded(Song*)), this, SLOT(update_template_combo()));
	connect(m_project, SIGNAL(songRemoved(Song*)), this, SLOT(update_template_combo()));
	
	update_template_combo();
}

void NewSongDialog::reject()
{
	hide();
}

void NewSongDialog::update_template_combo()
{
	templateComboBox->clear();
	
	foreach(Song* song, m_project->get_songs()) {
		QString text = "Song " + QString::number(m_project->get_song_index(song->get_id())) +
				" " + song->get_title();
		
		templateComboBox->addItem(text, song->get_id());
		connect(song, SIGNAL(propertyChanged()), this, SLOT(update_template_combo()));
	}
}

void NewSongDialog::use_template_checkbox_state_changed(int state)
{
	if (state == Qt::Checked) {
		templateComboBox->setEnabled(true);
		trackCountSpinBox->setEnabled(false);
	} else {
		templateComboBox->setEnabled(false);
		trackCountSpinBox->setEnabled(true);
	}
}

