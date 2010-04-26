#include "ThemeModifierDialog.h"

#include "Themer.h"

#include <QColorDialog>
#include <QPushButton>

ThemeModifierDialog::ThemeModifierDialog(QWidget* parent)
        : QDialog(parent)
{
        setupUi(this);

        m_colorDialog = new QColorDialog();

        QList<QString> colors = themer()->get_colors();
        qSort(colors.begin(), colors.end());

        foreach(const QString& color, colors) {
                colorComboBox->addItem(color);
        }

        color_combo_box_index_changed(colorComboBox->currentText());

        connect(m_colorDialog, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(current_color_changed(const QColor&)));
        connect(colorPushButton, SIGNAL(clicked()), this, SLOT(color_push_botton_clicked()));
        connect(colorComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(color_combo_box_index_changed(const QString&)));
}

void ThemeModifierDialog::current_color_changed(const QColor& color)
{
        QPalette p(color);
        colorPushButton->setPalette(p);
        themer()->set_color(colorComboBox->currentText(), color);
}

void ThemeModifierDialog::color_push_botton_clicked()
{
        m_colorDialog->setCurrentColor(themer()->get_color(colorComboBox->currentText()));
        m_colorDialog->show();
}

void ThemeModifierDialog::color_combo_box_index_changed(const QString& text)
{
        QColor color = themer()->get_color(text);
        QPalette p(color);
        colorPushButton->setPalette(p);
}

void ThemeModifierDialog::accept()
{
        themer()->save();
        close();
}
