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
 
    $Id: Utils.cpp,v 1.2 2006/06/20 19:27:03 r_sijrier Exp $
*/

#include "Utils.h"
#include "Mixer.h"

#include <QString>

QString frame_to_smpte(nframes_t nframes, int rate)
{
        QString spos;
        long unsigned int remainder;
        int mins, secs, frames;

        mins = nframes / (60 * rate);
        remainder = nframes - (mins * 60 * rate);
        secs = remainder / rate;
        remainder -= secs * rate;
        frames = remainder / (rate / 30);
        spos.sprintf (" %02d:%02d,%02d", mins, secs, frames);

        return spos;
}

QString coefficient_to_dbstring( float coeff )
{
	float db = coefficient_to_dB(coeff);

	QString gainIndB;
	
	if (db < -99)
		gainIndB = "- INF";
	else if ( db < 0)
		gainIndB = "- " + QByteArray::number((-1 * db), 'f', 1) + " dB";
	else
		gainIndB = "+ " + QByteArray::number(db, 'f', 1) + " dB";
		
	return gainIndB;
}

//eof
