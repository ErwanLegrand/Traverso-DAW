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

$Id: AudioClipView.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include <QPainter>

#include "AudioClipView.h"
#include "ColorManager.h"
#include "TrackView.h"
#include "SongView.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

AudioClipView::AudioClipView(ViewPort * vp, TrackView* parent, AudioClip* clip )
		: ViewItem(vp, parent, clip), m_tv(parent), m_clip(clip)
{
	PENTERCONS;
	clipNamePixmapActive = QPixmap();
	clipNamePixmapInActive = QPixmap();

	m_muted = m_clip->is_muted();
	waitingForPeaks = false;
	m_progress = 0;
	m_song = m_clip->get_parent_song();
	recreate_clipname_pixmap();

	connect(m_clip, SIGNAL(muteChanged(bool )), this, SLOT(mute_changed(bool )));
	connect(m_clip, SIGNAL(stateChanged()), this, SLOT(schedule_for_repaint()));
	connect(m_clip, SIGNAL(trackStartFrameChanged()), m_tv, SLOT (repaint_all_clips()));
	connect(m_clip, SIGNAL(edgePositionChanged()), m_tv, SLOT (repaint_all_clips()));


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


	// Check if the clipwidth is larger then 0!
	if ( (clipXWidth <=0) || (m_song->block_to_xpos(trackLastFrame) <= 0) ) {
		return QRect();
	}

	if ( m_song->block_to_xpos(trackFirstFrame) < 0) {
		startFrame += (-1 * m_song->block_to_xpos(trackFirstFrame)) * Peak::zoomStep[hzoom];
		clipXWidth -= (-1 * m_song->block_to_xpos(trackFirstFrame));
		baseX = m_tv->cliparea_basex();
	}

	if (clipXWidth > clipAreaWidth)
		clipXWidth = clipAreaWidth;


	draw_clipinfo_area(p);

	int normalBackgroundWidth = clipXWidth - rCrossX - lCrossX;

	if (crossAtLeft && crossAtRight) {
		crossAtLeft = crossAtRight = 0;
		normalBackgroundWidth = clipXWidth;
	}

	if (normalBackgroundWidth > 0 ) {
		// 		PWARN("Drawing normal background");
		// main bg (dont paint under crossings)
		if (m_muted)
			p.fillRect( baseX + lCrossX, baseY+16, normalBackgroundWidth, height-16, cm().get("CLIP_BG_MUTED"));
		else if (m_clip->is_selected())
			p.fillRect( baseX + lCrossX, baseY+16, normalBackgroundWidth, height-16, cm().get("CLIP_BG_SELECTED"));
		else
			p.fillRect( baseX + lCrossX, baseY+16, normalBackgroundWidth, height-16 , cm().get("CLIP_BG_DEFAULT"));
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
		p.setPen(Qt::black);
		QRect r(baseX + clipXWidth/10, m_clip->get_baseY(), 180, height);
		p.setFont( QFont( "Bitstream Vera Sans", 12 ) );
		QString si;
		si.setNum(m_progress);
		QString buildProcess = "Building Peaks: " + si + "%";
		p.drawText(r, Qt::AlignVCenter, buildProcess);
		return QRect();
	}

	// 	draw_crossings(p);

	draw_peaks(p);

	// 	draw_fades(p);
	//


	// Black Contour of clip Info area
	p.setPen(cm().get("DARK_TEXT")); // CHANGE TO CLIP_COUNTOUR
	p.drawRect(baseX, baseY , clipXWidth , 16);
	p.drawRect(baseX, baseY , clipXWidth , height);
	p.setPen(Qt::blue);

	PMESG("drawing clip");

	return QRect();
}


void AudioClipView::draw_peaks( QPainter& p )
{
	int normalBackgroundWidth = clipXWidth - rCrossX - lCrossX;
	if (normalBackgroundWidth < 0)
		return;

	const int channelHeight = (height-16) / channels;
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
			PWARN("Waiting for peak");
			waitingForPeaks = true;
			connect(peak, SIGNAL(progress( int )), this, SLOT(update_progress_info( int )));
			connect(peak, SIGNAL(finished()), this, SLOT (peaks_creation_finished()));
			return;
		}

		bufferPos = (nframes_t) (startFrame / Peak::zoomStep[hzoom]);
		upperHalf = peak->get_prepared_peakbuffer()->get_lower_half_buffer();
		lowerHalf = peak->get_prepared_peakbuffer()->get_lower_half_buffer();

		float scaleFactor = ( ((float)(channelHeight - 6)) / 512) * gain;
		centerY = channelHeight/2 + channelHeight*chan + 16 + baseY;

		if (microView) {
			int prev = 0;
			int microBufferPos = 0;
			short* buf = peak->get_prepared_peakbuffer()->get_microview_buffer();
			p.setPen(cm().get("CLIP_PEAK_MICROVIEW"));

			for (uint x = baseX; x < (nframes + baseX); x++) {
				posY = (int) (centerY + (scaleFactor * buf[microBufferPos]));
				p.drawLine(x, prev, x+1, posY);
				prev = posY;
				microBufferPos++;
			}
		} else if (classicView) {
			p.setPen(cm().get("CLIP_PEAK_MACROVIEW"));

			for (uint x = baseX; x < (nframes + baseX); x++) {
				posY = (int) (centerY + (scaleFactor * upperHalf[bufferPos]));
				negY = (int) (centerY - (scaleFactor * lowerHalf[bufferPos]));
				p.drawLine(x, negY, x, posY);
				bufferPos++;
			}
		} else {
			scaleFactor = ( ( (float) (channelHeight - 6) ) / 256) * gain;
			centerY = channelHeight*(chan+1)+ 16 + baseY;
			p.setPen(cm().get("CLIP_PEAK_MACROVIEW"));

			for (uint x = baseX; x < (nframes + baseX); x++) {
				posY = (int) (centerY - (scaleFactor * (f_max(upperHalf[bufferPos], lowerHalf[bufferPos]))));
				p.drawLine(x, centerY, x, posY);
				bufferPos++;
			}
		}
	}
}


void AudioClipView::draw_fades( QPainter& p )
{
	if ((fadeInFrames == 0) && (fadeOutFrames == 0))
		return;

	// Calculate fades in/out and clip gain
	int xrbfi = baseX ;
	int xrbfo = baseX + clipXWidth;
	int gy = (int) ( (float ) baseY + height - gain * ( height - 16 ) );
	/*	if (ie().get_current_mode() == m_song->EditingMode)
			{*/
	if (fadeInFrames>0)
		xrbfi = m_song->block_to_xpos(trackFirstFrame + fadeInFrames) + m_tv->cliparea_basex();
	if (fadeOutFrames>0)
		xrbfo = m_song->block_to_xpos(trackFirstFrame + ( sourceLastFrame - sourceFirstFrame)  - fadeOutFrames) + m_tv->cliparea_basex();
	// 		}
	if (xrbfi < 0 )
		xrbfi = 0;
	if (xrbfo > (baseX + clipXWidth))
		xrbfo = baseX + clipXWidth;

	// draw fades in/out and clip gain (shown only in normal editing mode)
	p.setPen(cm().get("MAGENTA"));
	if (fadeInFrames>0)
		p.drawLine(baseX, baseY + height, xrbfi, gy);
	if (fadeOutFrames>0)
		p.drawLine(baseX + clipXWidth - 1, baseY + height ,xrbfo, gy );
	if (gain<1.0)
		p.drawLine(xrbfi, gy,   xrbfo , gy );
}


void AudioClipView::draw_crossings( QPainter& p )
{
	// bg under RIGHT crossfade region  ( only RIGHT !)
	p.fillRect( baseX + clipXWidth - rCrossX , baseY+16, rCrossX, height-16, cm().get("CLIP_RIGHT_CROSSFADE"));

	if (crossAtLeft && crossAtRight)
		return;
	if (crossAtLeft)
		p.drawArc(baseX - lCrossX, baseY + 16, lCrossX * 2 , (height-16)*2 , 90*16, -90*16);
	if (crossAtRight)
		p.drawArc(baseX + clipXWidth - rCrossX, baseY + 16, rCrossX * 2, (height-16) * 2, 90*16, 90*16);
}


void AudioClipView::draw_clipinfo_area( QPainter& p )
{
	// clip info area bg
	if (m_clip->get_parent_track()->is_active())
		p.fillRect( baseX , baseY, clipXWidth, 16, cm().get("CLIP_INFO_AREA_BG_ACTIVE"));
	else
		p.fillRect( baseX , baseY, clipXWidth, 16, cm().get("CLIP_INFO_AREA_BG"));


	// Draw Clip Info Area
	if (clipXWidth>70) {
		if (m_clip->get_parent_track()->is_active())
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
	sclipGain.setNum((double)gain,'f',1);
	sclipGain = "G:"+sclipGain;
	sMuted = "";
	if (m_muted)
		sMuted = "M";
	clipInfo = sMuted + "  " + sRate +  "  " + sBitDepth + "   " + sourceType + "  " + sclipGain + "   " + clipName;
	int clipInfoAreaWidth = 500;
	int x=5;

	clipNamePixmapActive = QPixmap(clipInfoAreaWidth, CLIP_INFO_AREA_HEIGHT);
	clipNamePixmapInActive = QPixmap(clipInfoAreaWidth, CLIP_INFO_AREA_HEIGHT);
	clipNamePixmapActive.fill(cm().get("CLIPNAME_ACTIVE"));
	clipNamePixmapInActive.fill(cm().get("CLIPNAME_INACTIVE"));


	QPainter paint(&clipNamePixmapActive);
	paint.setPen(cm().get("DARK_TEXT"));
	paint.setFont( QFont( "Bitstream Vera Sans", 9 ) );
	int deltaX = 0;
	for (int i=0; i<channels; ++i) {
		paint.drawEllipse( x + 6*i, 4, 10 , 7);
		deltaX += 4*i;
	}
	x += 15 + deltaX;

	QRect r = QRect(x, 0, clipInfoAreaWidth, CLIP_INFO_AREA_HEIGHT);
	paint.drawText( r, Qt::AlignVCenter, clipInfo);


	QPainter painter(&clipNamePixmapInActive);
	paint.setPen(cm().get("DARK_TEXT"));
	painter.setFont( QFont( "Bitstream Vera Sans", 9 ) );
	x = 5;
	deltaX = 0;
	for (int i=0; i<channels; ++i) {
		painter.drawEllipse( x + 4*i, 4, 8 , 8);
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
	nframes_t trackFirstBlock =  m_clip->get_track_first_block();
	int baseX = m_song->block_to_xpos(trackFirstBlock) + m_tv->cliparea_basex();
	int clipWidth = m_clip->get_width();
	int baseY  = m_clip->get_baseY();
	int height = m_clip->get_height();

	if ( m_song->block_to_xpos(trackFirstBlock) < 0) {
		// if the above is true, clipWidth += m_song->block_to_xpos(trackFirstBlock)
		// will actually substract which is what we wanted :-)
		clipWidth += m_song->block_to_xpos(trackFirstBlock);
		if (clipWidth < 0)
			clipWidth = 0;
		baseX = m_tv->cliparea_basex();
	}

	set_geometry(baseX, baseY, clipWidth, height);
}


Command* AudioClipView::clip_fade_in()
{
	if (ie().is_holding())
		process_fade_clip();
	return (Command*) 0;
}


Command* AudioClipView::clip_fade_out()
{
	if (ie().is_holding())
		process_fade_clip();
	return (Command*) 0;
}


Command* AudioClipView::clip_fade_both()
{
	if (ie().is_holding())
		process_fade_clip();
	return (Command*) 0;
}

Command* AudioClipView::jog_fade_clip()
{
	PENTER4;
	int x = cpointer().clip_area_x();
	int y = cpointer().y();
	if (ie().is_jogging()) {
		nframes_t b = (x - origX) * Peak::zoomStep[m_song->get_hzoom()];
		int dbi = origBlockL + b;
		dbi = dbi < 0? 0:dbi;
		m_clip->set_fade_in(dbi);
	} else if (ie().is_jogging()) {
		nframes_t b = (origX - x) * Peak::zoomStep[m_song->get_hzoom()];
		int dbo = origBlockR + b;
		dbo = dbo < 0? 0:dbo;
		m_clip->set_fade_out(dbo);
	} else if (ie().is_jogging()) {
		nframes_t b = (x - origX) * Peak::zoomStep[m_song->get_hzoom()];
		int dbi =  + b;
		dbi = dbi < 0? 0:dbi;
		int dbo = origBlockR + b;
		dbo = dbo < 0? 0:dbo;
		m_clip->set_fade_in(dbi);
		m_clip->set_fade_out(dbo);
	} else if (ie().is_jogging()) {
		int dy = (origY - y);
		float g = origGain + ( (float) dy / 200 );
		m_clip->set_gain(g);
	}
	return (Command*) 0;
}

void AudioClipView::process_fade_clip()
{
	PENTER;
	if ( ie().is_holding() ) {
		origX = cpointer().clip_area_x();
		origY = cpointer().y();
		origGain = m_clip->get_gain();
		origBlockL = m_clip->get_fade_in_blocks();
		origBlockR = m_clip->get_fade_out_blocks();
	}
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
	sourceLastFrame = m_clip->get_source_last_block();
	sourceFirstFrame = m_clip->get_source_first_block();
	trackFirstFrame =  m_clip->get_track_first_block();
	trackLastFrame = m_clip->get_track_last_block();
	fadeOutFrames = m_clip->get_fade_out_blocks();
	fadeInFrames = m_clip->get_fade_in_blocks();
	startFrame = sourceFirstFrame;
	gain = m_clip->get_gain();
	hzoom = m_song->get_hzoom();
	baseY  = m_clip->get_baseY();
	baseX = m_song->block_to_xpos(trackFirstFrame) + m_tv->cliparea_basex();
	height = m_clip->get_height();
	clipXWidth = m_clip->get_width();
	clipAreaWidth = m_tv->cliparea_width();
	channels = m_clip->get_channels();

	// Check Cross Fades
	prevClip = m_clip->prev_clip();
	nextClip = m_clip->next_clip();
	prevTLF = prevClip ? prevClip->get_track_last_block() : 0;
	nextTFF = nextClip ? nextClip->get_track_first_block() : 0;
	crossAtLeft  = ((prevClip) && (prevTLF > trackFirstFrame));
	crossAtRight = ((nextClip) && (nextTFF < trackLastFrame));
	lCrossX = crossAtLeft  ? m_song->block_to_xpos(prevTLF) - m_song->block_to_xpos(trackFirstFrame) : 0;
	rCrossX = crossAtRight ? m_song->block_to_xpos(trackLastFrame) - m_song->block_to_xpos(nextTFF)  : 0;

	if (crossAtRight && nextClip && (nextClip->get_track_last_block() < trackLastFrame)) {
		lCrossX = rCrossX = 0;
	}
}

//eof
