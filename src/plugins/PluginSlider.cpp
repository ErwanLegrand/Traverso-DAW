/*
Copyright (C) 2006-2007 Remon Sijrier

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

#include "PluginSlider.h"
#include <Themer.h>
#include "Plugin.h"

PluginSlider::PluginSlider(PluginControlPort* port)
	: QWidget()
	, m_port(port)
{
	setMaximumHeight(18);
	setFixedWidth(200);
	highlight = dragging = false;
	
	m_min = m_port->get_min_control_value();
	m_max = m_port->get_max_control_value();
	m_value = m_port->get_control_value();
	
	if (m_port->get_hint() == PluginPort::INT_CONTROL) {
		m_stepvalue = 1;
	} else {
		m_stepvalue = (m_max - m_min) / 16;
	}
}

void PluginSlider::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	
	QColor color = themer()->get_color("PluginSlider:value");
	QColor background = themer()->get_color("PluginSlider:background");
	
	if (highlight) {
		color = color.light(110);
		background = background.light(105);
	}
	
	// avoid painting at 0, it looks bad...
	if (m_xpos <= 1) m_xpos = 2;
	
	painter.setBrush(background);
	QRectF rect(0.0, 0.0, width() - 0.5, height() - 0.5);
	painter.drawRect(rect);
	painter.fillRect(1, 1, m_xpos - 2, height() - 2, QBrush(color));
	if (m_port->get_hint() == PluginPort::INT_CONTROL) {
		painter.drawText(0, 0, width(), height(), Qt::AlignCenter, QString::number((int)m_value));
	} else {
		painter.drawText(0, 0, width(), height(), Qt::AlignCenter, QString::number(m_value, 'f', 2));
	}
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

void PluginSlider::calculate_new_value(float mouseX)
{	
	if (mouseX < 0) 
		mouseX = 0;
	if (mouseX > width())
		mouseX = width();
		
	m_xpos = (int)mouseX;
	
	
	float relativePos = ((float) mouseX) / width();
	
	float range = m_max - m_min;
	
	m_value = (relativePos * range) + m_min;
	
	// in case of INT_CONTROL, round float to nearest int value.
	if (m_port->get_hint() == PluginPort::INT_CONTROL) {
		m_value = float(int(0.5 + m_value));
	}
	
	emit sliderValueChanged(m_value);
	
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
	float mouseX = ((float)width() / range) * (m_value - m_min);
		
	calculate_new_value(mouseX);
}

void PluginSlider::reset_default_value()
{
	m_value = m_port->get_default_value();
	update_slider_position();
}

