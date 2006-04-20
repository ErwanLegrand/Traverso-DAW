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
 
    $Id: Curve.h,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#ifndef CURVE_H
#define CURVE_H


#include <QString>

#include "AudioPluginController.h"


class CurveNode;

class Curve
{
public:
        Curve(AudioPluginController* pAssocAudioPluginController, QString pType);
        ~Curve();

        CurveNode* add_node(nframes_t pPos, float pValue);
        void add_node(CurveNode* node);
        CurveNode* get_nearest_node(nframes_t pos);
        void delete_node(CurveNode* node);

        float get_value_at(nframes_t pos);

        CurveNode* get_current_curve_node();

        void show();
        void hide();

        void highlight(int pXpos);
        void activate();
        void deactivate();

        CurveNode* head;
        CurveNode* currentNode;

        QString get_type();

        QString get_schema();

private :
        bool active;

        AudioPluginController* assocAudioPluginController;

        QString type;

        friend class CurveNode;

};


#endif
