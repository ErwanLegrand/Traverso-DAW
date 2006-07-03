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
 
    $Id: Tsar.cpp,v 1.5 2006/07/03 17:51:56 r_sijrier Exp $
*/

#include "Tsar.h"

#include "ContextItem.h"
#include "AudioDevice.h"
#include "RingBuffer.h"

#include <QByteArray>
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
	processAddRemove = 0; 
	
/*	printf("Tsardata struct is %d bytes, Ringbuffer size is %d bytes\n",sizeof(TsarDataStruct), 1000 * sizeof(TsarDataStruct));
	rbToBeProcessed = new RingBuffer(1000 * sizeof(TsarDataStruct));
	
	printf("write space is %d\n",rbToBeProcessed->write_space());*/
	
	addRemoveRetryTimer.setSingleShot( true );

	connect(&addRemoveRetryTimer, SIGNAL(timeout()), this, SLOT(start_add_remove()));
	connect(&finishProcessedObjectsTimer, SIGNAL(timeout()), this, SLOT(finish_processed_objects()));
}

void Tsar::process_object( TsarDataStruct& data )
{
	mutex.lock();
	objectsToBeProcessed.append(data);
	mutex.unlock();
	
	start_add_remove();
	finishProcessedObjectsTimer.start( 20 );
}

//
//  Function called in RealTime AudioThread processing path
//
void Tsar::add_remove_items_in_audio_processing_path( )
{

	if ( ! processAddRemove ) {
		return;
	}
	
// 	PMESG("processAddRemove is true, entering add_remove_items_in_audio_processing_path()");
	
	if ( ! mutex.tryLock() ) {
// 		printf("Tsar::add_remove_items_in_audio_processing_path() : Couldn't lock mutex, retrying next cycle\n");
		return;
	}
	
	int count = 0;
	
// 	trav_time_t starttime = get_microseconds();
	
	foreach(TsarDataStruct tsarData, objectsToBeProcessed) {
	
		void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&tsarData.objectToBeAdded)) };
		// This equals QMetaObject::invokeMethod(), without type checking. But we know that the types
		// are the correct ones, and will casted just fine!
		if (! (tsarData.objectToAddTo->qt_metacall(QMetaObject::InvokeMetaMethod, tsarData.slotindex, _a) < 0) ) {
			
			qDebug("Tsar::add_remove_items_in_audio_processing_path() QMetaObject::invokeMethod failed");
// 			qDebug("ToBeAddedRemovedObject: %s from %s::%s\n", tsarData.objectToBeAdded->metaObject()->className(), tsarData.objectToAddTo->metaObject()->className(), tsarData.slot);
		
		} else {
			
// 			printf("Tsar::add_remove_items_in_audio_processing_path() QMetaObject::invokeMethod: Succes!\n");
// 			printf("ToBeAddedRemovedObject: %s from %s::%s\n", tsarData.objectToBeAdded->metaObject()->className(), tsarData.objectToAddTo->metaObject()->className(), tsarData.slot);
// 			
			processedObjects.append( tsarData );
		}	
		
// 		PMESG("ToBeAddedRemovedObject: %s from %s::%s", tsarData.objectToBeAdded->metaObject()->className(), tsarData.objectToAddTo->metaObject()->className(), tsarData.slot);
		

		count++;
	}
	
	objectsToBeProcessed.clear();
	
// 	int processtime = (int) (get_microseconds() - starttime);
// 	printf("\nNumber of objects processed: %d in %d useconds\n",count, processtime);

	mutex.unlock();
	
// 	PMESG("done add_remove_items_in_audio_processing_path(), set processAddRemove to 0\n");
	
	processAddRemove = 0;
}

void Tsar::start_add_remove( )
{
	PENTER;
	static int retryCount;
	
	if (processAddRemove) {
		
		if (!addRemoveRetryTimer.isActive()) {
			addRemoveRetryTimer.start( 10 );
			PMESG("processAddRemove is still true, retrying in 10 ms !!");
			retryCount++;
		}
		
		
		if (retryCount > 100) {
			qFatal("Cannot start ThreadSaveAddRemove::start_add_remove(), processAddRemove remains true.\n"
				"This is most likely caused by the audiodevice thread (or Jacks' one) gone wild or stalled\n"
				"One issue could be that you are not running with real time privileges! Please check for this!\n"
				"To improve future program behaviour, please report this so we can sort out the problem!\n");
		}
		
		return;
	}
	
	processAddRemove = 1;
	retryCount = 0;
	
}

void Tsar::finish_processed_objects( )
{
// 	printf("Entering finish_processed_objects()\n");
	
	mutex.lock();
	
	if (processedObjects.isEmpty() && objectsToBeProcessed.isEmpty()) {
		finishProcessedObjectsTimer.stop();
		mutex.unlock();
// 		printf("Stopped the finishProcessedObjectsTimer\n");
		return;
	}
	
	foreach(TsarDataStruct tsarData, processedObjects) {
		if (tsarData.sender) {
			// This equals emit someSignal() :-)
			void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&tsarData.objectToBeAdded)) };
			QMetaObject::activate(tsarData.sender, tsarData.sender->metaObject(), tsarData.signalindex, _a);
		}
	}
	
	processedObjects.clear();
	
	mutex.unlock();
}

//eof
 
