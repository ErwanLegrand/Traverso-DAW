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
#include <QShortcut>
#include <QMenu>

#include "ProjectManager.h"
#include "Project.h"
#include "TSession.h"
#include "TMainWindow.h"

#include "Debugger.h"

static const int BUTTON_HEIGHT = 28;

TSessionTabWidget::TSessionTabWidget(QWidget *parent, TSession *session)
        : QPushButton(parent)
{
        m_session = session;
        if (m_session->is_project_session()) {
                m_number = 1;
        } else {
                m_number = 2;
        }

        m_nameLabel = new QLabel();
        m_nameLabel->setMaximumWidth(75);
        session_property_changed();
        m_nameLabel->setEnabled(false);

        m_arrowButton = new QPushButton(this);
        m_arrowButton->setMinimumSize(28, BUTTON_HEIGHT - 2);
        m_arrowButton->setStyleSheet("background-color: none; border: none;");
        show_shortcut();

        // IMPORTANT: this menu needs MainWindow as parent, if the 'close view'
        // action is triggered, mouse events are dispatched correctly by Qt so
        // that the menu close vs view deletion vs context pointer set_view_port()
        // is processed in the correct order.
        m_arrowButtonMenu = new QMenu(TMainWindow::instance());
        connect(m_arrowButton, SIGNAL(clicked()), this, SLOT(arrow_button_clicked()));

        QAction* action;
        action = m_arrowButtonMenu->addAction(tr("Add Track"));
        connect(action, SIGNAL(triggered()), this, SLOT(add_track_action_triggered()));

        m_vLayout = new QVBoxLayout();
        m_vLayout->setMargin(0);

        m_hLayout = new QHBoxLayout();
        m_hLayout->setMargin(0);
        m_hLayout->setSpacing(0);

        m_childLayout = new QHBoxLayout;

        m_childLayout->addSpacing(10);
        m_childLayout->addWidget(m_nameLabel);
        m_childLayout->addStretch(1);
        m_childLayout->addWidget(m_arrowButton);
        m_childLayout->setMargin(0);

        if (!m_session->get_parent_session()) {
//                setMinimumSize(90, BUTTON_HEIGHT + m_session->get_child_sessions().count() * BUTTON_HEIGHT);

                QWidget* widget = new QWidget(this);
                widget->setLayout(m_childLayout);
                widget->setStyleSheet("background-color: none;");

//                m_vLayout->addWidget(widget);
//                setLayout(m_vLayout);

                m_hLayout->addWidget(widget);
                m_hLayout->setAlignment(widget, Qt::AlignVCenter);
                setLayout(m_hLayout);

                action = m_arrowButtonMenu->addAction(tr("New Work View"));
                connect(action, SIGNAL(triggered()), this, SLOT(add_new_work_view_action_triggered()));
                QShortcut* shortCut = new QShortcut(QKeySequence::mnemonic(QString("&%1").arg(m_number)), this);
                connect(shortCut, SIGNAL(activatedAmbiguously()), this, SLOT(shortcut_click()));
        }

        foreach(TSession* session, m_session->get_child_sessions()) {
                child_session_added(session);
        }

        if (m_session->get_parent_session()) {
                setMinimumSize(90, BUTTON_HEIGHT - 1);
                setLayout(m_childLayout);

                if (!m_session->is_project_session()) {
                        action = m_arrowButtonMenu->addAction(QIcon(":/exit"), tr("Close View"));
                        connect(action, SIGNAL(triggered()), this, SLOT(close_action_triggered()));
                }
        }

        m_nameLabel->setStyleSheet("color: black; border: none; background-color: none;");
        m_nameLabel->setMinimumHeight(BUTTON_HEIGHT - 2);

        child_layout_changed();

        connect(session, SIGNAL(transportStarted()), this, SLOT(session_transport_started()));
        connect(session, SIGNAL(transportStopped()), this, SLOT(session_transport_stopped()));
        connect(session, SIGNAL(sessionAdded(TSession*)), this, SLOT(child_session_added(TSession*)));
        connect(session, SIGNAL(sessionRemoved(TSession*)), this, SLOT(child_session_removed(TSession*)));
        connect(session, SIGNAL(propertyChanged()), this, SLOT(session_property_changed()));
        connect(this, SIGNAL(clicked()), this, SLOT(button_clicked()));
        connect(pm().get_project(), SIGNAL(currentSessionChanged(TSession*)), this, SLOT(project_current_session_changed(TSession*)));

        if (pm().get_project()->get_current_session() == m_session) {
                project_current_session_changed(m_session);
        }
}

void TSessionTabWidget::child_session_added(TSession *session)
{
        TSessionTabWidget* tabWidget = new TSessionTabWidget(TMainWindow::instance(), session);
        m_childTabWidgets.append(tabWidget);
//        m_vLayout->addWidget(tabWidget);
        m_hLayout->addWidget(tabWidget);

        child_layout_changed();
}

void TSessionTabWidget::child_session_removed(TSession *session)
{
        foreach(TSessionTabWidget* tabWidget, m_childTabWidgets) {
                if (tabWidget->get_session() == session) {
                        m_hLayout->removeWidget(tabWidget);
                        m_childTabWidgets.removeAll(tabWidget);
                        delete tabWidget;
                        break;
                }
        }

        child_layout_changed();
}

void TSessionTabWidget::child_layout_changed()
{
        if (!m_session->get_parent_session()) {
//                setMinimumSize(90, BUTTON_HEIGHT + m_session->get_child_sessions().count() * BUTTON_HEIGHT);

                setMinimumSize(90 + 8 + m_session->get_child_sessions().count() * 92, BUTTON_HEIGHT);
                setMaximumSize(90 + 8 + m_session->get_child_sessions().count() * 92, BUTTON_HEIGHT);
        }

        for (int i=0; i<m_childTabWidgets.size(); i++) {
                m_childTabWidgets.at(i)->set_number(i + 3);
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
        pm().get_project()->set_current_session(m_session->get_id());
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

void TSessionTabWidget::enterEvent( QEvent * )
{
        if (pm().get_project()->get_current_session() == m_session) {
                m_arrowButton->setStyleSheet("background-color: pink;");
        }
}

void TSessionTabWidget::close_action_triggered()
{
        PENTER;
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
        if (session == m_session) {
                show_icon();
        } else {
                show_shortcut();
        }
}

void TSessionTabWidget::show_icon()
{
        m_arrowButton->setText("");
        m_arrowButton->setIcon(QIcon(":/down"));
}

void TSessionTabWidget::show_shortcut()
{
        m_arrowButton->setText(QString("&%1").arg(m_number));
        m_arrowButton->setIcon(QIcon());
}

void TSessionTabWidget::set_number(int number)
{
        m_number = number;
        project_current_session_changed(pm().get_project()->get_current_session());
}
