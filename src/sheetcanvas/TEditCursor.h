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

#ifndef TEDITCURSOR_H
#define TEDITCURSOR_H

#include "ViewItem.h"

class SheetView;
class PositionIndicator;

class TEditCursor : public ViewItem
{
        Q_OBJECT
public:
        TEditCursor(SheetView* sv);
        ~TEditCursor();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        void set_text(const QString& text);
	void set_cursor_shape(const QString& shape);
        void set_pos(QPointF pos);

        // returns the real cursor position, i.e. the center of the cursor.
        QPointF get_scene_pos();
        void reset();

private:
	SheetView*		m_sv;
	PositionIndicator*	m_textItem;
	QPointF			m_pos;
	float			m_xOffset;
	float			m_yOffset;
	QPixmap			m_pixmap;
	QString			m_text;

	void create_cursor_pixmap(const QString& shape);
};

#endif // TEDITCURSOR_H
