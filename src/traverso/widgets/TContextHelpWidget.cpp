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
        const QMetaObject* mo = obj->metaObject();

        QString name = TMenuTranslator::instance()->get_translation_for(QString(mo->className()).remove(("View")));

        if (m_help.contains(name)) {
                return m_help.value(name);
        }

        QColor bgcolor = themer()->get_color("ResourcesBin:alternaterowcolor");
        QString html = QString("<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
              "<style type=\"text/css\">\n"
              "table {font-size: 11px;}\n"
              ".object {background-color: %1; font-size: 12px; font-weight: bold;}\n"
              ".description {background-color: %2; font-size: 11px; font-weight: bold;}\n"
              "</style>\n"
              "</head>\n<body>\n").arg(bgcolor.darker(105).name()).arg(bgcolor.darker(103).name());

        if (obj->inherits("PCommand")) {
                PCommand* pc = static_cast<PCommand*>(obj);
                html += "<table><tr class=\"object\">\n<td width=220 align=\"center\">" + pc->text() + "</td></tr>\n";
        } else {
                html += "<table><tr class=\"object\">\n<td colspan=\"2\" align=\"center\">" + name + "</td></tr>\n";
                html += "<tr><td width=110 class=\"description\">" +tr("Description") + "</td><td width=110 class=\"description\">" + tr("Key Sequence") + "</td></tr>\n";
        }

        QStringList result;

        ContextItem* ci = qobject_cast<ContextItem*>(obj);
        QList<const QMetaObject*> metas;
        metas.append(mo);
        if (ci && ci->get_context()) {
                metas.append(ci->get_context()->metaObject());
        }

        int j=0;
        QString submenuhtml;
        QList<QMenu* > menulist;
        QList<MenuData > list;

        foreach(const QMetaObject* mo, metas) {
                while (mo) {

                        ie().create_menudata_for_metaobject(mo, list);
                        mo = mo->superClass();
                }
        }

        QMenu* menu = TMainWindow::instance()->create_context_menu(0, &list);
        if (menu) {
                menulist.append(menu);
                foreach(QAction* action, menu->actions()) {
                        if (action->menu()) {
                                menulist.append(action->menu());
                        }
                }
        }

        QStringList submenushtml;

        for (int i=0; i<menulist.size(); ++i) {
                QMenu* somemenu = menulist.at(i);
                submenuhtml = "";
                if (i>0) {
                        submenuhtml = "<tr class=\"object\">\n<td colspan=\"2\" align=\"center\">" + somemenu->menuAction()->text() + "</td></tr>\n";
                }
                foreach(QAction* action, somemenu->actions()) {
                        QStringList strings = action->data().toStringList();
                        if (strings.size() >= 3) {
                                QString keyfact = strings.at(2);

                                TCommand* com = dynamic_cast<TCommand*>(obj);
                                if (com) {
                                        keyfact.replace("<", "");
                                        keyfact.replace(">", "");
                                }

                                keyfact.replace(QString("Up Arrow"), QString("&uarr;"));
                                keyfact.replace(QString("Down Arrow"), QString("&darr;"));
                                keyfact.replace(QString("Left Arrow"), QString("&larr;"));
                                keyfact.replace(QString("Right Arrow"), QString("&rarr;"));
                                keyfact.replace(QString("-"), QString("&#45;"));
                                keyfact.replace(QString("+"), QString("&#43;"));
                                keyfact.replace(QString("<<"), QString("&laquo;"));
                                keyfact.replace(QString(">>"), QString("&raquo;"));
                                keyfact.replace(QString("<"), QString("&lsaquo;"));
                                keyfact.replace(QString(">"), QString("&rsaquo;"));
                                keyfact.replace(QString(" , "), QString("<br />"));


                                QString alternatingColor;
                                if ((j % 2) == 1) {
                                        alternatingColor = QString("bgcolor=\"%1\"").arg(themer()->get_color("ResourcesBin:alternaterowcolor").name());
                                } else {
                                        alternatingColor = QString("bgcolor=\"%1\"").arg(palette().color(QPalette::Base).name());
                                }
                                j += 1;

                                if (i>0) {
                                        submenuhtml += QString("<tr %1><td>").arg(alternatingColor) + strings.at(1) + "</td><td>" + keyfact + "</td></tr>\n";
                                } else {
                                        result += QString("<tr %1><td>").arg(alternatingColor) + strings.at(1) + "</td><td>" + keyfact + "</td></tr>\n";
                                }
                        }
                }
                if (!submenuhtml.isEmpty()) {
                        submenushtml.append(submenuhtml);
                }
        }

        foreach(QString html, submenushtml) {
                result += html;
        }

        foreach(QMenu* menu, menulist) {
                delete menu;
        }

//        result.sort();
        result.removeDuplicates();
        html += result.join("");
        html += "</table>\n";
        html += "</body>\n</html>";

        m_help.insert(name, html);

        return html;
}
