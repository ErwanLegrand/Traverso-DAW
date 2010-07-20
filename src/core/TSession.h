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

#include <QDomNode>
#include "APILinkedList.h"
#include "defines.h"

class AudioTrack;
class SnapList;
class Snappable;
class TBusTrack;
class Track;
class TimeLine;

class TSession : public ContextItem
{
        Q_OBJECT
public:
        TSession(TSession* parentSession = 0);

        QDomNode get_state(QDomDocument doc);
        int set_state( const QDomNode & node );


        qreal get_hzoom() const;
        QPoint get_scrollbar_xy();
        int get_mode() const {return m_mode;}
        int is_transport_rolling() const;
        TimeRef get_work_location() const;
        virtual TimeRef get_last_location() const;
        TimeRef get_new_transport_location() const {return m_newTransportLocation;}
        virtual TimeRef get_transport_location() const;
        virtual SnapList* get_snap_list() const;
        Track* get_track(qint64 id) const;
        TimeLine* get_timeline() const;
        TSession* get_parent_session() const {return m_parentSession;}
        QString get_name() const {return m_name;}
        int get_track_height(qint64 trackId) const {return m_trackHeights.value(trackId, 150);}

        TBusTrack* get_master_out() const;
        virtual QList<Track*> get_tracks() const;
        QList<TBusTrack*> get_bus_tracks() const;
        QList<TSession*> get_child_sessions() const {return m_childSessions;}
        Snappable* get_work_snap() const;
        virtual bool is_snap_on() const	{return m_isSnapOn;}


        void set_hzoom(qreal hzoom);
        virtual void set_work_at(TimeRef location, bool isFolder=false);
        void set_scrollbar_xy(int x, int y);
        void set_scrollbar_x(int x);
        void set_scrollbar_y(int y);
        void set_parent_session(TSession* parentSession);
        void set_is_project_session(bool isProjectSession) {m_isProjectSession = isProjectSession;}
        void set_name(const QString& name);
        void set_track_height(qint64 trackId, int height) {m_trackHeights.insert(trackId, height);}

        Command* add_track(Track* api, bool historable=true);
        Command* remove_track(Track* api, bool historable=true);

        void add_child_session(TSession* child);
        void remove_child_session(TSession* child);

        audio_sample_t* 	mixdown;
        audio_sample_t*		gainbuffer;

        enum Mode {
                EDIT = 1,
                EFFECTS = 2
        };

protected:
        TSession*               m_parentSession;
        QList<TSession*>        m_childSessions;
        APILinkedList           m_rtAudioTracks;
        APILinkedList           m_rtBusTracks;
        QList<AudioTrack*>      m_audioTracks;
        QList<TBusTrack*>       m_busTracks;
        TBusTrack*              m_masterOut;
        QHash<qint64, int>      m_trackHeights;

        SnapList*	m_snaplist;
        Snappable*	m_workSnap;
        TimeLine*	m_timeline;
        QString         m_name;

        int		m_mode;
        int		m_sbx;
        int		m_sby;
        qreal		m_hzoom;
        bool 		m_isSnapOn;
        bool            m_isProjectSession;

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
        virtual Command* start_transport();

protected slots:
        void private_add_track(Track* track);
        void private_remove_track(Track* track);
        void private_track_added(Track* track);
        void private_track_removed(Track* track);


signals:
        void privateTrackRemoved(Track*);
        void privateTrackAdded(Track*);
        void trackRemoved(Track* );
        void trackAdded(Track* );
        void sessionAdded(TSession*);
        void sessionRemoved(TSession*);
        void hzoomChanged();
        void tempFollowChanged(bool state);
        void lastFramePositionChanged();
        void modeChanged();
        void transportStarted();
        void transportStopped();
        void workingPosChanged();
        void transportPosSet();
        void horizontalScrollBarValueChanged();
        void verticalScrollBarValueChanged();
        void propertyChanged();
};

#endif // TSESSION_H
