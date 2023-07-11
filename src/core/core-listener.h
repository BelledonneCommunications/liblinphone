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

#ifndef _L_CORE_LISTENER_H_
#define _L_CORE_LISTENER_H_

#include <bctoolbox/defs.h>

#include "linphone/types.h"

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Account;

class LINPHONE_PUBLIC CoreListener {
public:
	virtual ~CoreListener() = default;

	virtual void onGlobalStateChanged(BCTBX_UNUSED(LinphoneGlobalState state)) {
	}
	virtual void onNetworkReachable(BCTBX_UNUSED(bool sipNetworkReachable), BCTBX_UNUSED(bool mediaNetworkReachable)) {
	}
	virtual void onRegistrationStateChanged(BCTBX_UNUSED(LinphoneProxyConfig *cfg),
	                                        BCTBX_UNUSED(LinphoneRegistrationState state),
	                                        BCTBX_UNUSED(const std::string &message)) {
	}
	virtual void onAccountRegistrationStateChanged(BCTBX_UNUSED(std::shared_ptr<Account> account),
	                                               BCTBX_UNUSED(LinphoneRegistrationState state),
	                                               BCTBX_UNUSED(const std::string &message)) {
	}
	virtual void onCallStateChanged(BCTBX_UNUSED(LinphoneCall *call),
	                                BCTBX_UNUSED(LinphoneCallState state),
	                                BCTBX_UNUSED(const std::string &message)) {
	}
	virtual void onEnteringBackground() {
	}
	virtual void onEnteringForeground() {
	}
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CORE_LISTENER_H_
