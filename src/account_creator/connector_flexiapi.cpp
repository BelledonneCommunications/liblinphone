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

#include "bctoolbox/crypto.h"
#include "bctoolbox/defs.h"
#include "bctoolbox/regex.h"

#include "account_creator/private.h"
#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"
#include "linphone/account_creator.h"
#include "linphone/api/c-account.h"
#include "linphone/core.h"
#include "linphone/flexi-api-client.h"
#include "linphone/lpconfig.h"
#include "private_functions.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

using namespace LinphonePrivate;
using namespace std;

/**
 * Utility functions
 */

static const string generated_password_possible_characters =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_.:+=?!#%@&{}[]";
static const string generated_username_possible_characters = "0123456789abcdefghijklmnopqrstuvwxyz_";

static string generate_random_string(string seed, int size) {
	string result = "";
	int i;
	for (i = 0; i < size - 1; i++) {
		result += seed[bctbx_random() % (sizeof(seed) - 1)];
	}
	result += '\0';
	return result;
}

/**
 * Unsecure endpoint
 */
LinphoneAccountCreatorStatus linphone_account_creator_create_account_flexiapi(LinphoneAccountCreator *creator) {
	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	char *identity = linphone_account_creator_get_identity(creator);

	if (!identity || ((!creator->username && !creator->phone_number) || (!creator->password && !creator->ha1))) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		if (identity) ms_free(identity);
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);
	string auth_token = (creator->token) ? creator->token : "";

	if (creator->phone_number) {
		flexiAPIClient

		    ->accountCreate("", string(creator->password), "", "", string(creator->email),
		                    string(creator->phone_number), auth_token)
		    ->then([creator](FlexiAPIClient::Response response) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusAccountCreated,
			                                    response.body.c_str());
			    return LinphoneAccountCreatorStatusRequestOk;
		    })
		    ->error([creator](FlexiAPIClient::Response response) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusAccountNotCreated,
			                                    response.body.c_str())
			    return LinphoneAccountCreatorStatusRequestFailed;
		    });
	} else {
		flexiAPIClient
		    ->accountCreate(string(creator->username), string(creator->password), "", "", string(creator->email), "",
		                    auth_token)
		    ->then([creator](FlexiAPIClient::Response response) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusAccountCreated,
			                                    response.body.c_str());
			    return LinphoneAccountCreatorStatusAccountCreated;
		    })
		    ->error([creator](FlexiAPIClient::Response response) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusAccountNotCreated,
			                                    response.body.c_str())
			    return LinphoneAccountCreatorStatusRequestFailed;
		    });
	}

	ms_free(identity);
	return LinphoneAccountCreatorStatusRequestOk;
}

/**
 * Unsecure endpoint
 */
LinphoneAccountCreatorStatus linphone_account_creator_is_phone_number_used_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->phone_number) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_alias_used, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountInfoByPhone(string(creator->phone_number))
	    ->then([creator](FlexiAPIClient::Response response) {
		    LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusAliasIsAccount;
		    if (response.json()["phone"].asBool()) {
			    status = LinphoneAccountCreatorStatusAliasExist;
		    }
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_alias_used, creator, status, response.body.c_str());
		    return LinphoneAccountCreatorStatusRequestOk;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 404) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_alias_used, creator, LinphoneAccountCreatorStatusAliasNotExist,
			                                    response.body.c_str())
		    } else {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_alias_used, creator, LinphoneAccountCreatorStatusUnexpectedError,
			                                    response.body.c_str())
		    }

		    return LinphoneAccountCreatorStatusRequestFailed;
	    });

	return LinphoneAccountCreatorStatusRequestOk;
}

/**
 * Unsecure endpoint
 */
LinphoneAccountCreatorStatus linphone_account_creator_recover_phone_account_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->phone_number || !creator->token) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(recover_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}
	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	string auth_token = (creator->token) ? creator->token : "";

	flexiAPIClient->accountRecoverByPhone(creator->phone_number, auth_token)
	    ->then([creator](FlexiAPIClient::Response response) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(recover_account, creator, LinphoneAccountCreatorStatusRequestOk,
		                                    response.body.c_str());
		    return LinphoneAccountCreatorStatusRequestOk;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 404) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(recover_account, creator, LinphoneAccountCreatorStatusAccountNotExist,
			                                    response.body.c_str())
		    } else if (response.code == 401 || response.code == 403) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(recover_account, creator,
			                                    LinphoneAccountCreatorStatusRequestNotAuthorized, response.body.c_str())
		    } else {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(recover_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
			                                    response.body.c_str())
		    }

		    return LinphoneAccountCreatorStatusRequestFailed;
	    });

	return LinphoneAccountCreatorStatusRequestOk;
}

/**
 * Unsecure endpoint
 */
LinphoneAccountCreatorStatus linphone_account_creator_login_linphone_account_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->username || !creator->activation_code) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(login_linphone_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient
	    ->accountRecoverUsingRecoverKey(
	        string(creator->username)
	            .append("@")
	            .append(linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)),
	        string(creator->activation_code))
	    ->then([creator](FlexiAPIClient::Response response) {
		    auto passwords = response.json()["passwords"];
		    if (!passwords.isArray()) {
			    return LinphoneAccountCreatorStatusUnexpectedError;
		    }

		    string requestedAlgo = (creator->algorithm) ? creator->algorithm : "MD5";

		    unsigned int count = passwords.size();
		    for (unsigned int i = 0; i < count; i++) {
			    auto tuple = passwords[i];

			    string algorithm = tuple["algorithm"].asString();
			    string password = tuple["password"].asString();
			    if (requestedAlgo != algorithm) {
				    if (i == count - 1) {
					    ms_warning("[FlexiAPI Account Creator] Asked for password hashed using [%s], got algorithm "
					               "[%s] instead",
					               requestedAlgo.c_str(), algorithm.c_str());
				    }
				    set_string(&creator->algorithm, algorithm.c_str(), FALSE);
				    set_string(&creator->ha1, password.c_str(), FALSE);
			    } else {
				    ms_message("[FlexiAPI Account Creator] Found hashed password matching expected algorithm [%s]",
				               algorithm.c_str());
				    set_string(&creator->algorithm, algorithm.c_str(), FALSE);
				    set_string(&creator->ha1, password.c_str(), FALSE);
				    break;
			    }
		    }

		    string username = response.json()["username"].asString();
		    string expectedUsername = creator->username;
		    if (username != expectedUsername) {
			    ms_message("[FlexiAPI Account Creator] Phone number was an alias to [%s] username, using it instead",
			               username.c_str());
			    set_string(&creator->username, username.c_str(), FALSE);
		    }

		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(login_linphone_account, creator, LinphoneAccountCreatorStatusRequestOk,
		                                    response.body.c_str());
		    return LinphoneAccountCreatorStatusRequestOk;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 404) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(login_linphone_account, creator,
			                                    LinphoneAccountCreatorStatusAccountNotExist, response.body.c_str())
		    } else {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(login_linphone_account, creator,
			                                    LinphoneAccountCreatorStatusUnexpectedError, response.body.c_str())
		    }

		    return LinphoneAccountCreatorStatusRequestFailed;
	    });

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_exist_flexiapi(LinphoneAccountCreator *creator) {
	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (!creator->username || !creator->domain) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_exist, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	// Warning: breaking change from XMLRPC API: LinphoneAccountCreatorStatusAccountExistWithAlias is never returned
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
	if ((!creator->username && !creator->phone_number) || !creator->password || !creator->account) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(delete_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

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
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient
	    ->accountActivateEmail(string(creator->username)
	                               .append("@")
	                               .append(linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)),
	                           creator->activation_code)
	    ->then([creator](FlexiAPIClient::Response response) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusAccountActivated,
		                                    response.body.c_str());
		    return LinphoneAccountCreatorStatusRequestOk;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (creator->cbs->activate_account_response_cb != NULL) {
			    creator->cbs->activate_account_response_cb(creator, LinphoneAccountCreatorStatusRequestFailed,
			                                               response.body.c_str());
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
	char *identity = linphone_account_creator_get_identity(creator);
	if (!identity) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_activated, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient
	    ->accountInfo(string(creator->username)
	                      .append("@")
	                      .append(linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)))
	    ->then([creator](FlexiAPIClient::Response response) {
		    if (response.json()["activated"].asBool()) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_activated, creator,
			                                    LinphoneAccountCreatorStatusAccountActivated, response.body.c_str());
			    return LinphoneAccountCreatorStatusRequestOk;
		    }

		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_activated, creator,
		                                    LinphoneAccountCreatorStatusAccountNotActivated, response.body.c_str());
		    return LinphoneAccountCreatorStatusRequestFailed;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 404) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_activated, creator,
			                                    LinphoneAccountCreatorStatusAccountNotExist, response.body.c_str())
		    } else if (response.code == 422) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_activated, creator,
			                                    LinphoneAccountCreatorStatusMissingArguments, response.body.c_str())
		    } else {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_activated, creator,
			                                    LinphoneAccountCreatorStatusUnexpectedError, response.body.c_str())
		    }

		    return LinphoneAccountCreatorStatusRequestFailed;
	    });

	ms_free(identity);
	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus
linphone_account_creator_link_phone_number_with_account_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->phone_number || !creator->username) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(link_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountPhoneChangeRequest(creator->phone_number)
	    ->then([creator](BCTBX_UNUSED(FlexiAPIClient::Response response)) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(link_account, creator, LinphoneAccountCreatorStatusRequestOk,
		                                    response.body.c_str())
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

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus
linphone_account_creator_activate_phone_number_link_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->phone_number || !creator->username || !creator->activation_code ||
	    (!creator->password && !creator->ha1) ||
	    !linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_alias, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountPhoneChange(creator->activation_code)
	    ->then([creator](BCTBX_UNUSED(FlexiAPIClient::Response response)) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_alias, creator, LinphoneAccountCreatorStatusAccountActivated,
		                                    response.body.c_str())
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

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_phone_account_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->phone_number || !creator->activation_code) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient
	    ->accountActivatePhone(string(creator->username)
	                               .append("@")
	                               .append(linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)),
	                           creator->activation_code)
	    ->then([creator](BCTBX_UNUSED(FlexiAPIClient::Response response)) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusAccountActivated,
		                                    response.body.c_str())
		    return LinphoneAccountCreatorStatusRequestOk;
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

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_update_password_flexiapi(LinphoneAccountCreator *creator) {
	char *identity = linphone_account_creator_get_identity(creator);
	const char *new_pwd = (const char *)linphone_account_creator_get_user_data(creator);
	if (!identity || ((!creator->username && !creator->phone_number) ||
	                  !linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator) ||
	                  (!creator->password && !creator->ha1) || !new_pwd)) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(update_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}
	bctbx_free(identity);

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountPasswordChange(creator->algorithm, creator->password, new_pwd)
	    ->then([creator](BCTBX_UNUSED(FlexiAPIClient::Response response)) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(update_account, creator, LinphoneAccountCreatorStatusRequestOk,
		                                    response.body.c_str())
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
	if (!creator->username || !linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->me()
	    ->then([creator](FlexiAPIClient::Response response) {
		    if (!response.json()["phone"].empty()) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusAccountLinked,
			                                    response.body.c_str())
			    return LinphoneAccountCreatorStatusRequestOk;
		    }

		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusAccountNotLinked,
		                                    response.body.c_str())
		    return LinphoneAccountCreatorStatusRequestFailed;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 404) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusAccountNotExist,
			                                    response.body.c_str())
		    } else if (response.code == 422) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator,
			                                    LinphoneAccountCreatorStatusMissingArguments, response.body.c_str())
		    } else {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusUnexpectedError,
			                                    response.body.c_str())
		    }

		    return LinphoneAccountCreatorStatusRequestFailed;
	    });

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus linphone_account_creator_send_token_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->pn_provider || !creator->pn_param || !creator->pn_prid) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(send_token, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->sendAccountCreationTokenByPush(creator->pn_provider, creator->pn_param, creator->pn_prid)
	    ->then([creator](FlexiAPIClient::Response response) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(send_token, creator, LinphoneAccountCreatorStatusRequestOk,
		                                    response.body.c_str());
		    return LinphoneAccountCreatorStatusRequestOk;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 422) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(send_token, creator, LinphoneAccountCreatorStatusMissingArguments,
			                                    response.body.c_str())
		    } else if (response.code == 429) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(send_token, creator, LinphoneAccountCreatorStatusRequestTooManyRequests,
			                                    response.body.c_str())
		    } else {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(send_token, creator, LinphoneAccountCreatorStatusUnexpectedError,
			                                    response.body.c_str())
		    }

		    return LinphoneAccountCreatorStatusRequestFailed;
	    });

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus
linphone_account_creator_account_creation_request_token_flexiapi(LinphoneAccountCreator *creator) {
	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountCreationRequestToken()
	    ->then([creator](FlexiAPIClient::Response response) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(account_creation_request_token, creator,
		                                    LinphoneAccountCreatorStatusRequestOk, response.body.c_str());
		    return LinphoneAccountCreatorStatusRequestOk;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 404 || response.code == 403) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(account_creation_request_token, creator,
			                                    LinphoneAccountCreatorStatusServerError, response.body.c_str())
		    } else {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(account_creation_request_token, creator,
			                                    LinphoneAccountCreatorStatusUnexpectedError, response.body.c_str())
		    }

		    return LinphoneAccountCreatorStatusRequestFailed;
	    });

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus
linphone_account_creator_account_creation_token_using_request_token_flexiapi(LinphoneAccountCreator *creator) {
	if (!creator->account_creation_request_token) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(account_creation_token_using_request_token, creator,
		                                LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}
	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	flexiAPIClient->accountCreationTokenUsingRequestToken(creator->account_creation_request_token)
	    ->then([creator](FlexiAPIClient::Response response) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(account_creation_token_using_request_token, creator,
		                                    LinphoneAccountCreatorStatusRequestOk, response.body.c_str())
		    return LinphoneAccountCreatorStatusRequestOk;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 404) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(account_creation_token_using_request_token, creator,
			                                    LinphoneAccountCreatorStatusRequestFailed, response.body.c_str())
		    } else if (response.code == 422) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(account_creation_token_using_request_token, creator,
			                                    LinphoneAccountCreatorStatusMissingArguments, response.body.c_str())
		    } else if (response.code == 403) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(account_creation_token_using_request_token, creator,
			                                    LinphoneAccountCreatorStatusServerError, response.body.c_str())
		    } else {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(account_creation_token_using_request_token, creator,
			                                    LinphoneAccountCreatorStatusUnexpectedError, response.body.c_str())
		    }

		    return LinphoneAccountCreatorStatusRequestFailed;
	    });

	return LinphoneAccountCreatorStatusRequestOk;
}

LinphoneAccountCreatorStatus
linphone_account_creator_create_account_with_token_flexiapi(LinphoneAccountCreator *creator) {
	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	if (!creator->username || !creator->domain || !creator->token) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	if (creator->password == nullptr) {
		set_string(&creator->password, generate_random_string(generated_password_possible_characters, 12).c_str(),
		           FALSE);
	}

	string password = (creator->password) ? creator->password : "";
	string algo = (creator->algorithm) ? creator->algorithm : "MD5";
	string phone = (creator->phone_number) ? creator->phone_number : "";
	string mail = (creator->email) ? creator->email : "";
	string auth_token = (creator->token) ? creator->token : "";

	flexiAPIClient->accountCreate(creator->username, password, algo, creator->domain, mail, phone, auth_token)
	    ->then([creator](FlexiAPIClient::Response response) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusAccountCreated,
		                                    response.body.c_str());
		    return LinphoneAccountCreatorStatusRequestOk;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 422) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusMissingArguments,
			                                    response.body.c_str())
		    } else if (response.code == 401 || response.code == 403) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator,
			                                    LinphoneAccountCreatorStatusRequestNotAuthorized, response.body.c_str())
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
	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	if (!creator->username || !creator->domain) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	string generated_password = generate_random_string(generated_password_possible_characters, 12);
	string password = (creator->password) ? creator->password : generated_password;
	string algo = (creator->algorithm) ? creator->algorithm : "MD5";

	string phone = (creator->phone_number) ? creator->phone_number : "";
	string mail = (creator->email) ? creator->email : "";

	flexiAPIClient->useTestAdminAccount(true)
	    ->adminAccountCreate(creator->username, password, algo, creator->domain, true, mail, phone)
	    ->then([creator](FlexiAPIClient::Response response) {
		    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusAccountCreated,
		                                    response.body.c_str());

		    return LinphoneAccountCreatorStatusRequestOk;
	    })
	    ->error([creator](FlexiAPIClient::Response response) {
		    if (response.code == 422) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusMissingArguments,
			                                    response.body.c_str())
		    } else if (response.code == 401 || response.code == 403) {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator,
			                                    LinphoneAccountCreatorStatusRequestNotAuthorized, response.body.c_str())
		    } else {
			    NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusUnexpectedError,
			                                    response.body.c_str())
		    }

		    return LinphoneAccountCreatorStatusRequestFailed;
	    });

	return LinphoneAccountCreatorStatusRequestOk;
}

/**
 * Creates and activates a "push" account to be used as a dependent account for other sip account that cannot send push
 * notifications. Account is created with a generated username and password, and is automatically activated. Uses
 * FlexiApi endpoints : POST /accounts/with-account-creation-token (accountCreateWithAccountCreationToken) -> creates a
 * deactivated account GET /accounts/me/provision -> get provisioning token GET /provisioning/{provisioning_token} ->
 * activation (non api end point)
 */
LinphoneAccountCreatorStatus
linphone_account_creator_create_push_account_with_token_flexiapi(LinphoneAccountCreator *creator) {

	if (!creator->domain || !creator->token || !creator->algorithm) {
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameter (domain, token or algorithm)")
		return LinphoneAccountCreatorStatusMissingArguments;
	}
	auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);

	set_string(&creator->password, generate_random_string(generated_password_possible_characters, 12).c_str(), FALSE);
	set_string(&creator->username, generate_random_string(generated_username_possible_characters, 24).c_str(), FALSE);

	flexiAPIClient
	    ->accountCreateWithAccountCreationToken(creator->username, creator->password, creator->algorithm,
	                                            creator->token) // Create the account
	    ->then([creator](FlexiAPIClient::Response response) {
		    ms_message("[FlexiAPI Push Account Creator] push account created [%s]", response.body.c_str());
		    auto pushAccount = linphone_account_creator_create_account_in_core(
		        creator); // This is required by end point /accounts/me/provision to set the From header of http
		                  // request. To be revisited in the future when endpoint becomes
		                  // /accounts/me/provision?<username>
		    auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);
		    flexiAPIClient
		        ->accountProvision() // Generate a provisionning token for it
		        ->then([creator, pushAccount](FlexiAPIClient::Response response) {
			        ms_message("[FlexiAPI Push Account Creator] provisioning token retrieved [%s]",
			                   response.body.c_str());
			        auto provisioningToken = response.json()["provisioning_token"].asString();
			        auto flexiAPIClient = make_shared<FlexiAPIClient>(creator->core);
			        flexiAPIClient
			            ->accountProvisioningInformation(provisioningToken) // Activate it with the provsionning token
			            ->then([creator, pushAccount](FlexiAPIClient::Response response) {
				            linphone_core_remove_account(
				                creator->core,
				                pushAccount); // It is the app responsibilty to create the account locally (via
				                              // linphone_account_creator_create_account_in_core)
				            linphone_account_unref(pushAccount);
				            ms_message("[FlexiAPI Push Account Creator] account activated [%s]", response.body.c_str());
				            NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator,
				                                            LinphoneAccountCreatorStatusAccountCreated,
				                                            response.body.c_str());
				            return LinphoneAccountCreatorStatusRequestOk;
			            })
			            ->error([creator, pushAccount](FlexiAPIClient::Response response) {
				            linphone_core_remove_account(creator->core, pushAccount);
				            linphone_account_unref(pushAccount);
				            NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator,
				                                            LinphoneAccountCreatorStatusAccountNotCreated,
				                                            response.body.c_str())
				            return LinphoneAccountCreatorStatusRequestFailed;
			            });
		        })
		        ->error([creator, pushAccount](FlexiAPIClient::Response response) {
			        linphone_core_remove_account(creator->core, pushAccount);
			        linphone_account_unref(pushAccount);
			        NOTIFY_IF_EXIST_ACCOUNT_CREATOR(
			            create_account, creator, LinphoneAccountCreatorStatusAccountNotCreated, response.body.c_str())
			        return LinphoneAccountCreatorStatusRequestFailed;
		        });
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
