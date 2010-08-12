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

#ifndef MOVECURVENODE_H
#define MOVECURVENODE_H

#include "MoveCommand.h"
#include "defines.h"

class CurveView;
class CurveNode;
class QPoint;

class MoveCurveNode : public MoveCommand
{
        Q_OBJECT

public:
        MoveCurveNode(CurveNode* node,
                CurveView* curveview,
                qint64 scalefactor,
                TimeRef rangeMin,
                TimeRef rangeMax,
                const QString& des);

        int prepare_actions();
        int do_action();
        int undo_action();
        int finish_hold();
        void cancel_action();
        int begin_hold();
        int jog();
        void set_cursor_shape(int useX, int useY);
        void set_vertical_only();

private :
        struct	Data {
                CurveView*	curveView;
                qint64		scalefactor;
                TimeRef		rangeMin;
                TimeRef		rangeMax;
                QPoint		mousepos;
                bool		verticalOnly;
        };

        Data* d;
        CurveNode* m_node;
        double	m_origWhen;
        double	m_origValue;
        double	m_newWhen;
        double 	m_newValue;

        int calculate_and_set_node_values();


public slots:
        void move_up(bool autorepeat);
        void move_down(bool autorepeat);
        void move_left(bool autorepeat);
        void move_right(bool autorepeat);
};

#endif // MOVECURVENODE_H
