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


#include "ThemeModifierDialog.h"

#include "Themer.h"

#include <QColorDialog>
#include <QListWidgetItem>

ThemeModifierDialog::ThemeModifierDialog(QWidget* parent)
        : QDialog(parent)
{
        setupUi(this);

        listWidget->setMinimumWidth(300);

        m_colorDialog = new QColorDialog();
        m_colorDialog->setWindowFlags(Qt::Widget);
        m_colorDialog->setOptions(QColorDialog::NoButtons |
                                  QColorDialog::ShowAlphaChannel |
                                  QColorDialog::DontUseNativeDialog);

        QList<QString> colors = themer()->get_colors();
        qSort(colors.begin(), colors.end());

        foreach(const QString& color, colors) {
                listWidget->addItem(color);
        }

        horizontalLayout->addWidget(m_colorDialog);

        connect(m_colorDialog, SIGNAL(currentColorChanged(const QColor&)),
                this, SLOT(current_color_changed(const QColor&)));
        connect(listWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
                this, SLOT(list_widget_item_changed(QListWidgetItem*, QListWidgetItem*)));
}

void ThemeModifierDialog::current_color_changed(const QColor& color)
{
        themer()->set_new_theme_color(m_currentColor, color);
}

void ThemeModifierDialog::list_widget_item_changed(QListWidgetItem* item, QListWidgetItem*)
{
        m_currentColor = item->text();
        QColor color = themer()->get_color(m_currentColor);
        m_colorDialog->setCurrentColor(color);
}

void ThemeModifierDialog::accept()
{
        themer()->save();
        close();
}
