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
 
    $Id: Tsar.cpp,v 1.1 2006/06/27 00:05:13 r_sijrier Exp $
*/

#include "Tsar.h"

#include "ContextItem.h"
#include "AudioDevice.h"
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
	
	addRemoveRetryTimer.setSingleShot( true );

	connect(&addRemoveRetryTimer, SIGNAL(timeout()), this, SLOT(start_add_remove()));
}

void Tsar::process_object( QObject * objectToBeAdded, QObject* objectToAddTo, char * slot )
{
	PENTER;
	
	TsarDataStruct tsarstruct;
	tsarstruct.objectToBeAdded = objectToBeAdded;
	tsarstruct.objectToAddTo = objectToAddTo;
	tsarstruct.slot = slot;
	
	mutex.lock();
	objectsToBeAddedRemoved.append(tsarstruct);
	mutex.unlock();
	
	processAddRemove = 1;
// 	start_add_remove();
}


//
//  Function called in RealTime AudioThread processing path
//

void Tsar::add_remove_items_in_audio_processing_path( )
{

	if ( ! processAddRemove ) {
		return;
	}
	
	PMESG("processAddRemove is true, entering add_remove_items_in_audio_processing_path()");
	
	if ( ! mutex.tryLock() ) {
		printf("Tsar::add_remove_items_in_audio_processing_path() : Couldn't lock mutex, retrying next cycle\n");
		return;
	}
	
	int count = 0;
	
	trav_time_t starttime = get_microseconds();
	
	foreach(TsarDataStruct tsarData, objectsToBeAddedRemoved) {
		
		char* slot = QByteArray("thread_save_").append(tsarData.slot).data();
		
		if ( ! QMetaObject::invokeMethod(tsarData.objectToAddTo, slot, Qt::DirectConnection, Q_ARG(QObject*, tsarData.objectToBeAdded)) ) {
			qDebug("Tsar::add_remove_items_in_audio_processing_path() QMetaObject::invokeMethod failed");
			qDebug("ToBeAddedRemovedObject: %s from %s::%s\n", tsarData.objectToBeAdded->metaObject()->className(), tsarData.objectToAddTo->metaObject()->className(), slot);
		} else {
			PMESG("Tsar::add_remove_items_in_audio_processing_path() QMetaObject::invokeMethod: Succes!");
		}	
		
		PMESG("ToBeAddedRemovedObject: %s from %s::%s\n", tsarData.objectToBeAdded->metaObject()->className(), tsarData.objectToAddTo->metaObject()->className(), slot);
		
		count++;
	}
	
	objectsToBeAddedRemoved.clear();


	int processtime = (int) (get_microseconds() - starttime);
	printf("Number of objects processed: %d in %d useconds\n",count, processtime);

	mutex.unlock();
	
	PMESG("done add_remove_items_in_audio_processing_path(), set processAddRemove to 0\n");
	
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


//eof
 
