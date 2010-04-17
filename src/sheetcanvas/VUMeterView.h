/*
    Copyright (C) 2005-2010 Remon Sijrier and Nicola Doebelin

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

#ifndef VUMeterView_H
#define VUMeterView_H

#include "ViewItem.h"
#include <QTimer>

class AudioBus;
class AudioChannel;
class VUMeterLevelView;
class QLabel;
class QHBoxLayout;
class QVBoxLayout;

class VUMeterRulerView : public ViewItem
{
        Q_OBJECT

public:
       VUMeterRulerView(ViewItem* parent);

       void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


private:
        std::vector<int>	presetMark;
        std::vector<int>	lineMark;
        QFont			m_font;
        int			m_fontLabelAscent;
        QColor			m_colorActive;
        QColor			m_colorInactive;

private slots:
        void load_theme_data();
};



class VUMeterView : public ViewItem
{
        Q_OBJECT

public:
        VUMeterView(ViewItem* parent, AudioBus* bus);
        ~VUMeterView();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        void reset();

        static QVector<float>* VUMeterView_lut();

protected:
        void resizeEvent( QResizeEvent* e);
        QSize sizeHint () const;
        QSize minimumSizeHint () const;

private:
        bool			isActive;
        int			m_channels;
        int			m_minSpace;
        QString			m_name;
        QLabel*			channelNameLabel;
        VUMeterRulerView*		ruler;
        static QVector<float>	lut;
        QList<VUMeterLevelView*>	m_levels;
        int			m_vulevelspacing;
        int			m_vulayoutspacing;
        int			m_mainlayoutmargin;
        int			m_mainlayoutspacing;
        QFont			m_chanNameFont;
        QFont			m_labelFont;
        QBrush			m_widgetBgBrush;
        QVBoxLayout*		mainlayout;
        QHBoxLayout*		levelLedLayout;

        static void calculate_lut_data();

private slots:
        void peak_monitoring_stopped();
        void peak_monitoring_started();
        void load_theme_data();
};

/**
 * This function returns a pointer to a lookup table mapping dB values
 * to level meter deflection in percent. The transformation is compliant
 * with the IEC 60268-18 standard for digital level meters.
 *
 * The lookup table covers a range from +6 dB to -70 dB with a resolution
 * of 0.2 dB, starting with the highest value. +6.0 dB corresponds to
 * 115% deflection, 0.0 dB to 100%, -70.0 dB close to 0%.
 *
 * @return Pointer to the lookup table, which is of format QVector<float>
 */

inline QVector<float>* VUMeterView::VUMeterView_lut()
{
        if (lut.isEmpty()) {
                calculate_lut_data();
        }
        return &lut;
}



class VUMeterOverLedView : public ViewItem
{
        Q_OBJECT

public:

        VUMeterOverLedView(ViewItem* parent);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
/**
 * Switches the LED indicator on and off. Connect this slot with the signal
 * VUMeterViewLevelView::activate_over_led(bool).
 *
 * @param b new state of the LED indicator.
 */
        void set_active(bool b);


private:
        bool isActive;
        QColor m_colActive;
        QColor m_colInactive;
        QBrush m_colBg;

private slots:
        void load_theme_data();
};


class VUMeterLevelView : public ViewItem
{
        Q_OBJECT

public:
        VUMeterLevelView(ViewItem* parent, AudioChannel* chan);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void reset();

protected:
        void resizeEvent( QResizeEvent * );
        QSize sizeHint () const;
        QSize minimumSizeHint () const;


private:
        bool 		activeTail;
        bool		peakHoldFalling;
        AudioChannel*	m_channel;
        QBrush		levelClearColor,
                        m_colBg;
        QPixmap		levelPixmap;
        QPixmap		clearPixmap;
        QTimer 		timer,
                        phTimer;
        QLinearGradient	gradient2D;
        QColor		m_colOverLed;

        int                     m_orientation;
        float 			presetMark[7];
        float			tailDeltaY;
        float			prevPeakValue;
        float			peak;
        float			rms;
        float			maxFalloff;
        float			peakHoldValue;
        float			peakHistory[50];
        short unsigned int	rmsIndex;
        short unsigned int	overCount;

        void resize_level_pixmap();
        int get_meter_position(float);

private slots:
        void stop();
        void start();
        void update_peak();
        void reset_peak_hold_value();
        void load_theme_data();

signals:

/**
 * Connect this signal to VUMeterViewOverLedView::set_active(bool).
 * 'true' is emitted if a signal >0.0 dB is detected. 'false' is emitted
 * if the connected LED should reset.
 */
        void activate_over_led(bool);

};


#endif

