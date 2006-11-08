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
 
    $Id: PanelLed.h,v 1.1 2006/11/08 14:45:22 r_sijrier Exp $
*/

#ifndef PANEL_LED_VIEW_H
#define PANEL_LED_VIEW_H

#include <QObject>
#include <QString>

class TrackPanelView;

class PanelLed : public QObject
{
        Q_OBJECT

public:
        PanelLed(TrackPanelView* view, int xpos, char* on, char* off);
        ~PanelLed();

	void paint(QPainter *painter);

        void set_on_type(char* type);
        void set_of_type(char* type);
        void set_xpos(int x);

private:
        int m_xpos;
        bool m_isOn;
        QString onType;
        QString offType;


public slots:
        void ison_changed(bool isOn);
};


#endif

//eof
