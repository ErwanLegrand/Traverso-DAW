/*
    Copyright (C) 2005 Remon Sijrier 
 
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
 
    $Id: BorderLayout.h,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/


#ifndef BORDERLAYOUT_H
#define BORDERLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QWidgetItem>

class BorderLayout : public QLayout
{
public:
        enum Position { West, North, South, East, Center };

        BorderLayout(QWidget *parent, int margin = 0, int spacing = -1);
        BorderLayout(int spacing = -1);
        ~BorderLayout();

        void addItem(QLayoutItem *item);
        void addWidget(QWidget *widget, Position position);
        Qt::Orientations expandingDirections() const;
        bool hasHeightForWidth() const;
        int count() const;
        QLayoutItem *itemAt(int index) const;
        QSize minimumSize() const;
        void setGeometry(const QRect &rect);
        QSize sizeHint() const;
        QLayoutItem *takeAt(int index);

        void add
                (QLayoutItem *item, Position position);

private:
        struct ItemWrapper
        {
                ItemWrapper(QLayoutItem *i, Position p)
                {
                        item = i;
                        position = p;
                }

                QLayoutItem *item;
                Position position;
        };

        enum SizeType
        {
                MinimumSize, SizeHint
        };
        QSize calculateSize(SizeType sizeType) const;

        QList<ItemWrapper *> list;
};

#endif
