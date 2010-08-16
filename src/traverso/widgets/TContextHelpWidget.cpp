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

#include "ContextPointer.h"
#include "ContextItem.h"
#include "InputEngine.h"
#include "TCommand.h"
#include "PCommand.h"
#include "TMainWindow.h"
#include "TMenuTranslator.h"
#include "Themer.h"

TContextHelpWidget::TContextHelpWidget(QWidget* parent)
        : QTextEdit(parent)
{
        connect(&cpointer(), SIGNAL(contextChanged()), this, SLOT(context_changed()));
        connect(&ie(), SIGNAL(jogStarted()), this, SLOT(jog_started()));
        connect(&ie(), SIGNAL(jogFinished()), this, SLOT(context_changed()));
}

void TContextHelpWidget::context_changed()
{
        if (parentWidget()->isHidden()) {
                return;
        }
        QList<ContextItem*> items = cpointer().get_active_context_items();
        if (items.size()) {
                setHtml(create_html_for_object(items.first()));
        }
}

void TContextHelpWidget::jog_started()
{
        TCommand* hold = ie().get_holding_command();
        if (hold) {
                setHtml(create_html_for_object(hold));
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
