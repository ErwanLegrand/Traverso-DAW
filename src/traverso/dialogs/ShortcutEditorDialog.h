#ifndef SHORTCUTEDITORDIALOG_H
#define SHORTCUTEDITORDIALOG_H

#include <QDialog>


namespace Ui {
	class ShortcutEditorDialog;
}

class ShortcutEditorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ShortcutEditorDialog(QWidget *parent = 0);
	~ShortcutEditorDialog();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::ShortcutEditorDialog *ui;

private slots:
	void objects_combo_box_activated(int index);
	void shortcut_tree_widget_item_activated();
};

#endif // SHORTCUTEDITORDIALOG_H
