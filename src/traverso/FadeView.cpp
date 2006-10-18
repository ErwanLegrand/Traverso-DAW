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

$Id: FadeView.cpp,v 1.3 2006/10/18 12:08:56 r_sijrier Exp $
*/

#include "FadeView.h"

#include <QPainter>
#include <QPainterPath>
#include <QPointF>

#include "FadeCurve.h"
#include "ViewPort.h"
#include "AudioClipView.h"
#include "FadeContextDialog.h"

#include "Song.h"
#include "AudioClip.h"
#include <Peak.h>

FadeView::FadeView( ViewPort * vp, AudioClipView* parent, FadeCurve * fadeCurve )
	: ViewItem(vp, parent, fadeCurve), m_clipView(parent),
	  m_fadeCurve(fadeCurve), m_clip(parent->get_clip())
{
	m_dialog = 0;

	m_vp->register_viewitem(this);
	m_type = FADEVIEW;
}


FadeView::~ FadeView( )
{
}


QRect FadeView::draw( QPainter & painter )
{
	if (m_fadeCurve->get_fade_type() == FadeCurve::FadeIn) {
		draw_fade_in(painter);
	}
	
	if (m_fadeCurve->get_fade_type() == FadeCurve::FadeOut) {
		draw_fade_out(painter);
	}
	
	return QRect();
}

void FadeView::draw_fade_in(QPainter& painter) 

{
	if (m_fadeCurve->get_range() == 0) {
		return;
	}
	
	QPolygonF polygon;
	float value[2];
	int step = 0;
	int startPos = 0;
	Song* song = m_clip->get_song();
	int hzoom = song->get_hzoom();
	nframes_t startFrame = m_clip->get_track_start_frame();
	nframes_t endFrame = (nframes_t) m_fadeCurve->get_range() + startFrame;
	nframes_t firstVisibleFrame = song->get_first_visible_frame();
	
	
	if (endFrame < firstVisibleFrame) {
		return;
	}
	
	if (firstVisibleFrame > startFrame) {
		startPos = (firstVisibleFrame - startFrame) / Peak::zoomStep[hzoom];
		startFrame = firstVisibleFrame;
	}
	
	int fadeWidth = (endFrame - startFrame) / Peak::zoomStep[hzoom];
	
	if (fadeWidth > m_clipView->clipXWidth) {
		fadeWidth = m_clipView->clipXWidth;
	}
	
	
// 	PWARN("fadeWidth is %d", fadeWidth);
	
	// Populate the polygon with enough points to draw a smooth curve.
	// using a 2 pixel resolution, is sufficient enough
	for (int i=startPos; i < (fadeWidth+startPos); i+=2) {
		m_fadeCurve->get_vector(i*Peak::zoomStep[hzoom], i*Peak::zoomStep[hzoom] + 1, value, 2);
	 	polygon << QPointF( step, m_clipView->height - (value[1] * m_clipView->height) );
	 	step += 2;
	}
	
	// Always add the uppermost point in the polygon path 
	// since the above routine potentially does not include it.
	polygon << QPointF(fadeWidth, 0);
	
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing);
	
	m_path = QPainterPath();
	
	m_path.moveTo(0, m_clipView->height);
	m_path.addPolygon(polygon);
	m_path.lineTo(0, 0);
	m_path.closeSubpath();
	
	painter.setPen(Qt::NoPen);
	if (m_fadeCurve->is_bypassed()) {
		painter.setBrush(QColor(255, 0, 255, 40));
	} else {
		painter.setBrush(QColor(255, 0, 255, 85));
	}
	
	painter.drawPath(m_path);	
	painter.restore();
}


void FadeView::draw_fade_out(QPainter& painter) 
{

	if (m_fadeCurve->get_range() == 0) {
		return;
	}
	
	QPolygonF polygon;
	float value[2];
	int step = 0;
	int startPos = 0;
	Song* song = m_clip->get_song();
	int hzoom = song->get_hzoom();
	nframes_t endFrame = m_clip->get_track_end_frame();
	nframes_t startFrame = endFrame - (nframes_t) m_fadeCurve->get_range();
	nframes_t firstVisibleFrame = song->get_first_visible_frame();
	
	
	if (endFrame < firstVisibleFrame) {
		return;
	}
	
	int startX = (startFrame - m_clip->get_track_start_frame()) / Peak::zoomStep[hzoom];
	if (startFrame < m_clip->get_track_start_frame()) {
		startX = 0;
		startPos = (m_clip->get_track_start_frame() - startFrame) / Peak::zoomStep[hzoom];
	}
	
	if (m_clip->get_track_start_frame() < firstVisibleFrame) {
		startX -= (firstVisibleFrame - m_clip->get_track_start_frame()) / Peak::zoomStep[hzoom];
	}
	
	
	if (firstVisibleFrame > startFrame) {
		startPos = (firstVisibleFrame - startFrame) / Peak::zoomStep[hzoom];
		startFrame = firstVisibleFrame;
	}
	
	if (startX < 0) {
		startX = 0;
	}
	
	
	int fadeWidth = m_clipView->clipXWidth - startX;
	
	
/*	PWARN("fadeWidth is %d", fadeWidth);
	PWARN("StartX is %d", startX);
	PWARN("startPos is %d", startPos);
	PWARN("clipXWidth is %d", clipXWidth);*/
	
	if (fadeWidth < 0) {
// 		PWARN("FadeWidth < 0");
		return;
	}
	
	// Populate the polygon with enough points to draw a smooth curve.
	// using a 2 pixel resolution, is sufficient enough
	for (int i=startPos; i < (fadeWidth+startPos); i+=2) {
		m_fadeCurve->get_vector(i*Peak::zoomStep[hzoom], i*Peak::zoomStep[hzoom] + 1, value, 2);
	 	polygon << QPointF( step+startX, m_clipView->height - (value[1] * m_clipView->height) );
	 	step += 2;
	}
	
	// Always add the lowest point in the polygon path 
	// since the above routine potentially does not include it.
	polygon << QPointF(startX + fadeWidth, m_clipView->height);
	
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing);
	
	m_path = QPainterPath();
	m_path.addPolygon(polygon);
	m_path.lineTo(startX + fadeWidth, 0);
	m_path.lineTo(startX, 0);
	m_path.closeSubpath();
	
	painter.setPen(Qt::NoPen);
	if (m_fadeCurve->is_bypassed()) {
		painter.setBrush(QColor(255, 0, 255, 40));
	} else {
		painter.setBrush(QColor(255, 0, 255, 85));
	}
	
	painter.drawPath(m_path);	
	painter.restore();
}


Command* FadeView::edit_properties()
{
	if (!m_dialog) {
		m_dialog = new FadeContextDialog(m_fadeCurve);
	}
	
	m_dialog->show();
	
	return 0;
}

bool FadeView::is_pointed( ) const
{
	int x = cpointer().x() - m_clipView->baseX;
	// 14 == CLIPINFO_HEIGHT
	int y = cpointer().y() - (m_clipView->baseY + 14);
	
	return m_path.contains(QPointF(x, y));
}

//eof
