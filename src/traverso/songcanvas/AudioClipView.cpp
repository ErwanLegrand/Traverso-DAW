/*
Copyright (C) 2005-2007 Remon Sijrier 

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

#include <libtraversocore.h>

#include <QPainter>
#include <QPainterPath>

#include "AudioClipView.h"
#include "SongView.h"
#include "TrackView.h"
#include "FadeView.h"
#include "CurveView.h"
#include "PositionIndicator.h"

#include "Themer.h"
#include <Config.h>
#include <FadeCurve.h>
#include <Curve.h>
#include <Interface.h>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

AudioClipView::AudioClipView(SongView* sv, TrackView* parent, AudioClip* clip )
	: ViewItem(parent, clip)
	, m_tv(parent)
	, m_clip(clip)
	, m_dragging(false)
{
	PENTERCONS;
	
	setZValue(parent->zValue() + 1);
	
	m_sv = sv;
	m_tv->scene()->addItem(this);
	
	load_theme_data();
	create_clipinfo_string();

	m_waitingForPeaks = false;
	m_progress = m_peakloadingcount = 0;
	m_posIndicator = 0;
	m_song = m_clip->get_song();
	
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
	connect(m_clip, SIGNAL(positionChanged(Snappable*)), this, SLOT(position_changed()));
	
	connect(m_song, SIGNAL(modeChanged()), this, SLOT(repaint()));
	
	if (m_clip->recording_state() == AudioClip::RECORDING) {
		start_recording();
		connect(m_clip, SIGNAL(recordingFinished()), this, SLOT(finish_recording()));
	}
	
// 	setFlags(ItemIsSelectable | ItemIsMovable);
	setAcceptsHoverEvents(true);
	setCursor(themer()->get_cursor("AudioClip"));
}

AudioClipView::~ AudioClipView()
{
	PENTERDES;
}

void AudioClipView::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	PENTER2;
	Q_UNUSED(widget);
	

// 	printf("AudioClipView:: %s PAINT :: exposed rect is: x=%f, y=%f, w=%f, h=%f\n", QS_C(m_clip->get_name()), option->exposedRect.x(), option->exposedRect.y(), option->exposedRect.width(), option->exposedRect.height());
	
	int xstart = (int) option->exposedRect.x();
	int pixelcount = (int) option->exposedRect.width();
	if (pixelcount == 0) {
		PWARN("AudioClipView::paint : Exposed rectangle has 0 width ????");
		return;
	}
	
	painter->save();
	
	QRectF clipRect = m_boundingRect;
	clipRect.setWidth(clipRect.width() + 1);
	clipRect.setHeight(clipRect.height());
	painter->setClipRect(clipRect);
	
	if (m_drawbackground) {
		bool mousehover = (option->state & QStyle::State_MouseOver);
	
		if (m_clip->recording_state() == AudioClip::RECORDING) {
			m_backgroundColor = m_backgroundColorMouseHover = themer()->get_color("AudioClip:background:recording");
		} else {
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
	
	} else if (m_clip->recording_state() == AudioClip::NO_RECORDING) {
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
	
	if (m_song->get_mode() == Song::EFFECTS) {
// 		curveView->paint(painter, option, widget);
	}
	
	if (m_dragging) {
		m_posIndicator->set_value(frame_to_smpte( (nframes_t)(x() * m_sv->scalefactor), m_song->get_rate()));
	}
	
	painter->restore();

// 	printf("drawing clip\n");
	
}


void AudioClipView::draw_peaks(QPainter* p, int xstart, int pixelcount)
{
	PENTER2;
	
	// when painting with a path, I _have_ to use path.lineTo()
	// which looks ugly when only parts of the clip is repainted
	// when using a different color for the brush then the outline.
	// Painting 2 more pixels makes it getting clipped away.....
	pixelcount += 2;
	// Seems like we need one pixel more to the left as well, to 
	// make the outline painting painted correctly...
	xstart -= 1;
	// When painting skips one pixel at a time, we always have to start
	// at an even position with an even amount of pixels to paint
	if (xstart % 2) {
		xstart -= 1;
		pixelcount++;
	}
	if (xstart < 0) {
		xstart = 0;
	}
	if (pixelcount % 2) {
		pixelcount += 1;
	}
	
	int channels = m_clip->get_channels();
	bool microView = m_song->get_hzoom() > (Peak::MAX_ZOOM_USING_SOURCEFILE - 1) ? 0 : 1;
	int peakdatacount = microView ? pixelcount : pixelcount * 2;

	int buffersize = microView ? sizeof(short) * peakdatacount : sizeof(unsigned char) * peakdatacount;
	unsigned char buffers[channels][buffersize];
	float pixeldata[channels][buffersize];
	float curvemixdown[buffersize];
	
	
	// Load peak data for all channels, if no peakdata is returned
	// for a certain Peak object, schedule it for loading.
	for (int chan=0; chan < channels; chan++) {
		Peak* peak = m_clip->get_peak_for_channel(chan);
		int availpeaks = peak->calculate_peaks( buffers[chan],
							microView ? m_song->get_hzoom() : m_song->get_hzoom() + 1,
							(xstart * m_sv->scalefactor) + m_clip->get_source_start_frame(),
							microView ? peakdatacount : peakdatacount / 2);
		
		if (peakdatacount != availpeaks) {
// 			PWARN("peakdatacount != availpeaks (%d, %d)", peakdatacount, availpeaks);
		}

		if (availpeaks == Peak::NO_PEAK_FILE) {
			m_peakloadinglist.append(peak);
			m_waitingForPeaks = true;
			m_peakloadingcount++;
		}
		
		if (availpeaks == Peak::PERMANENT_FAILURE || availpeaks == Peak::NO_PEAKDATA_FOUND) {
			return;
		}		
	}
	
	
	if (m_waitingForPeaks) {
		start_peak_data_loading();
		return;
	}
	
	int mixcurvedata = 0;
	mixcurvedata |= curveView->get_vector(xstart, pixelcount, curvemixdown);
	
	float fademixdown[pixelcount];
	for (int i = 0; i < m_fadeViews.size(); ++i) {
		FadeView* view = m_fadeViews.at(i);
		int fademix;
		if (mixcurvedata) {
			fademix = view->get_vector(xstart, pixelcount, fademixdown);
		} else {
			fademix = view->get_vector(xstart, pixelcount, curvemixdown);
		}
			
		if (mixcurvedata && fademix) {
			for (int j=0; j<pixelcount; ++j) {
				curvemixdown[j] *= fademixdown[j];
			}
		}
		mixcurvedata |= fademix;
	}
	
	// Load the Peak data into the pixeldata float buffers
	// ClassicView uses both positive and negative values,
	// rectified view: pick the highest value of both
	// Merged view: calculate highest value for all channels, 
	// and store it in the first channels pixeldata.
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
		
		if (mixcurvedata) {
			int curvemixdownpos;
			for (int chan=0; chan < channels; chan++) {
				curvemixdownpos = 0;
				if (m_classicView) {
					for (int i = 0; i < (pixelcount*2); ++i) {
						pixeldata[chan][i] *= curvemixdown[curvemixdownpos];
						i++;
						pixeldata[chan][i] *= curvemixdown[curvemixdownpos];
						curvemixdownpos += 2;
					}
				} else {
					for (int i = 0; i < (pixelcount*2); i+=2) {
						pixeldata[chan][i] *= curvemixdown[curvemixdownpos];
						curvemixdownpos += 2;
					}
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
		
			QPolygon polygon;
			short* mbuffer = (short*) buffers[chan];
			int bufferPos = 0;
			
			ytrans = (height / 2) + (chan * height);
			p->setMatrix(matrix().translate(1, ytrans).scale(1, scaleFactor), true);
			
			if (m_clip->is_selected()) {
				p->setPen(themer()->get_color("AudioClip:channelseperator:selected"));
			} else {
				p->setPen(themer()->get_color("AudioClip:channelseperator"));
			}
			
			p->drawLine(xstart, 0, xstart + pixelcount, 0);
			
			for (int x = xstart; x < (pixelcount+xstart); x++) {
				polygon.append( QPoint(x, mbuffer[bufferPos++]) );
			}
			
			if (themer()->get_property("AudioClip:wavemicroview:antialiased", 0).toInt()) {
				p->setRenderHints(QPainter::Antialiasing);
			}
			
			p->setPen(themer()->get_color("AudioClip:wavemicroview"));
			p->drawPolyline(polygon);
		
		// Macroview, paint waveform with painterpath
		} else {
			
			if (m_song->get_mode() == Song::EDIT) {
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
			if (m_clip->is_muted()) {
				p->setBrush(themer()->get_color("AudioClip:wavemacroview:brush:muted"));
			}
				
			
			QPainterPath path;
			// in rectified view, we add an additional point, hence + 1
			QPolygonF polygontop(pixelcount + 1);
			int bufferpos = 0;
						
			if (m_classicView) {
				QPolygonF polygonbottom(pixelcount);
				
				int range = pixelcount+xstart;
				for (int x = xstart; x < range; x+=2) {
					polygontop.append( QPointF(x, pixeldata[chan][bufferpos++]) );
					polygonbottom.append( QPointF(x, - pixeldata[chan][bufferpos++]) );
				}
				
				path.addPolygon(polygontop);
				path.lineTo(polygonbottom.last());
				path.addPolygon(polygonbottom);
				
				ytrans = (height / 2) + (chan * height);
			
			} else {
				for (int x = xstart; x < (pixelcount+xstart); x+=2) {
					polygontop.append( QPointF(x, pixeldata[chan][bufferpos]) );
					bufferpos += 2;
				}
				
				polygontop.append(QPointF(xstart + pixelcount, 0));
				path.addPolygon(polygontop);
				path.lineTo(xstart, 0);
				
				ytrans = height + (chan * height);
				scaleFactor =  (float) height * 0.95 * m_clip->get_gain() * m_clip->get_norm_factor() / Peak::MAX_DB_VALUE;
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
	p->fillRect(xstart, 0, pixelcount, m_infoAreaHeight, themer()->get_color("AudioClip:clipinfobackground:inactive"));
	// clip info, only if xstart lies in the stringlenght range which is calculated by a rough estimate.
	if (xstart < m_clipinfoString.size() * 6) {
		p->setFont(themer()->get_font("AudioClip:title"));
		p->drawText(5, 10, m_clipinfoString);
	}
}

void AudioClipView::create_clipinfo_string()
{
	PENTER;
	QString sclipGain = "Gain: "+ coefficient_to_dbstring(m_clip->get_gain() * m_clip->get_norm_factor());
	m_clipinfoString = m_clip->get_name()  + "    " + sclipGain + "   " + QString::number(m_clip->get_rate()) +  " Hz";
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
	create_clipinfo_string();
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
			scene()->removeItem(view);
			delete view;
			break;
		}
	}
}

void AudioClipView::calculate_bounding_rect()
{
	prepareGeometryChange();
// 	printf("AudioClipView::calculate_bounding_rect()\n");
	set_height(m_tv->get_height());
	m_boundingRect = QRectF(0, 0, (m_clip->get_length() / m_sv->scalefactor), m_height);
	update_start_pos();
	ViewItem::calculate_bounding_rect();
}


void AudioClipView::repaint( )
{
	update(m_boundingRect);
}

void AudioClipView::set_height( int height )
{
	m_height = height;
}

int AudioClipView::get_childview_y_offset() const
{
	return (m_height > m_mimimumheightforinfoarea) ? m_infoAreaHeight : 0;
}

void AudioClipView::update_start_pos()
{
// 	printf("AudioClipView::update_start_pos()\n");
	setPos(m_clip->get_track_start_frame() / m_sv->scalefactor, m_tv->get_childview_y_offset());
}

Command * AudioClipView::fade_range()
{
	Q_ASSERT(m_song);
	int x = (int) ( cpointer().scene_pos() - scenePos()).x();

	if (x < (m_boundingRect.width() / 2)) {
		return m_clip->clip_fade_in();
	} else {
		return m_clip->clip_fade_out();
	}

	return 0;
}

Command * AudioClipView::reset_fade()
{
	Q_ASSERT(m_song);
	int x = (int) ( cpointer().scene_pos() - scenePos()).x();

	if (x < (m_boundingRect.width() / 2)) {
		return m_clip->reset_fade_in();
	} else {
		return m_clip->reset_fade_out();
	}

	return 0;
}

void AudioClipView::position_changed()
{
	calculate_bounding_rect();
}

void AudioClipView::load_theme_data()
{
	m_drawbackground = themer()->get_property("AudioClip:drawbackground", 1).toInt();
	m_infoAreaHeight = themer()->get_property("AudioClip:infoareaheight", 16).toInt();
	m_usePolygonPeakDrawing = themer()->get_property("AudioClip:polygonpeakdrawing", 0).toInt();
	m_mimimumheightforinfoarea = themer()->get_property("AudioClip:mimimumheightforinfoarea", 45).toInt();
	m_classicView = ! config().get_property("Themer", "paintaudiorectified", false).toBool();
	m_mergedView = config().get_property("Themer", "paintstereoaudioasmono", false).toBool();
	m_fillwave = themer()->get_property("AudioClip:fillwave", 1).toInt();
	calculate_bounding_rect();
}


void AudioClipView::start_peak_data_loading()
{
	Peak* peak = m_peakloadinglist.first();
	
	connect(peak, SIGNAL(progress(int)), this, SLOT(update_progress_info(int)));
	connect(peak, SIGNAL(finished(Peak*)), this, SLOT (peaks_creation_finished(Peak*)));
	
	peak->start_peak_loading();
}

void AudioClipView::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	Q_UNUSED(event)
	if (ie().is_holding()) {
		return;
	}
	
	update(m_boundingRect);
}


Command * AudioClipView::select_fade_in_shape( )
{
	Interface::instance()->select_fade_in_shape();
	
	return 0;
}

Command * AudioClipView::select_fade_out_shape( )
{
	Interface::instance()->select_fade_out_shape();
	
	return 0;
}

void AudioClipView::start_recording()
{
	m_oldRecordingPos = 0;
	connect(&m_recordingTimer, SIGNAL(timeout()), this, SLOT(update_recording()));
	m_recordingTimer.start(750);
}

void AudioClipView::finish_recording()
{
	m_recordingTimer.stop();
	prepareGeometryChange();
	m_boundingRect = QRectF(0, 0, (m_clip->get_length() / m_sv->scalefactor), m_height);
	curveView->calculate_bounding_rect();
	update();
}

void AudioClipView::update_recording()
{
	if (m_clip->recording_state() != AudioClip::RECORDING) {
		return;
	}
	
	prepareGeometryChange();
	nframes_t newPos = m_clip->get_length();
	m_boundingRect = QRectF(0, 0, (newPos / m_sv->scalefactor), m_height);
	
	QRect updaterect = QRect(m_oldRecordingPos, 0, newPos, (int)m_boundingRect.height());
	update(updaterect);
	m_oldRecordingPos = newPos;
}

void AudioClipView::set_dragging(bool dragging)
{
	if (dragging) {
		if (! m_posIndicator) {
			m_posIndicator = new PositionIndicator(this);
			scene()->addItem(m_posIndicator);
			m_posIndicator->set_position(2, get_childview_y_offset() + 1);
		}
	} else {
		if (m_posIndicator) {
			delete m_posIndicator;
			m_posIndicator = 0;
		}
	}
	
	m_dragging = dragging;
}

//eof

