/*
    Copyright (C) 2008 Remon Sijrier 
 
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

#ifndef EXPORT_FORMAT_OPTIONS_WIDGET_H
#define EXPORT_FORMAT_OPTIONS_WIDGET_H

#include "ui_ExportFormatOptionsWidget.h"

#include <QWidget>

class Project;
class Sheet;
struct ExportSpecification;

class ExportFormatOptionsWidget : public QWidget, protected Ui::ExportFormatOptionsWidget
{
	Q_OBJECT

public:
	ExportFormatOptionsWidget(QWidget* parent = 0);
	~ExportFormatOptionsWidget();
	
	void get_format_options(ExportSpecification* spec);


private slots:
	void audio_type_changed(int index);
	void mp3_method_changed(int index);
	void ogg_method_changed(int index);
};

#endif

//eof


 
