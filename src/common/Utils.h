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

#define QS_C(x) x.toUtf8().data()

class QString;

QString timeref_to_hms(const TimeRef& ref);
QString timeref_to_ms(const TimeRef& ref);
QString timeref_to_ms_2 (const TimeRef& ref);
QString timeref_to_ms_3 (const TimeRef& ref);
QString timeref_to_text(const TimeRef& ref, int scalefactor);
QString timeref_to_cd(const TimeRef& ref);
QString timeref_to_cd_including_hours(const TimeRef& ref);

TimeRef msms_to_timeref(QString str);
TimeRef cd_to_timeref(QString str);
TimeRef cd_to_timeref_including_hours(QString str);
QString coefficient_to_dbstring(float coeff, int decimals=1);
QDateTime extract_date_time(qint64 id);

qint64 create_id();

QStringList find_qm_files();
QString language_name_from_qm_file(const QString& lang);

bool t_MetaobjectInheritsClass(const QMetaObject* mo, const QString & className);

static inline unsigned int is_power_of_two (unsigned int n)
{
	return !(n & (n - 1));
}

static inline int cnt_bits(unsigned long val, int & highbit)
{
	int cnt = 0;
	highbit = 0;
	while (val) {
		if (val & 1) cnt++;
		val>>=1;
		highbit++;
	}
	return cnt;
}

// returns the next power of two greater or equal to val
static inline long nearest_power_of_two(unsigned long val, int& highbit)
{
	if (cnt_bits(val, highbit) > 1) {
		return 1<<highbit;
	}
	return val;
}

QPixmap find_pixmap(const QString& pixname);

#endif
