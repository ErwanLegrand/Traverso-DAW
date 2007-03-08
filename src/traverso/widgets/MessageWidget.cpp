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

#include "MessageWidget.h" 
#include <QPainter>
#include <QPushButton>
#include <Utils.h>
#include "Themer.h"

class MessageWidgetPrivate : public QWidget
{
Q_OBJECT

public:
	MessageWidgetPrivate(QWidget* parent = 0);

public slots:
	void enqueue_message(InfoStruct );
	void dequeue_messagequeue();

protected:
	void resizeEvent( QResizeEvent* e);
	void paintEvent( QPaintEvent* e);
	QSize sizeHint() const;

private:
	QTimer			m_messageTimer;
	QQueue<InfoStruct >	m_messageQueue;
	InfoStruct 		m_infoStruct;
};

#include "MessageWidget.moc"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MessageWidget::MessageWidget( QWidget * parent )
	: QWidget(parent)
{
	QHBoxLayout* lay = new QHBoxLayout;
	lay->setMargin(0);
// 	lay->addStretch(5);
	
	m_button = new QPushButton;
	m_button->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
	m_button->setEnabled(false);
	m_button->setMaximumHeight(20);
	
	MessageWidgetPrivate* message = new MessageWidgetPrivate(this);
		
	lay->addWidget(message);
	lay->addWidget(m_button);
	
	setLayout(lay);
}

QSize MessageWidget::sizeHint() const
{
	return QSize(300, 20);
}



MessageWidgetPrivate::MessageWidgetPrivate( QWidget * parent )
	: QWidget(parent)
{
	QHBoxLayout* lay = new QHBoxLayout;
	lay->addStretch(5);
	setLayout(lay);
	
	connect(&info(), SIGNAL(message(InfoStruct)), this, SLOT(enqueue_message(InfoStruct)));
	connect(&m_messageTimer, SIGNAL(timeout()), this, SLOT(dequeue_messagequeue()));
}


void MessageWidgetPrivate::paintEvent(QPaintEvent* )
{
	QPainter painter(this);

	if (m_infoStruct.message.isEmpty() ) {
//                 painter.fillRect(0, 0, width(), height(), themer()->get_color("InfoWidget:background") );
		return;
	}

	QPixmap pm;

	painter.fillRect(0, 0, width(), height(), QColor("#F7F9D0"));

	switch(m_infoStruct.type) {
		case INFO		:
			pm = find_pixmap(":/info");
			break;
		case WARNING	:
			pm = find_pixmap(":/warning");
			break;
		case CRITICAL	:
			pm = find_pixmap(":/critical");
			break;
		default		:
			pm = find_pixmap(":/info");
	}



	int stringLength = m_infoStruct.message.length();
	int fontSize = 12;

	int begin = (  width()  / 2 ) - ( (stringLength * 8) / 2 ) ;

	if (begin < 40)
		begin = 40;

// 	painter.setPen(QColor(Qt::white));
	painter.setFont(QFont("Bitstream Vera Sans", fontSize));
	painter.setRenderHint(QPainter::TextAntialiasing);
	painter.drawText(begin, 18, m_infoStruct.message);
	painter.drawPixmap(begin - 35, 1, pm);
}

void MessageWidgetPrivate::resizeEvent(QResizeEvent* )
{
	PENTER3;
	update();
}

void MessageWidgetPrivate::enqueue_message( InfoStruct info_struct)
{
//         PWARN("Enqueueing message %s", info_struct.message.toAscii().data());
	m_messageQueue.enqueue(info_struct);

	if (!m_messageTimer.isActive()) {
		m_infoStruct = m_messageQueue.dequeue();
		update();
	}

	if (m_messageQueue.size() >= 0) {
		m_messageTimer.start(2800);
	}

	// If Queue size is >= 1 start to dequeue faster.
	if (m_messageQueue.size() >= 1) {
		m_messageTimer.start(1200);
	}

}

void MessageWidgetPrivate::dequeue_messagequeue( )
{
	if (m_messageQueue.isEmpty()) {
		m_infoStruct.message = "";
		m_messageTimer.stop();
	}

	if (!m_messageQueue.isEmpty()) {
		// If Queue size == 1 it means, it's the last message.Display it the "normal time duration"
		if (m_messageQueue.size() == 1) {
			m_messageTimer.start(2000);
		}

		m_infoStruct = m_messageQueue.dequeue();
//                 PWARN("Dequeueing message %s", m_infoStruct.message.toAscii().data());
	}

	update();
}

QSize MessageWidgetPrivate::sizeHint() const
{
	return QSize(300, 22);
}

//eof
