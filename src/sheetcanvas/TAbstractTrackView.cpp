/*
Copyright (C) 2011 Remon Sijrier

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

*/

#include "TAbstractTrackView.h"

#include "TrackPanelView.h"

TAbstractTrackView::TAbstractTrackView(ViewItem* parentView, ContextItem* parentContext)
	: ViewItem(parentView, parentContext)
{
	m_panel = 0;
}

void TAbstractTrackView::move_to( int x, int y )
{
	Q_UNUSED(x);
	setPos(0, y);
	if (m_panel)
		m_panel->setPos(-200, y);
}
