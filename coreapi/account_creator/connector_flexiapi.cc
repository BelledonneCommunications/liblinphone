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

#include "FlexiAPIClient.hh"
#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"

#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

static void flexi_api_error_default_handler(LinphoneAccountCreator *creator, FlexiAPIClient::Response response) {
	if (response.code == 404) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusAccountNotExist,
										response.body)
	} else if (response.code == 422) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusMissingArguments,
										response.body)
	} else {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusUnexpectedError,
										response.body)
	}
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_exist_linphone_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->username && !creator->phone_number) {
		if (creator->cbs->is_account_exist_response_cb != NULL) {
			creator->cbs->is_account_exist_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
													   "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusMissingArguments,
										"Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	flexiAPIClient->accountInfo(string(creator->username).append("@").append(_get_domain(creator)))
		->then([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusAccountExist,
											response.body);
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			flexi_api_error_default_handler(creator, response);
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_delete_account_linphone_flexiapi(LinphoneAccountCreator *creator) {
	if ((!creator->username && !creator->phone_number) || !creator->password || !creator->proxy_cfg) {
		if (creator->cbs->delete_account_response_cb != NULL) {
			creator->cbs->delete_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
													 "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, delete_account, creator, LinphoneAccountCreatorStatusMissingArguments,
										"Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	flexiAPIClient->accountDelete()
		->then([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_exist, creator, LinphoneAccountCreatorStatusRequestOk,
											response.body);
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			flexi_api_error_default_handler(creator, response);
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_email_account_linphone_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->activation_code || !creator->username) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
														   "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, activate_account, creator, LinphoneAccountCreatorStatusMissingArguments,
										"Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	flexiAPIClient
		->accountActivateEmail(string(creator->username).append("@").append(_get_domain(creator)),
							   creator->activation_code)
		->then([](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			return LinphoneAccountCreatorStatusAccountActivated;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			flexi_api_error_default_handler(creator, response);
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_activated_linphone_flexiapi(LinphoneAccountCreator *creator) {
	char *identity = _get_identity(creator);
	if (!identity) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
														   "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_activated, creator,
										LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	flexiAPIClient->accountInfo(string(creator->username).append("@").append(_get_domain(creator)))
		->then([](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			if (response.json()["activated"].asBool()) {
				return LinphoneAccountCreatorStatusAccountActivated;
			}

			return LinphoneAccountCreatorStatusAccountNotActivated;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			flexi_api_error_default_handler(creator, response);
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	ms_free(identity);
	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_link_phone_number_with_account_linphone_flexiapi(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->phone_number || !creator->username) {
		if (creator->cbs->link_account_response_cb != NULL) {
			creator->cbs->link_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, link_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	flexiAPIClient->accountPhoneChangeRequest(creator->phone_number)
		->then([](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			flexi_api_error_default_handler(creator, response);
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestFailed;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_phone_number_link_linphone_flexiapi(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->phone_number || !creator->username || !creator->activation_code || (!creator->password && !creator->ha1) || !_get_domain(creator)) {
		if (creator->cbs->activate_alias_response_cb != NULL) {
			creator->cbs->activate_alias_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, activate_alias, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	flexiAPIClient->accountPhoneChange(creator->activation_code)
		->then([](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			flexi_api_error_default_handler(creator, response);
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestFailed;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_phone_account_linphone_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->phone_number || !creator->activation_code) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, activate_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	flexiAPIClient
		->accountActivatePhone(string(creator->username).append("@").append(_get_domain(creator)),
							   creator->activation_code)
		->then([](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			return LinphoneAccountCreatorStatusAccountActivated;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			flexi_api_error_default_handler(creator, response);
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestFailed;
}

LinphoneAccountCreatorStatus linphone_account_creator_update_password_linphone_flexiapi(LinphoneAccountCreator *creator) {
	char *identity = _get_identity(creator);
	const char *new_pwd = (const char *)linphone_account_creator_get_user_data(creator);
	if (!identity || ((!creator->username && !creator->phone_number) || !_get_domain(creator) ||
					  (!creator->password && !creator->ha1) || !new_pwd)) {
		if (creator->cbs->update_account_response_cb != NULL) {
			creator->cbs->update_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
													 "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, update_account, creator, LinphoneAccountCreatorStatusMissingArguments,
										"Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}
	bctbx_free(identity);

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	flexiAPIClient->accountPasswordChange(creator->algorithm, creator->password, new_pwd)
		->then([](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			flexi_api_error_default_handler(creator, response);
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_linked_linphone_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->username || !_get_domain(creator)) {
		if (creator->cbs->is_account_linked_response_cb != NULL) {
			creator->cbs->is_account_linked_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(Status, is_account_linked, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = new FlexiAPIClient(creator->core);

	flexiAPIClient->me()
		->then([](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			if (!response.json()["phone"].empty()) {
				return LinphoneAccountCreatorStatusAccountLinked;
			}

			return LinphoneAccountCreatorStatusAccountNotLinked;
		})
		->error([creator](FlexiAPIClient::Response response) -> LinphoneAccountCreatorStatus {
			flexi_api_error_default_handler(creator, response);
			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestFailed;
}