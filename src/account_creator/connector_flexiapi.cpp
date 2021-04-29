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

LinphoneAccountCreatorStatus linphone_account_creator_is_account_exist_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->username && !creator->phone_number) {
		if (creator->cbs->is_account_exist_response_cb != NULL) {
			creator->cbs->is_account_exist_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
													   "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_exist, creator, LinphoneAccountCreatorStatusMissingArguments,
										"Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountInfo(string(creator->username).append("@").append(creator->domain))
		->then([creator](FlexiAPIClient::Response response) {
			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_exist, creator, LinphoneAccountCreatorStatusAccountExist,
											response.body.c_str());
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) {
						if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_exist, creator, LinphoneAccountCreatorStatusAccountNotExist,
												response.body.c_str())
			} else if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_exist, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_exist, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_delete_account_flexiapi(LinphoneAccountCreator *creator) {
	if ((!creator->username && !creator->phone_number) || !creator->password || !creator->proxy_cfg) {
		if (creator->cbs->delete_account_response_cb != NULL) {
			creator->cbs->delete_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
													 "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(delete_account, creator, LinphoneAccountCreatorStatusMissingArguments,
										"Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountDelete()
		->then([creator](FlexiAPIClient::Response response) {
			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(delete_account, creator, LinphoneAccountCreatorStatusRequestOk,
											response.body.c_str());
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(delete_account, creator, LinphoneAccountCreatorStatusAccountNotExist,
												response.body.c_str())
			} else if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(delete_account, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(delete_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_email_account_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->activation_code || !creator->username) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
														   "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusMissingArguments,
										"Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient
		->accountActivateEmail(string(creator->username).append("@").append(_get_domain(creator)),
							   creator->activation_code)
		->then([creator](FlexiAPIClient::Response response) {
			if (creator->cbs->activate_account_response_cb != NULL) {
				creator->cbs->activate_account_response_cb(creator, LinphoneAccountCreatorStatusRequestFailed, response.body.c_str());
			}
			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusAccountActivated,
											response.body.c_str());
			return LinphoneAccountCreatorStatusAccountActivated;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (creator->cbs->activate_account_response_cb != NULL) {
				creator->cbs->activate_account_response_cb(creator, LinphoneAccountCreatorStatusRequestFailed, response.body.c_str());
			}

			if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusAccountNotExist,
												response.body.c_str())
			} else if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_activated_flexiapi(LinphoneAccountCreator *creator) {
	char *identity = _get_identity(creator);
	if (!identity) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
														   "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_activated, creator,
										LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountInfo(string(creator->username).append("@").append(_get_domain(creator)))
		->then([creator](FlexiAPIClient::Response response) {
			if (response.json()["activated"].asBool()) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusAccountActivated,
											response.body.c_str());
				return LinphoneAccountCreatorStatusAccountActivated;
			}

			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusAccountNotActivated,
											response.body.c_str());
			return LinphoneAccountCreatorStatusAccountNotActivated;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusAccountNotExist,
												response.body.c_str())
			} else if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	ms_free(identity);
	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_link_phone_number_with_account_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->phone_number || !creator->username) {
		if (creator->cbs->link_account_response_cb != NULL) {
			creator->cbs->link_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(link_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountPhoneChangeRequest(creator->phone_number)
		->then([](FlexiAPIClient::Response response) {
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(link_account, creator, LinphoneAccountCreatorStatusAccountNotExist,
												response.body.c_str())
			} else if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(link_account, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(link_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestFailed;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_phone_number_link_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->phone_number || !creator->username || !creator->activation_code || (!creator->password && !creator->ha1) || !_get_domain(creator)) {
		if (creator->cbs->activate_alias_response_cb != NULL) {
			creator->cbs->activate_alias_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_alias, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountPhoneChange(creator->activation_code)
		->then([](FlexiAPIClient::Response response) {
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_alias, creator, LinphoneAccountCreatorStatusAccountNotExist,
												response.body.c_str())
			} else if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_alias, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_alias, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestFailed;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_phone_account_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->phone_number || !creator->activation_code) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient
		->accountActivatePhone(string(creator->username).append("@").append(_get_domain(creator)),
							   creator->activation_code)
		->then([](FlexiAPIClient::Response response) {
			return LinphoneAccountCreatorStatusAccountActivated;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusAccountNotExist,
												response.body.c_str())
			} else if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestFailed;
}

LinphoneAccountCreatorStatus linphone_account_creator_update_password_flexiapi(LinphoneAccountCreator *creator) {
	char *identity = _get_identity(creator);
	const char *new_pwd = (const char *)linphone_account_creator_get_user_data(creator);
	if (!identity || ((!creator->username && !creator->phone_number) || !_get_domain(creator) ||
					  (!creator->password && !creator->ha1) || !new_pwd)) {
		if (creator->cbs->update_account_response_cb != NULL) {
			creator->cbs->update_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
													 "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(update_account, creator, LinphoneAccountCreatorStatusMissingArguments,
										"Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}
	bctbx_free(identity);

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountPasswordChange(creator->algorithm, creator->password, new_pwd)
		->then([](FlexiAPIClient::Response response) {
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(update_account, creator, LinphoneAccountCreatorStatusAccountNotExist,
												response.body.c_str())
			} else if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(update_account, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(update_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_linked_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->username || !_get_domain(creator)) {
		if (creator->cbs->is_account_linked_response_cb != NULL) {
			creator->cbs->is_account_linked_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->me()
		->then([](FlexiAPIClient::Response response) {
			if (!response.json()["phone"].empty()) {
				return LinphoneAccountCreatorStatusAccountLinked;
			}

			return LinphoneAccountCreatorStatusAccountNotLinked;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 404) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusAccountNotExist,
												response.body.c_str())
			} else if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_send_token_flexiapi(LinphoneAccountCreator *creator) {
	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->sendToken(creator->pn_provider, creator->pn_param, creator->pn_prid)
		->then([creator](FlexiAPIClient::Response response) {
			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(send_token, creator, LinphoneAccountCreatorStatusRequestOk,
											response.body.c_str());
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(send_token, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(send_token, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_create_account_with_token_flexiapi(LinphoneAccountCreator *creator) {
	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountCreate(creator->username, creator->domain, creator->password, creator->algorithm, creator->token)
		->then([creator](FlexiAPIClient::Response response) {
			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusRequestOk,
											response.body.c_str());
			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}

/**
 * The following method is only available if APP_EVERYONE_IS_ADMIN is enabled on FlexiAPI
 */
LinphoneAccountCreatorStatus linphone_account_creator_admin_create_account_flexiapi(LinphoneAccountCreator *creator) {
	fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	string phone = (creator->phone_number)
		? creator->phone_number
		: "";

	flexiAPIClient->setTest(TRUE)
		->adminAccountCreate(creator->username, creator->password, "MD5", creator->domain, true, creator->email, phone)
		->then([creator](FlexiAPIClient::Response response) {
			NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusAccountCreated,
											response.body.c_str());

			return LinphoneAccountCreatorStatusRequestOk;
		})
		->error([creator](FlexiAPIClient::Response response) {
			if (response.code == 422) {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusMissingArguments,
												response.body.c_str())
			} else {
				NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
												response.body.c_str())
			}

			return LinphoneAccountCreatorStatusRequestFailed;
		});

	return LinphoneAccountCreatorStatusRequestOk;
}