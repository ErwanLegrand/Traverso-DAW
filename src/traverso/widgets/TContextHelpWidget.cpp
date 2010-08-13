#include "TContextHelpWidget.h"

#include <QMenu>

#include "ContextPointer.h"
#include "ContextItem.h"
#include "InputEngine.h"
#include "Command.h"
#include "TMainWindow.h"
#include "TMenuTranslator.h"

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
        Command* hold = ie().get_holding_command();
        if (hold) {
                setHtml(create_html_for_object(hold));
        }
}

QString TContextHelpWidget::create_html_for_object(QObject *obj)
{
        QString html;

        const QMetaObject* mo = obj->metaObject();

        QString name = TMenuTranslator::instance()->get_translation_for(QString(mo->className()).remove(("View")));

        if (m_help.contains(name)) {
                return m_help.value(name);
        }

        html = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
              "<style type=\"text/css\">\n"
              "H1 {text-align: left; font-size: 20px;}\n"
              "table {font-size: 12px; border: solid; border-width: 1px; width: 600px;}\n"
              ".object {background-color: #ccc; font-size: 16px; font-weight: bold;}\n"
              ".description {background-color: #ddd; width: 300px; margin: 8px; font-size: 12px; font-weight: bold;}\n"
              "</style>\n"
              "</head>\n<body>\n";

        html += "<table><tr class=\"object\">\n<td colspan=\"2\">" + name + "</td></tr>\n";
        html += "<tr><td class=\"description\">" +tr("Description") + "</td><td class=\"description\">" + tr("Key Sequence") + "</td></tr>\n";

        QStringList result;

        ContextItem* ci = qobject_cast<ContextItem*>(obj);
        QList<const QMetaObject*> metas;
        metas.append(mo);
        if (ci && ci->get_context()) {
                metas.append(ci->get_context()->metaObject());
        }

        foreach(const QMetaObject* mo, metas) {
                while (mo) {
                        QList<MenuData > list;

                        ie().create_menudata_for_metaobject(mo, list);

                        QList<QMenu* > menulist;
                        QMenu* menu = TMainWindow::instance()->create_context_menu(0, &list);
                        if (menu) {
                                menulist.append(menu);
                                foreach(QAction* action, menu->actions()) {
                                        if (action->menu()) {
                                                menulist.append(action->menu());
                                        }
                                }
                                for (int i=0; i<menulist.size(); ++i) {
                                        QMenu* somemenu = menulist.at(i);
                                        foreach(QAction* action, somemenu->actions()) {
                                                QStringList strings = action->data().toStringList();
                                                if (strings.size() >= 3) {
                                                        QString submenuname = "";
                                                        if (i > 0) {
                                                                submenuname = somemenu->menuAction()->text() + "&#160;&#160;&#160;&#160;";
                                                        }

                                                        QString keyfact = strings.at(2);

                                                        Command* com = dynamic_cast<Command*>(obj);
                                                        if (com) {
                                                                keyfact.replace("<", "");
                                                                keyfact.replace(">", "");
                                                        }

                                                        keyfact.replace(QString("MouseScrollVerticalUp"), QString("Scroll Wheel"));
                                                        keyfact.replace(QString("MouseScrollVerticalDown"), QString("Scroll Wheel"));
                                                        keyfact.replace(QString("MouseButtonRight"), QString("R. M. Button"));
                                                        keyfact.replace(QString("MouseButtonLeft"), QString("L. M. Button"));
                                                        keyfact.replace(QString("UARROW"), QString("&uarr; Arrow"));
                                                        keyfact.replace(QString("DARROW"), QString("&darr; Arrow"));
                                                        keyfact.replace(QString("LARROW"), QString("&larr; Arrow"));
                                                        keyfact.replace(QString("RARROW"), QString("&rarr; Arrow"));
                                                        keyfact.replace(QString("DELETE"), QString("Del"));
                                                        keyfact.replace(QString("<<"), QString("&laquo;"));
                                                        keyfact.replace(QString(">>"), QString("&raquo;"));
                                                        keyfact.replace(QString("<"), QString("&lsaquo;"));
                                                        keyfact.replace(QString(">"), QString("&rsaquo;"));




                                                        result += QString("<tr><td>") + submenuname + strings.at(1) + "</td><td>" + keyfact + "</td></tr>\n";
                                                }
                                        }
                                }
                                delete menu;
                        }
                        mo = mo->superClass();
                }
        }

        result.sort();
        result.removeDuplicates();
        html += result.join("");
        html += "</table>\n";
        html += "</body>\n</html>";

        m_help.insert(name, html);

        return html;
}
