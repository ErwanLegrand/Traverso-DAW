/*
Copyright (C) 2010 Remon Sijrier

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

#include "TContextHelpWidget.h"

#include <QMenu>
#include <QLayout>
#include <QComboBox>

#include "TConfig.h"
#include "ContextPointer.h"
#include "ContextItem.h"
#include "TInputEventDispatcher.h"
#include "TCommand.h"
#include "PCommand.h"
#include "TMainWindow.h"
#include "TShortcutManager.h"
#include "Themer.h"

#include "Debugger.h"

TContextHelpWidget::TContextHelpWidget(QWidget* parent)
        : QWidget(parent)
{
	setObjectName("ShortcutsHelpWidget");

        m_comboBox = new QComboBox(parent);
        m_textEdit = new QTextEdit(parent);
        m_textEdit->setTextInteractionFlags(Qt::NoTextInteraction);

        QHBoxLayout* comboLayout = new QHBoxLayout;
        QVBoxLayout* mainLayout = new QVBoxLayout;
        comboLayout->addWidget(m_comboBox);

        mainLayout->addLayout(comboLayout);
        mainLayout->addWidget(m_textEdit);
        setLayout(mainLayout);

	m_comboBox->addItem(tr("Shortcuts Explained"));
	m_comboBox->addItem(tr("Current Context"));

	m_helpIntroduction = (
		"<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\"></head><body>"
		"To show the shortcuts for the item below the cursor, use the right mouse button, or "
		"select <u>%1</u> in the dropdown menu to auto update this help"
		"<p>"
		"<b>Edit Shortcuts</b>"
		"<p>"
		"Some shortcuts when keeping pressed support pressing additional keys. "
		"Available modes are: <br /><br />Keep the key pressed, and make your changes "
		"by moving the mouse, using arrow keys or any of the keys supported by the Edit Shortcut and finish "
		"it by releasing the Shortcut key or, <br /><br />Set in "
		"the keyboard preferences to finish an Edit Shortcut by pressing Enter (or the same shortcut key)."
		"<br /><br />(When started from the right click menu, finish by using Enter is the only option)"
		"<p>"
		"<b>Examples</b>"
		"<p>"
		"<table>"
		"<tr><td width=20 align=center><b>V</b></td><td>Press V and move mouse left/right or use arrow keys: Set Play Position</td></tr>"
		"<tr><td width=20 align=center><b>Z</b></td><td>Press Z and move mouse left/right or use arrow keys: Zoom In/Out</td></tr>"
		"<tr><td width=20 align=center><b>I</b></td><td>Hover mouse over an %2, type I (from Import) and select the file(s) to import</td></tr>"
		"<tr><td width=20 align=center><b>G</b></td><td>Hover mouse over an %3, press G (from Gain). Move mouse or use arrow keys to change Gain</td></tr>"
		"<tr><td width=20 align=center><b>D</b></td><td>Hover mouse over an %3, press D (from Drag, left mouse button works too) and move the mouse or use arrow keys to move the %3</td></tr>"
		"</table>"
		"<p>"
		"<b>Advice</b>"
		"<p>"
		"Place the left hand in the typing position, most of the used keys are there, so you can use the right hand to move the cursor with the mouse or the arrow keys"
		"<p>"
		"You can of course configure the shortcuts to your liking in the shortcut configuration dialog"
		"</body></html>"
	);

	m_helpIntroduction = m_helpIntroduction.arg(tr("Current Context")).arg(tShortCutManager().get_translation_for("AudioTrack")).arg(tShortCutManager().get_translation_for("AudioClip"));


	QMap<QString, QString> classNamesMap;
	QMap<QString, QString> commandClassNamesMap;

	foreach(QString className, tShortCutManager().getClassNames()) {
		if (tShortCutManager().isCommandClass(className))
		{
			commandClassNamesMap.insert(tShortCutManager().get_translation_for(className), className);
		}
		else
		{
			classNamesMap.insert(tShortCutManager().get_translation_for(className), className);
		}
	}

	foreach(QString className, classNamesMap.values()) {
		m_comboBox->addItem(classNamesMap.key(className), className);
	}

	foreach(QString className, commandClassNamesMap)
	{
		m_comboBox->addItem(commandClassNamesMap.key(className) + " " + tr("(Edit Function)"), className);
	}

	int index = config().get_property("ShortcutsHelp", "DropDownIndex", 0).toInt();
        m_comboBox->setCurrentIndex(index);
	combobox_activated(0);

        connect(&cpointer(), SIGNAL(contextChanged()), this, SLOT(context_changed()));
        connect(&ied(), SIGNAL(jogStarted()), this, SLOT(jog_started()));
        connect(m_comboBox, SIGNAL(activated(int)), this, SLOT(combobox_activated(int)));
	connect(&tShortCutManager(), SIGNAL(functionKeysChanged()), this, SLOT(function_keys_changed()));
}

TContextHelpWidget::~TContextHelpWidget()
{
	config().set_property("ShortcutsHelp", "DropDownIndex", m_comboBox->currentIndex());
}

void TContextHelpWidget::context_changed()
{
        if (parentWidget()->isHidden()) {
                return;
        }

        if (m_comboBox->currentIndex() != 1) {
                return;
        }

        QList<ContextItem*> items = cpointer().get_active_context_items();

        if (items.size()) {
		QString newClassName = items.first()->metaObject()->className();
		if (m_currentClassName == newClassName)
		{
			return;
		}
		m_textEdit->setHtml(create_html_for_object(items.first()));
		m_currentClassName = newClassName;
        }
}

void TContextHelpWidget::jog_started()
{
        if (m_comboBox->currentIndex() != 1) {
                return;
        }

        TCommand* hold = ied().get_holding_command();
        if (hold) {
                m_textEdit->setHtml(create_html_for_object(hold));
		m_currentClassName = hold->metaObject()->className();
        }
}

QString TContextHelpWidget::create_html_for_object(QObject *obj)
{
        const QMetaObject* mo = obj->metaObject();

        if (m_help.contains(mo->className())) {
                return m_help.value(mo->className());
        }

	QString html = tShortCutManager().createHtmlForClass(mo->className(), obj);

        m_help.insert(mo->className(), html);

        return html;
}

void TContextHelpWidget::combobox_activated(int index)
{
        if (index == 0) {
                m_textEdit->setHtml(m_helpIntroduction);
                return;
        }

        QString className = m_comboBox->itemData(index).toString();

        if (m_help.contains(className)) {
                m_textEdit->setHtml(m_help.value(className));
                return;
        }
	else
	{
		QString html = tShortCutManager().createHtmlForClass(className);
		m_help.insert(className, html);
		m_textEdit->setHtml(html);
	}

	m_currentClassName = className;
}

void TContextHelpWidget::function_keys_changed()
{
	m_help.clear();
	if (m_currentClassName.isEmpty())
	{
		m_textEdit->setHtml("");
		return;
	}
	QString html = tShortCutManager().createHtmlForClass(m_currentClassName.remove("View"));
	m_help.insert(m_currentClassName, html);
	m_textEdit->setHtml(html);
}
