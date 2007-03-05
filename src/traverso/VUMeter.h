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
 
    $Id: VUMeter.h,v 1.7 2007/03/05 20:51:24 r_sijrier Exp $
*/

#ifndef VUMETER_H
#define VUMETER_H

#include <QWidget>
#include <QString>
#include <QGridLayout>
#include <QLabel>
#include <QVector>
#include <QTimer>

class AudioBus;
class AudioChannel;

class VUMeterRuler : public QWidget
{
	Q_OBJECT
	
public:

        VUMeterRuler(QWidget* parent);

/**
 * Sets the offset from the top to the first tick mark (+6.0 dB) in pixels.
 * Set this value to the height of the 'over' LED (VUMeterOverLed) in order
 * to position the first tick mark at the top of the VUMeterLevel.
 *
 * @param i offset from top in pixels
 */
	void setYOffset(int);

protected:
        void paintEvent( QPaintEvent* e);


private:
	std::vector<int>	presetMark;
	std::vector<int>	lineMark;
	int			yOffset;

};



class VUMeter : public QWidget
{

public:
        VUMeter(QWidget* parent, AudioBus* bus);
        ~VUMeter();

	static QVector<float>* vumeter_lut();

protected:
        void resizeEvent( QResizeEvent* e);
        void paintEvent( QPaintEvent* e);
	QSize sizeHint () const;
	QSize minimumSizeHint () const;

private:
        bool			isActive;
        int 			m_id;
        int			m_channels;
	int			minSpace;
        QString			m_name;
	QLabel*			channelNameLabel;
	QGridLayout*		glayout;
	QBrush			bgBrush;
	QLinearGradient		bgGradient;
	VUMeterRuler*		ruler;
	bool			drawTickmarks;
	static QVector<float>	lut;

	static void calculate_lut_data();
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

inline QVector<float>* VUMeter::vumeter_lut()
{
	if (lut.isEmpty()) {
		calculate_lut_data();
	}
	return &lut;
}



class VUMeterOverLed : public QWidget
{
	Q_OBJECT
	
public:

        VUMeterOverLed(QWidget* parent);

public slots:
/**
 * Switches the LED indicator on and off. Connect this slot with the signal
 * VUMeterLevel::activate_over_led(bool).
 *
 * @param b new state of the LED indicator. 
 */
	void set_active(bool b);

protected:
        void paintEvent( QPaintEvent* e);


private:
        bool isActive;
};


class VUMeterLevel : public QWidget
{
	Q_OBJECT
	
public:

        VUMeterLevel(QWidget* parent, AudioChannel* chan);

protected:
        void paintEvent( QPaintEvent* e);
        void resizeEvent( QResizeEvent * );


private:
        bool 		activeTail;
	bool		peakHoldFalling;
        AudioChannel*	m_channel;
        QColor		levelClearColor;
        QPixmap		levelPixmap;
        QPixmap		clearPixmap;
        QTimer 		timer,
			phTimer;
	QLinearGradient	gradient2D,
			gradient3DLeft,
			gradient3DRight;

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
	void create_gradients();
	int get_meter_position(float);

private slots:
	void stop();
	void start();
        void update_peak();
	void reset_peak_hold_value();

signals:

/**
 * Connect this signal to VUMeterOverLed::set_active(bool).
 * 'true' is emitted if a signal >0.0 dB is detected. 'false' is emitted
 * if the connected LED should reset.
 */
	void activate_over_led(bool);

};


#endif

