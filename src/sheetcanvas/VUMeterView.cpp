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
        m_minSpace = 0;

        load_theme_data();

        int vertPos = 0;
        for (int i = 0; i < bus->get_channel_count(); ++i) {
                VUMeterOverLedView* led = new VUMeterOverLedView(this);
                VUMeterLevelView* level = new VUMeterLevelView(this, bus->get_channel(i));
                m_levels.append(level);
                connect(level, SIGNAL(activate_over_led(bool)), led, SLOT(set_active(bool)));
                if (i < bus->get_channel_count() - 1)  {
                        m_minSpace += m_vulevelspacing;
                }
        }

        // add a ruler with tickmarks and labels
        ruler = new VUMeterRulerView(this);
        ruler->hide();
        m_minSpace += m_vulayoutspacing;

        // add a tooltip showing the channel name
        m_name = bus->get_name();
        m_channels = bus->get_channel_count();
        setToolTip(m_name);

        // initialize some stuff
        isActive = false;

        channelNameLabel = new QLabel();
        channelNameLabel->setFont(m_chanNameFont);
        channelNameLabel->setAlignment(Qt::AlignHCenter);

        connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
}

VUMeterView::~ VUMeterView( )
{}


void VUMeterView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        PENTER3;

        painter->fillRect(m_boundingRect, QColor(Qt::gray));
}

void VUMeterView::calculate_bounding_rect()
{
        ViewItem::calculate_bounding_rect();
}

void VUMeterView::set_bounding_rect(QRectF rect)
{
        m_boundingRect = rect;
        int vertPos = 0;
        foreach(VUMeterLevelView* level, m_levels) {
                level->set_bounding_rect(QRectF(0, 0, m_boundingRect.width(), (m_boundingRect.height() / m_levels.size()) - 1));
                level->setPos(0, vertPos);
                vertPos += level->boundingRect().height() + m_vulevelspacing;
        }
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

static const int TICK_LINE_LENGTH	= 3;
static const float LUT_MULTIPLY		= 5.0;

VUMeterRulerView::VUMeterRulerView(ViewItem* parent)
        : ViewItem(parent)
{
        QFontMetrics fm(themer()->get_font("VUMeter:fontscale:label"));
//        setMinimumWidth(fm.width("-XX")+TICK_LINE_LENGTH + 3);
//        setMaximumWidth(fm.width("-XX")+TICK_LINE_LENGTH + 4);

        // labels
        presetMark.push_back(6);
        presetMark.push_back(0);
        presetMark.push_back(-5);
        presetMark.push_back(-10);
        presetMark.push_back(-15);
        presetMark.push_back(-20);
        presetMark.push_back(-25);
        presetMark.push_back(-30);
        presetMark.push_back(-35);
        presetMark.push_back(-40);
        presetMark.push_back(-50);
        presetMark.push_back(-60);

        // tick marks
        for (int i = 6; i >= -20; --i) {
                lineMark.push_back(i);
        }
        lineMark.push_back(-25);
        lineMark.push_back(-30);
        lineMark.push_back(-35);
        lineMark.push_back(-40);
        lineMark.push_back(-45);
        lineMark.push_back(-50);
        lineMark.push_back(-60);

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
        float levelRange = float(m_boundingRect.height() - VULED_HEIGHT);

        // draw line marks
        painter->setPen(m_colorInactive);
        for (uint j = 0; j < lineMark.size(); ++j) {
                int idx = int(LUT_MULTIPLY * float((-lineMark[j] + 6)));

                if ((idx < 0) || (idx >= VUMeterView::VUMeterView_lut()->size())) {
                        continue;
                }

                deltaY = (int) ( VUMeterView::VUMeterView_lut()->at(idx)/115.0  * levelRange );
                painter->drawLine(0, m_boundingRect.height() - deltaY, TICK_LINE_LENGTH, m_boundingRect.height() - deltaY);
        }

        painter->setPen(m_colorActive);
        QRect markRect(0, 0, m_boundingRect.width(), m_fontLabelAscent);

        // draw the labels
        for (uint j = 0; j < presetMark.size(); ++j) {

                // skip some labels if the widget is too small
                if ((m_boundingRect.height() < 120) && ((j == 0) || (j == 2) || (j == 4) || (j == 6) ||
                                        (j == 7) || (j == 9))) {
                        continue;
                }

                // skip some labels if the widget is too small
                if ((m_boundingRect.height() < 220) && ((j == 8) || (j == 10))) {
                        continue;
                }

                int idx = int(LUT_MULTIPLY * float(-presetMark[j] + 6));

                // check the LUT index (I had exceptions without that check)
                if ((idx < 0) || (idx >= VUMeterView::VUMeterView_lut()->size())) {
                        continue;
                }

                deltaY = (int) ( VUMeterView::VUMeterView_lut()->at(idx)/115.0  * levelRange );
                spm.sprintf("%2i", presetMark[j]);

                markRect.setY(m_boundingRect.height() - deltaY - m_fontLabelAscent/2 - 1);
                markRect.setHeight(m_fontLabelAscent);
                if (markRect.bottom() >= m_boundingRect.height()) {
                        markRect.translate(0, m_boundingRect.height() - markRect.bottom() - 1);
                }
                painter->drawText(markRect, Qt::AlignRight, spm);
                painter->drawLine(0, m_boundingRect.height() - deltaY, TICK_LINE_LENGTH, m_boundingRect.height() - deltaY);
        }
}

void VUMeterRulerView::load_theme_data()
{
        m_font = themer()->get_font("VUMeterView:fontscale:label");
        QFontMetrics fm(m_font);
//        setMinimumWidth(fm.width("-XX")+TICK_LINE_LENGTH + 3);
//        setMaximumWidth(fm.width("-XX")+TICK_LINE_LENGTH + 4);
        m_fontLabelAscent = fm.ascent();

        m_colorActive = themer()->get_color("VUMeterView:font:active");
        m_colorInactive = themer()->get_color("VUMeterView:font:inactive");
}

/**********************************************************************/
/*                      VUMeterOverLedView                                */
/**********************************************************************/


/**
 * \class VUMeterViewOverLedView
 * \brief An LED-styled widget indicating audio levels > 0.0 dB
 *
 * An LED-styled widget that indicates audio levels > 0.0 dB. It looks like a warning lamp
 * and can be switched on and off. It doesn't analyze audio data itself.
 *
 * A VUMeterViewOverLedView is usually constructed within a VUMeterView object in conjunction with a
 * VUMeterLevelView object. It can be switched on to indicate the occurrence of audio levels
 * > 0.0 dB.
 */

static const int THREE_D_LIMIT		= 8;

VUMeterOverLedView::VUMeterOverLedView(ViewItem* parent)
        : ViewItem(parent)
{
//        setAutoFillBackground(false);
//        setMinimumHeight(VULED_HEIGHT);
        isActive = false;

        load_theme_data();
        connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
}

void VUMeterOverLedView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        PENTER4;

        if (!isActive) {
                painter->fillRect(m_boundingRect, m_colInactive);
                return;
        }

        if (m_boundingRect.width() < THREE_D_LIMIT) {
                painter->fillRect(m_boundingRect, m_colActive);
                return;
        }

        // draw in 3d mode
        painter->fillRect(m_boundingRect, m_colBg);
        painter->fillRect(2, 2, m_boundingRect.width()-4, m_boundingRect.height()-4, m_colActive);

        QColor col;
        col.setRgb(255, 255, 255);
        col.setAlpha(200);
        painter->setPen(col);
        painter->drawLine(1, 1, 1, m_boundingRect.height()-2);
        painter->drawLine(1, 1, m_boundingRect.width()-2, 1);

        col.setRgb(0, 0, 0);
        col.setAlpha(100);
        painter->setPen(col);
        painter->drawLine(m_boundingRect.width()-2, 1, m_boundingRect.width()-2, m_boundingRect.height()-2);
        painter->drawLine(1, m_boundingRect.height()-2, m_boundingRect.width()-2, m_boundingRect.height()-2);

}

void VUMeterOverLedView::set_active(bool b)
{
        if (b == isActive) {
                return;
        }

        isActive = b;
        update();
}

void VUMeterOverLedView::load_theme_data()
{
        m_colActive = themer()->get_color("VUMeterView:overled:active");
        m_colInactive = themer()->get_color("VUMeterView:overled:inactive");
        m_colBg = themer()->get_brush("VUMeterView:background:bar");
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

        // falloff speed, according to IEC 60268-18: 20 dB in 1.7 sec.
        maxFalloff = 20.0 / (1700.0 / (float)UPDATE_FREQ);

        for (int i = 0; i < RMS_SAMPLES; i++) {
                peakHistory[i] = 0.0;
        }

//        setAttribute(Qt::WA_OpaquePaintEvent);
//        setAutoFillBackground(false);

        connect(&audiodevice(), SIGNAL(stopped()), this, SLOT(stop()));
        connect(&timer, SIGNAL(timeout()), this, SLOT(update_peak()));
        connect(&phTimer, SIGNAL(timeout()), this, SLOT(reset_peak_hold_value()));
        connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
        load_theme_data();

        timer.start(UPDATE_FREQ);

        if (PEAK_HOLD_MODE == 1) {
                phTimer.start(PEAK_HOLD_TIME);
        }
}

void VUMeterLevelView::paint(QPainter *directpainter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        PENTER4;
        QPixmap pix(m_boundingRect.width(), m_boundingRect.height());
        QPainter painter(&pix);

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
                peakHoldValue -= 1.2*maxFalloff;
        }

        if (peakHoldValue <= dBVal) {
                peakHoldFalling = false;
                peakHoldValue = dBVal;

                // Comment by Remon: EHH?? The phTimer is still running since it _is_ allready started from the
                // constructor, right? We don't stop the timer anywhere...
                // Reply by Nic: The timer is REstarted here, which is done by QTimer::start(int) as well.
                //	We want the new peak hold value to be held for 1 sec., so we restart the timer
                //	as soon as a new phvalue was detected.
                if (PEAK_HOLD_MODE == 1) {
                        phTimer.start(PEAK_HOLD_TIME);
                }
        }

        // convert dB values into widget position
        int meterLevel = get_meter_position(dBVal);
        int rmsLevel = get_meter_position(coefficient_to_dB(rms));
        int peakHoldLevel = get_meter_position(peakHoldValue);

        // draw levels
        painter.drawPixmap(0, 0, clearPixmap, 0, 0, meterLevel, -m_boundingRect.height());

        if (meterLevel < m_boundingRect.width()) {
                painter.drawPixmap(meterLevel, 0, levelPixmap, meterLevel, 0, m_boundingRect.width() - meterLevel, m_boundingRect.height());
        }

        // draw RMS lines
        if (SHOW_RMS) {
                painter.setPen(Qt::white);
                // Comment by Remon: Somehow the line seems to be one pixel shorter
                // at the right size. Makes the lines look odd (just one pixel)
                // when vu's are very small. Changing - 4 to  - 3 solves it for me
                // any ideas?
                // Reply by Nic: Indeed, obviously drawing stops 1 pixel before the
                //	length specified.
                painter.drawLine(rmsLevel, 1, rmsLevel, m_boundingRect.height() - 2);
        }

        // draw Peak hold lines
        if (PEAK_HOLD_MODE) {
                painter.setPen(m_colOverLed);
                painter.drawLine(peakHoldLevel, 0, peakHoldLevel, m_boundingRect.height());
        }

        directpainter->drawPixmap(0, 0, pix);
}

void VUMeterLevelView::resize_level_pixmap( )
{
        levelPixmap = QPixmap(m_boundingRect.width(), m_boundingRect.height());
        QPainter painter(&levelPixmap);

        gradient2D.setFinalStop(QPointF(m_boundingRect.width(), 0));

        painter.fillRect(m_boundingRect, gradient2D);

        // 3D look if there's enough space
        if (m_boundingRect.width() >= THREE_D_LIMIT) {
                QColor lcol(Qt::white);
                QColor rcol(Qt::black);
                lcol.setAlpha(200);
                rcol.setAlpha(100);

                painter.setPen(lcol);
                painter.drawLine(1, 0, 1, m_boundingRect.height());
                painter.setPen(rcol);
                painter.drawLine(m_boundingRect.width()-2, 0, m_boundingRect.width()-2, m_boundingRect.height());
        }

        painter.end();

        clearPixmap = QPixmap(m_boundingRect.width(), m_boundingRect.height());
        levelClearColor  = themer()->get_brush("VUMeter:background:bar", QPoint(0, 0), QPoint(m_boundingRect.width(), m_boundingRect.height()));
        painter.begin(&clearPixmap);
        painter.fillRect(m_boundingRect, levelClearColor);
}

void VUMeterLevelView::update_peak( )
{
        peak = m_channel->get_peak_value();

        // if the meter drops to -inf, reset the 'over LED' and peak hold values
        if ((peak == 0.0) && (tailDeltaY <= -70.0)) {
                peakHoldValue = -120.0;
                emit activate_over_led(false);
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

        update();
}

void VUMeterLevelView::stop( )
{
        timer.stop();
        emit activate_over_led(false);
}

void VUMeterLevelView::start( )
{
}

void VUMeterLevelView::calculate_bounding_rect()
{
        resize_level_pixmap();
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
//        setMinimumWidth(themer()->get_property("VUMeter:layout:minimumlevelwidth", 6).toInt());

        m_colOverLed = themer()->get_color("VUMeter:overled:active");
        m_colBg = themer()->get_brush("VUMeter:background:bar");

        resize_level_pixmap(); // applies the new theme to the buffer pixmaps
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
                return m_boundingRect.width();
        } else {
                return  m_boundingRect.width() - int(VUMeterView::VUMeterView_lut()->at(idx)/115.0 * (float)m_boundingRect.width());
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


