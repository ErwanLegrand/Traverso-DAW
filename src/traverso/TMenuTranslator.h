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

#ifndef TMENUTRANSLATOR_H
#define TMENUTRANSLATOR_H

#include <QObject>
#include <QHash>

struct TFunction;

class TMenuTranslator : public QObject
{
        Q_OBJECT
public:
        TMenuTranslator();
        static TMenuTranslator* instance();

        void add_entry(const QString& signature, const QString& translation);
        void add_meta_object(const QMetaObject* mo);
        QString get_translation_for(const QString& entry);
	QString createHtmlForMetaObects(QList<const QMetaObject*> metas, QObject* obj=0);
	QList<const QMetaObject*> get_metaobjects_for_class(const QString& className);
        QHash<QString, QList<const QMetaObject*> > get_meta_objects() const {return m_objects;}


private:
        QHash<QString, QString> m_dict;
        static TMenuTranslator* m_instance;
        QHash<QString, QList<const QMetaObject*> > m_objects;

};

#endif // TMENUTRANSLATOR_H
