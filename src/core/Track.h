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

public:
        Track (TSession* session=0);
        virtual ~Track ();

        enum {
                AUDIOTRACK = 0,
                BUS = 1
        };

        static const int INITIAL_HEIGHT = 90;

        void get_state(QDomDocument& doc, QDomElement& element, bool istemplate=false);
        int get_sort_index() const;
        VUMonitors get_vumonitors() const {return m_vumonitors;}

        void set_muted_by_solo(bool muted);
        void set_solo(bool solo);
        void set_sort_index(int index);
        int set_state( const QDomNode& node );
        virtual void set_name(const QString& name);
        void set_channel_count(int count);
        int get_channel_count() const {return m_channelCount;}
        int get_type() const {return m_type;}

        bool is_muted_by_solo();
        bool is_solo();
	bool presend_on() const {return m_preSendOn;}
	bool show_track_volume_automation() const {return m_showTrackVolumeAutomation;}
	bool is_smaller_then(APILinkedListNode* node) {return ((Track*)node)->get_sort_index() > get_sort_index();}

        void add_input_bus(const QString& name);
        void add_post_send(qint64 busId);
        void add_pre_send(qint64 busId);
        void remove_post_sends(QList<qint64> sendIds);
        void remove_pre_sends(QList<qint64> sendIds);
        void add_input_bus(qint64 busId);

        bool connect_to_jack(bool inports, bool outports);
        bool disconnect_from_jack(bool inports, bool outports);

        AudioBus* get_process_bus() const {return m_processBus;}
        AudioBus* get_input_bus() const {return m_inputBus;}

        QString get_bus_in_name() const {return m_busInName;}

        QList<TSend*> get_post_sends() const;
        QList<TSend*> get_pre_sends() const;
        TSend* get_send(qint64 sendId);


protected:
        VUMonitors      m_vumonitors;
        int             m_sortIndex;
        int             m_type;
        int             m_channelCount;
        bool            m_mutedBySolo;
        bool            m_isSolo;
	bool		m_showTrackVolumeAutomation;
	bool		m_preSendOn;

        APILinkedList   m_postSends;
        APILinkedList   m_preSends;

        AudioBus*       m_inputBus;
        QString         m_busInName;

        void process_post_sends(nframes_t nframes);
        void process_pre_sends(nframes_t nframes);
        virtual void add_input_bus(AudioBus* bus);
        void remove_input_bus(AudioBus* bus);

private:
        void process_send(TSend* send, nframes_t nframes);

public slots:
        TCommand* solo();
	TCommand* toggle_presend();
	TCommand* toggle_show_track_volume_automation();

private slots:
        void private_add_post_send(TSend*);
        void private_add_pre_send(TSend *);
        void private_remove_post_send(TSend*);
        void private_remove_pre_send(TSend*);
        void private_add_input_bus(AudioBus*);
        void private_remove_input_bus(AudioBus*);

signals:
        void soloChanged(bool isSolo);
	void preSendChanged(bool preSendOn);
	void automationVisibilityChanged();
        void routingConfigurationChanged();
};

#endif // TRACK_H
