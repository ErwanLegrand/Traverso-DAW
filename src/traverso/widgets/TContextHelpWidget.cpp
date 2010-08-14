#include "TContextHelpWidget.h"

#include <QMenu>

#include "ContextPointer.h"
#include "ContextItem.h"
#include "InputEngine.h"
#include "TCommand.h"
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

        html += "<table><tr class=\"object\">\n<td colspan=\"2\" align=\"center\">" + name + "</td></tr>\n";
        html += "<tr><td width=110 class=\"description\">" +tr("Description") + "</td><td width=110 class=\"description\">" + tr("Key Sequence") + "</td></tr>\n";

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

                                keyfact.replace(QString("MouseScrollVerticalUp"), QString("Scroll Wheel"));
                                keyfact.replace(QString("MouseScrollVerticalDown"), QString("Scroll Wheel"));
                                keyfact.replace(QString("MouseButtonRight"), QString("Right. MB"));
                                keyfact.replace(QString("MouseButtonLeft"), QString("Left MB"));
                                keyfact.replace(QString("MouseButtonMiddle"), QString("Center MB"));
                                keyfact.replace(QString("UARROW"), QString("&uarr;"));
                                keyfact.replace(QString("DARROW"), QString("&darr;"));
                                keyfact.replace(QString("LARROW"), QString("&larr;"));
                                keyfact.replace(QString("RARROW"), QString("&rarr;"));
                                keyfact.replace(QString("DELETE"), QString("Del"));
                                keyfact.replace(QString("MINUS"), QString("&#45;"));
                                keyfact.replace(QString("PLUS"), QString("&#43;"));
                                keyfact.replace(QString("PAGEDOWN"), QString("Page Down"));
                                keyfact.replace(QString("PAGEUP"), QString("Page Up"));
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
