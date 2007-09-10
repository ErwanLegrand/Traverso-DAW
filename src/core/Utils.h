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

#ifndef UTILS_H
#define UTILS_H

#include "defines.h"
#include <QPixmap>
#include <QDateTime>

#define QS_C(x) x.toAscii().data()

class QString;

QString frame_to_text(nframes_t nframes, int rate, int scalefactor);
QString frame_to_smpte(nframes_t nframes, int rate);
QString frame_to_ms_3(nframes_t nframes, int rate);
QString frame_to_ms_2(nframes_t nframes, int rate);
QString frame_to_cd(nframes_t nframes, int rate);
QString frame_to_hms(double nframes, int rate);
QString frame_to_ms(double nframes, int rate);

QString timeref_to_ms(const TimeRef& ref);
QString timeref_to_ms_2 (const TimeRef& ref);
QString timeref_to_ms_3 (const TimeRef& ref);
QString timeref_to_text(const TimeRef& ref, int scalefactor);

nframes_t smpte_to_frame(QString str, int rate);
TimeRef msms_to_timeref(QString str);
nframes_t cd_to_frame(QString str, int rate);
QString coefficient_to_dbstring(float coeff);
QDateTime extract_date_time(qint64 id);

qint64 create_id();

static inline unsigned int is_power_of_two (unsigned int n)
{
	return !(n & (n - 1));
}

QPixmap find_pixmap(const QString& pixname);

#endif
