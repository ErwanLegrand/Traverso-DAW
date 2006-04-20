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
 
    $Id: TraversoAudioPlugin.h,v 1.1 2006/04/20 14:51:40 r_sijrier Exp $
*/

#ifndef TRAVERSOAUDIOPLUGIN_H
#define TRAVERSOAUDIOPLUGIN_H

#include <libtraverso.h>
#include <qstringlist.h>

#define SUCCESS 1
#define NOT_IMPLEMENTED 0
#define FAILURE -1

class TraversoAudioPlugin
{
public :


        TraversoAudioPlugin();
        virtual ~TraversoAudioPlugin();

        //! This function must return a constant char* with a unique ID for
        //! this plugin. This ID is provided by /Traverso team , after
        //! plugin code audition. (all plugins must be GPL)
        virtual const char* id() = 0;

        void set_sample_rate(int);
        void set_channels(int);
        void set_preview_quality(float);
        void set_wet_out(float);
        void set_dry_out(float);
        void set_pre_gain(float);
        void set_pos_gain(float);

        float get_preview_quality();
        float get_wet_out();
        float get_dry_out();
        float get_pre_gain();
        float get_pos_gain();

        //! Return a list of strings with all parameters names for this plugin. Traverso will create one control-curve for
        //! each parameter returned here, PLUS the standard curves : preview quality, wet_out, dry_out, pre_gain and pos_gain
        virtual QStringList* get_parameters() = 0 ;

        // set the parameter to the given value. Called MUST know all parameters, which can be
        // obtained by calling get_parameters()
        virtual int set_parameter(int parameter, float value) = 0 ;

        // get the parameter into a given float*. Called MUST know all parameters, which can be
        // obtained by calling get_parameters()
        virtual int get_parameter(int parameter, float* value) = 0 ;

        //! load audio data into the plugin.  Must be implemented by the plugin
        virtual int load_data( char* pAudioFragment, int pFragmentSize ) =0 ;

        //! make pre-calculation  Must be implemented by the plugin
        virtual int prepare() =0 ;

        //! process the effect. Must be implemented by the plugin
        virtual int process() =0 ;

        //! prepare the tail BEFORE the playback
        virtual void init_tail(int secs);

        //! let the tail influetiate the next chunk of audio
        virtual void push_tail(int size);

        //! When a STOP action is performed, reset the tail and make other proper adjustments.
        virtual int cleanup();

protected :

        QStringList* parameterList;
        bool isBypassed;
        int tailSize;
        char* tail;
        int fragmentSize;
        char* fragment;
        int sampleRate;
        int channels;
        float previewQuality;
        float wetOut;
        float dryOut;
        float preGain;
        float posGain;
};

#endif
