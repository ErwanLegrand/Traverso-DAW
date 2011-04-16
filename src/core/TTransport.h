/*
Copyright (C) 2011 Remon Sijrier

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

#ifndef TTRANSPORT_H
#define TTRANSPORT_H

#include "TGlobalContext.h"

class TCommand;

class TTransport : public TGlobalContext
{
	Q_OBJECT
public:
	TTransport();

public slots:
	TCommand* start_transport();
	TCommand* set_recordable_and_start_transport();
	TCommand* to_start();
	TCommand* to_end();
};

#endif // TTRANSPORT_H
