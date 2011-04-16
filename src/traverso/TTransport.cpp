/*
Copyright (C) 2011 Remon Sijrier

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

#include "TTransport.h"

#include "TCommand.h"
#include "TSession.h"
#include "Sheet.h"
#include "Project.h"
#include "TMainWindow.h"
#include "SheetWidget.h"
#include "PlayHeadMove.h"

TTransport::TTransport()
{
}

TCommand* TTransport::start_transport()
{
	if (m_session)
	{
		m_session->start_transport();
	}

	return 0;
}

TCommand * TTransport::set_recordable_and_start_transport()
{
	if (m_project) {
		Sheet* sheet = m_project->get_active_sheet();
		if (sheet) {
			return sheet->set_recordable_and_start_transport();
		}
	}

	return 0;
}

TCommand * TTransport::to_start()
{
	if (m_session)
	{
		m_session->set_transport_pos(TimeRef());
	}

	return 0;
}

TCommand* TTransport::to_end()
{
	if (m_session)
	{
		m_session->set_transport_pos(m_session->get_last_location());
	}

	return 0;
}

TCommand* TTransport::set_transport_position()
{
	SheetWidget* widget = TMainWindow::instance()->getCurrentSheetWidget();
	if (widget)
	{
		return new PlayHeadMove(widget->get_sheetview());

	}
	return 0;
}
