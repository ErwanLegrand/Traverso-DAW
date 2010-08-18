/*
Copyright (C) 2010 Remon Sijrier

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

#include "TContextHelpWidget.h"

#include <QMenu>
#include <QLayout>
#include <QComboBox>

#include "TConfig.h"
#include "ContextPointer.h"
#include "ContextItem.h"
#include "InputEngine.h"
#include "TCommand.h"
#include "PCommand.h"
#include "TMainWindow.h"
#include "TMenuTranslator.h"
#include "Themer.h"

TContextHelpWidget::TContextHelpWidget(QWidget* parent)
        : QWidget(parent)
{
        m_comboBox = new QComboBox(parent);
        m_textEdit = new QTextEdit(parent);

        QHBoxLayout* comboLayout = new QHBoxLayout;
        QVBoxLayout* mainLayout = new QVBoxLayout;
        comboLayout->addWidget(m_comboBox);

        mainLayout->addLayout(comboLayout);
        mainLayout->addWidget(m_textEdit);
        setLayout(mainLayout);

        m_comboBox->addItem(tr("Keybindings Explained"));
        m_comboBox->addItem(tr("Active Context"));

        TMenuTranslator* translator = TMenuTranslator::instance();

        m_helpIntroduction = ("<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\"></head><body>"
                "Traverso uses a powerful keybinding concept with which you can control the program using the mouse "
                "or the keyboard + mouse or just the keyboard!"
                "<p>"
                "To show the keybindings for the object under the cursor, use the right mouse button, or "
                "select <u>%1</u> in the dropdown menu to auto update this help, or select an entry in the dropdown menu"
                "<p>"
                "<b>Keybinding Legenda</b>"
                "<p>"
                "<table>"
                "<tr><td><b> < K > </b>    </td><td>Press and release the key, like a click</td></tr>"
                "<tr><td><b> << K >> </b>  </td><td>Press and release the key two times, like a double click</td></tr>"
                "<tr><td><b> [ K ]   </b>  </td><td>Press the key and keep it pressed. You can move the mouse or use the arrow buttons to change the position or value</td></tr>"
                "<tr><td><b> < KL >  </td> <td>Press and release the K and L keys at the same time, like < K > but with 2 keys together</td></tr>"
                "<tr><td><b>[ KL ]   </td> <td>Same as [ K ] but now with 2 keys together</td></tr>"
                "</table>"
                "<p>"
                "<b>Advice</b>"
                "<p>"
                "Place the left hand in the typing position, most of the used keys are there, so you can use the right hand to move the cursor with the mouse or the arrow keys"
                "<p>"
                "<b>Examples</b>"
                "<p>"
                "<table>"
                "<tr><td><b> &lsaquo; I &rsaquo; </b></td><td>Hover mouse over an %2, type I (from Import) and select the file(s) to import</td></tr>"
                "<tr><td><b> [ G ] </b></td><td>Hover mouse over the %3, press G (from Gain) and keep it pressed. Now move mouse up/down to change gain, or use scroll wheel or up/down arrow key or just type in the gain value. <br />Works on all objects with a Gain value</td></tr>"
                "<tr><td><b> [ D ] </b></td><td>Hover mouse over an %3, press D (from Drag, left mouse button works too) and move the mouse to move the %3<br />Works on all objects that can be moved</td></tr>"
                "<tr><td><b> [ Z ] </b></td><td>Press Z and move mouse left/right: Zoom In/Out</td></tr>"
                "</table>"
                "</body></html>");

        m_helpIntroduction = m_helpIntroduction.arg(tr("Active Context")).arg(translator->get_translation_for("AudioTrack")).arg(translator->get_translation_for("AudioClip"));
        m_helpIntroduction.replace(QString("<< K"), QString("&laquo; K"));
        m_helpIntroduction.replace(QString("K >>"), QString("K &raquo;"));
        m_helpIntroduction.replace(QString("< K"), QString("&lsaquo; K"));
        m_helpIntroduction.replace(QString("K >"), QString("K &rsaquo;"));
        m_helpIntroduction.replace(QString("KL >"), QString("KL &rsaquo;"));

        combobox_activated(0);

        QMap<QString, QString> sorted;
        QHash<QString, QList<const QMetaObject*> > objects = translator->get_meta_objects();
        foreach(QList<const QMetaObject*> value, objects.values()) {
                if (value.size()) {
                        sorted.insert(translator->get_translation_for(value.first()->className()), value.first()->className());
                }
        }
        foreach(QString value, sorted.values()) {
                m_comboBox->addItem(sorted.key(value), value);
        }

        int index = config().get_property("Help", "DropDownIndex", 0).toInt();
        m_comboBox->setCurrentIndex(index);

        connect(&cpointer(), SIGNAL(contextChanged()), this, SLOT(context_changed()));
        connect(&ie(), SIGNAL(jogStarted()), this, SLOT(jog_started()));
        connect(&ie(), SIGNAL(jogFinished()), this, SLOT(context_changed()));
        connect(m_comboBox, SIGNAL(activated(int)), this, SLOT(combobox_activated(int)));
}

TContextHelpWidget::~TContextHelpWidget()
{
        config().set_property("Help", "DropDownIndex", m_comboBox->currentIndex());
}

void TContextHelpWidget::context_changed()
{
        if (parentWidget()->isHidden()) {
                return;
        }

        if (m_comboBox->currentIndex() != 1) {
                return;
        }

        QList<ContextItem*> items = cpointer().get_active_context_items();

        if (items.size()) {
                m_textEdit->setHtml(create_html_for_object(items.first()));
        }
}

void TContextHelpWidget::jog_started()
{
        if (m_comboBox->currentIndex() != 1) {
                return;
        }

        TCommand* hold = ie().get_holding_command();
        if (hold) {
                m_textEdit->setHtml(create_html_for_object(hold));
        }
}

QString TContextHelpWidget::create_html_for_object(QObject *obj)
{
        TMenuTranslator* translator = TMenuTranslator::instance();
        const QMetaObject* mo = obj->metaObject();

        if (m_help.contains(mo->className())) {
                return m_help.value(mo->className());
        }

        ContextItem* ci = qobject_cast<ContextItem*>(obj);
        QList<const QMetaObject*> metas;
        metas.append(mo);
        if (ci && ci->get_context()) {
                metas.append(ci->get_context()->metaObject());
        }

        QString html = translator->create_html_for_metas(metas, obj);

        m_help.insert(mo->className(), html);

        return html;
}

void TContextHelpWidget::combobox_activated(int index)
{
        if (index == 0) {
                m_textEdit->setHtml(m_helpIntroduction);
                return;
        }

        QString className = m_comboBox->itemData(index).toString();

        if (m_help.contains(className)) {
                m_textEdit->setHtml(m_help.value(className));
                return;
        }

        QList<const QMetaObject*> metas = TMenuTranslator::instance()->get_metaobjects_for_class(className);

        QString html = TMenuTranslator::instance()->create_html_for_metas(metas);

        m_help.insert(className, html);

        m_textEdit->setHtml(html);
}
