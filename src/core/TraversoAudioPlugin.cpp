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
 
    $Id: TraversoAudioPlugin.cpp,v 1.1 2006/04/20 14:51:40 r_sijrier Exp $
*/

#include "TraversoAudioPlugin.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

TraversoAudioPlugin::TraversoAudioPlugin()
{
        PENTERCONS;
        isBypassed=false;
        tail = (char*) 0;
        tailSize=0;
        parameterList=new QStringList();
        parameterList->append("WET_OUT");
        parameterList->append("DRY_OUT");
        parameterList->append("PRE_GAIN");
        parameterList->append("POS_GAIN");
        parameterList->append("PREVIEW_QUALITY");
}

TraversoAudioPlugin::~TraversoAudioPlugin()
{
        PENTERDES;
        if (tail)
                delete tail;
        delete parameterList;
}


void TraversoAudioPlugin::set_sample_rate(int pSampleRate)
{
        sampleRate = pSampleRate;
}


void TraversoAudioPlugin::set_channels(int pChannels)
{
        channels = pChannels;
}


void TraversoAudioPlugin::set_preview_quality(float value)
{
        previewQuality = value;
}

void TraversoAudioPlugin::set_wet_out(float value)
{
        wetOut = value;
}


void TraversoAudioPlugin::set_dry_out(float value)
{
        dryOut = value;
}

void TraversoAudioPlugin::set_pre_gain(float value)
{
        preGain = value;
}

void TraversoAudioPlugin::set_pos_gain(float value)
{
        posGain = value;
}

float TraversoAudioPlugin::get_preview_quality()
{
        return previewQuality;
}

float TraversoAudioPlugin::get_wet_out()
{
        return wetOut;
}

float TraversoAudioPlugin::get_dry_out()
{
        return dryOut;
}

float TraversoAudioPlugin::get_pre_gain()
{
        return preGain;
}

float TraversoAudioPlugin::get_pos_gain()
{
        return posGain;
}



void TraversoAudioPlugin::init_tail(int secs)
{
        tailSize = sampleRate*channels*secs;
        PMESG("Initializing Tail Buffer : size %d=",tailSize);
        tail=new char[tailSize];
        bzero(tail,tailSize);
}


void TraversoAudioPlugin::push_tail(int size)
{
        // push (shift) the tail left;
        memcpy(tail, tail+size, tailSize-size);
        bzero(tail+(tailSize-size),size);
}

int TraversoAudioPlugin::cleanup()
{
        bzero(tail,tailSize);
        return 1;
}

// eof
