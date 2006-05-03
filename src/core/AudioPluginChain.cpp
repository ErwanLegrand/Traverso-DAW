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
 
    $Id: AudioPluginChain.cpp,v 1.3 2006/05/03 11:59:39 r_sijrier Exp $
*/

#include "AudioPluginChain.h"

#include <QPainter>

#include "AudioPluginController.h"
#include "ColorManager.h"
#include "Track.h"
#include "Song.h"
#include "Project.h"
#include "Curve.h"
#include "CurveNode.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

AudioPluginChain::AudioPluginChain(Track* pAssocTrack)
{
        PENTERCONS;
        for (int k=0; k<MAX_CONTROLLERS; k++)
                pluginController[k] = (AudioPluginController*) 0;
        currentAudioPluginController=-1;
        assocTrack=pAssocTrack;
        draggingNode = false;
        isSelectingController = false;
}


AudioPluginChain::~AudioPluginChain()
{
        PENTERDES;
        for (int k=0; k<MAX_CONTROLLERS; k++)
                if (pluginController[k])
                        delete pluginController[k];
}

void AudioPluginChain::draw()
{
        PENTER4;
        /*	int baseY = assocTrack->real_baseY();
        	int height = assocTrack->get_height();
        	QPainter painter(iface->get_viewport());
         
        	if (assocTrack->is_active())
        		{
        		ColorManager::set_blur(ColorManager::CURVE_ACTIVE , false);
        		ColorManager::set_blur(ColorManager::FILTERCONTROLLER_ACTIVE, false);
        		ColorManager::set_blur(ColorManager::CURVE_NONACTIVE , false);
        		ColorManager::set_blur(ColorManager::FILTERCONTROLLER_NONACTIVE, false);
        		}
        	else
        		{
        		ColorManager::set_blur(ColorManager::CURVE_ACTIVE , true);
        		ColorManager::set_blur(ColorManager::FILTERCONTROLLER_ACTIVE, true);
        		ColorManager::set_blur(ColorManager::CURVE_NONACTIVE , true);
        		ColorManager::set_blur(ColorManager::FILTERCONTROLLER_NONACTIVE, true);
        		}
         
        	// draw the controllers queue
        	painter.setPen(QColor(0,0,100));
        	painter.setFont(QFont("Fixed",6));
        	int by = baseY + height - 25;
        	int y1 = by + 6;
        	QColor cf;
        	int j=0;
        	for (int k=0; k<MAX_CONTROLLERS; k++)
        		{
        		if (pluginController[k])
        			{
        			int bx = 15 + j*80;
        			int x1 = bx - 10;
        			painter.setPen(Qt::black);
        // 			painter.moveTo(x1,   y1);
        // 			painter.lineTo(x1+5, y1);
        // 			painter.lineTo(x1+5, y1-4);
        // 			painter.lineTo(x1+8, y1+4);
        // 			painter.lineTo(x1+5, y1+11);
        // 			painter.lineTo(x1+5, y1+7);
        // 			painter.lineTo(x1,   y1+7);
         
        			QString filterType = pluginController[k]->get_type();
        			if (pluginController[k]->get_audio_plugin()->is_bypassed())
        				cf = cm().get(FILTERCONTROLLER_NONACTIVE);
        			else
        				{
        				if (k==currentAudioPluginController)
        					{
        					pluginController[k]->get_current_curve()->activate();
        					cf = cm().get(FILTERCONTROLLER_ACTIVE);
        					}
        				else
        					{
        					pluginController[k]->get_current_curve()->deactivate();
        					cf = cm().get(FILTERCONTROLLER_NONACTIVE);
        					}
        				}
        			QString currentCurveName = pluginController[k]->get_current_curve()->get_type();
        			painter.fillRect(bx,   by,    70, 19, cf);
        			painter.setPen(Qt::black);
        			painter.drawRect(bx,   by,    70, 19);
        			painter.drawText(bx+3, by+9, filterType);
        			painter.drawText(bx+3, by+17, currentCurveName);
        			j++;
        			}
        		}
         
        	for (int k=0; k<MAX_CONTROLLERS; k++)
        		{
        		if (pluginController[k])
        			pluginController[k]->draw();
        		}*/
}


AudioPluginController* AudioPluginChain::add_audio_plugin_controller(QString )
{
        PENTER;
        return 0;// FIXME
        /*
        int fcIndex=-1;

        for (int k=0; k<MAX_CONTROLLERS; k++)
        	{
        	if (!pluginController[k])
        		{
        		fcIndex=k;
        //			assocTrack->parentSong->recreate_clip_area();
        		break;
        		}
        	}
        if (fcIndex<0)
        	{
        	PERROR("Max number of filter controllers reached for track %s", (const char*) assocTrack->get_name().latin1());
        	return (AudioPluginController*) 0;
        	}
        PMESG("Allocating %s controller in slot %d",(const char*) filterName.latin1() ,fcIndex);
        if (filterName=="")
        	{
        	assocTrack->parentSong->parentProject->parentInterface->info("Ok... no filter added :-/",1);
        	return (AudioPluginController*) 0;
        	}
        if (filterName=="Gain")
        	{
        	pluginController[fcIndex]=new GainController(this);
        	}
        else
        if (filterName=="Pan")
        	{
        	pluginController[fcIndex]=new PanController(this);
        	}
        else
        if (filterName=="Noise Gate")
        	{
        	pluginController[fcIndex]=new NoiseGateController(this);
        	}
        else
        if (filterName=="Delay")
        	{
        	pluginController[fcIndex]=new DelayController(this);
        	}
        //else if (filterName=="...
        else
        	{
        	assocTrack->parentSong->parentProject->parentInterface->info("Filter/Controller not implemented yet");
        	return (AudioPluginController*) 0;
        	}
        pluginController[fcIndex]->setupFrontend->setCaption(pluginController[fcIndex]->get_type()+" Controller Setup");
        set_current_audio_plugin_controller(fcIndex);
        assocTrack->recreate(); // should be recreate_mta
        return pluginController[currentAudioPluginController];
        */
}


int AudioPluginChain::set_current_audio_plugin_controller(AudioPluginController* fc)
{
        PENTER2;
        for (int k=0; k<MAX_CONTROLLERS; k++)
                if (pluginController[k] && pluginController[k]==fc) {
                        return set_current_audio_plugin_controller(k);
                }
        return -1;
}


int AudioPluginChain::set_current_audio_plugin_controller(int index)
{
        PENTER2;
        if ((index>=0) && (index<MAX_CONTROLLERS) && (pluginController[index])) {
                for (int k=0; k<MAX_CONTROLLERS; k++)
                        if (pluginController[k])
                                pluginController[k]->deactivate();
                currentAudioPluginController = index;
                /*		if (ie().get_current_mode() ==  assocTrack->get_song()->CurveMode)
                			pluginController[currentAudioPluginController]->activate();*/
                return 1;
        }
        return -1;
}


void AudioPluginChain::select_controller(bool isBeginning)
{
        isSelectingController = isBeginning;
}




int AudioPluginChain::remove_controller(AudioPluginController* fc)
{
        PENTER2;
        for (int k=0; k<MAX_CONTROLLERS; k++)
                if (pluginController[k] && pluginController[k]==fc) {
                        return remove_controller(k);
                }
        return -1;
}


int AudioPluginChain::remove_controller(int index)
{
        PENTER2;
        if (index==-1)
                index=currentAudioPluginController;
        if (currentAudioPluginController>=0) {
                delete (pluginController[currentAudioPluginController]);
                pluginController[currentAudioPluginController] = (AudioPluginController*) 0;
                currentAudioPluginController=-1;
                for (int k=0; k<MAX_CONTROLLERS; k++) {
                        if (pluginController[k]) {
                                currentAudioPluginController=k;
                                pluginController[currentAudioPluginController]->activate();
                                break;
                        }
                }
                // FIXME
                // 		assocTrack->recreate_clip_area();
                return 1;
        }
        return -1;
}


void AudioPluginChain::followMouse(int x, int y)
{
        PENTER4;
        if (isSelectingController) {
                int ncs=0;
                for (int k=0; k<MAX_CONTROLLERS; k++)
                        if (pluginController[k])
                                ncs++;
                if (ncs==0) {
                        return;
                }
                int index = 100;// FIXME :-) x/(assocTrack->get_width()/ncs) + 1;
                int i=0;
                for (int k=0; k<MAX_CONTROLLERS; k++) {
                        if (pluginController[k])
                                i++;
                        if (i==index) {
                                set_current_audio_plugin_controller(k);
                                break;
                        }
                }
                // FIXME
                // 		assocTrack->recreate_clip_area();
                return;
        }

        // NODE/CURVE MANIPULATION
        // 	if (
        // 		(currentAudioPluginController<0) ||
        // 		(ie().get_current_mode() == assocTrack->get_song()->CurveMode) ||
        // 		((!draggingNode) && (!assocTrack->is_pointed(y)))
        // 	    )
        // 	    	{
        // 			return;
        // 			}


        // 	nframes_t lpos = assocTrack->get_song()->xpos_to_frame(x);

        // 	float cv = pluginController[currentAudioPluginController]->get_current_curve()->get_value_at(lpos);

        /* improveme
        Lcd* lcd = assocTrack->parentSong->parentProject->get_default_lcd();
        lcd->clear(550,100,100,20);
        p->setPen(Qt::black);
        QString scv; scv.setNum(cv);
        p->setFont(QFont("Fixed",8));
        p->drawText(560,100,scv);
        lcd->update(550,100,100,20);
        */

        if (draggingNode) {
                CurveNode* cn = pluginController[currentAudioPluginController]->get_current_curve()->get_current_curve_node();
                if (cn->prev) // This is to avoid changing the root node position
                {
                        nframes_t pos = assocTrack->get_song()->xpos_to_frame( x - originX ) + originPos;
                        if(cn->prev->pos > pos)
                                pos=cn->prev->pos + 4;
                        if (cn->next)
                                if(cn->next->pos < pos)
                                        pos=cn->next->pos - 4;
                        cn->set_pos(pos);
                }
                int half = assocTrack->get_height()/2;
                int dy = y - originY;
                float variation = -100.0 * ( ( (float) ( dy - half) / half ) + 1.0 );
                float v = originValue + variation;
                cn->set_value(v);
                // FIXME
                // 		assocTrack->recreate_clip_area();
        } else if (assocTrack->is_pointed(y))
                pluginController[currentAudioPluginController]->update_current_node(x);
}

void AudioPluginChain::drag_node(bool isBeginning)
{
        draggingNode = isBeginning;
        if ((draggingNode) && (currentAudioPluginController>=0)) {
                originX = cpointer().x(); // assocTrack->get_mouse_x();
                originY = cpointer().y(); //assocTrack->get_mouse_y();
                originPos   = pluginController[currentAudioPluginController]->get_current_curve()->get_current_curve_node()->pos;
                originValue = pluginController[currentAudioPluginController]->get_current_curve()->get_current_curve_node()->value;
        }
}


void AudioPluginChain::add_node()
{
        PENTER3;
        if (currentAudioPluginController>=0) {
                int x = cpointer().x();//assocTrack->get_mouse_x();
                int y = cpointer().y();//assocTrack->get_mouse_y();
                nframes_t pos  = assocTrack->get_song()->xpos_to_frame(x);
                int half = assocTrack->get_height()/2;
                float value = -100.0 * ((float) ((y - assocTrack->get_baseY()) - half) / half );
                pluginController[currentAudioPluginController]->get_current_curve()->add_node(pos, value);
                // FIXME
                // 		assocTrack->recreate_clip_area();
        }
}


void AudioPluginChain::node_setup()
{
        if (currentAudioPluginController>=0) {
                pluginController[currentAudioPluginController]->node_setup();
                // FIXME
                // 		assocTrack->recreate_clip_area();
        }
}


void AudioPluginChain::audio_plugin_setup()
{
        if (currentAudioPluginController>=0) {
                pluginController[currentAudioPluginController]->setup();
                // FIXME
                // 		assocTrack->recreate_clip_area();
        }
}

int AudioPluginChain::process(char* data, int size)
{
        PENTER4;
        for (int k=0; k<MAX_CONTROLLERS; k++) {
                if (pluginController[k])
                        pluginController[k]->process(data,size);
        }
        return 1;
}

int AudioPluginChain::cleanup()
{
        for (int k=0; k<MAX_CONTROLLERS; k++) {
                if (pluginController[k])
                        pluginController[k]->cleanup();
        }
        return 1;
}


void AudioPluginChain::activate()
{
        PENTER2;
        if (currentAudioPluginController>=0)
                pluginController[currentAudioPluginController]->activate();
        // maybe some things more here ...
}


void AudioPluginChain::deactivate()
{
        PENTER2;
        if (currentAudioPluginController>=0)
                pluginController[currentAudioPluginController]->deactivate();
}



QString AudioPluginChain::get_schema()
{
        PENTER;
        QString schema="";
        for (int k=0; k<MAX_CONTROLLERS; k++) {
                if (pluginController[k]) {
                        schema.append("       <filtercontroller type=\""+pluginController[k]->get_type()+"\">\n");
                        schema.append(pluginController[k]->get_schema());
                        schema.append("       </filtercontroller>\n");
                }
        }
        return schema;
}

//eof
