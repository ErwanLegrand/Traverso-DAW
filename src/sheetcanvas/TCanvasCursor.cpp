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


#include "TCanvasCursor.h"

#include "ClipsViewPort.h"
#include "SheetView.h"
#include "ViewPort.h"
#include "PositionIndicator.h"

#include "Debugger.h"

TCanvasCursor::TCanvasCursor(SheetView* sv)
        : m_sv(sv)
{
	m_textItem = new PositionIndicator(this);
	m_textItem->hide();

	m_ignoreContext = true;
	m_xOffset = m_yOffset = 0.0f;

        setZValue(200);
}

TCanvasCursor::~TCanvasCursor( )
{
}

void TCanvasCursor::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
        Q_UNUSED(widget);
        Q_UNUSED(option);

	painter->drawPixmap(0, 0, m_pixmap);
}

void TCanvasCursor::create_cursor_pixmap(const QString &shape)
{
	int width = 9;
	int height = 16;
	int bottom = height + height / 2 - 2;
	m_pixmap = QPixmap(width + 2, bottom);
	m_boundingRect = m_pixmap.rect();
	m_pixmap.fill(Qt::transparent);
	QPainter painter(&m_pixmap);
	QPainterPath path;

	float halfWidth = float(width) / 2;
	QPointF endPoint(halfWidth + 1, 1);
	QPointF c1(1, height);
	QPointF c2(width + 1, height);
	path.moveTo(endPoint);
	path.quadTo(QPointF(-1, height * 0.75), c1);
	path.quadTo(QPointF(halfWidth + 1, bottom), c2);
	path.quadTo(QPointF(width + 3, height * 0.75), endPoint);
	QLinearGradient gradient;
	int graycolor = 160;
	int transparanty = 230;
	QColor gray(graycolor, graycolor, graycolor, transparanty);
	QColor black(0, 0, 0, transparanty);
	gradient.setColorAt(0.0, gray);
	gradient.setColorAt(1.0, black);
	gradient.setStart(0, 0);
	gradient.setFinalStop(0, -height);
	gradient.setSpread(QGradient::ReflectSpread);

	painter.setBrush(gradient);
	int white = 200;
	painter.setPen(QColor(white, white, white));
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawPath(path);
	QColor color (Qt::yellow);
	color.setAlpha(160);
	painter.setPen(color);
	QFont font;
	font.setPointSizeF(6);
	font.setKerning(false);
	painter.setFont(font);
	QRectF textRect(0, 8, width + 2, height - 8);
	painter.drawText(textRect, Qt::AlignHCenter, shape);
}

void TCanvasCursor::set_text( const QString & text )
{
        m_text = text;

        if (!m_text.isEmpty()) {
		m_textItem->set_value(m_text);
                m_textItem->show();
        } else {
                m_textItem->hide();
        }
}

void TCanvasCursor::set_cursor_shape( const QString & shape )
{
	create_cursor_pixmap(shape);
	m_xOffset = float(m_pixmap.width()) / 2;

	set_pos(m_pos);
}

void TCanvasCursor::reset()
{
        m_text = "";
        m_textItem->hide();
}

void TCanvasCursor::set_pos(QPointF p)
{
	m_pos = p;

	int diff = 0;
	int x = mapFromScene(pos()).x();
	int y = mapFromScene(pos()).y();
	int yoffset = m_pixmap.height() + 25;

	ViewPort* vp = static_cast<ViewPort*>(cpointer().get_viewport());
	if (vp)
	{
		if (y < 0) {
			yoffset = - y;
		} else if (y > vp->height() - m_pixmap.height()) {
			yoffset = vp->height() - y - m_pixmap.height();
		}

		diff = vp->width() - (x + m_pixmap.width() + 8);

	}

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

	p.setX(p.x() - m_xOffset);
	p.setY(p.y() - m_yOffset);

	setPos(p);
}

QPointF TCanvasCursor::get_scene_pos()
{
        QPointF shapeAdjust(boundingRect().width() / 2, boundingRect().height() / 2);
        return scenePos() + shapeAdjust;
}
