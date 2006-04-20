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
 
    $Id: AudioSourcesManagerWidget.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include "AudioSourcesManagerWidget.h"
#include "ui_AudioSourcesManagerWidget.h"

#include "libtraversocore.h"
#include "AudioSource.h"

#include <QMessageBox>
#include <QFile>

#include "AudioSourcesManagerWidget.h"
#include "AudioSourcesManagerWidget.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



AudioSourcesManagerWidget::AudioSourcesManagerWidget( QWidget * parent )
                : QWidget(parent)
{
        setupUi(this);
        treeAudioSourcesWidget->setColumnCount(7);
        QStringList stringList;
        stringList << "Name" << "Type" << "Length" << "Size" << "Clips" << "Rate/BitDepth" << "Channels";
        treeAudioSourcesWidget->setHeaderLabels(stringList);
}

AudioSourcesManagerWidget::~AudioSourcesManagerWidget()
{}


void AudioSourcesManagerWidget::update_audio_sources_list()
{
        PENTER;
        if (!pm().get_project()) {
                return;
        }
        Song* s = pm().get_project()->get_current_song();
        if (!s) {
                return;
        }
        /*	AudioSourcesList* asl= s->get_audiosources_list();
        	int tot = asl->get_total_sources();
        	treeAudioSourcesWidget->clear();
        	for (int i=0; i<tot; i++)
        		{
        		AudioSource* a = asl->get_source_for_index(i);
        		if (!a) continue;
        		QString sFile = a->get_filename();
         
        		int ln = sFile.length();
        		QString pTitle = pm().get_project()->get_title();
        		int skip = sFile.indexOf(pTitle);
        		ln -= (skip + pTitle.length());
        		sFile = sFile.right(ln);
         
        		QString sType;
        		if (sFile.indexOf("wav")>=0)
        			sType = "WAV";
        		else
        			sType = "PRAF";
        		QString sRate; sRate.setNum(a->get_rate());
        		QString sBitDepth; sBitDepth.setNum(16);
        		QString rateBitDepth = sRate + " / " + sBitDepth;
        		QString sChannels = (a->get_channel_count() == 1 ? "MONO" : "STEREO");
        		QString sLength; sLength.setNum((double)a->get_nframes(),'f',0);
        		QString sSize; sSize.setNum((double)a->get_nframes(),'f',0);
        		QString sClips; sClips.setNum(a->get_clips_count());
         
        		QTreeWidgetItem* item = new QTreeWidgetItem(treeAudioSourcesWidget);
        		item->setText(0, sFile);
        		item->setText(1, sType);
        		item->setText(2, sLength);
        		item->setText(3, sSize);
        		item->setText(4, sClips);
        		item->setText(5, rateBitDepth);
        		item->setText(6, sChannels);
        		}*/
}

void AudioSourcesManagerWidget::on_removeUnusedSourcesButton_clicked( )
{
        if(!pm().get_project()) {
                return;
        }

        int numberRemovedSources = 0;
        int numSongs = pm().get_project()->get_num_songs();
        /*	for (int i=0; i<numSongs; i++)
        		{
        		Song* s = pm().get_project()->get_song(i);
        		AudioSourcesList* asl= s->get_audiosources_list();
        		int tot = asl->get_total_sources();
        		for (int i=tot-1; i>=0; --i)
        			{
        			AudioSource* a = asl->get_source_for_index(i);
        			if (!a)		//No audio source for this index, strange....
        				{
        				PWARN("No audioSource for this index in AudioSourcesList!");
        				continue;
        				}
        			QString aName = a->get_filename();
        			if (s->get_clips_count_for_audio(a) == 0)
        				{
        				if (asl->remove(a) < 0)
        					PWARN("Could not remove audioSource from AudioSourcesList!");
        				QFile file(aName);
        				if (!file.remove())
        					{
        					PERROR("failed to remove file %s\n", aName.toAscii().data());
        					}
        				QString peakFile = aName + ".peak" ;
        				file.setFileName(peakFile);
        				if (!file.remove())
        					{
        					PERROR("failed to remove file %s\n", peakFile.toAscii().data());
        					}
        				delete a;
        				numberRemovedSources++;
        				}
        			}
        		}
        	if (numberRemovedSources == 0)
        		{
        		return;
        		}
        	update_audio_sources_list();
        	QString nrs; nrs.setNum(numberRemovedSources);
        	QMessageBox::information(this,
        			"Traverso - Information",
        			"Removed " +  nrs + " unused Sources!",
        			"Ok");
        */
}

void AudioSourcesManagerWidget::on_removeSourcesButton_clicked( )
{
        QTreeWidgetItem* item = treeAudioSourcesWidget->currentItem();
        if (item) {
                QString s = item->text(0);
                int r;
                if ( (r = (pm().delete_source(s)) > 0))
                        PMESG("File %s removed succesfully", s.toAscii().data());
        }
}

void AudioSourcesManagerWidget::on_removeAllSourcesButton_clicked( )
{
        QTreeWidgetItem* item;
        switch( QMessageBox::warning( this, "Traverso - Warning!",
                                      "This action irreversibly deletes ALL current Song \n"
                                      "          Audio Sources from your hard disk. \n"
                                      "                      Are you sure ?",
                                      "&Yes", "&No", 0, 1 ) ) {
        case 0: //Yes clicked or Alt+Y pressed or Enter pressed.
                //Continue executing
                break;
        case 1: // No clicked or Alt+N pressed
                //Pfewh, that was a close one :-)
                return;
        }

        while((item = treeAudioSourcesWidget->takeTopLevelItem(0))) {
                pm().delete_source(item->text(0));
        }
}



//oef

