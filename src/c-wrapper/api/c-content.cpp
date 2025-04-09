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

#include <bctoolbox/defs.h>

#include "linphone/api/c-content.h"

#include "c-wrapper/c-wrapper.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "content/file-content.h"
#include "content/file-transfer-content.h"
#include "content/header/header-param.h"
#include "private_functions.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

// =============================================================================
// Reference and user data handling functions.
// =============================================================================

LinphoneContent *linphone_content_ref(LinphoneContent *content) {
	Content::toCpp(content)->ref();
	return content;
}

void linphone_content_unref(LinphoneContent *content) {
	Content::toCpp(content)->unref();
}

void *linphone_content_get_user_data(const LinphoneContent *content) {
	return Content::toCpp(content)->getUserData().getValue<void *>();
}

void linphone_content_set_user_data(LinphoneContent *content, void *user_data) {
	return Content::toCpp(content)->setUserData(user_data);
}

// =============================================================================

const char *linphone_content_get_type(const LinphoneContent *content) {
	return L_STRING_TO_C(Content::toCpp(content)->getContentType().getType());
}

void linphone_content_set_type(LinphoneContent *content, const char *type) {
	auto contentCpp = Content::toCpp(content);
	auto contentType = contentCpp->getContentType();
	contentType.setType(L_C_TO_STRING(type));
	contentCpp->setContentType(contentType);
}

const char *linphone_content_get_subtype(const LinphoneContent *content) {
	return L_STRING_TO_C(Content::toCpp(content)->getContentType().getSubType());
}

void linphone_content_set_subtype(LinphoneContent *content, const char *subtype) {
	auto contentCpp = Content::toCpp(content);
	ContentType contentType = contentCpp->getContentType();
	contentType.setSubType(L_C_TO_STRING(subtype));
	contentCpp->setContentType(contentType);
}

void linphone_content_add_content_type_parameter(LinphoneContent *content, const char *name, const char *value) {
	auto contentCpp = Content::toCpp(content);
	ContentType contentType = contentCpp->getContentType();
	contentType.addParameter(L_C_TO_STRING(name), L_C_TO_STRING(value));
	contentCpp->setContentType(contentType);
}

const uint8_t *linphone_content_get_buffer(const LinphoneContent *content) {
	return reinterpret_cast<const uint8_t *>(linphone_content_get_utf8_text(content));
}

void linphone_content_set_buffer(LinphoneContent *content, const uint8_t *buffer, size_t size) {
	Content::toCpp(content)->setBody(buffer, size);
}

const char *linphone_content_get_string_buffer(const LinphoneContent *content) {
	return L_STRING_TO_C(Content::toCpp(content)->getBodyAsUtf8String());
}

const char *linphone_content_get_utf8_text(const LinphoneContent *content) {
	return L_STRING_TO_C(Content::toCpp(content)->getBodyAsUtf8String());
}

void linphone_content_set_utf8_text(LinphoneContent *content, const char *buffer) {
	Content::toCpp(content)->setBodyFromUtf8(L_C_TO_STRING(buffer));
}

void linphone_content_set_string_buffer(LinphoneContent *content, const char *buffer) {
	Content::toCpp(content)->setBodyFromUtf8(L_C_TO_STRING(buffer));
}

size_t linphone_content_get_file_size(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	size_t size = 0;
	if (c->isFile()) size = dynamic_cast<const FileContent *>(c)->getFileSize();
	else if (c->isFileTransfer()) size = dynamic_cast<const FileTransferContent *>(c)->getFileSize();
	return size;
}

size_t linphone_content_get_size(const LinphoneContent *content) {
	return Content::toCpp(content)->getSize();
}

void linphone_content_set_size(LinphoneContent *content, size_t size) {
	Content::toCpp(content)->setSize(size);
}

const char *linphone_content_get_encoding(const LinphoneContent *content) {
	return L_STRING_TO_C(Content::toCpp(content)->getContentEncoding());
}

void linphone_content_set_encoding(LinphoneContent *content, const char *encoding) {
	Content::toCpp(content)->setContentEncoding(L_C_TO_STRING(encoding));
}

const char *linphone_content_get_disposition(const LinphoneContent *content) {
	const auto &contentDisposition = Content::toCpp(content)->getContentDisposition();
	return L_STRING_TO_C(contentDisposition.asString());
}

void linphone_content_set_disposition(LinphoneContent *content, const char *disposition) {
	string strDisposition = L_C_TO_STRING(disposition);
	if (!strDisposition.empty()) {
		ContentDisposition contentDisposition = ContentDisposition(strDisposition);
		Content::toCpp(content)->setContentDisposition(contentDisposition);
	}
}

const char *linphone_content_get_name(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);

	if (c->isFile()) return L_STRING_TO_C(dynamic_cast<const FileContent *>(c)->getFileName());
	else if (c->isFileTransfer()) return L_STRING_TO_C(dynamic_cast<const FileTransferContent *>(c)->getFileName());

	return L_STRING_TO_C(c->getName());
}

void linphone_content_set_name(LinphoneContent *content, const char *name) {
	auto c = Content::toCpp(content);
	if (c->isFile()) dynamic_cast<FileContent *>(c)->setFileName(L_C_TO_STRING(name));
	else if (c->isFileTransfer()) dynamic_cast<FileTransferContent *>(c)->setFileName(L_C_TO_STRING(name));
	else c->setName(L_C_TO_STRING(name));
}

bool_t linphone_content_is_multipart(const LinphoneContent *content) {
	return Content::toCpp(content)->getContentType().isMultipart();
}

bctbx_list_t *linphone_content_get_parts(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	bctbx_list_t *parts = nullptr;
	SalBodyHandler *bodyHandler;
	if (!c->isDirty() && c->getBodyHandler() != nullptr) {
		bodyHandler = sal_body_handler_ref(c->getBodyHandler());
	} else {
		bodyHandler = sal_body_handler_from_content(content);
	}

	if (!sal_body_handler_is_multipart(bodyHandler)) {
		sal_body_handler_unref(bodyHandler);
		return parts;
	}

	const bctbx_list_t *sal_parts = sal_body_handler_get_parts(bodyHandler);
	auto it = (bctbx_list_t *)sal_parts;
	while (it != nullptr) {
		auto bh = (SalBodyHandler *)it->data;
		LinphoneContent *part = linphone_content_from_sal_body_handler(bh);
		parts = bctbx_list_append(parts, linphone_content_ref(part));
		linphone_content_unref(part);
		it = bctbx_list_next(it);
	}

	sal_body_handler_unref(bodyHandler);
	return parts;
}

LinphoneContent *linphone_content_get_part(const LinphoneContent *content, int idx) {
	const auto c = Content::toCpp(content);
	SalBodyHandler *bodyHandler;
	if (!c->isDirty() && c->getBodyHandler() != nullptr) {
		bodyHandler = sal_body_handler_ref(c->getBodyHandler());
	} else {
		bodyHandler = sal_body_handler_from_content(content);
	}

	if (!sal_body_handler_is_multipart(bodyHandler)) {
		sal_body_handler_unref(bodyHandler);
		return nullptr;
	}

	SalBodyHandler *partBodyHandler = sal_body_handler_get_part(bodyHandler, idx);
	LinphoneContent *result = linphone_content_from_sal_body_handler(partBodyHandler);
	sal_body_handler_unref(bodyHandler);
	return result;
}

LinphoneContent *
linphone_content_find_part_by_header(const LinphoneContent *content, const char *headerName, const char *headerValue) {
	const auto c = Content::toCpp(content);
	SalBodyHandler *bodyHandler;
	if (!c->isDirty() && c->getBodyHandler() != nullptr) {
		bodyHandler = sal_body_handler_ref(c->getBodyHandler());
	} else {
		bodyHandler = sal_body_handler_from_content(content);
	}

	if (!sal_body_handler_is_multipart(bodyHandler)) {
		sal_body_handler_unref(bodyHandler);
		return nullptr;
	}

	SalBodyHandler *partBodyHandler = sal_body_handler_find_part_by_header(bodyHandler, headerName, headerValue);
	LinphoneContent *result = linphone_content_from_sal_body_handler(partBodyHandler);
	sal_body_handler_unref(bodyHandler);
	return result;
}

const char *linphone_content_get_custom_header(const LinphoneContent *content, const char *headerName) {
	return L_STRING_TO_C(Content::toCpp(content)->getCustomHeader(L_C_TO_STRING(headerName)));
}

void linphone_content_add_custom_header(LinphoneContent *content, const char *header_name, const char *header_value) {
	Content::toCpp(content)->addHeader(L_C_TO_STRING(header_name), L_C_TO_STRING(header_value));
}

const char *linphone_content_get_key(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	if (c->isFileTransfer()) {
		auto ftc = dynamic_cast<const FileTransferContent *>(c);
		return ftc->getFileKey().data();
	}
	return nullptr;
}

size_t linphone_content_get_key_size(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	if (c->isFileTransfer()) {
		auto ftc = dynamic_cast<const FileTransferContent *>(c);
		return ftc->getFileKeySize();
	}
	return 0;
}

void linphone_content_set_key(LinphoneContent *content, const char *key, const size_t keyLength) {
	auto c = Content::toCpp(content);
	if (c->isFileTransfer()) {
		auto ftc = dynamic_cast<FileTransferContent *>(c);
		ftc->setFileKey(key, keyLength);
	}
}

const char *linphone_content_get_authTag(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	if (c->isFileTransfer()) {
		auto ftc = dynamic_cast<const FileTransferContent *>(c);
		return ftc->getFileAuthTag().data();
	}
	return nullptr;
}

size_t linphone_content_get_authTag_size(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	if (c->isFileTransfer()) {
		auto ftc = dynamic_cast<const FileTransferContent *>(c);
		return ftc->getFileAuthTagSize();
	}
	return 0;
}

void linphone_content_set_authTag(LinphoneContent *content, const char *tag, const size_t tagLength) {
	auto c = Content::toCpp(content);
	if (c->isFileTransfer()) {
		auto ftc = dynamic_cast<FileTransferContent *>(c);
		ftc->setFileAuthTag(tag, tagLength);
	}
}

const char *linphone_content_get_file_path(const LinphoneContent *content) {
	return L_STRING_TO_C(Content::toCpp(content)->getFilePath());
}

/* function deprecated on 07/01/2022 export_plain_file is more explicit */
char *linphone_content_get_plain_file_path(const LinphoneContent *content) {
	return linphone_content_export_plain_file(content);
}

char *linphone_content_export_plain_file(const LinphoneContent *content) {
	const Content *c = Content::toCpp(content);
	if (c->isFile()) {
		auto filePath = dynamic_cast<const FileContent *>(c)->exportPlainFile();
		return bctbx_strdup(L_STRING_TO_C(filePath));
	} else if (c->isFileTransfer()) {
		auto filePath = dynamic_cast<const FileTransferContent *>(c)->exportPlainFile();
		return bctbx_strdup(L_STRING_TO_C(filePath));
	}
	return nullptr;
}

void linphone_content_set_file_path(LinphoneContent *content, const char *file_path) {
	Content::toCpp(content)->setFilePath(L_C_TO_STRING(file_path));
}

int linphone_content_get_file_duration(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	if (c->isFile()) return dynamic_cast<const FileContent *>(c)->getFileDuration();
	else if (c->isFileTransfer()) return dynamic_cast<const FileTransferContent *>(c)->getFileDuration();
	return -1;
}

void linphone_content_set_file_duration(LinphoneContent *content, int duration) {
	auto c = Content::toCpp(content);
	if (c->isFile()) dynamic_cast<FileContent *>(c)->setFileDuration(duration);
	else if (c->isFileTransfer()) dynamic_cast<FileTransferContent *>(c)->setFileDuration(duration);
}

bool_t linphone_content_is_text(const LinphoneContent *content) {
	return Content::toCpp(content)->getContentType() == ContentType::PlainText;
}

bool_t linphone_content_is_voice_recording(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	if (c->isFile()) {
		return c->getContentType().strongEqual(ContentType::VoiceRecording);
	} else if (c->isFileTransfer()) {
		return dynamic_cast<const FileTransferContent *>(c)->getFileContentType().strongEqual(
		    ContentType::VoiceRecording);
	}
	return false;
}

bool_t linphone_content_is_icalendar(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	if (c->isFileTransfer()) {
		return dynamic_cast<const FileTransferContent *>(c)->getFileContentType().strongEqual(ContentType::Icalendar);
	} else {
		return c->getContentType().strongEqual(ContentType::Icalendar);
	}
}

bool_t linphone_content_is_file(const LinphoneContent *content) {
	return Content::toCpp(content)->isFile();
}

bool_t linphone_content_is_file_transfer(const LinphoneContent *content) {
	return Content::toCpp(content)->isFileTransfer();
}

bool_t linphone_content_is_file_encrypted(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	if (c->isFile()) {
		return dynamic_cast<const FileContent *>(c)->isEncrypted();
	} else if (c->isFileTransfer()) {
		return dynamic_cast<const FileTransferContent *>(c)->isEncrypted();
	}
	return FALSE;
}

time_t linphone_content_get_creation_timestamp(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	if (c->isFile()) {
		return dynamic_cast<const FileContent *>(c)->getCreationTimestamp();
	}
	return -1;
}

const char *linphone_content_get_related_chat_message_id(const LinphoneContent *content) {
	const auto c = Content::toCpp(content);
	return L_STRING_TO_C(c->getRelatedChatMessageId());
}

// =============================================================================
// Private functions.
// =============================================================================

static LinphoneContent *linphone_content_new_with_body_handler(const SalBodyHandler *bodyHandler, bool parseMultipart) {
	return Content::createCObject(bodyHandler, parseMultipart);
}

LinphoneContent *linphone_content_new(void) {
	return linphone_content_new_with_body_handler(nullptr, true);
}

LinphoneContent *linphone_content_copy(const LinphoneContent *ref) {
	return Content::toCpp(ref)->clone()->toC();
}

LinphoneContent *linphone_core_create_content(BCTBX_UNUSED(LinphoneCore *lc)) {
	return linphone_content_new();
}

// Crypto context is managed(allocated/freed) by the encryption function,
// so provide the address of field in the private structure.
void **linphone_content_get_cryptoContext_address(LinphoneContent *content) {
	return Content::toCpp(content)->getCryptoContextAddress();
}

LinphoneContent *linphone_content_from_sal_body_handler(const SalBodyHandler *bodyHandler, bool parseMultipart) {
	if (!bodyHandler) return nullptr;
	return linphone_content_new_with_body_handler(bodyHandler, parseMultipart);
}

SalBodyHandler *sal_body_handler_from_content(const LinphoneContent *content, bool parseMultipart) {
	if (!content) return nullptr;
	return Content::getBodyHandlerFromContent(*Content::toCpp(content), parseMultipart);
}
