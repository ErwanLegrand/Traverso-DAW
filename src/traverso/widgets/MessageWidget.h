/*
    Copyright (C) 2005-2007 Remon Sijrier 
 
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

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QWidget>
#include <QQueue>
#include <QTimer>

#include "Information.h"

class QPushButton;
class QTextBrowser;
class QDialog;

class MessageWidgetPrivate : public QWidget
{
	Q_OBJECT

public:
	MessageWidgetPrivate(QWidget* parent = 0);

public slots:
	void queue_message(InfoStruct );
	void dequeue_messagequeue();
	void show_history();

protected:
	void resizeEvent( QResizeEvent* e);
	void paintEvent( QPaintEvent* e);
	QSize sizeHint() const;

private:
	QTimer			m_messageTimer;
	QQueue<InfoStruct >	m_messageQueue;
	InfoStruct 		m_infoStruct;
	QTextBrowser*		m_log;
	QDialog*		m_logDialog;
	QString			m_stringLog;

	void log(InfoStruct infostruct);
};

class MessageWidget : public QWidget
{

public:
        MessageWidget(QWidget* parent = 0);

protected:
	QSize sizeHint() const;
	
private:
	QPushButton* m_button;
};

#endif

//eof
