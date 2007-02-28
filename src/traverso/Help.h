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
 
    $Id: Help.h,v 1.4 2007/02/28 21:23:09 r_sijrier Exp $
*/

#ifndef HELP_H
#define HELP_H

#include <QDialog>
#include <QTextBrowser>

class Command;

class Help : public QDialog
{
        Q_OBJECT
public:
        Help(QWidget* parent = 0);

private:
        QTextBrowser *view;
        bool created;

        void create();

public slots:
        Command* show_help();

};


#endif

//eof

