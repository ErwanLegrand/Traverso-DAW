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
 
    $Id: Zoom.h,v 1.7 2008/01/22 20:47:16 r_sijrier Exp $
*/

#ifndef ZOOM_H
#define ZOOM_H

#include "TCommand.h"

#include <QVariantList>
#include <QPointF>

class SheetView;
class TrackView;
class QPoint;

class Zoom : public TCommand
{
	Q_OBJECT

public :
        Zoom(SheetView* sv, QVariantList args);
        ~Zoom() {}

        int begin_hold();
        int finish_hold();
        int prepare_actions();
	int do_action();
	int undo_action();

        int jog();

        void set_cursor_shape(int useX, int useY);
        void set_collected_number(const QString & collected);
	bool supportsEnterFinishesHold() const {return false;}

private :
        int m_horizontalJogZoomLastX;
        int m_verticalJogZoomLastY;
        int m_trackHeight;
	bool m_jogVertical;
	bool m_jogHorizontal;
	qreal m_xScalefactor;
	qreal m_yScalefactor;
        QPoint	m_mousePos;
        QPointF	m_origPos;

        int collected_number_to_track_height(const QString& collected) const;
	
        SheetView* m_sv;
        TrackView* m_tv;
	
public slots:
	void vzoom_in(bool autorepeat);
	void vzoom_out(bool autorepeat);
        void hzoom_in(bool autorepeat);
        void hzoom_out(bool autorepeat);
        void track_vzoom_in(bool autorepeat);
        void track_vzoom_out(bool autorepeat);
	void toggle_vertical_horizontal_jog_zoom(bool autorepeat);
        void toggle_expand_all_tracks(bool autorepeat);
};

#endif

