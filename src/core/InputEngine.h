/*
    Copyright (C) 2005-2008 Remon Sijrier

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


#include <QKeyEvent>
#include <QWheelEvent>
#include <QString>
#include <QObject>
#include <QTimer>
#include <QHash>
#include <QStringList>

#include "defines.h"


class ContextItem;
class Command;
class CommandPlugin;

static const int FKEY = 0;                 // <K>    - press one key fast
static const int FKEY2 = 1;                // <KK>   - press two keys fast, together
static const int HOLDKEY = 2;                 // [K]    - Hold one key
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


struct IEAction
{
        void render_key_sequence(const QString& key1, const QString& key2);
	~IEAction();
        struct Data {
        	QStringList modes;
		QVariantList arguments;
		QList<int > modifierkeys;
        	QString slotsignature;
		QString pluginname;
		QString commandname;
		QString submenu;
		bool useX;
		bool useY;
		int sortorder;
                int autorepeatInterval;
                int autorepeatStartDelay;
                bool isHoldModifierKey;
        };

        QHash<QString, Data*> objects;
	QHash<QString, Data*> objectUsingModifierKeys;

        int type;
        int fact1_key1;
        int fact1_key2;
        int fact2_key1;
        int fact2_key2;
        int autorepeatInterval;
        int autorepeatStartDelay;
        bool isInstantaneous;
        QByteArray keySequence;
};


struct MenuData {
        static bool smaller(const MenuData left, const MenuData right )
        {
                return left.sortorder < right.sortorder;
        }
        static bool greater(const MenuData* left, const MenuData* right )
        {
                return left->sortorder > right->sortorder;
        }
	QString 	keysequence;
	QString		iedata;
        QString		description;
	QString		submenu;
        int		sortorder;
	QList<int > 	modifierkeys;
};

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

	QList<MenuData > create_menudata_for(QObject* item);
	Command* get_holding_command() const;
	void create_menudata_for_metaobject(const QMetaObject* mo, QList<MenuData >& list) const;

        int broadcast_action_from_contextmenu(const QString& name);

        void jog();
        void bypass_jog_until_mouse_movements_exceeded_manhattenlength(int length=50);
        void update_jog_bypass_pos();

        void activate();
        void suspend();
	void abort_current_hold_actions();
	
	Command* succes();
	Command* failure();
	Command* did_not_implement();


        int init_map(const QString& mapFilename);

        void set_clear_time(int time);
        void set_hold_sensitiveness(int factor);
        void set_double_fact_interval(int time);


private:
        InputEngine();
        InputEngine(const InputEngine&) : QObject() {}
        ~InputEngine();

        static const int 	STACK_SIZE = 4;
        static const int 	MAX_ACTIONS = 300;
        static const int 	PRESS_EVENT = 1;
        static const int 	RELEASE_EVENT = 2;
	
	enum BroadcastResult {
		SUCCES=1,
		FAILURE=2,
		DIDNOTIMPLEMENT=3
	};

        struct HoldModifierKey {
                int             keycode;
                bool            wasExecuted;
                trav_time_t     lastTimeExecuted;
                IEAction*       ieaction;
        };

        QList<IEAction* >	m_ieActions;
	QList<int>		m_modifierKeys;
	QList<int>		m_activeModifierKeys;
        QHash<int, HoldModifierKey*>  m_holdModifierKeys;
	QHash<QString, CommandPlugin*> m_commandplugins;
	QHash<QString, int>	m_modes;
        Command* 		m_holdingCommand;
        QString			m_sCollectedNumber;
	QPoint			m_jogBypassPos;
        QTimer                  m_holdKeyRepeatTimer;
        QTimer                  m_holdTimer;
        QTimer                  m_clearOutputTimer;
        QTimer                  m_secondChanceTimer;


        bool 			m_active;
        bool 			m_isHolding;
        bool 			m_isPressEventLocked;
        bool 			m_isHoldingOutput;
        bool 			m_isFirstFact;
        bool 			m_isDoubleKey;
        bool 			m_isJogging;
	bool			m_cancelHold;
	bool			m_bypassJog;

        int 			m_fact1_k1;
        int 			m_fact1_k2;
        int 			m_fact2_k1;
        int 			m_fact2_k2;
        int 			m_fact1Type;
        int 			m_wholeMapIndex;
        int 			m_wholeActionType;
        int 			m_collectedNumber;
        int 			m_stackIndex;
        int 			m_eventType[STACK_SIZE];
        int 			m_eventStack[STACK_SIZE];
        int 			m_pressEventCounter; // that avoid more than 2 press events in a same fact
        int 			m_pairOf2;
        int 			m_pairOf3;
        int 			m_clearTime;
        int 			m_assumeHoldTime;
        int 			m_doubleFactWaitTime;
        long 			m_eventTime[STACK_SIZE];
	int			m_broadcastResult;
	int			m_unbypassJogDistance;
        int                     m_holdEventCode;

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
        bool 			check_number_collection(int eventcode);

        //! call the slot that handler a given action
        int broadcast_action(IEAction* action, bool autorepeat=false, bool fromContextMenu=false);

        void set_jogging(bool jog);
        void set_numerical_input(const QString& number);
	void reset();
        void process_press_event(int eventcode);
        void process_release_event(int eventcode);
        int find_index_for_instant_hold_key( int key );
	int find_index_for_instant_fkey( int key );
	int find_index_for_instant_fkey2( int key1, int key2 );
        int find_index_for_single_fact(int type, int key1, int key2);
	bool is_modifier_keyfact(int eventcode);
        void clear_hold_modifier_keys();


        // allow this function to create one instance
        friend InputEngine& ie();

private slots:
        void assume_hold();
        void quit_second_chance();
        void clear_output();
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
