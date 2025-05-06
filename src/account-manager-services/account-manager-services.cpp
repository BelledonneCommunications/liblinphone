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

#include "account-manager-services.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/core.h"
#include "linphone/enums/c-enums.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

AccountManagerServices::AccountManagerServices(LinphoneCore *lc)
    : CoreAccessor(lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr) {
	mAccountManagerUrl = linphone_core_get_account_creator_url(lc);
	if (mAccountManagerUrl.compare(mAccountManagerUrl.length() - 1, 1, "/") != 0) {
		lInfo() << "[Account Manager Services] Appending '/' to URL [" << mAccountManagerUrl << "]";
		mAccountManagerUrl = mAccountManagerUrl.append("/");
	}

	mTesterEnv = !!linphone_config_get_int(linphone_core_get_config(lc), "tester", "test_env", FALSE);
}

AccountManagerServices::~AccountManagerServices() {
}

const std::string &AccountManagerServices::getLanguage() const {
	return mLanguage;
}

void AccountManagerServices::setLanguage(const std::string &language) {
	mLanguage = language;
}

// =============================================================================

shared_ptr<AccountManagerServicesRequest> AccountManagerServices::createRequest(
    LinphoneAccountManagerServicesRequestType type, Json::Value params, const string &from) {
	string path = "";
	string httpType = "POST";
	switch (type) {
		case LinphoneAccountManagerServicesRequestTypeSendAccountCreationTokenByPush:
			path = "account_creation_tokens/send-by-push";
			break;
		case LinphoneAccountManagerServicesRequestTypeSendAccountRecoveryTokenByPush:
			path = "account_recovery_tokens/send-by-push";
			break;
		case LinphoneAccountManagerServicesRequestTypeAccountCreationRequestToken:
			path = "account_creation_request_tokens";
			break;
		case LinphoneAccountManagerServicesRequestTypeAccountCreationTokenFromAccountCreationRequestToken:
			path = "account_creation_tokens/using-account-creation-request-token";
			break;
		case LinphoneAccountManagerServicesRequestTypeCreateAccountUsingToken:
			path = "accounts/with-account-creation-token";
			break;
		case LinphoneAccountManagerServicesRequestTypeSendPhoneNumberLinkingCodeBySms:
			path = "accounts/me/phone/request";
			break;
		case LinphoneAccountManagerServicesRequestTypeLinkPhoneNumberUsingCode:
			path = "accounts/me/phone";
			break;
		case LinphoneAccountManagerServicesRequestTypeSendEmailLinkingCodeByEmail:
			path = "accounts/me/email/request";
			break;
		case LinphoneAccountManagerServicesRequestTypeLinkEmailUsingCode:
			path = "accounts/me/email";
			break;
		case LinphoneAccountManagerServicesRequestTypeGetDevicesList:
			path = "accounts/me/devices";
			httpType = "GET";
			break;
		case LinphoneAccountManagerServicesRequestTypeDeleteDevice:
			path = string("accounts/me/devices/" + params["device_uuid"].asString());
			httpType = "DELETE";
			break;
		case LinphoneAccountManagerServicesRequestTypeGetCreationTokenAsAdmin:
			path = "account_creation_tokens";
			break;
		case LinphoneAccountManagerServicesRequestTypeGetAccountInfoAsAdmin:
			path = string("accounts/" + params["account_id"].asString());
			httpType = "GET";
			break;
		case LinphoneAccountManagerServicesRequestTypeDeleteAccountAsAdmin:
			path = string("accounts/" + params["account_id"].asString());
			httpType = "DELETE";
			break;
	}

	string url = string(mAccountManagerUrl + path);
	auto request =
	    (new AccountManagerServicesRequest(getCore(), type, url, httpType, params, mTesterEnv))->toSharedPtr();
	if (!from.empty()) {
		request->setFrom(from);
	}
	if (!mLanguage.empty()) {
		request->setLanguage(mLanguage);
	}
	return request;
}

// =============================================================================

shared_ptr<AccountManagerServicesRequest> AccountManagerServices::createSendAccountCreationTokenByPushRequest(
    const string &provider, const string &param, const string &prid) {
	LinphoneAccountManagerServicesRequestType type =
	    LinphoneAccountManagerServicesRequestTypeSendAccountCreationTokenByPush;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << provider << ", " << param << ", " << prid
	         << ")";
	Json::Value params;
	params["pn_provider"] = provider;
	params["pn_param"] = param;
	params["pn_prid"] = prid;

	return createRequest(type, params);
}

shared_ptr<AccountManagerServicesRequest> AccountManagerServices::createSendAccountRecoveryTokenByPushRequest(
    const string &provider, const string &param, const string &prid) {
	LinphoneAccountManagerServicesRequestType type =
	    LinphoneAccountManagerServicesRequestTypeSendAccountRecoveryTokenByPush;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << provider << ", " << param << ", " << prid
	         << ")";
	Json::Value params;
	params["pn_provider"] = provider;
	params["pn_param"] = param;
	params["pn_prid"] = prid;

	return createRequest(type, params);
}

shared_ptr<AccountManagerServicesRequest> AccountManagerServices::createGetAccountCreationRequestTokenRequest() {
	LinphoneAccountManagerServicesRequestType type =
	    LinphoneAccountManagerServicesRequestTypeAccountCreationRequestToken;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "]";
	Json::Value params;

	return createRequest(type, params);
}

shared_ptr<AccountManagerServicesRequest>
AccountManagerServices::createGetAccountCreationTokenFromRequestTokenRequest(const string &requestToken) {
	LinphoneAccountManagerServicesRequestType type =
	    LinphoneAccountManagerServicesRequestTypeAccountCreationTokenFromAccountCreationRequestToken;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << requestToken << ")";
	Json::Value params;
	params["account_creation_request_token"] = requestToken;

	return createRequest(type, params);
}

// =============================================================================

shared_ptr<AccountManagerServicesRequest> AccountManagerServices::createNewAccountUsingTokenRequest(
    const string &username, const string &password, const string &algorithm, const string &token) {
	LinphoneAccountManagerServicesRequestType type = LinphoneAccountManagerServicesRequestTypeCreateAccountUsingToken;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << username << ", ******, " << algorithm
	         << ", " << token << ")";

	Json::Value params;
	params["username"] = username;
	params["password"] = password;
	params["algorithm"] = algorithm;
	params["account_creation_token"] = token;

	return createRequest(type, params);
}

// =============================================================================

shared_ptr<AccountManagerServicesRequest>
AccountManagerServices::createSendPhoneNumberLinkingCodeBySmsRequest(const shared_ptr<const Address> &sipIdentity,
                                                                     const string &phoneNumber) {
	string identity = sipIdentity->asStringUriOnly();
	LinphoneAccountManagerServicesRequestType type =
	    LinphoneAccountManagerServicesRequestTypeSendPhoneNumberLinkingCodeBySms;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << identity << ", " << phoneNumber << ")";

	Json::Value params;
	params["phone"] = phoneNumber;

	return createRequest(type, params, identity);
}

shared_ptr<AccountManagerServicesRequest>
AccountManagerServices::createLinkPhoneNumberToAccountUsingCodeRequest(const shared_ptr<const Address> &sipIdentity,
                                                                       const string &code) {
	string identity = sipIdentity->asStringUriOnly();
	LinphoneAccountManagerServicesRequestType type = LinphoneAccountManagerServicesRequestTypeLinkPhoneNumberUsingCode;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << identity << ", " << code << ")";

	Json::Value params;
	params["code"] = code;

	return createRequest(type, params, identity);
}

// =============================================================================

shared_ptr<AccountManagerServicesRequest>
AccountManagerServices::createSendEmailLinkingCodeByEmailRequest(const shared_ptr<const Address> &sipIdentity,
                                                                 const string &emailAddress) {
	string identity = sipIdentity->asStringUriOnly();
	LinphoneAccountManagerServicesRequestType type =
	    LinphoneAccountManagerServicesRequestTypeSendEmailLinkingCodeByEmail;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << identity << ", " << emailAddress << ")";

	Json::Value params;
	params["email"] = emailAddress;

	return createRequest(type, params, identity);
}

shared_ptr<AccountManagerServicesRequest>
AccountManagerServices::createLinkEmailToAccountUsingCodeRequest(const shared_ptr<const Address> &sipIdentity,
                                                                 const string &code) {
	string identity = sipIdentity->asStringUriOnly();
	LinphoneAccountManagerServicesRequestType type = LinphoneAccountManagerServicesRequestTypeLinkEmailUsingCode;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << identity << ", " << code << ")";

	Json::Value params;
	params["code"] = code;

	return createRequest(type, params, identity);
}

// =============================================================================

shared_ptr<AccountManagerServicesRequest>
AccountManagerServices::createGetDevicesListRequest(const shared_ptr<const Address> &sipIdentity) {
	string identity = sipIdentity->asStringUriOnly();
	LinphoneAccountManagerServicesRequestType type = LinphoneAccountManagerServicesRequestTypeGetDevicesList;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << identity << ")";
	Json::Value params;

	return createRequest(type, params, identity);
}

shared_ptr<AccountManagerServicesRequest>
AccountManagerServices::createDeleteDeviceRequest(const shared_ptr<const Address> &sipIdentity,
                                                  const shared_ptr<const AccountDevice> &device) {
	string identity = sipIdentity->asStringUriOnly();
	LinphoneAccountManagerServicesRequestType type = LinphoneAccountManagerServicesRequestTypeDeleteDevice;
	lDebug() << "[Account Manager Services] Calling [" << __func__ << "](" << identity << ", " << device->toString()
	         << ")";
	Json::Value params;
	params["device_uuid"] = device->getUUID();

	return createRequest(type, params, identity);
}

// =============================================================================

shared_ptr<AccountManagerServicesRequest> AccountManagerServices::createGetAccountCreationTokenAsAdminRequest() {
	LinphoneAccountManagerServicesRequestType type = LinphoneAccountManagerServicesRequestTypeGetCreationTokenAsAdmin;
	Json::Value params;

	return createRequest(type, params);
}

shared_ptr<AccountManagerServicesRequest> AccountManagerServices::createGetAccountInfoAsAdminRequest(int accountId) {
	LinphoneAccountManagerServicesRequestType type = LinphoneAccountManagerServicesRequestTypeGetAccountInfoAsAdmin;
	Json::Value params;
	params["account_id"] = accountId;

	return createRequest(type, params);
}

shared_ptr<AccountManagerServicesRequest> AccountManagerServices::createDeleteAccountAsAdminRequest(int accountId) {
	LinphoneAccountManagerServicesRequestType type = LinphoneAccountManagerServicesRequestTypeDeleteAccountAsAdmin;
	Json::Value params;
	params["account_id"] = accountId;

	return createRequest(type, params);
}

LINPHONE_END_NAMESPACE
