/*
Copyright (C) 2010 Remon Sijrier

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

#ifndef TSESSION_H
#define TSESSION_H

#include "ContextItem.h"

#include "APILinkedList.h"
#include "defines.h"

class SnapList;
class Snappable;
class SubGroup;
class Track;
class TimeLine;

class TSession : public ContextItem
{
        Q_OBJECT
public:
        TSession(TSession* parentSession = 0);

        qreal get_hzoom() const {return m_hzoom;}
        void get_scrollbar_xy(int& x, int& y) {x = m_sbx; y = m_sby;}
        int get_mode() const {return m_mode;}
        int is_transport_rolling() const {return m_transport;}
        TimeRef get_work_location() const {return m_workLocation;}
        virtual TimeRef get_last_location() const = 0;
        TimeRef get_new_transport_location() const {return m_newTransportLocation;}
        TimeRef get_transport_location() const {return m_transportLocation;}
        virtual SnapList* get_snap_list() const;
        TimeLine* get_timeline() const {return m_timeline;}
        QString get_name() const {return m_name;}

        SubGroup* get_master_out() const {return m_masterOut;}
        virtual QList<Track*> get_tracks() const;
        QList<SubGroup*> get_subgroups() const;
        Snappable* get_work_snap() {return m_workSnap;}
        virtual bool is_snap_on() const	{return m_isSnapOn;}


        void set_hzoom(qreal hzoom);
        virtual void set_work_at(TimeRef location, bool isFolder=false);
        void set_scrollbar_xy(int x, int y) {m_sbx = x; m_sby = y;}

        Command* add_track(Track* api, bool historable=true);
        Command* remove_track(Track* api, bool historable=true);

        audio_sample_t* 	mixdown;
        audio_sample_t*		gainbuffer;

        enum Mode {
                EDIT = 1,
                EFFECTS = 2
        };

protected:
        TSession*       m_parentSession;
        SubGroup*       m_masterOut;
        APILinkedList   m_subGroups;
        SnapList*	m_snaplist;
        Snappable*	m_workSnap;
        TimeLine*	m_timeline;
        QString         m_name;

        int		m_mode;
        int		m_sbx;
        int		m_sby;
        qreal		m_hzoom;
        bool 		m_isSnapOn;

        volatile size_t		m_transport;
        TimeRef                 m_transportLocation;
        TimeRef                 m_workLocation;
        TimeRef                 m_newTransportLocation;

private:
        friend class TimeLine;

        void init();


public slots:
        void set_temp_follow_state(bool state);
        virtual void set_transport_pos(TimeRef location);

        Command* set_editing_mode();
        Command* set_effects_mode();
        Command* toggle_effects_mode();
        Command* start_transport();

protected slots:
        void private_add_track(Track* track);
        void private_remove_track(Track* track);


signals:
        void trackRemoved(Track* );
        void trackAdded(Track* );
        void hzoomChanged();
        void tempFollowChanged(bool state);
        void lastFramePositionChanged();
        void modeChanged();
        void transportStarted();
        void transportStopped();
        void workingPosChanged();
        void transportPosSet();


};

#endif // TSESSION_H
