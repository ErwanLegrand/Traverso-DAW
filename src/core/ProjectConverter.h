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

#ifndef PROJECT_CONVERTER_H
#define PROJECT_CONVERTER_H

#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QHash>

class AudioFileMerger;
class ReadSource;

class ProjectConverter : public QObject
{
	Q_OBJECT
			
public:
	ProjectConverter();
	
	void set_project(const QString& rootdir, const QString& name);
	int start();
	QString get_conversion_description();

private:
	QHash<qint64, ReadSource*> m_readsources;
	AudioFileMerger* m_merger;
	int m_filesToMerge;
	int m_filesMerged;
	QDomDocument m_document;
	QString m_rootdir;
	QString m_projectname;
	int m_projectfileversion;
	
	int save_converted_document();
	int start_conversion_from_version_2_to_3();
	
private slots:
	void conversion_finished();
	void file_merge_started(QString file);
	void file_merge_finished(QString file);
	void finish_2_3_conversion();

signals:
	void progress(int);
	void actionProgressInfo(QString);
	void fileMergeStarted(QString file);
	void fileMergeFinished(QString file);
	void conversionFinished();
	void message(QString);
};

#endif
