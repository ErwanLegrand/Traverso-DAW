#include "ShortcutEditorDialog.h"
#include "ui_ShortcutEditorDialog.h"

#include "TMenuTranslator.h"
#include "TShortcutManager.h"

#include <QTreeWidgetItem>

#include "Debugger.h"

ShortcutEditorDialog::ShortcutEditorDialog(QWidget *parent) :
		QDialog(parent),
		ui(new Ui::ShortcutEditorDialog)
{
	ui->setupUi(this);
	resize(700, 350);

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
	keys << "Left Button" << "Right Button" << "MouseScrollWheel";
	keys << "Enter" << "Home" << "End" << "Delete" << "PageUp" << "PageDown";
	keys << "/" << "\\" << "[" << "]" << "," << "." << ";" << "'";
	ui->keyComboBox1->addItems(keys);
	ui->keyComboBox2->addItems(keys);

	TMenuTranslator* translator = TMenuTranslator::instance();
	QMap<QString, QString> sorted;
	QHash<QString, QList<const QMetaObject*> > objects = translator->get_meta_objects();
	foreach(QList<const QMetaObject*> value, objects.values()) {
		if (value.size()) {
			sorted.insert(translator->get_translation_for(value.first()->className()), value.first()->className());
		}
	}
	foreach(QString value, sorted.values()) {
		ui->objectsComboBox->addItem(sorted.key(value), value);
	}

	ui->shortcutsTreeWidget->setColumnCount(2);
	ui->shortcutsTreeWidget->setHeaderLabels(QStringList() << tr("Description") << tr("Shortcut"));
	ui->shortcutsTreeWidget->header()->resizeSection(0, 280);

	connect(ui->objectsComboBox, SIGNAL(activated(int)), this, SLOT(objects_combo_box_activated(int)));
	connect(ui->shortcutsTreeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(shortcut_tree_widget_item_activated(QTreeWidgetItem*,int)));

	objects_combo_box_activated(0);
}

ShortcutEditorDialog::~ShortcutEditorDialog()
{
	delete ui;
}

void ShortcutEditorDialog::objects_combo_box_activated(int index)
{
	ui->shortcutsTreeWidget->clear();

	QString className = ui->objectsComboBox->itemData(index).toString();
	QList<const QMetaObject*> metas = TMenuTranslator::instance()->get_metaobjects_for_class(className);

	QList<TFunction* > functionsList;
	foreach(const QMetaObject* mo, metas) {
		while (mo) {
			functionsList << tShortCutManager().getFunctionsForMetaobject(mo);
			mo = mo->superClass();
		}
	}

	for (int j=0; j<functionsList.size(); ++j)
	{
		TFunction* function = functionsList.at(j);
		QTreeWidgetItem* item;
		item = new QTreeWidgetItem(QStringList() << function->getDescription() << function->getKeySequence());
		QVariant v = qVariantFromValue((void*) function);
		item->setData(0, Qt::UserRole, v);
		ui->shortcutsTreeWidget->addTopLevelItem(item);
	}
}

void ShortcutEditorDialog::shortcut_tree_widget_item_activated(QTreeWidgetItem *item, int)
{
	TFunction* function = (TFunction*) item->data(0, Qt::UserRole).value<void*>();

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

