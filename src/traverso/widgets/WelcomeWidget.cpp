/*
Copyright (C) 2010 Remon Sijrier

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


#include "WelcomeWidget.h"

#include "Config.h"
#include "ProjectManager.h"
#include "Project.h"
#include "Interface.h"

#include <QMessageBox>


WelcomeWidget::WelcomeWidget(QWidget *parent)
        : QWidget(parent)
{
        setupUi(this);

        update_previous_project_line_edit();
        update_projects_combo_box();

        connect(loadPreviousProjectButton, SIGNAL(clicked()), this, SLOT(load_previous_project_button_clicked()));
        connect(loadExistingProjectButton, SIGNAL(clicked()), this, SLOT(load_existing_project_button_clicked()));
        connect(createProjectPushbutton, SIGNAL(clicked()), this, SLOT(create_new_project_button_clicked()));
        connect(&pm(), SIGNAL(currentProjectDirChanged()), this, SLOT(update_projects_combo_box()));
        connect(&pm(), SIGNAL(projectsListChanged()), this, SLOT(update_projects_combo_box()));
        connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
}


WelcomeWidget::~WelcomeWidget()
{

}

void WelcomeWidget::set_project(Project* project)
{
        if (project) {
                previousProjectLabel->setText(tr("Resume running Project (%1)").arg(project->get_title()));
                loadPreviousProjectButton->setText(tr("Resume"));
                previousProjectLineEdit->setText(project->get_title());
        } else {
                previousProjectLabel->setText(tr("Load previous Project"));
                loadPreviousProjectButton->setText(tr("Load"));

        }
}


void WelcomeWidget::load_existing_project_button_clicked()
{
        Project* project = pm().get_project();
        QString projectToLoad = projectsComboBox->currentText();


        if (project && (project->get_title() == projectToLoad)) {
                QMessageBox::StandardButton button = QMessageBox::question(this,
                        "Traverso - Question",
                        tr("Project '%1'' is already running \n\n"
                           "Do you really wish to reload it?").arg(projectToLoad),
                        QMessageBox::Ok | QMessageBox::Cancel,
                        QMessageBox::Cancel );
                        if (button == QMessageBox::Cancel) {
                        return;
                }
        }

        pm().load_project(projectToLoad);
}


void WelcomeWidget::load_previous_project_button_clicked()
{
        Project* project = pm().get_project();
        if (project) {
                Interface::instance()->show_current_sheet();

        } else {

                QString previous = previousProjectLineEdit->text();
                if (previous.isEmpty() || previous.isNull()) {
                        return;
                }
                pm().load_project(previous);
        }
}

void WelcomeWidget::create_new_project_button_clicked()
{
        Interface::instance()->show_newproject_dialog();
}

void WelcomeWidget::update_projects_combo_box()
{
        projectsComboBox->clear();

        foreach(QString project, pm().get_projects_list()) {
                projectsComboBox->addItem(project);
        }

        update_previous_project_line_edit();

}

void WelcomeWidget::update_previous_project_line_edit()
{
        QString current = config().get_property("Project", "current", "").toString();
        if (pm().project_exists(current)) {
                previousProjectLineEdit->setText(current);
        } else {
                previousProjectLineEdit->setText("");
        }

}

//eof
