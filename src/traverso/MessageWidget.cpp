/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: MessageWidget.cpp,v 1.5 2007/02/05 17:12:02 r_sijrier Exp $
*/

#include "MessageWidget.h"
#include <QPainter>
#include <Utils.h>
#include "libtraversocore.h"
#include "Themer.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MessageWidget::MessageWidget( QWidget * parent )
                : QWidget(parent)
{
        setMinimumHeight(22);

        /*	QPalette palette;
        	palette.setColor(QPalette::Background, QColor(110,120,100));
        	setPalette(palette);
        	setAutoFillBackground(true);*/

        connect(&info(), SIGNAL(message(InfoStruct)), this, SLOT(enqueue_message(InfoStruct)));
        connect(&messageTimer, SIGNAL(timeout()), this, SLOT(dequeue_messagequeue()));
}

MessageWidget::~ MessageWidget( )
{}


void MessageWidget::paintEvent(QPaintEvent* )
{
        QPainter painter(this);

        if (infoStruct.message.isEmpty() ) {
                painter.fillRect(0, 0, width(), height(), themer()->get_color("INFO_WIDGET_BACKGROUND") );
                return;
        }

//         PWARN("Printing message");
        QPixmap pm;

        painter.fillRect(0, 0, width(), height(), QColor(80, 90, 70));

        switch(infoStruct.type) {
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

        //         pm = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxInformation);

        int stringLength = infoStruct.message.length();
        int fontSize = 12;

        int begin = (  width()  / 2 ) - ( (stringLength * 8) / 2 ) ;

        if (begin < 40)
                begin = 40;

        painter.setPen(QColor(Qt::white));
        painter.setFont(QFont("Bitstream Vera Sans", fontSize));
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.drawText(begin, 16, infoStruct.message);
        painter.drawPixmap(begin - 35, 0, pm);
}

void MessageWidget::resizeEvent(QResizeEvent* )
{
        PENTER3;
        update();
}

void MessageWidget::enqueue_message( InfoStruct info_struct)
{
//         PWARN("Enqueueing message %s", info_struct.message.toAscii().data());
        messageQueue.enqueue(info_struct);

        if (!messageTimer.isActive()) {
                infoStruct = messageQueue.dequeue();
                update();
        }

        if (messageQueue.size() >= 0) {
                messageTimer.start(2800);
        }

        // If Queue size is >= 1 start to dequeue faster.
        if (messageQueue.size() >= 1) {
                messageTimer.start(1200);
        }

}

void MessageWidget::dequeue_messagequeue( )
{
        if (messageQueue.isEmpty()) {
                infoStruct.message = "";
                messageTimer.stop();
        }

        if (!messageQueue.isEmpty()) {
                // If Queue size == 1 it means, it's the last message.Display it the "normal time duration"
                if (messageQueue.size() == 1) {
                        messageTimer.start(2000);
                }

                infoStruct = messageQueue.dequeue();
//                 PWARN("Dequeueing message %s", infoStruct.message.toAscii().data());
        }

        update();
}

//eof
