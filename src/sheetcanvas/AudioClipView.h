/*
Copyright (C) 2005-2009 Remon Sijrier 

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

#ifndef AUDIO_CLIP_VIEW_H
#define AUDIO_CLIP_VIEW_H

#include "ViewItem.h"
#include <defines.h>
#include <QList>
#include <QTimer>
#include <QPolygonF>
#include <QPixmap>

class AudioClip;
class Sheet;
class FadeCurve;
class CurveView;
class SheetView;
class AudioTrackView;
class FadeCurveView;
class Peak;


class AudioClipView : public ViewItem
{
	Q_OBJECT

public:
	AudioClipView(SheetView* view, AudioTrackView* parent, AudioClip* clip);
	~AudioClipView();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	
	AudioClip* get_clip() const {return m_clip;}
        AudioTrackView* get_audio_track_view() const {return m_tv;}
        CurveView* get_gain_curve_view() const {return m_gainCurveView;}
	int get_height() const {return m_height;}
	
	void calculate_bounding_rect();
	void load_theme_data();
	
private:
	AudioTrackView* 	m_tv;
        QList<FadeCurveView*> m_FadeCurveViews;
	AudioClip* 	m_clip;
	Sheet*		m_sheet;
        CurveView* 	m_gainCurveView;
	QPolygonF 	m_polygon;
	QPixmap 	m_clipInfo;
	QTimer 		m_recordingTimer;

	float m_progress;
	int m_peakloadingcount;

	bool m_waitingForPeaks;
	bool m_mergedView;
	bool m_classicView;
	bool m_paintWithOutline;
	bool m_drawDbGrid;
	int m_height;
	int m_infoAreaHeight;
	int m_mimimumheightforinfoarea;
	int m_lineOffset;
	int m_lineVOffset;
	TimeRef m_oldRecordingPos;
	
	// theme data
	int m_drawbackground;
	int m_fillwave;
	QColor m_backgroundColorTop;
	QColor m_backgroundColorBottom;
	QColor m_backgroundColorMouseHoverTop;
	QColor m_backgroundColorMouseHoverBottom;
	QColor minINFLineColor;
	QBrush m_waveBrush;
	QBrush m_brushBgRecording;
	QBrush m_brushBgMuted;
	QBrush m_brushBgMutedHover;
	QBrush m_brushBgSelected;
	QBrush m_brushBgSelectedHover;
	QBrush m_brushBg;
	QBrush m_brushBgHover;
	QBrush m_brushFg;
	QBrush m_brushFgHover;
	QBrush m_brushFgMuted;
	QBrush m_brushFgEdit;
	QBrush m_brushFgEditHover;

	void create_clipinfo_string();

	void draw_clipinfo_area(QPainter* painter, int xstart, int pixelcount);
	void draw_db_lines(QPainter* painter, qreal xstart, int pixelcount);
	void draw_peaks(QPainter* painter, qreal xstart, int pixelcount);
	void create_brushes();

	friend class FadeCurveView;

public slots:
        void add_new_fade_curve_view(FadeCurve* fade);
        void remove_fade_curve_view(FadeCurve* fade);
	void repaint();
	void update_start_pos();
	void position_changed();
	
	TCommand* fade_range();
	TCommand* clip_fade_in();
	TCommand* clip_fade_out();
	TCommand* select_fade_in_shape();
	TCommand* select_fade_out_shape();
	TCommand* reset_fade();
	TCommand* set_audio_file();
	TCommand* edit_properties();
	
private slots:
	void update_progress_info(int progress);
	void peak_creation_finished();
	void start_recording();
	void finish_recording();
	void update_recording();
	void clip_state_changed();
        void active_context_changed();
};

#endif

//eof
