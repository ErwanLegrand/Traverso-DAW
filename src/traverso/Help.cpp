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
 
    $Id: Help.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include "Help.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Help::Help()
{
        PENTERCONS;
        created=false;

        cpointer().add_contextitem(this);
}

Help::~Help()
{
        PENTERDES;
}


void Help::create()
{
        setMinimumSize(600,200);
        resize(600,200);

        view = new QTextEdit( this );
        view->setReadOnly(true);
        view->setPlainText( "This is a <b>Test</b> with <i>italic</i> <u>stuff</u> ");
        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(view);
        setMinimumSize( 450, 600 );
        setLayout(mainLayout);

        QFile file(":/helpfile");
        if ( file.open(QIODevice::ReadOnly) ) {
                QTextStream text( &file );
                view->setPlainText( text.readAll() );
        }
}


Command* Help::show_help()
{
        PENTER;
        if (!created) {
                create();
                created = true;
        }
        if (!isHidden())
                this->hide();
        else
                QWidget::show();
        return (Command*) 0;
}



//eof

