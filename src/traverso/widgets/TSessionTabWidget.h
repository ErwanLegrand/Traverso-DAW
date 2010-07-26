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

#ifndef TSESSIONTABWIDGET_H
#define TSESSIONTABWIDGET_H

#include <QPushButton>

class QLabel;
class QHBoxLayout;
class QVBoxLayout;
class QToolBar;
class TSession;

class TSessionTabWidget : public QPushButton
{
        Q_OBJECT
public:
        TSessionTabWidget(QWidget* parent, QToolBar* toolBar, TSession* session);

        TSession* get_session() const {return m_session;}

protected:
        void leaveEvent( QEvent* );
        void enterEvent( QEvent* );

private:
        TSession*       m_session;
        QWidget*        m_mainWidget;
        QWidget*        m_spacer;
        QLabel*         m_nameLabel;
        QPushButton*    m_arrowButton;
        QMenu*          m_arrowButtonMenu;
        QHBoxLayout*    m_childLayout;
        QToolBar*       m_toolBar;
        QList<TSessionTabWidget*> m_childTabWidgets;

private:
        void show_icon();
        void show_shortcut();
        void child_layout_changed();

private slots:
        void child_session_added(TSession* session);
        void child_session_removed(TSession* session);
        void session_transport_started();
        void session_transport_stopped();
        void session_property_changed();
        void button_clicked();
        void arrow_button_clicked();
        void shortcut_click();
        void close_action_triggered();
        void add_track_action_triggered();
        void add_new_work_view_action_triggered();
        void project_current_session_changed(TSession* session);
        void toolbar_orientation_changed (Qt::Orientation orientation);
};

#endif // TSESSIONTABWIDGET_H
