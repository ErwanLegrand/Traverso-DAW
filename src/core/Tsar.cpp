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
 
    $Id: Tsar.cpp,v 1.7 2006/07/06 17:38:03 r_sijrier Exp $
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
	
	rbToBeProcessed = new RingBuffer(1000 * sizeof(TsarDataStruct));
	rbProcessed = new RingBuffer(1000 * sizeof(TsarDataStruct));
	
	connect(&finishProcessedObjectsTimer, SIGNAL(timeout()), this, SLOT(finish_processed_objects()));
	
	finishProcessedObjectsTimer.start( 20 );
}

Tsar::~ Tsar( )
{
	delete rbToBeProcessed;
	delete rbProcessed;
}

void Tsar::process_object( TsarDataStruct& data )
{
	// write data to the to be processed objects ringbuffer
	rbToBeProcessed->write( (char*)(&data), sizeof(TsarDataStruct));
	
	count++;
}

//
//  Function called in RealTime AudioThread processing path
//
void Tsar::add_remove_items_in_audio_processing_path( )
{
	// We track how many objects we have inserted in process_objects
	// and how many we have processed in finish_processed_objects
	// if count == 0, there is nothing left, so we can return immediately!
	if ( ! count ) {
		return;
	}
	
/*	int objcount = 0;
	trav_time_t starttime = get_microseconds();*/
	
	while( (rbToBeProcessed->read_space() / sizeof(TsarDataStruct)) >= 1 ) {
		
		TsarDataStruct tsarData;
		rbToBeProcessed->read( (char*)(&tsarData), sizeof(TsarDataStruct));

			
		// If there is an object to be added, do the magic to call the slot :-)
		if (tsarData.funcArgument) {
			
			void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&tsarData.funcArgument)) };
			
			// This equals QMetaObject::invokeMethod(), without type checking. But we know that the types
			// are the correct ones, and will casted just fine!
			if (! (tsarData.objectToAddTo->qt_metacall(QMetaObject::InvokeMetaMethod, tsarData.slotindex, _a) < 0) ) {
				
				qDebug("Tsar::add_remove_items_in_audio_processing_path() QMetaObject::invokeMethod failed");
			
			}
		}
		
		rbProcessed->write( (char*)(&tsarData), sizeof(TsarDataStruct));
		
// 		objcount++;
	}
	
	
// 	int processtime = (int) (get_microseconds() - starttime);
// 	if (objcount)
// 		printf("\nNumber of objects processed: %d in %d useconds\n",objcount, processtime);

}

void Tsar::finish_processed_objects( )
{
	
	while( (rbProcessed->read_space() / sizeof(TsarDataStruct)) >= 1 ) {
		TsarDataStruct tsarData;
		// Read one TsarDataStruct from the processed objects ringbuffer
		rbProcessed->read( (char*)(&tsarData), sizeof(TsarDataStruct));
		
		// In case the sender object != 0, emit the signal!
		if (tsarData.sender) {
			// This equals emit someSignal() :-)
			void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&tsarData.funcArgument)) };
			QMetaObject::activate(tsarData.sender, tsarData.sender->metaObject(), tsarData.signalindex, _a);
		}
		
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


//eof
 
