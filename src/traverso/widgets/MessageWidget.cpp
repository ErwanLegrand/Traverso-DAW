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
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QStyle>
#include <QDialog>
#include <Utils.h>
#include "Themer.h"
#include <QDir>

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

#include "MessageWidget.moc"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MessageWidget::MessageWidget( QWidget * parent )
	: QWidget(parent)
{
	QHBoxLayout* lay = new QHBoxLayout;
	lay->setMargin(1);
	
	m_button = new QPushButton;
	m_button->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(14, 14));
	m_button->setMaximumHeight(20);
	m_button->setFocusPolicy(Qt::NoFocus);
	
	MessageWidgetPrivate* message = new MessageWidgetPrivate(this);
		
	lay->addSpacing(6);
	lay->addWidget(message);
	lay->addWidget(m_button);
	lay->addSpacing(6);
	
	setLayout(lay);

	connect(m_button, SIGNAL(clicked( bool )), message, SLOT(show_history()));
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
	
	m_log = 0;
	
	connect(&info(), SIGNAL(message(InfoStruct)), this, SLOT(queue_message(InfoStruct)));
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


	switch(m_infoStruct.type) {
		case INFO 	:
			pm = style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(16, 16);
			painter.fillRect(0, 0, width(), height(), QColor("#F4FFF4"));
			break;
		case WARNING	:
			pm = style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(16, 16);
			painter.fillRect(0, 0, width(), height(), QColor("#FDFFD1"));
			break;
		case CRITICAL	:
			pm = style()->standardIcon(QStyle::SP_MessageBoxCritical).pixmap(16, 16);
			painter.fillRect(0, 0, width(), height(), QColor("#FFC8C8"));
			break;
		default		:
			pm = style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(16, 16);
	}



	int stringLength = m_infoStruct.message.length();

	int begin = (  width()  / 2 ) - ( (stringLength * 8) / 2 ) ;

	if (begin < 40)
		begin = 40;

	painter.setFont(themer()->get_font("MessageWidget:fontscale:log"));
	painter.setRenderHint(QPainter::TextAntialiasing);
	painter.drawText(begin, 16, m_infoStruct.message);
	painter.drawPixmap(begin - 35, 3, pm);
}

void MessageWidgetPrivate::resizeEvent(QResizeEvent* )
{
	PENTER3;
	update();
}

void MessageWidgetPrivate::queue_message( InfoStruct infostruct)
{
	m_messageQueue.enqueue(infostruct);

	if (!m_messageTimer.isActive()) {
		m_infoStruct = m_messageQueue.dequeue();
		update();
	}

	if (m_messageQueue.size() <= 1) {
		m_messageTimer.start(10000);
	}

	// If Queue size is >= 1 start to dequeue faster.
	if (m_messageQueue.size() >= 1) {
		m_messageTimer.start(1000);
	}
	
	if (m_messageQueue.size() > 3) {
		int skip = m_messageQueue.size() - 3;
		for (int i=0; i<skip; ++i) {
			dequeue_messagequeue();
		}
	}
	
	log(infostruct);

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
			m_messageTimer.start(3000);
		}

		m_infoStruct = m_messageQueue.dequeue();
	}

	update();
}

void MessageWidgetPrivate::log(InfoStruct infostruct)
{
	QString time = "<td width=65>" + QTime::currentTime().toString().append(" :") + " </td>";
	QString color;
	QString iconname;
	
	if (infostruct.type == INFO) {
		iconname = "iconinfo";
		color  = "bgcolor=#F4FFF4";
	} else if (infostruct.type == WARNING) {
		iconname = "iconwarning";
		color = "bgcolor=#FDFFD1";
	} else if (infostruct.type == CRITICAL) {
		iconname = "iconcritical";
		color = "bgcolor=#FFC8C8";
	}

	
	QString image = "<td width=20><img src=\"" + iconname +"\"/></td>";
	QString string = "<table width=100% " + color + " cellspacing=5><tr>" + 
			image + time + "<td>" + infostruct.message + "</td></tr></table>";
	if (m_log) {
		m_log->append(string);
	} else {
		m_stringLog.append(string);
	}
	
}

QSize MessageWidgetPrivate::sizeHint() const
{
	return QSize(300, 22);
}


void MessageWidgetPrivate::show_history()
{
	if (!m_log) {
		m_logDialog = new QDialog(this);
		m_log = new QTextBrowser(m_logDialog);
		
		QImage img = style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(15, 15).toImage();
		m_log->document()->addResource(QTextDocument::ImageResource, QUrl("iconinfo"), img);
		img = style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(15, 15).toImage();
		m_log->document()->addResource(QTextDocument::ImageResource, QUrl("iconwarning"), img);
		img = style()->standardIcon(QStyle::SP_MessageBoxCritical).pixmap(15, 15).toImage();
		m_log->document()->addResource(QTextDocument::ImageResource, QUrl("iconcritical"), img);
		
		QHBoxLayout* lay = new QHBoxLayout(m_logDialog);
		m_logDialog->setLayout(lay);
		lay->addWidget(m_log);
		m_logDialog->resize(500, 200);
		m_log->append(m_stringLog);
		m_stringLog.clear();
	}

	if (m_logDialog->isHidden()) {
		m_logDialog->show();
	} else {
		m_logDialog->hide();
	}
}

//eof

