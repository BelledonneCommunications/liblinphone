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

#include "utils/xml-utils.h"
#include "address/address.h"
#include "core/core.h"
#include "http/http-client.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;

MediaStatusType XmlUtils::mediaDirectionToMediaStatus(LinphoneMediaDirection direction) {
	switch (direction) {
		case LinphoneMediaDirectionInactive:
			return MediaStatusType::inactive;
		case LinphoneMediaDirectionSendOnly:
			return MediaStatusType::sendonly;
		case LinphoneMediaDirectionRecvOnly:
			return MediaStatusType::recvonly;
		case LinphoneMediaDirectionSendRecv:
			return MediaStatusType::sendrecv;
		case LinphoneMediaDirectionInvalid:
			lError() << "LinphoneMediaDirectionInvalid shall not be used";
			return MediaStatusType::inactive;
	}
	return MediaStatusType::sendrecv;
}

bool XmlUtils::sendCcmpRequest(const std::shared_ptr<Core> core,
                               const std::string ccmpServerUrl,
                               const std::shared_ptr<Address> from,
                               const std::string body,
                               belle_http_request_listener_t *listener) {
	belle_http_request_t *req =
	    belle_http_request_create("POST", belle_generic_uri_parse(ccmpServerUrl.c_str()),
	                              belle_sip_header_content_type_create("application", "ccmp+xml"), nullptr, nullptr);
	if (!req) {
		lError() << "Cannot create a http request from config url [" << ccmpServerUrl << "]";
		return false;
	}

	const auto fromStr = from->asStringUriOnly();
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(req), belle_http_header_create("From", fromStr.c_str()));

	if (!body.empty()) {
		belle_sip_message_set_body(BELLE_SIP_MESSAGE(req), body.c_str(), body.size());
	}

	// Set the same User-Agent header as for the SAL
	belle_sip_header_user_agent_t *userAgentHeader = belle_sip_header_user_agent_new();
	belle_sip_object_ref(userAgentHeader);
	belle_sip_header_user_agent_set_products(userAgentHeader, nullptr);
	const auto cCore = core->getCCore();
	belle_sip_header_user_agent_add_product(userAgentHeader, linphone_core_get_user_agent(cCore));
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(req), BELLE_SIP_HEADER(userAgentHeader));

	belle_http_provider_send_request(core->getHttpClient().getProvider(), req, listener);
	belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "listener", listener, belle_sip_object_unref);
	belle_sip_object_unref(userAgentHeader);
	return true;
}

LinphoneMediaDirection XmlUtils::mediaStatusToMediaDirection(MediaStatusType status) {
	switch (status) {
		case MediaStatusType::inactive:
			return LinphoneMediaDirectionInactive;
		case MediaStatusType::sendonly:
			return LinphoneMediaDirectionSendOnly;
		case MediaStatusType::recvonly:
			return LinphoneMediaDirectionRecvOnly;
		case MediaStatusType::sendrecv:
			return LinphoneMediaDirectionSendRecv;
	}
	return LinphoneMediaDirectionSendRecv;
}

LINPHONE_END_NAMESPACE
