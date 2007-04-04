/*
Copyright (C) 2005-2006 Remon Sijrier

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

$Id: FileHelpers.cpp,v 1.7 2007/04/04 00:46:38 r_sijrier Exp $
*/

#include "FileHelpers.h"

#include <sys/stat.h>
#include "Config.h"
#include <QDir>
#include <Utils.h>

#include "Debugger.h"

// delete file/dir pName after prepending $HOME/traversoprojects/ to it
//
// if it is a directory, calls itself recursively  on any file/dir in the directory
// before removing the directory
int FileHelper::remove_recursively(const QString& pName)
{
	QString name = config().get_property("Project", "directory", "/directory/unknown").toString();
	name += "/" + pName;

	QFileInfo fileInfo(name);

	if (!fileInfo.exists()) {
		PERROR("File does not exist! %s", QS_C(name));
		return -1;
	}

	if (!fileInfo.isWritable()) {
		PERROR("failed to remove %s: you don't have write access to it\n", name.toAscii().data());
		return -1;
	}

	if(fileInfo.isFile()) {
		QFile file(name);
		if (!file.remove()) {
			PERROR("failed to remove file %s\n", name.toAscii().data());
			return -1;
		}
		return 1;
	} else if(fileInfo.isDir()) {
		QDir dir(name);
		QFileInfoList list = dir.entryInfoList();
		QFileInfo fi;

		for (int i = 0; i < list.size(); ++i) {
			fi = list.at(i);
			if ((fi.fileName() != ".") && (fi.fileName() != "..")) {
				QString nextFileName = pName + "/" + fi.fileName();
				if (remove_recursively(nextFileName) < 0) {
					PERROR("failed to remove directory %s\n", nextFileName.toAscii().data());
					return -1;
				}
			}
		}

		if (!dir.rmdir(name)) {
			PERROR("failed to remove directory %s\n", name.toAscii().data());
			return -1;
		}

		return 1;
	}

	return 1;
}


int FileHelper::copy_recursively(const QString& pNameFrom, const QString& pNameTo)
{
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
	QString nameFrom = config().get_property("Project", "directory", "/directory/unknown").toString();
	QString nameTo(nameFrom);

	nameFrom += pNameFrom;
	nameTo += pNameTo;

	QFileInfo fileFromInfo(nameFrom);
	QFileInfo fileToInfo(nameTo);

	if (!fileFromInfo.exists()) {
		PERROR("File or directory %s doesn't exist\n", pNameFrom.toAscii().data());
		return -1;
	}
	if (fileToInfo.exists()) {
		PERROR("File or directory %s already exists", pNameTo.toAscii().data());
		return -1;
	}

	if(fileFromInfo.isFile()) {
		QFile fileFrom(nameFrom);
		if (!fileFrom.open(QIODevice::ReadOnly)) {
			PERROR("failed to open file %s for reading\n", nameFrom.toAscii().data());
			return -1;
		}

		QFile fileTo(nameTo);
		if (!fileTo.open(QIODevice::WriteOnly)) {
			fileFrom.close();
			PERROR("failed to open file for writting%s\n", nameFrom.toAscii().data());
			return -1;
		}

		// the real copy part should perhaps be implemented using QDataStream
		// but .handle() will still be needed to get the optimal block-size
		//
		//! \todo does not keep file mode yet
		int bufferSize = 4096;
		int fileDescFrom = fileFrom.handle();
		int fileDescTo = fileTo.handle();

		struct stat fileStat;
		if (fstat(fileDescFrom, &fileStat) == 0)
			bufferSize = (int)fileStat.st_blksize;

		void *buffer = malloc(sizeof(char) * bufferSize);
		// QMemArray<char> buffer(bufferSize);

		for (;;) {
			int nRead = read(fileDescFrom, buffer, bufferSize);
			if (nRead < 0) {
				fileFrom.close();
				fileTo.close();
				PERROR("Error while reading file %s\n", nameFrom.toAscii().data());
				return -1;
			}
			if (nRead == 0)
				break;
			if (write(fileDescTo, buffer, nRead) < 0) {
				fileFrom.close();
				fileTo.close();
				PERROR("Error while writing file %s\n", nameTo.toAscii().data());
				return -1;
			}
		}
		free(buffer);

		fileFrom.close();
		fileTo.close();

		return 0;
	} else if(fileFromInfo.isDir()) {
		QDir dirFrom(nameFrom);
		QDir dirTo(nameTo);
		if (!dirTo.mkdir(nameTo)) {
			PERROR("failed to create directory %s\n", nameTo.toAscii().data());
			return -1;
		}

		QFileInfoList list = dirFrom.entryInfoList();
		QFileInfo fi;
		QString fileName;
		for (int i = 0; i < list.size(); ++i) {
			fileName = fi.fileName();
			if ((fileName != ".") && (fileName != "..")) {
				copy_recursively(pNameFrom + "/" + fileName, pNameTo + "/" + fileName);
			}
		}
		return 0;
	}

#endif

	return -1;
}

