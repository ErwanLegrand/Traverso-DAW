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

$Id: SongManagerWidget.cpp,v 1.3 2006/05/08 20:05:27 r_sijrier Exp $
*/

#include "SongManagerWidget.h"
#include "ui_SongManagerWidget.h"

#include "libtraversocore.h"
#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

SongManagerWidget::SongManagerWidget( QWidget * parent )
		: QWidget(parent)
{
	setupUi(this);

	treeSongWidget->setColumnCount(6);
	treeSongWidget->header()->resizeSection(0, 200);
	treeSongWidget->header()->resizeSection(1, 60);
	treeSongWidget->header()->resizeSection(2, 120);
	treeSongWidget->header()->resizeSection(4, 80);
	QStringList stringList;
	stringList << "Song Name" << "Tracks" << "Length h:m:s,fr" << "Rate/BitDepth" << "Status" << "Size" ;
	treeSongWidget->setHeaderLabels(stringList);
	update_song_list();

	connect(treeSongWidget, SIGNAL(itemClicked ( QTreeWidgetItem* , int  )), this, SLOT(songitem_clicked(QTreeWidgetItem*, int )));

}

SongManagerWidget::~SongManagerWidget()
{}


void SongManagerWidget::update_song_list( )
{
	if (!pm().get_project()) {
		return;
	}

	int numSongs = pm().get_project()->get_num_songs();

	treeSongWidget->clear();
	for (int i=1; i <= numSongs; i++) {
		Song* s = pm().get_project()->get_song(i);
		if (!s)
			continue;

		QString songNr;
		songNr.setNum(i);
		if( (i + 1 ) < 10)
			songNr.prepend("0");
		QString songName = "Song " + songNr + " - ";
		songName.append(s->get_title());
		QString numberOfTracks;
		numberOfTracks.setNum(s->get_numtracks());

		QString songLength = frame_to_smpte(s->get_last_frame(), s->get_rate());

		//QString songLength; songLength.setNum( spos );
		QString songRate;
		songRate.setNum(s->get_rate());
		QString songBitDepth;
		songBitDepth.setNum(s->get_bitdepth());
		QString songRateBitdepth = songRate + " / " + songBitDepth;
		QString songStatus = s->is_changed()?"UnSaved":"Saved";
		QString songSpaceAllocated = "Unknown";
		/* for later:
		QString sLength; sLength.setNum((double)a->file->totalBlocks,'f',0);
		QString sSize; sSize.setNum((double)a->file->fileSize,'f',0);
		*/

		QTreeWidgetItem* item = new QTreeWidgetItem(treeSongWidget);
		item->setTextAlignment(1, Qt::AlignHCenter);
		item->setTextAlignment(2, Qt::AlignHCenter);
		item->setTextAlignment(3, Qt::AlignHCenter);
		item->setTextAlignment(4, Qt::AlignHCenter);
		item->setTextAlignment(5, Qt::AlignHCenter);
		item->setText(0, songName);
		item->setText(1, numberOfTracks);
		item->setText(2, songLength);
		item->setText(3, songRateBitdepth);
		item->setText(4, songStatus);
		item->setText(5, songSpaceAllocated);
	}
}

void SongManagerWidget::songitem_clicked( QTreeWidgetItem* item, int)
{
	if (!item)
		return;

	Song* s;
	QString title;
	QString artists;
	title = item->text(0);
	int length = title.length();
	length -= 10;

	//find the selected Song
	QString t = title.mid(5,2);
	bool b;
	int nr = t.toInt(&b, 10);
	s = pm().get_project()->get_song( (nr) );

	if (s) {
		artists = s->get_artists();
		title = title.right(length);
		selectedSongName->setText(title);
	}
}

void SongManagerWidget::on_saveSongButton_clicked( )
{
	if(!pm().get_project()) {
		return;
	}
	
	QTreeWidgetItem* item = treeSongWidget->currentItem();
	QString name = "";
	
	if (item) {
		name = item->text(0);
		//find the selected Song
		QString t = name.mid(5,2);
		bool b;
		int nr = t.toInt(&b, 10);
		Song* s = pm().get_project()->get_song(nr);
		
		if (s) {
			s->set_title( selectedSongName->text() );
		}
		
	}

	update_song_list();
}

void SongManagerWidget::on_deleteSongButton_clicked( )
{
	QTreeWidgetItem* item = treeSongWidget->currentItem();
	int nr;
	QString title;
	title = item->text(0);
	int length = title.length();
	length -= 10;
	QString t = title.mid(5,2);
	bool b;
	nr = t.toInt(&b, 10);
	pm().get_project()->remove_song( nr );
	update_song_list();
}

void SongManagerWidget::on_createSongButton_clicked( )
{
	Project* proj = pm().get_project();
	if (proj) {
		proj->add_song();
		PMESG("song added");
		update_song_list();
	}
}

void SongManagerWidget::on_importFileButton_clicked( )
{
	/*	if (importFileButton->text() == "Import Now")
			{
			pm().get_project()->import_audio(importFileName->text());
			importFileButton->setText("Cancel Import!");
			}
		else
			{
			pm().get_project()->cancel_import_audio();
			importFileButton->setText("Import Now");
			importProgressBar->reset();
			}*/
}

void SongManagerWidget::on_chooseFileButton_clicked( )
{
	QString fileName = QFileDialog::getOpenFileName(this,
			tr("QFileDialog::getOpenFileName()"),
			importFileName->text(),
			tr("All files (*);;Audio files (*.wav)"));
	if (!fileName.isEmpty())
		importFileName->setText(fileName);
}


//eof

