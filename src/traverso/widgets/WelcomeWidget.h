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


#ifndef WELCOMEWIDGET_H
#define WELCOMEWIDGET_H

#include <QWidget>
#include "ui_WelcomeWidget.h"

class WelcomeWidget : public QWidget, protected Ui::WelcomeWidget
{
        Q_OBJECT;

public:

        WelcomeWidget(QWidget* parent);
        ~WelcomeWidget();

private slots:
        void load_existing_project_button_clicked();
        void load_previous_project_button_clicked();
        void create_new_project_button_clicked();
        void update_projects_combo_box();
        void update_previous_project_line_edit();
};

#endif // WELCOMEWIDGET_H
