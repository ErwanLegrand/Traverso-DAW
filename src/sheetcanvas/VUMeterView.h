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
#include "Interface.h"

class Track;
class VUMeterLevelView;
class VUMonitor;
class QLabel;
class QHBoxLayout;
class QVBoxLayout;

class VUMeterRulerView : public ViewItem
{
        Q_OBJECT

public:
       VUMeterRulerView(ViewItem* parent);

       void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
       void set_bounding_rect(QRectF rect);

private:
        std::vector<int>	m_presetMark;
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
        VUMeterView(ViewItem* parent, Track* track);
        ~VUMeterView();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void calculate_bounding_rect();
        void set_bounding_rect(QRectF rect);
        void update_orientation();

        void reset();

        static QVector<float>* VUMeterView_lut();

private:
        VUMeterRulerView*	m_ruler;
        static QVector<float>	lut;
        QList<VUMeterLevelView*> m_levels;
        int			m_vulevelspacing;
        QBrush			m_widgetBgBrush;
        Qt::Orientation         m_orientation;

        static void calculate_lut_data();

private slots:
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


class VUMeterLevelView : public ViewItem, public AbstractVUMeterLevel
{
        Q_OBJECT

public:
        VUMeterLevelView(ViewItem* parent, VUMonitor* monitor);
        ~VUMeterLevelView();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void reset();

        void calculate_bounding_rect();
        void set_bounding_rect(QRectF rect);
        void set_orientation(Qt::Orientation orientation);

        void update_peak();
        void reset_peak_hold_value();

private:
        bool		m_peakHoldFalling;
        VUMonitor*      m_monitor;
        // TODO: the variables below could be shared globally by
        // all the VUMeterLevelView's ?
        QBrush		m_levelClearColor,
                        m_colBg;
        QPixmap		m_levelPixmap;
        QPixmap		m_clearPixmap;
        QLinearGradient	m_gradient2D;
        QColor		m_colOverLed;

        Qt::Orientation         m_orientation;
        float			m_tailDeltaY;
        float			m_peak;
        float			m_rms;
        float			m_maxFalloff;
        float			m_peakHoldValue;
        float			m_peakHistory[50];
        short unsigned int	m_rmsIndex;
        short unsigned int	m_overCount;

        void resize_level_pixmap();
        int get_meter_position(float);

private slots:
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

