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

#include "file-content.h"

#include <algorithm>

#include "bctoolbox/charconv.h"

#include "linphone/utils/utils.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

FileContent::FileContent(const FileContent &other) : Content(other) {
	Content::copy(other);
	setFileName(other.getFileName());
	setFilePath(other.getFilePath());
	mFileSize = other.getFileSize();
	mFileDuration = other.getFileDuration();
	mFileCreationTimestamp = other.getCreationTimestamp();
}

FileContent::FileContent(FileContent &&other) noexcept : Content(other) {
	Content::copy(other);
	mFileName = std::move(other.mFileName);
	mFilePath = std::move(other.mFilePath);
	mFileSize = std::move(other.mFileSize);
	mFileDuration = std::move(other.mFileDuration);
	mFileCreationTimestamp = std::move(other.getCreationTimestamp());
}

FileContent &FileContent::operator=(const FileContent &other) {
	Content::operator=(other);
	setFileName(other.getFileName());
	setFilePath(other.getFilePath());
	mFileSize = other.getFileSize();
	mFileDuration = other.getFileDuration();
	mFileCreationTimestamp = other.getCreationTimestamp();
	return *this;
}

FileContent &FileContent::operator=(FileContent &&other) {
	Content::operator=(std::move(other));
	mFileName = std::move(other.mFileName);
	mFilePath = std::move(other.mFilePath);
	mFileSize = std::move(other.mFileSize);
	mFileDuration = std::move(other.mFileDuration);
	mFileCreationTimestamp = std::move(other.getCreationTimestamp());
	return *this;
}

bool FileContent::operator==(const FileContent &other) const {
	return Content::operator==(other) && getFileName() == other.getFileName() && getFilePath() == other.getFilePath() &&
	       mFileSize == other.getFileSize() && mFileDuration == other.getFileDuration() &&
	       mFileCreationTimestamp == other.getCreationTimestamp();
}

void FileContent::setFileSize(size_t size) {
	mFileSize = size;
}

size_t FileContent::getFileSize() const {
	return mFileSize;
}

void FileContent::setFileName(const string &name) {
	mFileName = Utils::normalizeFilename(name);
}

const string &FileContent::getFileName() const {
	return mFileName;
}

void FileContent::setFileNameUtf8(const string &name) {
	setFileName(Utils::utf8ToLocale(name));
}

string FileContent::getFileNameUtf8() const {
	return Utils::localeToUtf8(getFileName());
}

void FileContent::setFilePath(const string &path) {
	mFilePath = path;
}

const string &FileContent::getFilePath() const {
	return mFilePath;
}

void FileContent::setFilePathUtf8(const string &path) {
	setFilePath(Utils::utf8ToLocale(path));
}

string FileContent::getFilePathUtf8() const {
	return Utils::localeToUtf8(getFilePath());
}

void FileContent::setFileDuration(int duration) {
	mFileDuration = duration;
}

int FileContent::getFileDuration() const {
	return mFileDuration;
}

void FileContent::setCreationTimestamp(time_t timestamp) {
	mFileCreationTimestamp = timestamp;
}

time_t FileContent::getCreationTimestamp() const {
	return mFileCreationTimestamp;
}

bool FileContent::isFile() const {
	return true;
}

bool FileContent::isFileTransfer() const {
	return false;
}

bool FileContent::isEncrypted() const {
	return isFileEncrypted(getFilePath());
}

const string FileContent::exportPlainFile() const {
	return exportPlainFileFromEncryptedFile(getFilePath());
}

LINPHONE_END_NAMESPACE
