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

$Id: HistoryStack.h,v 1.2 2006/09/07 09:36:52 r_sijrier Exp $
*/

#ifndef HISTORY_STACK_H
#define HISTORY_STACK_H

#include <QObject>
#include <QString>
#include <QAction>


class QAction;
class Command;
class UndoGroup;

class HistoryStack : public QObject
{
	Q_OBJECT

public:
	HistoryStack(QObject* parent = 0);
	~HistoryStack();
	void clear();
	
	void push(Command* cmd);
	
	bool can_undo() const;
	bool can_redo() const;
	QString undo_text() const;
	QString redo_text() const;
	
	int count() const;
	int index() const;
	QString text(int idx) const;
	
	QAction* create_undo_action(QObject* parent, const QString& prefix = QString()) const;
	QAction* create_redo_action(QObject* parent, const QString& prefix = QString()) const;
	
	bool is_active() const;
	bool is_clean() const;
	int clean_index() const;

        UndoGroup& undogroup();

private:
	QList<Command*> m_commandList;
	UndoGroup* 	m_group;
	int		m_index;
	int 		m_cleanIndex;
	
	friend class UndoGroup;
	
	void private_set_index(int idx, bool clean);


public slots:
	void set_clean();
	void set_index(int idx);
	void undo();
	void redo();
	void set_active(bool active = true);
	

signals:
	void indexChanged(int idx);
	void cleanChanged(bool clean);
	void canUndoChanged(bool can_undo);
	void canRedoChanged(bool can_redo);
	void undoTextChanged(const QString& undo_text);
	void redoTextChanged(const QString& redo_text);

};

class UndoAction : public QAction
{
	Q_OBJECT
public:
	UndoAction(const QString& prefix, QObject* parent = 0);

public slots:
	void set_prefixed_text(const QString& text);

private:
	QString m_prefix;
};



class UndoGroup : public QObject
{
    Q_OBJECT

public:
	UndoGroup(QObject *parent = 0);
	
	void add_stack(HistoryStack *stack);
	void remove_stack(HistoryStack *stack);
	QList<HistoryStack*> stacks() const;
	HistoryStack* active_stack() const;
	
	static UndoGroup* instance();
	
	QAction* create_undo_action(QObject* parent, const QString &prefix = QString()) const;
	QAction* create_redo_action(QObject* parent, const QString &prefix = QString()) const;
	bool can_undo() const;
	bool can_redo() const;
	QString undo_text() const;
	QString redo_text() const;
	bool is_clean() const;
	
private:
	HistoryStack* 		m_active;
	QList<HistoryStack*> 	m_stack_list;
	static UndoGroup*	m_instance;


public slots:
	void undo();
	void redo();
	void set_active_stack(HistoryStack* stack);

signals:
	void activeStackChanged(HistoryStack* stack);
	void indexChanged(int idx);
	void cleanChanged(bool clean);
	void canUndoChanged(bool canUndo);
	void canRedoChanged(bool canRedo);
	void undoTextChanged(const QString& undoText);
	void redoTextChanged(const QString& redoText);
};


#endif // HISTORY_STACK_H
