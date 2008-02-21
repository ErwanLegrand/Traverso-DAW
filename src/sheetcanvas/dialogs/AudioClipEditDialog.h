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

#ifndef AUDIOCLIP_EDIT_DIALOG_H
#define AUDIOCLIP_EDIT_DIALOG_H

#include "ui_AudioClipEditDialog.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDialog>

#include "defines.h"

class AudioClip;

class AudioClipEditDialog : public QDialog, protected Ui::AudioClipEditDialog
{
	Q_OBJECT

public:
	AudioClipEditDialog(AudioClip* clip, QWidget* parent);
	~AudioClipEditDialog() {}

private:
	AudioClip* m_clip;
	QDomNode m_origState;

	TimeRef qtime_to_timeref(const QTime& time);
	QTime timeref_to_qtime(const TimeRef& ref);
	bool locked;

private slots:
	void external_processing();
	void clip_state_changed();
	void save_changes();
	void cancel_changes();
	void clip_position_changed();
	void gain_spinbox_value_changed(double value);

	void fadein_length_changed();
	void fadein_edit_changed(const QTime& time);
	void fadein_mode_changed();
	void fadein_mode_edit_changed(int index);
	void fadein_bending_changed();
	void fadein_bending_edit_changed(double value);
	void fadein_strength_changed();
	void fadein_strength_edit_changed(double value);
	void fadein_linear();
	void fadein_default();

	void fadeout_edit_changed(const QTime& time);
	void fadeout_length_changed();
	void fadeout_mode_changed();
	void fadeout_mode_edit_changed(int index);
	void fadeout_bending_changed();
	void fadeout_bending_edit_changed(double value);
	void fadeout_strength_changed();
	void fadeout_strength_edit_changed(double value);
	void fadeout_linear();
	void fadeout_default();

	void clip_start_edit_changed(const QTime& time);
	void clip_length_edit_changed(const QTime& time);
	void update_clip_end();

	void fade_curve_added();
};

#endif
