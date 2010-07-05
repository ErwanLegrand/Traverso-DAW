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

#ifndef ThemeModifierDialog_H
#define ThemeModifierDialog_H

#include "ui_ThemeModifierDialog.h"

#include <QColor>
#include <QDialog>


class QColorDialog;
class QListWidgetItem;


class ThemeModifierDialog : public QDialog, protected Ui::ThemeModifierDialog
{
        Q_OBJECT

public:
        ThemeModifierDialog(QWidget* parent);

private:
        QColorDialog*   m_colorDialog;
        QString         m_currentColor;


public slots:
        void accept();

private slots:
        void current_color_changed(const QColor &);
        void list_widget_item_changed(QListWidgetItem* item, QListWidgetItem*);
};

#endif // ThemeModifierDialog_H
