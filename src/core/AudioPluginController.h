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
 
    $Id: AudioPluginController.h,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#ifndef AUDIOPLUGINCONTROLLER_H
#define AUDIOPLUGINCONTROLLER_H

#include "libtraverso.h"

#include <QTimer>
#include <QWidget>

class Curve;
class CurveNode;
class AudioPluginChain;
class AudioPlugin;

class AudioPluginController : public QObject
{
        Q_OBJECT

public:

        static const int MAX_CURVES=10;

        AudioPluginController ( AudioPluginChain* chain );
        virtual ~AudioPluginController();

        virtual int process(char* data, int size);

        virtual void setup();
        virtual void node_setup();
        /*                virtual int get_baseX();
                        virtual int get_baseY();
                        virtual int get_width();
                        virtual int get_height();
                        virtual void set_baseX(int initialValue);
                        virtual void set_baseY(int initialValue);
                        virtual void set_width(int initialValue);
                        virtual void set_height(int initialValue);
        */
        virtual QString get_type();

        void draw();

        void update_current_node(int x);

        void activate();
        void deactivate();

        int cleanup();
        void start_blinking();
        void stop_blinking();

        Curve* add_curve(QString type);
        Curve* get_curve(QString curveType);
        Curve* get_current_curve();
        AudioPlugin* get_audio_plugin();

        QWidget* setupFrontend;

        QString get_schema();

        void set_current_curve(QString type);
        void set_current_curve(int index);

        int get_curve_index(Curve* c);

        AudioPluginChain* parentChain;

protected:

        int currentCurveIndex;
        QTimer* blinkCurveTimer;
        QTimer* blinkCurveNodeTimer;
        Curve* curve[MAX_CURVES];


        AudioPlugin* assocPlugin;

public slots:

        void blink_curve();
        void blink_curve_node();


};


#endif
