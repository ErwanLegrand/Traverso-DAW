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
 
    $Id: TrackView.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include <QPixmap>
#include <QPainter>
#include <QMenu>

#include "TrackView.h"
#include "AudioClipView.h"
#include "ColorManager.h"
#include "ViewPort.h"
#include "SongView.h"
#include "PanelLed.h"
#include "BusSelector.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

const int MUTE_LED_X = 10;
const int SOLO_LED_X = 48;
const int REC_LED_X = 86;
const int LOCK_LED_X = 124;

TrackView::TrackView(ViewPort* vp, SongView* parent, Track* track)
                : ViewItem(vp, parent, track), m_track(track), m_sv(parent)
{
        PENTERCONS2;
        panelPixmap = QPixmap(TRACKPANELWIDTH, 90);

        clipAreaWidth = m_vp->width() - CLIPAREABASEX;

        paintPanel = true;
        paintClipArea = true;

        busInMenu = new QMenu();
        busOutMenu = new QMenu();

        muteLed = new PanelLed(m_vp, this, MUTE_LED_X, ":/muteled_on", ":/muteled_off");
        soloLed = new PanelLed(m_vp, this, SOLO_LED_X, ":/sololed_on", ":/sololed_off");
        recLed = new PanelLed(m_vp, this, REC_LED_X, ":recled_on", ":/recled_off");
        lockLed = new PanelLed(m_vp, this, LOCK_LED_X, ":/lockled_on", ":/lockled_off");

        connect(m_vp, SIGNAL(resized()), this, SLOT(resize()));

        connect(m_sv->get_assocsong(), SIGNAL(hzoomChanged( )), this, SLOT(repaint_cliparea( )));
        connect (m_sv->get_assocsong(), SIGNAL(firstBlockChanged()), this, SLOT(repaint_cliparea()));

        connect(m_track, SIGNAL(audioClipAdded(AudioClip* )), this, SLOT(add_new_audioclipview(AudioClip* )));
        connect(m_track, SIGNAL(audioClipRemoved(AudioClip* )), this, SLOT(remove_audioclipview(AudioClip* )));
        connect(m_track, SIGNAL(heightChanged( )), this, SLOT(height_changed()));
        connect(m_track, SIGNAL(armedChanged(bool )), recLed, SLOT(ison_changed(bool )));
        connect(m_track, SIGNAL(soloChanged(bool )), soloLed, SLOT(ison_changed(bool )));
        connect(m_track, SIGNAL(muteChanged(bool )), muteLed, SLOT(ison_changed(bool )));
        connect(m_track, SIGNAL(gainChanged()), this, SLOT(panel_info_changed()));
        connect(m_track, SIGNAL(panChanged()), this, SLOT(panel_info_changed()));
        connect(m_track, SIGNAL(stateChanged()), this, SLOT(panel_info_changed()));

        connect(busInMenu, SIGNAL(triggered ( QAction* )), this, SLOT(set_bus_in( QAction* )));
        connect(busOutMenu, SIGNAL(triggered ( QAction* )), this, SLOT(set_bus_out( QAction* )));


        init_context_menu( this );
        m_type = TRACKVIEW;

}


TrackView::~TrackView()
{
        PENTERDES;
        delete muteLed;
        delete lockLed;
        delete soloLed;
        delete recLed;
        delete busInMenu;
        delete busOutMenu;
}


QRect TrackView::draw(QPainter& p)
{
        PENTER;


        if (paintPanel) {
                // Clear Panel:
                p.fillRect(0, m_track->get_baseY(), TRACKPANELWIDTH, m_track->get_height(), cm().get("TRACK_PANEL_BG"));

                //Draw Panel Head:
                panelPixmap.fill(cm().get("TRACK_PANEL_BG"));
                draw_panel_bus_in_out();
                draw_panel_gain();
                draw_panel_pan();
                draw_panel_head();
                draw_panel_track_name();

                //Update Panel
                p.drawPixmap(0, m_track->get_baseY(), panelPixmap, 0, 0, TRACKPANELWIDTH, m_track->get_height());

                // panelPixmap is overpainting the leds, fix it!
                draw_panel_leds(p);
                paintPanel = false;
        }

        if (paintClipArea) {
                clear_clip_area(p);
                paintClipArea = false;
        }

        return QRect();
}


void TrackView::draw_panel_gain()
{
        const int GAIN_Y = 36;
        const int GAIN_H = 8;

        panelWidth = TRACKPANELWIDTH;
        QPainter p(&panelPixmap);
        int sliderWidth=panelWidth-95;
        int sliderx=10;
        float gain = m_track->get_gain();
        QString s;
        p.setPen(cm().get("TRACK_PANEL_TEXT"));
        p.setFont( QFont( "Bitstream Vera Sans", (int)(GAIN_H*0.9)) );

        //first clear the m_track->get_gain()-area
        p.fillRect(0, GAIN_Y, panelWidth, GAIN_H, cm().get("TRACK_PANEL_BG"));
        p.drawText(sliderx, GAIN_Y+GAIN_H+1, "GAIN");

        float db = coefficient_to_dB(gain);
        if (db < -99)
                sgain = "- INF";
        else if ( db < 0)
                sgain = "- " + QByteArray::number((-1 * db), 'f', 1) + " dB";
        else
                sgain = "+" + QByteArray::number(db, 'f', 1) + " dB";

        p.fillRect(sliderx+30,GAIN_Y,sliderWidth,GAIN_H, cm().get("SLIDER_BACKGROUND"));
        p.drawRect(sliderx+30,GAIN_Y,sliderWidth,GAIN_H);


        if (db < -60)
                db = -60;
        int sliderdbx =  (sliderWidth - (sliderWidth*0.3)) - (int) ( ( (-1 * db) / 60 ) * sliderWidth);
        if (sliderdbx < 0)
                sliderdbx = 0;
        if (db > 0)
                sliderdbx =  (int)(sliderWidth*0.7) + (int) ( ( db / 6 ) * (sliderWidth*0.3));

        int cr = (gain >= 1 ? 30 + (int)(100 * gain) : (int)(50 * gain));
        int cb = ( gain < 1 ? 100 + (int)(50 * gain) : abs((int)(10 * gain)) );
        p.fillRect(sliderx+31, GAIN_Y+1, sliderdbx, GAIN_H-1, QColor(cr,0,cb));
        p.drawText(sliderx + sliderWidth + 35, GAIN_Y + GAIN_H, sgain);
}


void TrackView::draw_panel_head()
{
        QPainter p(&panelPixmap);
        if (m_track->is_active())
                p.fillRect(0,0,panelWidth,15, cm().get("TRACK_PANEL_HEAD_ACTIVE"));
        else
                p.fillRect(0,0,panelWidth,15, cm().get("TRACK_PANEL_HEAD_INACTIVE"));
        p.setPen(cm().get("TRACK_PANEL_SEPERATOR"));
        p.drawLine(0, 0, panelWidth, 0);
}



void TrackView::draw_panel_leds(QPainter& p)
{
        PENTER;
        recLed->draw(p);
        soloLed->draw(p);
        muteLed->draw(p);
        lockLed->draw(p);
}


void TrackView::draw_panel_pan()
{
        const int PAN_Y = 50;
        const int PAN_H = 8;

        panelWidth = TRACKPANELWIDTH;
        QPainter paint(&panelPixmap);
        int sliderWidth=panelWidth-95;
        int sliderx=10;
        float v;
        //	int y;
        QString s;
        paint.setPen(cm().get("TRACK_PANEL_TEXT"));
        paint.setFont( QFont( "Bitstream Vera Sans", (int)(PAN_H*0.9)) );

        //first clear the pan-area
        paint.fillRect(0, PAN_Y, panelWidth, PAN_H, cm().get("TRACK_PANEL_BG"));
        paint.drawText(sliderx, PAN_Y+PAN_H+1, "PAN");

        v = m_track->get_pan();
        span = QByteArray::number(v,'f',1);
        s = ( v > 0 ? QString("+") + span :  span );
        paint.fillRect(sliderx+30,PAN_Y,sliderWidth,PAN_H, cm().get("SLIDER_BACKGROUND"));
        paint.drawRect(sliderx+30,PAN_Y,sliderWidth,PAN_H);
        int pm=sliderx+30+sliderWidth/2;
        int z = abs((int)(v*(sliderWidth/2)));
        int c = abs((int)(255*v));
        if (v>=0)
                paint.fillRect(pm+1,PAN_Y+1,z,PAN_H-1,  QColor(c,0,0));
        else
                paint.fillRect(pm-z+1,PAN_Y+1,z,PAN_H-1,QColor(c,0,0));
        paint.drawText(sliderx+30+sliderWidth+10,PAN_Y+PAN_H+1, s);
}


//We still need shorter bus descriptions in order for them to fit in one row
void TrackView::draw_panel_bus_in_out()
{

        // Bus view at the bottom of the panel - I like this one better (Rudi)
        int y = 75;//m_track->get_height() - 12;

        // Bus view directly below pan
        //	int y = 60;
        QPainter paint(&panelPixmap);
        paint.setPen(cm().get("TRACK_PANEL_TEXT"));
        paint.setFont( QFont( "Bitstream Vera Sans", 8) );


        QString sir = "";
        sir.append(m_track->get_bus_in());

        // 	if (m_track->get_which_channels() == AudioDeviceMapper::MONO)
        // 		sir.append(" [MO]");
        // 	else if (m_track->get_which_channels() == AudioDeviceMapper::STEREO)
        // 		sir.append(" [LR]");
        // 	else if (m_track->get_which_channels() == AudioDeviceMapper::ONLY_LEFT_CHANNEL)
        // 		sir.append(" [L]");
        // 	else
        // 		sir.append(" [R]");

        QString sor = "";
        sor.append(m_track->get_bus_out());

        paint.drawPixmap(10, y, QPixmap(":/bus_in"));
        //	paint.fillRect(10,y,10,10,QColor(255,0,0));
        //	paint.setPen(Qt::black);
        //	paint.drawRect(10,y,10,10);
        paint.drawText(30,y+8, sir);

        paint.fillRect(95, y, 75, 10, cm().get("TRACK_PANEL_BG"));
        paint.drawPixmap(85, y, QPixmap(":/bus_out"));
        //	paint.fillRect(10,y+16,10,10,QColor(0,255,0));
        //	paint.setPen(Qt::black);
        //	paint.drawRect(10,y+16,10,10);
        paint.drawText(105,y+8, sor);

}


void TrackView::draw_panel_track_name()
{
        panelWidth = TRACKPANELWIDTH;
        QPainter paint(&panelPixmap);
        paint.setRenderHint(QPainter::TextAntialiasing);
        sid.setNum(m_track->get_id());

        paint.setFont( QFont( "Bitstream Vera Sans", 8) );
        paint.setPen(cm().get("TRACK_PANEL_NAME"));
        paint.drawText(4,12, sid);
        // 	paint.drawText(5,12, sid);

        paint.setFont( QFont( "Bitstream Vera Sans", 8) );
        paint.drawText(15,12, m_track->get_name());
}


void TrackView::resize_panel_pixmap()
{}


void TrackView::clear_clip_area(QPainter& p)
{
        PENTER3;
        if (m_track->is_active())
                p.fillRect(CLIPAREABASEX, m_track->get_baseY(), m_vp->width() , m_track->get_height(), cm().get("TRACK_BG_ACTIVE"));
        else
                p.fillRect(CLIPAREABASEX, m_track->get_baseY(), m_vp->width() , m_track->get_height(), cm().get("TRACK_BG"));

        // TRACK SEPARATOR (IMPROVE-ME (USE COLOMANAGER))
        /*	if (m_track->get_parent_song()->editingMode==Song::EDIT_TRACK_CURVES)
        		{
        		p.setPen(QColor(150,150,160, 170));// This is intentionally fixed color.
        		p.drawLine(CLIPAREABASEX, m_track->get_baseY() + m_track->get_height() -1 , m_vp->width(), m_track->get_baseY() + m_track->get_height() -1);
        		}
        	else
        		{*/
        p.setPen(cm().get("TRACK_SEPERATOR"));
        p.drawLine(CLIPAREABASEX, m_track->get_baseY() + m_track->get_height() -1 , m_vp->width(), m_track->get_baseY() + m_track->get_height() -1);
        // 		}

        // Draw region markers
        /*	MtaRegion* m = m_track->get_parent_song()->regionList->head();
        	while (m)
        		{
        		int xrs = m_track->get_parent_song()->block_to_xpos(m->beginBlock) + CLIPAREABASEX;
        		int xre = m_track->get_parent_song()->block_to_xpos(m->endBlock) + CLIPAREABASEX;
        		int xs,xe=0;
        		int w = clipAreaWidth;
        		if ((xrs>=0) && (xrs<w))
        			xs=xrs;
        		else if (xrs>=0)
        			xs=w;
        		else
        			xs=0;
         
        		if ((xre>=0) && (xre<w))
        			xe=xre;
        		else if (xre>=0)
        			xe=w;
        		else
        			xe=0;
        		p.fillRect(xs,m_track->get_baseY(),xe-xs,3,QColor(155,100,250));
        		m=m->next;
        		}*/
}

void TrackView::add_new_audioclipview( AudioClip * clip )
{
        PENTER;
        AudioClipView* audioClipView = new AudioClipView(m_vp, this, clip);
        audioClipViewList.append(audioClipView);

        connect(m_sv->get_assocsong(), SIGNAL(hzoomChanged( )), audioClipView, SLOT(schedule_for_repaint()));
        connect(m_sv->get_assocsong(), SIGNAL(firstBlockChanged()), audioClipView, SLOT(schedule_for_repaint()));
        connect(m_track, SIGNAL(heightChanged()), audioClipView, SLOT(schedule_for_repaint()));
        connect(m_vp, SIGNAL(resized()), audioClipView, SLOT(schedule_for_repaint()));
}

void TrackView::remove_audioclipview( AudioClip* clip )
{
        PENTER2;
        foreach(AudioClipView* acv, audioClipViewList) {
                if (acv->get_clip() == clip) {
                        audioClipViewList.removeAll(acv);
                        m_vp->unregister_viewitem(acv);
                        // FIXME this is not a nice way?
                        disconnect(clip, SIGNAL(trackStartFrameChanged()), this, SLOT (repaint_all_clips()));
                        disconnect(clip, SIGNAL(edgePositionChanged()), this, SLOT (repaint_all_clips()));

                        delete acv;
                }
        }
        repaint_all_clips();
}

void TrackView::repaint_cliparea()
{
        if (audioClipViewList.size() > 0) {
                schedule_for_repaint();
                paintClipArea = true;
        }
}

void TrackView::schedule_for_repaint( )
{
        set_geometry(0, m_track->get_baseY(), m_vp->width(), m_track->get_height());
        if (visible())
                m_vp->schedule_for_repaint(this);
}

void TrackView::height_changed( )
{
        PENTER;
        paintPanel = true;
        paintClipArea = true;
        //FIXME hmm, not a better way to do this? Added to clear the rootspace of SongView
        // It also adds much to much calls for adding SongView to the repaintList in ViewPort!
        m_vp->schedule_for_repaint(m_parent);
        schedule_for_repaint();
}

int TrackView::cliparea_basex( ) const
{
        return CLIPAREABASEX;
}

int TrackView::cliparea_width( ) const
{
        return clipAreaWidth;
}

void TrackView::resize( )
{
        clipAreaWidth = m_vp->width() - CLIPAREABASEX;
        paintClipArea = true;
        paintPanel = true;
        schedule_for_repaint();
}

Command* TrackView::jog_track_pan()
{
        int x = cpointer().clip_area_x();
        float w = (float) (m_vp->width() * 0.6);
        float ofx = (float) origX - x;
        float p = -2.0f *  (ofx) / w ;
        float fp = p + origPan;
        m_track->set_pan( fp );

        return (Command*) 0;
}


Command* TrackView::gain_and_pan()
{
        jog_gain_pan(cpointer().clip_area_x(), cpointer().y());
        return (Command*) 0;
}


Command* TrackView::capture_from_channel_both()
{
        // 	m_track->set_which_channels(AudioDeviceMapper::STEREO); //FIXME !!!!!!! AudioDeviceMapper::STEREO, AudioDeviceMapper::ONLY_LEFT_CHANNEL or AudioDeviceMapper::ONLY_RIGHT_CHANNEL
        return (Command*) 0;
}


Command* TrackView::capture_from_channel_left()
{
        // 	m_track->set_which_channels(AudioDeviceMapper::ONLY_LEFT_CHANNEL);
        return (Command*) 0;
}


Command* TrackView::capture_from_channel_right()
{
        // 	m_track->set_which_channels(AudioDeviceMapper::ONLY_RIGHT_CHANNEL);
        return (Command*) 0;
}


Command* TrackView::touch()
{
        int x = cpointer().clip_area_x();
        // 	int trackNumber = ie().collected_number();
        /*	if (trackNumber > 0) // there is a track number collected !!
        		{
        		if ((trackNumber<=numTracks) && (trackNumber>=0))
        			touch_track(trackNumber, x);
        		}
        	else
        		{
        		foreach(Track* track, m_song->get_tracks())
        			{
        			if (track->is_pointed(m->mousey()))
        				{
        				touch_track(i);
        				break;
        				}
        			}
        		}*/
        touch_track(m_track->get_id(), x);
        // 	pvp->update();
        return (Command*) 0;
}


Command* TrackView::touch_and_center()
{
        touch();
        m_sv->center();
        return (Command*) 0;
}


Command* TrackView::select_bus_in()
{
        PENTER;
        busInMenu->clear();
        QStringList names = audiodevice().get_capture_buses_names();
        foreach(QString name, names) {
                busInMenu->addAction(name);
        }
        busInMenu->exec(QCursor::pos());
        return (Command*) 0;
}


Command* TrackView::select_bus_out()
{
        busOutMenu->clear();
        QStringList names = audiodevice().get_playback_buses_names();
        foreach(QString name, names) {
                busOutMenu->addAction(name);
        }
        busOutMenu->exec(QCursor::pos());
        return (Command*) 0;
}


void TrackView::touch_track(int , int x)
{
        PENTER;
        m_sv->get_assocsong()->set_work_at(m_sv->xpos_to_block(x));
        m_sv->get_assocsong()->set_active_track(m_track->get_id());
}



void TrackView::jog_gain_pan(int mousex, int mousey)
{
        PENTER;
        if (jogMode == JOG_NONE) {
                PMESG("Starting jog gain/pan for track %d",m_track->get_id() );
                jogMode = JOG_GAIN_AND_PAN;
                origX = mousex;
                origY = mousey;
        } else {
                PMESG("Finishing jog gain/pan for track %d",m_track->get_id() );
                jogMode = JOG_NONE;
        }
}

void TrackView::panel_info_changed( )
{
        paintPanel = true;
        schedule_for_repaint();
}

void TrackView::repaint_all_clips( )
{
        paintClipArea = true;
        schedule_for_repaint();
        foreach(AudioClipView* view, audioClipViewList) {
                view->schedule_for_repaint();
        }
}

void TrackView::set_bus_in( QAction* action )
{
        PENTER;
        m_track->set_bus_in(action->text().toAscii());
}

void TrackView::set_bus_out( QAction* action )
{
        PENTER;
        m_track->set_bus_out(action->text().toAscii());
}

//eof
