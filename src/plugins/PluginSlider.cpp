/*
Copyright (C) 2006 Remon Sijrier

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

$Id: PluginSlider.cpp,v 1.2 2006/08/09 21:12:45 r_sijrier Exp $
*/

#include "PluginSlider.h"

PluginSlider::PluginSlider()
	: QWidget()
{
	setMaximumHeight(18);
	highlight = dragging = false;
	m_stepvalue = 1;
}

void PluginSlider::set_minimum(float min)
{
	m_min = min;
}


void PluginSlider::set_maximum(float max)
{
	m_max = max;
}

void PluginSlider::set_value(float value)
{
	m_value = value;
	update_slider_position();
}

void PluginSlider::set_step_value( float value )
{
	m_stepvalue = value;
}

void PluginSlider::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	
	QColor color;
	
	if (dragging) {
		color.setRgb(169, 48, 111, 255);
	} else {
		if (highlight) {
			color.setRgb(169, 48, 111, 220);
		} else {
			color.setRgb(169, 48, 111, 180);
		}
	}
	
	painter.fillRect(0, 0, width(), height(), QBrush(QColor("#C7E4D3")));
	painter.fillRect(0, 0, m_xpos, height(), QBrush(color));
}

void PluginSlider::mousePressEvent( QMouseEvent * e )
{
	dragging = true;
	calculate_new_value(e->x());
}

void PluginSlider::mouseMoveEvent( QMouseEvent * e )
{
	calculate_new_value(e->x());
}

void PluginSlider::mouseReleaseEvent( QMouseEvent * e )
{
	dragging = false;
	calculate_new_value(e->x());
}

void PluginSlider::calculate_new_value(int mouseX)
{	
	if (mouseX < 0) 
		mouseX = 0;
	if (mouseX > width())
		mouseX = width();
		
	m_xpos = mouseX;
	
	float relativePos = ((float) mouseX) / width();
	
	float range = m_max - m_min;
	
	m_value = (relativePos * range) + m_min;
	
	emit sliderValueChanged(m_value);
	emit sliderValueChangedDouble((double)m_value);
	
	update();
}

void PluginSlider::leaveEvent( QEvent * )
{
	highlight = false;
	update();
}

void PluginSlider::enterEvent( QEvent * )
{
	highlight = true;
	update();
}

void PluginSlider::wheelEvent( QWheelEvent* e )
{
	if (e->orientation() == Qt::Vertical) {
		if (e->delta() > 0) {
			m_value += m_stepvalue;
			if (m_value > m_max) {
				m_value = m_max;
			}
		}
		if (e->delta() < 0) {
			m_value -= m_stepvalue;
			if (m_value < m_min) {
				m_value = m_min;
			}
		}
		
		update_slider_position();
	}
}

void PluginSlider::update_slider_position( )
{
	float range = m_max - m_min;
	int mouseX = (int) (( (float)width() / range) * (m_value - m_min));
		
	calculate_new_value(mouseX);
}

//eof
