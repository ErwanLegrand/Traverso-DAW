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

$Id: Gain.cpp,v 1.6 2006/11/08 14:52:11 r_sijrier Exp $
*/

#include "Gain.h"

#include "ContextItem.h"
#include "ContextPointer.h"
#include "Mixer.h"
#include <ViewPort.h>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Gain::Gain(ContextItem* context, const QString& des, float gain)
	: Command(context, des)
{
	gainObject = context;
	
	if (gain >= 0) {
		newGain = gain;
		QMetaObject::invokeMethod(gainObject, "get_gain",
					Qt::DirectConnection,
					Q_RETURN_ARG(float, origGain));
	}
}


Gain::~Gain()
{}

int Gain::prepare_actions()
{
	return 1;
}

int Gain::begin_hold(int useX, int useY)
{
	if ( ! QMetaObject::invokeMethod(gainObject, "get_gain",
					Qt::DirectConnection,
					Q_RETURN_ARG(float, origGain)) ) {
		PWARN("Gain::begin_hold QMetaObject::invokeMethod failed");
		return 0;
	}	
	
	set_cursor_shape(useX, useY);
	
	newGain = origGain;
	origY = cpointer().y();
	return 1;
}

int Gain::finish_hold()
{
	cpointer().get_viewport()->reset_context();
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


void Gain::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	mousePos = QCursor::pos();	
	cpointer().get_viewport()->set_hold_cursor(":/cursorGain");
}


int Gain::jog()
{
	PENTER;
	float ofy = 0;
	
	float dbFactor = coefficient_to_dB(newGain);
	
	if (dbFactor > -1)
		ofy = (origY - cpointer().y()) * 0.05;
	if (dbFactor <= -1) {
		ofy = (origY - cpointer().y()) * ((1 - dB_to_scale_factor(dbFactor)) / 3);
	}
		
		
	newGain = dB_to_scale_factor( dbFactor + ofy );
	origY = cpointer().y();
	
	cpointer().get_viewport()->set_hold_cursor_text(QByteArray::number(dbFactor, 'f', 2).append(" dB"));
	
	return QMetaObject::invokeMethod(gainObject, "set_gain", Q_ARG(float, newGain));
}

// eof

