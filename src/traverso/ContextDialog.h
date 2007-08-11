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

$Id: ContextDialog.h,v 1.3 2007/08/11 22:54:58 benjie Exp $
*/

#ifndef CONTEXT_DIALOG_H
#define CONTEXT_DIALOG_H

#include <ViewPort.h>


class ContextItem;

class ContextDialog : public ViewPort
{
public:
	ContextDialog();
	~ContextDialog();
	
	void get_pointed_context_items(QList<ContextItem* > &list) {Q_UNUSED(list);};

protected:
        void keyPressEvent ( QKeyEvent* e);
        void keyReleaseEvent ( QKeyEvent* e);
};

#endif

//eof
