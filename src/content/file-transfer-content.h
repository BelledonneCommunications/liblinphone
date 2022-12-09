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

#ifndef _L_FILE_TRANSFER_CONTENT_H_
#define _L_FILE_TRANSFER_CONTENT_H_

#include <vector>

#include "content.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class FileContent;
class FileTransferContentPrivate;

class LINPHONE_PUBLIC FileTransferContent : public Content {
public:
	FileTransferContent ();
	FileTransferContent (const FileTransferContent &other);
	FileTransferContent (FileTransferContent &&other);

	FileTransferContent* clone () const override {
		return new FileTransferContent(*this);
	}

	FileTransferContent &operator= (const FileTransferContent &other);
	FileTransferContent &operator= (FileTransferContent &&other);

	bool operator== (const FileTransferContent &other) const;

	void setFileName (const std::string &name);// App Locale
	const std::string &getFileName () const;
	
	void setFileNameSys (const std::string &name);// System Locale
	std::string getFileNameSys () const;
	
	void setFileNameUtf8 (const std::string &name);// UTF8
	std::string getFileNameUtf8 () const;

	void setFileUrl (const std::string &url);
	const std::string &getFileUrl () const;

	void setFilePath (const std::string &path);// App Locale
	const std::string &getFilePath () const;
	
	void setFilePathSys (const std::string &path);// System Locale
	std::string getFilePathSys () const;
	
	void setFilePathUtf8 (const std::string &path);// UTF8
	std::string getFilePathUtf8 () const;

	void setFileContent (FileContent *content);
	FileContent *getFileContent () const;

	void setFileSize (size_t size);
	size_t getFileSize () const;

	void setFileDuration (int durationInSeconds);
	int getFileDuration () const;

	void setFileKey (const char *key, size_t size);
	const std::vector<char> &getFileKey () const;
	size_t getFileKeySize() const;

	void setFileAuthTag (const char *authTag, size_t size);
	const std::vector<char> &getFileAuthTag () const;
	size_t getFileAuthTagSize() const;

	const ContentType &getFileContentType () const;
	void setFileContentType (const ContentType &contentType);

	bool isFile () const override;
	bool isFileTransfer () const override;

	bool isEncrypted () const;
	const std::string exportPlainFile () const;

private:
	L_DECLARE_PRIVATE(FileTransferContent);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_FILE_TRANSFER_CONTENT_H_
