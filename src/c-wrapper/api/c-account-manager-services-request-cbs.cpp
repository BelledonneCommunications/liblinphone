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

#include "linphone/api/c-account-manager-services-request-cbs.h"
#include "account-manager-services/account-manager-services-request.h"
#include "c-wrapper/c-wrapper.h"
#include "private_functions.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneAccountManagerServicesRequestCbs *_linphone_account_manager_services_request_cbs_new(void) {
	return AccountManagerServicesRequestCbs::createCObject();
}

LinphoneAccountManagerServicesRequestCbs *
linphone_account_manager_services_request_cbs_ref(LinphoneAccountManagerServicesRequestCbs *cbs) {
	AccountManagerServicesRequestCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_account_manager_services_request_cbs_unref(LinphoneAccountManagerServicesRequestCbs *cbs) {
	AccountManagerServicesRequestCbs::toCpp(cbs)->unref();
}

void *linphone_account_manager_services_request_cbs_get_user_data(const LinphoneAccountManagerServicesRequestCbs *cbs) {
	return AccountManagerServicesRequestCbs::toCpp(cbs)->getUserData();
}

void linphone_account_manager_services_request_cbs_set_user_data(LinphoneAccountManagerServicesRequestCbs *cbs,
                                                                 void *ud) {
	AccountManagerServicesRequestCbs::toCpp(cbs)->setUserData(ud);
}

// =============================================================================

LinphoneAccountManagerServicesRequestCbsOnSuccessfulRequestCb
linphone_account_manager_services_request_cbs_get_request_successful(
    const LinphoneAccountManagerServicesRequestCbs *cbs) {
	return AccountManagerServicesRequestCbs::toCpp(cbs)->getRequestSuccessful();
}

void linphone_account_manager_services_request_cbs_set_request_successful(
    LinphoneAccountManagerServicesRequestCbs *cbs, LinphoneAccountManagerServicesRequestCbsOnSuccessfulRequestCb cb) {
	AccountManagerServicesRequestCbs::toCpp(cbs)->setRequestSuccessful(cb);
}

LinphoneAccountManagerServicesRequestCbsOnRequestErrorCb
linphone_account_manager_services_request_cbs_get_request_error(const LinphoneAccountManagerServicesRequestCbs *cbs) {
	return AccountManagerServicesRequestCbs::toCpp(cbs)->getRequestError();
}

void linphone_account_manager_services_request_cbs_set_request_error(
    LinphoneAccountManagerServicesRequestCbs *cbs, LinphoneAccountManagerServicesRequestCbsOnRequestErrorCb cb) {
	AccountManagerServicesRequestCbs::toCpp(cbs)->setRequestError(cb);
}

LinphoneAccountManagerServicesRequestCbsOnDevicesListFetchedCb
linphone_account_manager_services_request_cbs_get_devices_list_fetched(
    const LinphoneAccountManagerServicesRequestCbs *cbs) {
	return AccountManagerServicesRequestCbs::toCpp(cbs)->getDevicesListFetched();
}

void linphone_account_manager_services_request_cbs_set_devices_list_fetched(
    LinphoneAccountManagerServicesRequestCbs *cbs, LinphoneAccountManagerServicesRequestCbsOnDevicesListFetchedCb cb) {
	AccountManagerServicesRequestCbs::toCpp(cbs)->setDevicesListFetched(cb);
}