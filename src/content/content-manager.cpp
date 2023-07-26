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

#include <belle-sip/belle-sip.h>

#include "c-wrapper/c-wrapper.h"
#include "content-disposition.h"
#include "content-manager.h"
#include "content-type.h"
#include "content/content.h"
#include "content/header/header-param.h"
#include "linphone/api/c-content.h"
#include "private_functions.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

list<Content> ContentManager::multipartToContentList(const Content &content) {
	SalBodyHandler *sbh = Content::getBodyHandlerFromContent(content);

	list<Content> contents;
	for (const belle_sip_list_t *parts = sal_body_handler_get_parts(sbh); parts; parts = parts->next) {
		auto part = (SalBodyHandler *)parts->data;
		contents.emplace_back(part, false);
	}

	sal_body_handler_unref(sbh);
	return contents;
}

Content
ContentManager::contentListToMultipart(const list<Content *> &contents, const string &boundary, bool encrypted) {
	belle_sip_multipart_body_handler_t *mpbh =
	    belle_sip_multipart_body_handler_new(nullptr, nullptr, nullptr, boundary.empty() ? nullptr : boundary.c_str());
	mpbh = (belle_sip_multipart_body_handler_t *)belle_sip_object_ref(mpbh);

	for (auto &content : contents) {
		SalBodyHandler *sbh = Content::getBodyHandlerFromContent(*content, false);
		belle_sip_multipart_body_handler_add_part(mpbh, BELLE_SIP_BODY_HANDLER(sbh));
	}

	auto sbh = (SalBodyHandler *)mpbh;
	sal_body_handler_set_type(sbh, ContentType::Multipart.getType().c_str());
	sal_body_handler_set_subtype(sbh, encrypted ? ContentType::Encrypted.getSubType().c_str()
	                                            : ContentType::Multipart.getSubType().c_str());
	sal_body_handler_set_content_type_parameter(sbh, "boundary", belle_sip_multipart_body_handler_get_boundary(mpbh));

	auto content = Content(sbh);
	belle_sip_object_unref(mpbh);

	return content;
}

Content ContentManager::contentListToMultipart(const list<shared_ptr<Content>> &contents,
                                               const string &boundary,
                                               bool encrypted) {
	list<Content *> contentsPtrs;
	for (const auto &c : contents)
		contentsPtrs.push_back(c.get());
	return contentListToMultipart(contentsPtrs, boundary, encrypted);
}

Content ContentManager::contentListToMultipart(const std::list<Content *> &contents, bool encrypted) {
	return contentListToMultipart(contents, "", encrypted);
}

Content ContentManager::contentListToMultipart(const list<shared_ptr<Content>> &contents, bool encrypted) {
	return contentListToMultipart(contents, "", encrypted);
}

Content ContentManager::contentListToMultipart(const std::list<Content *> &contents) {
	return contentListToMultipart(contents, "", false);
}

Content ContentManager::contentListToMultipart(const list<shared_ptr<Content>> &contents) {
	return contentListToMultipart(contents, "", false);
}

LINPHONE_END_NAMESPACE
