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

$Id: AudioSourceManager.h,v 1.2 2006/05/01 22:10:58 r_sijrier Exp $
*/

#ifndef AUDIOSOURCEMANAGER_H
#define AUDIOSOURCEMANAGER_H

#include <QString>
#include <QHash>
#include <QDomDocument>
#include <QObject>


class AudioSource;
class ReadSource;

class AudioSourceManager : public QObject
{
	Q_OBJECT
	
public:
	AudioSourceManager();
	~AudioSourceManager();

	ReadSource* new_readsource(QString dir, QString name, uint channel, int songId, int bitDepth, int rate=0 );
	
	int add (ReadSource*  source);
	int remove (AudioSource* source);
	int set_state( const QDomNode& node );

	ReadSource* get_readsource(QString fileName, int channel);
	
	ReadSource* get_readsource(qint64 id);
	
	QDomNode get_state(QDomDocument doc);
	
	int get_total_sources();


private:
	QHash<qint64, ReadSource* >	sources;
	
	
signals:
	void sourceAdded();
	void sourceRemoved();
};



#endif

