/*
    Copyright (C) 2005-2010 Remon Sijrier, Nicola Doebelin

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

#include "VUMeterView.h"

#include <QPainter>
#include <QGradient>
#include <QFont>
#include <QFontMetrics>
#include <QLabel>

#include "Themer.h"
#include "Mixer.h"
#include <AudioDevice.h>
#include <AudioChannel.h>
#include <AudioBus.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
 * \class VUMeterView
 * \brief A widget holding level indicators, 'over' LEDs, and a scale
 *
 * The VUMeterView class is a widget that holds one or more level indicators (VUMeterLevelView),
 * one or more 'over' LEDs (VUMeterViewOverLedView), and one scale (VUMeterRulerView). Usually one
 * VUMeterView per track is created, which contains one level indicator and 'over' LED per
 * audio channel (2 for stereo) and one scale. The name of the track is shown below
 * the level indicators.
 *
 * The scale is hidden automatically if the width of the widget becomes too narrow.
 *
 * All VUMeterView related classes are designed to be arranged in a QGridLayout so as to
 * resize correctly.
 */

static const int VULED_HEIGHT	= 8;

// initialize static variables
QVector<float> VUMeterView::lut;

VUMeterView::VUMeterView(ViewItem* parent, AudioBus* bus)
        : ViewItem(parent)
{
        load_theme_data();

        for (int i = 0; i < bus->get_channel_count(); ++i) {
                VUMeterLevelView* level = new VUMeterLevelView(this, bus->get_channel(i));
                m_levels.append(level);

//                VUMeterOverLedView* led = new VUMeterOverLedView(this);
//                connect(level, SIGNAL(activate_over_led(bool)), led, SLOT(set_active(bool)));
        }

//        add a ruler with tickmarks and labels
//        ruler = new VUMeterRulerView(this);
//        ruler->setPos(0, 10);

        connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
}

VUMeterView::~ VUMeterView( )
{}


void VUMeterView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        PENTER3;

        painter->setPen(QColor(Qt::lightGray));
        painter->drawLine(6, 0, 6, int(m_boundingRect.height()));
}

void VUMeterView::calculate_bounding_rect()
{
        ViewItem::calculate_bounding_rect();
}

void VUMeterView::set_bounding_rect(QRectF rect)
{
        m_boundingRect = rect;
        int vertPos = 0;
        int horizontalPos = 0;
        foreach(VUMeterLevelView* level, m_levels) {
                if (false) {
                        level->set_bounding_rect(QRectF(0, 0, m_boundingRect.width(), (m_boundingRect.height() / m_levels.size()) - 1));
                        level->setPos(0, vertPos);
                        vertPos += level->boundingRect().height() + m_vulevelspacing;
                } else {
                        level->set_bounding_rect(QRectF(0, 0, m_boundingRect.width() / m_levels.size(), m_boundingRect.height()));
                        level->setPos(horizontalPos, 0);
                        horizontalPos += level->boundingRect().width() + m_vulevelspacing;
                }
        }

//        ruler->set_bounding_rect(rect);
}

void VUMeterView::calculate_lut_data()
{
        for (int i = 60; i >= -700; i -= 2) {
                if (i >= -200) {
                        lut.push_back(100.0 + (float)i * 2.5 / 10.0);
                } else if (i >= -300) {
                        lut.push_back( 50.0 + float(i+200) * 2.0 / 10.0);
                } else if (i >= -400) {
                        lut.push_back( 30.0 + float(i+300) * 1.5 / 10.0);
                } else if (i >= -500) {
                        lut.push_back( 15.0 + float(i+400) * 0.75 / 10.0);
                } else {
                        lut.push_back(  7.5 + float(i+500) * 0.5 / 10.0);
                }
        }
}

void VUMeterView::peak_monitoring_stopped()
{
        hide();
}

void VUMeterView::peak_monitoring_started()
{
        show();
}

void VUMeterView::reset()
{
        foreach(VUMeterLevelView* level, m_levels) {
                level->reset();
        }
}

void VUMeterView::load_theme_data()
{
        m_vulevelspacing = themer()->get_property("VUMeterView:layout:vuspacing", 1).toInt();
        m_vulayoutspacing = themer()->get_property("VUMeter:layout:vulayoutspacing", 5).toInt();
        m_mainlayoutmargin = themer()->get_property("VUMeter:layout:mainlayoutmargin", 1).toInt();
        m_mainlayoutspacing = themer()->get_property("VUMeter:layout:mainlayoutspacing", 2).toInt();

        m_chanNameFont = themer()->get_font("VUMeter:fontscale:label");
        m_widgetBgBrush = themer()->get_brush("VUMeter:background:widget");
        m_labelFont = themer()->get_font("VUMeter:fontscale:label");
}

/**********************************************************************/
/*                      VUMeterRulerView                                  */
/**********************************************************************/


/**
 * \class VUMeterRulerView
 * \brief A scale which places tick marks and labels from +6.0 to -60.0 dB
 *
 * The scale is compliant with IEC standard 60268-18 and uses VUMeterView::VUMeterView_lut()
 * to map dB values to widget positions.
 */

static const int TICK_LINE_LENGTH	= 2;
static const float LUT_MULTIPLY		= 5.0;

VUMeterRulerView::VUMeterRulerView(ViewItem* parent)
        : ViewItem(parent)
{
        QFontMetrics fm(themer()->get_font("VUMeter:fontscale:label"));
//        setMinimumWidth(fm.width("-XX")+TICK_LINE_LENGTH + 3);
//        setMaximumWidth(fm.width("-XX")+TICK_LINE_LENGTH + 4);
        m_boundingRect = parent->boundingRect();

        // labels
        presetMark.push_back(6);
        presetMark.push_back(0);
        presetMark.push_back(-6);
        presetMark.push_back(-12);
        presetMark.push_back(-24);
        presetMark.push_back(-70);

        load_theme_data();
        connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
}

void VUMeterRulerView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        PENTER4;

        QString spm;
        int deltaY;

        painter->setFont(m_font);

        // offset is the space occupied by the 'over' LED
        float levelRange = m_boundingRect.width();

        painter->setPen(m_colorActive);

        // draw the labels
        for (uint j = 0; j < presetMark.size(); ++j) {

                int idx = int(LUT_MULTIPLY * float(-presetMark[j] + 6));

                // check the LUT index (I had exceptions without that check)
                if ((idx < 0) || (idx >= VUMeterView::VUMeterView_lut()->size())) {
                        continue;
                }

                deltaY = (int) ( VUMeterView::VUMeterView_lut()->at(idx)/115.0  * levelRange );
                spm.sprintf("%2i", presetMark[j]);

                painter->drawText(deltaY - m_fontLabelAscent + 2, m_fontLabelAscent + 3, spm);
                painter->drawLine(deltaY, - 6, deltaY, TICK_LINE_LENGTH - 6);
        }
}

void VUMeterRulerView::load_theme_data()
{
        m_font = themer()->get_font("VUMeter:fontscale:label");
        QFontMetrics fm(m_font);
        m_fontLabelAscent = fm.ascent();

        int gray = 70;
        m_colorActive = QColor(gray, gray, gray);//themer()->get_color("VUMeterView:font:active");
        m_colorInactive = themer()->get_color("VUMeterView:font:inactive");
}

void VUMeterRulerView::set_bounding_rect(QRectF rect)
{
        m_boundingRect = rect;
}


/**********************************************************************/
/*                      VUMeterLevelView                                    */
/**********************************************************************/


/**
 * \class VUMeterLevelView
 * \brief An audio level indicator widget
 *
 * This is a digital audio level meter which is compliant with the IEC standard
 * 60268-18. It ranges from +6.0 dB to -70.0 dB.
 *
 * A VUMeterLevelView is usually constructed within a VUMeterView object. The audio level is
 * read from an AudioChannel object, which must be given in the constructor. The
 * VUMeterLevelView is optimized for efficient space usage and switches from 3D look
 * to 2D look for narrow sizes.
 */

static const int OVER_SAMPLES_COUNT = 2;	// sensitivity of the 'over' indicator
static const int RMS_SAMPLES = 50;		// number of updates to be stored for RMS calculation
static const int UPDATE_FREQ = 40;		// frame rate of the level meter (update interval in ms)
static const int PEAK_HOLD_TIME = 1000;		// peak hold time (ms)
static const int PEAK_HOLD_MODE = 1;		// 0 = no peak hold, 1 = dynamic, 2 = constant
static const bool SHOW_RMS = false;		// toggle RMS lines on / off


VUMeterLevelView::VUMeterLevelView(ViewItem* parent, AudioChannel* chan)
        : ViewItem(parent)
        , m_channel(chan)
{
        m_boundingRect = QRectF(0, 0, parent->boundingRect().width(), 5);
        tailDeltaY = peakHoldValue = rms = -120.0;
        overCount = rmsIndex = 0;
        peakHoldFalling = false;
        peak = 0.0;
        m_orientation = Qt::Vertical;

        // falloff speed, according to IEC 60268-18: 20 dB in 1.7 sec.
        maxFalloff = 20.0 / (1700.0 / (float)UPDATE_FREQ);

        for (int i = 0; i < RMS_SAMPLES; i++) {
                peakHistory[i] = 0.0;
        }

        connect(&audiodevice(), SIGNAL(stopped()), this, SLOT(stop()));
        connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
        load_theme_data();

        Interface::instance()->register_vumeter_level(this);
}

VUMeterLevelView::~VUMeterLevelView()
{
        Interface::instance()->unregister_vumeter_level(this);
}

void VUMeterLevelView::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        PENTER4;
        if (levelPixmap.width() != int(m_boundingRect.width())
                || levelPixmap.height() != int(m_boundingRect.height())) {
                resize_level_pixmap();
        }

        // convert the peak value to dB and make sure it's in a valid range
        float dBVal = coefficient_to_dB(peak);
        if (dBVal > 6.0) {
                dBVal = 6.0;
        }

        // calculate smooth falloff
        if (tailDeltaY - dBVal > maxFalloff) {
                dBVal = tailDeltaY - maxFalloff;
                tailDeltaY -= maxFalloff;
        } else {
                tailDeltaY = dBVal;
        }

        // check for a new peak hold value
        if (peakHoldFalling && (peakHoldValue >= -120.0)) {
                // smooth falloff, a little faster than the level meter, looks better
                peakHoldValue -= 1.2 * maxFalloff;
        }

        if (peakHoldValue <= dBVal) {
                peakHoldFalling = false;
                peakHoldValue = dBVal;

                // ehm, oops, this timer was moved to Interface
                if (PEAK_HOLD_MODE == 1) {
//                        phTimer.start(PEAK_HOLD_TIME);
                }
        }

        // convert dB values into widget position
        int meterLevel = get_meter_position(dBVal);
        int rmsLevel = get_meter_position(coefficient_to_dB(rms));
        int peakHoldLevel = get_meter_position(peakHoldValue);

        // draw levels
//        painter->drawPixmap(0, 0, clearPixmap, 0, 0, meterLevel, -m_boundingRect.height());

        if (meterLevel > 0) {
                if (m_orientation == Qt::Horizontal) {
                        painter->drawPixmap(0, 0, levelPixmap, 0, 0, meterLevel, m_boundingRect.height());
                } else {
                        painter->drawPixmap(0, meterLevel, levelPixmap, 0, meterLevel, m_boundingRect.width(), m_boundingRect.height() - meterLevel);
                }
        }

        // draw RMS lines
        if (SHOW_RMS) {
                painter->setPen(Qt::blue);
                if (m_orientation == Qt::Horizontal) {
                        painter->drawLine(rmsLevel, 0, rmsLevel, m_boundingRect.height() - 1);
                } else {
                        painter->drawLine(2, rmsLevel, m_boundingRect.width() - 3, rmsLevel);
                }
        }

        // draw Peak hold lines
        if (PEAK_HOLD_MODE) {
                painter->setPen(m_colOverLed);
                if (m_orientation == Qt::Horizontal) {
                        painter->drawLine(peakHoldLevel, 0, peakHoldLevel, m_boundingRect.height() - 1);
                } else {
                        painter->drawLine(0, peakHoldLevel, m_boundingRect.width() - 1, peakHoldLevel);
                }
        }
}

void VUMeterLevelView::resize_level_pixmap( )
{
        PENTER;
        levelPixmap = QPixmap(m_boundingRect.width(), m_boundingRect.height());
        QPainter painter(&levelPixmap);

        if (m_orientation == Qt::Horizontal) {
                gradient2D.setStart(QPointF(m_boundingRect.width(), 0));
        } else {
                gradient2D.setFinalStop(QPointF(0.0, m_boundingRect.height()));
        }

        // WAS: 	painter.fillRect(0, 0, width(), height(), gradient2D);
        painter.fillRect(m_boundingRect, gradient2D);
        painter.end();

        clearPixmap = QPixmap(m_boundingRect.width(), m_boundingRect.height());
        if (m_orientation == Qt::Horizontal) {
                levelClearColor = themer()->get_brush("VUMeter:background:bar", QPoint(0, 0), QPoint(m_boundingRect.width(), m_boundingRect.height()));
        } else {
                levelClearColor  = themer()->get_brush("VUMeter:background:bar", QPoint(0, 0), QPoint(0, m_boundingRect.height()));
        }
        painter.begin(&clearPixmap);
        painter.fillRect(m_boundingRect, levelClearColor);
}

void VUMeterLevelView::update_peak( )
{
        peak = m_channel->get_peak_value();

        // if the meter drops to -inf, reset the 'over LED' and peak hold values
        if ((peak == 0.0) && (tailDeltaY <= -70.0)) {
                peakHoldValue = -120.0;
//                emit activate_over_led(false);
                return;
        }

        // RMS lines
        if (SHOW_RMS) {
                // use some kind of 'ring buffer' to store the last couple of peak values
                if (rmsIndex >= RMS_SAMPLES) {
                        rmsIndex = 0;
                }

                peakHistory[rmsIndex] = peak;

                // calculate the RMS
                float squares = 0.0;

                for (int i = 0; i < RMS_SAMPLES; i++) {
                        squares += peakHistory[i] * peakHistory[i];
                }

                rms = sqrt(squares / float(RMS_SAMPLES));
                rmsIndex++;
        }

        // 'over' detection
        if (peak >= 1.0) overCount++;
        if (peak <  1.0) overCount = 0;

        if (overCount >= OVER_SAMPLES_COUNT) {
                emit activate_over_led(true);
                overCount = 0;
        }

        update(m_boundingRect);
}

void VUMeterLevelView::calculate_bounding_rect()
{
}

void VUMeterLevelView::set_bounding_rect(QRectF rect)
{
        m_boundingRect = rect;
        calculate_bounding_rect();
}


void VUMeterLevelView::reset_peak_hold_value()
{
        peakHoldFalling = true;
}

void VUMeterLevelView::load_theme_data()
{
        float zeroDB = 1.0 - 100.0/115.0;  // 0 dB position
        float msixDB = 1.0 -  80.0/115.0;  // -6 dB position
        float smooth = 0.05;

        gradient2D.setStart(0,0);

        gradient2D.setColorAt(0.0,           themer()->get_color("VUMeter:foreground:6db"));
        gradient2D.setColorAt(zeroDB-smooth, themer()->get_color("VUMeter:foreground:6db"));
        gradient2D.setColorAt(zeroDB+smooth, themer()->get_color("VUMeter:foreground:0db"));
        gradient2D.setColorAt(msixDB-smooth, themer()->get_color("VUMeter:foreground:0db"));
        gradient2D.setColorAt(msixDB+smooth, themer()->get_color("VUMeter:foreground:-6db"));
        gradient2D.setColorAt(1.0,           themer()->get_color("VUMeter:foreground:-60db"));

        levelClearColor  = themer()->get_brush("VUMeter:background:bar", QPoint(0, 0), QPoint(0, m_boundingRect.height()));
        m_colOverLed = themer()->get_color("VUMeter:overled:active");
        m_colBg = themer()->get_brush("VUMeter:background:bar");
}

// accepts dB-values and returns the position in the widget from top
int VUMeterLevelView::get_meter_position(float f)
{
        int idx = int(LUT_MULTIPLY * (-f + 6.0));

        // Comment by Remon: If it happens, then it's a coding error?
        // if so, it could be an idea to hard check for this by using:
        // Q_ASSERT(idx < 0) (iirc) or, using something like a
        // qDebug("....") thing. Just an idea...

        // Reply by Nic: It's not a coding error, it happens for f > +6.0.
        //	The following part clips the value to +6.0, which is the
        //	highest value the LUT contains. I changed the comment accordingly.

        // clipping to the highest value covered by the LUT (+6.0 dB)
        if (idx < 0) {
                idx = 0;
        }

        // if idx > size of the LUT, dBVal is somewhere < -70 dB, which is not displayed
        if (idx >= VUMeterView::VUMeterView_lut()->size()) {
                if (m_orientation == Qt::Horizontal) {
                        return 0;
                } else {
                        return m_boundingRect.height();
                }
        } else {
                if (m_orientation == Qt::Horizontal) {
                        return  int(VUMeterView::VUMeterView_lut()->at(idx)/115.0 * (float)m_boundingRect.width());
                } else {
                        return  m_boundingRect.height() - int(VUMeterView::VUMeterView_lut()->at(idx)/115.0 * m_boundingRect.height());
                }
        }
}


void VUMeterLevelView::reset()
{
        tailDeltaY = -120.0;
        peakHoldValue = -120.0;
        overCount = 0;
        emit activate_over_led(false);
        peak = 0;
        update();
}


