/*
    Copyright (C) 2007 Remon Sijrier, Nicola Doebelin 
 
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

#include "MarkerDialog.h"

#include <QMessageBox>
#include <QHeaderView>
#include <QPushButton>
#include <QCheckBox>
#include <QString>
#include <ProjectManager.h>
#include <Project.h>
#include <Sheet.h>
#include <TimeLine.h>
#include <Marker.h>
#include <Utils.h>
#include <QDebug>
#include <QTextStream>
#include <QFileDialog>
#include <QDateTime>
#include <AddRemove.h>
#include "Information.h"
#include "PCommand.h"

MarkerDialog::MarkerDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
	
	m_project = pm().get_project();
	m_sheet = m_project->get_current_sheet();
	setWindowTitle("Marker Editor - Project " + m_project->get_title());

	QString mask = "99:99:99,99";
	lineEditPosition->setInputMask(mask);

	markersTreeWidget->header()->resizeSection(1, 100);

	pushButtonRemove->setAutoDefault(false);
	pushButtonExport->setAutoDefault(false);
	pushButtonOk->setAutoDefault(false);

	// connect other stuff related to the treeWidget
	connect(lineEditTitle, SIGNAL(textEdited(const QString &)), this, SLOT(description_changed(const QString &)));
	connect(lineEditPosition, SIGNAL(textEdited(const QString &)), this, SLOT(position_changed(const QString &)));
	connect(markersTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		 this, SLOT(item_changed(QTreeWidgetItem *, QTreeWidgetItem *)));
	connect(pushButtonRemove, SIGNAL(clicked()), this, SLOT(remove_marker()));

	// connect the CD-Text widgets (LineEdits and ToolButtons)
	connect(lineEditPosition, SIGNAL(returnPressed()), this, SLOT(position_enter()));
	connect(lineEditTitle, SIGNAL(returnPressed()), this, SLOT(title_enter()));
	connect(lineEditComposer, SIGNAL(returnPressed()), this, SLOT(composer_enter()));
	connect(lineEditPerformer, SIGNAL(returnPressed()), this, SLOT(performer_enter()));
	connect(lineEditArranger, SIGNAL(returnPressed()), this, SLOT(arranger_enter()));
	connect(lineEditMessage, SIGNAL(returnPressed()), this, SLOT(message_enter()));
	connect(lineEditSheetwriter, SIGNAL(returnPressed()), this, SLOT(sheetwriter_enter()));
	connect(lineEditIsrc, SIGNAL(returnPressed()), this, SLOT(isrc_enter()));

	connect(toolButtonTitleAll, SIGNAL(clicked()), this, SLOT(title_all()));
	connect(toolButtonComposerAll, SIGNAL(clicked()), this, SLOT(composer_all()));
	connect(toolButtonPerformerAll, SIGNAL(clicked()), this, SLOT(performer_all()));
	connect(toolButtonArrangerAll, SIGNAL(clicked()), this, SLOT(arranger_all()));
	connect(toolButtonMessageAll, SIGNAL(clicked()), this, SLOT(message_all()));
	connect(toolButtonSheetwriterAll, SIGNAL(clicked()), this, SLOT(sheetwriter_all()));
	connect(toolButtonCopyAll, SIGNAL(clicked()), this, SLOT(copy_all()));
	connect(toolButtonPEmphAll, SIGNAL(clicked()), this, SLOT(pemph_all()));

	connect(pushButtonExport, SIGNAL(clicked()), this, SLOT(export_toc()));
	connect(pushButtonOk, SIGNAL(clicked()), this, SLOT(apply()));
	connect(pushButtonCancel, SIGNAL(clicked()), this, SLOT(cancel()));

	update_marker_treeview();
}

void MarkerDialog::update_marker_treeview()
{
	int currentIndex = markersTreeWidget->indexOfTopLevelItem(markersTreeWidget->currentItem());

	// since the treeWidget will be cleared, point m_marker to somewhere else
	m_marker = (Marker*)0;
	markersTreeWidget->clear();

	TimeLine* tl = m_sheet->get_timeline();
		
	foreach(Marker* marker, tl->get_markers()) {
		QString name = marker->get_description();
		QString pos = timeref_to_cd_including_hours(marker->get_when());

		QTreeWidgetItem* item = new QTreeWidgetItem(markersTreeWidget);
		item->setText(0, pos.simplified());
		item->setText(1, name);
		item->setData(0, Qt::UserRole, marker->get_id());
	}

	if (currentIndex >= markersTreeWidget->topLevelItemCount()) {
		currentIndex = markersTreeWidget->topLevelItemCount() - 1;
	}

	markersTreeWidget->setCurrentItem(markersTreeWidget->topLevelItem(currentIndex));
}

void MarkerDialog::item_changed(QTreeWidgetItem * current, QTreeWidgetItem * previous)
{
	if (!current) {
		m_marker = (Marker*)0;
		return;
	}

	m_marker = get_marker(current->data(0, Qt::UserRole).toLongLong());

	if (!m_marker) {
		return;
	}

	if (previous) {
		Marker *marker = get_marker(previous->data(0, Qt::UserRole).toLongLong());
		marker->set_when(cd_to_timeref_including_hours(lineEditPosition->text()));
		marker->set_description(lineEditTitle->text());
		marker->set_performer(lineEditPerformer->text());
		marker->set_composer(lineEditComposer->text());
		marker->set_sheetwriter(lineEditSheetwriter->text());
		marker->set_arranger(lineEditArranger->text());
		marker->set_message(lineEditMessage->text());
		marker->set_isrc(lineEditIsrc->text());
		marker->set_preemphasis(checkBoxPreEmph->isChecked());
		marker->set_copyprotect(checkBoxCopy->isChecked());
	}

	lineEditPosition->setText(timeref_to_cd_including_hours(m_marker->get_when()));
	lineEditTitle->setText(m_marker->get_description());
	lineEditPerformer->setText(m_marker->get_performer());
	lineEditComposer->setText(m_marker->get_composer());
	lineEditSheetwriter->setText(m_marker->get_sheetwriter());
	lineEditArranger->setText(m_marker->get_arranger());
	lineEditMessage->setText(m_marker->get_message());
	lineEditIsrc->setText(m_marker->get_isrc());
	checkBoxPreEmph->setChecked(m_marker->get_preemphasis());
	checkBoxCopy->setChecked(m_marker->get_copyprotect());
}

// update the entry in the tree widget in real time
void MarkerDialog::description_changed(const QString &s)
{
	QTreeWidgetItem* item = markersTreeWidget->currentItem();

	if (!item || !m_marker) {
		return;
	}

	item->setText(1, s);
	m_marker->set_description(s);
}

// update the entry in the tree widget in real time
void MarkerDialog::position_changed(const QString &s)
{
	QTreeWidgetItem* item = markersTreeWidget->currentItem();

	if (!item || !m_marker) {
		return;
	}

	item->setText(0, s);
	
// 	AAAH, wouldn't it be sooo fun to have un/redo when 
// 	editing the Markers from here ?
// 	But the realtime thing plays not nice, what about only
// 	calling this function when the user hits enter ?

// 	TimeRef newpos = cd_to_timeref(s);
// 	TimeRef oldpos = m_marker->get_when();
// 	QVariant newv, oldv;
// 	newv.setValue(newpos);
// 	oldv.setValue(oldpos);
// 	PCommand* command = new PCommand(m_marker, "set_when", oldv, newv, tr("Move Marker (from Marker Editor)"));
// 	Command::process_command(command);

	TimeRef location = cd_to_timeref(s);
	m_marker->set_when(location);
	markersTreeWidget->sortItems(0, Qt::AscendingOrder);
}

// find the marker based on it's id.
Marker * MarkerDialog::get_marker(qint64 id)
{
	TimeLine* tl = m_sheet->get_timeline();

	foreach(Marker* marker, tl->get_markers()) {
		if (marker->get_id() == id) {
			return marker;
		}
	}
	return 0;
}

void MarkerDialog::apply()
{
	accept();
}

void MarkerDialog::cancel()
{
	reject();
}

// One slot per widget, to avoid using QObject::sender() to determine the sender
void MarkerDialog::position_enter()
{
	next_item(lineEditPosition);
}

void MarkerDialog::title_enter()
{
	next_item(lineEditTitle);
}

void MarkerDialog::performer_enter()
{
	next_item(lineEditPerformer);
}

void MarkerDialog::composer_enter()
{
	next_item(lineEditComposer);
}

void MarkerDialog::arranger_enter()
{
	next_item(lineEditArranger);
}

void MarkerDialog::sheetwriter_enter()
{
	next_item(lineEditSheetwriter);
}

void MarkerDialog::message_enter()
{
	next_item(lineEditMessage);
}

void MarkerDialog::isrc_enter()
{
	next_item(lineEditIsrc);
}

// this handles the case when "enter" is pressed on a lineEdit. It sets the next item current,
// and selects the text in the lineEdit
void MarkerDialog::next_item(QLineEdit *ledit)
{
	QTreeWidgetItem *pitem = markersTreeWidget->currentItem();
	int max = markersTreeWidget->topLevelItemCount();
	int idx = markersTreeWidget->indexOfTopLevelItem(pitem);
	int nidx = 0;

	if (idx < max-1) {
		nidx = idx + 1;
	}

	QTreeWidgetItem *citem = markersTreeWidget->topLevelItem(nidx);
	markersTreeWidget->setCurrentItem(citem);
	ledit->setSelection(0, ledit->text().length());
}

void MarkerDialog::title_all()
{
	QString str = lineEditTitle->text();
	if (QMessageBox::question(this, tr("Set all Titles"), 
					tr("Do you really want to set all titles to\n\"")
					+str+"\"?", QMessageBox::Yes | QMessageBox::No, 
					QMessageBox::Yes) == QMessageBox::No)
	{
		return;
	}

	for (int i = 0; i < markersTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *it = markersTreeWidget->topLevelItem(i);
		Marker *m = get_marker(it->data(0, Qt::UserRole).toLongLong());
		m->set_description(str);
		it->setText(1, str);
	}
}

void MarkerDialog::performer_all()
{
	QString str = lineEditPerformer->text();
	if (QMessageBox::question(this, tr("Set all Performers"), 
					tr("Do you really want to set all performers to\n\"")
					+str+"\"?", QMessageBox::Yes | QMessageBox::No, 
					QMessageBox::Yes) == QMessageBox::No)
	{
		return;
	}

	for (int i = 0; i < markersTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *it = markersTreeWidget->topLevelItem(i);
		Marker *m = get_marker(it->data(0, Qt::UserRole).toLongLong());
		m->set_performer(str);
	}
}

void MarkerDialog::composer_all()
{
	QString str = lineEditComposer->text();
	if (QMessageBox::question(this, tr("Set all Composers"), 
					tr("Do you really want to set all composers to\n\"")
					+str+"\"?", QMessageBox::Yes | QMessageBox::No, 
					QMessageBox::Yes) == QMessageBox::No)
	{
		return;
	}

	for (int i = 0; i < markersTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *it = markersTreeWidget->topLevelItem(i);
		Marker *m = get_marker(it->data(0, Qt::UserRole).toLongLong());
		m->set_composer(str);
	}
}

void MarkerDialog::arranger_all()
{
	QString str = lineEditArranger->text();
	if (QMessageBox::question(this, tr("Set all Arrangers"), 
					tr("Do you really want to set all arrangers to\n\"")
					+str+"\"?", QMessageBox::Yes | QMessageBox::No, 
					QMessageBox::Yes) == QMessageBox::No)
	{
		return;
	}

	for (int i = 0; i < markersTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *it = markersTreeWidget->topLevelItem(i);
		Marker *m = get_marker(it->data(0, Qt::UserRole).toLongLong());
		m->set_arranger(str);
	}
}

void MarkerDialog::sheetwriter_all()
{
	QString str = lineEditSheetwriter->text();
	if (QMessageBox::question(this, tr("Set all Sheetwriters"), 
					tr("Do you really want to set all sheetwriters to\n\"")
					+str+"\"?", QMessageBox::Yes | QMessageBox::No, 
					QMessageBox::Yes) == QMessageBox::No)
	{
		return;
	}

	for (int i = 0; i < markersTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *it = markersTreeWidget->topLevelItem(i);
		Marker *m = get_marker(it->data(0, Qt::UserRole).toLongLong());
		m->set_sheetwriter(str);
	}
}

void MarkerDialog::message_all()
{
	QString str = lineEditMessage->text();
	if (QMessageBox::question(this, tr("Set all Messages"), 
					tr("Do you really want to set all messages to\n\"")
					+str+"\"?", QMessageBox::Yes | QMessageBox::No, 
					QMessageBox::Yes) == QMessageBox::No)
	{
		return;
	}

	for (int i = 0; i < markersTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *it = markersTreeWidget->topLevelItem(i);
		Marker *m = get_marker(it->data(0, Qt::UserRole).toLongLong());
		m->set_message(str);
	}
}

void MarkerDialog::copy_all()
{
	QString str = "off";

	if (checkBoxCopy->isChecked()) {
		str = "on";
	}

	if (QMessageBox::question(this, tr("Set all Copy Protection Flags"), 
					tr("Do you really want to set all copy protection flags to\n\"")
					+str+"\"?", QMessageBox::Yes | QMessageBox::No, 
					QMessageBox::Yes) == QMessageBox::No)
	{
		return;
	}

	for (int i = 0; i < markersTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *it = markersTreeWidget->topLevelItem(i);
		Marker *m = get_marker(it->data(0, Qt::UserRole).toLongLong());
		m->set_copyprotect(checkBoxCopy->isChecked());
	}
}

void MarkerDialog::pemph_all()
{
	QString str = "off";

	if (checkBoxPreEmph->isChecked()) {
		str = "on";
	}

	if (QMessageBox::question(this, tr("Set all Pre-Emphasis Flags"), 
					tr("Do you really want to set all pre-emphasis flags to\n\"")
					+str+"\"?", QMessageBox::Yes | QMessageBox::No, 
					QMessageBox::Yes) == QMessageBox::No)
	{
		return;
	}

	for (int i = 0; i < markersTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *it = markersTreeWidget->topLevelItem(i);
		Marker *m = get_marker(it->data(0, Qt::UserRole).toLongLong());
		m->set_preemphasis(checkBoxPreEmph->isChecked());
	}
}

void MarkerDialog::remove_marker()
{
	if (!m_marker) {
		return;
	}
	
	if (m_marker->get_type() == Marker::ENDMARKER) {
		info().information(tr("It's not possible to remove the endmarker!!"));
		return;
	}

	TimeLine* tl = m_sheet->get_timeline();
		
	AddRemove *ar = (AddRemove*) tl->remove_marker(m_marker);
	Command::process_command(ar);
	update_marker_treeview();
}

void MarkerDialog::export_toc()
{
	QString fn = QFileDialog::getSaveFileName (0, tr("Export Table of Contents"), m_project->get_root_dir(), tr("HTML File (*.html)"));

	// if aborted exit here
	if (fn.isEmpty()) {
		return;
	}

	QFile file(fn);

	// check if the selected file can be opened for writing
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		printf("Could not open file for writing.");
		return;
	}

	QTextStream out(&file);

	out << "<html>\n  <head>\n    <meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n  </head>\n\n  <body>\n";

	out << "    <h1>" << m_project->get_title() << "</h1>\n";
	out << "    <h2>" << m_project->get_description() << "</h2>\n";
	
	out << "    <hr>\n";
	out << "    <table>\n      <tr><th>Position (mm:ss:frames)</th><th>Title</th>\n";

	TimeLine* tl = m_sheet->get_timeline();
	foreach(Marker* marker, tl->get_markers()) {
		QString name = marker->get_description();
		QString pos = timeref_to_cd(marker->get_when());

		out << "      <tr><td>" << pos << "</td>\n        <td>" << name << "</td></tr>\n";
	}	

	QDateTime dt = QDateTime::currentDateTime();

	out << "    </table>\n  <hr>\n  " << dt.toString("MMM dd, yyyy, hh:mm") << "\n</body>\n</html>\n";
}


//eof
