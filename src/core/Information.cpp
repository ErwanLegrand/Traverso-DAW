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
 
    $Id: Information.cpp,v 1.3 2007/04/02 20:51:45 r_sijrier Exp $
*/

#include "Information.h"
#include "Utils.h"

#include "Debugger.h"


Information& info()
{
        static Information information;
        return information;
}

void Information::information( const QString & mes )
{
        InfoStruct s;
        s.message = mes;
        s.type = INFO;
	PWARN("Information::information %s", QS_C(mes));
	emit message(s);
}

void Information::warning( const QString & mes )
{
        InfoStruct s;
        s.message = mes;
        s.type = WARNING;
	PWARN("Information::warning %s", QS_C(mes));
        emit message(s);
}


void Information::critical( const QString & mes )
{
        InfoStruct s;
        s.message = mes;
        s.type = CRITICAL;
	PERROR("Information::critical %s", QS_C(mes));
	emit message(s);
}

//eof
