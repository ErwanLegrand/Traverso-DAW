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


#include "TEditCursor.h"

#include "ClipsViewPort.h"
#include "SheetView.h"
#include "ViewPort.h"

#include "Debugger.h"

TEditCursor::TEditCursor(SheetView* sv)
        : m_sv(sv)
{
        m_textItem = new QGraphicsTextItem(this);
        m_textItem->setFont(themer()->get_font("ViewPort:fontscale:infocursor"));

	m_ignoreContext = true;

        setZValue(200);
}

TEditCursor::~TEditCursor( )
{
}

void TEditCursor::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
        Q_UNUSED(widget);
        Q_UNUSED(option);

        painter->drawPixmap(0, 0, m_pixmap);
}


void TEditCursor::set_text( const QString & text )
{
        m_text = text;

        if (!m_text.isEmpty()) {
                QString html = "<html><body bgcolor=ghostwhite>" + m_text + "</body></html>";
                m_textItem->setHtml(html);
                m_textItem->show();
        } else {
                m_textItem->hide();
        }
}

void TEditCursor::set_cursor_shape( const QString & shape )
{
        QPointF origPos = scenePos();
        origPos.setX(origPos.x() + (qreal(m_pixmap.width()) / 2));
        origPos.setY(origPos.y() + (qreal(m_pixmap.height()) / 2));
	m_pixmap = find_pixmap(shape);
	set_pos(origPos);
}

QRectF TEditCursor::boundingRect( ) const
{
        return QRectF(0, 0, m_pixmap.width(), m_pixmap.height());
}

void TEditCursor::reset()
{
        m_text = "";
        m_textItem->hide();
}

void TEditCursor::set_pos(QPointF p)
{
        ViewPort* vp = m_sv->get_clips_viewport();
        int x = vp->mapFromScene(pos()).x();
        int y = vp->mapFromScene(pos()).y();
	int yoffset = m_pixmap.height() + 25;

        if (y < 0) {
                yoffset = - y;
        } else if (y > vp->height() - m_pixmap.height()) {
                yoffset = vp->height() - y - m_pixmap.height();
        }

        int diff = vp->width() - (x + m_pixmap.width() + 8);

	if (m_textItem->isVisible())
	{
		if (diff < m_textItem->boundingRect().width()) {
			m_textItem->setPos(diff - m_pixmap.width(), yoffset);
		} else if (x < -m_pixmap.width()) {
			m_textItem->setPos(8 - x, yoffset);
		} else {
			m_textItem->setPos(m_pixmap.width() + 8, yoffset);
		}
	}

        p.setX(p.x() - (qreal(m_pixmap.width()) / 2));
        p.setY(p.y() - (qreal(m_pixmap.height()) / 2));

        setPos(p);
}

QPointF TEditCursor::get_scene_pos()
{
        QPointF shapeAdjust(boundingRect().width() / 2, boundingRect().height() / 2);
        return scenePos() + shapeAdjust;
}
