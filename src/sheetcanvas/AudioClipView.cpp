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

#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QGraphicsSimpleTextItem>

#include "AudioClipView.h"
#include "SheetView.h"
#include "TrackView.h"
#include "FadeView.h"
#include "CurveView.h"

#include "AudioClip.h"
#include "ReadSource.h"
#include "InputEngine.h"
#include "ContextPointer.h"
#include "Sheet.h"
#include "ResourcesManager.h"
#include "ProjectManager.h"
#include "Peak.h"
#include "Information.h"
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


AudioClipView::AudioClipView(SheetView* sv, TrackView* parent, AudioClip* clip )
	: ViewItem(parent, clip)
	, m_tv(parent)
	, m_clip(clip)
{
	PENTERCONS;
	
	setZValue(parent->zValue() + 1);
	
	m_sv = sv;
	m_sheet = m_clip->get_sheet();
	
	load_theme_data();
	create_brushes();
	create_clipinfo_string();

	m_waitingForPeaks = false;
	m_progress = 0;
	
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
	connect(m_clip, SIGNAL(positionChanged()), this, SLOT(position_changed()));
	
	connect(m_sheet, SIGNAL(modeChanged()), this, SLOT(repaint()));
	
	if (m_clip->recording_state() == AudioClip::RECORDING) {
		start_recording();
		connect(m_clip, SIGNAL(recordingFinished(AudioClip*)), this, SLOT(finish_recording()));
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
	
	int xstart = qRound(option->exposedRect.x());
	int pixelcount = qRound(option->exposedRect.width());
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
	
	bool mousehover = (option->state & QStyle::State_MouseOver) || m_clip->is_moving();
	
	if (m_drawbackground) {
		if (m_clip->recording_state() == AudioClip::RECORDING) {
			painter->fillRect(xstart, 0, pixelcount+1, m_height, m_brushBgRecording);
		} else {
			if (m_clip->is_muted()) {
				if (mousehover) painter->fillRect(xstart, 0, pixelcount+1, m_height, m_brushBgMutedHover);
				else            painter->fillRect(xstart, 0, pixelcount+1, m_height, m_brushBgMuted);
			} else if (m_clip->is_selected()) {
				if (mousehover) painter->fillRect(xstart, 0, pixelcount+1, m_height, m_brushBgSelectedHover);
				else            painter->fillRect(xstart, 0, pixelcount+1, m_height, m_brushBgSelected);
			} else {
				if (mousehover) painter->fillRect(xstart, 0, pixelcount+1, m_height, m_brushBgHover);
				else            painter->fillRect(xstart, 0, pixelcount+1, m_height, m_brushBg);
			}
		}
	}
	
	if (m_clip->is_muted()) {
		m_waveBrush = m_brushFgMuted;
	} else {
		if (m_sheet->get_mode() == Sheet::EDIT) {
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
// 			PROFILE_START;
			draw_peaks(painter, option->exposedRect.x(), pixelcount);
// 			PROFILE_END("draw peaks");
		}
	}

	if (m_height > m_mimimumheightforinfoarea) {
		draw_clipinfo_area(painter, xstart, pixelcount);
	}

	// Draw the db lines at 0 and -6 db
	if (m_drawDbGrid) {
		draw_db_lines(painter, xstart, pixelcount);
	}

	// Draw the contour
	painter->setPen(themer()->get_color("AudioClip:contour"));
	int adjust = 0;
	float round = (m_boundingRect.width() - int(m_boundingRect.width()));
	if (round < 0.5) {
		adjust = 1;
	}
	QRect rect(0, 0, m_boundingRect.width() - adjust, m_height - 1);
	painter->drawRect(rect);
	
	// Paint a pixmap if the clip is locked
	if (m_clip->is_locked()) {
		int center = (int)(m_clip->get_length() / (2 * m_sv->timeref_scalefactor));
		painter->drawPixmap(center - 8, m_height - 20, find_pixmap(":/lock"));
	}

	painter->restore();
}

void AudioClipView::draw_peaks(QPainter* p, qreal xstart, int pixelcount)
{
	PENTER2;
	
	Peak* peak = m_clip->get_peak();
	
	if (!peak) {
		PERROR("No Peak object available for clip %s", QS_C(m_clip->get_name()));
		return;
	}
	
	bool microView = m_sheet->get_hzoom() < 64 ? 1 : 0;
	TimeRef clipstartoffset = m_clip->get_source_start_location();
	int channels = m_clip->get_channels();
	int peakdatacount = microView ? pixelcount : pixelcount * 2;
	float* pixeldata[channels];
	float curveDefaultValue = 1.0;
	int mixcurvedata = 0;
	mixcurvedata |= curveView->has_nodes();
	int offset = (int)(m_clip->get_source_start_location() / m_sv->timeref_scalefactor);
		
	if (!mixcurvedata) {
		curveDefaultValue = curveView->get_default_value();
	}
	
	float curvemixdown[peakdatacount];
	if (mixcurvedata) {
		mixcurvedata |= curveView->get_vector(qRound(xstart) + offset, peakdatacount, curvemixdown);
	}
	
	for (int i = 0; i < m_fadeViews.size(); ++i) {
		FadeView* view = m_fadeViews.at(i);
		float fademixdown[peakdatacount];
		int fademix = 0;
		
		if (mixcurvedata) {
			fademix = view->get_vector(qRound(xstart), peakdatacount, fademixdown);
		} else {
			fademix = view->get_vector(qRound(xstart), peakdatacount, curvemixdown);
		}
		
		if (mixcurvedata && fademix) {
			for (int j=0; j<peakdatacount; ++j) {
				curvemixdown[j] *= fademixdown[j];
			}
		}
		
		mixcurvedata |= fademix;
	}
	
	// Load peak data, mix curvedata and start painting it
	// if no peakdata is returned for a certain Peak object, schedule it for loading.
	for (int chan=0; chan < channels; ++chan) {
		
		int availpeaks = peak->calculate_peaks(
				chan,
				&pixeldata[chan],
				TimeRef(xstart * m_sv->timeref_scalefactor) + clipstartoffset,
				peakdatacount,
				m_sheet->get_hzoom());
				
		
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
		
		if (m_mergedView && channels == 2 && chan == 0) continue;

		
// 		pixelcount = std::min(pixelcount, availpeaks);
	
		// ClassicView uses both positive and negative values,
		// rectified view: pick the highest value of both
		// Merged view: calculate highest value for all channels, 
		// and store it in the first channels pixeldata.
		if (!microView) {
			// if Rectified View, calculate max of the minimum and maximum value.
			if (!m_classicView) {
				for (int i=0, j=0; i < (pixelcount*2); i+=2, ++j) {
					pixeldata[chan][j] = - fabs(f_max(pixeldata[chan][i], - pixeldata[chan][i+1]));
				}
			}
		
			if (m_mergedView && channels == 2) {
				for (int i = 0; i < (pixelcount*2); ++i) {
					pixeldata[0][i] = f_max(pixeldata[chan - 1][i], pixeldata[chan][i]);
				}
			}
		}
	
		if (mixcurvedata) {
			int curvemixdownpos = 0;
			if (m_classicView) {
				for (int i = 0; i < (pixelcount*2); ++i) {
					pixeldata[chan][i++] *= curvemixdown[curvemixdownpos];
					pixeldata[chan][i] *= curvemixdown[curvemixdownpos];
					curvemixdownpos++;
				}
			} else {
				for (int i = 0; i < pixelcount; i++) {
					pixeldata[chan][i] *= curvemixdown[curvemixdownpos];
					curvemixdownpos++;
				}
			}
		}
		
		p->save();
		
		// calculate the height of the area available for peak drawing 
		// and if the infoarea is displayed, translate the painter
		// drawing by dy = m_infoAreaHeight
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
			p->setMatrix(matrix().translate(xstart, ytrans), true);
			p->drawLine(0, 0, pixelcount, 0);
			p->restore();
		}
		
		
		// Microview, paint waveform as polyline
		if (microView) {
		
			m_polygon.clear();
			m_polygon.reserve(pixelcount);
			
			int bufferPos = 0;

			if (m_mergedView) {
				ytrans = (height / 2) * channels;
				scaleFactor *= channels;
			} else {
				ytrans = (height / 2) + (chan * height);
			}
			
			p->setMatrix(matrix().translate(xstart, ytrans), true);
			
			if (m_clip->is_selected()) {
				p->setPen(themer()->get_color("AudioClip:channelseperator:selected"));
			} else {
				p->setPen(themer()->get_color("AudioClip:channelseperator"));
			}
			
			p->drawLine(0, 0, pixelcount, 0);
			
			for (int x = 0; x < pixelcount; x++) {
				m_polygon.append( QPointF(x, -scaleFactor * pixeldata[chan][bufferPos++]) );
			}
			
			if (themer()->get_property("AudioClip:wavemicroview:antialiased", 0).toInt()) {
				p->setRenderHints(QPainter::Antialiasing);
			}
			
			p->setPen(themer()->get_color("AudioClip:wavemicroview"));
			p->drawPolyline(m_polygon);

		// Macroview, paint waveform with painterpath
		} else {
			if (m_fillwave) {
				p->setBrush(m_waveBrush);
			}

			if (m_paintWithOutline) {
				if (m_sheet->get_mode() == Sheet::EDIT) {
					p->setPen(themer()->get_color("AudioClip:wavemacroview:outline"));
				} else  {
					p->setPen(themer()->get_color("AudioClip:wavemacroview:outline:curvemode"));
				}
				if (m_clip->is_muted()) {
					p->setPen(themer()->get_color("AudioClip:wavemacroview:outline:muted"));
				}
			} else {
				p->setPen(Qt::NoPen);
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
			
			int bufferpos = 0;

			if (m_classicView) {
				
				if (m_mergedView) {
					ytrans = (height / 2) * channels;
				} else {
					ytrans = (height / 2) + (chan * height);
				}
			
				p->setMatrix(matrix().translate(xstart, ytrans), true);
				
				m_polygon.clear();
				m_polygon.reserve(pixelcount*2);
				
				for (int x = 0; x < pixelcount; x++) {
					m_polygon.append( QPointF(x, -scaleFactor * pixeldata[chan][bufferpos]) );
					bufferpos+=2;
				}
				
				bufferpos -= 1;
				
				for (int x = pixelcount - 1; x >= 0; x--) {
					m_polygon.append( QPointF(x, scaleFactor * pixeldata[chan][bufferpos]) );
					bufferpos-=2;
				}
				
				QPainterPath path;
				path.addPolygon(m_polygon);
				
// 				PROFILE_START;
				p->drawPath(path);
// 				PROFILE_END("draw path");

				// Draw 'the' -INF line
				p->setPen(minINFLineColor);
				p->drawLine(0, 0, pixelcount, 0);

			} else {
				scaleFactor =  (float) height * 0.95 * m_clip->get_gain() / Peak::MAX_DB_VALUE * curveDefaultValue;
				ytrans = height + (chan * height);
		
				if (m_mergedView) {
					ytrans = height * channels;
					scaleFactor *= channels;
				}

				p->setMatrix(matrix().translate(xstart, ytrans), true);
				
				m_polygon.clear();
				m_polygon.reserve(pixelcount + 2);
				
				for (int x=0; x<pixelcount; x++) {
					m_polygon.append( QPointF(x, scaleFactor * pixeldata[chan][bufferpos++]) );
				}
				
				m_polygon.append(QPointF(pixelcount, 0));
				
				QPainterPath path;
				path.addPolygon(m_polygon);
				path.lineTo(0, 0);
				
				p->drawPath(path);

			}
		}
		
		p->restore();
	}
}

void AudioClipView::draw_clipinfo_area(QPainter* p, int xstart, int pixelcount)
{
	// fill info area bg
	p->fillRect(xstart, 1, pixelcount, m_infoAreaHeight, themer()->get_color("AudioClip:clipinfobackground:inactive"));
	if (m_height >= m_mimimumheightforinfoarea) {
		p->drawPixmap(7, 1, m_clipInfo);
	}
}


void AudioClipView::draw_db_lines(QPainter* p, qreal xstart, int pixelcount)
{
	p->save();

	int height;
	int channels = m_clip->get_channels();
	bool microView = m_sheet->get_hzoom() < 64 ? 1 : 0;
	int linestartpos = xstart;
	if (xstart < m_lineOffset) linestartpos = m_lineOffset;

	if (m_mergedView) {
		channels = 1;
	}

	// calculate the height of one channel
	if (m_height >= m_mimimumheightforinfoarea) {
		p->setMatrix(matrix().translate(0, m_infoAreaHeight), true);
		height = (m_height - m_infoAreaHeight) / channels;
	} else {
		height = m_height / channels;
	}

	p->setPen(themer()->get_color("AudioClip:db-grid"));
	p->setFont( themer()->get_font("AudioClip:fontscale:dblines") );

	if (m_classicView || microView) { // classicView = non-rectified

		// translate the painter to set the first channel center line to 0
		p->setMatrix(matrix().translate(0, height / 2), true);

		// determine the distance of the db line from the center line
		int zeroDb = 0.9 * height / 2;
		int msixDb = 0.9 * height / 4;

		// draw the lines above and below the center line, then translate
		// the painter to the next channel
		for (int i = 0; i < channels; ++i) {
			p->drawLine(linestartpos, zeroDb, xstart+pixelcount, zeroDb);
			p->drawLine(linestartpos, -zeroDb, xstart+pixelcount, -zeroDb);
			p->drawLine(linestartpos, msixDb, xstart+pixelcount, msixDb);
			p->drawLine(linestartpos, -msixDb + 1, xstart+pixelcount, -msixDb + 1);

			if (xstart < m_lineOffset) {
				p->drawText(0.0, zeroDb - 1 + m_lineVOffset, "  0 dB");
				p->drawText(0.0, -zeroDb + m_lineVOffset, "  0 dB");
				p->drawText(0.0, msixDb + m_lineVOffset, " -6 dB");
				p->drawText(0.0, -msixDb + m_lineVOffset, " -6 dB");
			}


			p->setMatrix(matrix().translate(0, height), true);
		}
	} else {  // rectified

		// translate the painter to set the first channel base line to 0
		p->setMatrix(matrix().translate(0, height), true);		

		// determine the distance of the db line from the center line
		int zeroDb = 0.95 * height;
		int msixDb = 0.95 * height / 2;

		// draw the lines above the center line, then translate
		// the painter to the next channel
		for (int i = 0; i < channels; ++i) {
			p->drawLine(linestartpos, -zeroDb, xstart+pixelcount, -zeroDb);
			p->drawLine(linestartpos, -msixDb + 1, xstart+pixelcount, -msixDb + 1);

			if (xstart < m_lineOffset) {
				p->drawText(0.0, -zeroDb + m_lineVOffset, "  0 dB");
				p->drawText(0.0, -msixDb + m_lineVOffset, " -6 dB");
			}

			p->setMatrix(matrix().translate(0, height), true);
		}
	}

	p->restore();
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
	
	QFont font = themer()->get_font("AudioClip:fontscale:title");
	QFontMetrics fm(font);
	
	QString clipinfoString = fm.elidedText(m_clip->get_name(), Qt::ElideMiddle, 150) 
			   + "    " + sclipGain + "   " + QString::number(m_clip->get_rate()) +  " Hz";
	
	int clipInfoWidth = fm.boundingRect(clipinfoString).width();
	
	m_clipInfo = QPixmap(clipInfoWidth, m_infoAreaHeight);
	m_clipInfo.fill(Qt::transparent);
	
	QPainter painter(&m_clipInfo);
	painter.setFont(font);
	painter.drawText(m_clipInfo.rect(), clipinfoString);
}

void AudioClipView::update_progress_info( int progress )
{
	m_progress = progress;
	update(10, 0, 150, m_height);
}

void AudioClipView::peak_creation_finished()
{
	m_waitingForPeaks = false;
	update();
}

void AudioClipView::add_new_fadeview( FadeCurve * fade )
{
	PENTER;
	FadeView* view = new FadeView(m_sv, this, fade);
	m_fadeViews.append(view);
	connect(view, SIGNAL(fadeModified()), m_sv, SLOT(stop_follow_play_head()));
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
	PENTER4;
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
	setPos(qRound(m_clip->get_track_start_location() / m_sv->timeref_scalefactor), m_tv->get_childview_y_offset());
}

Command * AudioClipView::fade_range()
{
	Q_ASSERT(m_sheet);
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
	Q_ASSERT(m_sheet);
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
	m_mimimumheightforinfoarea = themer()->get_property("AudioClip:mimimumheightforinfoarea", 45).toInt();
	m_classicView = ! config().get_property("Themer", "paintaudiorectified", false).toBool();
	m_mergedView = config().get_property("Themer", "paintstereoaudioasmono", false).toBool();
	m_fillwave = themer()->get_property("AudioClip:fillwave", 1).toInt();
	minINFLineColor = themer()->get_color("AudioClip:channelseperator");
	m_paintWithOutline = config().get_property("Themer", "paintwavewithoutline", true).toBool();
	m_drawDbGrid = config().get_property("Themer", "drawdbgrid", false).toBool();
	calculate_bounding_rect();

	QFont dblfont = themer()->get_font("AudioClip:fontscale:dblines");
	QFontMetrics fm(dblfont);
	m_lineOffset = fm.width(" -6 dB ");
	m_lineVOffset = fm.ascent()/2;
}


void AudioClipView::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	Q_UNUSED(event)
	if (ie().is_holding()) {
		return;
	}
	update(m_boundingRect);
	m_tv->to_front(this);
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
	QRect updaterect = QRect(int(m_oldRecordingPos / m_sv->timeref_scalefactor) - 1, 0, updatewidth + 1, m_height);
	update(updaterect);
	m_oldRecordingPos = newPos;
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
		
		
		// FIXME This is a hack. When a ReadSource didn't have a valid file it wasn't added
		// to DiskIO in AudioClip::set_sheet(). So when resetting the audiofile this solves it,
		// but it's not the proper place to do so!!
		m_clip->set_sheet(m_sheet);
		
		info().information(tr("Succesfully set AudioClip file to %1").arg(filename));
		
		return ie().succes();
	}
	
	return ie().did_not_implement();
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

