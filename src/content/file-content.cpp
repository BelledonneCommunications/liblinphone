/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// TODO: Remove me later.
#include "linphone/core.h"

#include "content-p.h"
#include "file-content.h"
#include <algorithm>

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class FileContentPrivate : public ContentPrivate {
public:
	string fileName;
	string filePath;
	size_t fileSize = 0;
};

// -----------------------------------------------------------------------------

FileContent::FileContent () : Content(*new FileContentPrivate) {}

FileContent::FileContent (const FileContent &other) : Content(*new FileContentPrivate) {
	L_D();
	Content::copy(other);
	d->fileName = other.getFileName();
	d->filePath = other.getFilePath();
	d->fileSize = other.getFileSize();
}

FileContent::FileContent (FileContent &&other) : Content(*new FileContentPrivate) {
	L_D();
	Content::copy(other);
	d->fileName = move(other.getPrivate()->fileName);
	d->filePath = move(other.getPrivate()->filePath);
	d->fileSize = move(other.getPrivate()->fileSize);
}

FileContent &FileContent::operator= (const FileContent &other) {
	L_D();
	Content::operator=(other);
	d->fileName = other.getFileName();
	d->filePath = other.getFilePath();
	d->fileSize = other.getFileSize();
	return *this;
}

FileContent &FileContent::operator= (FileContent &&other) {
	L_D();
	Content::operator=(move(other));
	d->fileName = move(other.getPrivate()->fileName);
	d->filePath = move(other.getPrivate()->filePath);
	d->fileSize = move(other.getPrivate()->fileSize);
	return *this;
}

bool FileContent::operator== (const FileContent &other) const {
	L_D();
	return Content::operator==(other) &&
		d->fileName == other.getFileName() &&
		d->filePath == other.getFilePath() &&
		d->fileSize == other.getFileSize();
}

void FileContent::setFileSize (size_t size) {
	L_D();
	d->fileSize = size;
}

size_t FileContent::getFileSize () const {
	L_D();
	return d->fileSize;
}

void FileContent::setFileName (const string &name) {
	L_D();
#ifdef WIN32	
	const std::string illegalCharacters = "\\/:*\"<>|";
#elseif APPLE
	const std::string illegalCharacters = ":/";
#else
	const std::string illegalCharacters = "/";
#endif
	d->fileName = name;
// Invisible and illegal characters should not be part of a filename	
	d->fileName.erase(std::remove_if(d->fileName.begin(), d->fileName.end(), [illegalCharacters](const char& c){ return c<' ' || illegalCharacters.find(c) != std::string::npos; }), d->fileName.end());
}

const string &FileContent::getFileName () const {
	L_D();
	return d->fileName;
}

void FileContent::setFilePath (const string &path) {
	L_D();
	d->filePath = path;
}

const string &FileContent::getFilePath () const {
	L_D();
	return d->filePath;
}

bool FileContent::isFile () const {
	return true;
}

bool FileContent::isFileTransfer () const {
	return false;
}

LINPHONE_END_NAMESPACE
