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
class TrackView;
class FadeView;
class Peak;


class AudioClipView : public ViewItem
{
	Q_OBJECT
	Q_CLASSINFO("fade_range", tr("Closest: Adjust Length"))
	Q_CLASSINFO("clip_fade_in", tr("In: Adjust Length"))
	Q_CLASSINFO("clip_fade_out", tr("Out: Adjust Length"))
	Q_CLASSINFO("select_fade_in_shape", tr("In: Select Preset"));
	Q_CLASSINFO("select_fade_out_shape", tr("Out: Select Preset"));
	Q_CLASSINFO("reset_fade", tr("Closest: Delete"));
	Q_CLASSINFO("set_audio_file", tr("Reset Audio File"));
	Q_CLASSINFO("edit_properties", tr("Edit Properties"));

public:
	AudioClipView(SheetView* view, TrackView* parent, AudioClip* clip);
	~AudioClipView();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	void set_height(int height);
	
	AudioClip* get_clip() const {return m_clip;}
	int get_height() const;
	int get_childview_y_offset() const;
	
	void calculate_bounding_rect();
	void load_theme_data();
	
protected:
	void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );

private:
	TrackView* 	m_tv;
	QList<FadeView*> m_fadeViews;
	AudioClip* 	m_clip;
	Sheet*		m_sheet;
	CurveView* 	curveView;
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

	friend class FadeView;

public slots:
	void add_new_fadeview(FadeCurve* fade);
	void remove_fadeview(FadeCurve* fade);
	void repaint();
	void update_start_pos();
	void position_changed();
	
	Command* fade_range();
	Command* clip_fade_in();
	Command* clip_fade_out();
	Command* select_fade_in_shape();
	Command* select_fade_out_shape();
	Command* reset_fade();
	Command* set_audio_file();
	Command* edit_properties();
	
private slots:
	void update_progress_info(int progress);
	void peak_creation_finished();
	void start_recording();
	void finish_recording();
	void update_recording();
	void clip_state_changed();
};


inline int AudioClipView::get_height() const {
	int height;
	(m_height > m_mimimumheightforinfoarea) ? height = m_height - m_infoAreaHeight : height = m_height;
	return height;
}

#endif

//eof
