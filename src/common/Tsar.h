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

$Id: Tsar.h,v 1.4 2008/02/11 10:11:52 r_sijrier Exp $
*/

#ifndef TSAR_H
#define TSAR_H

#include <QObject>
#include <QTimer>
#include <QByteArray>
#include "RingBufferNPT.h"

#define THREAD_SAVE_INVOKE(caller, argument, slotSignature)  { \
		TsarEvent event = tsar().create_event(caller, argument, #slotSignature, ""); \
		while (!tsar().add_event(event)) { printf("THREAD_SAVE_INVOKE: failed to add event, trying again\n");} \
	}

#define RT_THREAD_EMIT(cal, arg, signalSignature) {\
	TsarEvent event; \
	event.caller = cal; \
	event.argument = arg; \
	event.slotindex = -1; \
	static int retrievedsignalindex; \
	\
	if ( ! retrievedsignalindex ) { \
		/* the signal index seems to have an offset of 4, so we have to substract 4 from */ \
		/* the value returned by caller->metaObject()->indexOfMethod*/  \
		retrievedsignalindex = cal->metaObject()->indexOfMethod(#signalSignature) - 4; \
		Q_ASSERT(retrievedsignalindex >= 0); \
	} \
	event.signalindex = retrievedsignalindex; \
	event.valid = true; \
	tsar().add_rt_event(event); \
}\


#define THREAD_SAVE_INVOKE_AND_EMIT_SIGNAL(caller, argument, slotSignature, signalSignature)  { \
	TsarEvent event = tsar().create_event(caller, argument, #slotSignature, #signalSignature); \
	tsar().add_event(event);\
	}\


struct TsarEvent {
// used for slot invokation stuff
	QObject* 	caller;
	void*		argument;
	int		slotindex;
	void*		_a[];

// Used for the signal emiting stuff
	int signalindex;
	
	bool valid;
};

class Tsar : public QObject
{
	Q_OBJECT

public:
	TsarEvent create_event(QObject* caller, void* argument, const char* slotSignature, const char* signalSignature);
	
	bool add_event(TsarEvent& event);
	void add_rt_event(TsarEvent& event);
	void process_event_slot(const TsarEvent& event);
	void process_event_signal(const TsarEvent& event);
	void process_event_slot_signal(const TsarEvent& event);

private:
	Tsar();
	~Tsar();
	Tsar(const Tsar&);

	// allow this function to create one instance
	friend Tsar& tsar();
	// The AudioDevice instance is the _only_ one who
	// is allowed to call process_events() !!
	friend class AudioDevice;

	QList<RingBufferNPT<TsarEvent>*>	m_events;
	RingBufferNPT<TsarEvent>*		oldEvents;
	QTimer	finishOldEventsTimer;
	int 	m_eventCounter;
	int 	m_retryCount;

#if defined (THREAD_CHECK)
	unsigned long	m_threadId;
#endif

	void process_events();

private slots:
	void finish_processed_events();
	
};

// use this function to access the context pointer
Tsar& tsar();

#endif


//eof



