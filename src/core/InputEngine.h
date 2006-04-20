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
 
    $Id: InputEngine.h,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#ifndef INPUTENGINE_H
#define INPUTENGINE_H


#include <QEvent>
#include <QWheelEvent>
#include <QString>
#include <QObject>
#include <QTimer>

#include "IEAction.h"
#include "Command.h"

class ContextItem;

static const int FKEY = 0;                 // <K>    - press one key fast
static const int FKEY2 = 1;                // <KK>   - press two keys fast, together
static const int HKEY = 2;                 // [K]    - Hold one key
static const int HKEY2 = 3;                // [KK]   - Hold two keys, together
static const int D_FKEY = 4;               // <<K>>  - double press one key fast
static const int D_FKEY2 = 5;              // <<KK>> - double press two keys fast, together
static const int FHKEY = 6;                // <[K]>  - press fast and then hold one key
static const int FHKEY2 = 7;               // <[KK]> - press fast and then hold two keys together
static const int S_FKEY_FKEY = 8;          // >K>K   - press one key and then other key fast and sequentially
static const int S_FKEY_FKEY2 = 9;         // >K>KK  - press one key and then two other keys fast and sequentially
static const int S_FKEY2_FKEY = 10;        // >KK>K  - press two keys and then another one fast and sequentially
static const int S_FKEY2_FKEY2 = 11;       // >KK>KK  - press two keys and then another one fast and sequentially
static const int S_FKEY_HKEY = 12;         // >K[K]  - press fastly one key and then hold another key, sequentially
static const int S_FKEY_HKEY2 = 13;        // >K[KK] - press fastly one key and then hold two ther ones, sequentially
static const int S_FKEY2_HKEY = 14;        // >KK[K] - press two keys together fastly and then hold a third one, sequentially
static const int S_FKEY2_HKEY2 = 15;       // >KK[KK] - press two keys together fastly and then hold a third one, sequentially

// internal class used for qt event catching
class EventCatcher : public QObject
{
        Q_OBJECT
public:
        EventCatcher();

        QTimer holdTimer;
        QTimer clearOutputTimer;
        QTimer secondChanceTimer;

public slots:
        void assume_hold();
        void quit_second_chance();
        void clear_output();
};


class InputEngine
{
public:

        //! Free the memory on program exit.
        void free_memory();

        //! process a QT press event */
        void catch_press(QKeyEvent *);


        //! process a QT release event */
        void catch_release(QKeyEvent *);

        //! process a Qt Wheel Event
        void catch_scroll(QWheelEvent * e );

        QWheelEvent*	get_scroll_event();

        int collected_number();

        bool is_holding();

        QList<IEAction* > get_contextitem_actionlist(ContextItem* item);

        int broadcast_action_from_contextmenu(QString name);

        void jog();

        bool is_jogging();

        //! activate the actions recognition engine and start responding to them
        void activate();


        //! Suspends the engine . No action will be handled until activate() is called again
        void suspend();


        //! Initialize the JMB map using mapFilename as the map
        //! @param mapFilename the jmb map file. More on jmb map files in here : libtraverso.h
        int init_map(QString mapFilename);


        //! internal method. Not meant to be called by clients
        void reset();


        //! internal method. Not meant to be called by clients
        void set_clear_time(int time);


        //! set the hold sensitiveness
        void set_hold_sensitiveness(int factor);


        //! set the double press interval in ms
        void set_double_fact_interval(int time);


        //! lock the JMB engine
        void lock()
                ;


        //! return unlock the JMB engine */
        void unlock();


        //! return return true JMB engine is locked*/
        bool is_locked();



private:
        InputEngine();
        InputEngine(const InputEngine&)
{}
        ~InputEngine();

        static const int 	STACK_SIZE = 4;
        static const int 	MAX_ACTIONS = 300;
        static const int 	PRESS_EVENT = 1;
        static const int 	RELEASE_EVENT = 2;

        QList<IEAction* >	ieActions;
        EventCatcher 		catcher;
        Command* 		holdingCommand;
        QString			sCollectedNumber;
        QString 			targetHoldSlot;
        QObject* 			targetHoldObject;
        QWheelEvent*		scrollEvent;

        bool 			active;
        bool 			isHolding;
        bool 			isPressEventLocked;
        bool 			isHoldingOutput;
        bool 			isFirstFact;
        bool 			locked;
        bool 			isDoubleKey;
        bool 			isJogging;
        bool 			isCollecting;

        int 			fact1_k1;
        int 			fact1_k2;
        int 			fact2_k1;
        int 			fact2_k2;
        int 			fact1Type;
        int 			wholeMapIndex;
        int 			wholeActionType;
        int 			collectedNumber;
        int 			stackIndex;
        int 			eventType[STACK_SIZE];
        int 			eventStack[STACK_SIZE];
        int 			pressEventCounter; // that avoid more than 2 press events in a same fact
        int 			pairOf2;
        int 			pairOf3;
        int 			clearTime;
        int 			assumeHoldTime;
        int 			doubleFactWaitTime;
        long 			eventTime[STACK_SIZE];

        bool 			is_fake( int keyval);
        int 			identify_first_fact();
        int 			identify_first_and_second_facts_together();
        void 			push_event(int pType,  int pKey);
        void 			press_checker();
        void 			release_checker();
        void 			push_fact( int k1 ,  int k2);
        void 			give_a_chance_for_second_fact();
        void 			dispatch_action(int mapIndex);
        void 			dispatch_hold();
        void 			finish_hold();
        void 			conclusion();
        void 			hold_output();
        void 			stop_collecting();
        void 			check_number_collection();

        //! call the slot that handler a given action
        int broadcast_action(IEAction* action);

        void set_jogging(bool jog);

        friend class EventCatcher;

        // allow this function to create one instance
        friend InputEngine& ie();


};

// use this function to get the InputEngine object
InputEngine& ie();


#endif

//eof

