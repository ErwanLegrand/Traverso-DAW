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
 
    $Id: InfoBox.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "InfoBox.h"
#include "ProjectInfoWidget.h"
#include "SongInfoWidget.h"
#include "SystemInfoWidget.h"
#include "MessageWidget.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

InfoBox::InfoBox(QWidget* parent)
                : QWidget(parent)
{
        PENTERCONS;
        setAttribute(Qt::WA_NoBackground);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setMargin(0);
        mainLayout->setSpacing(0);

        QWidget* infoWidget= new QWidget(this);
        QHBoxLayout* infoWidgetsLayout= new QHBoxLayout(infoWidget);
        infoWidgetsLayout->setMargin(0);
        infoWidgetsLayout->setSpacing(0);

        projectInfo = new ProjectInfoWidget(infoWidget);
        songInfo	= new SongInfoWidget(infoWidget);
        systemInfo = new SystemInfoWidget(infoWidget);

        infoWidgetsLayout->addWidget(projectInfo, 2);
        infoWidgetsLayout->addWidget(songInfo, 3);
        infoWidgetsLayout->addWidget(systemInfo, 2);

        infoWidget->setLayout(infoWidgetsLayout);

        QWidget* bottomnWidget = new QWidget(this);
        messageWidget = new MessageWidget(bottomnWidget);
        QHBoxLayout* messageWidgetLayout= new QHBoxLayout(bottomnWidget);
        messageWidgetLayout->addWidget(messageWidget, 10);
        messageWidgetLayout->setMargin(0);
        messageWidgetLayout->setSpacing(0);


        bottomnWidget->setLayout(messageWidgetLayout);

        mainLayout->addWidget(infoWidget, 4);
        mainLayout->addWidget(bottomnWidget);
        setLayout(mainLayout);
}

InfoBox::~InfoBox()
{}


//eof
