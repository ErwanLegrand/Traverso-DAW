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
#include <Utils.h>
#include "Themer.h"
#include <QDir>

class MessageWidgetPrivate : public QWidget
{
	Q_OBJECT

public:
	MessageWidgetPrivate(QWidget* parent = 0);

public slots:
	void enqueue_message(InfoStruct );
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
	
	void create_icons();
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
	
	m_log = new QTextBrowser(this);
	m_log->setWindowFlags(Qt::Dialog);
	m_log->resize(500, 200);
	
	create_icons();
	
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
	int fontSize = 11;

	int begin = (  width()  / 2 ) - ( (stringLength * 8) / 2 ) ;

	if (begin < 40)
		begin = 40;

// 	painter.setPen(QColor(Qt::white));
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

void MessageWidgetPrivate::enqueue_message( InfoStruct infostruct)
{
	m_messageQueue.enqueue(infostruct);

	if (!m_messageTimer.isActive()) {
		m_infoStruct = m_messageQueue.dequeue();
		update();
	}

	if (m_messageQueue.size() >= 0) {
		m_messageTimer.start(3000);
	}

	// If Queue size is >= 1 start to dequeue faster.
	if (m_messageQueue.size() >= 1) {
		m_messageTimer.start(2000);
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
	QString iconpath;
	
	if (infostruct.type == INFO) {
		iconpath = QDir::home().path() + QString("/.traverso/.temp/iconinfo.png");
		color  = "bgcolor=#F4FFF4";
	} else if (infostruct.type == WARNING) {
		iconpath = QDir::home().path() + QString("/.traverso/.temp/iconwarning.png");
		color = "bgcolor=#FDFFD1";
	} else if (infostruct.type == CRITICAL) {
		iconpath = QDir::home().path() + QString("/.traverso/.temp/iconcritical.png");
		color = "bgcolor=#FFC8C8";
	}

	QString image = "<td width=20><img src=\"" + iconpath + "\" /></td>";
	m_log->append("<table width=100% " + color + " cellspacing=5><tr>" + 
			image + time + "<td>" + infostruct.message + "</td></tr></table>");
}

QSize MessageWidgetPrivate::sizeHint() const
{
	return QSize(300, 22);
}


void MessageWidgetPrivate::show_history()
{
	if (m_log->isHidden()) {
		m_log->show();
	} else {
		m_log->hide();
	}
}

void MessageWidgetPrivate::create_icons()
{
	QDir dir(QDir::home().path() + QString("/.traverso/.temp/"));
	if (! dir.exists()) {
		dir.mkdir(QDir::home().path() + QString("/.traverso/.temp/"));
	}

	QPixmap pix = style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(15, 15);
	QString iconpath = QDir::home().path() + QString("/.traverso/.temp/iconinfo.png");
	pix.save(iconpath);
	
	pix  = style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(15, 15);
	iconpath = QDir::home().path() + QString("/.traverso/.temp/iconwarning.png");
	pix.save(iconpath);
	
	pix = style()->standardIcon(QStyle::SP_MessageBoxCritical).pixmap(15, 15);
	iconpath = QDir::home().path() + QString("/.traverso/.temp/iconcritical.png");
	pix.save(iconpath);
}

//eof

