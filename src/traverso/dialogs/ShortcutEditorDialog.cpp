/*
Copyright (C) 2011 Remon Sijrier

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

#include "ShortcutEditorDialog.h"
#include "ui_ShortcutEditorDialog.h"

#include "TShortcutManager.h"

#include <QTreeWidgetItem>

#include "Debugger.h"

ShortcutEditorDialog::ShortcutEditorDialog(QWidget *parent) :
		QDialog(parent),
		ui(new Ui::ShortcutEditorDialog)
{
	ui->setupUi(this);
	resize(800, 455);

	QStringList keys;
	keys << "";

	for (int i=65; i<=90; ++i)
	{
		keys << QChar(i);
	}
	for (int i=1; i<=12; ++i)
	{
		keys << "F" + QString::number(i);
	}
	keys << "Left Button" << "Right Button" << "Scroll Up" << "Scroll Down";
	keys << "Enter" << "Home" << "End" << "Delete" << "Page Up" << "Page Down";
	keys << "+" << "-" << "/" << "\\" << "[" << "]" << "," << "." << ";" << "'";
	ui->keyComboBox1->addItems(keys);
	ui->keyComboBox2->addItems(keys);

	QMap<QString, QString> classNamesMap;
	QMap<QString, QString> commandClassNamesMap;

	foreach(QString className, tShortCutManager().getClassNames()) {
		if (tShortCutManager().isCommandClass(className))
		{
			commandClassNamesMap.insert(tShortCutManager().get_translation_for(className), className);
		}
		else
		{
			classNamesMap.insert(tShortCutManager().get_translation_for(className), className);
		}
	}

	foreach(QString className, classNamesMap.values()) {
		ui->objectsComboBox->addItem(classNamesMap.key(className), className);
	}

	foreach(QString className, commandClassNamesMap)
	{
		ui->objectsComboBox->addItem(commandClassNamesMap.key(className) + " " + tr("(Hold Function)"), className);
	}

	connect(ui->objectsComboBox, SIGNAL(activated(int)), this, SLOT(objects_combo_box_activated(int)));
	connect(ui->shortcutsTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(shortcut_tree_widget_item_activated()));
	connect(ui->showfunctionsCheckBox, SIGNAL(clicked()), this, SLOT(show_functions_checkbox_clicked()));
	connect(ui->keyComboBox1, SIGNAL(activated(int)), this, SLOT(key1_combo_box_activated(int)));

	// teasing the dialog to get into the 'no functions selected' state
	// and updating it accordingly.
	show_functions_checkbox_clicked();
}

ShortcutEditorDialog::~ShortcutEditorDialog()
{
	delete ui;
}

void ShortcutEditorDialog::objects_combo_box_activated(int index)
{
	ui->shortcutsTreeWidget->clear();

	QString className = ui->objectsComboBox->itemData(index).toString();

	if (tShortCutManager().isCommandClass(className))
	{
		ui->shortCutGroupBox->setTitle(tr("Modifier Key"));
		ui->shortcutsTreeWidget->setHeaderLabels(QStringList() << tr("Function") << tr("Modifier Key"));
	}
	else {
		ui->shortCutGroupBox->setTitle(tr("Shortcuts"));
		ui->shortcutsTreeWidget->setHeaderLabels(QStringList() << tr("Function") << tr("Shortcut"));
	}

	QList<TFunction* > functionsList = tShortCutManager().getFunctionsFor(className);
	QMap<QString, QList<TFunction*> > functionsMap;

	foreach(TFunction* function, functionsList)
	{
		QList<TFunction*> listForKey = functionsMap.value(function->submenu);
		listForKey.append(function);
		functionsMap.insert(function->submenu, listForKey);
	}

	QStringList subMenus = functionsMap.keys();

	foreach(QString submenu, subMenus)
	{
		QList<TFunction*> subMenuFunctionList = functionsMap.value(submenu);

		foreach(TFunction* function, subMenuFunctionList)
		{
			QTreeWidgetItem* item;
			item = new QTreeWidgetItem(QStringList() << function->getLongDescription() << function->getKeySequence());
			QVariant v = qVariantFromValue((void*) function);
			item->setData(0, Qt::UserRole, v);
			ui->shortcutsTreeWidget->addTopLevelItem(item);
		}
	}

	QTreeWidgetItem* item = ui->shortcutsTreeWidget->topLevelItem(0);
	if (item)
	{
		ui->shortcutsTreeWidget->setCurrentItem(item);
	}
}

void ShortcutEditorDialog::key1_combo_box_activated(int /*index*/)
{
	if (!ui->showfunctionsCheckBox->isChecked())
	{
		return;
	}

	ui->shortcutsTreeWidget->clear();

	QString keyString = ui->keyComboBox1->currentText();
	TShortcut* shortCut = tShortCutManager().getShortcut(keyString);

	if (!shortCut)
	{
		return;
	}

	QList<QStringList> itemList;
	QList<QStringList> holdCommandItemList;

	foreach(TFunction* function, shortCut->getFunctions())
	{
		QString translatedObjectName = tShortCutManager().get_translation_for(function->object);
		if (tShortCutManager().isCommandClass(function->object))
		{
			// FIXME: doesn't work!
			translatedObjectName = translatedObjectName + " " + tr("(Hold Function)");
			holdCommandItemList.append(QStringList() << translatedObjectName << function->getDescription() << function->getKeySequence());
		} else {
			itemList.append(QStringList() << translatedObjectName << function->getLongDescription() << function->getKeySequence());
		}
	}

	itemList.append(holdCommandItemList);

	foreach(QStringList stringlist, itemList)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(stringlist);
		ui->shortcutsTreeWidget->addTopLevelItem(item);
	}

}

void ShortcutEditorDialog::shortcut_tree_widget_item_activated()
{
	if (ui->showfunctionsCheckBox->isChecked())
	{
		return;
	}

	QList<QTreeWidgetItem*> items = ui->shortcutsTreeWidget->selectedItems();

	if (!items.size())
	{
		return;

	}
	QTreeWidgetItem *item = items.first();

	TFunction* function = (TFunction*) item->data(0, Qt::UserRole).value<void*>();

	bool isHoldFunction = false;
	int index = ui->objectsComboBox->currentIndex();
	if (index >=0)
	{
		QString objectClassName = ui->objectsComboBox->itemData(index).toString();
		isHoldFunction = tShortCutManager().classInherits(objectClassName, "TCommand");
	}

	ui->keyComboBox1->setCurrentIndex(0);
	ui->keyComboBox2->setCurrentIndex(0);
	ui->shiftCheckBox->setChecked(false);
	ui->ctrlCheckBox->setChecked(false);
	ui->altCheckBox->setChecked(false);
	ui->metaCheckBox->setChecked(false);

	QStringList keys = function->getKeys();
	if (keys.size() > 0)
	{
		QString keySequence = keys.at(0);
		tShortCutManager().makeShortcutKeyHumanReadable(keySequence);
		int index = ui->keyComboBox1->findText(keySequence, Qt::MatchFixedString);
		ui->keyComboBox1->setCurrentIndex(index);
	}
	if (keys.size() > 1)
	{
		QString keySequence = keys.at(1);
		tShortCutManager().makeShortcutKeyHumanReadable(keySequence);
		int index = ui->keyComboBox2->findText(keySequence, Qt::MatchFixedString);
		ui->keyComboBox2->setCurrentIndex(index);
	}

	TFunction* inheritedFunction = function->getInheritedFunction();
	if (inheritedFunction)
	{
		ui->baseFunctionGroupBox->setTitle(inheritedFunction->getDescription());
		ui->baseFunctionShortCutLable->setText(inheritedFunction->getKeySequence());
		ui->useBaseFunctionCheckBox->show();
		ui->baseFunctionGroupBox->show();

		ui->useBaseFunctionCheckBox->setChecked(true);
	}
	else
	{
		ui->baseFunctionGroupBox->hide();
		ui->useBaseFunctionCheckBox->hide();
	}

	if (isHoldFunction)
	{
		ui->modifiersGroupBox->setEnabled(false);
		if (function->usesAutoRepeat())
		{
			ui->autorepeatGroupBox->show();
			ui->startDelaySpinBox->setValue(function->getAutoRepeatStartDelay());
			ui->repeatIntervalSpinBox->setValue(function->getAutoRepeatInterval());
		}
		else
		{
			ui->autorepeatGroupBox->hide();
		}

	} else
	{
		ui->autorepeatGroupBox->hide();

		QList<int> modifierKeys = function->getModifierKeys();
		if (modifierKeys.contains(Qt::Key_Shift)) {
			ui->shiftCheckBox->setChecked(true);
		}
		if (modifierKeys.contains(Qt::Key_Control)) {
			ui->ctrlCheckBox->setChecked(true);
		}
		if (modifierKeys.contains(Qt::Key_Alt))	{
			ui->altCheckBox->setChecked(true);
		}
		if (modifierKeys.contains(Qt::Key_Meta)) {
			ui->metaCheckBox->setChecked(true);
		}
	}

	if (isHoldFunction)
	{
		ui->shortCutGroupBox->setEnabled(true);
		ui->modifiersGroupBox->setEnabled(false);
	}
	else if (inheritedFunction)
	{
		ui->shortCutGroupBox->setEnabled(false);
		ui->modifiersGroupBox->setEnabled(false);
	}
	else
	{
		ui->shortCutGroupBox->setEnabled(true);
		ui->modifiersGroupBox->setEnabled(true);
	}

}

void ShortcutEditorDialog::show_functions_checkbox_clicked()
{
	if (ui->showfunctionsCheckBox->isChecked())
	{
		ui->shortcutsTreeWidget->setColumnCount(3);
		ui->shortcutsTreeWidget->setHeaderLabels(QStringList() << tr("Item") << tr("Function") << tr("Shortcut"));
		ui->shortcutsTreeWidget->header()->resizeSection(0, 200);
		ui->shortcutsTreeWidget->header()->resizeSection(1, 200);
		ui->objectsComboBox->hide();
		ui->keyComboBox2->hide();
		ui->baseFunctionGroupBox->hide();
		ui->useBaseFunctionCheckBox->hide();
		key1_combo_box_activated(ui->keyComboBox1->currentIndex());
		ui->shortCutGroupBox->setTitle(tr("Shortcut"));
		ui->shortCutGroupBox->setEnabled(true);
		ui->modifiersGroupBox->setEnabled(false);
	}
	else
	{
		ui->objectsComboBox->show();
		ui->modifiersGroupBox->show();
		ui->shortCutGroupBox->setEnabled(true);
		ui->modifiersGroupBox->setEnabled(true);
		ui->keyComboBox2->show();
		ui->shortcutsTreeWidget->setColumnCount(2);
		ui->shortcutsTreeWidget->header()->resizeSection(0, 280);
		objects_combo_box_activated(ui->objectsComboBox->currentIndex());
	}
}

void ShortcutEditorDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

