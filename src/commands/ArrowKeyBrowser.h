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

#ifndef ARROWKEYBROWSER_H
#define ARROWKEYBROWSER_H

#include "Command.h"
#include <QTimer>

class SheetView;

class ArrowKeyBrowser : public Command
{
        Q_OBJECT
public:
        ArrowKeyBrowser(SheetView* sv, QVariantList args);

        int begin_hold();
        int finish_hold();


private:
        int             m_arrow;
        int             m_repeatInterval;
        SheetView*      m_sv;
        QTimer          m_browseTimer;

private slots:
        void browse();
};

#endif // ARROWKEYBROWSER_H
