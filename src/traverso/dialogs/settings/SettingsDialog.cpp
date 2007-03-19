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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02100-1301  USA.

*/

#include <QtGui>

#include "SettingsDialog.h"
#include "Pages.h"

#include <Debugger.h>

SettingsDialog::SettingsDialog(QWidget* parent)
	: QDialog(parent)
{
	contentsWidget = new QListWidget;
	contentsWidget->setViewMode(QListView::IconMode);
	contentsWidget->setIconSize(QSize(32, 32));
	contentsWidget->setMovement(QListView::Static);
	contentsWidget->setMaximumWidth(140);
	contentsWidget->setMinimumWidth(135);
	contentsWidget->setMinimumHeight(350);
	contentsWidget->setSpacing(12);
	
	pagesWidget = new QStackedWidget;
	pagesWidget->addWidget(new BehaviorPage);
	pagesWidget->addWidget(new AppearancePage);
	pagesWidget->addWidget(new AudioDriverPage);
	pagesWidget->addWidget(new DiskIOPage);
	pagesWidget->addWidget(new KeyboardPage);
	
	createIcons();
	contentsWidget->setCurrentRow(0);
	
	QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
	
	QPushButton* closeButton = buttonBox->addButton(QDialogButtonBox::Close);
	QPushButton* saveButton = buttonBox->addButton(QDialogButtonBox::Save); 
	QPushButton* restoreDefaultsButton = buttonBox->addButton(QDialogButtonBox::RestoreDefaults); 
	
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(save_config()));
	connect(restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(restore_defaults_button_clicked()));
	
	QHBoxLayout *buttonsLayout = new QHBoxLayout;
	buttonsLayout->addWidget(buttonBox);
	
	QHBoxLayout *horizontalLayout = new QHBoxLayout;
	horizontalLayout->addWidget(contentsWidget);
	horizontalLayout->addWidget(pagesWidget, 1);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(horizontalLayout);
	mainLayout->addStretch(1);
	mainLayout->addSpacing(12);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
	
	setWindowTitle(tr("Configure - Traverso"));
	
	resize(500, 400);
}

void SettingsDialog::createIcons()
{
	QListWidgetItem* behaviorButton = new QListWidgetItem(contentsWidget);
	behaviorButton->setIcon(QIcon(":/songmanagement"));
	behaviorButton->setTextAlignment(Qt::AlignHCenter);
	behaviorButton->setText(tr("Behavior"));
	behaviorButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	behaviorButton->setSizeHint(QSize(100, 50));

	QListWidgetItem* appearanceButton = new QListWidgetItem(contentsWidget);
	appearanceButton->setIcon(QIcon(":/appearance"));
	appearanceButton->setText(tr("Appearance"));
	appearanceButton->setTextAlignment(Qt::AlignHCenter);
	appearanceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	appearanceButton->setSizeHint(QSize(100, 50));
	
	QListWidgetItem* driverButton = new QListWidgetItem(contentsWidget);
	driverButton->setIcon(QIcon(":/audiocard"));
	driverButton->setText(tr("Audio Driver"));
	driverButton->setTextAlignment(Qt::AlignHCenter);
	driverButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	driverButton->setSizeHint(QSize(100, 50));
	
	QListWidgetItem* diskioButton = new QListWidgetItem(contentsWidget);
	diskioButton->setIcon(QIcon(":/diskdrive"));
	diskioButton->setText(tr("Disk I/O"));
	diskioButton->setTextAlignment(Qt::AlignHCenter);
	diskioButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	diskioButton->setSizeHint(QSize(100, 50));
	
	QListWidgetItem* keyboardButton = new QListWidgetItem(contentsWidget);
	keyboardButton->setIcon(QIcon(":/keyboard"));
	keyboardButton->setText(tr("Keyboard"));
	keyboardButton->setTextAlignment(Qt::AlignHCenter);
	keyboardButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	keyboardButton->setSizeHint(QSize(100, 50));
	
	
	connect(contentsWidget,
		SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
		this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
}

void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void SettingsDialog::save_config()
{
	for (int i=0; i<pagesWidget->count(); ++i) {
		qobject_cast<ConfigPage*>(pagesWidget->widget(i))->save_config();
	}
}

void SettingsDialog::restore_defaults_button_clicked()
{
	ConfigPage* page = qobject_cast<ConfigPage*>(pagesWidget->widget(contentsWidget->row(contentsWidget->currentItem())));
	if (page) {
		page->reset_default_config();
	} else {
		PERROR("SettingsDialog::restore_defaults_button_clicked: No ConfigPage found!!??\n");
	}
}
