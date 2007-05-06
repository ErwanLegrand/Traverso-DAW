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

#include "Utils.h"
#include "Mixer.h"

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QPixmapCache>
#include <QRegExp>
#include <QLocale>
#include <QChar>


// Frame to MM:SS.30 (smpte time = 30ths of a second)
QString frame_to_smpte ( nframes_t nframes, int rate )
{
	QString spos;
	long unsigned int remainder;
	int mins, secs, frames;

	mins = nframes / ( 60 * rate );
	remainder = nframes - ( mins * 60 * rate );
	secs = remainder / rate;
	remainder -= secs * rate;
	frames = remainder / ( rate / 30 );
	spos.sprintf ( " %02d:%02d%c%02d", mins, secs, QLocale::system().decimalPoint().toAscii(), frames );

	return spos;
}

// Frame to MM:SS.999 (ms)
QString frame_to_msms ( nframes_t nframes, int rate )
{
	QString spos;
	long unsigned int remainder;
	int mins, secs, frames;

	mins = nframes / ( 60 * rate );
	remainder = nframes - ( mins * 60 * rate );
	secs = remainder / rate;
	remainder -= secs * rate;
	frames = remainder * 1000 / rate;
	spos.sprintf ( " %02d:%02d%c%03d", mins, secs, QLocale::system().decimalPoint().toAscii(), frames );

	return spos;
}

// Frame to MM:SS.9 (tenths of a second)
QString frame_to_mst ( nframes_t nframes, int rate )
{
	QString spos;
	long unsigned int remainder;
	int mins, secs, frames;

	mins = nframes / ( 60 * rate );
	remainder = nframes - ( mins * 60 * rate );
	secs = remainder / rate;
	remainder -= secs * rate;
	frames = remainder / ( rate / 10 );
	spos.sprintf ( " %02d:%02d%c%01d", mins, secs, QLocale::system().decimalPoint().toAscii(), frames );

	return spos;
}

// Frame to MM:SS:75 (75ths of a second, for CD burning)
QString frame_to_cd ( nframes_t nframes, int rate )
{
	QString spos;
	long unsigned int remainder;
	int mins, secs, frames;

	mins = nframes / ( 60 * rate );
	remainder = nframes - ( mins * 60 * rate );
	secs = remainder / rate;
	remainder -= secs * rate;
	frames = remainder * 75 / rate;
	spos.sprintf ( " %02d:%02d:%02d", mins, secs, frames );

	return spos;
}

QString frame_to_hms(double nframes, int rate)
{
	long unsigned int remainder;
	int hours, mins, secs;

	hours = (int) (nframes / (3600 * rate));
	remainder = (long unsigned int) (nframes - (hours * 3600 * rate));
	mins = (int) (remainder / ( 60 * rate ));
	remainder -= mins * 60 * rate;
	secs = (int) (remainder / rate);
	return QString().sprintf("%02d:%02d:%02d", hours, mins, secs);
}

QString frame_to_ms(double nframes, int rate)
{
	long unsigned int remainder;
	int hours, mins, secs;

	mins = (int) (nframes / ( 60 * rate ));
	remainder = nframes - (mins * 60 * rate);
	secs = (int) (remainder / rate);
	return QString().sprintf("%02d:%02d", mins, secs);
}


nframes_t smpte_to_frame( QString str, int rate )
{
	nframes_t out = 0;
	QStringList lst = str.simplified().split(QRegExp("[;,.:]"), QString::SkipEmptyParts);

	if (lst.size() >= 1) out += lst.at(0).toInt() * 60 * rate;
	if (lst.size() >= 2) out += lst.at(1).toInt() * rate;
	if (lst.size() >= 3) out += lst.at(2).toInt() * rate / 30;

	return out;
}

nframes_t msms_to_frame( QString str, int rate )
{
	nframes_t out = 0;
	QStringList lst = str.simplified().split(QRegExp("[;,.:]"), QString::SkipEmptyParts);

	if (lst.size() >= 1) out += lst.at(0).toInt() * 60 * rate;
	if (lst.size() >= 2) out += lst.at(1).toInt() * rate;
	if (lst.size() >= 3) out += lst.at(2).toInt() * rate / 1000;

	return out;
}

nframes_t cd_to_frame( QString str, int rate )
{
	nframes_t out = 0;
	QStringList lst = str.simplified().split(QRegExp("[;,.:]"), QString::SkipEmptyParts);

	if (lst.size() >= 1) out += lst.at(0).toInt() * 60 * rate;
	if (lst.size() >= 2) out += lst.at(1).toInt() * rate;
	if (lst.size() >= 3) out += lst.at(2).toInt() * rate / 75;

	return out;
}

QString coefficient_to_dbstring ( float coeff )
{
	float db = coefficient_to_dB ( coeff );

	QString gainIndB;

	if ( db < -99 )
		gainIndB = "- INF";
	else if ( db < 0 )
		gainIndB = "- " + QByteArray::number ( ( -1 * db ), 'f', 1 ) + " dB";
	else
		gainIndB = "+ " + QByteArray::number ( db, 'f', 1 ) + " dB";

	return gainIndB;
}

qint64 create_id( )
{
	int r = rand();
	QDateTime time = QDateTime::currentDateTime();
	uint timeValue = time.toTime_t();
	qint64 id = timeValue;
	id *= 1000000000;
	id += r;

	return id;
}

QDateTime extract_date_time(qint64 id)
{
	QDateTime time;
	time.setTime_t(id / 1000000000);
	return time;
}

QPixmap find_pixmap ( const QString & pixname )
{
	QPixmap pixmap;

	if ( ! QPixmapCache::find ( pixname, pixmap ) )
	{
		pixmap = QPixmap ( pixname );
		QPixmapCache::insert ( pixname, pixmap );
	}

	return pixmap;
}


//eof
