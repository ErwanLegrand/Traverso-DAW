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

$Id: Tsar.h,v 1.1 2006/06/27 00:05:13 r_sijrier Exp $
*/

#ifndef TSAR_H
#define TSAR_H

#include <QObject>
#include <QTimer>
#include <QMutex>

#define THREAD_SAVE_ADD(ObjectToAdd, ObjectAddedTo, functionName)  tsar().process_object(ObjectToAdd, ObjectAddedTo, functionName)
#define THREAD_SAVE_REMOVE  THREAD_SAVE_ADD

class ContextItem;

struct TsarDataStruct {
	QObject*	objectToBeAdded;
	QObject*	objectToAddTo;
	char*		slot;
};

class Tsar : public QObject
{
	Q_OBJECT

public:

	void add_remove_items_in_audio_processing_path();
	
	void process_object(QObject* objectToBeAdded, QObject* objectsToBeAddedRemoved, char* slot);

private:
	Tsar();
	Tsar(const Tsar&);

	// allow this function to create one instance
	friend Tsar& tsar();

	QTimer			addRemoveRetryTimer;
	QMutex			mutex;
	QList<TsarDataStruct >	objectsToBeAddedRemoved;
	
	volatile size_t		processAddRemove;
	

private slots:
	void start_add_remove( );
	
};

#endif

// use this function to access the context pointer
Tsar& tsar();

//eof



