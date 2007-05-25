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


#include "NewTrackDialog.h"
#include <QPushButton>

#include <libtraversocore.h>
#include <CommandGroup.h>

NewTrackDialog::NewTrackDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
	
	set_project(pm().get_project());
	
	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
}

void NewTrackDialog::accept()
{
	if (! m_project) {
		info().information(tr("I can't create a new Track if there is no Project loaded!!"));
		return;
	}
	
	Song* song = m_project->get_current_song();
	if ( ! song ) {
		return ;
	}
	
	int count = countSpinBox->value();
	QString title = titleLineEdit->text();
	
	if (title.isEmpty()) {
		title = "Untitled";
	}
	
	CommandGroup* group = new CommandGroup(song, "");
	
	for (int i=0; i<count; ++i) {
		Track* track = new Track(song, "Unnamed", Track::INITIAL_HEIGHT);
		track->set_name(title);
		group->add_command(song->add_track(track));
	}
		
	group->setText(tr("Added %n Track(s)", "", count));
	Command::process_command(group);
	
	hide();
}

void NewTrackDialog::set_project(Project * project)
{
	m_project = project;
}

void NewTrackDialog::reject()
{
	hide();
}

