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

$Id: AudioClipView.cpp,v 1.14 2006/06/16 20:21:47 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include <QPainter>
#include <QPainterPath>

#include "AudioClipView.h"
#include "ColorManager.h"
#include "TrackView.h"
#include "SongView.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const int MINIMAL_CLIPINFO_HEIGHT = 30;
static const int CLIPINFO_HEIGHT = 14;

AudioClipView::AudioClipView(ViewPort * vp, TrackView* parent, AudioClip* clip )
		: ViewItem(vp, parent, clip), m_tv(parent), m_clip(clip)
{
	PENTERCONS;
	clipNamePixmapActive = QPixmap();
	clipNamePixmapInActive = QPixmap();

	m_muted = m_clip->is_muted();
	waitingForPeaks = false;
	m_progress = 0;
	m_song = m_clip->get_song();
	recreate_clipname_pixmap();
	
	connect(m_clip, SIGNAL(muteChanged(bool )), this, SLOT(mute_changed(bool )));
	connect(m_clip, SIGNAL(stateChanged()), this, SLOT(schedule_for_repaint()));
	connect(m_clip, SIGNAL(gainChanged()), this, SLOT (gain_changed()));
	connect(m_clip, SIGNAL(positionChanged()), m_tv, SLOT (repaint_all_clips()));
	connect(m_clip->get_fade_in(), SIGNAL(stateChanged()), this, SLOT(schedule_for_repaint()));
	connect(m_clip->get_fade_out(), SIGNAL(stateChanged()), this, SLOT(schedule_for_repaint()));
	
	create_fade_selectors();
	
	connect(&fadeInShapeSelector, SIGNAL(triggered ( QAction* )), this, SLOT(set_fade_in_shape( QAction* )));
	connect(&fadeOutShapeSelector, SIGNAL(triggered ( QAction* )), this, SLOT(set_fade_out_shape( QAction* )));

	update_geometry();
	init_context_menu( this );
	m_type = AUDIOCLIPVIEW;
}

AudioClipView::~ AudioClipView()
{
	PENTERDES;
	// FIXME If I enable this, it crashes on Traverso exit !
	// Is it needed? As soon as an object is destroyed, the connection is disconnected by Qt
	// automatically !!
	// 	disconnect(m_clip, SIGNAL(stateChanged()), this, SLOT(schedule_for_repaint()));
}

QRect AudioClipView::draw(QPainter& p)
{
	PENTER;

	if (m_clip->is_recording()) {
		// For now, just exit. For later, draw the recording audio :-)
		return QRect();
	}


	update_variables();


	if ( m_song->frame_to_xpos(trackFirstFrame) < 0) {
		startFrame += (-1 * m_song->frame_to_xpos(trackFirstFrame)) * Peak::zoomStep[hzoom];
		clipXWidth -= (-1 * m_song->frame_to_xpos(trackFirstFrame));
		baseX = m_tv->cliparea_basex();
	}

	if (clipXWidth > clipAreaWidth)
		clipXWidth = clipAreaWidth;
		
	// Check if the clipwidth is larger then 0!
	if ( (clipXWidth <=0) || (m_song->frame_to_xpos(trackLastFrame) <= 0) || (height == 0)) {
		return QRect();
	}
	
	QPixmap pix(clipXWidth, height);
	pix.fill(cm().get("CLIP_BG_DEFAULT"));
	QPainter painter(&pix);



	int normalBackgroundWidth = clipXWidth - rCrossX - lCrossX;

	if (crossAtLeft && crossAtRight) {
		crossAtLeft = crossAtRight = 0;
		normalBackgroundWidth = clipXWidth;
	}

	if (normalBackgroundWidth > 0 ) {
		// 		PWARN("Drawing normal background");
		// main bg (dont paint under crossings)
		if (m_muted)
			painter.fillRect( lCrossX, 0, normalBackgroundWidth, height, cm().get("CLIP_BG_MUTED"));
		else if (m_clip->is_selected())
			painter.fillRect( lCrossX, 0, normalBackgroundWidth, height, cm().get("CLIP_BG_SELECTED"));
		else
			painter.fillRect( lCrossX, 0, normalBackgroundWidth, height, cm().get("CLIP_BG_DEFAULT"));
	} else {
		PWARN("Covered completely by crossings");
	}


	// Only continue if our AudioClip has more then 1 channel
	if (channels == 0) {
		return QRect();
	}


	if (waitingForPeaks) {
		// Hmm, do we paint here something?
		// Progress info, I think so....
		painter.setPen(Qt::black);
		QRect r(clipXWidth/10, 0, 180, height);
		painter.setFont( QFont( "Bitstream Vera Sans", 12 ) );
		QString si;
		si.setNum(m_progress);
		QString buildProcess = "Building Peaks: " + si + "%";
		painter.drawText(r, Qt::AlignVCenter, buildProcess);
	
	} else {
		
		// 	draw_crossings(p);
	
		draw_peaks(painter);
	
		draw_fade_in(painter);
		draw_fade_out(painter);
	
	}

	if (m_clip->get_height() < (MINIMAL_CLIPINFO_HEIGHT + CLIPINFO_HEIGHT)) {
		p.drawPixmap(baseX, baseY, pix);
	} else {
		draw_clipinfo_area(p);
		p.drawPixmap(baseX, baseY + CLIPINFO_HEIGHT, pix);
	}

        // Black Contour of clip Info area
        p.setPen(cm().get("DARK_TEXT")); // CHANGE TO CLIP_COUNTOUR
        p.drawRect(baseX, baseY , clipXWidth , CLIPINFO_HEIGHT);
        p.drawRect(baseX, baseY , clipXWidth , height + CLIPINFO_HEIGHT);
        p.setPen(Qt::blue);
	PMESG("drawing clip");
	
	return QRect();
}


void AudioClipView::draw_peaks( QPainter& p )
{
	int normalBackgroundWidth = clipXWidth - rCrossX - lCrossX;
	if (normalBackgroundWidth < 0)
		return;

	const int height = this->height / channels;
	Peak* peak;
	nframes_t nframes, bufferPos;
	nframes = clipXWidth;
	int posY, negY, centerY;
	unsigned char* lowerHalf;
	unsigned char* upperHalf;
	bool microView = hzoom > Peak::MAX_ZOOM_USING_SOURCEFILE ? 0 : 1;
	bool classicView = true;


	for (int chan=0; chan < channels; chan++) {
		peak = m_clip->get_peak_for_channel(chan);
		nframes = peak->prepare_buffer_for_zoomlevel(hzoom, startFrame, nframes);

		if (nframes <= 0) {
			// It seems there are no peak buffers yet, but they are now generated
			// just wait for the finished() signal.....
// 			PWARN("Waiting for peak");
			waitingForPeaks = true;
			connect(peak, SIGNAL(progress( int )), this, SLOT(update_progress_info( int )));
			connect(peak, SIGNAL(finished()), this, SLOT (peaks_creation_finished()));
			return;
		}

		bufferPos = (nframes_t) (startFrame / Peak::zoomStep[hzoom]);
		upperHalf = peak->get_prepared_peakbuffer()->get_lower_half_buffer();
		lowerHalf = peak->get_prepared_peakbuffer()->get_lower_half_buffer();

		float scaleFactor = ( ((float)(height)) / (Peak::MAX_DB_VALUE * 2)) * gain;
		centerY = height/2 + height*chan;

		if (microView) {
			int microBufferPos = 0;
			short* buf = peak->get_prepared_peakbuffer()->get_microview_buffer();
			int prev =  (int) (centerY + (scaleFactor * buf[0]));
			p.setPen(cm().get("CLIP_PEAK_MICROVIEW"));

			for (uint x = 0; x < nframes; x++) {
				posY = (int) (centerY + (scaleFactor * buf[microBufferPos]));
				p.drawLine(x, prev, x+1, posY);
				prev = posY;
				microBufferPos++;
			}
		} else if (classicView) {
			p.setPen(cm().get("CLIP_PEAK_MACROVIEW"));

			for (uint x = 0; x < nframes; x++) {
                                posY = (int) (centerY + (scaleFactor * upperHalf[bufferPos]));
                                negY = (int) (centerY - (scaleFactor * lowerHalf[bufferPos]));
				p.drawLine(x, negY, x, posY);
				bufferPos++;
			}
		} else {
			scaleFactor = ( ( (float) height ) / Peak::MAX_DB_VALUE) * gain;
			centerY = height*(chan+1);
			p.setPen(cm().get("CLIP_PEAK_MACROVIEW"));

			for (uint x = 0; x < nframes; x++) {
                                posY = (int) (centerY - (scaleFactor * (f_max(upperHalf[bufferPos], lowerHalf[bufferPos]))));
				p.drawLine(x, centerY, x, posY);
				bufferPos++;
			}
		}
	}
}


void AudioClipView::draw_fade_in( QPainter& painter )
{
	Curve* fadeIn = m_clip->get_fade_in();
	
	if (fadeIn->get_range() == 0) {
		return;
	}
	
	QPolygonF polygon;
	float value[2];
	int step = 0;
	int startPos = 0;
	nframes_t startFrame = m_clip->get_track_start_frame();
	nframes_t endFrame = (nframes_t) fadeIn->get_range() + startFrame;
	nframes_t firstVisibleFrame = m_song->get_first_visible_frame();
	
	if (endFrame < firstVisibleFrame) {
		return;
	}
	
	if (firstVisibleFrame > startFrame) {
		startPos = (firstVisibleFrame - startFrame) / Peak::zoomStep[hzoom];
		startFrame = firstVisibleFrame;
	}
	
	int fadeWidth = (endFrame - startFrame) / Peak::zoomStep[hzoom];
	
	if (fadeWidth > clipXWidth) {
		fadeWidth = clipXWidth;
	}
	
	
// 	PWARN("fadeWidth is %d", fadeWidth);
	
	// Populate the polygon with enough points to draw a smooth curve.
	// using a 2 pixel resolution, is sufficient enough
	for (int i=startPos; i < (fadeWidth+startPos); i+=2) {
		fadeIn->get_vector(i*Peak::zoomStep[hzoom], i*Peak::zoomStep[hzoom] + 1, value, 2);
	 	polygon << QPointF( step, height - (value[1] * height) );
	 	step += 2;
	}
	
	// Always add the uppermost point in the polygon path 
	// since the above routine potentially does not include it.
	polygon << QPoint(fadeWidth, 0);
	
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing);
	
	QPainterPath path;
	path.moveTo(0, height);
	path.addPolygon(polygon);
	path.lineTo(0, 0);
	path.closeSubpath();
	
	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor(255, 0, 255, 50));
	
	painter.drawPath(path);	
	painter.restore();
}

void AudioClipView::draw_fade_out(QPainter& painter) 
{
	Curve* fadeOut = m_clip->get_fade_out();
	
	if (fadeOut->get_range() == 0) {
		return;
	}
	
	QPolygonF polygon;
	float value[2];
	int step = 0;
	int startPos = 0;
	nframes_t endFrame = m_clip->get_track_end_frame();
	nframes_t startFrame = endFrame - (nframes_t) fadeOut->get_range();
	nframes_t firstVisibleFrame = m_song->get_first_visible_frame();
	
	
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
	
	
	int fadeWidth = clipXWidth - startX;
	
	
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
		fadeOut->get_vector(i*Peak::zoomStep[hzoom], i*Peak::zoomStep[hzoom] + 1, value, 2);
	 	polygon << QPointF( step+startX, height - (value[1] * height) );
	 	step += 2;
	}
	
	// Always add the lowest point in the polygon path 
	// since the above routine potentially does not include it.
	polygon << QPoint(startX + fadeWidth, height);
	
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing);
	
	QPainterPath path;
	path.addPolygon(polygon);
	path.lineTo(startX + fadeWidth, 0);
	path.lineTo(startX, 0);
	path.closeSubpath();
	
	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor(255, 0, 255, 50));
	
	painter.drawPath(path);	
	painter.restore();
}


void AudioClipView::draw_crossings( QPainter& p )
{
	// bg under RIGHT crossfade region  ( only RIGHT !)
	p.fillRect( clipXWidth - rCrossX , 0, rCrossX, height, cm().get("CLIP_RIGHT_CROSSFADE"));

	if (crossAtLeft && crossAtRight)
		return;
	if (crossAtLeft)
		p.drawArc(lCrossX, 0, lCrossX * 2 , height * 2 , 90*16, -90*16);
	if (crossAtRight)
		p.drawArc(clipXWidth - rCrossX, 0, rCrossX * 2, height * 2, 90*16, 90*16);
}


void AudioClipView::draw_clipinfo_area( QPainter& p )
{
	// clip info area bg
	if (m_clip->get_track()->is_active())
		p.fillRect( baseX , baseY, clipXWidth, 16, cm().get("CLIP_INFO_AREA_BG_ACTIVE"));
	else
		p.fillRect( baseX , baseY, clipXWidth, 16, cm().get("CLIP_INFO_AREA_BG"));


	// Draw Clip Info Area
	if (clipXWidth>70) {
		if (m_clip->get_track()->is_active())
			p.drawPixmap(baseX, baseY, clipNamePixmapActive, 0, 0, clipXWidth, CLIP_INFO_AREA_HEIGHT);
		else
			p.drawPixmap(baseX, baseY, clipNamePixmapInActive, 0, 0, clipXWidth, CLIP_INFO_AREA_HEIGHT);
	}

}


void AudioClipView::recreate_clipname_pixmap()
{
	PENTER;
	int channels = m_clip->get_channels();
	Q_ASSERT(channels < 8);
	
	int rate = m_clip->get_rate();
	int bitDepth = m_clip->get_bitdepth();
	bool isTake = m_clip->is_take();
	float gain = m_clip->get_gain();
	QString clipName = m_clip->get_name();

	sRate.setNum(rate);
	sBitDepth.setNum(bitDepth);
	sourceType=(isTake?"CAP":"SRC");
	
	float db = coefficient_to_dB(gain);
	float norm = coefficient_to_dB(m_clip->get_norm_factor());
	QString gainIndB;
	QString normIndB;
	
	if (db < -99)
		gainIndB = "- INF";
	else if ( db < 0)
		gainIndB = "- " + QByteArray::number((-1 * db), 'f', 1) + " dB";
	else
		gainIndB = "+ " + QByteArray::number(db, 'f', 1) + " dB";
	
	sclipGain = "Gain "+gainIndB;
	
	if (norm < -99)
		normIndB = "- INF";
	else if ( norm < 0)
		normIndB = "- " + QByteArray::number((-1 * norm), 'f', 1) + " dB";
	else
		normIndB = "+ " + QByteArray::number(norm, 'f', 1) + " dB";
	
	QString sclipNormGain = "Normalization "+ normIndB;
	
	sMuted = "";
	if (m_muted)
		sMuted = "M";
	
	clipInfo = sMuted + "  " + sRate +  "  " + sBitDepth + "   " + sourceType + "  " + sclipGain + "   " + sclipNormGain + "   " + clipName;
	int clipInfoAreaWidth = 500;
	int x=5;

	clipNamePixmapActive = QPixmap(clipInfoAreaWidth, CLIP_INFO_AREA_HEIGHT);
	clipNamePixmapInActive = QPixmap(clipInfoAreaWidth, CLIP_INFO_AREA_HEIGHT);
	clipNamePixmapActive.fill(cm().get("CLIPNAME_ACTIVE"));
	clipNamePixmapInActive.fill(cm().get("CLIPNAME_INACTIVE"));


	QPainter paint(&clipNamePixmapActive);
	paint.setRenderHint(QPainter::TextAntialiasing );
	paint.setPen(cm().get("DARK_TEXT"));
	paint.setFont( QFont( "Bitstream Vera Sans", 8 ) );
	int deltaX = 0;
	for (int i=0; i<channels; ++i) {
		paint.drawEllipse( x + 4*i, 3, 7 , 7);
		deltaX += 4*i;
	}
	x += 15 + deltaX;

	QRect r = QRect(x, 0, clipInfoAreaWidth, CLIP_INFO_AREA_HEIGHT);
	paint.drawText( r, Qt::AlignVCenter, clipInfo);


	QPainter painter(&clipNamePixmapInActive);
	painter.setRenderHint(QPainter::TextAntialiasing );
	painter.setPen(cm().get("DARK_TEXT"));
	painter.setFont( QFont( "Bitstream Vera Sans", 8 ) );
	x = 5;
	deltaX = 0;
	for (int i=0; i<channels; ++i) {
		painter.drawEllipse( x + 4*i, 3, 7 , 7);
		deltaX += 4*i;
	}
	x += 15 + deltaX;
	painter.drawText( r, Qt::AlignVCenter, clipInfo);
}

void AudioClipView::mute_changed( bool mute )
{
	PENTER;
	m_muted = mute;
	schedule_for_repaint();
}

void AudioClipView::schedule_for_repaint( )
{
	PENTER2;
	update_geometry();
	if (visible())
		m_vp->schedule_for_repaint(this);
}

void AudioClipView::update_geometry( )
{
	PENTER2;
	nframes_t trackFirstBlock =  m_clip->get_track_start_frame();
	int baseX = m_song->frame_to_xpos(trackFirstBlock) + m_tv->cliparea_basex();
	int clipWidth = m_clip->get_width();
	int baseY  = m_clip->get_baseY();
	int height = m_clip->get_height();

	if ( m_song->frame_to_xpos(trackFirstBlock) < 0) {
		// if the above is true, clipWidth += m_song->frame_to_xpos(trackFirstBlock)
		// will actually substract which is what we wanted :-)
		clipWidth += m_song->frame_to_xpos(trackFirstBlock);
		if (clipWidth < 0)
			clipWidth = 0;
		baseX = m_tv->cliparea_basex();
	}

	set_geometry(baseX, baseY, clipWidth, height);
}

void AudioClipView::update_progress_info( int progress )
{
	PENTER;
	m_progress = progress;
	schedule_for_repaint();
}

void AudioClipView::peaks_creation_finished( )
{
	waitingForPeaks = false;
	schedule_for_repaint();
}

AudioClip * AudioClipView::get_clip( )
{
	return m_clip;
}

void AudioClipView::update_variables( )
{
	sourceLastFrame = m_clip->get_source_end_frame();
	sourceFirstFrame = m_clip->get_source_start_frame();
	trackFirstFrame =  m_clip->get_track_start_frame();
	trackLastFrame = m_clip->get_track_end_frame();
	startFrame = sourceFirstFrame;
	gain = m_clip->get_gain() * m_clip->get_norm_factor();
	hzoom = m_song->get_hzoom();
	baseY  = m_clip->get_baseY();
	baseX = m_song->frame_to_xpos(trackFirstFrame) + m_tv->cliparea_basex();
	height = m_clip->get_height() - CLIPINFO_HEIGHT;
	if (height < MINIMAL_CLIPINFO_HEIGHT)
		height = m_clip->get_height();
	clipXWidth = m_clip->get_width();
	clipAreaWidth = m_tv->cliparea_width();
	channels = m_clip->get_channels();

	// Check Cross Fades
	prevClip = m_clip->prev_clip();
	nextClip = m_clip->next_clip();
	prevTLF = prevClip ? prevClip->get_track_end_frame() : 0;
	nextTFF = nextClip ? nextClip->get_track_start_frame() : 0;
	crossAtLeft  = ((prevClip) && (prevTLF > trackFirstFrame));
	crossAtRight = ((nextClip) && (nextTFF < trackLastFrame));
	lCrossX = crossAtLeft  ? m_song->frame_to_xpos(prevTLF) - m_song->frame_to_xpos(trackFirstFrame) : 0;
	rCrossX = crossAtRight ? m_song->frame_to_xpos(trackLastFrame) - m_song->frame_to_xpos(nextTFF)  : 0;

	if (crossAtRight && nextClip && (nextClip->get_track_end_frame() < trackLastFrame)) {
		lCrossX = rCrossX = 0;
	}
}

Command * AudioClipView::select_fade_in_shape( )
{
	fadeInShapeSelector.exec(QCursor::pos());
	
	return (Command*) 0;
}

Command * AudioClipView::select_fade_out_shape( )
{
	fadeOutShapeSelector.exec(QCursor::pos());
	
	return (Command*) 0;
}

void AudioClipView::set_fade_in_shape( QAction * action )
{
	m_clip->set_fade_in_shape( (AudioClip::FadeShape) action->data().toInt(), (uint) m_clip->get_fade_in()->get_range());
}

void AudioClipView::set_fade_out_shape( QAction * action )
{
	m_clip->set_fade_out_shape( (AudioClip::FadeShape) action->data().toInt(), (uint) m_clip->get_fade_out()->get_range());
}

void AudioClipView::create_fade_selectors( )
{
	QStringList names;
	names << tr("Fastest") << tr("Fast") << tr("Linear")  << tr("Slow") << tr("Slowest");
	int shape = 0;
	
	foreach(QString name, names) {
		QAction* action = fadeOutShapeSelector.addAction(name);
		action->setData(shape);
		
		action = fadeInShapeSelector.addAction(name);
		action->setData(shape++);
	}
}

void AudioClipView::gain_changed( )
{
	recreate_clipname_pixmap();
	schedule_for_repaint();
}

//eof
