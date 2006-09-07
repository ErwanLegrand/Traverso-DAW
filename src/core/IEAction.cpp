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

$Id: IEAction.cpp,v 1.3 2006/09/07 09:36:52 r_sijrier Exp $
*/

#include "IEAction.h"
#include "InputEngine.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

void IEAction::set
	(	int pType,
		int pFact1_k1,
		int pFact1_k2,
		int pFact2_k1,
		int pFact2_k2,
		bool newUseX,
		bool newUseY,
		const QString& slot,
		const QString& key1, 
		const QString& key2, 
		const QString& key3, 
		const QString& key4, 
		const QString& actionName, 
		int order)
{
	type = pType;
	fact1_key1 = pFact1_k1;
	fact1_key2 = pFact1_k2;
	fact2_key1 = pFact2_k1;
	fact2_key2 = pFact2_k2;
	useX = newUseX;
	useY = newUseY;
	isInstantaneous = false;
	slotName = slot.toAscii().data();
	name = actionName.toAscii().data();
	sortOrder = order;
	switch(type) {
	case	FKEY:
		keySequence = QString("< " + key1 + " >").toAscii().data();
		break;
	case	FKEY2:
		keySequence = QString("< " + key1 + " " + key2 + " >").toAscii().data();
		break;
	case	HOLDKEY:
		keySequence = QString("[ " + key1 + " ]").toAscii().data();
		break;
	case	HKEY2:
		keySequence = QString("[ " + key1 +  " " + key2 + " ]").toAscii().data();
		break;
	case	D_FKEY:
		keySequence = QString("<< " + key1 + " >>").toAscii().data();
		break;
	case	D_FKEY2:
		keySequence = QString("<< " + key1 + " " + key2 + " >>").toAscii().data();
		break;
	case	S_FKEY_FKEY:
		keySequence = QString("> " + key1 + " > " + key2).toAscii().data();
		break;
	default	:
		keySequence = "Unknown Key Sequence";
	}
}


void IEAction::set_instantaneous(bool status)
{
	isInstantaneous = status;
}


// EOF

