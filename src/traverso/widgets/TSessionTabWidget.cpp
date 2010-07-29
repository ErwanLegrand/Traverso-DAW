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

#include "TSessionTabWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QToolBar>
#include <QApplication>

#include "ProjectManager.h"
#include "Project.h"
#include "TSession.h"
#include "TMainWindow.h"
#include "Utils.h"

#include "Debugger.h"

static const int HOR_BUTTON_HEIGHT = 28;
static const int VER_BUTTON_HEIGHT = 28;
static const int LABEL_WIDTH = 70;
static const int TAB_WIDTH = 110;


// QToolBar items don't like to have a parent widget. Moving the toolbar
// to another area crashes T when closing a project when adding a parent widget
// to the QPushButton constructor!!
TSessionTabWidget::TSessionTabWidget(QToolBar* toolBar, TSession *session)
        : QPushButton(0)
{
        m_toolBar = toolBar;
        m_session = session;

        m_spacer = new QWidget(this);
        m_spacer->setMinimumWidth(4);
        m_spacer->setMinimumHeight(4);

        m_nameLabel = new QLabel();
        m_nameLabel->setMinimumWidth(LABEL_WIDTH);
        session_property_changed();
        m_nameLabel->setEnabled(false);

        m_arrowButton = new QPushButton(this);
        m_arrowButton->setMinimumSize(TAB_WIDTH - LABEL_WIDTH, HOR_BUTTON_HEIGHT - 2);
        m_arrowButton->setStyleSheet("background-color: none; border: none;");
        update_arrow_button_shortcut_and_icon();

        // IMPORTANT: this menu needs MainWindow as parent, if the 'close view'
        // action is triggered, mouse events are dispatched correctly by Qt so
        // that the menu close vs view deletion vs context pointer set_view_port()
        // is processed in the correct order.
        m_arrowButtonMenu = new QMenu(TMainWindow::instance());
        m_arrowButtonMenu->installEventFilter(TMainWindow::instance());
        connect(m_arrowButton, SIGNAL(clicked()), this, SLOT(arrow_button_clicked()));

        m_childLayout = new QHBoxLayout;

        m_childLayout->addSpacing(6);
        m_childLayout->addWidget(m_nameLabel);
        m_childLayout->addStretch(1);
        m_childLayout->addWidget(m_arrowButton);
        m_childLayout->setMargin(0);

        QAction* action;

        if ( ! m_session->is_child_session()) {
                QPalette pal = palette();
                pal.setBrush(QPalette::Button, pal.button().color().darker(117));
                setPalette(pal);

                m_mainWidget = new QWidget(this);
                m_mainWidget->setLayout(m_childLayout);
                m_mainWidget->setStyleSheet("background-color: none;");

                toolbar_orientation_changed(toolBar->orientation());

                if (m_session->is_project_session()) {
                        action = m_arrowButtonMenu->addAction(tr("Edit..."));
                        connect(action, SIGNAL(triggered()), TMainWindow::instance(), SLOT(show_project_manager_dialog()));
                        action = m_arrowButtonMenu->addSeparator();
                }

                action = m_arrowButtonMenu->addAction(tr("New Track..."));
                action->setIcon(find_pixmap(":/new"));
                connect(action, SIGNAL(triggered()), this, SLOT(add_track_action_triggered()));

                if (m_session->is_project_session()) {
                        action = m_arrowButtonMenu->addAction(tr("New Sheet..."));
                        action->setIcon(find_pixmap(":/new"));
                        connect(action, SIGNAL(triggered()), TMainWindow::instance(), SLOT(show_newsheet_dialog()));
                }

                action = m_arrowButtonMenu->addAction(tr("New Sub View..."));
                action->setIcon(find_pixmap(":/new"));
                connect(action, SIGNAL(triggered()), this, SLOT(add_new_work_view_action_triggered()));

                if (m_session->is_project_session()) {

                        action = m_arrowButtonMenu->addSeparator();

                        action = m_arrowButtonMenu->addAction(tr("Close Project"));
                        action->setIcon(QIcon(":/exit"));
                        connect(action, SIGNAL(triggered()), this, SLOT(close_current_project()));
                }
        }

        foreach(TSession* session, m_session->get_child_sessions()) {
                child_session_added(session);
        }

        // called twice, else the m_spacer doesn't show up (for whatever reason)
        toolbar_orientation_changed(toolBar->orientation());

        if (m_session->is_child_session()) {
                QPalette pal = TMainWindow::instance()->palette();
                pal.setBrush(QPalette::Button, pal.button());
                setPalette(pal);

                setLayout(m_childLayout);

                action = m_arrowButtonMenu->addAction(tr("Edit..."));
                connect(action, SIGNAL(triggered()), this, SLOT(add_track_action_triggered()));

                m_arrowButtonMenu->addSeparator();
                action = m_arrowButtonMenu->addAction(QIcon(":/exit"), tr("Close View"));
                action->setIcon(QIcon(":/exit"));
                connect(action, SIGNAL(triggered()), this, SLOT(close_action_triggered()));
        }

        m_nameLabel->setStyleSheet("color: black; border: none; background-color: none;");

        calculate_size();

        connect(session, SIGNAL(transportStarted()), this, SLOT(session_transport_started()));
        connect(session, SIGNAL(transportStopped()), this, SLOT(session_transport_stopped()));
        connect(session, SIGNAL(sessionAdded(TSession*)), this, SLOT(child_session_added(TSession*)));
        connect(session, SIGNAL(sessionRemoved(TSession*)), this, SLOT(child_session_removed(TSession*)));
        connect(session, SIGNAL(propertyChanged()), this, SLOT(session_property_changed()));
        connect(m_toolBar, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(toolbar_orientation_changed(Qt::Orientation)));
        connect(this, SIGNAL(clicked()), this, SLOT(button_clicked()));
        connect(pm().get_project(), SIGNAL(currentSessionChanged(TSession*)), this, SLOT(project_current_session_changed(TSession*)));
        connect(pm().get_project(), SIGNAL(sessionIsAlreadyCurrent(TSession*)), this, SLOT(project_session_is_current(TSession*)));

        if (pm().get_project()->get_current_session() == m_session) {
                project_current_session_changed(m_session);
        }
}


void TSessionTabWidget::toolbar_orientation_changed(Qt::Orientation orientation)
{
        if ( ! m_session->is_child_session()) {
                foreach(TSessionTabWidget* tabWidget, m_childTabWidgets) {
                        layout()->removeWidget(tabWidget);
                }

                if (layout()) {
                        layout()->removeWidget(m_mainWidget);
                        layout()->removeWidget(m_spacer);
                }

                delete layout();

                if (orientation == Qt::Vertical) {
                        QVBoxLayout* vLayout = new QVBoxLayout();
                        vLayout->setMargin(0);
                        setLayout(vLayout);
                } else {
                        QHBoxLayout* hLayout = new QHBoxLayout();
                        hLayout->setMargin(0);
                        setLayout(hLayout);
                }

                layout()->addWidget(m_mainWidget);

                foreach(TSessionTabWidget* tabWidget, m_childTabWidgets) {
                        layout()->addWidget(tabWidget);
                }

                if (m_childTabWidgets.size()) {
                        layout()->addWidget(m_spacer);
                }

                calculate_size();
        }
}

void TSessionTabWidget::child_session_added(TSession *session)
{
        TSessionTabWidget* tabWidget = new TSessionTabWidget(m_toolBar, session);
        m_childTabWidgets.append(tabWidget);
        layout()->removeWidget(m_spacer);
        layout()->addWidget(tabWidget);
        layout()->addWidget(m_spacer);

        calculate_size();
}

void TSessionTabWidget::child_session_removed(TSession *session)
{
        foreach(TSessionTabWidget* tabWidget, m_childTabWidgets) {
                if (tabWidget->get_session() == session) {
                        layout()->removeWidget(tabWidget);
                        m_childTabWidgets.removeAll(tabWidget);
                        delete tabWidget;
                        break;
                }
        }
        if (!m_childTabWidgets.size()) {
                layout()->removeWidget(m_spacer);
        }

        calculate_size();
}

void TSessionTabWidget::calculate_size()
{
        if (!m_session->is_child_session()) {
                if (m_toolBar->orientation() == Qt::Vertical) {
                        setMinimumSize(TAB_WIDTH, VER_BUTTON_HEIGHT + 4 + m_session->get_child_sessions().count() * VER_BUTTON_HEIGHT);
                        setMaximumSize(TAB_WIDTH, VER_BUTTON_HEIGHT + 4 + m_session->get_child_sessions().count() * VER_BUTTON_HEIGHT);
                } else {
                        setMinimumSize(TAB_WIDTH + 4 + m_session->get_child_sessions().count() * (TAB_WIDTH + 4), HOR_BUTTON_HEIGHT);
                        setMaximumSize(TAB_WIDTH + 4 + m_session->get_child_sessions().count() * (TAB_WIDTH + 4), HOR_BUTTON_HEIGHT);
                }
                foreach(TSessionTabWidget* tabWidget, m_childTabWidgets) {
                        if (m_toolBar->orientation() == Qt::Vertical) {
                                tabWidget->setMinimumSize(TAB_WIDTH - 4, VER_BUTTON_HEIGHT);
                                tabWidget->setStyleSheet("margin-left: 2px; margin-right: 2px;");
                        } else {
                                tabWidget->setMinimumSize(TAB_WIDTH, HOR_BUTTON_HEIGHT - 2);
                                tabWidget->setStyleSheet("margin-bottom: 1px; margin-top: 1px;");
                        }
                }

        }
}

void TSessionTabWidget::session_transport_started()
{
        TSession* session = pm().get_project()->get_current_session();
        if (session == m_session || !m_session->get_parent_session()) {
                QString stylesheet = "color: blue; border: none; margin-left: 0px; background-color: none;";
                m_nameLabel->setStyleSheet(stylesheet);
        }
}

void TSessionTabWidget::session_transport_stopped()
{
        QString stylesheet = "color: black; border: none; margin-left: 0px; background-color: none;";
        m_nameLabel->setStyleSheet(stylesheet);
}

void TSessionTabWidget::session_property_changed()
{
        m_nameLabel->setText(m_session->get_name());
}

void TSessionTabWidget::button_clicked()
{
        if (pm().get_project()->get_current_session() != m_session) {
                pm().get_project()->set_current_session(m_session->get_id());
        } else {
                arrow_button_clicked();
        }
}

void TSessionTabWidget::arrow_button_clicked()
{
        if (pm().get_project()->get_current_session() == m_session) {
                m_arrowButtonMenu->move(QCursor::pos());
                m_arrowButtonMenu->show();
        } else {
                shortcut_click();
        }
}

void TSessionTabWidget::shortcut_click()
{
        animateClick(120);
}

void TSessionTabWidget::leaveEvent( QEvent * )
{
        m_arrowButton->setStyleSheet("background-color: none; border: none;");
}

void TSessionTabWidget::enterEvent( QEvent * e)
{
        if (pm().get_project()->get_current_session() == m_session) {
                m_arrowButton->setStyleSheet("background-color: lightblue;");
        }
}

void TSessionTabWidget::close_action_triggered()
{
        pm().get_project()->remove_child_session();
}

void TSessionTabWidget::add_track_action_triggered()
{
        TMainWindow::instance()->show_newtrack_dialog();
}

void TSessionTabWidget::add_new_work_view_action_triggered()
{
        TMainWindow::instance()->show_add_child_session_dialog();
}

void TSessionTabWidget::project_current_session_changed(TSession *session)
{
        if (!session) {
                return;
        }

        update_arrow_button_shortcut_and_icon();

        if (session == m_session) {
                QPixmap pix(":/down");
                m_arrowButton->setIcon(QIcon(pix.scaled(12, 10, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
        } else {
                m_arrowButton->setIcon(QIcon());
        }
}

void TSessionTabWidget::project_session_is_current(TSession *session)
{
        if (session == m_session) {
                arrow_button_clicked();
        }
}

void TSessionTabWidget::update_arrow_button_shortcut_and_icon()
{
        int number = pm().get_project()->get_session_index(m_session->get_id());
        if (number < 10) {
                m_arrowButton->setText(QString("&%1").arg(number));
        } else {
                m_arrowButton->setText(QString("%1").arg(number));
        }
}

void TSessionTabWidget::close_current_project()
{
        // for some reason if we have focus, and the project closes
        // all it's views, Qt crashes somewhere in it's widget backingstore
        // so unset the focus first to the main window, seems to help :)
        TMainWindow::instance()->setFocus(Qt::MouseFocusReason);
        qApp->processEvents();
        pm().close_current_project();
}

