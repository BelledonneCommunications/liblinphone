/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#include <ctype.h>

#include "account/account.h"
#include "account/account-params.h"
#include "c-wrapper/internal/c-tools.h"
#include "dial-plan/dial-plan.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-account-cbs.h"
#include "linphone/api/c-account-params.h"
#include "linphone/wrapper_utils.h"
#include "utils/enum.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneAccount* linphone_account_new(LinphoneCore *lc, LinphoneAccountParams *params) {
	return Account::createCObject(lc, AccountParams::toCpp(params)->getSharedFromThis());
}

LinphoneAccount* linphone_account_new_with_config(LinphoneCore *lc, LinphoneAccountParams *params, LinphoneProxyConfig *config) {
	return Account::createCObject(lc, AccountParams::toCpp(params)->getSharedFromThis(), config);
}

LinphoneAccount* linphone_account_clone(const LinphoneAccount *account) {
	return Account::toCpp(account)->clone()->toC();
}

LinphoneAccount* linphone_account_ref(LinphoneAccount *account) {
	Account::toCpp(account)->ref();
	return account;
}

void linphone_account_unref(LinphoneAccount *account) {
	Account::toCpp(account)->unref();
}

int linphone_account_set_params(LinphoneAccount *account, LinphoneAccountParams* params) {
	return Account::toCpp(account)->setAccountParams(AccountParams::toCpp(params)->getSharedFromThis());
}

const LinphoneAccountParams* linphone_account_get_params(LinphoneAccount *account) {
	return Account::toCpp(account)->getAccountParams()->toC();
}

void linphone_account_set_user_data(LinphoneAccount *account, void *user_data) {
	Account::toCpp(account)->setUserData(user_data);
}

void* linphone_account_get_user_data(LinphoneAccount *account) {
	return Account::toCpp(account)->getUserData();
}

void linphone_account_set_custom_header(LinphoneAccount *account, const char *header_name, const char *header_value) {
	Account::toCpp(account)->setCustomheader(std::string(header_name), std::string(header_value));
}

const char* linphone_account_get_custom_header(LinphoneAccount *account, const char *header_name) {
	return Account::toCpp(account)->getCustomHeader(std::string(header_name));
}

void linphone_account_set_dependency(LinphoneAccount *account, LinphoneAccount *depends_on) {
	Account::toCpp(account)->setDependency(depends_on ? Account::toCpp(depends_on)->getSharedFromThis() : nullptr);
}

LinphoneAccount* linphone_account_get_dependency(LinphoneAccount *account) {
	if (Account::toCpp(account)->getDependency() != nullptr) {
		return Account::toCpp(account)->getDependency()->toC();
	}

	return NULL;
}

LinphoneCore* linphone_account_get_core(LinphoneAccount *account) {
	return Account::toCpp(account)->getCore();
}

LinphoneErrorInfo* linphone_account_get_error_info(LinphoneAccount *account) {
	return Account::toCpp(account)->getErrorInfo();
}

LinphoneAddress* linphone_account_get_contact_address(LinphoneAccount *account) {
	return Account::toCpp(account)->getContactAddress();
}

LinphoneRegistrationState linphone_account_get_state(LinphoneAccount *account) {
	return Account::toCpp(account)->getState();
}

void linphone_account_refresh_register(LinphoneAccount *account) {
	Account::toCpp(account)->refreshRegister();
}

void linphone_account_pause_register(LinphoneAccount *account) {
	Account::toCpp(account)->pauseRegister();
}

LinphoneReason linphone_account_get_error(LinphoneAccount *account) {
	return Account::toCpp(account)->getError();
}

LinphoneTransportType linphone_account_get_transport(LinphoneAccount *account) {
	return Account::toCpp(account)->getTransport();
}

bool_t linphone_account_is_avpf_enabled(LinphoneAccount *account) {
	return Account::toCpp(account)->isAvpfEnabled();
}

const LinphoneAuthInfo* linphone_account_find_auth_info(LinphoneAccount *account) {
	return Account::toCpp(account)->findAuthInfo();
}

int linphone_account_get_unread_chat_message_count(LinphoneAccount *account) {
	return Account::toCpp(account)->getUnreadChatMessageCount();
}

void linphone_account_add_callbacks(LinphoneAccount *account, LinphoneAccountCbs *cbs) {
	Account::toCpp(account)->addCallbacks(linphone_account_cbs_ref(cbs));
}

void linphone_account_remove_callbacks(LinphoneAccount *account, LinphoneAccountCbs *cbs) {
	Account::toCpp(account)->removeCallbacks(cbs);
	linphone_account_cbs_unref(cbs);
}

LinphoneAccountCbs *linphone_account_get_current_callbacks(const LinphoneAccount *account) {
	return Account::toCpp(account)->getCurrentCallbacks();
}

void linphone_account_set_current_callbacks(LinphoneAccount *account, LinphoneAccountCbs *cbs) {
	Account::toCpp(account)->setCurrentCallbacks(cbs);
}

const bctbx_list_t *linphone_account_get_callbacks_list(const LinphoneAccount *account) {
	return Account::toCpp(account)->getCallbacksList();
}

#define NOTIFY_IF_EXIST(cbName, functionName, ...) \
	bctbx_list_t *callbacksCopy = bctbx_list_copy(Account::toCpp(account)->getCallbacksList()); \
	for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) { \
		Account::toCpp(account)->setCurrentCallbacks(reinterpret_cast<LinphoneAccountCbs *>(bctbx_list_get_data(it))); \
		LinphoneAccountCbs ## cbName ## Cb cb = linphone_account_cbs_get_ ## functionName (Account::toCpp(account)->getCurrentCallbacks()); \
		if (cb) \
			cb(__VA_ARGS__); \
	} \
	Account::toCpp(account)->setCurrentCallbacks(nullptr); \
	bctbx_list_free(callbacksCopy);

void _linphone_account_notify_registration_state_changed(LinphoneAccount *account, LinphoneRegistrationState state, const char *message) {
	NOTIFY_IF_EXIST(RegistrationStateChanged, registration_state_changed, account, state, message)
}

bool_t linphone_account_is_phone_number(LinphoneAccount *account, const char *username) {
	if (!username) return FALSE;

	const char *p;
	char* unescaped_username = belle_sip_username_unescape_unnecessary_characters(username);
	for (p = unescaped_username; *p != '\0'; ++p) {
		if (isdigit(*p) ||
				*p==' ' ||
				*p=='.' ||
				*p=='-' ||
				*p==')' ||
				*p=='(' ||
				*p=='/' ||
				*p=='+' ||
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

static char *linphone_account_flatten_phone_number(const char *number) {
	char *unescaped_phone_number = belle_sip_username_unescape_unnecessary_characters(number);
	char *result = reinterpret_cast<char *>(ms_malloc0(strlen(unescaped_phone_number) + 1));
	char *w = result;
	const char *r;

	for (r = unescaped_phone_number; *r != '\0'; ++r) {
		if (*r == '+' || isdigit(*r)) {
			*w++ = *r;
		}
	}
	
	*w++ = '\0';
	belle_sip_free(unescaped_phone_number);
	return result;
}

static char* replace_icp_with_plus(char *phone, const char *icp){
	return (strstr(phone, icp) == phone) ?  ms_strdup_printf("+%s", phone+strlen(icp)) : ms_strdup(phone);
}

char* linphone_account_normalize_phone_number(LinphoneAccount *account, const char *username) {
	LinphoneAccountParams *tmpparams = account ? NULL : linphone_account_params_new(NULL);
	LinphoneAccount *tmpaccount = account ? account : linphone_account_new(NULL, tmpparams);
	if (tmpparams) linphone_account_params_unref(tmpparams);
	char* result = NULL;
	std::shared_ptr<DialPlan> dialplan;
	char * nationnal_significant_number = NULL;
	int ccc = -1;

	if (linphone_account_is_phone_number(tmpaccount, username)) {
		char *flatten = linphone_account_flatten_phone_number(username);
		ms_debug("Flattened number is '%s' for '%s'", flatten, username);

		ccc = DialPlan::lookupCccFromE164(flatten);
		if (ccc > -1) { /*e164 like phone number*/
			dialplan = DialPlan::findByCcc(ccc);
			nationnal_significant_number = strstr(flatten, dialplan->getCountryCallingCode().c_str());
			if (nationnal_significant_number) {
				nationnal_significant_number += strlen(dialplan->getCountryCallingCode().c_str());
			}
		} else if (flatten[0] == '+') {
			ms_message("Unknown ccc for e164 like number [%s]", flatten);
			goto end;
		} else {
			const char *dial_prefix = linphone_account_params_get_international_prefix(linphone_account_get_params(tmpaccount));
			if (dial_prefix) {
				dialplan = DialPlan::findByCcc(dial_prefix); //copy dial plan;
			} else {
				dialplan = DialPlan::MostCommon;
			}
			if (dial_prefix) {
				if (strcmp(dial_prefix, dialplan->getCountryCallingCode().c_str()) != 0){
					//probably generic dialplan, preserving proxy dial prefix
					dialplan->setCountryCallingCode(dial_prefix);
				}

				/*it does not make sens to try replace icp with + if we are not sure from the country we are (I.E dial_prefix==NULL)*/
				if (strstr(flatten, dialplan->getInternationalCallPrefix().c_str()) == flatten) {
					char *e164 = replace_icp_with_plus(flatten, dialplan->getInternationalCallPrefix().c_str());
					result = linphone_account_normalize_phone_number(tmpaccount, e164);
					ms_free(e164);
					goto end;
				}
			}
			nationnal_significant_number = flatten;
		}
		ms_debug("Using dial plan '%s'", dialplan->getCountry().c_str());

		/*if proxy has a dial prefix, modify phonenumber accordingly*/
		if (dialplan->getCountryCallingCode().c_str()[0] != '\0') {
			/* the number already starts with + or international prefix*/
			/*0. keep at most national number significant digits */
			char* nationnal_significant_number_start = nationnal_significant_number
														+ MAX(0, (int)strlen(nationnal_significant_number)
														- (int)dialplan->getNationalNumberLength());
			ms_debug("Prefix not present. Keeping at most %d digits: %s", dialplan->getNationalNumberLength(), nationnal_significant_number_start);

			/*1. First prepend international calling prefix or +*/
			/*2. Second add prefix*/
			/*3. Finally add user digits */
			bool_t dial_escape_plus = linphone_account_params_get_dial_escape_plus_enabled(linphone_account_get_params(tmpaccount));
			result = ms_strdup_printf("%s%s%s"
										, dial_escape_plus ? dialplan->getInternationalCallPrefix().c_str() : "+"
										, dialplan->getCountryCallingCode().c_str()
										, nationnal_significant_number_start);
			ms_debug("Prepended prefix resulted in %s", result);
		}

	end:
		if (result==NULL) {
			result = flatten;
		} else {
			ms_free(flatten);
		}
	}
	if (account == NULL) {
		//linphone_account_params_unref(tmpparams);
		linphone_account_unref(tmpaccount);
	}
	return result;
}


static LinphoneAddress* _destroy_addr_if_not_sip(LinphoneAddress* addr) {
	if (linphone_address_is_sip(addr)) {
		return addr;
	} else {
		linphone_address_unref(addr);
		return NULL;
	}
}

LinphoneAddress* linphone_account_normalize_sip_uri(LinphoneAccount *account, const char* username) {
	enum_lookup_res_t *enumres=NULL;
	char *enum_domain=NULL;
	char *tmpurl;
	LinphoneAddress *uri;

	if (!username || *username=='\0') return NULL;

	if (is_enum(username,&enum_domain)){
		if (enum_lookup(enum_domain,&enumres)<0){
			ms_free(enum_domain);
			return NULL;
		}
		ms_free(enum_domain);
		tmpurl=enumres->sip_address[0];
		uri=linphone_address_new(tmpurl);
		enum_lookup_res_free(enumres);
		return _destroy_addr_if_not_sip(uri);
	}
	/* check if we have a "sip:" or a "sips:" */
	if ( (strstr(username,"sip:")==NULL) && (strstr(username,"sips:")==NULL) ){
		/* this doesn't look like a true sip uri */
		if (strchr(username,'@')!=NULL){
			/* seems like sip: is missing !*/
			tmpurl=ms_strdup_printf("sip:%s",username);
			uri=linphone_address_new(tmpurl);
			ms_free(tmpurl);
			if (uri){
				return _destroy_addr_if_not_sip(uri);
			}
		}

		if (account!=NULL && linphone_account_params_get_identity_address(linphone_account_get_params(account))!=NULL){
			/* append the proxy domain suffix but remove any custom parameters/headers */
			LinphoneAddress *uri=linphone_address_clone(linphone_account_params_get_identity_address(linphone_account_get_params(account)));
			if (uri==NULL){
				return NULL;
			} else {
				linphone_address_clean(uri);
				linphone_address_set_display_name(uri,NULL);
				// Unescape character if possible
				char *unescaped_username = belle_sip_username_unescape_unnecessary_characters(username);
				linphone_address_set_username(uri,unescaped_username);
				belle_sip_free(unescaped_username);
				return _destroy_addr_if_not_sip(uri);
			}
		} else {
			return NULL;
		}
	}
	uri=linphone_address_new(username);
	if (uri!=NULL){
		return _destroy_addr_if_not_sip(uri);
	}

	return NULL;
}
