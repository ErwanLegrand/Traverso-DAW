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

$Id: AudioClipView.cpp,v 1.17 2007/02/05 17:12:02 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include <QPainter>
#include <QPainterPath>
#include <QGraphicsScene>

#include "AudioClipView.h"
#include "SongView.h"
#include "TrackView.h"
#include "FadeView.h"
#include "CurveView.h"

#include "Themer.h"
#include <Config.h>
#include <FadeCurve.h>
#include <Curve.h>

#include <MoveClip.h>
#include <MoveEdge.h>
#include <SplitClip.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const int MINIMAL_CLIPINFO_HEIGHT = 30;
static const int CLIPINFO_HEIGHT = 14;

AudioClipView::AudioClipView(SongView* sv, TrackView* parent, AudioClip* clip )
	: ViewItem(parent, clip), m_tv(parent), m_clip(clip)
{
	PENTERCONS;
	
	setZValue(parent->zValue() + 1);
	
	m_sv = sv;
	m_tv->scene()->addItem(this);
	
	clipNamePixmapActive = QPixmap();
	clipNamePixmapInActive = QPixmap();

	waitingForPeaks = false;
	m_progress = 0;
	m_song = m_clip->get_song();
	set_height(m_tv->get_clipview_height());
	recreate_clipname_pixmap();
	calculate_bounding_rect();
	
	classicView = config().get_property("AudioClip", "WaveFormRectified", 0).toInt() == 0 ? 1 : 0;
	mergedView = config().get_property("AudioClip", "WaveFormMerged", 0).toInt() == 0 ? 0 : 1;
	
	if (FadeCurve* curve = m_clip->get_fade_in()) {
		add_new_fadeview(curve);
	}
	if (FadeCurve* curve = m_clip->get_fade_out()) {
		add_new_fadeview(curve);
	}
	
	CurveView* curveView = new CurveView(m_sv, this, m_clip->get_gain_envelope());
	
	connect(m_clip, SIGNAL(muteChanged()), this, SLOT(repaint()));
	connect(m_clip, SIGNAL(stateChanged()), this, SLOT(repaint()));
	connect(m_clip, SIGNAL(gainChanged()), this, SLOT (gain_changed()));
	connect(m_clip, SIGNAL(fadeAdded(FadeCurve*)), this, SLOT(add_new_fadeview( FadeCurve*)));
	connect(m_clip, SIGNAL(fadeRemoved(FadeCurve*)), this, SLOT(remove_fadeview( FadeCurve*)));
	connect(m_clip, SIGNAL(positionChanged()), this, SLOT(position_changed()));
	
	connect(m_sv, SIGNAL(viewModeChanged()), this, SLOT(repaint()));
	
	setFlags(ItemIsSelectable | ItemIsMovable);
	setAcceptsHoverEvents(true);
}

AudioClipView::~ AudioClipView()
{
	PENTERDES;
}

void AudioClipView::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	PENTER;
	Q_UNUSED(widget);
	
/*	QPixmap* pix = dynamic_cast<QPixmap*>(painter->paintEngine()->paintDevice());
	if (pix) {
		printf("painting on a pixmap\n");
	}*/
	
	if (m_clip->is_recording()) {
		// For now, just exit. For later, draw the recording audio :-)
		return;
	}

// 	printf("exposed rect is: x=%f, y=%f, w=%f, h=%f\n", option->exposedRect.x(), option->exposedRect.y(), option->exposedRect.width(), option->exposedRect.height());

	
	int xstart = (int) option->exposedRect.x();
	int pixelcount = (int) option->exposedRect.width();
	if (pixelcount == 0) {
		PWARN("AudioClipView::paint : Exposed rectangle has 0 width ????");
		return;
	}
	
	
	QRectF clipRect = m_boundingRectangle;
	clipRect.setWidth(clipRect.width() + 1);
	clipRect.setHeight(clipRect.height() + 1);
	painter->setClipRect(clipRect);
	
	QColor color;
	
	// paint background color
	if (m_clip->is_muted()) {
		color = themer()->get_color("CLIP_BG_MUTED");
	} else if (m_clip->is_selected()) {
		color = themer()->get_color("CLIP_BG_SELECTED");
	} else {
		color = themer()->get_color("CLIP_BG_DEFAULT");
	}
	
	if (option->state & QStyle::State_MouseOver) {
		color = color.dark(102);
	}
	painter->fillRect(xstart, CLIPINFO_HEIGHT, pixelcount, m_height, color);
	

	int channels = m_clip->get_channels();

	// Only continue if our AudioClip has more then 1 channel
	if (channels == 0) {
		PWARN("This AudioClipView has 0 channels!!!!!");
		return;
	}
	

	if (waitingForPeaks) {
		PMESG("Waiting for peaks!");
		// Hmm, do we paint here something?
		// Progress info, I think so....
/*		painter.setPen(Qt::black);
		QRect r(clipXWidth/10, 0, 180, height);
		painter.setFont( QFont( "Bitstream Vera Sans", 12 ) );
		QString si;
		si.setNum(m_progress);
		if (m_progress == 100) m_progress = 0;
		QString buildProcess = "Building Peaks: " + si + "%";
		painter.drawText(r, Qt::AlignVCenter, buildProcess);*/
	
	} else {
		
		// 	draw_crossings(p);
		
		if (m_clip->is_selected()) {
			color = QColor(124, 137, 210); // Channel seperator color.
		} else {
			color = QColor(178, 191, 182); // Channel seperator color.
		}
		
		if (option->state & QStyle::State_MouseOver) {
			color = color.light(110);
		}
		painter->setPen(color);
		
		// Draw channel seperator horizontal lines.
		if (!mergedView) {
			for (int i=1; i<channels; ++i) {
				if (m_height < (MINIMAL_CLIPINFO_HEIGHT + CLIPINFO_HEIGHT)) {
					painter->drawLine(xstart, (m_height/channels) * i, xstart + pixelcount, (m_height/channels) * i);
				} else {
					painter->drawLine(xstart, (m_height/channels) * i + CLIPINFO_HEIGHT, xstart + pixelcount, (m_height/channels) * i + CLIPINFO_HEIGHT);
				}
			}
		}
	
		
		draw_peaks(painter, xstart, pixelcount);
	}

        painter->setPen(themer()->get_color("DARK_TEXT")); // CHANGE TO CLIP_COUNTOUR
	
	if (m_height < (MINIMAL_CLIPINFO_HEIGHT + CLIPINFO_HEIGHT)) {
	        // Black Contour
		painter->drawRect(xstart, 0, pixelcount, m_height);
	} else {
		draw_clipinfo_area(painter, xstart, pixelcount);
        	// Black Contour
		painter->setPen(themer()->get_color("CLIP_BG_DEFAULT").dark(255));
		painter->drawRect(xstart, 0, pixelcount, CLIPINFO_HEIGHT);
		painter->drawRect(xstart, 0, pixelcount, m_height + CLIPINFO_HEIGHT);
	}
	
	if (xstart == 0) {
		painter->setPen(themer()->get_color("CLIP_BG_DEFAULT").dark(110));
		painter->drawLine(1, CLIPINFO_HEIGHT, 1, m_height + CLIPINFO_HEIGHT);
		painter->setPen(themer()->get_color("CLIP_BG_DEFAULT").light(105));
		painter->drawLine(2, CLIPINFO_HEIGHT, 2, m_height + CLIPINFO_HEIGHT);
	}

	PMESG2("drawing clip");
}


void AudioClipView::draw_peaks(QPainter* p, int xstart, int pixelcount)
{
	PENTER2;

	int channels = m_clip->get_channels();
	const int height = m_height / channels;
	Peak* peak;
	int posY, negY, centerY;
	bool microView = m_song->get_hzoom() > Peak::MAX_ZOOM_USING_SOURCEFILE ? 0 : 1;
	int peakdatacount = microView ? pixelcount : pixelcount * 2;


	for (int chan=0; chan < channels; chan++) {
		peak = m_clip->get_peak_for_channel(chan);
		
		int buffersize;
		if (microView) {
			buffersize = sizeof(short) * peakdatacount;
		} else {
			buffersize = sizeof(unsigned char) * peakdatacount;
		}
	
		unsigned char peakBuffer[buffersize];
		
		int availpeaks = peak->calculate_peaks(peakBuffer, m_song->get_hzoom(), xstart * m_sv->scalefactor + m_clip->get_source_start_frame(), peakdatacount);
		if (peakdatacount != availpeaks) {
			PWARN("peakdatacount != availpeaks (%d, %d)", peakdatacount, availpeaks);
		}

		if (availpeaks <= 0) {
			// It seems there are no peak buffers yet, but they are now generated
			// just wait for the finished() signal.....
// 			PWARN("Waiting for peak");
			waitingForPeaks = true;
			connect(peak, SIGNAL(progress(int)), this, SLOT(update_progress_info(int)));
			connect(peak, SIGNAL(finished()), this, SLOT (peaks_creation_finished()));
			return;
		}

		float gain = m_clip->get_gain() * m_clip->get_norm_factor();
		float scaleFactor = ( ((float)(height - channels * 2)) / (Peak::MAX_DB_VALUE * 2)) * gain;
		
		if (m_height < (MINIMAL_CLIPINFO_HEIGHT + CLIPINFO_HEIGHT)) {
			centerY = height/2 + height*chan;
		} else {
			centerY = height/2 + height*chan + CLIPINFO_HEIGHT;
		}
		
		if (!microView) {
			Curve* curve = m_clip->get_gain_envelope();
			float value[2];
			int count = 0, peakbufferpos = 0;
			do {
				curve->get_vector((xstart+count)*m_sv->scalefactor, (xstart+count)*m_sv->scalefactor + 1, value, 2);
				count ++;
			
				peakBuffer[peakbufferpos] = peakBuffer[peakbufferpos] * value[1] * scaleFactor;
				peakbufferpos++;
				peakBuffer[peakbufferpos] = peakBuffer[peakbufferpos] * value[1] * scaleFactor;
				peakbufferpos++;
			
			} while (count < pixelcount);
		}
		
		if (microView) {
		
			short* mbuffer = (short*) peakBuffer;
			 
			int prev =  (int) (centerY + (scaleFactor * mbuffer[0]));
			if (m_sv->viewmode == EditMode) {
				p->setPen(themer()->get_color("CLIP_PEAK_MICROVIEW"));
			} else  {
				p->setPen(themer()->get_color("CLIP_PEAK_MICROVIEW").light(230));
			}
			int bufferPos = 0;
			for (int x = xstart; x < (pixelcount+xstart); x++) {
				posY = (int) (centerY + (scaleFactor * mbuffer[bufferPos++]));
				p->drawLine(x, prev, x+1, posY);
				prev = posY;
			}
		} else if (classicView) {
			int bufferPos = 0;
			if (mergedView) {
				scaleFactor = ( ( (float) m_height ) / Peak::MAX_DB_VALUE) * gain / 2.0;
				centerY = m_height/2;
			}
			if (m_sv->viewmode == EditMode) {
				p->setPen(themer()->get_color("AudioClip:wavemacro"));
// 				p->setPen(themer()->get_color("CLIP_PEAK_MICROVIEW"));
			} else  {
				p->setPen(themer()->get_color("AudioClip:Curvemode:wavemacro"));
			}


			for (int x = xstart; x < (pixelcount+xstart); x++) {
                                posY = (int) (centerY + peakBuffer[bufferPos]);
                                negY = (int) (centerY - peakBuffer[bufferPos + 1]);
				p->drawLine(x, negY, x, posY);
				bufferPos += 2;
			}
		} else {
			if (mergedView) {
				scaleFactor = ( ( (float) m_height ) / Peak::MAX_DB_VALUE) * gain;
				centerY = m_height + CLIPINFO_HEIGHT;
			} else {
				scaleFactor = ( ( (float) height ) / Peak::MAX_DB_VALUE) * gain;
				centerY = height*(chan+1) + CLIPINFO_HEIGHT;
			}
			if (m_sv->viewmode == EditMode) {
				p->setPen(themer()->get_color("CLIP_PEAK_MACROVIEW"));
			} else  {
				p->setPen(themer()->get_color("CLIP_PEAK_MACROVIEW").light(230));
			}

			int bufferPos = 0;
			for (int x = xstart; x < (pixelcount+xstart); x++) {
                                posY = (int) (centerY - (scaleFactor * (f_max(peakBuffer[bufferPos], peakBuffer[bufferPos + 1]))));
				p->drawLine(x, centerY, x, posY);
				bufferPos += 2;
			}
		}
	}
}


void AudioClipView::draw_crossings( QPainter& )
{
}


void AudioClipView::draw_clipinfo_area(QPainter* p, int xstart, int pixelcount)
{
	// clip info area bg
	if (m_clip->get_track()->is_active())
		p->fillRect(xstart, 0, pixelcount, 16, themer()->get_color("CLIPNAME_ACTIVE"));
	else
		p->fillRect(xstart, 0, pixelcount, 16, themer()->get_color("CLIPNAME_INACTIVE"));


	// Draw Clip Info Area
	if (m_clip->get_track()->is_active())
		p->drawPixmap(0, 0, clipNamePixmapActive, 0, 0, 600, CLIP_INFO_AREA_HEIGHT);
	else
		p->drawPixmap(0, 0, clipNamePixmapInActive, 0, 0, 600, CLIP_INFO_AREA_HEIGHT);
}


void AudioClipView::recreate_clipname_pixmap()
{
	PENTER;
	int channels = m_clip->get_channels();
	Q_ASSERT(channels < 8);
	
	int rate = m_clip->get_rate();
	int bitDepth = m_clip->get_bitdepth();
	bool isTake = m_clip->is_take();
	QString clipName = m_clip->get_name();

	QString sRate = QString::number(rate);
	QString sBitDepth = QString::number(bitDepth);
	QString sourceType = (isTake?"CAP":"SRC");
	
	QString sclipGain = "Gain: "+ coefficient_to_dbstring(m_clip->get_gain());
	QString sclipNormGain = "Norm: "+ coefficient_to_dbstring(m_clip->get_norm_factor());
	
	QString sMuted = "";
	if (m_clip->is_muted())
		sMuted = "M";
	
	QString clipInfo = clipName  + "    " + sclipGain + "   " + sclipNormGain + "    " + sRate +  " Hz";
	int clipInfoAreaWidth = 700;
	int x=5;

	clipNamePixmapActive = QPixmap(clipInfoAreaWidth, CLIP_INFO_AREA_HEIGHT);
	clipNamePixmapInActive = QPixmap(clipInfoAreaWidth, CLIP_INFO_AREA_HEIGHT);
	
	clipNamePixmapActive.fill(themer()->get_color("CLIPNAME_ACTIVE"));
	clipNamePixmapInActive.fill(themer()->get_color("CLIPNAME_INACTIVE"));


	QPainter paint(&clipNamePixmapActive);
	paint.setRenderHint(QPainter::TextAntialiasing );
	paint.setPen(themer()->get_color("DARK_TEXT"));
	paint.setFont(themer()->get_font("AudioClip:title"));

	QRect r = QRect(5, 0, clipInfoAreaWidth, CLIP_INFO_AREA_HEIGHT);
	paint.drawText( r, Qt::AlignVCenter, clipInfo);


	QPainter painter(&clipNamePixmapInActive);
	painter.setRenderHint(QPainter::TextAntialiasing );
	painter.setPen(themer()->get_color("DARK_TEXT"));
	painter.setFont(themer()->get_font("AudioClip:title"));
	painter.drawText( r, Qt::AlignVCenter, clipInfo);
}

void AudioClipView::update_progress_info( int progress )
{
	PENTER4;
	if (progress > (m_progress + 4)) {
		m_progress = progress;
		update();
	}
}

void AudioClipView::peaks_creation_finished( )
{
	waitingForPeaks = false;
	update();
}

AudioClip * AudioClipView::get_clip( )
{
	return m_clip;
}

void AudioClipView::gain_changed( )
{
	recreate_clipname_pixmap();
	update();
}

void AudioClipView::add_new_fadeview( FadeCurve * fade )
{
	PENTER;
	FadeView* view = new FadeView(m_sv, this, fade);
	m_fadeViews.append(view);
	scene()->addItem(view);
}

void AudioClipView::remove_fadeview( FadeCurve * fade )
{
	for (int i = 0; i < m_fadeViews.size(); ++i) {
		FadeView* view = m_fadeViews.at(i);
		if (view->get_fade() == fade) {
			m_fadeViews.takeAt(i);
			delete view;
			break;
		}
	}
}


void AudioClipView::calculate_bounding_rect()
{
	set_height(m_tv->get_clipview_height());
	m_boundingRectangle = QRectF(0, 0, (m_clip->get_length() / m_sv->scalefactor), m_height + CLIPINFO_HEIGHT);
	update_start_pos();
	update();
}


void AudioClipView::mousePressEvent ( QGraphicsSceneMouseEvent*  )
{
	PENTER;
}

void AudioClipView::mouseMoveEvent( QGraphicsSceneMouseEvent * event )
{
	PENTER;
	
	QPointF newPos(mapToParent(event->pos()) - event->buttonDownPos(Qt::LeftButton));
	
	printf("newPos x, y is %f, %f\n", event->pos().x(), event->pos().y());
	
	TrackView* trackView = m_sv->get_trackview_under(event->scenePos());
	if (!trackView) {
		printf("no trackview returned\n");
	} else if (trackView != m_tv) {
		printf("Setting new TrackView!\n");
		m_tv = trackView;
		setParentItem(trackView);
	}
		
	if (newPos.x() < 0) 
		newPos.setX(0);
	
	newPos.setY(pos().y());
	setPos(newPos);
}

void AudioClipView::repaint( )
{
	update();
}

void AudioClipView::set_height( int height )
{
	prepareGeometryChange();
	if (m_height < (MINIMAL_CLIPINFO_HEIGHT + CLIPINFO_HEIGHT)) {
		m_height = height;
	} else {
		m_height = height - CLIPINFO_HEIGHT;
	}
// 	calculate_bounding_rect();
}

int AudioClipView::get_fade_y_offset() const
{
	return CLIPINFO_HEIGHT;
}

void AudioClipView::update_start_pos()
{
	setPos(m_clip->get_track_start_frame() / m_sv->scalefactor, m_tv->get_clipview_y_offset());
}


Command* AudioClipView::drag()
{
	return new MoveClip(m_sv, this, m_clip);
}

Command* AudioClipView::drag_edge()
{
	Q_ASSERT(m_song);
	int x = (int) ( cpointer().scene_pos() - scenePos()).x();

	MoveEdge* me;

	if (x < (m_boundingRectangle.width() / 2))
		me =   new  MoveEdge(this, m_sv, "set_left_edge");
	else
		me = new MoveEdge(this, m_sv, "set_right_edge");

	return me;
}

Command* AudioClipView::split()
{
	Q_ASSERT(m_song);
	return new SplitClip(m_sv, m_clip);
}

void AudioClipView::position_changed( )
{
	prepareGeometryChange();
	calculate_bounding_rect();
}


//eof
