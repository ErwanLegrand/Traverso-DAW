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
 
    $Id: Export.cpp,v 1.16 2008/01/21 16:22:13 r_sijrier Exp $
*/

#include "Export.h"
#include "Project.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ExportThread::ExportThread(Project* project)
	: QThread(project)
{
        m_project = project;
}

void ExportThread::set_specification(ExportSpecification * spec)
{
	m_spec  = spec;
	m_spec->thread = this;
}


void ExportThread::run( )
{
        m_project->start_export(m_spec);
}

ExportSpecification::ExportSpecification()
{
	sample_rate = -1;
	
	src_quality = SRC_SINC_MEDIUM_QUALITY;
	channels = -1;
	startLocation = qint64(-1);
	endLocation = qint64(-1);
	dither_type = GDitherTri;
	
	dataF = 0;
	blocksize = -1;
	data_width = -1;
	
	totalTime = qint64(-1);
	pos = qint64(-1);
	
	allSheets = false;
	stop = false;
	breakout = false;
	isRecording = -1;
	exportdir = "";
	basename = "";
	name = "";
	writeToc = false;
	normalize = false;
	renderpass = WRITE_TO_HARDDISK;
	normvalue = 1.0;
	peakvalue = 0.0;
	isCdExport = false;
}

int ExportSpecification::is_valid()
{

	if (sample_rate == -1) {
		printf("ExportSpecification: No samplerate configured!\n");
		return -1;
	}
	
	if (channels == -1) {
		printf("ExportSpecification: No channels configured!\n");
		return -1;
	}
	
	if (startLocation == qint64(-1)) {
		printf("ExportSpecification: No start frame configured!\n");
		return -1;
	}

	if (endLocation == qint64(-1)) {
		printf("ExportSpecification: No end frame configured!\n");
		return -1;
	}

	if (! dataF ) {
		printf("ExportSpecification: No mixdown buffer created!!\n");
		return -1;
	}

	if (blocksize == -1) {
		printf("ExportSpecification: No blocksize configured!\n");
		return -1;
	}

	if (data_width == -1) {
		printf("ExportSpecification: No data width configured!\n");
		return -1;
	}

	if (totalTime == qint64(-1)) {
		printf("ExportSpecification: No total frames configured!\n");
		return -1;
	}

	if (isRecording == -1) {
		printf("ExportSpecification: No isRecording configured!\n");
		return -1;
	}

	if (pos == qint64(-1) && isRecording == 0) {
		printf("ExportSpecification: No position configured!\n");
		return -1;
	}
	
	if (exportdir.isEmpty()) {
		printf("ExportSpecification: No export dir configured!\n");
		return -1;
	}

	if (name.isEmpty()) {
		printf("ExportSpecification: No name configured!\n");
		return -1;
	}

	return 1;
}
