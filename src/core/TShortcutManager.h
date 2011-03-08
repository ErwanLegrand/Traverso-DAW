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

#ifndef TSHORTCUTMANAGER_H
#define TSHORTCUTMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QStringList>

struct TShortcutData {

	static bool smaller(const TShortcutData* left, const TShortcutData* right )
	{
		return left->sortorder < right->sortorder;
	}
	static bool greater(const TShortcutData* left, const TShortcutData* right )
	{
		return left->sortorder > right->sortorder;
	}

	TShortcutData() {
		useX = false;
		useY = false;
		sortorder = 0;
		autorepeatInterval = 120;
		autorepeatStartDelay = 200;
	}

	QString getKeySequence();

	QStringList modes;
	QVariantList arguments;
	QList<int > modifierkeys;
	QString classname;
	QString slotsignature;
	QString pluginname;
	QString commandname;
	QString submenu;
	QString description;
	QString	keyString;
	bool useX;
	bool useY;
	int sortorder;
	int autorepeatInterval;
	int autorepeatStartDelay;
};

class TShortcutManager : public QObject
{
	Q_OBJECT
public:

	void addFunctionShortcut(const QString& function, TShortcutData* data);
	TShortcutData* getFunctionshortcut(const QString& function) const;

	QList<TShortcutData* > getShortcutDataFor(QObject* item);
	void getShortcutDataForMetaobject(const QMetaObject* mo, QList<TShortcutData* >& list) const;

	void loadDefaultShortcuts();
	static void makeShortcutKeyHumanReadable(QString& key);

private:
	QHash<QString, TShortcutData*>	m_dict;

	TShortcutManager();
	TShortcutManager(const TShortcutManager&) : QObject() {}

	friend TShortcutManager& tShortCutManager();
};

TShortcutManager& tShortCutManager();


#endif // TSHORTCUTMANAGER_H
