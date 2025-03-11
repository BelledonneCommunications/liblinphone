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

#ifndef _L_CONTENT_H_
#define _L_CONTENT_H_

#include <list>
#include <vector>

#include "belle-sip/object++.hh"

#include "c-wrapper/internal/c-sal.h"
#include "content-disposition.h"
#include "content-type.h"
#include "header/header.h"
#include "linphone/api/c-types.h"
#include "object/property-container.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC Content : public bellesip::HybridObject<LinphoneContent, Content>, public PropertyContainer {
public:
	Content() = default;
	explicit Content(const SalBodyHandler *bodyHandler, bool parseMultipart = true);
	Content(const Content &other);
	Content(Content &&other) noexcept;
	Content(ContentType &&ct, const std::string &data);
	Content(ContentType &&ct, std::vector<uint8_t> &&data);
	virtual ~Content();

	Content *clone() const override {
		return new Content(*this);
	}

	Content &operator=(const Content &other);
	Content &operator=(Content &&other) noexcept;

	bool operator==(const Content &other) const;

	void copy(const Content &other);

	/* This accessor returns a modifiable ContentType, which is useful to add parameters to an already constructed
	 * Content */
	ContentType &getContentType();
	const ContentType &getContentType() const;
	void setContentType(const ContentType &contentType);

	const ContentDisposition &getContentDisposition() const;
	void setContentDisposition(const ContentDisposition &contentDisposition);

	const std::string &getContentEncoding() const;
	void setContentEncoding(const std::string &contentEncoding);

	const std::vector<uint8_t> &getBody() const;
	std::string getBodyAsString() const;
	const std::string &getBodyAsUtf8String() const;

	void setBody(const std::vector<uint8_t> &body);
	void setBody(std::vector<uint8_t> &&body);
	void setBodyFromLocale(const std::string &body);
	void setBody(const void *buffer, size_t size);
	void setBodyFromUtf8(const std::string &body);

	const std::string &getName() const;
	void setName(const std::string &name);

	size_t getSize() const;
	void setSize(size_t size);

	SalBodyHandler *getBodyHandler() const;
	void setBodyHandler(SalBodyHandler *bodyHandler);

	void **getCryptoContextAddress();

	bool isValid() const;
	bool isMultipart() const;
	bool isEmpty() const;
	bool isDirty() const;

	virtual bool isFile() const;
	virtual bool isFileTransfer() const;

	virtual const std::string &getFilePath() const;
	virtual void setFilePath(const std::string &path);

	const std::list<Header> &getHeaders() const;
	const Header &getHeader(const std::string &headerName) const;
	void addHeader(const std::string &headerName, const std::string &headerValue);
	void addHeader(const Header &header);
	void removeHeader(const std::string &headerName);
	std::list<Header>::const_iterator findHeader(const std::string &headerName) const;
	const std::string &getCustomHeader(const std::string &headerName) const;

	void setRelatedChatMessageId(const std::string &messageId);
	const std::string &getRelatedChatMessageId() const;

	void setUserData(const Variant &userData);
	Variant getUserData() const;

	static SalBodyHandler *getBodyHandlerFromContent(const Content &content, bool parseMultipart = true);

	/**
	 * compress this content
	 * content size is modified and set the content-encoding to deflate
	 * @return true on success
	 */
	bool deflateBody(void);
	/**
	 * decompress this content
	 * content size is modified and the content-encoding is unset
	 * @return true on success
	 */
	bool inflateBody(void);

protected:
	bool isFileEncrypted(const std::string &filePath) const;
	const std::string exportPlainFileFromEncryptedFile(const std::string &filePath) const;

private:
	std::vector<uint8_t> mBody;
	ContentType mContentType;
	ContentDisposition mContentDisposition;
	std::string mContentEncoding;
	std::list<Header> mHeaders;

	void *mCryptoContext = nullptr; // Used to encrypt file for RCS file transfer.
	bool mIsDirty = false;
	SalBodyHandler *mBodyHandler = nullptr;
	std::string mMessageId;

	struct Cache {
		std::string name;
		std::string buffer;
		std::string filePath;
		std::string headerValue;
	} mutable mCache;
	mutable size_t mSize = 0;
};

std::ostream &operator<<(std::ostream &stream, const Content &content);
std::ostream &operator<<(std::ostream &stream, const std::list<Content> &contents);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONTENT_H_
