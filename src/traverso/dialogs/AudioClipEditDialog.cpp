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


#include "AudioClipEditDialog.h"

#include <QWidget>
#include "ui_AudioClipEditWidget.h"

#include "AudioClip.h"
#include "Utils.h"
#include "defines.h"
#include "Mixer.h"
#include "Command.h"
#include "AudioClipExternalProcessing.h"

class AudioClipEditWidget : public QWidget, protected Ui::AudioClipEditWidget
{
	Q_OBJECT

public:
	AudioClipEditWidget(AudioClip* clip, QWidget* parent) : QWidget(parent), m_clip(clip)
	{
		setupUi(this);
		
		clip_state_changed();
		clip_start_value_changed(m_clip->get_track_start_frame() / m_clip->get_rate());
		
		connect(clip, SIGNAL(stateChanged()), this, SLOT(clip_state_changed()));
		connect(clip, SIGNAL(positionChanged(Snappable*)), this, SLOT(clip_position_changed()));
		
		connect(clipGainSpinBox, SIGNAL(valueChanged(double)), this, SLOT(gain_spinbox_value_changed(double)));
		connect(clipStartSpinBox, SIGNAL(valueChanged(double)), this, SLOT(clip_start_value_changed(double)));
		
		connect(externalProcessingButton, SIGNAL(clicked()), this, SLOT(external_processing()));
		connect(buttonBox, SIGNAL(accepted()), this, SLOT(save_changes()));
	}
	
	~AudioClipEditWidget() {}
	
private:
	AudioClip* m_clip;
	friend class AudioClipEditDialog;
	
private slots:
	void external_processing();
	void clip_state_changed();
	void save_changes();
	void clip_position_changed();
	void gain_spinbox_value_changed(double value);
	void clip_start_value_changed(double value);
};


AudioClipEditDialog::AudioClipEditDialog(AudioClip* clip, QWidget* parent)
{
	m_edit = new AudioClipEditWidget(clip, this);
	
	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->addWidget(m_edit);
	
	setLayout(mainLayout);
	
	connect(m_edit->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void AudioClipEditWidget::external_processing()
{
	parentWidget()->hide();
	Command::process_command(new AudioClipExternalProcessing(m_clip));
}

void AudioClipEditWidget::clip_state_changed()
{
	if (m_clip->get_name() != clipNameLineEdit->text()) {
		parentWidget()->setWindowTitle(m_clip->get_name());
		clipNameLineEdit->setText(m_clip->get_name());
	}
	
	clipGainSpinBox->setValue(coefficient_to_dB(m_clip->get_gain()));
}

void AudioClipEditWidget::save_changes()
{
	parentWidget()->hide();
	QString name = clipNameLineEdit->text();
	if (!name.isEmpty()) {
		m_clip->set_name(name);
	} else {
		clipNameLineEdit->setText(m_clip->get_name());
	}		
}

void AudioClipEditWidget::gain_spinbox_value_changed(double value)
{
	float gain = dB_to_scale_factor(value);
	m_clip->set_gain(gain);
}

void AudioClipEditWidget::clip_start_value_changed(double value)
{
	clipStartSpinBox->setValue(value);
	
	nframes_t startframe = (nframes_t)(value * m_clip->get_rate());
	
	if (m_clip->get_track_start_frame() != startframe) {
		m_clip->set_track_start_frame(startframe);
	}
}

void AudioClipEditWidget::clip_position_changed()
{
	clip_start_value_changed((double)m_clip->get_track_start_frame() / m_clip->get_rate());
}

#include "AudioClipEditDialog.moc"
