/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: AudioSourcesList.h,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#ifndef AUDIOSOURCESLIST_H
#define AUDIOSOURCESLIST_H

#include <QString>
#include <QHash>
#include <QDomDocument>


class AudioSource;

class AudioSourcesList
{
public:
        AudioSourcesList();
        ~AudioSourcesList();

        AudioSource* new_audio_source(int rate, int bitDepth, int songId, QString name, uint channel, QString dir);
        int add
                (AudioSource*  source, int channel);
        int remove
                (AudioSource* source);

        AudioSource* get_source(qint64 id);
        AudioSource* get_source(QString fileName, int channel);
        QDomNode get_state(QDomDocument doc);
        int get_total_sources();

        int set_state( const QDomNode& node );

private:
        QHash<qint64, AudioSource* >	sources;
};



#endif

