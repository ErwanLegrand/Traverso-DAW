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

    $Id: HistoryWidget.h,v 1.1 2006/09/07 09:30:56 r_sijrier Exp $
*/


#ifndef HISTORY_WIDGET_H
#define HISTORY_WIDGET_H

#include <QtGui>
#include <QString>


class HistoryStack;
class QIcon;


class UndoModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    UndoModel(QObject *parent = 0);

    HistoryStack* get_stack() const;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QModelIndex selectedIndex() const;
    QItemSelectionModel *selectionModel() const;

    QString empty_label() const;
    void set_empty_label(const QString& label);

    void set_clean_icon(const QIcon& icon);
    QIcon get_clean_icon() const;

public slots:
    void set_stack(HistoryStack* stack);

private slots:
    void stackChanged();
    void stackDestroyed(QObject *obj);
    void setStackCurrentIndex(const QModelIndex& index);

private:
    HistoryStack*		m_stack;
    QItemSelectionModel*	m_selModel;
    QString 			m_emtyLabel;
    QIcon 			m_cleanIcon;
};



class  HistoryWidget : public QListView
{
    Q_OBJECT
    Q_PROPERTY(QString empty_label READ empty_label WRITE set_empty_label)
    Q_PROPERTY(QIcon get_clean_icon READ get_clean_icon WRITE set_clean_icon)

public:
	explicit HistoryWidget(QWidget* parent = 0);
	explicit HistoryWidget(HistoryStack* stack, QWidget* parent = 0);
	~HistoryWidget();
	
	HistoryStack* get_stack() const;
	
	void set_empty_label(const QString& label);
	QString empty_label() const;
	
	void set_clean_icon(const QIcon& icon);
	QIcon get_clean_icon() const;

private:
	void init();
	
	UndoModel*	m_model;

public slots:
	void set_stack(HistoryStack* stack);
};


#endif // HISTORY_WIDGET_H
 
