/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "linphone/account_creator.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"
#include "FlexiAPIClient.hh"

#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

LinphoneAccountCreatorStatus linphone_account_creator_is_account_exist_linphone_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->username && !creator->phone_number) {
		if (creator->cbs->is_account_exist_response_cb != NULL) {
			creator->cbs->is_account_exist_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	// Request it
	flexiAPIClient
		->accountInfo(string(creator->username).append("@").append(_get_domain(creator)))
		->then([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusAccountExist, response.body);
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusAccountNotExist, response.body)
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusUnexpectedError, response.body)
			}
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}