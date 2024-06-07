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

#include "account-manager-services/account-manager-services-request.h"
#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-account-manager-services-request.h"
#include "linphone/wrapper_utils.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_request_ref(LinphoneAccountManagerServicesRequest *request) {
	AccountManagerServicesRequest::toCpp(request)->ref();
	return request;
}

void linphone_account_manager_services_request_unref(LinphoneAccountManagerServicesRequest *request) {
	AccountManagerServicesRequest::toCpp(request)->unref();
}

// =============================================================================

void linphone_account_manager_services_request_submit(LinphoneAccountManagerServicesRequest *request) {
	AccountManagerServicesRequest::toCpp(request)->submit();
}

LinphoneAccountManagerServicesRequestType
linphone_account_manager_services_request_get_type(const LinphoneAccountManagerServicesRequest *request) {
	return AccountManagerServicesRequest::toCpp(request)->getType();
}

// =============================================================================

void linphone_account_manager_services_request_add_callbacks(LinphoneAccountManagerServicesRequest *request,
                                                             LinphoneAccountManagerServicesRequestCbs *cbs) {
	AccountManagerServicesRequest::toCpp(request)->addCallbacks(
	    AccountManagerServicesRequestCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_account_manager_services_request_remove_callbacks(LinphoneAccountManagerServicesRequest *request,
                                                                LinphoneAccountManagerServicesRequestCbs *cbs) {
	AccountManagerServicesRequest::toCpp(request)->removeCallbacks(
	    AccountManagerServicesRequestCbs::toCpp(cbs)->getSharedFromThis());
}

LinphoneAccountManagerServicesRequestCbs *
linphone_account_manager_services_request_get_current_callbacks(const LinphoneAccountManagerServicesRequest *request) {
	return AccountManagerServicesRequest::toCpp(request)->getCurrentCallbacks()->toC();
}

void linphone_account_manager_services_request_set_current_callbacks(LinphoneAccountManagerServicesRequest *request,
                                                                     LinphoneAccountManagerServicesRequestCbs *cbs) {
	AccountManagerServicesRequest::toCpp(request)->setCurrentCallbacks(
	    AccountManagerServicesRequestCbs::toCpp(cbs)->getSharedFromThis());
}

const bctbx_list_t *
linphone_account_manager_services_request_get_callbacks_list(const LinphoneAccountManagerServicesRequest *request) {
	return AccountManagerServicesRequest::toCpp(request)->getCCallbacksList();
}

// =============================================================================