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

#include "AudioClipExternalProcessing.h"

#include "ExternalProcessingDialog.h"

#include <AudioClip.h>
#include <AudioClipView.h>
#include <AudioTrack.h>
#include <InputEngine.h>
#include <ReadSource.h>
#include <ProjectManager.h>
#include <Project.h>
#include <ResourcesManager.h>
#include <Utils.h>
#include "Interface.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"




AudioClipExternalProcessing::AudioClipExternalProcessing(AudioClip* clip)
	: Command(clip, tr("Clip: External Processing"))
{
	m_clip = clip;
	m_resultingclip = 0;
	m_track = m_clip->get_track();
}


AudioClipExternalProcessing::~AudioClipExternalProcessing()
{}


int AudioClipExternalProcessing::prepare_actions()
{
	PENTER;
	ExternalProcessingDialog epdialog(Interface::instance(), this);
	
	epdialog.exec();
	
	if (! m_resultingclip) {
		return -1;
	}
	
	return 1;
}


int AudioClipExternalProcessing::do_action()
{
	PENTER;
	Command::process_command(m_track->remove_clip(m_clip, false));
	Command::process_command(m_track->add_clip(m_resultingclip, false));
	
	return 1;
}

int AudioClipExternalProcessing::undo_action()
{
	PENTER;
	Command::process_command(m_track->remove_clip(m_resultingclip, false));
	Command::process_command(m_track->add_clip(m_clip, false));
	return 1;
}



