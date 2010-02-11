/*
Copyright (C) 2010 Remon Sijrier

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


#ifndef SUBGROUP_H
#define SUBGROUP_H

#include "AudioProcessingItem.h"

class SubGroup : public AudioProcessingItem
{

public:
        SubGroup(const QString& name, int channelCount);
        ~SubGroup();

        bool is_smaller_then(APILinkedListNode* node) {return true;}


};

#endif // SUBGROUP_H
