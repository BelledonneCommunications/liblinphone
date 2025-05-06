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

#include <ctype.h>

#include <bctoolbox/defs.h>

#include "account-manager-services/account-manager-services.h"
#include "c-wrapper/c-wrapper.h"
#include "core/core-p.h"
#include "linphone/api/c-account-manager-services.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneAccountManagerServices *_linphone_account_manager_services_new(LinphoneCore *lc) {
	return AccountManagerServices::createCObject(lc);
}

LinphoneAccountManagerServices *linphone_account_manager_services_ref(LinphoneAccountManagerServices *ams) {
	AccountManagerServices::toCpp(ams)->ref();
	return ams;
}

void linphone_account_manager_services_unref(LinphoneAccountManagerServices *ams) {
	AccountManagerServices::toCpp(ams)->unref();
}

// =============================================================================

void linphone_account_manager_services_set_language(LinphoneAccountManagerServices *ams, const char *language) {
	if (language != nullptr) {
		AccountManagerServices::toCpp(ams)->setLanguage(L_C_TO_STRING(language));
	}
}

const char *linphone_account_manager_services_get_language(const LinphoneAccountManagerServices *ams) {
	return L_STRING_TO_C(AccountManagerServices::toCpp(ams)->getLanguage());
}

// =============================================================================

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_send_account_creation_token_by_push_request(
    LinphoneAccountManagerServices *ams, const char *pn_provider, const char *pn_param, const char *pn_prid) {
	auto request = AccountManagerServices::toCpp(ams)->createSendAccountCreationTokenByPushRequest(
	    L_C_TO_STRING(pn_provider), L_C_TO_STRING(pn_param), L_C_TO_STRING(pn_prid));
	request->ref();
	return request->toC();
}

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_send_account_recovery_token_by_push_request(
    LinphoneAccountManagerServices *ams, const char *pn_provider, const char *pn_param, const char *pn_prid) {
	auto request = AccountManagerServices::toCpp(ams)->createSendAccountRecoveryTokenByPushRequest(
	    L_C_TO_STRING(pn_provider), L_C_TO_STRING(pn_param), L_C_TO_STRING(pn_prid));
	request->ref();
	return request->toC();
}

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_account_creation_request_token_request(
    LinphoneAccountManagerServices *ams) {
	auto request = AccountManagerServices::toCpp(ams)->createGetAccountCreationRequestTokenRequest();
	request->ref();
	return request->toC();
}

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_account_creation_token_from_request_token_request(
    LinphoneAccountManagerServices *ams, const char *request_token) {
	auto request =
	    AccountManagerServices::toCpp(ams)->createGetAccountCreationTokenFromRequestTokenRequest(request_token);
	request->ref();
	return request->toC();
}

// =============================================================================

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_new_account_using_token_request(LinphoneAccountManagerServices *ams,
                                                                         const char *username,
                                                                         const char *password,
                                                                         const char *algorithm,
                                                                         const char *token) {
	auto request = AccountManagerServices::toCpp(ams)->createNewAccountUsingTokenRequest(
	    L_C_TO_STRING(username), L_C_TO_STRING(password), L_C_TO_STRING(algorithm), L_C_TO_STRING(token));
	request->ref();
	return request->toC();
}

// =============================================================================

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_send_phone_number_linking_code_by_sms_request(
    LinphoneAccountManagerServices *ams, const LinphoneAddress *sip_identity, const char *phone_number) {
	auto request = AccountManagerServices::toCpp(ams)->createSendPhoneNumberLinkingCodeBySmsRequest(
	    Address::getSharedFromThis(sip_identity), L_C_TO_STRING(phone_number));
	request->ref();
	return request->toC();
}

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_link_phone_number_to_account_using_code_request(
    LinphoneAccountManagerServices *ams, const LinphoneAddress *sip_identity, const char *code) {
	auto request = AccountManagerServices::toCpp(ams)->createLinkPhoneNumberToAccountUsingCodeRequest(
	    Address::getSharedFromThis(sip_identity), L_C_TO_STRING(code));
	request->ref();
	return request->toC();
}

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_send_email_linking_code_by_email_request(LinphoneAccountManagerServices *ams,
                                                                                  const LinphoneAddress *sip_identity,
                                                                                  const char *email_address) {
	auto request = AccountManagerServices::toCpp(ams)->createSendEmailLinkingCodeByEmailRequest(
	    Address::getSharedFromThis(sip_identity), L_C_TO_STRING(email_address));
	request->ref();
	return request->toC();
}

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_link_email_to_account_using_code_request(LinphoneAccountManagerServices *ams,
                                                                                  const LinphoneAddress *sip_identity,
                                                                                  const char *code) {
	auto request = AccountManagerServices::toCpp(ams)->createLinkEmailToAccountUsingCodeRequest(
	    Address::getSharedFromThis(sip_identity), L_C_TO_STRING(code));
	request->ref();
	return request->toC();
}

// =============================================================================

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_devices_list_request(LinphoneAccountManagerServices *ams,
                                                                  const LinphoneAddress *sip_identity) {
	auto request =
	    AccountManagerServices::toCpp(ams)->createGetDevicesListRequest(Address::getSharedFromThis(sip_identity));
	request->ref();
	return request->toC();
}

LinphoneAccountManagerServicesRequest *linphone_account_manager_services_create_delete_device_request(
    LinphoneAccountManagerServices *ams, const LinphoneAddress *sip_identity, const LinphoneAccountDevice *device) {
	auto request = AccountManagerServices::toCpp(ams)->createDeleteDeviceRequest(
	    Address::getSharedFromThis(sip_identity), AccountDevice::getSharedFromThis(device));
	request->ref();
	return request->toC();
}

// =============================================================================

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_account_creation_token_as_admin_request(
    LinphoneAccountManagerServices *ams) {
	auto request = AccountManagerServices::toCpp(ams)->createGetAccountCreationTokenAsAdminRequest();
	request->ref();
	return request->toC();
}

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_account_info_as_admin_request(LinphoneAccountManagerServices *ams,
                                                                           int account_id) {
	auto request = AccountManagerServices::toCpp(ams)->createGetAccountInfoAsAdminRequest(account_id);
	request->ref();
	return request->toC();
}

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_delete_account_as_admin_request(LinphoneAccountManagerServices *ams,
                                                                         int account_id) {
	auto request = AccountManagerServices::toCpp(ams)->createDeleteAccountAsAdminRequest(account_id);
	request->ref();
	return request->toC();
}