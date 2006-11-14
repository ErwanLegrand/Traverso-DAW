/*
    Copyright (C) 2006 Remon Sijrier 
 
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
 
    $Id: ClipsViewPort.h,v 1.2 2006/11/14 14:59:07 r_sijrier Exp $
*/

#ifndef CLIPS_VIEW_PORT_H
#define CLIPS_VIEW_PORT_H

#include <QGraphicsScene>

#include "ViewPort.h"

class SongWidget;
		
class ClipsViewPort : public ViewPort
{
	Q_OBJECT

public:
	ClipsViewPort(QGraphicsScene* scene, SongWidget* sw);
	~ClipsViewPort() {};
	
        void get_pointed_view_items(QList<ViewItem* > &list);

protected:
        void resizeEvent(QResizeEvent* e);

private:
	SongWidget*	m_sw;
};


#endif

//eof
