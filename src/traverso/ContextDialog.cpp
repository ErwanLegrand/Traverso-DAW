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

$Id: ContextDialog.cpp,v 1.1 2006/08/03 14:33:02 r_sijrier Exp $
*/
 
#include "ContextDialog.h"

#include <QHBoxLayout>

#include "ViewPort.h"
#include <InputEngine.h>

#include <Debugger.h>

ContextDialog::ContextDialog()
	: QDialog(0, Qt::SubWindow)
{
	PENTERCONS;
	
	QHBoxLayout* layout = new QHBoxLayout(this);
	
	m_vp = new ViewPort(this);
	
	layout->addWidget(m_vp);
	layout->setMargin(0);
	layout->setSpacing(0);
	
	setLayout(layout);
	
}

ContextDialog::~ContextDialog()
{
	PENTERDES;
}

void ContextDialog::keyPressEvent( QKeyEvent * e)
{
	if ( (e->key() == Qt::Key_Return) || (e->key() == Qt::Key_Escape) ) {
		close();
	}
	
	if (!e->isAutoRepeat())
		ie().catch_press(e);
	e->ignore();
}

void ContextDialog::keyReleaseEvent( QKeyEvent * e)
{
	if (!e->isAutoRepeat())
		ie().catch_release(e);
	e->ignore();
}


//eof
