/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

bool XmlUtils::sendCcmpRequest(const std::shared_ptr<Core> &core,
                               const std::string &ccmpServerUrl,
                               const std::shared_ptr<const Address> &from,
                               const std::string &body,
                               const std::function<void(const HttpResponse &)> &listener) {
	try {
		auto &httpClient = core->getHttpClient();
		auto &httpRequest = httpClient.createRequest("POST", ccmpServerUrl);
		httpRequest.addHeader("From", from->asStringUriOnly());

		if (!body.empty()) {
			auto content = Content(ContentType("application/ccmp+xml"), body);
			httpRequest.setBody(content);
		}

		httpRequest.execute([listener](const HttpResponse &response) -> void { listener(response); });
	} catch (const std::exception &e) {
		lError() << __func__ << ": Error while sending CCMP request : " << e.what();
		return false;
	}

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
