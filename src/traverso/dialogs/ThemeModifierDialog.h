#ifndef ThemeModifierDialog_H
#define ThemeModifierDialog_H

#include "ui_ThemeModifierDialog.h"

#include <QColor>
#include <QDialog>

class QColorDialog;


class ThemeModifierDialog : public QDialog, protected Ui::ThemeModifierDialog
{
        Q_OBJECT

public:
            ThemeModifierDialog(QWidget* parent);

private:
            QColorDialog* m_colorDialog;

public slots:
        void accept();

private slots:
        void current_color_changed(const QColor &);
        void color_push_botton_clicked();
        void color_combo_box_index_changed(const QString& text);
};

#endif // ThemeModifierDialog_H
