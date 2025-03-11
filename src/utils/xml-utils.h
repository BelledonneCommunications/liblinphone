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

#ifndef _L_XML_UTILS_H_
#define _L_XML_UTILS_H_

#include <belle-sip/http-listener.h>

#include "http/http-client.h"
#include "linphone/types.h"
#include "xml/conference-info.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Core;
class Address;

namespace XmlUtils {
LinphoneMediaDirection mediaStatusToMediaDirection(Xsd::ConferenceInfo::MediaStatusType status);
Xsd::ConferenceInfo::MediaStatusType mediaDirectionToMediaStatus(LinphoneMediaDirection direction);
bool sendCcmpRequest(const std::shared_ptr<Core> &core,
                     const std::string &ccmpServerUrl,
                     const std::shared_ptr<const Address> &from,
                     const std::string &body,
                     const std::function<void(const HttpResponse &)> &listener);

} // namespace XmlUtils

LINPHONE_END_NAMESPACE

#endif // ifndef _L_XML_UTILS_H_
