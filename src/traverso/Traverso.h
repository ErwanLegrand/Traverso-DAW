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

    $Id: Traverso.h,v 1.10 2007/10/27 17:57:16 r_sijrier Exp $
*/

#ifndef Traverso_H
#define Traverso_H

#include <QApplication>
#include <QSessionManager>

class Traverso : public QApplication
{
	Q_OBJECT

public :

        Traverso(int &argc, char **argv );
        ~Traverso();


        void shutdown(int signal);

protected:
        void saveState ( QSessionManager& manager );
        void commitData ( QSessionManager& manager );

private :
	void init_sse();
	void setup_fpu();
        void prepare_audio_device();
	
private slots:
	void create_interface();
};

#endif
