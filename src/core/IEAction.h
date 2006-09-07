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
 
    $Id: IEAction.h,v 1.3 2006/09/07 09:36:52 r_sijrier Exp $
*/

#ifndef IEACTION_H
#define IEACTION_H

#include <QString>
#include <QByteArray>
#include <QObject>

class IEAction
{
public:
        //! constructs a IEAction
        IEAction(){};
        ~IEAction(){};

        //! set the action caracteristics
        //! @param pId
        //! @param pType
        //! @param pFact1_k1
        //! @param pFact1_k2
        //! @param pFact2_k1
        //! @param pFact2_k2
        //! @param useX
        //! @param useY
        void set(int pType,
        	 int pFact1_k1, 
        	 int pFact1_k2, 
        	 int pFact2_k1, 
        	 int pFact2_k2,
        	 bool useX, 
        	 bool useY, 
        	 const QString& slot,
        	 const QString& key1, 
        	 const QString& key2, 
        	 const QString& key3, 
        	 const QString& key4,
        	 const QString& actionName,
        	 int order);

        //! if an action is set to be instantaneous, when it is recognized by jmb , jmb will not wait for a second fact.
        void set_instantaneous(bool status);

        static bool smaller(const IEAction* left, const IEAction* right )
        {
                return left->sortOrder < right->sortOrder;
        }
        static bool greater(const IEAction* left, const IEAction* right )
        {
                return left->sortOrder > right->sortOrder;
        }

        int type;
        int fact1_key1;
        int fact1_key2;
        int fact2_key1;
        int fact2_key2;
        bool useX;
        bool useY;
        bool isInstantaneous;
        QByteArray slotName;
        QByteArray keySequence;
        QByteArray name;
        int sortOrder;
};



#endif

// eof

