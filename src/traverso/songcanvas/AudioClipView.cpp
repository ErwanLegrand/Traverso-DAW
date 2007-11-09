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
#include <QFont>

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
#include "PluginChain.h"

#include <QFileDialog>
#include "dialogs/AudioClipEditDialog.h"
#include "Fade.h"
#include "AudioDevice.h"

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

#if QT_VERSION < 0x040300
	m_tv->scene()->addItem(this);
#endif
	
	load_theme_data();
	create_brushes();
	create_clipinfo_string();

	m_waitingForPeaks = false;
	m_progress = 0;
	m_posIndicator = 0;
	m_song = m_clip->get_song();
	
	if (FadeCurve* curve = m_clip->get_fade_in()) {
		add_new_fadeview(curve);
	}
	if (FadeCurve* curve = m_clip->get_fade_out()) {
		add_new_fadeview(curve);
	}
	
	curveView = new CurveView(m_sv, this, m_clip->get_plugin_chain()->get_fader()->get_curve());
	// CurveViews don't 'get' their start offset, it's only a property for AudioClips..
	// So to be sure the CurveNodeViews start offset get updated as well,
	// we call curveviews calculate_bounding_rect() function!
	curveView->set_start_offset(m_clip->get_source_start_location());
	curveView->calculate_bounding_rect();
	connect(curveView, SIGNAL(curveModified()), m_sv, SLOT(stop_follow_play_head()));
	
	connect(m_clip, SIGNAL(muteChanged()), this, SLOT(repaint()));
	connect(m_clip, SIGNAL(stateChanged()), this, SLOT(clip_state_changed()));
	connect(m_clip, SIGNAL(lockChanged()), this, SLOT(repaint()));
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
	
	painter->setClipRect(m_boundingRect);
	
	if (m_clip->is_readsource_invalid()) {
		painter->fillRect(xstart, 0, pixelcount, m_height, themer()->get_color("AudioClip:invalidreadsource"));
		draw_clipinfo_area(painter, xstart, pixelcount);
		painter->setPen(themer()->get_color("AudioClip:contour"));
		painter->drawRect(xstart, 0, pixelcount, m_height - 1);
		painter->setPen(Qt::black);
		painter->setFont( themer()->get_font("AudioClip:fontscale:title") );
		painter->drawText(30, 0, 300, m_height, Qt::AlignVCenter, tr("Click to reset AudioFile !"));
		painter->restore();
		return;
	}
	
	bool mousehover = (option->state & QStyle::State_MouseOver) || m_dragging;
	
	if (m_drawbackground) {
		if (m_clip->recording_state() == AudioClip::RECORDING) {
			painter->fillRect(xstart, 0, pixelcount, m_height, m_brushBgRecording);
		} else {
			if (m_clip->is_muted()) {
				if (mousehover) painter->fillRect(xstart, 0, pixelcount, m_height, m_brushBgMutedHover);
				else            painter->fillRect(xstart, 0, pixelcount, m_height, m_brushBgMuted);
			} else if (m_clip->is_selected()) {
				if (mousehover) painter->fillRect(xstart, 0, pixelcount, m_height, m_brushBgSelectedHover);
				else            painter->fillRect(xstart, 0, pixelcount, m_height, m_brushBgSelected);
			} else {
				if (mousehover) painter->fillRect(xstart, 0, pixelcount, m_height, m_brushBgHover);
				else            painter->fillRect(xstart, 0, pixelcount, m_height, m_brushBg);
			}
		}
	}
	
	if (m_clip->is_muted()) {
		m_waveBrush = m_brushFgMuted;
	} else {
		if (m_song->get_mode() == Song::EDIT) {
			if (mousehover) m_waveBrush = m_brushFgHover;
			else            m_waveBrush = m_brushFg;
		} else {
			if (mousehover) m_waveBrush = m_brushFgEditHover;
			else            m_waveBrush = m_brushFgEdit;
		}
	}
	
	int channels = m_clip->get_channels();

	if (channels > 0) {
		if (m_waitingForPeaks) {
			PMESG("Waiting for peaks!");
			// Hmm, do we paint here something?
			// Progress info, I think so....
			painter->setPen(Qt::black);
			QRect r(10, 0, 150, m_height);
			painter->setFont( themer()->get_font("AudioClip:fontscale:title") );
			QString si;
			si.setNum((int)m_progress);
			if (m_progress == 100) m_progress = 0;
			QString buildProcess = "Building Peaks: " + si + "%";
			painter->drawText(r, Qt::AlignVCenter, buildProcess);
		
		} else if (m_clip->recording_state() == AudioClip::NO_RECORDING) {
			draw_peaks(painter, xstart, pixelcount);
		}
	}
	
	// Draw the contour
	if (m_height < m_mimimumheightforinfoarea) {
		painter->setPen(themer()->get_color("AudioClip:contour"));
		QRectF rect(0.5, 0, m_boundingRect.width() - 1, m_height - 0.5);
		painter->drawRect(rect);
	} else {
		draw_clipinfo_area(painter, xstart, pixelcount);
		painter->setPen(themer()->get_color("AudioClip:contour"));
		QRectF rectinfo(0.5, 0, m_boundingRect.width() - 1, m_infoAreaHeight - 0.5);
		painter->drawRect(rectinfo);
		QRectF rect(0.5, 0, m_boundingRect.width() - 1, m_height - 0.5);
		painter->drawRect(rect);
	}
	
	// Paint a pixmap if the clip is locked
	if (m_clip->is_locked()) {
		int center = (int)(m_clip->get_length() / (2 * m_sv->timeref_scalefactor));
		painter->drawPixmap(center - 8, m_height - 20, find_pixmap(":/lock"));
	}

	if (m_dragging) {
		m_posIndicator->set_value(timeref_to_text(TimeRef(x() * m_sv->timeref_scalefactor), m_sv->timeref_scalefactor));
	}
	
	painter->restore();
}

void AudioClipView::draw_peaks(QPainter* p, int xstart, int pixelcount)
{
	PENTER2;
	// when painting with a path, I _have_ to use path.lineTo()
	// which looks ugly when only parts of the clip is repainted
	// when using a different color for the brush then the outline.
	// Painting 2 more pixels makes it getting clipped away.....
	pixelcount += 2;
	
	bool microView = m_song->get_hzoom() > (Peak::MAX_ZOOM_USING_SOURCEFILE - 1) ? 0 : 1;
	// boundary checking, important for microview only, macroview needs the additional
	// pixels to paint the waveform correctly
	if ( /*microView &&*/ ((xstart + pixelcount) > m_boundingRect.width()) ) {
		pixelcount = (int) m_boundingRect.width() - xstart;
	}
	
	Peak* peak = m_clip->get_peak();
	if (!peak) {
		PERROR("No Peak object available for clip %s", QS_C(m_clip->get_name()));
		return;
	}
	
/*	When painting skips one pixel at a time, we always have to start
	at an even position for 'sample' accurate painting */
	TimeRef clipstartoffset = m_clip->get_source_start_location();
	int adjustforevenpixel = 0;
	if (xstart % 2) {
		xstart -= 1;
		pixelcount += 1;
	}

	if ( (clipstartoffset.to_frame(44100) / Peak::zoomStep[m_song->get_hzoom()]) % 2) {
		clipstartoffset -= m_sv->timeref_scalefactor;
		adjustforevenpixel -= 1;
	}
	
	// Painting seems to start 1 pixel too much to the left
	// this 'fixes it, but I'd rather like a real fix :D
	adjustforevenpixel++;
	
	int channels = m_clip->get_channels();
	int peakdatacount = microView ? pixelcount : pixelcount * 2;

	float* pixeldata[channels];
	
	// Load peak data for all channels, if no peakdata is returned
	// for a certain Peak object, schedule it for loading.
	for (int chan=0; chan < channels; ++chan) {
		
		int availpeaks = peak->calculate_peaks( chan,
				&pixeldata[chan],
    				microView ? m_song->get_hzoom() : m_song->get_hzoom() + 1,
				TimeRef(xstart * m_sv->timeref_scalefactor) + clipstartoffset,
				microView ? peakdatacount : peakdatacount / 2 + 2);
		
		if (peakdatacount != availpeaks) {
// 			PWARN("peakdatacount != availpeaks (%d, %d)", peakdatacount, availpeaks);
		}

		if (availpeaks == Peak::NO_PEAK_FILE) {
			connect(peak, SIGNAL(progress(int)), this, SLOT(update_progress_info(int)));
			connect(peak, SIGNAL(finished()), this, SLOT (peak_creation_finished()));
			m_waitingForPeaks = true;
			peak->start_peak_loading();
			return;
		}
		
		if (availpeaks == Peak::PERMANENT_FAILURE || availpeaks == Peak::NO_PEAKDATA_FOUND) {
			return;
		}		
		
// 		pixelcount = std::min(pixelcount, availpeaks);
	}
	
	
	float curvemixdown[peakdatacount];
	int mixcurvedata = 0;
	float curveDefaultValue = 1.0;
	int offset = (int)(m_clip->get_source_start_location() / m_sv->timeref_scalefactor);
	mixcurvedata |= curveView->has_nodes();
	
	if (mixcurvedata) {
		mixcurvedata |= curveView->get_vector(xstart + offset, pixelcount, curvemixdown);
	} else {
		curveDefaultValue = curveView->get_default_value();
	}
	
	for (int i = 0; i < m_fadeViews.size(); ++i) {
		FadeView* view = m_fadeViews.at(i);
		float fademixdown[pixelcount];
		int fademix = 0;
		
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
		if (!m_classicView) {
			for (int chan=0; chan < channels; chan++) {
				for (int i=0, j=0; i < (pixelcount*2); i+=2, ++j) {
					pixeldata[chan][j] = - f_max(pixeldata[chan][i], - pixeldata[chan][i+1]);
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
				for (int i = 0; i < pixelcount; i++) {
					pixeldata[chan][i] *= curvemixdown[curvemixdownpos];
					curvemixdownpos += 2;
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
		
		if (m_height >= m_mimimumheightforinfoarea) {
			p->setMatrix(matrix().translate(0, m_infoAreaHeight), true);
			height = (m_height - m_infoAreaHeight) / channels;
		} else {
			height = m_height / channels;
		}
	
		
		float scaleFactor = ( (float) height * 0.90 / 2) * m_clip->get_gain() * curveDefaultValue;
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
			p->setMatrix(matrix().translate(0, ytrans), true);
			p->drawLine(xstart, 0, xstart + pixelcount, 0);
			p->restore();
		}
		
		// Microview, paint waveform as polyline
		if (microView) {
		
			m_polygontop.clear();
			m_polygontop.reserve(pixelcount);
			
			int bufferPos = 0;
			
			if (m_mergedView) {
				ytrans = (height / 2) * channels;
			} else {
				ytrans = (height / 2) + (chan * height);
			}
			
			p->setMatrix(matrix().translate(0, ytrans), true);
			
			if (m_clip->is_selected()) {
				p->setPen(themer()->get_color("AudioClip:channelseperator:selected"));
			} else {
				p->setPen(themer()->get_color("AudioClip:channelseperator"));
			}
			
			p->drawLine(xstart, 0, xstart + pixelcount, 0);
			
			for (int x = xstart; x < (pixelcount+xstart); x++) {
				m_polygontop.append( QPointF(x, scaleFactor * pixeldata[chan][bufferPos++]) );
			}
			
			if (themer()->get_property("AudioClip:wavemicroview:antialiased", 0).toInt()) {
				p->setRenderHints(QPainter::Antialiasing);
			}
			
			p->setPen(themer()->get_color("AudioClip:wavemicroview"));
			p->drawPolyline(m_polygontop);
		
		// Macroview, paint waveform with painterpath
		} else {
			if (m_fillwave) {
				p->setBrush(m_waveBrush);
			}

			if (m_song->get_mode() == Song::EDIT) {
				p->setPen(themer()->get_color("AudioClip:wavemacroview:outline"));
			} else  {
				p->setPen(themer()->get_color("AudioClip:wavemacroview:outline:curvemode"));
			}
			if (m_clip->is_muted()) {
				p->setPen(themer()->get_color("AudioClip:wavemacroview:outline:muted"));
			}
				
			scaleFactor = ( (float) height * 0.90 / (Peak::MAX_DB_VALUE * 2)) * m_clip->get_gain() * curveDefaultValue;
			
			if (m_mergedView) {
				if (m_classicView) {
					ytrans = (height / 2) * channels;
					scaleFactor *= channels;
				} else {
					ytrans = height * channels;
					scaleFactor *= channels;
				}
			}
			
			// we add one start/stop point so reserve some more...
			m_polygontop.clear();
			m_polygontop.reserve(pixelcount + 3);
			int bufferpos = 0;

			if (m_classicView) {
				QPainterPath pathtop;
				QPainterPath pathbottom;
				
				m_polygonbottom.clear();
				m_polygonbottom.reserve(pixelcount + 3);
				
				for (int x = 0; x < pixelcount; x+=2) {
					m_polygontop.append( QPointF(x, -scaleFactor * pixeldata[chan][bufferpos++]) );
					m_polygonbottom.append( QPointF(x, scaleFactor * pixeldata[chan][bufferpos++]) );
				}
				
				pathtop.addPolygon(m_polygontop);
				pathbottom.addPolygon(m_polygonbottom);
				pathtop.connectPath(pathbottom.toReversed());
				
				if (m_mergedView) {
					ytrans = (height / 2) * channels;
				} else {
					ytrans = (height / 2) + (chan * height);
				}
			
				p->setMatrix(matrix().translate(xstart + adjustforevenpixel, ytrans), true);
				
				p->drawLine(0, 0, pixelcount, 0);
				p->drawPath(pathtop);
			
			} else {
				QPainterPath path;
				
				scaleFactor =  (float) height * 0.95 * m_clip->get_gain() / Peak::MAX_DB_VALUE * curveDefaultValue;
				ytrans = height + (chan * height);
		
				if (m_mergedView) {
					ytrans = height * channels;
					scaleFactor *= channels;
				}

				for (int x=0; x<pixelcount; x+=2) {
					m_polygontop.append( QPointF(x, scaleFactor * pixeldata[chan][bufferpos]) );
					bufferpos++;
				}
				
				m_polygontop.append(QPointF(pixelcount, 0));
				path.addPolygon(m_polygontop);
				path.lineTo(0, 0);
				
				p->setMatrix(matrix().translate(xstart + adjustforevenpixel, ytrans), true);
				p->drawPath(path);
			}
			
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
		p->setFont(themer()->get_font("AudioClip:fontscale:title"));
		p->drawText(5, 10, m_clipinfoString);
	}
}

void AudioClipView::create_brushes()
{
	// make sure the brushes are made from solid colors (not gradients) if top and bottom color are equal

	// create brushes for background states
	QColor bg_colRecTop = themer()->get_color("AudioClip:background:recording:top");
	QColor bg_colRecBottom = themer()->get_color("AudioClip:background:recording:Bottom");
	if (bg_colRecTop == bg_colRecBottom) {
		m_brushBgRecording = QBrush(bg_colRecTop);
	} else {
		QLinearGradient bg_gradientRec(QPoint(0, 0), QPoint(0, m_height));
		bg_gradientRec.setSpread(QGradient::RepeatSpread);
		bg_gradientRec.setColorAt(0, bg_colRecTop);
		bg_gradientRec.setColorAt(1, bg_colRecBottom);
		m_brushBgRecording = QBrush(bg_gradientRec);
	}

	QColor bg_colMutedTop = themer()->get_color("AudioClip:background:muted:top");
	QColor bg_colMutedBottom = themer()->get_color("AudioClip:background:muted:bottom");
	if (bg_colMutedTop == bg_colMutedBottom) {
		m_brushBgMuted = QBrush(bg_colMutedTop);
	} else {
		QLinearGradient bg_gradientMuted(QPoint(0, 0), QPoint(0, m_height));
		bg_gradientMuted.setSpread(QGradient::RepeatSpread);
		bg_gradientMuted.setColorAt(0, bg_colMutedTop);
		bg_gradientMuted.setColorAt(1, bg_colMutedBottom);
		m_brushBgMuted = QBrush(bg_gradientMuted);
	}

	QColor bg_colMutedHoverTop = themer()->get_color("AudioClip:background:muted:mousehover:top");
	QColor bg_colMutedHoverBottom = themer()->get_color("AudioClip:background:muted:mousehover:bottom");
	if (bg_colMutedHoverTop == bg_colMutedHoverBottom) {
		m_brushBgMutedHover = QBrush(bg_colMutedHoverTop);
	} else {
		QLinearGradient bg_gradientMutedHover(QPoint(0, 0), QPoint(0, m_height));
		bg_gradientMutedHover.setSpread(QGradient::RepeatSpread);
		bg_gradientMutedHover.setColorAt(0, bg_colMutedHoverTop);
		bg_gradientMutedHover.setColorAt(1, bg_colMutedHoverBottom);
		m_brushBgMutedHover = QBrush(bg_gradientMutedHover);
	}

	QColor bg_colSelectedTop = themer()->get_color("AudioClip:background:selected:top");
	QColor bg_colSelectedBottom = themer()->get_color("AudioClip:background:selected:bottom");
	if (bg_colSelectedTop == bg_colSelectedBottom) {
		m_brushBgSelected = QBrush(bg_colSelectedTop);
	} else {
		QLinearGradient bg_gradientSelected(QPoint(0, 0), QPoint(0, m_height));
		bg_gradientSelected.setSpread(QGradient::RepeatSpread);
		bg_gradientSelected.setColorAt(0, bg_colSelectedTop);
		bg_gradientSelected.setColorAt(1, bg_colSelectedBottom);
		m_brushBgSelected = QBrush(bg_gradientSelected);
	}

	QColor bg_colSelectedHoverTop = themer()->get_color("AudioClip:background:selected:mousehover:top");
	QColor bg_colSelectedHoverBottom = themer()->get_color("AudioClip:background:selected:mousehover:bottom");
	if (bg_colSelectedHoverTop == bg_colSelectedHoverBottom) {
		m_brushBgSelectedHover = QBrush(bg_colSelectedHoverTop);
	} else {
		QLinearGradient bg_gradientSelectedHover(QPoint(0, 0), QPoint(0, m_height));
		bg_gradientSelectedHover.setSpread(QGradient::RepeatSpread);
		bg_gradientSelectedHover.setColorAt(0, bg_colSelectedHoverTop);
		bg_gradientSelectedHover.setColorAt(1, bg_colSelectedHoverBottom);
		m_brushBgSelectedHover = QBrush(bg_gradientSelectedHover);
	}

	QColor bg_colTop = themer()->get_color("AudioClip:background:top");
	QColor bg_colBottom = themer()->get_color("AudioClip:background:bottom");
	if (bg_colTop == bg_colBottom) {
		m_brushBg = QBrush(bg_colTop);
	} else {
		QLinearGradient bg_gradient(QPoint(0, 0), QPoint(0, m_height));
		bg_gradient.setSpread(QGradient::RepeatSpread);
		bg_gradient.setColorAt(0, bg_colTop);
		bg_gradient.setColorAt(1, bg_colBottom);
		m_brushBg = QBrush(bg_gradient);
	}

	QColor bg_colHoverTop = themer()->get_color("AudioClip:background:mousehover:top");
	QColor bg_colHoverBottom = themer()->get_color("AudioClip:background:mousehover:bottom");
	if (bg_colHoverTop == bg_colHoverBottom) {
		m_brushBgHover = QBrush(bg_colHoverTop);
	} else {
		QLinearGradient bg_gradientHover(QPoint(0, 0), QPoint(0, m_height));
		bg_gradientHover.setSpread(QGradient::RepeatSpread);
		bg_gradientHover.setColorAt(0, bg_colHoverTop);
		bg_gradientHover.setColorAt(1, bg_colHoverBottom);
		m_brushBgHover = QBrush(bg_gradientHover);
	}

	// Foreground (Waveforms)
	QColor fg_colTop = themer()->get_color("AudioClip:wavemacroview:brush:top");
	QColor fg_colBottom = themer()->get_color("AudioClip:wavemacroview:brush:bottom");
	if (fg_colTop == fg_colBottom) {
		m_brushFg = QBrush(fg_colTop);
	} else {
		QLinearGradient fg_gradient(QPoint(0, 0), QPoint(0, m_height));
		fg_gradient.setSpread(QGradient::RepeatSpread);
		fg_gradient.setColorAt(0, fg_colTop);
		fg_gradient.setColorAt(1, fg_colBottom);
		m_brushFg = QBrush(fg_gradient);
	}

	QColor fg_colHoverTop = themer()->get_color("AudioClip:wavemacroview:brush:hover:top");
	QColor fg_colHoverBottom = themer()->get_color("AudioClip:wavemacroview:brush:hover:bottom");
	if (fg_colHoverTop == fg_colHoverBottom) {
		m_brushFgHover = QBrush(fg_colHoverTop);
	} else {
		QLinearGradient fg_gradientHover(QPoint(0, 0), QPoint(0, m_height));
		fg_gradientHover.setSpread(QGradient::RepeatSpread);
		fg_gradientHover.setColorAt(0, fg_colHoverTop);
		fg_gradientHover.setColorAt(1, fg_colHoverBottom);
		m_brushFgHover = QBrush(fg_gradientHover);
	}

	QColor fg_colEditTop = themer()->get_color("AudioClip:wavemacroview:brush:curvemode:top");
	QColor fg_colEditBottom = themer()->get_color("AudioClip:wavemacroview:brush:curvemode:bottom");
	if (fg_colEditTop == fg_colEditBottom) {
		m_brushFgEdit = QBrush(fg_colEditTop);
	} else {
		QLinearGradient fg_gradientEdit(QPoint(0, 0), QPoint(0, m_height));
		fg_gradientEdit.setSpread(QGradient::RepeatSpread);
		fg_gradientEdit.setColorAt(0, fg_colEditTop);
		fg_gradientEdit.setColorAt(1, fg_colEditBottom);
		m_brushFgEdit = QBrush(fg_gradientEdit);
	}

	QColor fg_colEditHoverTop = themer()->get_color("AudioClip:wavemacroview:brush:curvemode:hover:top");
	QColor fg_colEditHoverBottom = themer()->get_color("AudioClip:wavemacroview:brush:curvemode:hover:bottom");
	if (fg_colEditHoverTop == fg_colEditHoverBottom) {
		m_brushFgEditHover = QBrush(fg_colEditHoverTop);
	} else {
		QLinearGradient fg_gradientEditHover(QPoint(0, 0), QPoint(0, m_height));
		fg_gradientEditHover.setSpread(QGradient::RepeatSpread);
		fg_gradientEditHover.setColorAt(0, fg_colEditHoverTop);
		fg_gradientEditHover.setColorAt(1, fg_colEditHoverBottom);
		m_brushFgEditHover = QBrush(fg_gradientEditHover);
	}

	QColor fg_colMutedTop = themer()->get_color("AudioClip:wavemacroview:brush:muted:top");
	QColor fg_colMutedBottom = themer()->get_color("AudioClip:wavemacroview:brush:muted:bottom");
	if (fg_colMutedTop == fg_colMutedBottom) {
		m_brushFgMuted = QBrush(fg_colMutedTop);
	} else {
		QLinearGradient fg_gradientMuted(QPoint(0, 0), QPoint(0, m_height));
		fg_gradientMuted.setSpread(QGradient::RepeatSpread);
		fg_gradientMuted.setColorAt(0, fg_colMutedTop);
		fg_gradientMuted.setColorAt(1, fg_colMutedBottom);
		m_brushFgMuted = QBrush(fg_gradientMuted);
	}
}

void AudioClipView::create_clipinfo_string()
{
	PENTER;
	QString sclipGain = "Gain: "+ coefficient_to_dbstring(m_clip->get_gain());
	m_clipinfoString = m_clip->get_name()  + "    " + sclipGain + "   " + QString::number(m_clip->get_rate()) +  " Hz";
}

void AudioClipView::update_progress_info( int progress )
{
// 	if (progress > m_progress) {
// 	}
	m_progress = progress;
	update(10, 0, 150, m_height);
}

void AudioClipView::peak_creation_finished()
{
	m_waitingForPeaks = false;
	update();
}

AudioClip * AudioClipView::get_clip( )
{
	return m_clip;
}

void AudioClipView::add_new_fadeview( FadeCurve * fade )
{
	PENTER;
	FadeView* view = new FadeView(m_sv, this, fade);
	m_fadeViews.append(view);
	connect(view, SIGNAL(fadeModified()), m_sv, SLOT(stop_follow_play_head()));
#if QT_VERSION < 0x040300
	scene()->addItem(view);
#endif
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
	m_boundingRect = QRectF(0, 0, (m_clip->get_length() / m_sv->timeref_scalefactor), m_height);
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
	create_brushes();
}

int AudioClipView::get_childview_y_offset() const
{
	return (m_height >= m_mimimumheightforinfoarea) ? m_infoAreaHeight : 0;
}

void AudioClipView::update_start_pos()
{
// 	printf("AudioClipView::update_start_pos()\n");
	setPos(m_clip->get_track_start_location() / m_sv->timeref_scalefactor, m_tv->get_childview_y_offset());
}

Command * AudioClipView::fade_range()
{
	Q_ASSERT(m_song);
	int x = (int) (cpointer().on_first_input_event_scene_x() - scenePos().x());

	if (x < (m_boundingRect.width() / 2)) {
		return clip_fade_in();
	} else {
		return clip_fade_out();
	}

	return 0;
}

Command * AudioClipView::clip_fade_in( )
{
	if (! m_clip->get_fade_in()) {
		// This implicitely creates the fadecurve
		m_clip->set_fade_in(1);
	}
	return new FadeRange(m_clip, m_clip->get_fade_in(), m_sv->timeref_scalefactor);
}

Command * AudioClipView::clip_fade_out( )
{
	if (! m_clip->get_fade_out()) {
		m_clip->set_fade_out(1);
	}
	return new FadeRange(m_clip, m_clip->get_fade_out(), m_sv->timeref_scalefactor);
}

Command * AudioClipView::reset_fade()
{
	Q_ASSERT(m_song);
	int x = (int) (cpointer().on_first_input_event_scene_x() - scenePos().x());

	if (x < (m_boundingRect.width() / 2)) {
		return m_clip->reset_fade_in();
	} else {
		return m_clip->reset_fade_out();
	}

	return 0;
}

void AudioClipView::position_changed()
{
	// Update the curveview start offset, only needed for left edge dragging
	// but who cares :)
	// the calculate_bounding_rect() will update AudioClipViews children, so
	// the CurveView and it's nodes get updated as well, no need to set
	// the start offset for those manually!
	curveView->set_start_offset(m_clip->get_source_start_location());
	calculate_bounding_rect();
	update();
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
	m_oldRecordingPos = TimeRef();
	connect(&m_recordingTimer, SIGNAL(timeout()), this, SLOT(update_recording()));
	m_recordingTimer.start(750);
}

void AudioClipView::finish_recording()
{
	m_recordingTimer.stop();
	prepareGeometryChange();
	m_boundingRect = QRectF(0, 0, (m_clip->get_length() / m_sv->timeref_scalefactor), m_height);
	curveView->calculate_bounding_rect();
	update();
}

void AudioClipView::update_recording()
{
	if (m_clip->recording_state() != AudioClip::RECORDING) {
		return;
	}
	
	TimeRef newPos = m_clip->get_length();
	m_boundingRect = QRectF(0, 0, (newPos / m_sv->timeref_scalefactor), m_height);
	
	int updatewidth = int((newPos - m_oldRecordingPos) / m_sv->timeref_scalefactor);
	QRect updaterect = QRect(int(m_oldRecordingPos / m_sv->timeref_scalefactor) - 1, 0, updatewidth, m_height);
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
			scene()->removeItem(m_posIndicator);
			delete m_posIndicator;
			m_posIndicator = 0;
		}
	}
	
	m_dragging = dragging;
	update();
}

Command * AudioClipView::set_audio_file()
{
	if (m_clip->is_readsource_invalid()) {
		ReadSource* rs = m_clip->get_readsource();
		if ( ! rs ) {
			return ie().failure();
		}
		
		QString filename = QFileDialog::getOpenFileName(0,
				tr("Reset Audio File for Clip: %1").arg(m_clip->get_name()),
				   rs->get_filename(),
						   tr("All files (*);;Audio files (*.wav *.flac)"));
		
		if (filename.isEmpty()) {
			info().information(tr("No file selected!"));
			return ie().failure();
		}
		
		if (rs->set_file(filename) < 0) {
			return ie().failure();
		}
			
		resources_manager()->set_source_for_clip(m_clip, rs);
		
		info().information(tr("Succesfully set AudioClip file to %1").arg(filename));
		
		return ie().succes();
	}
	
	return ie().did_not_implement();
}

void AudioClipView::set_trackview(TrackView * view)
{
	if (m_posIndicator) {
		m_posIndicator->update();
	}
	m_tv = view;
	setParentItem(m_tv);
}

Command * AudioClipView::edit_properties()
{
	AudioClipEditDialog editdialog(m_clip, Interface::instance());
	
	editdialog.exec();
	
	return 0;
}

void AudioClipView::clip_state_changed()
{
	create_clipinfo_string();
	update();
}

