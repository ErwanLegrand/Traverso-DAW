/*
    Copyright (C) 2005 Remon Sijrier 
 
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
 
    $Id: AudioPluginSelector.h,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#ifndef  AUDIOPLUGINSELECTOR_H
#define AUDIOPLUGINSELECTOR_H

#include <libtraverso.h>

class QTimer;

class AudioPluginSelector : public QObject
{
        Q_OBJECT

public:

        static const int MENU_W = 200;
        static const int MENU_H = 300;
        static const int ROW_HEIGHT = 14;
        static const int MAX_FILTER_TYPES=18;
        static QString filterType[MAX_FILTER_TYPES];

        AudioPluginSelector();
        ~AudioPluginSelector();
        void show_list();
        void start();
        void stop();
        QString get_selected_filter_type();

private:
        QTimer* updateTimer;
        int rootx;
        int rooty;
        int windowWidth;
        int windowHeight;
        int lastPointedLine;
        int colorChangeVal;
        int colorChangeFactor;

public slots:
        void update_selection();

};

#endif
