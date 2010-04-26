#include "ThemeModifierDialog.h"

#include "Themer.h"

#include <QColorDialog>
#include <QListWidgetItem>

ThemeModifierDialog::ThemeModifierDialog(QWidget* parent)
        : QDialog(parent)
{
        setupUi(this);

        listWidget->setMinimumWidth(300);

        m_colorDialog = new QColorDialog();
        m_colorDialog->setWindowFlags(Qt::Widget);
        m_colorDialog->setOptions(QColorDialog::NoButtons | QColorDialog::ShowAlphaChannel);

        QList<QString> colors = themer()->get_colors();
        qSort(colors.begin(), colors.end());

        foreach(const QString& color, colors) {
                listWidget->addItem(color);
        }

        horizontalLayout->addWidget(m_colorDialog);

        connect(m_colorDialog, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(current_color_changed(const QColor&)));
        connect(listWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(list_widget_item_changed(QListWidgetItem*, QListWidgetItem*)));
}

void ThemeModifierDialog::current_color_changed(const QColor& color)
{
        themer()->set_color(m_currentColor, color);
}

void ThemeModifierDialog::list_widget_item_changed(QListWidgetItem* item, QListWidgetItem*)
{
        m_currentColor = item->text();
        QColor color = themer()->get_color(m_currentColor);
        m_colorDialog->setCurrentColor(color);
}

void ThemeModifierDialog::accept()
{
        themer()->save();
        close();
}
