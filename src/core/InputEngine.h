/*
    Copyright (C) 2005-2011 Remon Sijrier

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

#ifndef INPUTENGINE_H
#define INPUTENGINE_H



#include <QObject>
#include <QTimer>
#include <QHash>
#include <QPoint>

#include "defines.h"

class TCommand;
class CommandPlugin;
class TFunction;
class QKeyEvent;
class QWheelEvent;
class QMouseEvent;

struct TShortcut;


class InputEngine : public QObject
{
	Q_OBJECT
public:

        void catch_key_press(QKeyEvent *);
        void catch_key_release(QKeyEvent *);
        void catch_mousebutton_press( QMouseEvent * e );
        void catch_mousebutton_release( QMouseEvent * e );
        void catch_mousebutton_doubleclick( QMouseEvent * e );
        void catch_scroll(QWheelEvent * e );

        int collected_number();
        bool has_collected_number();
        QString get_collected_number() const {return m_sCollectedNumber;}

	bool is_jogging();
	bool is_holding();

	TCommand* get_holding_command() const;
	void filter_unknown_sequence(QString& sequence);

	int dispatch_shortcut_from_contextmenu(const QString& name);

        void jog();
        void bypass_jog_until_mouse_movements_exceeded_manhattenlength(int length=50);
        void update_jog_bypass_pos();
	void abort_current_hold_actions();
	
	TCommand* succes();
	TCommand* failure();
	TCommand* did_not_implement();

private:
        InputEngine();
        InputEngine(const InputEngine&) : QObject() {}
        ~InputEngine();

	enum BroadcastResult {
		SUCCES=1,
		FAILURE=2,
		DIDNOTIMPLEMENT=3
	};

        struct HoldModifierKey {
                int             keycode;
                bool            wasExecuted;
                trav_time_t     lastTimeExecuted;
		TShortcut*      shortcut;
        };

	QList<int>		m_modifierKeys;
	QList<int>		m_activeModifierKeys;
        QHash<int, HoldModifierKey*>  m_holdModifierKeys;
	QHash<QString, int>	m_modes;
        TCommand* 		m_holdingCommand;
        QString			m_sCollectedNumber;
	QPoint			m_jogBypassPos;
        QTimer                  m_holdKeyRepeatTimer;


        bool 			m_isHolding;
        bool 			m_isJogging;
	bool			m_cancelHold;
	bool			m_bypassJog;

        int 			m_collectedNumber;
	int			m_dispatchResult;
	int			m_unbypassJogDistance;
        int                     m_holdEventCode;

        void 			finish_hold();
        void 			conclusion();
        void 			stop_collecting();
        bool 			check_number_collection(int eventcode);

        //! call the slot that handler a given action
	int dispatch_shortcut(TShortcut* shortCut, bool autorepeat=false, bool fromContextMenu=false);

        void set_jogging(bool jog);
        void set_numerical_input(const QString& number);
	void reset();
	void process_press_event(int keyValue);
	void process_release_event(int keyValue);
	bool is_modifier_keyfact(int eventcode);
	bool modifierKeysMatch(QList<int> first, QList<int> second);
        void clear_hold_modifier_keys();


        // allow this function to create one instance
        friend InputEngine& ie();

private slots:
        void process_hold_modifier_keys();

signals:
        void collectedNumberChanged();
        void jogStarted();
        void jogFinished();
};

// use this function to get the InputEngine object
InputEngine& ie();


#endif

//eof
