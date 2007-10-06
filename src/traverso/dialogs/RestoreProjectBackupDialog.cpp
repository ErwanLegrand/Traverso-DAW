/*
    Copyright (C) 2007 Remon Sijrier
 
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

#include "RestoreProjectBackupDialog.h"
#include "ProjectManager.h"
#include <QTreeWidgetItem>
#include <QDateTime>

#include "Information.h"

RestoreProjectBackupDialog::RestoreProjectBackupDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
}


void RestoreProjectBackupDialog::accept()
{
	QTreeWidgetItem* item = dateTreeWidget->currentItem();
	
	if (!item) {
		reject();
		return;
	}
	
	uint restoretime = item->data(0, Qt::UserRole).toUInt();
	int sucess = pm().restore_project_from_backup(m_projectname, restoretime);
	
	if (sucess) {
		pm().load_project(m_projectname);
		info().information(tr("Succesfully restored backup from %1").arg(QDateTime::fromTime_t(restoretime).toString()));
		hide();
	}
	
}

void RestoreProjectBackupDialog::reject()
{
	hide();
}

void RestoreProjectBackupDialog::populate_treeview()
{
	dateTreeWidget->clear();
	
	currentDateLable->setText(QDateTime::currentDateTime ().toString("dd-MM-yy hh:mm:ss"));
	
	QList<uint> list = pm().get_backup_date_times(m_projectname);
	QDateTime datetime;
	foreach(uint time, list) {
		QTreeWidgetItem* item = new QTreeWidgetItem(dateTreeWidget);
		datetime.setTime_t(time);
		item->setText(0, datetime.toString("dd-MM-yy"));
		item->setText(1, datetime.toString("hh:mm:ss"));
		item->setData(0, Qt::UserRole, time);
	}
	
	dateTreeWidget->sortItems(0, Qt::DescendingOrder);
	
	if (list.size()) {
		QTreeWidgetItem* item = dateTreeWidget->invisibleRootItem()->child(0);
		dateTreeWidget->setCurrentItem(item);
		lastBackupLable->setText(item->text(0) + " " + item->text(1));
	} else {
		lastBackupLable->setText(tr("No backup(s) available!"));
	}
}

void RestoreProjectBackupDialog::set_project_name(const QString & projectname)
{
	m_projectname = projectname;
	populate_treeview();
}

