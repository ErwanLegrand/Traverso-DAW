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

        m_comboBox->addItem(tr("Active Context"));

        QMap<QString, QString> sorted;

        TMenuTranslator* translator = TMenuTranslator::instance();
        QHash<QString, QList<const QMetaObject*> > objects = translator->get_meta_objects();
        foreach(QList<const QMetaObject*> value, objects.values()) {
                if (value.size()) {
                        sorted.insert(translator->get_translation_for(value.first()->className()), value.first()->className());
                }
        }
        foreach(QString value, sorted.values()) {
                m_comboBox->addItem(sorted.key(value), value);
        }

        connect(&cpointer(), SIGNAL(contextChanged()), this, SLOT(context_changed()));
        connect(&ie(), SIGNAL(jogStarted()), this, SLOT(jog_started()));
        connect(&ie(), SIGNAL(jogFinished()), this, SLOT(context_changed()));
        connect(m_comboBox, SIGNAL(activated(int)), this, SLOT(combobox_activated(int)));
}

void TContextHelpWidget::context_changed()
{
        if (parentWidget()->isHidden()) {
                return;
        }

        if (m_comboBox->currentIndex() != 0) {
                return;
        }

        QList<ContextItem*> items = cpointer().get_active_context_items();

        if (items.size()) {
                m_textEdit->setHtml(create_html_for_object(items.first()));
        }
}

void TContextHelpWidget::jog_started()
{
        if (m_comboBox->currentIndex() != 0) {
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
