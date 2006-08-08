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

$Id: AudioClipView.h,v 1.10 2006/08/08 19:37:03 r_sijrier Exp $
*/

#ifndef AUDIOCLIPVIEW_H
#define AUDIOCLIPVIEW_H

#include "ViewItem.h"
#include <QList>

class AudioClip;
class TrackView;
class Song;
class FadeView;
class FadeCurve;

class AudioClipView : public ViewItem
{
	Q_OBJECT

public:

	static const int LEFT = 0;  ///< Universal constant for Left
	static const int RIGHT = 1; ///< Universal constant for Right
	static const int LEFT_AND_RIGHT = 2;
	static const int CLIP_INFO_AREA_HEIGHT = 16;

	AudioClipView(ViewPort* vp, TrackView* parent,AudioClip* clip);
	~AudioClipView();

	AudioClip* get_clip();
	
	
	QRect draw(QPainter& painter);

private:
	TrackView* 	m_tv;
	QList<FadeView*> m_fadeViews;
	AudioClip* 	m_clip;
	Song*		m_song;
	QMenu		contextMenu;
	QMenu 		fadeInShapeSelector;
	QMenu 		fadeOutShapeSelector;

	QString clipInfo;
	QString sRate;
	QString sBitDepth;
	QString sourceType;
	QString sclipGain;
	QString sMuted;

	QPixmap clipNamePixmapActive;
	QPixmap clipNamePixmapInActive;

	int m_hzoom;
	int m_muted;
	int m_progress;

	bool waitingForPeaks;
	bool mergedView;
	bool classicView;

	void create_fade_selectors();
	void recreate_clipname_pixmap();
	void update_geometry();

	void process_fade_clip();
	void update_variables();
	void draw_clipinfo_area(QPainter& painter);
	void draw_peaks(QPainter& painter);
	void draw_crossings(QPainter& painter);

	nframes_t sourceLastFrame;
	nframes_t sourceFirstFrame;
	nframes_t trackFirstFrame;
	nframes_t trackLastFrame;
	nframes_t startFrame;
	float gain;
	int hzoom;
	int baseY;
	int baseX;
	int height;
	int clipXWidth;
	int clipAreaWidth;
	int channels;

	AudioClip* prevClip;
	AudioClip* nextClip;
	nframes_t prevTLF;
	nframes_t nextTFF;
	bool crossAtLeft;
	bool crossAtRight;
	int lCrossX;
	int rCrossX;
	
	friend class FadeView;

public slots:
	void add_new_fadeview(FadeCurve* fade);
	void remove_fadeview(FadeCurve* fade);
	void mute_changed(bool mute);
	void schedule_for_repaint();
	void update_progress_info(int progress);
	void peaks_creation_finished();
	void gain_changed();
	
	void set_fade_in_shape(QAction* action);
	void set_fade_out_shape(QAction* action);
	
	Command* select_fade_in_shape();
	Command* select_fade_out_shape();
};

#endif

//eof
