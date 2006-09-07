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

$Id: Tsar.h,v 1.6 2006/09/07 09:36:52 r_sijrier Exp $
*/

#ifndef TSAR_H
#define TSAR_H

#include <QObject>
#include <QTimer>
#include <QByteArray>

#define THREAD_SAVE_CALL(caller, argument, slotSignature)  { \
		TsarEvent event = tsar().create_event(caller, argument, #slotSignature, ""); \
		tsar().add_event(event);\
	}

#define THREAD_SAVE_EMIT_SIGNAL(caller, argument, signalSignature) {\
		TsarEvent event = tsar().create_event(caller, argument, "", #signalSignature); \
		tsar().add_event(event);\
}\

#define THREAD_SAVE_CALL_EMIT_SIGNAL(caller, argument, slotSignature, signalSignature)  { \
	TsarEvent event = tsar().create_event(caller, argument, #slotSignature, #signalSignature); \
	tsar().add_event(event);\
	}\


class ContextItem;
class RingBuffer;

struct TsarEvent {
// used for slot invokation stuff
	QObject* 	caller;
	void*		argument;
	int		slotindex;

// Used for the signal emiting stuff
	int signalindex;
	
	bool valid;
};

class Tsar : public QObject
{
	Q_OBJECT

public:

	void process_events();
	TsarEvent create_event(QObject* caller, void* argument, char* slotSignature, char* signalSignature);
	
	void add_event(TsarEvent& event);
	void process_event_slot(TsarEvent& event);
	void process_event_signal(TsarEvent& event);
	void process_event_slot_signal(TsarEvent& event);

private:
	Tsar();
	~Tsar();
	Tsar(const Tsar&);

	// allow this function to create one instance
	friend Tsar& tsar();

	QTimer			finishProcessedObjectsTimer;
	
	RingBuffer*		rbToBeProcessed;
	RingBuffer*		rbProcessed;
	
	int 			count;


private slots:
	void finish_processed_events();
	
};

#endif

// use this function to access the context pointer
Tsar& tsar();

//eof



