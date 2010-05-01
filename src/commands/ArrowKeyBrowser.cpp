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

#include "ArrowKeyBrowser.h"

#include "SheetView.h"

#include "Debugger.h"

ArrowKeyBrowser::ArrowKeyBrowser(SheetView *sv, QVariantList args)
{
        m_sv = sv;
        m_arrow = -1;
        m_repeatInterval = 100;

        if (args.size() >= 1) {
                m_arrow = args.at(0).toInt();
        }
        if (args.size() >=2) {
                m_repeatInterval = args.at(1).toInt();
        }

        connect(&m_browseTimer, SIGNAL(timeout()), this, SLOT(browse()));
}

int ArrowKeyBrowser::begin_hold()
{
        if (m_arrow == -1) {
                // we couldn't detect in the constructor
                // what type of browsing we should do, so
                // bail out here
                return -1;
        }
        m_browseTimer.start(m_repeatInterval);

        return 1;
}

int ArrowKeyBrowser::finish_hold()
{
        m_browseTimer.stop();
        return -1;
}


void ArrowKeyBrowser::browse()
{
        if (m_arrow == Qt::LeftArrow) {
                m_sv->browse_to_previous_context_item();
        } else if (m_arrow == Qt::RightArrow) {
                m_sv->browse_to_next_context_item();
        } else if (m_arrow == Qt::UpArrow) {
                m_sv->browse_to_context_item_above();
        } else if (m_arrow == Qt::DownArrow) {
                m_sv->browse_to_context_item_below();
        } else {
                printf("m_arrow is not a supported arrow key %d\n", m_arrow);
        }
}
