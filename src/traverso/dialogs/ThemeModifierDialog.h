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
