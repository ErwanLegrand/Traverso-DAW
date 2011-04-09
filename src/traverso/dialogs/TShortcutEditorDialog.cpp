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

#include "TShortcutEditorDialog.h"
#include "ui_TShortcutEditorDialog.h"

#include "TShortcutManager.h"
#include "config.h"
#include <QTreeWidgetItem>

#include "Debugger.h"

TShortcutEditorDialog::TShortcutEditorDialog(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::TShortcutEditorDialog)
{
	ui->setupUi(this);

	ui->upPushButton->hide();
	ui->downPushButton->hide();
#ifdef USE_DEBUGGER
	ui->upPushButton->show();
	ui->downPushButton->show();
#endif

	resize(740, 400);

	QStringList keys;
	keys << "|";

	for (int i=65; i<=90; ++i)
	{
		QString string = QChar(i);
		string = string + "|" + string;
		keys << string;
	}
	for (int i=1; i<=12; ++i)
	{
		QString string("F%1");
		string = string.arg(i);
		string = QString(string + "|" + string);
		keys << string;
	}
	keys << "Left Arrow|LEFTARROW" << "Right Arrow|RIGHTARROW" << "Up Arrow|UPARROW" << "Down Arrow|DOWNARROW";
	keys << "Enter|ENTER" << "Home|HOME" << "End|END" << "Delete|DELETE";
	keys << "Page Up|PAGEUP" << "Page Down|PAGEDOWN";
	keys << "Left Button|MouseButtonLeft" << "Right Button|MouseButtonRight";
	keys << "Scroll Up|MOUSESCROLLVERTICALUP" << "Scroll Down|MOUSESCROLLVERTICALDOWN";
	keys << "+|PLUS" << "-|MINUS" << "/|/" << "\\|\\" << "[|[" << "]|]" << ",|," << ".|." << ";|;" << "'|'";

	foreach(QString string, keys)
	{
		QStringList list = string.split("|");
		ui->keyComboBox1->addItem(list.at(0), list.at(1));
		ui->keyComboBox2->addItem(list.at(0), list.at(1));
	}


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
	connect(&tShortCutManager(), SIGNAL(functionKeysChanged()), this, SLOT(function_keys_changed()));
	connect(ui->keyComboBox1, SIGNAL(activated(int)), this, SLOT(key1_combo_box_activated(int)));
	connect(ui->keyComboBox1, SIGNAL(activated(int)), this, SLOT(key_combo_box_activated(int)));
	connect(ui->keyComboBox2, SIGNAL(activated(int)), this, SLOT(key_combo_box_activated(int)));
	connect(ui->altCheckBox, SIGNAL(clicked()), this, SLOT(modifier_combo_box_toggled()));
	connect(ui->ctrlCheckBox, SIGNAL(clicked()), this, SLOT(modifier_combo_box_toggled()));
	connect(ui->shiftCheckBox, SIGNAL(clicked()), this, SLOT(modifier_combo_box_toggled()));
	connect(ui->metaCheckBox, SIGNAL(clicked()), this, SLOT(modifier_combo_box_toggled()));
	connect(ui->startDelaySpinBox, SIGNAL(editingFinished()), this, SLOT(modifier_combo_box_toggled()));
	connect(ui->repeatIntervalSpinBox, SIGNAL(editingFinished()), this, SLOT(modifier_combo_box_toggled()));
	connect(ui->configureInheritedShortcutPushButton, SIGNAL(clicked()), this, SLOT(configure_inherited_shortcut_pushbutton_clicked()));
	connect(ui->baseFunctionGroupBox, SIGNAL(clicked()), this, SLOT(base_function_checkbox_clicked()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(button_box_button_clicked(QAbstractButton*)));

	// teasing the dialog to get into the 'no functions selected' state
	// and updating it accordingly.
	show_functions_checkbox_clicked();
}

TShortcutEditorDialog::~TShortcutEditorDialog()
{
	delete ui;
}

void TShortcutEditorDialog::objects_combo_box_activated(int index)
{
	ui->shortcutsTreeWidget->clear();

	QString className = ui->objectsComboBox->itemData(index).toString();

	if (tShortCutManager().isCommandClass(className))
	{
		ui->shortCutGroupBox->setTitle(tr("Modifier &Key"));
		ui->shortcutsTreeWidget->setHeaderLabels(QStringList() << tr("Function") << tr("Modifier Key"));
	}
	else {
		ui->shortCutGroupBox->setTitle(tr("&Shortcuts"));
		ui->shortcutsTreeWidget->setHeaderLabels(QStringList() << tr("Function") << tr("Shortcut"));
	}

	QList<TFunction* > functionsList = tShortCutManager().getFunctionsFor(className);

	foreach(TFunction* function, functionsList)
	{
		QTreeWidgetItem* item;
		item = new QTreeWidgetItem(QStringList() << function->getLongDescription() << function->getKeySequence());
		QVariant v = qVariantFromValue((void*) function);
		item->setData(0, Qt::UserRole, v);
		ui->shortcutsTreeWidget->addTopLevelItem(item);
	}

	QTreeWidgetItem* item = ui->shortcutsTreeWidget->topLevelItem(0);
	if (item)
	{
		ui->shortcutsTreeWidget->setCurrentItem(item);
	}
}

void TShortcutEditorDialog::modifier_combo_box_toggled()
{
	key_combo_box_activated(0);
}

void TShortcutEditorDialog::key_combo_box_activated(int)
{
	if (ui->showfunctionsCheckBox->isChecked()) {
		return;
	}

	TFunction* function = getSelectedFunction();
	if (!function)
	{
		return;
	}

	QStringList modifiers;

	if (ui->altCheckBox->isChecked()) modifiers << "ALT";
	if (ui->ctrlCheckBox->isChecked()) modifiers << "CTRL";
	if (ui->shiftCheckBox->isChecked()) modifiers << "SHIFT";
	if (ui->metaCheckBox->isChecked()) modifiers << "META";

	QStringList keys;

	QString key1 = ui->keyComboBox1->itemData(ui->keyComboBox1->currentIndex()).toString();
	if (!key1.isEmpty())
	{
		keys << key1;
	}

	QString key2 = ui->keyComboBox2->itemData(ui->keyComboBox2->currentIndex()).toString();
	if (!key2.isEmpty())
	{
		keys << key2;
	}

	if (function->usesAutoRepeat())
	{
		function->setAutoRepeatInterval(ui->repeatIntervalSpinBox->value());
		function->setAutoRepeatStartDelay(ui->startDelaySpinBox->value());
	}

	tShortCutManager().modifyFunctionKeys(function, keys, modifiers);
}

void TShortcutEditorDialog::key1_combo_box_activated(int /*index*/)
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

	foreach(TFunction* function, shortCut->getFunctions())
	{
		QString translatedObjectName = tShortCutManager().get_translation_for(function->object);
		QStringList stringlist;
		stringlist << translatedObjectName << function->getLongDescription() << function->getKeySequence();
		QTreeWidgetItem* item = new QTreeWidgetItem(stringlist);
		QVariant v = qVariantFromValue((void*) function);
		item->setData(0, Qt::UserRole, v);

		ui->shortcutsTreeWidget->addTopLevelItem(item);
	}
}

TFunction* TShortcutEditorDialog::getSelectedFunction()
{
	QList<QTreeWidgetItem*> items = ui->shortcutsTreeWidget->selectedItems();

	if (!items.size())
	{
		return 0;

	}
	QTreeWidgetItem *item = items.first();

	return (TFunction*) item->data(0, Qt::UserRole).value<void*>();
}

void TShortcutEditorDialog::shortcut_tree_widget_item_activated()
{
	if (ui->showfunctionsCheckBox->isChecked())
	{
		TFunction* function = getSelectedFunction();
		if (function)
		{
			int index = ui->objectsComboBox->findData(tShortCutManager().getClassForObject(function->object));
			if (index >= 0)
			{
				ui->objectsComboBox->setCurrentIndex(index);
				ui->showfunctionsCheckBox->setChecked(false);
				show_functions_checkbox_clicked();
				QList<QTreeWidgetItem*> items = ui->shortcutsTreeWidget->findItems(function->getLongDescription(), Qt::MatchCaseSensitive);
				if (items.size())
				{
					ui->shortcutsTreeWidget->setCurrentItem(items.first());
				}
			}
		}

		return;
	}

	TFunction* function = getSelectedFunction();
	if (!function)
	{
		return;
	}

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

	QStringList keys = function->getKeys(false);
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
	bool usesInheritedBase = function->usesInheritedBase();
	if (inheritedFunction)
	{
		ui->baseFunctionGroupBox->setTitle(tr("Inherits:") + " " + inheritedFunction->getDescription());
		ui->baseFunctionShortCutLable->setText(inheritedFunction->getKeySequence());
		ui->baseFunctionGroupBox->show();

		if (usesInheritedBase)
		{
			ui->baseFunctionGroupBox->setChecked(true);
		}
		else
		{
			ui->baseFunctionGroupBox->setChecked(false);
		}
	}
	else
	{
		ui->baseFunctionGroupBox->hide();
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

		QList<int> modifierKeys = function->getModifierKeys(false);
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
	else if (usesInheritedBase)
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

void TShortcutEditorDialog::base_function_checkbox_clicked()
{
	TFunction* function = getSelectedFunction();
	if (!function)
	{
		return;
	}


	if (ui->baseFunctionGroupBox->isChecked())
	{
		tShortCutManager().modifyFunctionInheritedBase(function, true);
	}
	else
	{
		tShortCutManager().modifyFunctionInheritedBase(function, false);
	}
}

void TShortcutEditorDialog::show_functions_checkbox_clicked()
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
		key1_combo_box_activated(ui->keyComboBox1->currentIndex());
		ui->shortCutGroupBox->setTitle(tr("&Shortcut"));
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

void TShortcutEditorDialog::function_keys_changed()
{
	QWidget* fWidget = focusWidget();

	QTreeWidgetItem* item = ui->shortcutsTreeWidget->currentItem();
	if (!item) {
		return;
	}

	TFunction* function = (TFunction*) item->data(0, Qt::UserRole).value<void*>();

	objects_combo_box_activated(ui->objectsComboBox->currentIndex());

	// item is now deleted!

	QList<QTreeWidgetItem*> items = ui->shortcutsTreeWidget->findItems(function->getLongDescription(), Qt::MatchCaseSensitive);
	if (items.size())
	{
		ui->shortcutsTreeWidget->setCurrentItem(items.first());
	}

	fWidget->setFocus();
}

void TShortcutEditorDialog::configure_inherited_shortcut_pushbutton_clicked()
{
	TFunction* function = getSelectedFunction();
	if (!function)
	{
		return;
	}
	function = function->getInheritedFunction();
	if (!function)
	{
		return;
	}

	int index = ui->objectsComboBox->findData(function->object);
	if (index >= 0)
	{
		ui->objectsComboBox->setCurrentIndex(index);
		objects_combo_box_activated(index);
	}
}

void TShortcutEditorDialog::on_restoreDefaultPushButton_clicked()
{
	TFunction* function = getSelectedFunction();
	if (!function)
	{
		return;
	}

	tShortCutManager().restoreDefaultFor(function);
}

void TShortcutEditorDialog::button_box_button_clicked(QAbstractButton* button)
{
	if (button == ui->buttonBox->button(QDialogButtonBox::RestoreDefaults))
	{
		tShortCutManager().restoreDefaults();

	}
}

void TShortcutEditorDialog::on_downPushButton_clicked()
{
	moveItemUpDown(1);
}

void TShortcutEditorDialog::on_upPushButton_clicked()
{
	moveItemUpDown(-1);
}

void TShortcutEditorDialog::moveItemUpDown(int direction)
{
	QTreeWidgetItem* item = ui->shortcutsTreeWidget->currentItem();
	if (!item)
	{
		return;
	}

	int index = ui->shortcutsTreeWidget->indexOfTopLevelItem(item);
	if ((index + direction) < 0 || (index + direction) >= ui->shortcutsTreeWidget->topLevelItemCount())
	{
		return;
	}
	ui->shortcutsTreeWidget->takeTopLevelItem(index);
	ui->shortcutsTreeWidget->insertTopLevelItem(index + direction, item);
	ui->shortcutsTreeWidget->setCurrentItem(item);

	for (int i=0; i<ui->shortcutsTreeWidget->topLevelItemCount(); ++i)
	{
		item = ui->shortcutsTreeWidget->topLevelItem(i);
		TFunction* function = (TFunction*) item->data(0, Qt::UserRole).value<void*>();
		function->sortorder = i;
	}

	tShortCutManager().exportFunctions();
}

void TShortcutEditorDialog::changeEvent(QEvent *e)
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

