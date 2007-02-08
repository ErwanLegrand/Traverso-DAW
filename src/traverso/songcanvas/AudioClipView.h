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

$Id: AudioClipView.h,v 1.9 2007/02/08 13:30:42 r_sijrier Exp $
*/

#ifndef AUDIO_CLIP_VIEW_H
#define AUDIO_CLIP_VIEW_H

#include "ViewItem.h"
#include <defines.h>
#include <QList>

class AudioClip;
class Song;
class FadeCurve;
class CurveView;
class SongView;
class TrackView;
class FadeView;

class AudioClipView : public ViewItem
{
	Q_OBJECT
	Q_CLASSINFO("drag", tr("Move Clip"))
	Q_CLASSINFO("drag_edge", tr("Move Edge"))
	Q_CLASSINFO("split", tr("Split"))

public:
	AudioClipView(SongView* view, TrackView* parent, AudioClip* clip);
	~AudioClipView();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	void set_height(int height);
	
	AudioClip* get_clip();
	int get_height() const;
	int get_fade_y_offset() const;
	
	void calculate_bounding_rect();
	
	TrackView* get_trackview() const {return m_tv;}
	void set_trackview(TrackView* view) {m_tv = view;}
	
	void load_theme_data();

private:
	TrackView* 	m_tv;
	QList<FadeView*> m_fadeViews;
	AudioClip* 	m_clip;
	Song*		m_song;
	CurveView* 	curveView;

	QPixmap clipNamePixmapActive;
	QPixmap clipNamePixmapInActive;

	int m_progress;

	bool waitingForPeaks;
	bool mergedView;
	bool classicView;
	int m_height;
	int m_infoAreaHeight;
	int m_mimimumheightforinfoarea;
	int m_usePolygonPeakDrawing;
	
	// theme data
	int m_drawbackground;
	int m_fillwave;
	QColor m_backgroundColor;
	QColor m_backgroundColorMouseHover;

	void recreate_clipname_pixmap();

	void draw_clipinfo_area(QPainter* painter, int xstart, int pixelcount);
	void draw_peaks(QPainter* painter, int xstart, int pixelcount);

	
	friend class FadeView;

public slots:
	void add_new_fadeview(FadeCurve* fade);
	void remove_fadeview(FadeCurve* fade);
	void update_progress_info(int progress);
	void peaks_creation_finished();
	void gain_changed();
	void repaint();
	void update_start_pos();
	void position_changed();
	
	Command* drag();
	Command* drag_edge();
	Command* split();
};


inline int AudioClipView::get_height() const {return m_height;}


#endif

//eof
