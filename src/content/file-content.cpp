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
#include "bctoolbox/vfs_encrypted.hh"
#include "logger/logger.h"

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
	d->fileName = name;
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

bool FileContent::isEncrypted () const {
	L_D();
	if (d->filePath.empty()) {
		return false;
	}
	// open the file using encrypted vfs
	auto fp = bctbx_file_open(&bctoolbox::bcEncryptedVfs, d->filePath.data(), "r");
	if (fp==NULL)   {
		lError()<<"Can't open file "<<d->filePath<<" to decrypt it";
		return false;
	}

	auto ret = bctbx_file_isEncrypted(fp);
	bctbx_file_close(fp);

	return (ret==TRUE);
}

const string FileContent::getPlainFilePath() const {
	L_D();
	if (d->filePath.empty()) {
		return d->filePath;
	}

	string plainPath(d->filePath);
	plainPath.append(".bctbx_evfs_plain"); // TODO: have a tmp dir to store all plain version so it is easier to clean?

	// open the file using encrypted vfs
	auto cf = bctbx_file_open(&bctoolbox::bcEncryptedVfs, d->filePath.data(), "r");
	// open the plain file using standard vfs
	if (cf==NULL)   {
		lError()<<"Can't open file "<<d->filePath<<" to decrypt it";
		return std::string();
	}
	auto fileSize = bctbx_file_size(cf);
	if (fileSize<0) {
		lError()<<"Can't read size of file "<<d->filePath;
		bctbx_file_close(cf);
		return std::string();
	}
	auto pf = bctbx_file_open(bctbx_vfs_get_standard(), plainPath.data(), "w");
	if (pf==NULL)   {
		lError()<<"Can't create file "<<plainPath<<" to decrypt "<<d->filePath;
		return std::string();
	}
	size_t constexpr bufSize = 100*1024; // read by chunks of 100 kB
	uint8_t readBuf[bufSize];
	ssize_t readSize = 0;
	while(readSize<fileSize) {
		auto read = bctbx_file_read(cf, readBuf, bufSize, readSize);
		if (read < 0 ) {
			lError()<<"Can't read file "<<d->filePath<<" to decrypt it";
			bctbx_file_close(cf);
			bctbx_file_close(pf);
			std::remove(plainPath.data());
			return std::string();
		}
		auto write = bctbx_file_write(pf, readBuf, read, readSize);
		if (write < 0 || write != read) {
			lError()<<"Can't write file "<<plainPath<<" - plain version of "<<d->filePath;
			bctbx_file_close(cf);
			bctbx_file_close(pf);
			std::remove(plainPath.data());
			return std::string();
		}
		readSize += read;
	}
	bctbx_file_close(cf);
	bctbx_file_close(pf);

	return plainPath;
}

LINPHONE_END_NAMESPACE
