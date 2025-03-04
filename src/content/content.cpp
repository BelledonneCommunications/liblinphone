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

#include "content.h"

#include "bctoolbox/port.h"
#include "bctoolbox/vfs_encrypted.hh"

#include "factory/factory.h"
#include "header/header-param.h"
#include "linphone/utils/algorithm.h"
#include "logger/logger.h"

#ifdef HAVE_ZLIB
#include "zlib.h"
#endif // HAVE_ZLIB

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================

Content::Content(const SalBodyHandler *bodyHandler, bool parseMultipart) {
	if (bodyHandler == nullptr) return;

	mBodyHandler = sal_body_handler_ref((SalBodyHandler *)bodyHandler);

	mContentType.setType(L_C_TO_STRING(sal_body_handler_get_type(bodyHandler)));
	mContentType.setSubType(L_C_TO_STRING(sal_body_handler_get_subtype(bodyHandler)));
	for (const belle_sip_list_t *params = sal_body_handler_get_content_type_parameters_names(bodyHandler); params;
	     params = params->next) {
		const char *paramName = reinterpret_cast<const char *>(params->data);
		const char *paramValue = sal_body_handler_get_content_type_parameter(bodyHandler, paramName);
		mContentType.addParameter(paramName, paramValue);
	}

	if (mContentType.isMultipart() && parseMultipart) {
		belle_sip_multipart_body_handler_t *mpbh = BELLE_SIP_MULTIPART_BODY_HANDLER(bodyHandler);
		char *body = belle_sip_object_to_string(mpbh);
		setBodyFromUtf8(body);
		belle_sip_free(body);
	} else {
		setBody(reinterpret_cast<char *>(sal_body_handler_get_data(bodyHandler)),
		        sal_body_handler_get_size(bodyHandler));
	}

	auto headers = reinterpret_cast<const belle_sip_list_t *>(sal_body_handler_get_headers(bodyHandler));
	while (headers) {
		belle_sip_header_t *cHeader = BELLE_SIP_HEADER(headers->data);
		Header header = Header(belle_sip_header_get_name(cHeader), belle_sip_header_get_unparsed_value(cHeader));
		addHeader(header);
		headers = headers->next;
	}

	if (sal_body_handler_get_encoding(bodyHandler)) mContentEncoding = sal_body_handler_get_encoding(bodyHandler);

	const char *disposition = sal_body_handler_get_content_disposition(bodyHandler);
	if (disposition) mContentDisposition = ContentDisposition(disposition);
}

Content::Content(const Content &other) : HybridObject(other), PropertyContainer(other) {
	copy(other);
}

Content::Content(Content &&other) noexcept : HybridObject(std::move(other)) {
	mBody = std::move(other.mBody);
	mContentType = std::move(other.mContentType);
	mContentDisposition = std::move(other.mContentDisposition);
	mContentEncoding = std::move(other.mContentEncoding);
	mHeaders = std::move(other.mHeaders);
	mCryptoContext = std::move(other.mCryptoContext);
	other.mCryptoContext = nullptr;
	mSize = std::move(other.mSize);
	mIsDirty = std::move(other.mIsDirty);
	mBodyHandler = std::move(other.mBodyHandler);
	other.mBodyHandler = nullptr;
}

Content::Content(ContentType &&ct, const std::string &data) : mContentType(ct) {
	setBodyFromLocale(data);
}

Content::Content(ContentType &&ct, std::vector<uint8_t> &&data) : mContentType(ct) {
	setBody(data);
}

Content::~Content() {
	/*
	 * Fills the body with zeros before releasing since it may contain
	 * private data like cipher keys or decoded messages.
	 */
	mBody.assign(mBody.size(), 0);
	if (mBodyHandler != nullptr) sal_body_handler_unref(mBodyHandler);
}

Content &Content::operator=(const Content &other) {
	if (this != &other) {
		PropertyContainer::operator=(other);
		copy(other);
	}
	return *this;
}

Content &Content::operator=(Content &&other) noexcept {
	mBody = std::move(other.mBody);
	mContentType = std::move(other.mContentType);
	mContentDisposition = std::move(other.mContentDisposition);
	mContentEncoding = std::move(other.mContentEncoding);
	mHeaders = std::move(other.mHeaders);
	mCryptoContext = std::move(other.mCryptoContext);
	other.mCryptoContext = nullptr;
	mSize = std::move(other.mSize);
	mIsDirty = std::move(other.mIsDirty);
	mBodyHandler = std::move(other.mBodyHandler);
	other.mBodyHandler = nullptr;
	return *this;
}

bool Content::operator==(const Content &other) const {
	return mContentType == other.getContentType() && mBody == other.getBody() &&
	       mContentDisposition == other.getContentDisposition() && mContentEncoding == other.getContentEncoding() &&
	       mHeaders == other.getHeaders();
}

void Content::copy(const Content &other) {
	mBody = other.getBody();
	mContentType = other.getContentType();
	mContentDisposition = other.getContentDisposition();
	mContentEncoding = other.getContentEncoding();
	mHeaders = other.getHeaders();
	mSize = other.mSize;
	mCache = other.mCache;
	if (!mIsDirty && mBodyHandler != nullptr) mBodyHandler = sal_body_handler_ref(other.mBodyHandler);
}

const ContentType &Content::getContentType() const {
	return mContentType;
}

ContentType &Content::getContentType() {
	return mContentType;
}

void Content::setContentType(const ContentType &contentType) {
	mContentType = contentType;
}

const ContentDisposition &Content::getContentDisposition() const {
	return mContentDisposition;
}

void Content::setContentDisposition(const ContentDisposition &contentDisposition) {
	mContentDisposition = contentDisposition;
}

const string &Content::getContentEncoding() const {
	return mContentEncoding;
}

void Content::setContentEncoding(const string &contentEncoding) {
	mContentEncoding = contentEncoding;
}

const vector<uint8_t> &Content::getBody() const {
	return mBody;
}

string Content::getBodyAsString() const {
	return Utils::utf8ToLocale(string(mBody.begin(), mBody.end()));
}

const string &Content::getBodyAsUtf8String() const {
	mCache.buffer = string(mBody.begin(), mBody.end());
	return mCache.buffer;
}

void Content::setBody(const vector<uint8_t> &body) {
	mBody = body;
}

void Content::setBody(vector<uint8_t> &&body) {
	mBody = std::move(body);
}

void Content::setBodyFromLocale(const string &body) {
	string toUtf8 = Utils::localeToUtf8(body);
	mBody = vector<uint8_t>(toUtf8.cbegin(), toUtf8.cend());
}

void Content::setBody(const void *buffer, size_t size) {
	mIsDirty = true;

	const char *start = static_cast<const char *>(buffer);
	if (start != nullptr) mBody = vector<uint8_t>(start, start + size);
	else mBody.clear();
}

void Content::setBodyFromUtf8(const string &body) {
	mIsDirty = true;

	mBody = vector<uint8_t>(body.cbegin(), body.cend());
}

const std::string &Content::getName() const {
	return mCache.name;
}

void Content::setName(const std::string &name) {
	mCache.name = name;
}

size_t Content::getSize() const {
	return mBody.empty() ? mSize : mBody.size();
}

void Content::setSize(size_t size) {
	mSize = size;
}

SalBodyHandler *Content::getBodyHandler() const {
	return mBodyHandler;
}

void Content::setBodyHandler(SalBodyHandler *bodyHandler) {
	mBodyHandler = bodyHandler;
}

void **Content::getCryptoContextAddress() {
	return &mCryptoContext;
}

bool Content::isEmpty() const {
	return getSize() == 0;
}

bool Content::isDirty() const {
	return mIsDirty;
}

bool Content::isMultipart() const {
	return mContentType.isValid() && mContentType == ContentType::Multipart;
}

bool Content::isValid() const {
	return mContentType.isValid() || (!mBody.empty());
}

bool Content::isFile() const {
	return false;
}

bool Content::isFileTransfer() const {
	return false;
}

const std::string &Content::getFilePath() const {
	return mCache.filePath;
}

void Content::setFilePath(const std::string &path) {
	mCache.filePath = path;
}

void Content::setRelatedChatMessageId(const string &messageId) {
	mMessageId = messageId;
}

const string &Content::getRelatedChatMessageId() const {
	return mMessageId;
}

void Content::addHeader(const string &headerName, const string &headerValue) {
	removeHeader(headerName);
	Header header = Header(headerName, headerValue);
	mHeaders.push_back(header);
}

void Content::addHeader(const Header &header) {
	removeHeader(header.getName());
	mHeaders.push_back(header);
}

const list<Header> &Content::getHeaders() const {
	return mHeaders;
}

const Header &Content::getHeader(const string &headerName) const {
	auto it = findHeader(headerName);
	if (it != mHeaders.cend()) {
		return *it;
	}
	return Utils::getEmptyConstRefObject<Header>();
}

void Content::removeHeader(const string &headerName) {
	auto it = findHeader(headerName);
	if (it != mHeaders.cend()) mHeaders.remove(*it);
}

list<Header>::const_iterator Content::findHeader(const string &headerName) const {
	return findIf(mHeaders, [&headerName](const Header &header) { return header.getName() == headerName; });
}

const std::string &Content::getCustomHeader(const std::string &headerName) const {
	SalBodyHandler *bodyHandler;

	if (!mIsDirty && mBodyHandler != nullptr) {
		bodyHandler = sal_body_handler_ref(mBodyHandler);
	} else {
		bodyHandler = getBodyHandlerFromContent(*this);
	}

	mCache.headerValue = L_C_TO_STRING(sal_body_handler_get_header(bodyHandler, headerName.c_str()));
	sal_body_handler_unref(bodyHandler);
	return mCache.headerValue;
}

void Content::setUserData(const Variant &userData) {
	setProperty("LinphonePrivate::Content::userData", userData);
}

Variant Content::getUserData() const {
	return getProperty("LinphonePrivate::Content::userData");
}

SalBodyHandler *Content::getBodyHandlerFromContent(const Content &content, bool parseMultipart) {
	if (!content.mIsDirty && content.mBodyHandler != nullptr) return sal_body_handler_ref(content.mBodyHandler);

	SalBodyHandler *bodyHandler = nullptr;
	ContentType contentType = content.mContentType;
	if (contentType.isMultipart() && parseMultipart) {
		size_t size = content.getSize();
		char *buffer = bctbx_strdup(content.getBodyAsUtf8String().c_str());
		const char *boundary = L_STRING_TO_C(contentType.getParameter("boundary").getValue());
		belle_sip_multipart_body_handler_t *bh = nullptr;
		if (boundary) bh = belle_sip_multipart_body_handler_new_from_buffer(buffer, size, boundary);
		else if (size > 2) {
			size_t startIndex = 2, index = 0;
			while (startIndex < size &&
			       (buffer[startIndex - 1] != '-' // Take accout of first "--"
			        || (startIndex == 2 && buffer[0] != '-') ||
			        (startIndex > 2 && (buffer[startIndex] != '-' ||
			                            buffer[startIndex - 2] != '\n')))) { // Must be at the beginning of the line
				++startIndex;
			}
			index = startIndex;
			while (index < size && buffer[index] != '\n' && buffer[index] != '\r')
				++index;
			if (startIndex != index) {
				char *boundaryStr = bctbx_strndup(buffer + startIndex, (int)(index - startIndex));
				bh = belle_sip_multipart_body_handler_new_from_buffer(buffer, size, boundaryStr);
				bctbx_free(boundaryStr);
			}
		}

		bodyHandler = reinterpret_cast<SalBodyHandler *>(BELLE_SIP_BODY_HANDLER(bh));
		bctbx_free(buffer);
	} else {
		bodyHandler = sal_body_handler_new_from_buffer(content.mBody.data(), content.mBody.size());
	}

	for (const auto &header : content.getHeaders()) {
		sal_body_handler_add_header(bodyHandler, header.getName().c_str(), header.getValueWithParams().c_str());
	}

	sal_body_handler_set_type(bodyHandler, contentType.getType().c_str());
	sal_body_handler_set_subtype(bodyHandler, contentType.getSubType().c_str());
	sal_body_handler_set_size(bodyHandler, content.getSize());
	for (const auto &param : contentType.getParameters())
		sal_body_handler_set_content_type_parameter(bodyHandler, param.getName().c_str(), param.getValue().c_str());

	if (!content.mContentEncoding.empty()) sal_body_handler_set_encoding(bodyHandler, content.mContentEncoding.c_str());

	const ContentDisposition &disposition = content.getContentDisposition();
	if (disposition.isValid()) sal_body_handler_set_content_disposition(bodyHandler, disposition.asString().c_str());

	return bodyHandler;
}

bool Content::isFileEncrypted(const string &filePath) const {
	if (filePath.empty()) {
		return false;
	}

	// is the encryptedVfs set up?
	if (bctoolbox::VfsEncryption::openCallbackGet() == nullptr) {
		// no: we can't open the file using encryptedVFS
		// no real way to check if the file is encrypted but there is no way to decrypt it anyway
		return false;
	}

	// open the file using encrypted vfs
	auto fp = bctbx_file_open(&bctoolbox::bcEncryptedVfs, filePath.data(), "r");
	if (fp == NULL) {
		lError() << "[Content] Can't open file " << filePath << " to decrypt it";
		return false;
	}

	auto ret = bctbx_file_is_encrypted(fp);
	bctbx_file_close(fp);

	return (ret == TRUE);
}

const string Content::exportPlainFileFromEncryptedFile(const string &filePath) const {
	if (filePath.empty()) {
		return filePath;
	}

	// plain files are stored in an "evfs" subdirectory of the cache directory
	std::string cacheDir(Factory::get()->getCacheDir(nullptr) + "/evfs/");

	// Create the directory if it is not present
	if (!bctbx_directory_exists(cacheDir.c_str())) {
		bctbx_mkdir(cacheDir.c_str());
	}

	std::string plainPath(cacheDir);
	auto basename = bctbx_basename(filePath.c_str());
	plainPath.append(basename);

	if (bctbx_file_exist(plainPath.c_str()) == 0) {
		lWarning() << "[Content] File [" << plainPath << "] already exists, trying another name";

		int index = 1;
		string plainPathTest(cacheDir + std::to_string(index) + "_" + basename);
		while (bctbx_file_exist(plainPathTest.c_str()) == 0) {
			lWarning() << "[Content] File [" << plainPathTest << "] already exists, trying another name";
			index += 1;
			plainPathTest = cacheDir + std::to_string(index) + "_" + basename;
		}
		plainPath = plainPathTest;
		lInfo() << "[Content] Using file [" << plainPath << "]";
	}
	bctbx_free(basename);

	// open the file using encrypted vfs
	auto cf = bctbx_file_open(&bctoolbox::bcEncryptedVfs, filePath.c_str(), "r");
	// open the plain file using standard vfs
	if (cf == NULL) {
		lError() << "[Content] Can't open file " << filePath << " to decrypt it";
		return std::string();
	}

	ssize_t fileSize = bctbx_file_size(cf);
	if (fileSize < 0) {
		lError() << "[Content] Can't read size of file " << filePath;
		bctbx_file_close(cf);
		return std::string();
	}

	auto pf = bctbx_file_open(bctbx_vfs_get_standard(), plainPath.c_str(), "w");
	if (pf == NULL) {
		lError() << "[Content] Can't create file " << plainPath << " to decrypt " << filePath;
		return std::string();
	}

	size_t constexpr bufSize = 100 * 1024; // read by chunks of 100 kB
	uint8_t *readBuf = new uint8_t[bufSize];
	off_t readSize = 0;
	while (readSize < fileSize) {
		ssize_t read = bctbx_file_read(cf, readBuf, bufSize, readSize);
		if (read < 0) {
			lError() << "[Content] Can't read file " << filePath << " to decrypt it";
			bctbx_file_close(cf);
			bctbx_file_close(pf);
			std::remove(plainPath.data());
			delete[] readBuf;
			return std::string();
		}

		auto write = bctbx_file_write(pf, readBuf, (size_t)read, readSize);
		if (write < 0 || write != read) {
			lError() << "[Content] Can't write file " << plainPath << " - plain version of " << filePath;
			bctbx_file_close(cf);
			bctbx_file_close(pf);
			std::remove(plainPath.data());
			delete[] readBuf;
			return std::string();
		}

		readSize += (off_t)read;
	}

	delete[] readBuf;
	bctbx_file_close(cf);
	bctbx_file_close(pf);

	return plainPath;
}

std::ostream &operator<<(std::ostream &stream, const Content &content) {
	return stream << "Content of type " << content.getContentType() << " with body " << content.getBodyAsUtf8String();
}

std::ostream &operator<<(std::ostream &stream, const std::list<Content> &contents) {
	for (const auto &content : contents) {
		stream << content << "\n";
	}
	return stream;
}

bool Content::deflateBody(void) {
#ifdef HAVE_ZLIB
	z_stream zlibStream = {};
	auto ret = deflateInit(&zlibStream, Z_DEFAULT_COMPRESSION);
	if (ret != Z_OK) {
		lError() << "Content deflateInit failed: " << ret;
		deflateEnd(&zlibStream);
		return false;
	} else {
		zlibStream.avail_in = static_cast<uInt>(mBody.size());
		zlibStream.next_in = mBody.data();
		std::vector<uint8_t> compressedMessage(deflateBound(&zlibStream, static_cast<uLong>(mBody.size())));
		zlibStream.avail_out = static_cast<uInt>(compressedMessage.size());
		zlibStream.next_out = compressedMessage.data();
		ret = deflate(&zlibStream, Z_FINISH);
		if (ret != Z_STREAM_END) {
			lError() << "Content deflate failed: " << ret;
			deflateEnd(&zlibStream);
			return false;
		} else {
			auto compressedSize = compressedMessage.size() - zlibStream.avail_out;
			// resize the output buffer according to what was actually written in it
			compressedMessage.resize(compressedSize);
			lInfo() << "Content deflate body from " << mBody.size() << " bytes to " << compressedMessage.size()
			        << " bytes";
			mBody = std::move(compressedMessage);
			setContentEncoding("deflate");
		}
	}
	deflateEnd(&zlibStream);
	return true;
#else  // HAVE_ZLIB
	lError() << "Cannot deflate content as zlib support is missing" return false;
#endif // HAVE_ZLIB
}

bool Content::inflateBody(void) {
#ifdef HAVE_ZLIB
	z_stream zlibStream = {};
	auto ret = inflateInit(&zlibStream);
	auto initialSize = mBody.size();
	if (ret != Z_OK) {
		lError() << "Content inflateInit failed: " << ret;
		inflateEnd(&zlibStream);
		return false;
	} else {
		zlibStream.avail_in = static_cast<uInt>(mBody.size());
		zlibStream.next_in = mBody.data();
		std::vector<uint8_t> outBuf(MIN(static_cast<size_t>(4096), 3 * mBody.size()));
		zlibStream.avail_out = static_cast<uInt>(outBuf.size());
		zlibStream.next_out = outBuf.data();
		ret = inflate(&zlibStream, Z_SYNC_FLUSH);
		if (ret != Z_STREAM_END && ret != Z_OK) {
			lError() << "Content inflate failed: " << ret;
			inflateEnd(&zlibStream);
			return false;
		}
		if (ret == Z_STREAM_END) { // inflate finished in one pass
			outBuf.resize(outBuf.size() - zlibStream.avail_out);
			mBody = std::move(outBuf);
		} else { // more passes to perform
			std::vector<uint8_t> inflatedBody{outBuf.cbegin(), outBuf.cend()};
			do {
				zlibStream.avail_out = static_cast<uInt>(outBuf.size());
				zlibStream.next_out = outBuf.data();
				ret = inflate(&zlibStream, Z_SYNC_FLUSH);
				if (ret == Z_OK) { // Still more data to inflate
					inflatedBody.insert(inflatedBody.end(), outBuf.cbegin(), outBuf.cend());
				} else {
					if (ret != Z_STREAM_END) {
						lError() << "Content inflate failed: " << ret;
						inflateEnd(&zlibStream);
						return false;
					}
				}
			} while (ret != Z_STREAM_END);
			// get the output of the last pass
			outBuf.resize(outBuf.size() - zlibStream.avail_out);
			inflatedBody.insert(inflatedBody.end(), outBuf.cbegin(), outBuf.cend());
			mBody = std::move(inflatedBody);
		}
	}
	lInfo() << "Content inflate message from " << initialSize << " bytes to " << mBody.size() << " bytes";
	inflateEnd(&zlibStream);
	setContentEncoding("");
	return true;
#else  // HAVE_ZLIB
	lError() << "Cannot inflate content as zlib support is missing" return false;
#endif // HAVE_ZLIB
}
LINPHONE_END_NAMESPACE
