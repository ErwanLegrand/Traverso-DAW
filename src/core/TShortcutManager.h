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
class TCommand;

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
		m_autorepeatInterval = -1;
		m_autorepeatStartDelay = -1;
		m_usesInheritedBase = false;
	}

	QString getKeySequence();
	QString getModifierSequence(bool fromInheritedBase=true);
	QString getSlotSignature() const;
	QString getDescription() const;
	QString getInheritedBase() const {return m_inheritedBase;}
	QString getLongDescription() const;
	QList<int> getModifierKeys(bool fromInheritedBase=true);
	QStringList getKeys(bool fromInheritedBase=true) const;

	int getAutoRepeatInterval() const;
	int getAutoRepeatStartDelay() const;
	TFunction* getInheritedFunction() const {return m_inheritedFunction;}

	bool usesAutoRepeat() const {return m_usesAutoRepeat;}
	bool usesInheritedBase() const {return m_usesInheritedBase;}

	void setDescription(const QString& des);
	void setInheritedBase(const QString& base);
	void setUsesInheritedbase(bool b) {m_usesInheritedBase = b;}
	void setUsesAutoRepeat(bool b) {m_usesAutoRepeat = b;}
	void setAutoRepeatInterval(int interval) {m_autorepeatInterval = interval;}
	void setAutoRepeatStartDelay(int delay) {m_autorepeatStartDelay = delay;}

	QStringList modes;
	QVariantList arguments;
	QString object;
	QString pluginname;
	QString commandName;
	QString submenu;
	bool useX;
	bool useY;
	int sortorder;

private:
	QStringList	m_keys;
	QString		slotsignature;
	QString		m_description;
	QString		m_inheritedBase;
	TFunction*	m_inheritedFunction;
	QList<int >	m_modifierkeys;
	int		m_autorepeatInterval;
	int		m_autorepeatStartDelay;
	bool		m_usesAutoRepeat;
	bool		m_usesInheritedBase;

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
	~ TShortcut() {}

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

	QList<TFunction* > getFunctionsFor(QString className);
	TShortcut* getShortcut(const QString& key);
	TShortcut* getShortcut(int key);
	CommandPlugin* getCommandPlugin(const QString& pluginName);
	void modifyFunctionKeys(TFunction* function, QStringList keys, QStringList modifiers);
	void modifyFunctionInheritedBase(TFunction* function, bool usesInheritedBase);
	void add_translation(const QString& signature, const QString& translation);
	void add_meta_object(const QMetaObject* mo);
	void registerItemClass(const QString& item, const QString& className);
	QString get_translation_for(const QString& entry);
	QString createHtmlForClass(const QString& className, QObject* obj=0);
	QHash<QString, QList<const QMetaObject*> > get_meta_objects() const {return m_metaObjects;}
	QList<QString> getClassNames() const;
	QString getClassForObject(const QString& object) const;
	bool classInherits(const QString& className, const QString &inherited);

	void loadFunctions();
	void saveFunction(TFunction* function);
	void loadShortcuts();
	void restoreDefaultFor(TFunction* function);
	void restoreDefaults();
	static void makeShortcutKeyHumanReadable(QString& key);

	bool isCommandClass(const QString& className);

private:
	QHash<QString, CommandPlugin*>	m_commandPlugins;
	QHash<QString, TFunction*>	m_functions;
	QHash<int, TShortcut*>		m_shortcuts;
	QHash<QString, QString>		m_translations;
	QHash<QString, QList<const QMetaObject*> > m_metaObjects;
	QHash<QString, QStringList>	m_classes;

	TShortcutManager();
	TShortcutManager(const TShortcutManager&) : QObject() {}

	friend TShortcutManager& tShortCutManager();

public slots:
	TCommand* export_keymap();
	TCommand* get_keymap(QString &);

signals:
	void functionKeysChanged();
};

TShortcutManager& tShortCutManager();


#endif // TSHORTCUTMANAGER_H
