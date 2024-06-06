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

#include <string>

#include "content/content-type.h"
#include "content/content.h"
#include "content/header/header-param.h"
#include "encryption-engine.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

bool LimeX3dhUtils::isMessageEncrypted(const Content &internalContent) {
	const ContentType &incomingContentType = internalContent.getContentType();
	ContentType expectedContentType = ContentType::Encrypted;

	if (incomingContentType == expectedContentType) {
		string protocol = incomingContentType.getParameter("protocol").getValue();
		if (protocol == "\"application/lime\"") {
			return true;
		} else if (protocol.empty()) {
			lWarning() << "Accepting possible legacy lime message.";
			return true;
		}
	}

	return false;
}

LINPHONE_END_NAMESPACE
