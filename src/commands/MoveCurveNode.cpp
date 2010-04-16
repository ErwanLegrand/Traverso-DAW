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


#include "MoveCurveNode.h"

#include "CurveView.h"
#include "CurveNode.h"
#include "SheetView.h"
#include "Mixer.h"

MoveCurveNode::MoveCurveNode(CurveNode* node,
        CurveView* curveview,
        qint64 scalefactor,
        TimeRef rangeMin,
        TimeRef rangeMax,
        const QString& des)
        : MoveCommand(curveview->get_context(), des)
        , d(new Data)
{
        m_node = node;
        d->rangeMin = rangeMin;
        d->rangeMax = rangeMax;
        d->curveView = curveview;
        d->scalefactor = scalefactor;
        d->verticalOnly = false;
}

void MoveCurveNode::set_vertical_only()
{
        d->verticalOnly = true;
}

int MoveCurveNode::prepare_actions()
{
        return 1;
}

int MoveCurveNode::finish_hold()
{
        delete d;
        return 1;
}

void MoveCurveNode::cancel_action()
{
        delete d;
        undo_action();
}

int MoveCurveNode::begin_hold()
{
        m_origWhen = m_newWhen = m_node->get_when();
        m_origValue = m_newValue = m_node->get_value();

        d->mousepos = QPoint(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
        return 1;
}


int MoveCurveNode::do_action()
{
        m_node->set_when_and_value(m_newWhen, m_newValue);
        return 1;
}

int MoveCurveNode::undo_action()
{
        m_node->set_when_and_value(m_origWhen, m_origValue);
        return 1;
}

void MoveCurveNode::move_up(bool )
{
        m_newValue = m_newValue + ( 1 / d->curveView->boundingRect().height());
        calculate_and_set_node_values();
}

void MoveCurveNode::move_down(bool )
{
        m_newValue = m_newValue - ( 1 / d->curveView->boundingRect().height());
        calculate_and_set_node_values();
}

void MoveCurveNode::move_left(bool )
{
        m_newWhen = m_newWhen - (d->curveView->get_sheetview()->timeref_scalefactor * m_speed);
        calculate_and_set_node_values();
}

void MoveCurveNode::move_right(bool )
{
        m_newWhen = m_newWhen + (d->curveView->get_sheetview()->timeref_scalefactor * m_speed);
        calculate_and_set_node_values();
}

void MoveCurveNode::set_cursor_shape(int useX, int useY)
{
        cpointer().get_viewport()->set_holdcursor(":/cursorHoldLrud");
}

int MoveCurveNode::jog()
{
        QPoint mousepos = cpointer().pos();

        int dx, dy;
        dx = mousepos.x() - d->mousepos.x();
        dy = mousepos.y() - d->mousepos.y();

        d->mousepos = mousepos;

        if (!d->verticalOnly) {
                m_newWhen = m_newWhen + dx * d->scalefactor;
        }
        m_newValue = m_newValue - ( dy / d->curveView->boundingRect().height());

        TimeRef startoffset = d->curveView->get_start_offset();
        if ( ((TimeRef(m_newWhen) - startoffset) / d->scalefactor) > d->curveView->boundingRect().width()) {
                m_newWhen = double(d->curveView->boundingRect().width() * d->scalefactor + startoffset.universal_frame());
        }
        if ((TimeRef(m_newWhen) - startoffset) < TimeRef()) {
                m_newWhen = startoffset.universal_frame();
        }

        return calculate_and_set_node_values();
}

int MoveCurveNode::calculate_and_set_node_values()
{
        if (m_newValue < 0.0) {
                m_newValue = 0.0;
        }
        if (m_newValue > 1.0) {
                m_newValue = 1.0;
        }
        if (m_newWhen < 0.0) {
                m_newWhen = 0.0;
        }

        if (m_newWhen < d->rangeMin) {
                m_newWhen = double(d->rangeMin.universal_frame());
        } else if (d->rangeMax != qint64(-1) && m_newWhen > d->rangeMax) {
                m_newWhen = double(d->rangeMax.universal_frame());
        }

        // NOTE: this obviously only makes sense when the Node == GainEnvelope Node
        // Use a delegate (or something similar) in the future that set's the correct value.
        float dbFactor = coefficient_to_dB(m_newValue);
        cpointer().get_viewport()->set_holdcursor_text(QByteArray::number(dbFactor, 'f', 2).append(" dB"));
        cpointer().get_viewport()->set_holdcursor_pos(cpointer().scene_pos());

        return do_action();
}

