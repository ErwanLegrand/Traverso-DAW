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
 
    $Id: Command.h,v 1.4 2006/09/07 09:36:52 r_sijrier Exp $
*/

#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QString>
#include <QCursor>
#include <QPixmap>

class HistoryStack;
class ContextItem;

class Command : public QObject
{
public :
        Command(ContextItem* item, const QString& des = "No description set!");
        Command(const QString& des = "No description set!");
        virtual ~Command();

        virtual int begin_hold();
        virtual int finish_hold();
        virtual int prepare_actions();
        virtual int do_action();
        virtual int undo_action();
        virtual int jog();
        virtual bool merg_with(const Command* cmd);
        virtual int command_type();
        virtual void set_cursor_shape(int useX = 0, int useY = 0);

        void set_valid(bool valid);
        void set_description(const QString& des);
        int push_to_history_stack();
        QString get_description() const {return m_description;}

        bool is_valid() const {return m_isValid;}

        Command* prev;
        Command* next;


protected:
        bool 		m_isValid;
        bool		m_historable;
        bool		m_isMergable;
        QString		m_description;

private:
        bool expired;
        HistoryStack* m_historyStack;
        ContextItem* m_context;


};


#endif


