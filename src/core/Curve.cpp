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
 
    $Id: Curve.cpp,v 1.2 2006/05/01 21:21:37 r_sijrier Exp $
*/

#include "Curve.h"

#include <QPainter>

#include "ColorManager.h"
#include "Song.h"
#include "Track.h"
#include "CurveNode.h"
#include "AudioPluginController.h"
#include "AudioPluginChain.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Curve::Curve(AudioPluginController* pAssocController, QString pType)
{
        PENTERCONS;
        assocAudioPluginController = pAssocController;
        type = pType;
        PMESG("Creating Curve : %s ", (const char*) pType.toAscii().data());
        active=false;
        PMESG("Adding root node [0:0.00]");
        // Initial node, always added.
        head = new CurveNode( this, 0, 0.0);
        head->prev=(CurveNode*)0;
        head->next=(CurveNode*)0;
        currentNode = head;
}


Curve::~Curve()
{
        PENTERDES;
        CurveNode* n = head;
        while (n) {
                CurveNode* n2 = n->next;
                delete n;
                n=n2;
        }
}

void Curve::activate()
{
        active=true;
}

void Curve::deactivate()
{
        active=false;
}

CurveNode* Curve::add_node(nframes_t p, float v)
{
        PENTER2;
        CurveNode* no;
        if (p==0) {
                no = head;
                no->set_value(v);
        } else {
                no = new CurveNode( this, p, v);
                add_node(no);
        }
        return no;
}


void Curve::add_node(CurveNode* node)
{
        PENTER2;
        if (!head) {
                head = node;
                node->prev=0;
                node->next=0;
        } else {
                CurveNode* n1 = head;
                CurveNode* n2 = head;
                while (n1 && (n1->pos < node->pos)) {
                        n2=n1;
                        n1=n1->next;
                }
                if (!n1) {
                        n2->next=node;
                        node->prev=n2;
                        node->next=0;
                } else {
                        node->prev=n2;
                        node->next=n1;
                        n2->next=node;
                        n1->prev=node;
                }
        }
        currentNode = node;

        /* ONLY FOR DEBUG PURPOSES
        printf("Node List is now :\n");
        CurveNode* no = head;
        while (no)
        	{
        	printf("\t\t%ld=%2.2f\n",(long)no->pos,no->value);
        	no=no->next;
        	}
        */
}




CurveNode* Curve::get_nearest_node(nframes_t pos)
{
        PENTER4;
        CurveNode* nno = (CurveNode*) 0;
        CurveNode* no = head;
        while (no) {
                if (no->next) {
                        if (abs(pos - no->pos) < abs(pos - no->next->pos)) {
                                nno=no;
                                break;
                        }
                } else
                        nno=no;
                no=no->next;
        }
        return nno;
}




void Curve::delete_node(CurveNode* )
{
        PENTER;
}


float Curve::get_value_at(nframes_t pos)
// THIS FUNCTION NEEDS HUGE OPTIMIZATION !! It finds the nearest node everytime it needs to get the value. for
// playback this is a waste of CPU. it could "follow" the playback and keep  updating the nearest node for playback cursor during the playback.
{
        PENTER4;
        float value=0.0;
        CurveNode* cn = get_nearest_node(pos);
        if (!cn) // MAYBE THIS CHECK IS NOT NECESSARY
        {
                return 0.0;
        }
        if ( (!cn->next) && (pos >= cn->pos)) // after last node, all points use the last node value
        {
                return cn->value;
        }
        CurveNode* cl;
        CurveNode* cr;
        if ( pos >= cn->pos)
        {
                cl = cn;
                cr = cn->next;
        } else
        {
                cl = cn->prev;
                cr = cn;
        }
        nframes_t dx = pos - cl->pos;
        nframes_t dd = cr->pos - cl->pos;
        float dv = cr->value - cl->value;
        double qd = (double)dx/(double)dd;
        float dy = dv*(float)qd;
        value = cl->value + dy;
        return value;
}

void Curve::show()
{
        PENTER3;
        if (!head) {
                return;
        }

        // /*	Track* parentTrack = assocAudioPluginController->parentChain->assocTrack;
        //
        // 	int baseY = parentTrack->get_baseY();
        // 	int half = parentTrack->get_height()/2;
        // 	QPainter p ((QWidget*)iface->get_songview());
        //
        // 	CurveNode* n = head;
        // 	int yc = baseY + half;
        //
        // 	int lastx = 0;
        // 	int lasty = yc;
        //
        // 	//if ( n->pos < firstBlock ) continue; FOR LATER
        // 	//if ( n->pos > lastBlock ) break; FOR LATER
        // 	Song* parentSong = parentTrack->get_song();
        // 	if (active)
        // 		p.setPen(cm().get(CURVE_ACTIVE));
        // 	else
        // 		p.setPen(cm().get(CURVE_NONACTIVE));
        // 	while (n)
        // 		{
        // 		int x = parentSong->block_to_xpos(n->pos);
        // 		int y = yc + (int) (-1*n->value * half / 100 );
        // 		p.drawLine(lastx, lasty, x, y);
        // 		lastx=x;
        // 		lasty=y;
        // 		n=n->next;
        // 		}
        // 	p.drawLine(lastx, lasty, 500 /*FIXME :-) parentTrack->get_width()*/, lasty);
        //
        // 	n = head;
        // 	while (n)
        // 		{
        // 		n->draw();
        // 		n=n->next;
        // 		}*/
}


void Curve::hide()
{
        PENTER;
}


void Curve::highlight(int )
{
        PENTER;
}



CurveNode* Curve::get_current_curve_node()
{
        return currentNode;
}

QString Curve::get_type()
{
        return type;
}

QString Curve::get_schema()
{
        QString schema="";
        CurveNode* n = head;
        while (n) {
                QString sp;
                sp.setNum((double)n->pos,'g',10);
                QString sv;
                sv.setNum(n->value);
                schema=schema+"                 <node pos="+sp+" value="+sv+" />\n";
                n=n->next;
        }
        return schema;
}
//eof

