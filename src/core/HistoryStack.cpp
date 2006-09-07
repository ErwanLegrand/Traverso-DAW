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

$Id: HistoryStack.cpp,v 1.2 2006/09/07 09:36:52 r_sijrier Exp $
*/

#include "HistoryStack.h"

#include "Command.h"

UndoAction::UndoAction(const QString& prefix, QObject* parent)
	: QAction(parent)
{
	m_prefix = prefix;
}

void UndoAction::set_prefixed_text(const QString& text)
{
	QString s = m_prefix;
	if (!m_prefix.isEmpty() && !text.isEmpty())
		s.append(QLatin1Char(' '));
	s.append(text);
	setText(s);
}

HistoryStack::HistoryStack(QObject* parent)
	: QObject(parent), m_group(0), m_index(0), m_cleanIndex(0)
{
	if (UndoGroup* group = qobject_cast<UndoGroup*>(parent))
		group->add_stack(this);
}


HistoryStack::~HistoryStack()
{
	if (m_group != 0)
		m_group->remove_stack(this);
	clear();
}


void HistoryStack::private_set_index(int idx, bool clean)
{
	bool was_clean = (m_index == m_cleanIndex);
	
	if (idx != m_index) {
		m_index = idx;
		emit indexChanged(m_index);
		emit canUndoChanged(can_undo());
		emit undoTextChanged(undo_text());
		emit canRedoChanged(can_redo());
		emit redoTextChanged(redo_text());
	}
	
	if (clean)
		m_cleanIndex = m_index;
	
	bool is_clean = m_index == m_cleanIndex;
	if (is_clean != was_clean)
		emit cleanChanged(is_clean);
}


void HistoryStack::clear()
{
	qDeleteAll(m_commandList);
	m_commandList.clear();
	private_set_index(0, true);
}

void HistoryStack::push(Command* cmd)
{
	Command* cur = 0;

	if (m_index > 0)
		cur = m_commandList.at(m_index - 1);
	while (m_index < m_commandList.size())
		delete m_commandList.takeLast();
	if (m_cleanIndex > m_index)
		m_cleanIndex = -1; // we've deleted the clean state

	m_commandList.append(cmd);
	private_set_index(m_index + 1, false);
}

void HistoryStack::set_clean()
{
	private_set_index(m_index, true);
}

bool HistoryStack::is_clean() const
{
	return m_cleanIndex == m_index;
}

int HistoryStack::clean_index() const
{
	return m_cleanIndex;
}

void HistoryStack::undo()
{
	if (m_index == 0)
		return;
	
	
	int idx = m_index - 1;
	m_commandList.at(idx)->undo_action();
	private_set_index(idx, false);
}

void HistoryStack::redo()
{
	if (m_index == m_commandList.size())
		return;
	
	
	m_commandList.at(m_index)->do_action();
	private_set_index(m_index + 1, false);
}

int HistoryStack::count() const
{
	return m_commandList.size();
}

int HistoryStack::index() const
{
	return m_index;
}

void HistoryStack::set_index(int idx)
{
	if (idx < 0)
		idx = 0;
	else if (idx > m_commandList.size())
		idx = m_commandList.size();
	
	int i = m_index;
	while (i < idx)
		m_commandList.at(i++)->do_action();
	while (i > idx)
		m_commandList.at(--i)->undo_action();
	
	private_set_index(idx, false);
}

bool HistoryStack::can_undo() const
{
	return m_index > 0;
}

bool HistoryStack::can_redo() const
{
	return m_index < m_commandList.size();
}

QString HistoryStack::undo_text() const
{
	if (m_index > 0)
		return m_commandList.at(m_index - 1)->get_description();
	return QString();
}

QString HistoryStack::redo_text() const
{
	if (m_index < m_commandList.size())
		return m_commandList.at(m_index)->get_description();
	return QString();
}

QAction* HistoryStack::create_undo_action(QObject* parent, const QString& prefix) const
{
	QString pref = prefix.isEmpty() ? tr("Redo") : prefix;
	UndoAction *result = new UndoAction(pref, parent);
	result->setEnabled(can_undo());
	result->set_prefixed_text(undo_text());
	connect(this, SIGNAL(canUndoChanged(bool)), result, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(undoTextChanged(QString)),	result, SLOT(set_prefixed_text(QString)));
	connect(result, SIGNAL(triggered()), this, SLOT(undo()));
	return result;
}

QAction* HistoryStack::create_redo_action(QObject* parent, const QString& prefix) const
{
	QString pref = prefix.isEmpty() ? tr("Redo") : prefix;
	UndoAction *result = new UndoAction(pref, parent);
	result->setEnabled(can_redo());
	result->set_prefixed_text(redo_text());
	connect(this, SIGNAL(can_redoChanged(bool)),
		result, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(redo_textChanged(QString)),
		result, SLOT(set_prefixed_text(QString)));
	connect(result, SIGNAL(triggered()), this, SLOT(redo()));
	return result;
}


QString HistoryStack::text(int idx) const
{
	if (idx < 0 || idx >= m_commandList.size())
		return QString();
	return m_commandList.at(idx)->get_description();
}


void HistoryStack::set_active(bool active)
{
	if (m_group != 0) {
		if (active)
		m_group->set_active_stack(this);
		else if (m_group->active_stack() == this)
		m_group->set_active_stack(0);
	}
}

bool HistoryStack::is_active() const
{
    return m_group == 0 || m_group->active_stack() == this;
}


UndoGroup& HistoryStack::undogroup( )
{
	static UndoGroup group;
	return group;
}


/************ UndoGroup****************/
/**************************************/


UndoGroup* UndoGroup::m_instance = 0;


UndoGroup::UndoGroup(QObject* parent)
    : QObject(parent), m_active(0)
{
}

void UndoGroup::add_stack(HistoryStack* stack)
{
	if (m_stack_list.contains(stack))
		return;
	m_stack_list.append(stack);
	
	if (UndoGroup* other = stack->m_group)
		other->remove_stack(stack);
	stack->m_group = this;
}


void UndoGroup::remove_stack(HistoryStack* stack)
{
	if (m_stack_list.removeAll(stack) == 0)
		return;
	if (stack == m_active)
		set_active_stack(0);
	stack->m_group = 0;
}


QList<HistoryStack*> UndoGroup::stacks() const
{
	return m_stack_list;
}

void UndoGroup::set_active_stack(HistoryStack* stack)
{
	if (m_active == stack)
		return;
	
	if (m_active != 0) {
		disconnect(m_active, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
		disconnect(m_active, SIGNAL(undoTextChanged(QString)), this, SIGNAL(undoTextChanged(QString)));
		disconnect(m_active, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
		disconnect(m_active, SIGNAL(redoTextChanged(QString)), this, SIGNAL(redoTextChanged(QString)));
		disconnect(m_active, SIGNAL(indexChanged(int)), this, SIGNAL(indexChanged(int)));
		disconnect(m_active, SIGNAL(cleanChanged(bool)), this, SIGNAL(cleanChanged(bool)));
	}
	
	m_active = stack;
	
	if (m_active == 0) {
		emit canUndoChanged(false);
		emit undoTextChanged(QString());
		emit canRedoChanged(false);
		emit redoTextChanged(QString());
		emit cleanChanged(true);
		emit indexChanged(0);
	} else {
		connect(m_active, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
		connect(m_active, SIGNAL(undoTextChanged(QString)), this, SIGNAL(undoTextChanged(QString)));
		connect(m_active, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
		connect(m_active, SIGNAL(redoTextChanged(QString)), this, SIGNAL(redoTextChanged(QString)));
		connect(m_active, SIGNAL(indexChanged(int)), this, SIGNAL(indexChanged(int)));
		connect(m_active, SIGNAL(cleanChanged(bool)), this, SIGNAL(cleanChanged(bool)));
		emit canUndoChanged(m_active->can_undo());
		emit undoTextChanged(m_active->undo_text());
		emit canRedoChanged(m_active->can_redo());
		emit redoTextChanged(m_active->redo_text());
		emit cleanChanged(m_active->is_clean());
		emit indexChanged(m_active->index());
	}
	
	emit activeStackChanged(m_active);
}

HistoryStack* UndoGroup::active_stack() const
{
	return m_active;
}

void UndoGroup::undo()
{
	if (m_active != 0)
		m_active->undo();
}

void UndoGroup::redo()
{
	if (m_active != 0)
		m_active->redo();
}

bool UndoGroup::can_undo() const
{
	return m_active != 0 && m_active->can_undo();
}

bool UndoGroup::can_redo() const
{
	return m_active != 0 && m_active->can_redo();
}

QString UndoGroup::undo_text() const
{
	return m_active == 0 ? QString() : m_active->undo_text();
}

QString UndoGroup::redo_text() const
{
	return m_active == 0 ? QString() : m_active->redo_text();
}

bool UndoGroup::is_clean() const
{
	return m_active == 0 || m_active->is_clean();
}

QAction* UndoGroup::create_undo_action(QObject *parent, const QString &prefix) const
{
	QString pref = prefix.isEmpty() ? tr("Redo") : prefix;
	UndoAction* result = new UndoAction(pref, parent);
	result->setEnabled(can_undo());
	result->set_prefixed_text(undo_text());
	connect(this, SIGNAL(canUndoChanged(bool)), result, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(undoTextChanged(QString)),	result, SLOT(set_prefixed_text(QString)));
	connect(result, SIGNAL(triggered()), this, SLOT(undo()));
	return result;
}

QAction* UndoGroup::create_redo_action(QObject *parent, const QString &prefix) const
{
	QString pref = prefix.isEmpty() ? tr("Redo") : prefix;
	UndoAction* result = new UndoAction(pref, parent);
	result->setEnabled(can_redo());
	result->set_prefixed_text(redo_text());
	connect(this, SIGNAL(canRedoChanged(bool)), result, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(redoTextChanged(QString)), result, SLOT(set_prefixed_text(QString)));
	connect(result, SIGNAL(triggered()), this, SLOT(redo()));
	return result;
}

UndoGroup * UndoGroup::instance( )
{
	if (! m_instance) {
		m_instance = new UndoGroup();
	}
	
	return m_instance;
}

//eof
