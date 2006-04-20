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
 
    $Id: AudioPluginChain.h,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#ifndef AUDIOPLUGINCHAIN_H
#define AUDIOPLUGINCHAIN_H

class QString;

#include "AudioPluginController.h"

class Track;

class AudioPluginChain
{

public:

        static const int MAX_CONTROLLERS = 20;
        AudioPluginChain(Track* pAssocTrack);
        ~AudioPluginChain();

        AudioPluginController* add_audio_plugin_controller(QString filterName);
        void select_controller(bool isBeginning);

        int remove_controller(int index = -1);
        int remove_controller(AudioPluginController* fc);
        int set_current_audio_plugin_controller(int index);
        int set_current_audio_plugin_controller(AudioPluginController* fc);


        void draw();
        void followMouse(int x, int y);
        void drag_node(bool isBeginning);
        void add_node();
        void node_setup();
        void audio_plugin_setup();
        int process(char* data, int size);

        void deactivate();
        void activate();
        int cleanup();

        QString get_schema();

        Track* assocTrack;

private:

        AudioPluginController* pluginController[MAX_CONTROLLERS];
        int currentAudioPluginController;

        bool draggingNode;

        int originX;
        int originY;
        nframes_t originPos;
        float originValue;

        bool isSelectingController;

        friend class Track;
        friend class AudioPluginController;
};



#endif
//eof
