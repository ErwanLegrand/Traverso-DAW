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
 
    $Id: IEAction.cpp,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#include "IEAction.h"
#include "InputEngine.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

IEAction::IEAction()
                : QObject()
{}

IEAction::~IEAction()
{}

void IEAction::set
        (int pType,
                        int pFact1_k1,
                        int pFact1_k2,
                        int pFact2_k1,
                        int pFact2_k2,
                        bool newUseX,
                        bool newUseY,
                        QString slot,
                        QString key1, QString key2, QString key3, QString key4, QString actionName, int order)
{
        type = pType;
        fact1_key1 = pFact1_k1;
        fact1_key2 = pFact1_k2;
        fact2_key1 = pFact2_k1;
        fact2_key2 = pFact2_k2;
        useX = newUseX;
        useY = newUseY;
        isInstantaneous = false;
        slotName = slot;
        name = actionName;
        sortOrder = order;
        switch(type) {
        case	FKEY:
                keySequence = "< " + key1 + " >";
                break;
        case	FKEY2:
                keySequence = "< " + key1 + " " + key2 + " >";
                break;
        case	HKEY:
                keySequence = "[ " + key1 + " ]";
                break;
        case	HKEY2:
                keySequence = "[ " + key1 +  " " + key2 + " ]";
                break;
        case	D_FKEY:
                keySequence = "<< " + key1 + " >>";
                break;
        case	D_FKEY2:
                keySequence = "<< " + key1 + " " + key2 + " >>";
                break;
        case	S_FKEY_FKEY:
                keySequence = "> " + key1 + " > " + key2;
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


