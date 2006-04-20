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
 
    $Id: Track.cpp,v 1.1 2006/04/20 14:51:40 r_sijrier Exp $
*/

#include <libtraverso.h>

#include <QTextStream>
#include <QString>
#include "Track.h"
#include "Song.h"
#include "AudioSource.h"
#include "AudioClip.h"
#include "AudioSourcesList.h"
#include "Mixer.h"
#include "AudioPluginController.h"
#include "AudioPluginChain.h"
#include "ColorManager.h"
#include "MtaRegion.h"
#include "MtaRegionList.h"
#include "Curve.h"
#include "CurveNode.h"
#include "Information.h"
#include "ContextItem.h"
#include "AudioChannel.h"
#include "AudioBus.h"

#include <commands.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



Track::Track(Song* song, int pID, QString pName, int pBaseY, int pHeight )
                : ContextItem(), m_song(song), ID(pID), name(pName),
                baseY(pBaseY)
{
        PENTERCONS;
        isActive = isMuted = isSolo = isArmed = false;
        m_pan = 0;
        busIn = "Not Set";
        busOut = "MasterOut";
        m_gain = 0.5;
        height = pHeight;
        init();

        emit m_song->trackCreated(this);
}

Track::Track( Song * song, const QDomNode node )
                : ContextItem(), m_song(song)
{
        PENTERCONS;
        isArmed=false;
        isActive = false;
        // These are use by TrackView _before_ they are initialized by set_state !!
        baseY = height = 0;
        // ALWAYS call init() before new member objects are created!
        init();
        emit m_song->trackCreated(this);
        set_state(node);
}


Track::~Track()
{
        PENTERDES;
        delete audioPluginChain;
        while (!audioClipList.isEmpty())
                delete audioClipList.takeFirst();
}

void Track::init()
{
        set_history_stack(m_song->get_history_stack());
        audioPluginChain = new AudioPluginChain(this);
        connect(m_song, SIGNAL( transferStarted() ), this, SLOT (init_recording() ));
}

QDomNode Track::get_state( QDomDocument doc )
{
        QDomElement node = doc.createElement("Track");
        node.setAttribute("id", ID);
        node.setAttribute("name", name);
        node.setAttribute("gain", m_gain);
        node.setAttribute("pan", m_pan);
        node.setAttribute("mute", isMuted);
        node.setAttribute("solo", isSolo);
        node.setAttribute("height", height);
        node.setAttribute("InBus", busIn.data());
        node.setAttribute("OutBus", busOut.data());

        QDomNode clips = doc.createElement("Clips");
        foreach(AudioClip* clip, audioClipList) {
                clips.appendChild(clip->get_state(doc));
        }
        node.appendChild(clips);
        return node;
}


int Track::set_state( const QDomNode & node )
{
        QDomElement e = node.toElement();
        name = e.attribute( "name", "" );
        set_muted(e.attribute( "mute", "" ).toInt());
        set_solo( e.attribute( "solo", "" ).toInt() );
        m_gain =  e.attribute( "gain", "" ).toFloat();
        set_pan( e.attribute( "pan", "" ).toFloat() );
        set_bus_in( e.attribute( "InBus", "" ).toAscii() );
        set_bus_out( e.attribute( "OutBus", "" ).toAscii() );
        set_height( e.attribute( "height", "160" ).toInt() );
        ID = e.attribute( "id", "").toInt();

        QDomElement ClipsNode = node.firstChildElement("Clips");
        if (!ClipsNode.isNull()) {
                QDomNode ClipNode = ClipsNode.firstChild();
                while (!ClipNode.isNull()) {
                        QDomElement e = ClipNode.toElement();
                        AudioClip* clip = new AudioClip(this, ClipNode);
                        add_clip( clip );
                        ClipNode = ClipNode.nextSibling();
                }
        }

        return 1;
}


bool Track::is_pointed(int y)
{
        return ( (y >= real_baseY()) && (y <= real_baseY() + height) );
}


QList<AudioClip* > Track::split_clip(nframes_t splitPoint)
{
        PENTER2;

        QList<AudioClip* > clipPair;

        PMESG("Trying to find clip under %ld", (long)splitPoint);
        AudioClip* c = get_clip_under(splitPoint);

        if (c)
                clipPair = split_clip(c, splitPoint);
        else
                PMESG("I'm uppon no clip :-(");

        return clipPair;
}


QList<AudioClip* > Track::split_clip(AudioClip* clip, nframes_t splitPoint)
{
        PENTER2;
        QList<AudioClip* > clipPair;

        nframes_t leftLenght = splitPoint - (clip->get_track_first_block());
        nframes_t rightSourceFirstBlock = clip->get_source_first_block() + leftLenght;

        AudioClip* leftClip = clip->create_copy();
        leftClip->set_last_source_block(rightSourceFirstBlock);
        clipPair.append(leftClip);

        AudioClip* rightClip = clip->create_copy();
        rightClip->set_first_source_block( rightSourceFirstBlock );
        rightClip->set_track_first_block( splitPoint );
        clipPair.append(rightClip);

        return clipPair;
}


AudioClip* Track::get_clip_under(nframes_t blockPos)
{
        AudioClip* clip = (AudioClip*) 0;
        for (int i=0; i < audioClipList.size(); ++i) {
                clip = audioClipList.at(i);
                if ((clip->get_track_first_block() <= blockPos) && (clip->get_track_last_block() >= blockPos))
                        return clip;
                if (clip->get_track_first_block() > blockPos)
                        break;
        }
        return clip;
}


AudioClip* Track::get_clip_between(nframes_t , nframes_t )
{
        PENTER4;
        AudioClip* theClip = (AudioClip*) 0;
        return theClip;
}


AudioClip* Track::get_clip_after(nframes_t blockPos)
{
        AudioClip* clip;
        for (int i=0; i < audioClipList.size(); ++i) {
                clip = audioClipList.at(i);
                if (clip->get_track_first_block() > blockPos)
                        return clip;
        }
        return (AudioClip*) 0;
}



AudioClip* Track::get_clip_before(nframes_t blockPos)
{
        AudioClip* clip;
        for (int i=0; i < audioClipList.size(); ++i) {
                clip = audioClipList.at(i);
                if (clip->get_track_first_block() < blockPos)
                        return clip;
        }
        return (AudioClip*) 0;
}



//FIXME needs to be reviewed
int Track::delete_clip(AudioClip* clip, bool permanently)
{
        PENTER;
        if (!clip) {
                PERROR("Trying to delete invalid Clip");
                return -1;
        }
        if(permanently && (m_song->get_clips_count_for_audio(clip->get_audio_source()) == 1))//FIXME this clip is the only one with this audioSource,
                clip->get_audio_source()->get_peak()->free_buffer_memory();					//so we have to unbuild the peak RAMBuffers... or not? Waste of memory usage if we keep it there...


        return 1;
}


int Track::remove_clip(AudioClip* clip)
{
        PENTER;
        if (audioClipList.remove_clip(clip)) {
                emit audioClipRemoved(clip);
                return 1;
        }
        return -1;
}


void Track::add_clip(AudioClip* clip)
{
        PENTER;
        clip->set_track(this);
        audioClipList.add_clip(clip);
        emit audioClipAdded(clip);
}


void Track::activate()
{
        if (!isActive) {
                isActive=true;
                /*		if (m_song->editingMode==Song::EDIT_TRACK_CURVES)
                			{
                			int cfc  = audioPluginChain->currentAudioPluginController;
                			if ((cfc>=0) && (audioPluginChain->pluginController[cfc]))
                				audioPluginChain->pluginController[cfc]->start_blinking();
                			}*/
        }
}


void Track::deactivate()
{
        if (isActive) {
                for (int k=0; k<AudioPluginChain::MAX_CONTROLLERS; k++)
                        if (audioPluginChain->pluginController[k])
                                audioPluginChain->pluginController[k]->deactivate();
                isActive=false;
        }
}


void Track::set_blur(bool )
{
        /*	ColorManager::set_blur(ColorManager::TRACK_BG , stat);
        	ColorManager::set_blur(ColorManager::TRACK_BG_ACTIVE, stat);
        	for (int i=0; i < audioClipList.size(); ++i)
        		audioClipList.at(i)->set_blur(stat);*/
}


int Track::arm()
{
        PENTER;
        set_armed(true);
        return 1;
}


int Track::disarm()
{
        PENTER;
        set_armed(false);
        return 1;
}

void Track::set_bus_in(QByteArray bus)
{
        bool wasArmed=isArmed;
        if (isArmed)
                disarm();
        busIn=bus;
        if (wasArmed)
                arm();
        emit stateChanged();
}

void Track::set_bus_out(QByteArray bus)
{
        busOut=bus;
        emit stateChanged();
}


bool Track::is_muted()
{
        return isMuted;
}

bool Track::is_solo()
{
        return isSolo;
}


bool Track::armed()
{
        return isArmed;
}

bool Track::is_active()
{
        return isActive;
}

void Track::toggle_active()
{
        isActive=!isActive;
        emit isActiveChanged(isActive);
}


void Track::init_recording()
{
        PENTER;
        if (isArmed) {
                QByteArray name = "Audio_" + QByteArray::number(ID) + "." + QByteArray::number(audioClipList.size() + 1);
                AudioClip* clip = new AudioClip(this, m_song->transport_frame, name);
                if (clip->init_recording(busIn) < 0) {
                        PERROR("Could not create AudioClip to record to!");
                        delete clip;
                } else {
                        add_clip( clip );
                }
        }
}


void Track::set_gain(float gain)
{
        if (gain < 0.0)
                gain = 0.0;
        if (gain > 2.0)
                gain = 2.0;
        m_gain = gain;
        emit gainChanged();
}


void Track::set_pan(float pan)
{
        if ( pan < -1.0 )
                m_pan=-1.0;
        else
                if ( pan > 1.0 )
                        m_pan=1.0;
                else
                        m_pan=pan;
        emit panChanged();
}


void Track::vzoom_in(int newBaseY)
{
        baseY = newBaseY;
        if (1.1*height<500.0)
                set_height((int) (1.10*height));
}


void Track::vzoom_out(int newBaseY)
{
        baseY = newBaseY;
        if ( 0.9 * height > MINIMUM_HEIGHT )
                set_height((int) (0.90 * height));
}


void Track::set_baseY(int b)
{
        baseY = b;

        // I know, this is not true, but who cares. It causes everything to be repainted!
        emit heightChanged();
}


void Track::set_height(int h)
{
        height = h;
        emit heightChanged();
}


int Track::get_total_clips()
{
        return audioClipList.count();
}


void Track::set_solo(bool solo)
{
        isSolo=solo;
        emit soloChanged(isSolo);
}

void Track::toggle_mute()
{
        set_muted(!isMuted);
}

void Track::set_muted( bool muted )
{
        isMuted = muted;
        emit muteChanged(isMuted);
}

void Track::set_armed( bool armed )
{
        isArmed = armed;
        emit armedChanged(isArmed);
}

int Track::real_baseY( ) const
{
        //FIXME!!!!
        return baseY;// + m_song->mtaBaseY;
}


int Track::process( nframes_t nframes )
{
        int processResult = 0;
        if (!isMuted) {
                foreach(AudioClip* clip, audioClipList) {
                        processResult |= clip->process(nframes);
                }
        } else {
                return 0;
        }

        return processResult;
}

Command* Track::mute()
{
        PENTER;
        toggle_mute();
        return (Command*) 0;
}

Command* Track::toggle_arm()
{
        if (isArmed)
                disarm();
        else
                arm();
        return (Command*) 0;
}

Command* Track::solo(  )
{
        m_song->solo_track(this);
        return (Command*) 0;
}

Command* Track::gain()
{
        return new TrackGain(this, m_song);
}

Command* Track::pan()
{
        return new TrackPan(this, m_song);
}

Command * Track::import_audiosource(  )
{
        return new Import(this);
}


// eof
