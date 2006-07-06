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

$Id: Tsar.h,v 1.5 2006/07/06 17:38:03 r_sijrier Exp $
*/

#ifndef TSAR_H
#define TSAR_H

#include <QObject>
#include <QTimer>
#include <QByteArray>

#define THREAD_SAVE_CALL(ObjectAddedTo, slotSignature, slotArgument)  { \
		THREAD_SAVE_CALL_EMIT_SIGNAL(ObjectAddedTo, slotArgument, slotSignature, "")\
	}

#define THREAD_SAVE_EMIT_SIGNAL(sender, signalSignature, functionArgument) {\
	THREAD_SAVE_CALL_EMIT_SIGNAL(sender, functionArgument, "", signalSignature)\
}\

#define THREAD_SAVE_CALL_EMIT_SIGNAL(ObjectAddedTo, functionArgument, slotSignature, signalSignature)  { \
	TsarDataStruct tsardata;\
	tsardata.objectToAddTo = ObjectAddedTo;\
	tsardata.funcArgument = (void*)functionArgument;\
	\
	int index = ObjectAddedTo->metaObject()->indexOfMethod(#slotSignature);\
	if (index < 0) {\
	        QByteArray norm = QMetaObject::normalizedSignature(#slotSignature);\
        	index = ObjectAddedTo->metaObject()->indexOfMethod(norm.constData());\
        }\
        \
	tsardata.slotindex = index;\
	\
	if ( ! QString(#signalSignature).isEmpty()) {\
		/* the signal index seems to have an offset of 4, so we have to substract 4 from */\
		/* the value returned by ObjectAddedTo->metaObject()->indexOfMethod*/ \
		index = ObjectAddedTo->metaObject()->indexOfMethod(#signalSignature) - 4;\
		if (index < 0) {\
			QByteArray norm = QMetaObject::normalizedSignature(#signalSignature);\
			index = ObjectAddedTo->metaObject()->indexOfMethod(norm.constData()) - 4;\
		}\
		tsardata.signalindex = index; \
		tsardata.sender = ObjectAddedTo;\
	} else {\
		tsardata.sender = 0;\
	}\
	\
	tsar().process_object(tsardata);\
	}\


class ContextItem;
class RingBuffer;

struct TsarDataStruct {
// used for slot invokation stuff
	void*		funcArgument;
	QObject*	objectToAddTo;
	int		slotindex;

// Used for the signal emiting stuff
	QObject* sender;
	int signalindex;
};

class Tsar : public QObject
{
	Q_OBJECT

public:

	void add_remove_items_in_audio_processing_path();
	
	void process_object(TsarDataStruct& data);

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
	void finish_processed_objects();
	
};

#endif

// use this function to access the context pointer
Tsar& tsar();

//eof



