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

$Id: InputEngine.cpp,v 1.7 2006/10/17 00:10:25 r_sijrier Exp $
*/

#include "InputEngine.h"

#include "ContextItem.h"
#include "ContextPointer.h"
#include "Information.h"
#include "IEAction.h"
#include "Command.h"

#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QMetaMethod>
#include <QTranslator>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


#define MAX_TIME_DIFFERENCE_FOR_DOUBLE_KEY_CONSIDERATION 50


static void set_hexcode(int & variable, const QString& text)
{
	variable = 0;
	QString s;
	int x  = 0;
	if ((text != QString::null) && (text.length() > 0) ) {
		s="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		x = s.indexOf(text);
		if (x>=0)
			variable = Qt::Key_A + x;
		else {
			s="|ESC     |TAB     |BACKTAB |BKSPACE |RETURN  |ENTER   |INSERT  |DELETE  "
			"|PAUSE   |PRINT   |SYSREQ  |CLEAR   ";
			x = s.indexOf("|" + text);
			if (x>=0)
				variable = Qt::Key_Escape + (x/9);
			else {
				s="|HOME    |END     |LARROW  |UARROW  |RARROW  "
				"|DARROW  |PRIOR   |NEXT    ";
				x = s.indexOf("|" + text);
				if (x>=0)
					variable = Qt::Key_Home + (x/9);
				else {
					s="|SHIFT   |CTRL    |META    |ALT     |CAPS    "
					"|NUMLOCK |SCROLL  ";
					x = s.indexOf("|" + text);
					if (x>=0)
						variable = Qt::Key_Shift + (x/9);
					else {
						s="F1 F2 F3 F4 F5 F6 F7 F8 F9 F10F11F12";
						x=s.indexOf(text);
						if (x>=0)
							variable = Qt::Key_F1 + (x/3);
						else if (text=="SPACE")
							variable = Qt::Key_Space;
					}
				}
			}
		}
	}
	PMESG3("HEXCODE FOR %s=%d", text.toAscii().data(), variable);
}


InputEngine& ie()
{
	static InputEngine inputengine;
	return inputengine;
}

InputEngine::InputEngine()
{
	PENTERCONS;
	holdingCommand = 0;

	isJogging = false;
	reset();

	clearTime = 2000;
	assumeHoldTime = 200; // it will wait a release for 200 ms. Otherwise it will assume a hold
	doubleFactWaitTime = 200;
	collectedNumber = -1;
	sCollectedNumber = "-1";
	locked=false; // THIS SHOULD CALL everythingLocked
	activate();
}

InputEngine::~ InputEngine( )
{}

void InputEngine::free_memory( )
{
	while(!ieActions.isEmpty())
		delete  ieActions.takeFirst();
}

void InputEngine::activate()
{
	PENTER3;
	isFirstFact=true;
	active=true;
}


void InputEngine::suspend()
{
	PENTER3;
	active=false;
}

int InputEngine::broadcast_action_from_contextmenu(const QString& name)
{
	PENTER2;
	IEAction* action = 0;

	foreach(IEAction* ieaction, ieActions) {
		if (ieaction->name == name) {
			action = ieaction;
			break;
		}
	}

	if ( (action->type == HOLDKEY) || (action->type == HKEY2) ) {
		info().information(QObject::tr("Hold actions are not supported yet from Context Menu"));
		return -1;
	}

	return broadcast_action(action);
}


int InputEngine::broadcast_action(IEAction* action)
{
	PENTER2;

	Command* k = 0;
	QObject* item = 0;

	QList<QObject* > list = cpointer().get_context_items();

	QString slotName = action->slotName;
	PMESG("slotName is %s", slotName.toAscii().data());

	for (int i=0; i < list.size(); ++i) {
		item = list.at(i);
		if (!item)
			continue;
		if(QMetaObject::invokeMethod(item, slotName.toAscii().data(),
					Qt::DirectConnection,
					Q_RETURN_ARG(Command*, k))) {
			PMESG("HIT, invoking %s::%s", item->metaObject()->className(), slotName.toAscii().data());
			break;
		} else {
			PMESG("nope %s wasn't the right one, next ...", item->metaObject()->className());
		}
	}

	if (k && (!isHolding)) {
		if (k->prepare_actions() && k->do_action()) {
			k->set_valid(true);
			k->push_to_history_stack();
		}
	}
	if (k && isHolding) {
		if (k->begin_hold()) {
			k->set_valid(true);
			k->set_cursor_shape(action->useX, action->useY);
			holdingCommand = k;
			set_jogging(true);
		} else {
			// OOPSSS, something went wrong when making the Command
			// set following stuff to zero to make finish_hold do nothing
			delete k;
			set_jogging( false );
			wholeMapIndex = -1;
		}
	}


	return 1;
}

void InputEngine::process_command( Command * cmd )
{
	Q_ASSERT(cmd);
	
	if (cmd->prepare_actions() && cmd->do_action()) {
		cmd->set_valid(true);
		if (cmd->push_to_history_stack() < 0) {
			delete cmd;
		}
	}
}


void InputEngine::jog()
{
	PENTER3;
	if (isJogging) {
		if (holdingCommand) {
			holdingCommand->jog();
		}
	}
}


void InputEngine::set_jogging(bool jog)
{
	if (jog)
		cpointer().grab_mouse();
	else
		cpointer().release_mouse();

	isJogging = jog;
}

bool InputEngine::is_jogging()
{
	return isJogging;
}



// JMB ENGINE : EVENT LEVEL HANDLING -------------------------------

// reset the event stack and the press 'stack' (not exactly a stack)
void InputEngine::reset()
{
	PENTER3;
	isFirstFact = true;
	isDoubleKey = false;
	fact1_k1 = 0;
	fact1_k2 = 0;
	isHolding = false;
	isPressEventLocked = false;
	stackIndex = 0;
	pressEventCounter = 0;
	fact2_k1 = 0;
	fact2_k2 = 0;
	wholeMapIndex = -1;
	for (int i=0; i < STACK_SIZE; i++) {
		eventType[i] = 0;
		eventStack[i] = 0;
		eventTime[i] = 0;
	}
}



// Everthing starts here. Catch event takes anything happen in the keyboard
// and pushes it into a stack.
void InputEngine::catch_press(QKeyEvent * e )
{
	PENTER3;
	int k = e->key();
	if (!isPressEventLocked) {
		if (pressEventCounter < 2) {
			pressEventCounter++;
			push_event(PRESS_EVENT, k);
			press_checker();
			if (!catcher.holdTimer.isActive())
				catcher.holdTimer.start( assumeHoldTime); // single shot timer
		} else {
			isPressEventLocked = true;
		}
	}
}

void InputEngine::catch_release( QKeyEvent * e)
{
	PENTER3;
	int k = e->key();
	if ((!is_fake(k)) && (stackIndex != 0)) {
		push_event(RELEASE_EVENT, k);
		release_checker();
	}
}

void InputEngine::catch_scroll(QWheelEvent* e)
{
	scrollEvent = e;
	IEAction action;

	if (e->orientation() == Qt::Horizontal) {
		if (e->delta() > 0)
			action.slotName = "scroll_left";
		if (e->delta() < 0)
			action.slotName = "scroll_right";
	} else {
		if (e->delta() > 0)
			action.slotName = "scroll_up";
		if (e->delta() < 0)
			action.slotName = "scroll_down";
	}

	broadcast_action(&action);
}

QWheelEvent* InputEngine::get_scroll_event()
{
	return scrollEvent;
}

// This pushes an event to the stack
void InputEngine::push_event(  int pType,  int pCode )
{
	PENTER3;
	if (stackIndex < STACK_SIZE) {
		eventType[stackIndex] = pType;
		eventStack[stackIndex] = pCode;
		QTime currTime = QTime::currentTime();
		long ts=currTime.msec() + (currTime.second() * 1000) + (currTime.minute() * 1000 * 60);
		eventTime[stackIndex] = ts;
		PMESG3("Pushing EVENT %d (%s) key=%d at %ld",stackIndex,( pType==PRESS_EVENT ? "PRESS" : "RELEASE" ),pCode,ts);
		stackIndex++;
		for (int j=0; j<STACK_SIZE; j++) {
			PMESG4("eventStack[%d]=%d %s at timestamp=%ld\n",j,eventStack[j],(eventType[j]==PRESS_EVENT?"P":"R"),eventTime[j]);
		}
	}
}


// this is a intermediate step to push. Only non-fake events are allowed to be pushed
bool InputEngine::is_fake(int codeVal)
{
	// A fake event is an event which is generated by a ignored PRESS_EVENT
	bool b =   ( codeVal!=eventStack[0] )
		&& ( codeVal!=eventStack[1] )
		&& ( codeVal!=eventStack[2] )
		&& ( codeVal!=eventStack[3] );
	return b;
}

// this is the method that detects wether a fact is double key (KK) or single key (K)
// by measuring the time difference between the first 2 presses (in case event[0] AND
// event[1] are PRESS_EVENT's
void InputEngine::press_checker()
{
	PENTER3;
	if (stackIndex==2) // first two events happend
	{
		PMESG3("Checking if 2 first events are PRESS'es and what is the time diff between them");
		if (     ( eventType[0]==PRESS_EVENT )
				&& ( eventType[1]==PRESS_EVENT )
		)
		{
			isDoubleKey = ( eventTime[1] - eventTime[0] < MAX_TIME_DIFFERENCE_FOR_DOUBLE_KEY_CONSIDERATION );
		}
		if (isDoubleKey)
		{
			PMESG3("Detected 2 initial PRESS almost together. It is a double key (KK) !");
		}
	}
}

// This is the one who consolidate a fact. It can detect
// if the fact is K or KK, and also calls the push_fact handler
void InputEngine::release_checker()
{
	PENTER3;
	if (isHolding) {
		finish_hold();
		return;
	}
	// If it is not fake, then lets take a look in the pairs
	// Remember that event 0 is always PRESS_EVENT
	if (eventType[1]==RELEASE_EVENT) {
		// event 0 and event 1 are a pair because event 1 must be the release of 0
		// since the event 1 was a release, then the first fact is finished
		// Now we must push_fact, but only if it is a release very fast (that
		// didnt cause a HOLD.
		if (!isHolding)
			push_fact(eventStack[0],0);
		return;
	}
	// event 1 is ALSO a PRESS_EVENT. so we must check for a double key
	if (isDoubleKey) // ( very short time difference between 2 first presses)
	{
		// Now we dont know if this is the event 2 or event 3
		if (stackIndex==3)
		{
			// So we are processing event (??), which is a release
			// So this can be release of event 0 or 1 , so..
			if (eventStack[2]==eventStack[0])
				pairOf2 = 0;
			else
				pairOf2 = 1;
			// the event is not finished yet but it must be avoided
			// any reentrance (any new PRESS_EVENT)
			isPressEventLocked = true;
			return;
		}
		// we are in the event 3 . the last!
		// the event is finished , surely. But we must check the pairs
		// we already know who event 2 is pair of. So ...
		if (pairOf2 == 0)
			pairOf3 = 1;
		else
			pairOf3 = 0;
		// The event is finished, and we know the pairs. So..
		// I must push_fact, but only if it is a release very fast (that
		// didnt cause a HOLD.
		if (!isHolding)
			push_fact(eventStack[0],eventStack[1]);
	} else {
		// although the sequence was P-P-R-R (press, press, release, release)
		// the time difference between two first presses was too long
		// so it CAN'T be a double key (KK).
		// So the only possible explanation is that this is a >K>K action
		// where user typed too fast, so the release of 1st P happend AFTER the
		// 2nd press. In this case, I have to assume this is a >K>K
		// action, and dispatch TWO facts.
		// Now forcing prematurally a 2 facts by splitting and syncronizing
		// these events into 2 different facts. Probably this is a >K>K
		// of course, this  >K>K assumption mentioned above can be made ONLY if it is the first fact.
		if (isFirstFact) {
			PMESG("Inital double key too slow. It must be a premature >K>K. Dispatching 2 individual <K> facts.");
			int f1_k1=eventStack[0];
			int f2_k1=eventStack[1];
			push_fact (f1_k1,0);
			push_fact (f2_k1,0);
		}
	}

}


// ----------------------------- JMB ENGINE : PRESS LEVEL HANDLING -------------------------


void InputEngine::push_fact(int k1,int k2)
{
	PENTER3;
	PMESG3("Pushing FACT : k1=%d k2=%d",k1,k2);
	catcher.holdTimer.stop(); // quit the holding check..
	if (isFirstFact) {
		// this is the first fact
		PMESG3("First fact detected !");
		// first try to find some action like k1k2
		fact1_k1 = k1;
		fact1_k2 = k2;

		// first check if this fact is just a collected number
		check_number_collection();
		if (isCollecting) {
			// another digit was collected.
			reset();
			return;
		}
		int mapIndex = identify_first_fact();
		if (mapIndex < 0) {
			PMESG3("First fact alone does not match anything in the map. Waiting for a second fact...");
			// Action not identified. Maybe is part of a double fact action. so...
			give_a_chance_for_second_fact();
			return;
		}
		PMESG3("First fact matches map in position %d",mapIndex);
		// there is a single-fact action which matches this !! now must check if is not an immediate action
		if (!ieActions.at(mapIndex)->isInstantaneous) {
			PMESG3("Although this could be an SINGLE PRESS Action, it is not protected, so...");
			// action is not an immediate action, so...
			give_a_chance_for_second_fact();
			return;
		}
		// Action exists AND it is a immediate action. So
		// forces it to be a single fact action
		PMESG3("This is protected (immediate) action. It'll be treated as <K>");
		dispatch_action(mapIndex);
		conclusion();
	} else // ok . We are in the second fact.
	{
		catcher.secondChanceTimer.stop();
		fact2_k1 = k1;
		fact2_k2 = k2;
		if (fact2_k1!=0) {
			// this is the second press
			PMESG3("Second fact detected !");
		}
		// try to complement the first press.
		wholeMapIndex = identify_first_and_second_facts_together();
		if (wholeMapIndex >= 0) {
			PMESG3("First and second facts together matches with action %d !! Dispatching it...",wholeMapIndex );
			if (locked) {
				return;
			}
			dispatch_action(wholeMapIndex);
		} else {
			PMESG3("Apparently, first and second facts together do not match any action. Sorry :-(");
		}
		conclusion();
	}
}





int InputEngine::identify_first_fact()
{
	PENTER3;
	fact1Type = 0;
	// First we need to know the first fact type.
	if (fact1_k2==0) // <K> or [K]
	{
		if (!isHolding)
		{
			PMESG3("Detected <K>");
			fact1Type = FKEY;
		} else
		{
			PMESG3("Detected [K]");
			fact1Type = HOLDKEY;
		}
	}
	else // <KK> or [KK]
	{
		if (!isHolding) {
			PMESG3("Detected <KK>");
			fact1Type = FKEY2;
		} else {
			PMESG3("Detected [KK]");
			fact1Type = HKEY2;
		}
	}

	// Fact 1 Type identified .
	PMESG3("Now, is there a SINGLE FACT action which matches type %d and %d,%d", fact1Type, fact1_k1, fact1_k2);
	foreach(IEAction* action, ieActions) {
		int ap1k1 = action->fact1_key1;
		int ap1k2 = action->fact1_key2;
		int ap2k1 = action->fact2_key1;
		int ap2k2 = action->fact2_key2;
		PMESG4("COMPARING %d,%d,%d,%d \tWITH %d,%d,%d,%d",ap1k1,ap1k2,ap2k1,ap2k2,fact1_k1,fact1_k2,fact2_k1,fact2_k2);
		if (action->type != fact1Type )
			continue;
		if (
			(
				((ap1k1==fact1_k1) && (ap1k2==fact1_k2))
				||
				((ap1k1==fact1_k2) && (ap1k2==fact1_k1))
			)
			&&
			( ap2k1 == 0 )
			&&
			( ap2k2 == 0 )
		) {
			// 'i' is a candidate for first press
			PMESG3("Found a match in map position %d", ieActions.indexOf(action));
			return ieActions.indexOf(action);
		}
	}
	PMESG3("No single fact candidate action found. Keep going, since a 2nd fact might come soon");
	give_a_chance_for_second_fact();
	return -1;
}


// This is called whenever a second fact happens right after the first
int InputEngine::identify_first_and_second_facts_together()
{
	PENTER3;
	PMESG3("Adding a 2nd fact %d,%d  to the 1st one  %d,%d",fact2_k1,fact2_k2,fact1_k1,fact1_k2);
	if (fact2_k1!=0) {
		if (!isHolding) {
			if (fact1_k2==0) // first press is <K> (I know that its not [K] because if it was I'd never reach identify_first_and_second_facts_together())
			{
				if (fact2_k2==0)
					if (fact1_k1 == fact2_k1)
					{
						PMESG3("Whole action is a <<K>>");
						wholeActionType = D_FKEY;  // <<K>>
					} else
					{
						PMESG3("Whole action is a >K>K");
						wholeActionType = S_FKEY_FKEY; // >K>K
					}
				else
				{
					PMESG3("Whole action is a >K>KK");
					wholeActionType = S_FKEY_FKEY2;  // >K>KK
				}
			}

			else {
				if (fact2_k2==0) {
					PMESG3("Whole action is a >KK>K");
					wholeActionType = S_FKEY2_FKEY;  // >KK>K
				} else {
					if (
						((fact1_k1==fact2_k1) && (fact1_k2==fact2_k2)) ||
						((fact1_k1==fact2_k2) && (fact1_k2==fact2_k1))
					) {
						PMESG3("Whole action is a <<KK>>");
						wholeActionType = D_FKEY2;
					} else {
						PMESG3("Whole action is a >KK>KK");
						wholeActionType = S_FKEY2_FKEY2;
					}
				}
			}
		} else {
			if (fact1_k2==0) // first press is <K> (I know that its not [K] because if it was I'd never reach identify_first_and_second_facts_together())
			{
				if (fact2_k2==0)
					if (fact1_k1 == fact2_k1)
					{
						PMESG3("Whole action is a <[K]>");
						wholeActionType = FHKEY;
					} else
					{
						PMESG3("Whole action is a >K[K]");
						wholeActionType = S_FKEY_HKEY;
					}
				else
				{
					PMESG3("Whole action is a >K[KK]");
					wholeActionType = S_FKEY_HKEY2;
				}
			}
			else {
				if (fact2_k2==0) {
					PMESG3("Whole action is a KK[K]");
					wholeActionType = S_FKEY2_HKEY;
				} else {
					PMESG3("Whole action is a >KK[KK]");
					wholeActionType = S_FKEY2_HKEY2;
				}
			}
		}
	} else {
		PMESG3("Second fact is null (0,0). Assuming wholeActionType is %d", fact1Type);
		wholeActionType = fact1Type;
	}

	// whole action type identified .
	PMESG3("Searching for a %d action that matches %d,%d,%d,%d ", wholeActionType, fact1_k1, fact1_k2, fact2_k1, fact2_k2);
	foreach(IEAction* action, ieActions) {
		if ( action->type != wholeActionType )
			continue;
		int ap1k1 = action->fact1_key1;
		int ap1k2 = action->fact1_key2;
		int ap2k1 = action->fact2_key1;
		int ap2k2 = action->fact2_key2;
		PMESG4("COMPARING %d,%d,%d,%d  \tWITH  %d,%d,%d,%d",ap1k1,ap1k2,ap2k1,ap2k2,fact1_k1,fact1_k2,fact2_k1,fact2_k2);
		if (
			(
				((ap1k1==fact1_k1) && (ap1k2==fact1_k2))
				||
				((ap1k1==fact1_k2) && (ap1k2==fact1_k1))
			)
			&&
			(
				((ap2k1==fact2_k1) && (ap2k2==fact2_k2))
				||
				((ap2k1==fact2_k2) && (ap2k2==fact2_k1))
			)
		) {
			// 'i' is a candidate the whole action
			PMESG3("Found a match : action %s", action->slotName.data());
			return ieActions.indexOf(action);
		}
	}
	PMESG3("No candidates found :-(");
	return -1;
}


void InputEngine::give_a_chance_for_second_fact()
{
	PENTER3;
	PMESG3("Waiting %d ms for second fact ...",doubleFactWaitTime );
	catcher.secondChanceTimer.start( doubleFactWaitTime );
	isFirstFact=false;
	isHolding = false;
	isPressEventLocked = false;
	stackIndex = 0;
	pressEventCounter = 0;
	fact2_k1 = 0;
	fact2_k2 = 0;
	wholeMapIndex = -1;
	for (int i=0; i < STACK_SIZE; i++) {
		eventType[i] = 0;
		eventStack[i] = 0;
		eventTime[i] = 0;
	}
}



// ----------------------------- JMB ENGINE : ACTION LEVEL HANDLING -------------------------
void InputEngine::dispatch_action(int mapIndex)
{
	PENTER2;
	broadcast_action(ieActions.at(mapIndex));
}



// This is called by
void InputEngine::dispatch_hold()
{
	PENTER2;
	catcher.clearOutputTimer.stop();
	isHoldingOutput=false;

	wholeMapIndex = -1;
	if (isFirstFact) {
		fact1_k1 = eventStack[0];
		fact1_k2 = eventStack[1];
		wholeMapIndex = identify_first_fact(); // I can consider first press the last because there is nothing after a []
	} else {
		fact2_k1 = eventStack[0];
		fact2_k2 = eventStack[1];
		wholeMapIndex = identify_first_and_second_facts_together();
	}

	if (wholeMapIndex>=0) {
		IEAction* action = ieActions.at(wholeMapIndex);
		if ((action->useX) && (action->useY)) {
			// print("Move Mouse U/D/L/R",1);
		} else if (action->useX) {
			// print("Move Mouse Left/Right",1);
		} else if(action->useY) {
			// print("Move Mouse Up/Down",1);
		}
		broadcast_action(ieActions.at(wholeMapIndex));
	} else {}
	stop_collecting();
	// note that we dont call conclusion() here :-)
}




void InputEngine::finish_hold()
{
	PENTER3;
	PMESG("Finishing hold action %d",wholeMapIndex);

	isHolding = false;
	set_jogging(false);

	if (holdingCommand) {

		holdingCommand->finish_hold();
		holdingCommand->set_cursor_shape();

		if (holdingCommand->prepare_actions()) {
			holdingCommand->set_valid(true);
			holdingCommand->do_action();
		} else {
			holdingCommand->set_valid( false );
		}

		if (holdingCommand->push_to_history_stack() < 0) {
			delete holdingCommand;
		}

		holdingCommand = (Command*) 0;
	}
	
	conclusion();
}


void InputEngine::conclusion()
{
	PENTER3;
	reset();
	hold_output();
}


void InputEngine::hold_output()
{
	PENTER3;
	if (!isHoldingOutput) {
		catcher.clearOutputTimer.start(clearTime);
		isHoldingOutput=true;
	}
}


int InputEngine::init_map(const QString& mapFilename)
{
	PENTER2;
	PMESG2("INITIALIZING KEYMAP ... ");

	IEAction* action;

	QDomDocument doc("keymap");
	QFile file(mapFilename);
	if (!file.open(QIODevice::ReadOnly))
		return -1;
	if (!doc.setContent(&file)) {
		file.close();
		return -1;
	}
	file.close();

	QDomElement root = doc.documentElement();
	QDomNode n = root.firstChild();

	QString keyFactType;
	QString key1;
	QString key2;
	QString key3;
	QString key4;
	QString mouseHint;
	QString slot;
	QString name;
	int sortOrder = 0;

	int keyCode1, keyCode2, keyCode3, keyCode4 = 0;
	int parsedActionType=-1;

	bool useX , useY;


	while( !n.isNull() ) {
		QDomElement e = n.toElement();
		if( !e.isNull() ) {
			if( e.tagName() == "keyseq" ) {
				keyFactType = e.attribute( "keyfacttype", "" );
				key1 = e.attribute( "key1", "" );
				key2 = e.attribute( "key2", "" );
				name = e.attribute( "name", "Name not set");
				sortOrder = e.attribute( "sortorder", "1").toInt();

				mouseHint = e.attribute( "mousehint", "" );
				slot = e.attribute( "slotname", "" );

				if (keyFactType == "FKEY")
					parsedActionType = FKEY;
				else if (keyFactType == "FKEY2")
					parsedActionType = FKEY2;
				else if (keyFactType == "HKEY")
					parsedActionType = HOLDKEY;
				else if (keyFactType == "HKEY2")
					parsedActionType = HKEY2;
				else if (keyFactType == "D_FKEY")
					parsedActionType = D_FKEY;
				else if (keyFactType == "D_FKEY2")
					parsedActionType = D_FKEY2;
				else if (keyFactType == "S_FKEY_FKEY")
					parsedActionType = S_FKEY_FKEY;
				else {
					PWARN("keyFactType not supported!");
				}

				useX = useY = false;
				
				if (mouseHint == "LR")
					useX = true;
				if (mouseHint == "UD")
					useY = true;
				if (mouseHint == "LRUD")
					useX = useY = true;

				set_hexcode(keyCode1, key1);
				set_hexcode(keyCode2, key2);
				set_hexcode(keyCode3, key3);
				set_hexcode(keyCode4, key4);

				// Fix the keyCode positions
				if  (( parsedActionType == D_FKEY  ) || ( parsedActionType == FHKEY  )) {
					keyCode3=keyCode1;
				} else if (( parsedActionType == D_FKEY2 ) || ( parsedActionType == FHKEY  )) {
					keyCode3=keyCode1;
					keyCode4=keyCode2;
				} else if (( parsedActionType == S_FKEY_FKEY ) || ( parsedActionType == S_FKEY_HKEY )) {
					keyCode3=keyCode2;
					keyCode2=0;
				}

			}
			PMESG2("ADDED action: type=%d keys=%d,%d,%d,%d useX=%d useY=%d, slot=%s", parsedActionType, keyCode1,keyCode2,keyCode3,keyCode4,useX,useY, slot.toAscii().data());
			action = new IEAction();
			action->set
			(parsedActionType, keyCode1, keyCode2, keyCode3, keyCode4, useX, useY, slot, key1, key2, key3, key4, name, sortOrder);
			ieActions.append(action);
		}
		n = n.nextSibling();
	}


	PMESG2("Optimizing map for best performance ...");
	int optimizedActions=0;
	foreach(IEAction* action, ieActions) {
		int c1A = action->fact1_key1;
		int c2A = action->fact1_key2;

		if ( (action->type == FKEY) ) {
			bool alone = true;
			foreach(IEAction* aloneAction, ieActions) {
				if (action == aloneAction)
					continue;
				int tt    = aloneAction->type;
				int t1A   = aloneAction->fact1_key1;
				if  (
					(t1A==c1A)  &&
					(
						(tt==D_FKEY)       ||
						(tt==FHKEY)        ||
						(tt==S_FKEY_FKEY)  ||
						(tt==S_FKEY_FKEY2) ||
						(tt==S_FKEY_HKEY)  ||
						(tt==S_FKEY_HKEY2)
					)
				)
					alone=false;
			}
			if (alone) {
				PMESG3("Setting <K> fact (slot=%s) as protected (Instantaneous response)", action->slotName.data());
				action->set_instantaneous(true);
				optimizedActions++;
			} else
				action->set_instantaneous(false);

		} else if ((action->type == FKEY2)) {
			bool alone = true;
			foreach(IEAction* aloneAction, ieActions) {
				if (action == aloneAction)
					continue;
				int tt    = aloneAction->type;
				int t1A   = aloneAction->fact1_key1;
				int t2A   = aloneAction->fact1_key2;
				if (
					((t1A==c1A) && (t2A==c2A)) &&
					(
						(tt==HOLDKEY)         ||
						(tt==D_FKEY2)      ||
						(tt==S_FKEY2_FKEY) ||
						(tt==S_FKEY2_FKEY2)||
						(tt==S_FKEY2_HKEY) ||
						(tt==S_FKEY2_HKEY2)
					)
				)
					alone=false;
			}
			if (alone) {
				PMESG3("Setting <KK> fact (slot=%s) for instantaneous response", action->slotName.data());
				action->set_instantaneous(true);
				optimizedActions++;
			} else
				action->set_instantaneous(false);
		}
	}
	PMESG2("Keymap initialized! %d actions registered ( %d instantanious) .", ieActions.size(), optimizedActions);

	return 1;
}


void InputEngine::set_clear_time(int time)
{
	clearTime = time;
}

void InputEngine::set_hold_sensitiveness(int htime)
{
	assumeHoldTime=htime;
}

void InputEngine::set_double_fact_interval(int time)
{
	doubleFactWaitTime = time;
}


// Number colector
void InputEngine::check_number_collection()
{
	PENTER3;
	if ((fact1_k1 >= 0x30) && (fact1_k1 <= 0x39)) {
		if (!isCollecting) {
			PMESG3("Starting number collection...");
			sCollectedNumber="";
		}
		isCollecting = true;
		sCollectedNumber.append( QChar(fact1_k1) ); // it had a ",1" complement after fact1_k1... why?
		PMESG3("Collected %s so far...",sCollectedNumber.toAscii().data() ) ;
		QString sn = "NUMBER " + sCollectedNumber;
		collectedNumber = sCollectedNumber.toInt();
	} else
		stop_collecting();
}

void InputEngine::stop_collecting()
{
	PENTER3;
	isCollecting=false;
	collectedNumber = sCollectedNumber.toInt();
	sCollectedNumber ="-1";
}


void InputEngine::lock()
{
	PENTER3;
	locked=true;
	PMESG3("now locked=%d",locked);
}

void InputEngine::unlock()
{
	PENTER3;
	locked=false;
	PMESG3("now locked=%d",locked);
}

bool InputEngine::is_locked()
{
	return locked;
}


int InputEngine::collected_number( )
{
	int n = collectedNumber;
	sCollectedNumber = "-1"; // collectedNumber has a life of only one get.
	return n;
}

bool InputEngine::is_holding( )
{
	return isHolding;
}

QList< IEAction* > InputEngine::get_contextitem_actionlist( ContextItem * ci )
{
	QList<IEAction* > list;
	ContextItem* item = ci;
	QString string;
	do {
		// 		PWARN("My name is %s ", item->metaObject()->className());
		const QMetaObject* mo = item->metaObject();
		for (int i=0; i < mo->methodCount(); i++) {
			if (mo->method(i).methodType() == QMetaMethod::Slot) {
				string = mo->method(i).signature();
				string = string.left(string.indexOf("("));
				// 				PWARN("signature is %s", string.toAscii().data());
				for (int i=0; i<ieActions.size(); i++) {
					if (string == ieActions.at(i)->slotName) {
						list.append(ieActions.at(i));
					}
				}
			}
		}
	} while ( (item = item->get_context()) );

	return list;
}


//---------- EVENT CATCHER -------------



EventCatcher::EventCatcher()
{
	holdTimer.setSingleShot(true);
	connect( &holdTimer, SIGNAL(timeout()), this, SLOT(assume_hold()));
	connect( &secondChanceTimer, SIGNAL(timeout()), this, SLOT(quit_second_chance()));
	connect( &clearOutputTimer, SIGNAL(timeout()), this, SLOT(clear_output()));
}


void EventCatcher::assume_hold() // no release so far ? so consider it a hold...
{
	PENTER3;
	PMESG3("No release so far (waited %d ms). Assuming this is a hold and dispatching it",ie().assumeHoldTime);
	holdTimer.stop(); // quit the holding check..
	ie().isHolding = true;
	ie().dispatch_hold();
}

void EventCatcher::clear_output()
{
	PENTER3;
	if ((ie().isHoldingOutput)) {
		clearOutputTimer.stop();
		ie().isHoldingOutput=false;
	}
}

void EventCatcher::quit_second_chance()
{
	PENTER3;
	secondChanceTimer.stop();
	if (!ie().isHolding) // if it is holding, there is no need to push a new fact
	{
		PMESG3("No second fact (waited %d ms) ... Forcing a null second fact",ie().doubleFactWaitTime);
		ie().push_fact(0,0); // no second press performed, so I am adding a pair of Zeros as second press and keep going...
	}
}

//eof
