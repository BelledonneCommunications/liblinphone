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

#include "file-transfer-content.h"

#include <algorithm>

#include "bctoolbox/charconv.h"

#include "linphone/utils/utils.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

FileTransferContent::FileTransferContent() {
	setContentType(ContentType::FileTransfer);
}

FileTransferContent::FileTransferContent(const FileTransferContent &other) : Content(other) {
	Content::copy(other);
	setFileName(other.getFileName());
	setFilePath(other.getFilePath());
	mFileUrl = other.getFileUrl();
	mFileContent = other.getFileContent();
	mFileSize = other.getFileSize();
	mFileKey = other.getFileKey();
	mFileAuthTag = other.getFileAuthTag();
	mFileContentType = other.getFileContentType();
	mFileDuration = other.getFileDuration();
}

FileTransferContent::FileTransferContent(FileTransferContent &&other) : Content(other) {
	Content::copy(other);
	mFileName = std::move(other.mFileName);
	mFileUrl = std::move(other.mFileUrl);
	mFilePath = std::move(other.mFilePath);
	mFileContent = std::move(other.mFileContent);
	mFileSize = std::move(other.mFileSize);
	mFileKey = std::move(other.mFileKey);
	mFileAuthTag = std::move(other.mFileAuthTag);
	mFileContentType = std::move(other.mFileContentType);
	mFileDuration = std::move(other.mFileDuration);
}

FileTransferContent &FileTransferContent::operator=(const FileTransferContent &other) {
	if (this != &other) {
		Content::operator=(other);
		setFileName(other.getFileName());
		setFilePath(other.getFilePath());
		mFileUrl = other.getFileUrl();
		mFileContent = other.getFileContent();
		mFileSize = other.getFileSize();
		mFileKey = other.getFileKey();
		mFileAuthTag = other.getFileAuthTag();
		mFileContentType = other.getFileContentType();
		mFileDuration = other.getFileDuration();
	}

	return *this;
}

FileTransferContent &FileTransferContent::operator=(FileTransferContent &&other) {
	Content::operator=(std::move(other));
	mFileName = std::move(other.mFileName);
	mFileUrl = std::move(other.mFileUrl);
	mFilePath = std::move(other.mFilePath);
	mFileContent = std::move(other.mFileContent);
	mFileSize = std::move(other.mFileSize);
	mFileKey = std::move(other.mFileKey);
	mFileAuthTag = std::move(other.mFileAuthTag);
	mFileContentType = std::move(other.mFileContentType);
	mFileDuration = std::move(other.mFileDuration);

	return *this;
}

bool FileTransferContent::operator==(const FileTransferContent &other) const {
	return Content::operator==(other) && getFileName() == other.getFileName() && mFileUrl == other.getFileUrl() &&
	       getFilePath() == other.getFilePath() && mFileSize == other.getFileSize() &&
	       mFileContentType == other.getFileContentType() && mFileDuration == other.getFileDuration();
}

void FileTransferContent::setFileName(const string &name) {
	mFileName = Utils::normalizeFilename(name);
}

const string &FileTransferContent::getFileName() const {
	return mFileName;
}

void FileTransferContent::setFileNameUtf8(const string &name) {
	setFileName(Utils::utf8ToLocale(name));
}

string FileTransferContent::getFileNameUtf8() const {
	return Utils::localeToUtf8(getFileName());
}

void FileTransferContent::setFileUrl(const string &url) {
	mFileUrl = url;
}

const string &FileTransferContent::getFileUrl() const {
	return mFileUrl;
}

void FileTransferContent::setFilePath(const string &path) {
	mFilePath = path;
}

const string &FileTransferContent::getFilePath() const {
	return mFilePath;
}

void FileTransferContent::setFilePathUtf8(const string &path) {
	setFilePath(Utils::utf8ToLocale(path));
}

string FileTransferContent::getFilePathUtf8() const {
	return Utils::localeToUtf8(getFilePath());
}

void FileTransferContent::setFileContent(std::shared_ptr<FileContent> content) {
	mFileContent = content;
}

std::shared_ptr<FileContent> FileTransferContent::getFileContent() const {
	return mFileContent;
}

void FileTransferContent::setFileSize(size_t size) {
	mFileSize = size;
}

size_t FileTransferContent::getFileSize() const {
	return mFileSize;
}

void FileTransferContent::setFileDuration(int duration) {
	mFileDuration = duration;
}

int FileTransferContent::getFileDuration() const {
	return mFileDuration;
}

void FileTransferContent::setFileKey(const char *key, size_t size) {
	mFileKey = vector<char>(key, key + size);
}

const vector<char> &FileTransferContent::getFileKey() const {
	return mFileKey;
}

size_t FileTransferContent::getFileKeySize() const {
	return mFileKey.size();
}

void FileTransferContent::setFileAuthTag(const char *tag, size_t size) {
	mFileAuthTag = vector<char>(tag, tag + size);
}

const vector<char> &FileTransferContent::getFileAuthTag() const {
	return mFileAuthTag;
}

size_t FileTransferContent::getFileAuthTagSize() const {
	return mFileAuthTag.size();
}

void FileTransferContent::setFileContentType(const ContentType &contentType) {
	mFileContentType = contentType;
}

const ContentType &FileTransferContent::getFileContentType() const {
	return mFileContentType;
}

bool FileTransferContent::isFile() const {
	return false;
}

bool FileTransferContent::isFileTransfer() const {
	return true;
}

bool FileTransferContent::isEncrypted() const {
	return isFileEncrypted(getFilePath());
}

const string FileTransferContent::exportPlainFile() const {
	return exportPlainFileFromEncryptedFile(getFilePath());
}

LINPHONE_END_NAMESPACE
