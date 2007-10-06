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

$Id: FileHelpers.h,v 1.4 2007/10/06 14:17:58 r_sijrier Exp $
*/

#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include <QString>

class FileHelper
{
public:

	static int remove_recursively(const QString& pName);
	static int copy_recursively(const QString& pNameFrom, const QString& pNameTo);
	static QString fileerror_to_string(int error);
};

#endif
