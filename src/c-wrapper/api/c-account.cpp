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

#include <ctype.h>

#include <bctoolbox/defs.h>

#include "account/account-params.h"
#include "account/account.h"
#include "c-wrapper/c-wrapper.h"
#include "core/core-p.h"
#include "dial-plan/dial-plan.h"
#include "linphone/api/c-account-cbs.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/wrapper_utils.h"
#include "utils/enum.h"

// =============================================================================

using namespace LinphonePrivate;

bool_t linphone_account_lime_enabled(LinphoneAccount *account) {
	auto cppAccount = Account::toCpp(account);
	auto params = cppAccount->getAccountParams();
	if (params) {
		if (params->getLimeServerUrl().empty()) {
			return FALSE;
		}
	} else {
		return FALSE;
	}
	return (Account::toCpp(account)->getLimeUserAccountStatus() != LimeUserAccountStatus::LimeUserAccountNone &&
	        Account::toCpp(account)->getLimeUserAccountStatus() !=
	            LimeUserAccountStatus::LimeUserAccountCreationSkiped);
}

LinphoneAccount *linphone_account_new(LinphoneCore *lc, LinphoneAccountParams *params) {
	return Account::createCObject(lc, AccountParams::toCpp(params)->getSharedFromThis());
}

LinphoneAccount *
linphone_account_new_with_config(LinphoneCore *lc, LinphoneAccountParams *params, LinphoneProxyConfig *config) {
	return Account::createCObject(lc, AccountParams::toCpp(params)->getSharedFromThis(), config);
}

LinphoneAccount *linphone_account_clone(const LinphoneAccount *account) {
	return Account::toCpp(account)->clone()->toC();
}

LinphoneAccount *linphone_account_ref(LinphoneAccount *account) {
	Account::toCpp(account)->ref();
	return account;
}

void linphone_account_unref(LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	Account::toCpp(account)->unref();
}

int linphone_account_set_params(LinphoneAccount *account, LinphoneAccountParams *params) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->setAccountParams(AccountParams::toCpp(params)->getSharedFromThis());
}

const LinphoneAccountParams *linphone_account_get_params(const LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->getAccountParams()->toC();
}

void linphone_account_add_custom_param(LinphoneAccount *account, const char *key, const char *value) {
	AccountLogContextualizer logContextualizer(account);
	Account::toCpp(account)->addCustomParam(L_C_TO_STRING(key), L_C_TO_STRING(value));
}

const char *linphone_account_get_custom_param(const LinphoneAccount *account, const char *key) {
	AccountLogContextualizer logContextualizer(account);
	return L_STRING_TO_C(Account::toCpp(account)->getCustomParam(L_C_TO_STRING(key)));
}

void linphone_account_set_user_data(LinphoneAccount *account, void *user_data) {
	Account::toCpp(account)->setUserData(user_data);
}

void *linphone_account_get_user_data(LinphoneAccount *account) {
	return Account::toCpp(account)->getUserData();
}

void linphone_account_set_custom_header(LinphoneAccount *account, const char *header_name, const char *header_value) {
	AccountLogContextualizer logContextualizer(account);
	Account::toCpp(account)->setCustomheader(std::string(header_name), std::string(header_value));
}

const char *linphone_account_get_custom_header(LinphoneAccount *account, const char *header_name) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->getCustomHeader(std::string(header_name));
}

void linphone_account_set_dependency(LinphoneAccount *account, LinphoneAccount *depends_on) {
	AccountLogContextualizer logContextualizer(account);
	Account::toCpp(account)->setDependency(depends_on ? Account::toCpp(depends_on)->getSharedFromThis() : nullptr);
}

LinphoneAccount *linphone_account_get_dependency(LinphoneAccount *account) {
	if (Account::toCpp(account)->getDependency() != nullptr) {
		return Account::toCpp(account)->getDependency()->toC();
	}

	return NULL;
}

LinphoneProxyConfig *linphone_account_get_proxy_config(LinphoneAccount *account) {
	return Account::toCpp(account)->getConfig();
}

LinphoneCore *linphone_account_get_core(LinphoneAccount *account) {
	return L_GET_C_BACK_PTR(Account::toCpp(account)->getCore());
}

const LinphoneErrorInfo *linphone_account_get_error_info(LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->getErrorInfo();
}

LinphoneAddress *linphone_account_get_contact_address(LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	return (Account::toCpp(account)->getContactAddress()) ? Account::toCpp(account)->getContactAddress()->toC() : NULL;
}

void linphone_account_set_contact_address(LinphoneAccount *account, const LinphoneAddress *addr) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->setContactAddress(Address::toCpp(addr)->getSharedFromThis());
}

LinphoneRegistrationState linphone_account_get_state(const LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->getState();
}

void linphone_account_refresh_register(LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	Account::toCpp(account)->refreshRegister();
}

void linphone_account_pause_register(LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	Account::toCpp(account)->pauseRegister();
}

LinphoneReason linphone_account_get_error(LinphoneAccount *account) {
	return Account::toCpp(account)->getError();
}

LinphoneTransportType linphone_account_get_transport(LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->getTransport();
}

bool_t linphone_account_is_avpf_enabled(LinphoneAccount *account) {
	return linphone_account_avpf_enabled(account);
}

bool_t linphone_account_avpf_enabled(LinphoneAccount *account) {
	return Account::toCpp(account)->isAvpfEnabled();
}

const LinphoneAuthInfo *linphone_account_find_auth_info(LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->findAuthInfo();
}

int linphone_account_get_unread_chat_message_count(const LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->getUnreadChatMessageCount();
}

const bctbx_list_t *linphone_account_get_chat_rooms(const LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->getChatRoomsCList();
}

bctbx_list_t *linphone_account_filter_chat_rooms(const LinphoneAccount *account, const char *filter) {
	AccountLogContextualizer logContextualizer(account);
	const auto list = Account::toCpp(account)->filterChatRooms(L_C_TO_STRING(filter));
	return ChatRoom::getCListFromCppList(list);
}

int linphone_account_get_missed_calls_count(const LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	return Account::toCpp(account)->getMissedCallsCount();
}

void linphone_account_reset_missed_calls_count(LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);
	Account::toCpp(account)->resetMissedCallsCount();
}

bctbx_list_t *linphone_account_get_call_logs(const LinphoneAccount *account) {
	AccountLogContextualizer logContextualizer(account);

	bctbx_list_t *results = NULL;
	std::list list = Account::toCpp(account)->getCallLogs();
	if (!list.empty()) {
		for (auto &log : list) {
			results = bctbx_list_append(results, linphone_call_log_ref(log->toC()));
		}
	}

	return results;
}

bctbx_list_t *linphone_account_get_call_logs_for_address(const LinphoneAccount *account,
                                                         const LinphoneAddress *remote_address) {
	AccountLogContextualizer logContextualizer(account);

	bctbx_list_t *results = NULL;
	const auto remote = Address::toCpp(remote_address)->getSharedFromThis();
	std::list list = Account::toCpp(account)->getCallLogsForAddress(remote);
	if (!list.empty()) {
		for (auto &log : list) {
			results = bctbx_list_append(results, linphone_call_log_ref(log->toC()));
		}
	}

	return results;
}

void linphone_account_clear_call_logs(const LinphoneAccount *account) {
	Account::toCpp(account)->deleteCallLogs();
}

bctbx_list_t *linphone_account_get_conference_information_list(const LinphoneAccount *account) {
	return linphone_account_get_conference_information_list_2(account, NULL);
}

bctbx_list_t *linphone_account_get_conference_information_list_2(const LinphoneAccount *account,
                                                                 bctbx_list_t *capabilities) {
	AccountLogContextualizer logContextualizer(account);

	std::list<LinphoneStreamType> capabilityList;
	if (capabilities) {
		for (bctbx_list_t *capability = capabilities; capability != NULL; capability = capability->next) {
			capabilityList.push_back(
			    static_cast<LinphoneStreamType>(LINPHONE_PTR_TO_INT(bctbx_list_get_data(capability))));
		}
	}
	bctbx_list_t *results = NULL;
	std::list list = Account::toCpp(account)->getConferenceInfos(capabilityList);
	if (!list.empty()) {
		for (auto &confInfo : list) {
			results = bctbx_list_append(results, linphone_conference_info_ref(confInfo->toC()));
		}
	}

	return results;
}

void linphone_account_add_callbacks(LinphoneAccount *account, LinphoneAccountCbs *cbs) {
	Account::toCpp(account)->addCallbacks(AccountCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_account_remove_callbacks(LinphoneAccount *account, LinphoneAccountCbs *cbs) {
	Account::toCpp(account)->removeCallbacks(AccountCbs::toCpp(cbs)->getSharedFromThis());
}

LinphoneAccountCbs *linphone_account_get_current_callbacks(const LinphoneAccount *account) {
	return Account::toCpp(account)->getCurrentCallbacks()->toC();
}

void linphone_account_set_current_callbacks(LinphoneAccount *account, LinphoneAccountCbs *cbs) {
	Account::toCpp(account)->setCurrentCallbacks(AccountCbs::toCpp(cbs)->getSharedFromThis());
}

const bctbx_list_t *linphone_account_get_callbacks_list(const LinphoneAccount *account) {
	return Account::toCpp(account)->getCCallbacksList();
}

void _linphone_account_notify_registration_state_changed(LinphoneAccount *account,
                                                         LinphoneRegistrationState state,
                                                         const char *message) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Account, Account::toCpp(account),
	                                  linphone_account_cbs_get_registration_state_changed, state, message);
}

void _linphone_account_notify_conference_information_updated(LinphoneAccount *account, const bctbx_list_t *infos) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Account, Account::toCpp(account),
	                                  linphone_account_cbs_get_conference_information_updated, infos);
}

bool_t linphone_account_is_phone_number(const LinphoneAccount *account, const char *username) {
	AccountLogContextualizer logContextualizer(account);
	if (!username) return FALSE;

	const char *p;
	char *unescaped_username = belle_sip_username_unescape_unnecessary_characters(username);
	for (p = unescaped_username; *p != '\0'; ++p) {
		if (isdigit((unsigned char)*p) || *p == ' ' || *p == '.' || *p == '-' || *p == ')' || *p == '(' || *p == '/' ||
		    *p == '+' ||
		    // non-breakable space (iOS uses it to format contacts phone number)
		    (unsigned char)*p == 0xca || (unsigned char)*p == 0xc2 || (unsigned char)*p == 0xa0) {
			continue;
		}

		belle_sip_free(unescaped_username);
		return FALSE;
	}

	belle_sip_free(unescaped_username);
	return TRUE;
}

char *linphone_account_normalize_phone_number(const LinphoneAccount *account, const char *username) {
	AccountLogContextualizer logContextualizer(account);

	if (!linphone_account_is_phone_number(account, username)) {
		lWarning() << "Username [" << username << "] isn't a phone number, can't normalize it";
		return NULL;
	}

	const char *dial_prefix;
	bool_t dial_escape_plus;

	if (account) {
		const LinphoneAccountParams *accountParams = linphone_account_get_params(account);
		dial_prefix = linphone_account_params_get_international_prefix(accountParams);
		dial_escape_plus = linphone_account_params_dial_escape_plus_enabled(accountParams);
	} else {
		LinphoneAccountParams *accountParams = linphone_account_params_new(nullptr, FALSE);
		dial_prefix = linphone_account_params_get_international_prefix(accountParams);
		dial_escape_plus = linphone_account_params_dial_escape_plus_enabled(accountParams);
		linphone_account_params_unref(accountParams);
	}

	char *flatten = ms_strdup(Utils::flattenPhoneNumber(username).c_str());
	lDebug() << "Flattened number is [" << flatten << "] for [" << username << "]";

	// if local short number, do not add international prefix
	if (dial_prefix) {
		std::shared_ptr<DialPlan> dialplan = DialPlan::findByCcc(dial_prefix);
		if ((flatten[0] != '+') && (strlen(flatten) < (size_t)dialplan->getMinNationalNumberLength())) {
			lDebug() << "Short number for [" << flatten << "] identified in dial plan [" << dialplan->getCountry()
			         << "] (min length is " << dialplan->getMinNationalNumberLength() << ")";
			return flatten;
		}
	}

	int ccc = -1;
	if (strlen(flatten) > 2 && flatten[0] == '0' && flatten[1] == '0') {
		std::string copy = std::string(flatten).substr(2);
		std::string zeroReplacedByPlus = std::string("+" + copy);
		ccc = DialPlan::lookupCccFromE164(zeroReplacedByPlus.c_str());
		lDebug() << "Flattened number started by 00, replaced it by + to lookup CCC";
		if (ccc > -1) {
			// If a dialplan was found using this, test if it is a short number
			if (DialPlan::isShortNumber(ccc, flatten)) {
				lDebug() << "Do not set international prefix for flattened short number";
				return flatten;
			} else if (!DialPlan::hasEnoughSignificantDigits(ccc, zeroReplacedByPlus)) {
				lDebug() << "Flattened number is too short, do not format phone number [" << flatten << "]";
				return flatten;
			} else {
				// If a dialplan was found using this, remove the + in the flatenned number
				ms_free(flatten);
				flatten = ms_strdup(zeroReplacedByPlus.substr(1).c_str());
			}
		}
	} else {
		ccc = DialPlan::lookupCccFromE164(flatten);
	}
	lDebug() << "CCC from flattened number is [" << ccc << "]";
	std::shared_ptr<DialPlan> dialplan;

	if (ccc > -1) {
		dialplan = DialPlan::findByCcc(ccc);
		lDebug() << "Using dial plan [" << dialplan->getCountry() << "]";
		if (dialplan == DialPlan::MostCommon && dial_prefix) {
			lDebug() << "MostCommon dial plan found, applying account dial prefix [" << dial_prefix << "]";
			dialplan->setCountryCallingCode(dial_prefix);
		}
		std::string formattedNumber = dialplan->formatPhoneNumber(flatten, dial_escape_plus);

		ms_free(flatten);
		if (formattedNumber.empty()) return NULL;
		return ms_strdup(formattedNumber.c_str());
	} else {
		if (flatten[0] == '+') {
			lDebug() << "Unknown ccc for e164 like number [" << flatten << "]";
			return flatten;
		}

		if (dial_prefix) {
			dialplan = DialPlan::findByCcc(dial_prefix);
		} else {
			dialplan = DialPlan::MostCommon;
		}
		if (dialplan == DialPlan::MostCommon && dial_prefix) {
			lDebug() << "MostCommon dial plan found, applying account dial prefix [" << dial_prefix << "]";
			dialplan->setCountryCallingCode(dial_prefix);
		}

		lDebug() << "Using dial plan [" << dialplan->getCountry() << "]";
		std::string formattedNumber = dialplan->formatPhoneNumber(flatten, dial_escape_plus);

		ms_free(flatten);
		if (formattedNumber.empty()) return NULL;
		return ms_strdup(formattedNumber.c_str());
	}

	return flatten;
}

static LinphoneAddress *_destroy_addr_if_not_sip(LinphoneAddress *addr) {
	if (linphone_address_is_sip(addr)) {
		return addr;
	} else {
		linphone_address_unref(addr);
		return NULL;
	}
}

LinphoneAddress *linphone_account_normalize_sip_uri(LinphoneAccount *account, const char *username) {
	AccountLogContextualizer logContextualizer(account);
	enum_lookup_res_t *enumres = NULL;
	char *enum_domain = NULL;
	char *tmpurl;
	LinphoneAddress *uri;

	if (!username || *username == '\0') return NULL;

#ifndef _WIN32
	if (is_enum(username, &enum_domain)) {
		if (enum_lookup(enum_domain, &enumres) < 0) {
			ms_free(enum_domain);
			return NULL;
		}
		ms_free(enum_domain);
		tmpurl = enumres->sip_address[0];
		uri = linphone_address_new(tmpurl);
		enum_lookup_res_free(enumres);
		return _destroy_addr_if_not_sip(uri);
	}
#endif

	/* check if we have a "sip:" or a "sips:" */
	if ((strstr(username, "sip:") == NULL) && (strstr(username, "sips:") == NULL)) {
		/* this doesn't look like a true sip uri */
		if (strchr(username, '@') != NULL) {
			/* seems like sip: is missing !*/
			tmpurl = ms_strdup_printf("sip:%s", username);
			uri = linphone_address_new(tmpurl);
			ms_free(tmpurl);
			if (uri) {
				return _destroy_addr_if_not_sip(uri);
			}
		}

		if (account != NULL &&
		    linphone_account_params_get_identity_address(linphone_account_get_params(account)) != NULL) {
			/* append the proxy domain suffix but remove any custom parameters/headers */
			LinphoneAddress *uri = linphone_address_clone(
			    linphone_account_params_get_identity_address(linphone_account_get_params(account)));
			if (uri == NULL) {
				return NULL;
			} else {
				linphone_address_clean(uri);
				linphone_address_set_display_name(uri, NULL);
				// Unescape character if possible
				char *unescaped_username = belle_sip_username_unescape_unnecessary_characters(username);
				linphone_address_set_username(uri, unescaped_username);
				belle_sip_free(unescaped_username);
				return _destroy_addr_if_not_sip(uri);
			}
		} else {
			return NULL;
		}
	}
	uri = linphone_address_new(username);
	if (uri != NULL) {
		return _destroy_addr_if_not_sip(uri);
	}

	return NULL;
}
