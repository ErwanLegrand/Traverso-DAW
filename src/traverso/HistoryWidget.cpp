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

    $Id: HistoryWidget.cpp,v 1.1 2006/09/07 09:30:56 r_sijrier Exp $
*/


#include <HistoryStack.h>
#include "HistoryWidget.h"


UndoModel::UndoModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    m_stack = 0;
    m_selModel = new QItemSelectionModel(this, this);
    connect(m_selModel, SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(setStackCurrentIndex(QModelIndex)));
    m_emtyLabel = tr("<empty>");
}

QItemSelectionModel* UndoModel::selectionModel() const
{
    return m_selModel;
}

HistoryStack* UndoModel::get_stack() const
{
    return m_stack;
}

void UndoModel::set_stack(HistoryStack* stack)
{
    if (m_stack == stack)
        return;

    if (m_stack != 0) {
        disconnect(m_stack, SIGNAL(cleanChanged(bool)), this, SLOT(stackChanged()));
        disconnect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(stackChanged()));
        disconnect(m_stack, SIGNAL(destroyed(QObject*)), this, SLOT(stackDestroyed(QObject*)));
    }
    m_stack = stack;
    if (m_stack != 0) {
        connect(m_stack, SIGNAL(cleanChanged(bool)), this, SLOT(stackChanged()));
        connect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(stackChanged()));
        connect(m_stack, SIGNAL(destroyed(QObject*)), this, SLOT(stackDestroyed(QObject*)));
    }

    stackChanged();
}

void UndoModel::stackDestroyed(QObject* obj)
{
    if (obj != m_stack)
        return;
    m_stack = 0;

    stackChanged();
}

void UndoModel::stackChanged()
{
    reset();
    m_selModel->setCurrentIndex(selectedIndex(), QItemSelectionModel::ClearAndSelect);
}

void UndoModel::setStackCurrentIndex(const QModelIndex& index)
{
    if (m_stack == 0)
        return;

    if (index == selectedIndex())
        return;

    if (index.column() != 0)
        return;

    m_stack->set_index(index.row());
}

QModelIndex UndoModel::selectedIndex() const
{
    return m_stack == 0 ? QModelIndex() : createIndex(m_stack->index(), 0);
}

QModelIndex UndoModel::index(int row, int column, const QModelIndex &parent) const
{
    if (m_stack == 0)
        return QModelIndex();

    if (parent.isValid())
        return QModelIndex();

    if (column != 0)
        return QModelIndex();

    if (row < 0 || row > m_stack->count())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex UndoModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int UndoModel::rowCount(const QModelIndex &parent) const
{
    if (m_stack == 0)
        return 0;

    if (parent.isValid())
        return 0;

    return m_stack->count() + 1;
}

int UndoModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant UndoModel::data(const QModelIndex &index, int role) const
{
    if (m_stack == 0)
        return QVariant();

    if (index.column() != 0)
        return QVariant();

    if (index.row() < 0 || index.row() > m_stack->count())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (index.row() == 0)
            return m_emtyLabel;
        return m_stack->text(index.row() - 1);
    } else if (role == Qt::DecorationRole) {
        if (index.row() == m_stack->clean_index() && !m_cleanIcon.isNull())
            return m_cleanIcon;
        return QVariant();
    }

    return QVariant();
}

QString UndoModel::empty_label() const
{
    return m_emtyLabel;
}

void UndoModel::set_empty_label(const QString& label)
{
    m_emtyLabel = label;
    stackChanged();
}

void UndoModel::set_clean_icon(const QIcon& icon)
{
    m_cleanIcon = icon;
    stackChanged();
}

QIcon UndoModel::get_clean_icon() const
{
    return m_cleanIcon;
}

void HistoryWidget::init()
{
	m_model = new UndoModel(this);
	setModel(m_model);
	setSelectionModel(m_model->selectionModel());
		
	connect(UndoGroup::instance(), SIGNAL(activeStackChanged(HistoryStack*)), m_model, SLOT(set_stack(HistoryStack*)));
}

HistoryWidget::HistoryWidget(QWidget* parent)
    : QListView(parent)
{
	init();
}

HistoryWidget::HistoryWidget(HistoryStack* stack, QWidget* parent)
    : QListView(parent)
{
    init();
    set_stack(stack);
}


HistoryWidget::~HistoryWidget()
{
}

HistoryStack* HistoryWidget::get_stack() const
{
    return m_model->get_stack();
}

void HistoryWidget::set_stack(HistoryStack* stack)
{
    m_model->set_stack(stack);
}


void HistoryWidget::set_empty_label(const QString& label)
{
    m_model->set_empty_label(label);
}

QString HistoryWidget::empty_label() const
{
    return m_model->empty_label();
}

void HistoryWidget::set_clean_icon(const QIcon& icon)
{
    m_model->set_clean_icon(icon);

}

QIcon HistoryWidget::get_clean_icon() const
{
    return m_model->get_clean_icon();
}

//eof
