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

#include "Curve.h"
#include "CurveView.h"
#include "CurveNode.h"
#include "SheetView.h"
#include "Mixer.h"

MoveCurveNode::MoveCurveNode(Curve* curve,
	QList<CurveNode*> nodes,
	float height,
	qint64 scalefactor,
	TimeRef minWhenDiff,
	TimeRef maxWhenDiff,
	double	minValueDiff,
	double	maxValueDiff,
	const QString& des)
	: MoveCommand(curve, des)
	, d(new MoveCurveNode::Data)
{
	foreach(CurveNode* node, nodes) {
		CurveNodeData curveData;
		curveData.node = node;
		curveData.origValue = node->get_value();
		curveData.origWhen = node->get_when();
		m_nodeDatas.append(curveData);
	}

	d->height = height;
	d->minWhenDiff = minWhenDiff;
	d->maxWhenDiff = maxWhenDiff;
	d->minValueDiff = minValueDiff;
	d->maxValueDiff = maxValueDiff;
        d->scalefactor = scalefactor;
        d->verticalOnly = false;

	m_valueDiff = 0.0f;
}

void MoveCurveNode::toggle_vertical_only(bool autorepeat)
{
	if (autorepeat) {
		return;
	}

	d->verticalOnly = !d->verticalOnly;
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
        d->mousepos = QPoint(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
        return 1;
}


int MoveCurveNode::do_action()
{
	foreach(const CurveNodeData& nodeData, m_nodeDatas) {
		nodeData.node->set_when_and_value(nodeData.origWhen + m_whenDiff.universal_frame(), nodeData.origValue + m_valueDiff);
	}

        return 1;
}

int MoveCurveNode::undo_action()
{
	foreach(const CurveNodeData& nodeData, m_nodeDatas) {
		nodeData.node->set_when_and_value(nodeData.origWhen, nodeData.origValue);
	}

        return 1;
}

void MoveCurveNode::move_up(bool )
{
	m_valueDiff += m_speed / d->height;

	check_and_apply_when_and_value_diffs();
}

void MoveCurveNode::move_down(bool )
{
	m_valueDiff -= m_speed / d->height;

	check_and_apply_when_and_value_diffs();
}

void MoveCurveNode::move_left(bool )
{
	m_whenDiff -= d->scalefactor * m_speed;

	check_and_apply_when_and_value_diffs();
}

void MoveCurveNode::move_right(bool )
{
	m_whenDiff += d->scalefactor * m_speed;

	check_and_apply_when_and_value_diffs();
}

void MoveCurveNode::set_cursor_shape(int useX, int useY)
{
//        cpointer().setCursor(":/cursorHoldLrud");
}

int MoveCurveNode::jog()
{
	QPoint mousepos = cpointer().pos();

	int dx, dy;
	dx = mousepos.x() - d->mousepos.x();
	dy = mousepos.y() - d->mousepos.y();

	d->mousepos = mousepos;

	m_whenDiff += dx * d->scalefactor;
	m_valueDiff -= dy / d->height;

	return check_and_apply_when_and_value_diffs();
}

int MoveCurveNode::check_and_apply_when_and_value_diffs()
{
	if (d->verticalOnly) {
		m_whenDiff = TimeRef();
	}

	if (m_whenDiff > d->maxWhenDiff) {
		m_whenDiff = d->maxWhenDiff;
	}

	if (m_whenDiff < d->minWhenDiff) {
		m_whenDiff = d->minWhenDiff;
	}

	if (m_valueDiff > d->maxValueDiff) {
		m_valueDiff = d->maxValueDiff;
	}

	if (m_valueDiff < d->minValueDiff) {
		m_valueDiff = d->minValueDiff;
	}

        // NOTE: this obviously only makes sense when the Node == GainEnvelope Node
        // Use a delegate (or something similar) in the future that set's the correct value.
	if (m_nodeDatas.size() == 1) {
		float dbFactor = coefficient_to_dB(m_nodeDatas.first().origValue + m_valueDiff);
		cpointer().setCursorText(QByteArray::number(dbFactor, 'f', 2).append(" dB"));
	}

        cpointer().setCursorPos(cpointer().scene_pos());

        return do_action();
}



