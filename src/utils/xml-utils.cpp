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

LINPHONE_END_NAMESPACE
