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


struct TFunction {

	static bool smaller(const TFunction* left, const TFunction* right )
	{
		return left->sortorder < right->sortorder;
	}
	static bool greater(const TFunction* left, const TFunction* right )
	{
		return left->sortorder > right->sortorder;
	}

	TFunction() {
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

struct TShortcutKey
{
	TShortcutKey(int keyValue)
	{
		keyvalue = keyValue;
	}

	~TShortcutKey();

	QHash<QString, TFunction*> objects;

	int type;
	int keyvalue;
	int autorepeatInterval;
	int autorepeatStartDelay;
	bool isInstantaneous;
	QString keyString;
};


class TShortcutManager : public QObject
{
	Q_OBJECT
public:

	void addFunction(const QString& function, TFunction* data);
	TFunction* getFunctionshortcut(const QString& function) const;

	QList<TFunction* > getFunctionsFor(QObject* item);
	void getFunctionsForMetaobject(const QMetaObject* mo, QList<TFunction* >& list) const;
	TShortcutKey* getShortcutFor(const QString& key);

	void loadFunctions();
	static void makeShortcutKeyHumanReadable(QString& key);

private:
	QHash<QString, TFunction*>	m_functions;
	QHash<int, TShortcutKey*>	m_shortcutKeys;

	TShortcutManager();
	TShortcutManager(const TShortcutManager&) : QObject() {}

	friend TShortcutManager& tShortCutManager();
};

TShortcutManager& tShortCutManager();


#endif // TSHORTCUTMANAGER_H
