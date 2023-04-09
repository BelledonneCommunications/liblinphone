/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone 
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// TODO: Remove me later.
#include "linphone/core.h"
#include "linphone/utils/utils.h"


#include "content-p.h"
#include "file-content.h"
#include "bctoolbox/charconv.h"
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
	int fileDuration = 0;
};

// -----------------------------------------------------------------------------

FileContent::FileContent () : Content(*new FileContentPrivate) {}

FileContent::FileContent (const FileContent &other) : Content(*new FileContentPrivate) {
	L_D();
	Content::copy(other);
	setFileName(other.getFileName());
	setFilePath(other.getFilePath());
	d->fileSize = other.getFileSize();
	d->fileDuration = other.getFileDuration();
}

FileContent::FileContent (FileContent &&other) : Content(*new FileContentPrivate) {
	L_D();
	Content::copy(other);
	d->fileName = std::move(other.getPrivate()->fileName);
	d->filePath = std::move(other.getPrivate()->filePath);
	d->fileSize = std::move(other.getPrivate()->fileSize);
	d->fileDuration = std::move(other.getPrivate()->fileDuration);
}

FileContent &FileContent::operator= (const FileContent &other) {
	L_D();
	Content::operator=(other);
	setFileName(other.getFileName());
	setFilePath(other.getFilePath());
	d->fileSize = other.getFileSize();
	d->fileDuration = other.getFileDuration();
	return *this;
}

FileContent &FileContent::operator= (FileContent &&other) {
	L_D();
	Content::operator=(std::move(other));
	d->fileName = std::move(other.getPrivate()->fileName);
	d->filePath = std::move(other.getPrivate()->filePath);
	d->fileSize = std::move(other.getPrivate()->fileSize);
	d->fileDuration = std::move(other.getPrivate()->fileDuration);
	return *this;
}

bool FileContent::operator== (const FileContent &other) const {
	L_D();
	return Content::operator==(other) &&
		getFileName() == other.getFileName() &&
		getFilePath() == other.getFilePath() &&
		d->fileSize == other.getFileSize() &&
		d->fileDuration == other.getFileDuration();
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
	d->fileName = Utils::normalizeFilename(name);
}

const string &FileContent::getFileName () const {
	L_D();
	return d->fileName;
}

void FileContent::setFileNameSys (const string &name) {
	setFileName(Utils::convert(name, "", bctbx_get_default_encoding()));
}

string FileContent::getFileNameSys () const {
	return Utils::convert(getFileName(), bctbx_get_default_encoding(), "");
}

void FileContent::setFileNameUtf8 (const string &name) {
	setFileName(Utils::utf8ToLocale(name));
}

string FileContent::getFileNameUtf8 () const {
	return Utils::localeToUtf8(getFileName());
}

void FileContent::setFilePath (const string &path) {
	L_D();
	d->filePath = path;
}

const string &FileContent::getFilePath () const {
	L_D();
	return d->filePath;
}

void FileContent::setFilePathSys (const string &path) {
	setFilePath(Utils::convert(path, "", bctbx_get_default_encoding()));
}

string FileContent::getFilePathSys () const {
	return Utils::convert(getFilePath(), bctbx_get_default_encoding(), "");
}

void FileContent::setFilePathUtf8 (const string &path) {
	setFilePath(Utils::utf8ToLocale(path));
}

string FileContent::getFilePathUtf8 () const {
	return Utils::localeToUtf8(getFilePath());
}

void FileContent::setFileDuration (int durationInSeconds) {
	L_D();
	d->fileDuration = durationInSeconds;
}

int FileContent::getFileDuration () const {
	L_D();
	return d->fileDuration;
}

bool FileContent::isFile () const {
	return true;
}

bool FileContent::isFileTransfer () const {
	return false;
}

bool FileContent::isEncrypted () const {
	return isFileEncrypted(getFilePathSys());
}

const string FileContent::exportPlainFile() const {
	return Utils::convert(exportPlainFileFromEncryptedFile(getFilePathSys()), "", bctbx_get_default_encoding());
}

LINPHONE_END_NAMESPACE
