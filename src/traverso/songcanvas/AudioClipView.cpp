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

$Id: AudioClipView.cpp,v 1.25 2007/02/12 19:58:59 r_sijrier Exp $
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

AudioClipView::AudioClipView(SongView* sv, TrackView* parent, AudioClip* clip )
	: ViewItem(parent, clip), m_tv(parent), m_clip(clip)
{
	PENTERCONS;
	
	setZValue(parent->zValue() + 1);
	
	m_sv = sv;
	m_tv->scene()->addItem(this);
	
	clipNamePixmapActive = QPixmap();
	clipNamePixmapInActive = QPixmap();
	
	load_theme_data();

	m_waitingForPeaks = false;
	m_progress = m_peakloadingcount = 0;
	m_song = m_clip->get_song();
	
	calculate_bounding_rect();
	
// 	m_mergedView = config().get_property("AudioClip", "WaveFormMerged", 0).toInt() == 0 ? 0 : 1;
	
	if (FadeCurve* curve = m_clip->get_fade_in()) {
		add_new_fadeview(curve);
	}
	if (FadeCurve* curve = m_clip->get_fade_out()) {
		add_new_fadeview(curve);
	}
	
	curveView = new CurveView(m_sv, this, m_clip->get_gain_envelope());
	
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
	
	if (m_clip->is_recording()) {
		// For now, just exit. For later, draw the recording audio :-)
		return;
	}

// 	printf("AudioClipView:: PAINT :: exposed rect is: x=%f, y=%f, w=%f, h=%f\n", option->exposedRect.x(), option->exposedRect.y(), option->exposedRect.width(), option->exposedRect.height());
	
	int xstart = (int) option->exposedRect.x();
	int pixelcount = (int) option->exposedRect.width();
	if (pixelcount == 0) {
		PWARN("AudioClipView::paint : Exposed rectangle has 0 width ????");
		return;
	}
	
	
	QRectF clipRect = m_boundingRectangle;
	clipRect.setWidth(clipRect.width() + 1);
	clipRect.setHeight(clipRect.height());
	painter->setClipRect(clipRect);
	
	if (m_drawbackground) {
		bool mousehover = (option->state & QStyle::State_MouseOver);
	
		if (m_clip->is_muted()) {
			m_backgroundColor = themer()->get_color("AudioClip:background:muted");
			m_backgroundColorMouseHover = themer()->get_color("AudioClip:background:muted:mousehover");
		} else if (m_clip->is_selected()) {
			m_backgroundColor = themer()->get_color("AudioClip:background:selected");
			m_backgroundColorMouseHover = themer()->get_color("AudioClip:background:selected:mousehover");
		} else {
			m_backgroundColor = themer()->get_color("AudioClip:background");
			m_backgroundColorMouseHover = themer()->get_color("AudioClip:background:mousehover");
		}

		if (mousehover) {
			painter->fillRect(xstart, 0, pixelcount, m_height, m_backgroundColorMouseHover);
		} else {
			painter->fillRect(xstart, 0, pixelcount, m_height, m_backgroundColor);
		}
	}
	

	int channels = m_clip->get_channels();

	// Only continue if our AudioClip has more then 1 channel
	if (channels == 0) {
		PWARN("This AudioClipView has 0 channels!!!!!");
		return;
	}
	
	
	if (m_waitingForPeaks) {
		PMESG("Waiting for peaks!");
		// Hmm, do we paint here something?
		// Progress info, I think so....
		painter->setPen(Qt::black);
		QRect r(10, 0, 150, m_height);
		painter->setFont( QFont( "Bitstream Vera Sans", 11 ) );
		QString si;
		si.setNum((int)m_progress);
		if (m_progress == 100) m_progress = 0;
		QString buildProcess = "Building Peaks: " + si + "%";
		painter->drawText(r, Qt::AlignVCenter, buildProcess);
	
	} else {
		draw_peaks(painter, xstart, pixelcount);
	}

	
	// Draw the contour
	if (m_height < m_mimimumheightforinfoarea) {
		painter->setPen(themer()->get_color("AudioClip:contour"));
		painter->drawRect(xstart, 0, pixelcount, m_height - 1);
	} else {
		draw_clipinfo_area(painter, xstart, pixelcount);
		painter->setPen(themer()->get_color("AudioClip:contour"));
		painter->drawRect(xstart, 0, pixelcount, m_infoAreaHeight - 1);
		painter->drawRect(xstart, 0, pixelcount, m_height - 1);
	}
	
	// If the beginning of the clip is painted, add some effect to make it
	// more visible that it _is_ the start of the clip ?????????
	if (xstart == 0) {
/*		painter->setPen(themer()->get_color("AudioClip:contour").light(115));
		painter->drawLine(1, m_infoAreaHeight, 1, m_height);*/
	}
	
	if (!m_sv->viewmode == CurveMode) {
// 		curveView->paint(painter, option, widget);
	}

// 	printf("drawing clip\n");
	
}


void AudioClipView::draw_peaks(QPainter* p, int xstart, int pixelcount)
{
	PENTER2;
	
	// FIXME ?
	// when painting with a path, I _have_ to use path.lineTo()
	// which looks ugly when only parts of the clip is repainted
	// when using a different color for the brush then the outline.
	// Painting one more pixel makes it getting clipped away.....
	pixelcount += 1;
	int channels = m_clip->get_channels();
	
	bool microView = m_song->get_hzoom() > Peak::MAX_ZOOM_USING_SOURCEFILE ? 0 : 1;
	int peakdatacount = microView ? pixelcount : pixelcount * 2;

	int buffersize = microView ? sizeof(short) * peakdatacount : sizeof(unsigned char) * peakdatacount;
	unsigned char buffers[channels][buffersize];
	float pixeldata[channels][buffersize];
	
	
	// Load peak data for all channels, if no peakdata is returned
	// for a certain Peak object, schedule it for loading.
	for (int chan=0; chan < channels; chan++) {
		Peak* peak = m_clip->get_peak_for_channel(chan);
		int availpeaks = peak->calculate_peaks(buffers[chan], m_song->get_hzoom(), xstart * m_sv->scalefactor + m_clip->get_source_start_frame(), peakdatacount);
		
		if (peakdatacount != availpeaks) {
			PWARN("peakdatacount != availpeaks (%d, %d)", peakdatacount, availpeaks);
		}

		if (availpeaks <= 0) {
			m_peakloadinglist.append(peak);
			m_waitingForPeaks = true;
			m_peakloadingcount++;
		}
	}
	
	
	if (m_waitingForPeaks) {
		start_peak_data_loading();
		return;
	}
	
	
	// Load the Peak data into the pixeldata float buffers
	// ClassicView uses both positive and negative values,
	// rectified view: pick the highest value of both
	// Merged view: calculate highest value for all channels, 
	// and store it in the first channels pixeldata.
	// TODO mix curve pixel data with the pixeldata buffers!
	if (!microView) {
		for (int chan=0; chan < channels; chan++) {
			if (m_classicView) {
				for (int i = 0; i < (pixelcount*2); ++i) {
					pixeldata[chan][i] = buffers[chan][i];
					i++;
					pixeldata[chan][i] = buffers[chan][i];
				}
			} else {
				for (int i = 0; i < (pixelcount*2); i+=2) {
					pixeldata[chan][i] = - f_max(buffers[chan][i], - buffers[chan][i+1]);
					
				}
			}
		}
		
		if (m_mergedView) {
			for (int chan=1; chan < channels; chan++) {
				for (int i = 0; i < (pixelcount*2); ++i) {
					pixeldata[0][i] = f_max(pixeldata[chan - 1][i], pixeldata[chan][i]);
				}
			}
		}
		
	}
	
	
	for (int chan=0; chan < channels; chan++) {
		
		p->save();
	
		// calculate the height of the area available for peak drawing 
		// and if the infoarea is displayed, translate the painter
		// drawing by dy = m_infoAreaheight
		int height;
		
		if (m_height > m_mimimumheightforinfoarea) {
			p->setMatrix(matrix().translate(0, m_infoAreaHeight), true);
			height = (m_height - m_infoAreaHeight) / channels;
		} else {
			height = m_height / channels;
		}
	
		
		float scaleFactor = ( (float) height * 0.90 / (Peak::MAX_DB_VALUE * 2)) * m_clip->get_gain() * m_clip->get_norm_factor();
		float ytrans;
		
		// Draw channel seperator horizontal lines, if needed.
		if (channels >= 2 && ! m_mergedView && m_classicView && chan >=1 ) {
			p->save();
			
			if (m_clip->is_selected()) {
				p->setPen(themer()->get_color("AudioClip:channelseperator:selected"));
			} else {
				p->setPen(themer()->get_color("AudioClip:channelseperator"));
			}
		
			ytrans = height * chan;
			p->setMatrix(matrix().translate(1, ytrans), true);
			p->drawLine(xstart, 0, xstart + pixelcount, 0);
			p->restore();
		}
		
		// Microview, paint waveform as polyline
		if (microView) {
		
			p->setPen(themer()->get_color("AudioClip:wavemicroview"));
			
			QPolygon polygon;
			short* mbuffer = (short*) buffers[chan];
			int bufferPos = 0;
			
			for (int x = xstart; x < (pixelcount+xstart); x++) {
				polygon.append( QPoint(x, mbuffer[bufferPos++]) );
			}
			
			if (themer()->get_property("AudioClip:wavemicroview:antialiased", 0).toInt()) {
				p->setRenderHints(QPainter::Antialiasing);
			}
			
			ytrans = (height / 2) + (chan * height);
			
			p->setMatrix(matrix().translate(1, ytrans).scale(1, scaleFactor), true);
			p->drawPolyline(polygon);
		
		// Macroview, paint waveform with painterpath
		} else {
			
			if (m_sv->viewmode == EditMode) {
				p->setPen(themer()->get_color("AudioClip:wavemacroview:outline"));
				if (m_fillwave) {
					p->setBrush(themer()->get_color("AudioClip:wavemacroview:brush"));
				}
			} else  {
				p->setPen(themer()->get_color("AudioClip:wavemacroview:outline:curvemode"));
				if (m_fillwave) {
					p->setBrush(themer()->get_color("AudioClip:wavemacroview:brush:curvemode"));
				}
			}
			
			QPainterPath path;
			// in rectified view, we add an additional point, hence + 1
			QPolygonF polygontop(pixelcount + 1);
			int bufferpos = 0;
						
			if (m_classicView) {
				QPolygonF polygonbottom(pixelcount);
				
				for (int x = xstart; x < (pixelcount+xstart); x++) {
					polygontop.append( QPointF(x, pixeldata[chan][bufferpos++]) );
					polygonbottom.append( QPointF(x, - pixeldata[chan][bufferpos++]) );
				}
				
				path.addPolygon(polygontop);
				path.lineTo(polygonbottom.last());
				path.addPolygon(polygonbottom);
				
				ytrans = (height / 2) + (chan * height);
			
			} else {
				for (int x = xstart; x < (pixelcount+xstart); x++) {
					polygontop.append( QPointF(x, pixeldata[chan][bufferpos]) );
					bufferpos += 2;
				}
				
				polygontop.append(QPointF(xstart + pixelcount, 0));
				path.addPolygon(polygontop);
				path.lineTo(xstart, 0);
				
				ytrans = height + (chan * height);
				scaleFactor = 2 * ( (float) height * 0.95 / (Peak::MAX_DB_VALUE * 2)) * m_clip->get_gain() * m_clip->get_norm_factor();
			}
			
			if (m_mergedView) {
				if (m_classicView) {
					ytrans = (height / 2) * channels;
					scaleFactor *= channels;
				} else {
					ytrans = height * channels;
					scaleFactor *= channels;
				}
			}
			
			p->setMatrix(matrix().translate(1, ytrans).scale(1, scaleFactor), true);
			p->drawPath(path);	
		}
		
		p->restore();
		
		if (m_mergedView) {
			break;
		}
	}
}



void AudioClipView::draw_clipinfo_area(QPainter* p, int xstart, int pixelcount)
{
	// clip info area bg
	if (m_clip->get_track()->is_active())
		p->fillRect(xstart, 0, pixelcount, m_infoAreaHeight, themer()->get_color("AudioClip:clipinfobackground"));
	else
		p->fillRect(xstart, 0, pixelcount, m_infoAreaHeight, themer()->get_color("AudioClip:clipinfobackground:inactive"));


	// Draw Clip Info Area
	if (m_clip->get_track()->is_active())
		p->drawPixmap(0, 0, clipNamePixmapActive, 0, 0, 600, m_infoAreaHeight);
	else
		p->drawPixmap(0, 0, clipNamePixmapInActive, 0, 0, 600, m_infoAreaHeight);
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

	clipNamePixmapActive = QPixmap(clipInfoAreaWidth, m_infoAreaHeight);
	clipNamePixmapInActive = QPixmap(clipInfoAreaWidth, m_infoAreaHeight);
	
	clipNamePixmapActive.fill(themer()->get_color("AudioClip:clipinfobackground"));
	clipNamePixmapInActive.fill(themer()->get_color("AudioClip:clipinfobackground:inactive"));


	QPainter paint(&clipNamePixmapActive);
	paint.setRenderHint(QPainter::TextAntialiasing );
	paint.setPen(themer()->get_color("Text:dark"));
	paint.setFont(themer()->get_font("AudioClip:title"));

	QRect r = QRect(5, 0, clipInfoAreaWidth, m_infoAreaHeight);
	paint.drawText( r, Qt::AlignVCenter, clipInfo);


	QPainter painter(&clipNamePixmapInActive);
	painter.setRenderHint(QPainter::TextAntialiasing );
	painter.setPen(themer()->get_color("Text:dark"));
	painter.setFont(themer()->get_font("AudioClip:title"));
	painter.drawText( r, Qt::AlignVCenter, clipInfo);
}

void AudioClipView::update_progress_info( int progress )
{
	PENTER4;
	float prev = m_progress;
	m_progress +=  ((float)progress / m_peakloadingcount);
	
	if ((int)m_progress > (int)prev) {
		update(10, 0, 150, m_height);
	}
}

void AudioClipView::peaks_creation_finished(Peak* peak)
{
	m_peakloadinglist.removeAll(peak);
	if (m_peakloadinglist.size() > 0) {
		start_peak_data_loading();
	} else {
		m_waitingForPeaks = false;
		update();
	}
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
// 	printf("AudioClipView::calculate_bounding_rect()\n");
	set_height(m_tv->get_clipview_height());
	m_boundingRectangle = QRectF(0, 0, (m_clip->get_length() / m_sv->scalefactor), m_height);
	update_start_pos();
}


void AudioClipView::repaint( )
{
	update();
}

void AudioClipView::set_height( int height )
{
	m_height = height;
}

int AudioClipView::get_fade_y_offset() const
{
	return m_infoAreaHeight;
}

void AudioClipView::update_start_pos()
{
// 	printf("AudioClipView::update_start_pos()\n");
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

void AudioClipView::load_theme_data()
{
	m_drawbackground = themer()->get_property("AudioClip:drawbackground", 1).toInt();
	m_infoAreaHeight = themer()->get_property("AudioClip:infoareaheight", 16).toInt();
	m_usePolygonPeakDrawing = themer()->get_property("AudioClip:polygonpeakdrawing", 0).toInt();
	m_mimimumheightforinfoarea = themer()->get_property("AudioClip:mimimumheightforinfoarea", 45).toInt();
	m_classicView = ! themer()->get_property("AudioClip:paintrectified", 0).toInt();
	m_mergedView = themer()->get_property("AudioClip:paintmerged", 0).toInt();
	m_fillwave = themer()->get_property("AudioClip:fillwave", 1).toInt();
	recreate_clipname_pixmap();
}


void AudioClipView::start_peak_data_loading()
{
	Peak* peak = m_peakloadinglist.first();
	
	connect(peak, SIGNAL(progress(int)), this, SLOT(update_progress_info(int)));
	connect(peak, SIGNAL(finished(Peak*)), this, SLOT (peaks_creation_finished(Peak*)));
	
	peak->start_peak_loading();
}

//eof
