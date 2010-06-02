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

class TSend;

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

        void get_state(QDomDocument& doc, QDomElement& element, bool istemplate=false);
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

        void add_input_bus(const QString& name);
        void add_post_send(qint64 busId);
        void remove_post_sends(QList<qint64> sendIds);
        void add_input_bus(qint64 busId);



        AudioBus* get_process_bus() const {return m_processBus;}
        AudioBus* get_input_bus() const {return m_inputBus;}

        QString get_bus_in_name() const {return m_busInName;}
        QString get_bus_out_name() const{return m_busOutName;}

        QList<TSend*> get_post_sends() const;


protected:
        VUMonitors      m_vumonitors;
        int     m_sortIndex;
        int     m_height;
        int     m_type;
        bool    m_mutedBySolo;
        bool    m_isSolo;

        APILinkedList   m_postSends;

        APILinkedList   m_inputBuses;
        APILinkedList   m_outputBuses;
        APILinkedList   m_preFaderBuses;

        AudioBus*       m_inputBus;
        AudioBus*       m_outputBus;
        QString         m_busInName;
        QString         m_busOutName;
        void process_post_sends(nframes_t nframes);
        void process_pre_fader_sends(nframes_t nframes);
        virtual void add_input_bus(AudioBus* bus);

public slots:
        Command* solo();

private slots:
        void audiodevice_params_changed();
        void private_add_post_send(TSend*);
        void private_remove_post_send(TSend*);
        void private_add_input_bus(AudioBus*);
        void rescan_buses();


signals:
        void soloChanged(bool isSolo);
        void busConfigurationChanged();
        void routingConfigurationChanged();
};

#endif // TRACK_H
