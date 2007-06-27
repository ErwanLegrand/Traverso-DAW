/*
Copyright (C) 2006-2007 Remon Sijrier 

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

#ifndef PLUGIN_SELECTOR_DIALOG_H
#define PLUGIN_SELECTOR_DIALOG_H

#include <QDialog>

#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>


class Plugin;

class PluginSelectorDialog : public QDialog
{
	Q_OBJECT

public:
	
	static PluginSelectorDialog* instance();
	
	Plugin* get_selected_plugin();
	void set_description(const QString& des);

private:
	PluginSelectorDialog(QWidget* parent = 0);
	~PluginSelectorDialog();
	
	static PluginSelectorDialog* m_instance;
	
	QVBoxLayout *vboxLayout;
	QLabel *objectToAddPluginTooLabel;
	QTreeWidget *pluginTreeWidget;
	QHBoxLayout *hboxLayout;
	QSpacerItem *spacerItem;
	QPushButton *okButton;
	QPushButton *cancelButton;
	
	Plugin* m_plugin;

	void setupUi(QDialog *PluginSelectorDialog)
	{
		if (PluginSelectorDialog->objectName().isEmpty())
			PluginSelectorDialog->setObjectName(QString::fromUtf8("PluginSelectorDialog"));
		QSize size(471, 433);
		size = size.expandedTo(PluginSelectorDialog->minimumSizeHint());
		PluginSelectorDialog->resize(size);
		vboxLayout = new QVBoxLayout(PluginSelectorDialog);
#ifndef Q_OS_MAC
		vboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
		vboxLayout->setMargin(9);
#endif
		vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
		objectToAddPluginTooLabel = new QLabel(PluginSelectorDialog);
		objectToAddPluginTooLabel->setObjectName(QString::fromUtf8("objectToAddPluginTooLabel"));

		vboxLayout->addWidget(objectToAddPluginTooLabel);

		pluginTreeWidget = new QTreeWidget(PluginSelectorDialog);
		pluginTreeWidget->setObjectName(QString::fromUtf8("pluginTreeWidget"));

		vboxLayout->addWidget(pluginTreeWidget);

		hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
		hboxLayout->setSpacing(6);
#endif
		hboxLayout->setMargin(0);
		hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
		spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

		hboxLayout->addItem(spacerItem);

		okButton = new QPushButton(PluginSelectorDialog);
		okButton->setObjectName(QString::fromUtf8("okButton"));

		hboxLayout->addWidget(okButton);

		cancelButton = new QPushButton(PluginSelectorDialog);
		cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

		hboxLayout->addWidget(cancelButton);


		vboxLayout->addLayout(hboxLayout);


		retranslateUi(PluginSelectorDialog);
		QObject::connect(cancelButton, SIGNAL(clicked()), PluginSelectorDialog, SLOT(reject()));
		QObject::connect(okButton, SIGNAL(clicked()), PluginSelectorDialog, SLOT(accept()));
		QObject::connect(pluginTreeWidget, SIGNAL(activated(QModelIndex)), okButton, SLOT(click()));

		QMetaObject::connectSlotsByName(PluginSelectorDialog);
	} // setupUi

	void retranslateUi(QDialog *PluginSelectorDialog)
	{
		PluginSelectorDialog->setWindowTitle(QApplication::translate("PluginSelectorDialog", "Plugin Selector", 0, QApplication::UnicodeUTF8));
		objectToAddPluginTooLabel->setText(QApplication::translate("PluginSelectorDialog", "Add Plugin too", 0, QApplication::UnicodeUTF8));
		pluginTreeWidget->headerItem()->setText(0, QApplication::translate("PluginSelectorDialog", "Plugin Name", 0, QApplication::UnicodeUTF8));
		pluginTreeWidget->headerItem()->setText(1, QApplication::translate("PluginSelectorDialog", "Type", 0, QApplication::UnicodeUTF8));
		pluginTreeWidget->headerItem()->setText(2, QApplication::translate("PluginSelectorDialog", "In/Out", 0, QApplication::UnicodeUTF8));
		okButton->setText(QApplication::translate("PluginSelectorDialog", "OK", 0, QApplication::UnicodeUTF8));
		cancelButton->setText(QApplication::translate("PluginSelectorDialog", "Cancel", 0, QApplication::UnicodeUTF8));
		Q_UNUSED(PluginSelectorDialog);
	} // retranslateUi
	
private slots:
	void on_okButton_clicked();
	void on_cancelButton_clicked();
	void plugin_double_clicked();
	
};

#endif

//eof


