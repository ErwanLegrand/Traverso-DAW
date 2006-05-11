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

$Id: Help.cpp,v 1.3 2006/05/11 13:51:53 r_sijrier Exp $
*/

#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include "Help.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Help::Help(QWidget* parent)
		: QWidget()
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
	resize(700,600);
	setMinimumSize( 500, 400 );

	view = new QTextBrowser( this );
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(view);
	setLayout(mainLayout);

	view->setSource(QUrl("qrc:/index.html"));
	
	created = true;
}


Command* Help::show_help()
{
	PENTER;
	if (!created) {
		create();
	}
	if (!isHidden())
		this->hide();
	else
		this->show();
	return (Command*) 0;
}



//eof

