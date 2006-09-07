/*
Copyright (C) 2006 Remon Sijrier 

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

$Id: Tsar.cpp,v 1.8 2006/09/07 09:36:52 r_sijrier Exp $
*/

#include "Tsar.h"

#include "ContextItem.h"
#include "AudioDevice.h"
#include "RingBuffer.h"

#include <QThread>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Tsar& tsar()
{
	static Tsar ThreadSaveAddRemove;
	return ThreadSaveAddRemove;
}

Tsar::Tsar()
{
	count = 0;
	
	rbToBeProcessed = new RingBuffer(1000 * sizeof(TsarEvent));
	rbProcessed = new RingBuffer(1000 * sizeof(TsarEvent));
	
	connect(&finishProcessedObjectsTimer, SIGNAL(timeout()), this, SLOT(finish_processed_events()));
	
	finishProcessedObjectsTimer.start( 20 );
}

Tsar::~ Tsar( )
{
	delete rbToBeProcessed;
	delete rbProcessed;
}

void Tsar::add_event( TsarEvent& data )
{
	// write the event the the lockless ringbuffer 'queue'
	rbToBeProcessed->write( (char*)(&data), sizeof(TsarEvent));
	
	count++;
}

//
//  Function called in RealTime AudioThread processing path
//
void Tsar::process_events( )
{
	// We track how many events we have added in add_event
	// and how many we have processed in finish_processed_events
	// if count == 0, there is nothing left, so we can return immediately!
	if ( ! count ) {
		return;
	}
	
/*	int objcount = 0;
	trav_time_t starttime = get_microseconds();*/
	
	while( (rbToBeProcessed->read_space() / sizeof(TsarEvent)) >= 1 ) {
		
		TsarEvent event;
		rbToBeProcessed->read( (char*)(&event), sizeof(TsarEvent));

// 		printf("called %s, slotindex %d, signalindex %d\n", event.caller->metaObject()->className(), event.slotindex, event.signalindex);
		process_event_slot(event);
		
		rbProcessed->write( (char*)(&event), sizeof(TsarEvent));
		
// 		objcount++;
	}
	
	
/*	int processtime = (int) (get_microseconds() - starttime);
	if (objcount)
		printf("\nNumber of objects processed: %d in %d useconds\n",objcount, processtime);
*/
}

void Tsar::finish_processed_events( )
{
	
	while( (rbProcessed->read_space() / sizeof(TsarEvent)) >= 1 ) {
		TsarEvent event;
		// Read one TsarEvent from the processed events ringbuffer 'queue'
		rbProcessed->read( (char*)(&event), sizeof(TsarEvent));
		
		process_event_signal(event);
		
		--count;
// 		printf("finish_processed_objects:: Count is %d\n", count);
	}
	
	static int retryCount;
	
	retryCount++;
	
	if (retryCount > 100) {
		qFatal("Unable to process thread save adding/removing of object into audio processing execution path!\n"
			"This is most likely caused by the audiodevice thread (or Jacks' one) gone wild or stalled\n"
			"One issue could be that you are not running with real time privileges! Please check for this!\n"
			"To improve future program behaviour, please report this so we can sort out the problem!\n");
	}
	
	if (count == 0) {
		retryCount = 0;
	}
	
}

TsarEvent Tsar::create_event( QObject* caller, void* argument, char* slotSignature, char* signalSignature )
{
	PENTER;
	TsarEvent event;
	event.caller = caller;
	event.argument = argument;
	int index;
	
	if (qstrlen(slotSignature) > 0) {
		index = caller->metaObject()->indexOfMethod(slotSignature);
		if (index < 0) {
			PWARN("Slot signature contains whitespaces, please remove to avoid unneeded processing (%s::%s)", caller->metaObject()->className(), slotSignature);
			QByteArray norm = QMetaObject::normalizedSignature(slotSignature);
			index = caller->metaObject()->indexOfMethod(norm.constData());
		}
		event.slotindex = index;
	} else {
		event.slotindex = -1;
	}
	
	if (qstrlen(signalSignature) > 0) {
		/* the signal index seems to have an offset of 4, so we have to substract 4 from */
		/* the value returned by caller->metaObject()->indexOfMethod*/ 
		index = caller->metaObject()->indexOfMethod(signalSignature) - 4;
		if (index < 0) {
			PWARN("Signal signature contains whitespaces, please remove to avoid unneeded processing (%s::%s)", caller->metaObject()->className(), signalSignature);
			QByteArray norm = QMetaObject::normalizedSignature(signalSignature);
			index = caller->metaObject()->indexOfMethod(norm.constData()) - 4;
		}
		event.signalindex = index; 
	} else {
		event.signalindex = -1; 
	}
	
	event.valid = true;
	
	return event;
}

void Tsar::process_event_slot( TsarEvent& event )
{
	// If there is an object to be added, do the magic to call the slot :-)
	if (event.slotindex > -1) {
		
		void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&event.argument)) };
		
		// This equals QMetaObject::invokeMethod(), without type checking. But we know that the types
		// are the correct ones, and will be casted just fine!
		if ( ! (event.caller->qt_metacall(QMetaObject::InvokeMetaMethod, event.slotindex, _a) < 0) ) {
			qDebug("Tsar::add_remove_items_in_audio_processing_path() QMetaObject::invokeMethod failed");
		}
	}
}

void Tsar::process_event_signal( TsarEvent & event )
{
	// In case the signalindex > -1, emit the signal!
	if (event.signalindex > -1) {
		// This equals emit someSignal() :-)
		void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&event.argument)) };
		QMetaObject::activate(event.caller, event.caller->metaObject(), event.signalindex, _a);
	}
}

void Tsar::process_event_slot_signal( TsarEvent & event )
{
	process_event_slot(event);
	process_event_signal(event);
}


//eof

