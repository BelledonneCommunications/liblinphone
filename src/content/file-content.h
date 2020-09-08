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

#ifndef _L_FILE_CONTENT_H_
#define _L_FILE_CONTENT_H_

#include "content.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class FileContentPrivate;

class LINPHONE_PUBLIC FileContent : public Content {
public:
	FileContent ();
	FileContent (const FileContent &other);
	FileContent (FileContent &&other);

	FileContent* clone () const override {
		return new FileContent(*this);
	}

	FileContent &operator= (const FileContent &other);
	FileContent &operator= (FileContent &&other);

	bool operator== (const FileContent &other) const;

	void setFileSize (size_t size);
	size_t getFileSize () const;

	void setFileName (const std::string &name);
	const std::string &getFileName () const;

	void setFilePath (const std::string &path);
	const std::string &getFilePath () const;

	bool isFile () const override;
	bool isFileTransfer () const override;

	bool isEncrypted () const;
	/**
	 * Return the path to a file's plain version.
	 * This shall be a temporary copy
	 * Caller is then responsible to delete it when no more needed
	 */
	const std::string getPlainFilePath () const;

private:
	L_DECLARE_PRIVATE(FileContent);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_FILE_CONTENT_H_
