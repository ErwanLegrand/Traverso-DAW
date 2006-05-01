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
 
    $Id: AudioPluginController.cpp,v 1.2 2006/05/01 21:21:37 r_sijrier Exp $
*/

#include "AudioPluginController.h"
#include "Curve.h"
#include "CurveNode.h"
#include "AudioPluginChain.h"
#include "Song.h"
#include "Track.h"
#include <QWidget>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AudioPluginController::AudioPluginController ( AudioPluginChain* chain )
                : QObject()
{
        PENTERCONS;
        this->parentChain = chain;

        currentCurveIndex=-1;
        for (int i=0; i<MAX_CURVES; i++)
                curve[i]=(Curve*) 0;

        blinkCurveTimer = new QTimer(this);
        blinkCurveNodeTimer = new QTimer(this);

        connect( blinkCurveTimer , SIGNAL(timeout()), this, SLOT(blink_curve()));
        connect( blinkCurveNodeTimer , SIGNAL(timeout()), this, SLOT(blink_curve_node()));

        setupFrontend = new QWidget(0);
        setupFrontend->resize(300,200);
}

AudioPluginController::~AudioPluginController()
{
        PENTERDES;
}

void AudioPluginController::setup()
{
        PENTER2;
        setupFrontend->show();
}


void AudioPluginController::node_setup()
{
        PERROR("Ooops ! I should have not been called ... I am a virtual method!");
}


QString AudioPluginController::get_type()
{
        PERROR("Ooops ! I should have not been called ... I am a virtual method!");
        return "";
}


int AudioPluginController::process(char* , int )
{
        PERROR("Ooops ! I should have not been called ... I am a virtual method!");
        return -1;
}


void AudioPluginController::draw()
{
        curve[currentCurveIndex]->show();
}


void AudioPluginController::update_current_node(int x)
{
        PENTER4;
        nframes_t blockPos = parentChain->assocTrack->get_song()->xpos_to_block(x);
        CurveNode* lastnode = curve[currentCurveIndex]->currentNode;
        if (blockPos > 0) {
                CurveNode* cn = curve[currentCurveIndex]->get_nearest_node(blockPos);
                if (cn)
                        curve[currentCurveIndex]->currentNode=cn;
                if ((lastnode != curve[currentCurveIndex]->currentNode) && (lastnode->isHighLighted))
                        lastnode->toggle_highlight();
        }
}

void AudioPluginController::activate()
{
        PENTER2;
        start_blinking();
}


void AudioPluginController::deactivate()
{
        PENTER2;
        stop_blinking();
}


void AudioPluginController::blink_curve()
{
        //curve[currentCurveIndex]->...
}

void AudioPluginController::blink_curve_node()
{
        curve[currentCurveIndex]->currentNode->toggle_highlight();
}


void AudioPluginController::start_blinking()
{
        PENTER2;
        blinkCurveTimer->start(250);
        blinkCurveNodeTimer->start(250);
}


void AudioPluginController::stop_blinking()
{
        PENTER2;
        blinkCurveTimer->stop();
        blinkCurveNodeTimer->stop();
        if (curve[currentCurveIndex]->currentNode && (curve[currentCurveIndex]->currentNode->isHighLighted))
                curve[currentCurveIndex]->currentNode->toggle_highlight();
}



Curve* AudioPluginController::get_current_curve()
{
        return curve[currentCurveIndex];
}


void AudioPluginController::set_current_curve(QString type)
{
        PENTER3;
        for (int i=0; i<MAX_CURVES; i++) {
                if (curve[i] && (curve[i]->get_type()==type)) {
                        set_current_curve(i);
                        break;
                }
        }
}

void AudioPluginController::set_current_curve(int index)
{
        PENTER3;
        currentCurveIndex=index;
}


Curve*  AudioPluginController::get_curve(QString curveType)
{
        PENTER3;
        for (int i=0; i<MAX_CURVES; i++) {
                if ((curve[i]) && (curveType==curve[i]->get_type())) {
                        return curve[i];
                }
        }
        return (Curve*) 0;
}


int AudioPluginController::get_curve_index(Curve* c)
{
        PENTER3;
        for (int i=0; i<MAX_CURVES; i++) {
                if ((curve[i]) && (c==curve[i])) {
                        return i;
                }
        }
        return -1;
}


Curve* AudioPluginController::add_curve(QString type)
{
        PENTER2;
        int ind=-1;
        for (int i=0; i<MAX_CURVES; i++) {
                // cannot add a same audio plugin twice
                if (curve[i] && (curve[i]->get_type()==type)) {
                        return (Curve* )0;
                }
        }
        for (int i=0; i<MAX_CURVES; i++) {
                if (!curve[i]) {
                        ind=i;
                        break;
                }
        }
        if (ind>=0) {
                curve[ind]=new Curve(this,type);
                set_current_curve(ind);
                return curve[ind];
        }
        return (Curve*) 0;
}


AudioPlugin* AudioPluginController::get_audio_plugin()
{
        return assocPlugin;
}

int AudioPluginController::cleanup()
{
        assocPlugin->cleanup();
        return 1;
}

QString AudioPluginController::get_schema()
{
        PENTER2;
        QString schema="";
        for (int i=0; i<MAX_CURVES; i++) {
                if (curve[i]) {
                        schema=schema + "            <curve type=\"" + curve[i]->get_type()+"\">\n";
                        schema=schema + curve[i]->get_schema();
                        schema=schema + "            </curve>\n";
                }
        }
        return schema;
}

//eof
