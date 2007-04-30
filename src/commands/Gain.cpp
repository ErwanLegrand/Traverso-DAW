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

$Id: Gain.cpp,v 1.19 2007/04/30 11:08:18 r_sijrier Exp $
*/

#include "Gain.h"

#include "ContextItem.h"
#include "ContextPointer.h"
#include "Mixer.h"
#include <ViewPort.h>
#include <Track.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Gain::Gain(ContextItem* context, QVariantList args)
	: Command(context, "")
{
	gainObject = context;
	horiz = false;

	float gain = -1;
	QString des = "";
	
	if (args.size() > 0 && args[0].toString() != "horizontal") {
		gain = args.at(0).toDouble();
		des = QString(context->metaObject()->className()) + ": Reset gain";
	} else {
		des = QString(context->metaObject()->className()) + " Gain";
	}

	if (args.size() > 0 && args[0].toString() == "horizontal") {
		horiz = true;
	}
	
	setText(des);
	
	if (gain >= 0) {
		newGain = gain;
		get_gain_from_object(origGain);
	}
	
	Track* track = qobject_cast<Track*>(context);
	if (track && origGain == 0.5) {
		newGain = 1.0;
	}
}


Gain::~Gain()
{}

int Gain::prepare_actions()
{
	if (origGain == newGain) {
		// Nothing happened!
		return -1;
	}
	return 1;
}

int Gain::begin_hold()
{
	if ( ! get_gain_from_object(origGain)) {
		return -1;
	}
	newGain = origGain;
	origPos = QPoint(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
	return 1;
}

int Gain::finish_hold()
{
	QCursor::setPos(mousePos);
	return 1;
}

int Gain::do_action()
{
	PENTER;
	if ( ! QMetaObject::invokeMethod(gainObject, "set_gain", Q_ARG(float, newGain))) {
		PWARN("Gain::do_action QMetaObject::invokeMethod failed");
		return 0;
	}	
	
	return 1;
}

int Gain::undo_action()
{
	PENTER;
	if ( ! QMetaObject::invokeMethod(gainObject, "set_gain", Q_ARG(float, origGain)) ) {
		PWARN("Gain::undo_action QMetaObject::invokeMethod failed");
		return 0;
	}
	
	return 1;
}


void Gain::cancel_action()
{
	finish_hold();
	undo_action();
}


void Gain::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	mousePos = QCursor::pos();
	if (horiz) {
		cpointer().get_viewport()->set_holdcursor(":/cursorHoldLr");
	} else {
		cpointer().get_viewport()->set_holdcursor(":/cursorGain");
	}
}


void Gain::increase_gain( bool autorepeat )
{
	float dbFactor = coefficient_to_dB(newGain);
	dbFactor += 0.2;
	newGain = dB_to_scale_factor(dbFactor);
	QMetaObject::invokeMethod(gainObject, "set_gain", Q_ARG(float, newGain));
	
	if (!horiz) {
		// now we get the new gain value from gainObject, since we don't know if 
		// gainobject accepted the change or not!
		get_gain_from_object(newGain);
	
		// Update the vieport's hold cursor with the _actuall_ gain value!
		cpointer().get_viewport()->set_holdcursor_text(QByteArray::number(dbFactor, 'f', 2).append(" dB"));
	}
}

void Gain::decrease_gain(bool autorepeat)
{
	float dbFactor = coefficient_to_dB(newGain);
	dbFactor -= 0.2;
	newGain = dB_to_scale_factor(dbFactor);
	
	QMetaObject::invokeMethod(gainObject, "set_gain", Q_ARG(float, newGain));
	
	if (!horiz) {
		// now we get the new gain value from gainObject, since we don't know if 
		// gainobject accepted the change or not!
		get_gain_from_object(newGain);

		// Update the vieport's hold cursor with the _actuall_ gain value!
		cpointer().get_viewport()->set_holdcursor_text(QByteArray::number(dbFactor, 'f', 2).append(" dB"));
	}
}


int Gain::jog()
{
	PENTER;
	
	float of = 0;
	
	float dbFactor = coefficient_to_dB(newGain);
	
	int diff;

	if (horiz) {
		diff = cpointer().x() - origPos.x();
	} else {
		diff = origPos.y() - cpointer().y();
	}

	if (dbFactor > -1) {
		of = diff * 0.05;
	}
	if (dbFactor <= -1) {
		of = diff * ((1 - dB_to_scale_factor(dbFactor)) / 3);
	}
	
	
	newGain = dB_to_scale_factor( dbFactor + of );
	
	// Set the gain for gainObject
	QMetaObject::invokeMethod(gainObject, "set_gain", Q_ARG(float, newGain));
	
	// now we get the new gain value from gainObject, since we don't know if 
	// gainobject accepted the change or not!
	int result = get_gain_from_object(newGain);
	
	// Update the vieport's hold cursor!
	if (!horiz) {
		cpointer().get_viewport()->set_holdcursor_text(QByteArray::number(dbFactor, 'f', 2).append(" dB"));
	}

	QCursor::setPos(mousePos);
// 	origPos = cpointer().pos();

	return result;
}

int Gain::get_gain_from_object(float& gain)
{
	if ( ! QMetaObject::invokeMethod(gainObject, "get_gain",
					Qt::DirectConnection,
					Q_RETURN_ARG(float, gain)) ) {
		PWARN("Gain::get_gain_from_object QMetaObject::invokeMethod failed");
		return 0;
	}	
	
	return 1;
}


// eof

