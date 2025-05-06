/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef _L_ACCOUNT_MANAGER_SERVICES_H_
#define _L_ACCOUNT_MANAGER_SERVICES_H_

#include <json/json.h>

#include "account-manager-services-request.h"
#include "belle-sip/object++.hh"
#include "c-wrapper/c-wrapper.h"
#include "core/core-accessor.h"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AccountManagerServices : public bellesip::HybridObject<LinphoneAccountManagerServices, AccountManagerServices>,
                               public CoreAccessor {

public:
	AccountManagerServices(LinphoneCore *lc);
	AccountManagerServices(const AccountManagerServices &other) = delete;
	virtual ~AccountManagerServices();

	const std::string &getLanguage() const;
	void setLanguage(const std::string &language);

	std::shared_ptr<AccountManagerServicesRequest> createSendAccountCreationTokenByPushRequest(
	    const std::string &provider, const std::string &param, const std::string &prid);
	std::shared_ptr<AccountManagerServicesRequest> createSendAccountRecoveryTokenByPushRequest(
	    const std::string &provider, const std::string &param, const std::string &prid);
	std::shared_ptr<AccountManagerServicesRequest> createGetAccountCreationRequestTokenRequest();
	std::shared_ptr<AccountManagerServicesRequest>
	createGetAccountCreationTokenFromRequestTokenRequest(const std::string &requestToken);

	std::shared_ptr<AccountManagerServicesRequest> createNewAccountUsingTokenRequest(const std::string &username,
	                                                                                 const std::string &password,
	                                                                                 const std::string &algorihtm,
	                                                                                 const std::string &token);

	std::shared_ptr<AccountManagerServicesRequest>
	createSendPhoneNumberLinkingCodeBySmsRequest(const std::shared_ptr<const Address> &sipIdentity,
	                                             const std::string &phoneNumber);
	std::shared_ptr<AccountManagerServicesRequest>
	createLinkPhoneNumberToAccountUsingCodeRequest(const std::shared_ptr<const Address> &sipIdentity,
	                                               const std::string &code);

	std::shared_ptr<AccountManagerServicesRequest>
	createSendEmailLinkingCodeByEmailRequest(const std::shared_ptr<const Address> &sipIdentity,
	                                         const std::string &email);
	std::shared_ptr<AccountManagerServicesRequest>
	createLinkEmailToAccountUsingCodeRequest(const std::shared_ptr<const Address> &sipIdentity,
	                                         const std::string &code);

	std::shared_ptr<AccountManagerServicesRequest>
	createGetDevicesListRequest(const std::shared_ptr<const Address> &sipIdentity);
	std::shared_ptr<AccountManagerServicesRequest>
	createDeleteDeviceRequest(const std::shared_ptr<const Address> &sipIdentity,
	                          const std::shared_ptr<const AccountDevice> &device);

	std::shared_ptr<AccountManagerServicesRequest> createGetAccountCreationTokenAsAdminRequest();
	std::shared_ptr<AccountManagerServicesRequest> createGetAccountInfoAsAdminRequest(int accountId);
	std::shared_ptr<AccountManagerServicesRequest> createDeleteAccountAsAdminRequest(int accountId);

private:
	std::shared_ptr<AccountManagerServicesRequest>
	createRequest(LinphoneAccountManagerServicesRequestType type, Json::Value params, const std::string &from = "");

	std::string mAccountManagerUrl;
	std::string mLanguage;
	bool mTesterEnv;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ACCOUNT_MANAGER_SERVICES_H_
