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
 
    $Id: CurveNode.cpp,v 1.3 2006/05/03 11:59:39 r_sijrier Exp $
*/

#include "CurveNode.h"
#include "Curve.h"
#include "AudioPluginController.h"
#include "Track.h"
#include "Song.h"
#include "AudioPluginChain.h"

#include <QWidget>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


CurveNode::CurveNode(Curve* pParentCurve, nframes_t pPos, float  pValue)
{
        PENTERCONS;
        parentCurve = pParentCurve;
        set_pos(pPos);
        set_value(pValue);
        isHighLighted=false;
}

CurveNode::~CurveNode()
{
        PENTERDES;
}

void CurveNode::toggle_highlight()
{
        isHighLighted=!isHighLighted;
        draw();
}

void CurveNode::set_pos(nframes_t pPos)
{
        if (pPos<0)
                pPos = 0;
        pos = pPos;
}


void CurveNode::set_value(float pValue)
{
        // (LG) Standard range : -100.0 to + 100.0
        // Each filter "converts" this range to a meanfull value
        // but all them use it as % or track height, when drawing.
        // For example. +100 means the top level, always.
        // -100 means the botton level, always.
        // But, for different filters,the meaning is different depending
        // on What the curve means:
        // in a Gain filter, in its Wet curve, 100 means + 25 dB of gain,
        // and -100 means -inf (mute) gain.
        // In a reverb, 100 mean "all wet", 0 means 50%/50% and -100 means
        // "all dry".
        // So, each filter is responsible to interpret this value.
        if (pValue>100.0)
                pValue=100.0;
        if (pValue<-100.0)
                pValue=-100.0;
        value=pValue;
}



void CurveNode::draw()
{
        /*	PENTER4;
        		AudioPluginController* fc = parentCurve->assocAudioPluginController;
        		Widget* drawArea = fc->get_drawarea();
        		QPainter* painter = drawArea->painter;
        	Track* parentTrack = fc->parentChain->assocTrack;
         
        		const int NODE_WIDTH=3;
         
        		if (this->isHighLighted)
        				painter->setPen(QColor(0, 255, 0));
         
        		else
        				painter->setPen(QColor(0,0,0));
         
        		int baseY = parentTrack->get_baseY();
        		int half = parentTrack->get_height()/2;
        		int yc = baseY + half;
        		int x = parentTrack->get_song()->frame_to_xpos(pos);// + iface->get_songview()->cliparea_basex();
        		int y = yc + (int) (-1*value*half/100);
         
         
        		// MODE SUPERMARKERS
        		/*for (int k=-NODE_WIDTH; k<NODE_WIDTH; k++)
        						{
        						painter->drawLine(x+k, baseY, x, y);
        						painter->drawLine(x+k, floorY, x, y);
        						}
        		*/

        // MODE CLASSICAL NODE
        /*		int x1=x-(NODE_WIDTH/2);
        		int y1=y-(NODE_WIDTH/2);
        		painter->drawRect(x1,y1,NODE_WIDTH,NODE_WIDTH);*/
        // 		drawArea->update(x1,y1,NODE_WIDTH,NODE_WIDTH);*/
}

void CurveNode::setup()
{
        parentCurve->assocAudioPluginController->setupFrontend->show();
}


//eof
