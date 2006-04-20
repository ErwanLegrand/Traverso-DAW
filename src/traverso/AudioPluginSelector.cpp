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
 
    $Id: AudioPluginSelector.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "AudioPluginSelector.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



QString AudioPluginSelector::filterType[MAX_FILTER_TYPES] = {"CANCEL","Gain","Pan","Noise Gate","Reverb","AmpSim","Delay","Compressor","Multiband Compressor","Parametric EQ","Paragraphic EQ","Multiband EQ","Chorus","Flanger","Octaver","Pitch Shifter","Stereo Expansion","Phase Inversor"};

// Note : The filters DCCorrection, Pitch Scaling (Sharp Sing)
// HissKiller, HumKiller, GlitchKiller, Normalizer are allowed only
// in Audio Correction Session, so they dont appear in this selector.


AudioPluginSelector::AudioPluginSelector()
{
        updateTimer = new QTimer(this);
        connect( updateTimer , SIGNAL(timeout()), this, SLOT(update_selection()));
}

AudioPluginSelector::~AudioPluginSelector()
{}

void AudioPluginSelector::show_list()
{
        rootx = cpointer().x();//drawArea->get_mouse_x();
        rooty = cpointer().y();//drawArea->get_mouse_y();
        rooty = rooty<0?0:rooty;
        windowWidth = 1000;//drawArea->width();
        windowHeight = 800;//drawArea->height();
        if (rootx+MENU_W>windowWidth-10)
                rootx=rootx-MENU_W;
        if (rooty+MENU_H>windowHeight-10)
                rooty=rooty-MENU_H;
        /*        drawArea->painter->fillRect(rootx,rooty,MENU_W,MENU_H,Qt::white);
                drawArea->painter->setPen(QColor(0,20,40));*/
        //         for (int i=0; i<5; i++)
        //                 drawArea->painter->drawRect(rootx+i,rooty+i,MENU_W-i*2,MENU_H-i*2);

        //         for (int i=0; i<5; i++)
        //                 drawArea->painter->drawLine(rootx,rooty+25+i,rootx+MENU_W,rooty+25+i);

        //         drawArea->painter->setFont( QFont( "Helvetica", 8 ) );
        //         drawArea->painter->drawText(rootx+20,rooty+20,"Select a Filter Controller");
        //
        //         drawArea->painter->setPen(QColor(0,0,0));
        //         for (int i=0; i<MAX_FILTER_TYPES; i++)
        //                 drawArea->painter->drawText(rootx+12,rooty+45+i*ROW_HEIGHT,filterType[i]);

        //         drawArea->update(rootx,rooty,MENU_W,MENU_H);
}


void AudioPluginSelector::start()
{
        PENTER;
        rootx=-1;
        rooty=-1;
        windowWidth=-1;
        windowHeight=-1;
        lastPointedLine=-1;
        colorChangeVal=0;
        colorChangeFactor=1;
        show_list();
        updateTimer->start(50);
}

void AudioPluginSelector::stop()
{
        PENTER;
        updateTimer->stop();
        //         drawArea->recreate();
}

void AudioPluginSelector::update_selection()
{
        int y = 8;//drawArea->get_mouse_y();
        int pointedLine = ((y+5-rooty)/ROW_HEIGHT)-3;
        if ((pointedLine<0) || (pointedLine>MAX_FILTER_TYPES-1))
                return;
        //         int cx = rootx+5;
        //         int cy = rooty+pointedLine*ROW_HEIGHT+35;
        if (pointedLine!=lastPointedLine) {
                if ((lastPointedLine>=0) && (lastPointedLine<MAX_FILTER_TYPES)) {
                        //                         int lcy = rooty+lastPointedLine*ROW_HEIGHT+35;
                        /*                        drawArea->painter->fillRect(cx,lcy,MENU_W-10,ROW_HEIGHT, QColor(255,255,255));
                                                drawArea->painter->setPen(QColor(0,0,0));
                                                drawArea->painter->drawText(rootx+12,rooty+46+lastPointedLine*ROW_HEIGHT,filterType[lastPointedLine]);*/
                        //                         drawArea->update(cx,lcy,MENU_W-10,ROW_HEIGHT);
                }
                lastPointedLine=pointedLine;
        }
        /*        drawArea->painter->fillRect(cx,cy,MENU_W-10,ROW_HEIGHT, QColor(255,colorChangeVal,colorChangeVal));
                drawArea->painter->setPen(QColor(0,0,0));
                drawArea->painter->drawText(rootx+12,rooty+46+pointedLine*ROW_HEIGHT,filterType[pointedLine]);*/
        //         drawArea->update(cx,cy,MENU_W-10,ROW_HEIGHT);

        colorChangeVal+=colorChangeFactor;
        if (colorChangeVal>240)
                colorChangeFactor=-20;
        if (colorChangeVal<60)
                colorChangeFactor=20;
}

QString AudioPluginSelector::get_selected_filter_type()
{
        if (lastPointedLine==0)
                return "";
        if ((lastPointedLine>0) && (lastPointedLine<MAX_FILTER_TYPES)) {
                return filterType[lastPointedLine];
        } else
                return "";
}


//eof
