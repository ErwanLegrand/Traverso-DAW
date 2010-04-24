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

$Id: Tsar.cpp,v 1.4 2008/02/11 10:11:52 r_sijrier Exp $
*/

#include "Tsar.h"

#include "AudioDevice.h"
#include "InputEngine.h"
#include <QMetaMethod>
#include <QMessageBox>
#include <QCoreApplication>
#include <QThread>
#include <QTimerEvent>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
 * 	\class Tsar
 * 	\brief Tsar (Thread Save Add and Remove) is a singleton class to call  
 *		functions (both signals and slots) in a thread save way without
 *		using any mutual exclusion primitives (mutex)
 *
 */


/**
 * 
 * @return The Tsar instance. 
 */
Tsar& tsar()
{
	static Tsar ThreadSaveAddRemove;
	return ThreadSaveAddRemove;
}

Tsar::Tsar()
{
	m_eventCounter = 0;

        int guiThreadEventsBufferSize = 10000;
        int audioThreadEventsBufferSize = 1000;
	
	m_events.append(new RingBufferNPT<TsarEvent>(guiThreadEventsBufferSize));
	oldEvents = new RingBufferNPT<TsarEvent>(guiThreadEventsBufferSize);

	m_events.append(new RingBufferNPT<TsarEvent>(audioThreadEventsBufferSize));

	m_retryCount = 0;
	
#if defined (THREAD_CHECK)
	m_threadId = QThread::currentThreadId ();
#endif

        m_timer.start(20, this);
}

Tsar::~ Tsar( )
{
	foreach(RingBufferNPT<TsarEvent>* eventBuffer, m_events) {
		delete eventBuffer;
	}
	delete oldEvents;
}

void Tsar::timerEvent(QTimerEvent *event)
{
        if (event->timerId() == m_timer.timerId()) {
                finish_processed_events();
        }
}

/**
 * 	Use this function to add events to the event queue when 
 * 	called from the GUI thread.
 *
 *	Note: This function should be called ONLY from the GUI thread! 
 * @param event  The event to add to the event queue
 */
bool Tsar::add_event(TsarEvent& event )
{
#if defined (THREAD_CHECK)
	Q_ASSERT_X(m_threadId == QThread::currentThreadId (), "Tsar::add_event", "Adding event from other then GUI thread!!");
#endif
	if (m_events.at(0)->write(&event, 1) == 1) {
		m_eventCounter++;
		return true;
	}
	m_retryCount = 0;
	return false;
}

/**
 * 	Use this function to add events to the event queue when  
 * 	called from the audio processing (real time) thread
 *
 *	Note: This function should be called ONLY from the realtime audio thread and has a 
 *	non blocking behaviour! (That is, it's a real time save function)
 *
 * @param event The event to add to the event queue
 */
void Tsar::add_rt_event( TsarEvent& event )
{
#if defined (THREAD_CHECK)
	Q_ASSERT_X(m_threadId != QThread::currentThreadId (), "Tsar::add_rt_event", "Adding event from NON-RT Thread!!");
#endif
	m_events.at(1)->write(&event, 1);
}

//
//  Function called in RealTime AudioThread processing path
//
void Tsar::process_events( )
{
//#define profile

	for (int i=0; i<m_events.size(); ++i) {
		RingBufferNPT<TsarEvent>* newEvents = m_events.at(i);
		
		int processedCount = 0;
		int newEventCount = newEvents->read_space();
	
		while((newEventCount > 0) && (processedCount < 50)) {
#if defined (profile)
			trav_time_t starttime = get_microseconds();
#endif
			TsarEvent event;
			
			newEvents->read(&event, 1);
	
			process_event_slot(event);
			
			oldEvents->write(&event, 1);
			
			--newEventCount;
			++processedCount;
	
#if defined (profile)
			int processtime = (int) (get_microseconds() - starttime);
			printf("called %s::%s, (signal: %s) \n", event.caller->metaObject()->className(), 
			(event.slotindex >= 0) ? event.caller->metaObject()->method(event.slotindex).signature() : "", 
                        (event.signalindex >= 0) ? event.caller->metaObject()->method(event.signalindex).signature() : "");
			printf("Process time: %d useconds\n\n", processtime);
#endif
		}
	}
}

void Tsar::finish_processed_events( )
{
	
	while(oldEvents->read_space() >= 1 ) {
		TsarEvent event;
		// Read one TsarEvent from the processed events ringbuffer 'queue'
		oldEvents->read(&event, 1);
		
		process_event_signal(event);
		
		--m_eventCounter;
// 		printf("finish_processed_objects:: Count is %d\n", m_eventCounter);
	}
	
	m_retryCount++;
	
	if (m_retryCount > 200) {
		if (ie().is_holding()) {
			ie().suspend();
		}
		
		if (audiodevice().get_driver_type() != "Null Driver") {
			QMessageBox::critical( 0, 
				tr("Traverso - Malfunction!"), 
				tr("The Audiodriver Thread seems to be stalled/stopped, but Traverso didn't ask for it!\n"
				"This effectively makes Traverso unusable, since it relies heavily on the AudioDriver Thread\n"
				"To ensure proper operation, Traverso will fallback to the 'Null Driver'.\n"
				"Potential issues why this can show up are: \n\n"
				"* You're not running with real time privileges! Please make sure this is setup properly.\n\n"
				"* The audio chipset isn't supported (completely), you probably have to turn off some of it's features.\n"
				"\nFor more information, see the Help file, section: \n\n AudioDriver: 'Thread stalled error'\n\n"),
				"OK", 
				0 );
                        AudioDeviceSetup ads;
                        ads.driverType = "Null Driver";
                        audiodevice().set_parameters(ads);
			m_retryCount = 0;
		} else {
			QMessageBox::critical( 0, 
				tr("Traverso - Fatal!"), 
				tr("The Null AudioDriver stalled too, exiting application!"),
				"OK", 
				0 );
			QCoreApplication::exit(-1);
		}
	}
	
	if (m_eventCounter <= 0) {
		m_retryCount = 0;
	}
}

/**
 * 	Creates a Tsar event. Add the tsar event to the event queue by calling add_event()
 * 	If you need to add an event from the real time audio processing thread, use
 * 	add_rt_event() instead!
 *
 *	Note: This function can be called both from the GUI and realtime audio thread and has a 
 *	non blocking behaviour! (That is, it's a real time save function)
 *
 * @param caller	The calling object, needs to be derived from a QObject
 * @param argument 	The slot and/or signal argument which can be of any type.
 * @param slotSignature The 'signature' of the calling objects slot (equals the name of the slot function)
 * @param signalSignature The 'signature' of the calling objects signal (equals the name of the signal function) 
 * @return The newly created event.
 */
TsarEvent Tsar::create_event( QObject* caller, void* argument, const char* slotSignature, const char* signalSignature )
{
	PENTER3;
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
			if (index < 0) {
				PERROR("Couldn't find a valid index for %s", slotSignature);
			}
		}
		event.slotindex = index;
	} else {
		event.slotindex = -1;
	}
	
	if (qstrlen(signalSignature) > 0) {
                index = caller->metaObject()->indexOfMethod(signalSignature);
		if (index < 0) {
			PWARN("Signal signature contains whitespaces, please remove to avoid unneeded processing (%s::%s)", caller->metaObject()->className(), signalSignature);
			QByteArray norm = QMetaObject::normalizedSignature(signalSignature);
                        index = caller->metaObject()->indexOfMethod(norm.constData());
		}
		event.signalindex = index; 
	} else {
		event.signalindex = -1; 
	}
	
	event.valid = true;
	
	return event;
}

/**
*	This function can be used to process the events 'slot' part.
*	Usefull when you have a Tsar event, but don't want/need to use tsar
*	to call the events slot in a thread save way
*
*	Note: This function doesn't provide the thread safetyness you get with
*		the add_event() function!
*
* @param event The TsarEvent to be processed 
*/
void Tsar::process_event_slot(const TsarEvent& event )
{
	// If there is an object to be added, do the magic to call the slot :-)
	if (event.slotindex > -1) {

		void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&event.argument)) };
		
		// This equals QMetaObject::invokeMethod(), without type checking. But we know that the types
		// are the correct ones, and will be casted just fine!
		if ( ! (event.caller->qt_metacall(QMetaObject::InvokeMetaMethod, event.slotindex, _a) < 0) ) {
			qDebug("Tsar::process_event_slot failed (%s::%s)", event.caller->metaObject()->className(), event.caller->metaObject()->method(event.slotindex).signature());
		}
	}
}

/**
*	This function can be used to process the events 'signal' part.
*	Usefull when you have a Tsar event, but don't want/need to use tsar
*	to call the events signal in a thread save way
*
*	Note: This function doesn't provide the thread safetyness you get with
*		the add_event() function!
*
* @param event The TsarEvent to be processed 
*/
void Tsar::process_event_signal(const TsarEvent & event )
{
	// In case the signalindex > -1, emit the signal!
	if (event.signalindex > -1) {

                void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&event.argument)) };

                // This equals QMetaObject::invokeMethod(), without type checking. But we know that the types
                // are the correct ones, and will be casted just fine!
                if ( ! (event.caller->qt_metacall(QMetaObject::InvokeMetaMethod, event.signalindex, _a) < 0) ) {
                        qDebug("Tsar::process_event_signal failed (%s::%s)", event.caller->metaObject()->className(), event.caller->metaObject()->method(event.signalindex).signature());
                }
        }
}

/**
*	Convenience function. Calls both process_event_slot() and process_event_signal()
*
*	\sa process_event_slot() \sa process_event_signal()
*
*	Note: This function doesn't provide the thread safetyness you get with
*		the add_event() function!
*
* @param event The TsarEvent to be processed 
*/
void Tsar::process_event_slot_signal(const TsarEvent & event )
{
	process_event_slot(event);
	process_event_signal(event);
}

//eof

