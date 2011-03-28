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

#ifndef TCONTEXTHELPWIDGET_H
#define TCONTEXTHELPWIDGET_H

#include <QTextEdit>
#include <QWidget>

class ContextItem;
class QComboBox;

class TContextHelpWidget : public QWidget
{
        Q_OBJECT
public:
        TContextHelpWidget(QWidget* parent=0);
        ~TContextHelpWidget();

private:
        QString create_html_for_object(QObject* obj);

        QHash<QString, QString> m_help;
        QTextEdit*              m_textEdit;
        QComboBox*              m_comboBox;
        QString                 m_helpIntroduction;
	QString			m_currentClassName;

private slots:
        void context_changed();
        void jog_started();
        void combobox_activated(int);
	void function_keys_changed();
};

#endif // TCONTEXTHELPWIDGET_H
