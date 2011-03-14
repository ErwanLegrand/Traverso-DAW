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

class CommandPlugin;

class TFunction {

public:
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
	QStringList getModifierKeys();
	QStringList getKeys() const;

	QStringList modes;
	QVariantList arguments;
	QList<int > modifierkeys;
	QString object;
	QString slotsignature;
	QString pluginname;
	QString commandName;
	QString submenu;
	QString description;
	bool useX;
	bool useY;
	int sortorder;
	int autorepeatInterval;
	int autorepeatStartDelay;

private:
	QStringList m_keys;

	friend class TShortcutManager;
};

class TShortcut
{
public:
	TShortcut(int keyValue)
	{
		m_keyValue = keyValue;
	}
	~TShortcut();

	int getKeyValue() const {return m_keyValue;}

	QList<TFunction*> getFunctionsForObject(const QString& objectName);

	int		autorepeatInterval;
	int		autorepeatStartDelay;

private:
	QHash<QString, TFunction*> objects;

	int		m_keyValue;

	friend class TShortcutManager;
};


class TShortcutManager : public QObject
{
	Q_OBJECT
public:

	void addFunction(TFunction* function);
	TFunction* getFunction(const QString& function) const;

	QList<TFunction* > getFunctionsFor(QObject* item);
	QList<TFunction* > getFunctionsForMetaobject(const QMetaObject* mo) const;
	TShortcut* getShortcut(const QString& key);
	TShortcut* getShortcut(int key);
	CommandPlugin* getCommandPlugin(const QString& pluginName);

	void loadFunctions();
	void saveFunction(TFunction* function);
	void loadShortcuts();
	static void makeShortcutKeyHumanReadable(QString& key);

private:
	QHash<QString, CommandPlugin*>	m_commandPlugins;
	QHash<QString, TFunction*>	m_functions;
	QHash<int, TShortcut*>		m_shortcuts;

	TShortcutManager();
	TShortcutManager(const TShortcutManager&) : QObject() {}

	friend TShortcutManager& tShortCutManager();
};

TShortcutManager& tShortCutManager();


#endif // TSHORTCUTMANAGER_H
