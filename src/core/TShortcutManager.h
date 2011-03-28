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
		m_inheritedFunction = 0;
		useX = false;
		useY = false;
		sortorder = 0;
		m_usesAutoRepeat = false;
		m_autorepeatInterval = 120;
		m_autorepeatStartDelay = 200;
	}

	QString getKeySequence();
	QString getSlotSignature() const;
	QString getDescription() const;
	QString getLongDescription() const;
	QList<int> getModifierKeys();
	QStringList getKeys() const;

	int getAutoRepeatInterval() const;
	int getAutoRepeatStartDelay() const;
	TFunction* getInheritedFunction() const {return m_inheritedFunction;}

	bool usesAutoRepeat() const {return m_usesAutoRepeat;}

	void setDescription(const QString& des);
	void setUsesAutoRepeat(bool b) {m_usesAutoRepeat = b;}

	QStringList modes;
	QVariantList arguments;
	QString object;
	QString pluginname;
	QString commandName;
	QString submenu;
	QString inherits;
	bool useX;
	bool useY;
	int sortorder;

private:
	QStringList	m_keys;
	QString		slotsignature;
	QString		m_description;
	TFunction*	m_inheritedFunction;
	QList<int >	m_modifierkeys;
	int		m_autorepeatInterval;
	int		m_autorepeatStartDelay;
	bool		m_usesAutoRepeat;

	void setInheritedFunction(TFunction* inherited);


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
	QList<TFunction*> getFunctions();

	int		autorepeatInterval;
	int		autorepeatStartDelay;

private:
	QMultiHash<QString, TFunction*> objects;
	int		m_keyValue;


	friend class TShortcutManager;
};


class TShortcutManager : public QObject
{
	Q_OBJECT
public:

	void addFunction(TFunction* function);
	TFunction* getFunction(const QString& function) const;

	QList<TFunction* > getFunctionsFor(const QString& className);
	TShortcut* getShortcut(const QString& key);
	TShortcut* getShortcut(int key);
	CommandPlugin* getCommandPlugin(const QString& pluginName);
	void modifyFunctionKeys(TFunction* function, QStringList keys);
	void add_translation(const QString& signature, const QString& translation);
	void add_meta_object(const QMetaObject* mo);
	QString get_translation_for(const QString& entry);
	QString createHtmlForMetaObects(QList<const QMetaObject*> metas, QObject* obj=0);
	QList<const QMetaObject*> get_metaobjects_for_class(const QString& className);
	QHash<QString, QList<const QMetaObject*> > get_meta_objects() const {return m_metaObjects;}
	QList<QString> getClassNames() const;
	bool classInherits(const QString& className, const QString &inherited);

	void loadFunctions();
	void saveFunction(TFunction* function);
	void loadShortcuts();
	static void makeShortcutKeyHumanReadable(QString& key);

	bool isCommandClass(const QString& className);

private:
	QHash<QString, CommandPlugin*>	m_commandPlugins;
	QHash<QString, TFunction*>	m_functions;
	QHash<int, TShortcut*>		m_shortcuts;
	QHash<QString, QString>		m_translations;
	QHash<QString, QList<const QMetaObject*> > m_metaObjects;

	TShortcutManager();
	TShortcutManager(const TShortcutManager&) : QObject() {}

	QList<TFunction* > functionsForMetaobject(const QMetaObject* mo) const;

	friend TShortcutManager& tShortCutManager();
};

TShortcutManager& tShortCutManager();


#endif // TSHORTCUTMANAGER_H
