/*
Copyright (C) 2005-2010 Remon Sijrier

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

*/


#ifndef TRACK_H
#define TRACK_H

#include "ProcessingData.h"
#include "defines.h"

class Track : public ProcessingData
{
        Q_OBJECT
        Q_CLASSINFO("solo", tr("Solo"))

public:
        Track (Sheet* sheet=0);
        virtual ~Track ();

        enum {
                AUDIOTRACK = 0,
                SUBGROUP = 1
        };

        static const int INITIAL_HEIGHT = 60;

        void get_state(QDomElement& element, bool istemplate=false);
        int get_height() const {return m_height;}
        int get_sort_index() const;
        VUMonitors get_vumonitors() const {return m_vumonitors;}

        virtual void set_height(int h);
        void set_muted_by_solo(bool muted);
        void set_solo(bool solo);
        void set_sort_index(int index);
        int set_state( const QDomNode& node );
        int get_type() const {return m_type;}

        bool is_muted_by_solo();
        bool is_solo();
        bool is_smaller_then(APILinkedListNode* node) {return ((Track*)node)->get_sort_index() > get_sort_index();}


protected:
        VUMonitors      m_vumonitors;
        int     m_sortIndex;
        int     m_height;
        int     m_type;
        bool    m_mutedBySolo;
        bool    m_isSolo;


public slots:
        Command* solo();


signals:
        void soloChanged(bool isSolo);
};

#endif // TRACK_H
