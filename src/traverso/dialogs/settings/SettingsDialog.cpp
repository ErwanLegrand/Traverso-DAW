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

#include <QListWidgetItem>
#include <QStackedWidget>
#include <QDialogButtonBox>


#include <Config.h>

#include "SettingsDialog.h"
#include "Pages.h"

#include <Debugger.h>

SettingsDialog::SettingsDialog(QWidget* parent)
	: QDialog(parent)
{
	m_saving = false;
	
	contentsWidget = new QListWidget;
	contentsWidget->setViewMode(QListView::IconMode);
	contentsWidget->setIconSize(QSize(32, 32));
	contentsWidget->setMovement(QListView::Static);
//	contentsWidget->setMaximumWidth(140);
	contentsWidget->setMinimumWidth(135);
	contentsWidget->setMinimumHeight(390);
	contentsWidget->setSpacing(12);
	
	pagesWidget = new QStackedWidget;
	pagesWidget->addWidget(new BehaviorConfigPage(this));
	pagesWidget->addWidget(new AppearenceConfigPage(this));
	pagesWidget->addWidget(new AudioDriverConfigPage(this));
	pagesWidget->addWidget(new RecordingConfigPage(this));
	pagesWidget->addWidget(new KeyboardConfigPage(this));
	pagesWidget->addWidget(new PerformanceConfigPage(this));
	
	createIcons();
	contentsWidget->setCurrentRow(0);
	
	QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
	
	QPushButton* cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
	QPushButton* okButton = buttonBox->addButton(QDialogButtonBox::Ok); 
	QPushButton* restoreDefaultsButton = buttonBox->addButton(QDialogButtonBox::RestoreDefaults); 
        restoreDefaultsButton->setText(tr("Restore Defaults"));
	
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(save_config()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(restore_defaults_button_clicked()));
	
	QHBoxLayout *buttonsLayout = new QHBoxLayout;
	buttonsLayout->addWidget(buttonBox);
	
	QHBoxLayout *horizontalLayout = new QHBoxLayout;
	horizontalLayout->addWidget(contentsWidget);
	horizontalLayout->addWidget(pagesWidget, 1);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(horizontalLayout);
	mainLayout->addLayout(buttonsLayout);
	
	setLayout(mainLayout);
	
	setWindowTitle(tr("Preferences - Traverso"));
	
	connect(&config(), SIGNAL(configChanged()), this, SLOT(external_change_to_settings()));
	
	resize(400, 300);
}

void SettingsDialog::show_page(const QString & page)
{
	if (page == "Sound System") {
		contentsWidget->setCurrentRow(2);
	}
}


void SettingsDialog::createIcons()
{
	QListWidgetItem* behaviorButton = new QListWidgetItem(contentsWidget);
	behaviorButton->setIcon(QIcon(":/sheetmanagement"));
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
	driverButton->setText(tr("Sound System"));
	driverButton->setTextAlignment(Qt::AlignHCenter);
	driverButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	driverButton->setSizeHint(QSize(100, 50));
	
	QListWidgetItem* diskioButton = new QListWidgetItem(contentsWidget);
	diskioButton->setIcon(QIcon(":/audiosettings"));
	diskioButton->setText(tr("Audio Options"));
	diskioButton->setTextAlignment(Qt::AlignHCenter);
	diskioButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	diskioButton->setSizeHint(QSize(100, 50));
	
	QListWidgetItem* keyboardButton = new QListWidgetItem(contentsWidget);
	keyboardButton->setIcon(QIcon(":/keyboard"));
	keyboardButton->setText(tr("Keyboard"));
	keyboardButton->setTextAlignment(Qt::AlignHCenter);
	keyboardButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	keyboardButton->setSizeHint(QSize(100, 50));
	
	QListWidgetItem* performanceButton = new QListWidgetItem(contentsWidget);
	performanceButton->setIcon(QIcon(":/performance"));
	performanceButton->setText(tr("Performance"));
	performanceButton->setTextAlignment(Qt::AlignHCenter);
	performanceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	performanceButton->setSizeHint(QSize(100, 50));
	
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
	m_saving = true;
	config().save();
	m_saving = false;
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

void SettingsDialog::external_change_to_settings()
{
	if (!m_saving) {
		for (int i=0; i<pagesWidget->count(); ++i) {
			qobject_cast<ConfigPage*>(pagesWidget->widget(i))->load_config();
		}
	}
}

