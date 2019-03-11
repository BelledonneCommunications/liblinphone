 /*
 tester - liblinphone test suite
 Copyright (C) 2013  Belledonne Communications SARL

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <belle-sip/belle-sip.h>
#include <ctype.h>
#include "liblinphone_tester.h"
#include "tester_utils.h"

struct _Account{
	LinphoneAddress *identity;
	LinphoneAddress *modified_identity;
	char *password;
	int registered;
	int done;
	int created;
	char *phone_alias;
	char *uuid;
};

typedef struct _Account Account;

static Account *account_new(LinphoneAddress *identity, const char *unique_id){
	char *modified_username;
	Account *obj=ms_new0(Account,1);

	// we need to inhibit leak detector because the two LinphoneAddress will remain behond the scope of the test being run
	belle_sip_object_inhibit_leak_detector(TRUE);
	obj->identity=linphone_address_clone(identity);
	obj->password=sal_get_random_token(8);
	obj->phone_alias = NULL;
	obj->modified_identity=linphone_address_clone(identity);
	modified_username=ms_strdup_printf("%s_%s",linphone_address_get_username(identity), unique_id);
	linphone_address_set_username(obj->modified_identity, modified_username);
	ms_free(modified_username);
	belle_sip_object_inhibit_leak_detector(FALSE);
	return obj;
};

void account_destroy(Account *obj){
	if (obj->uuid) bctbx_free(obj->uuid);
	linphone_address_unref(obj->identity);
	linphone_address_unref(obj->modified_identity);
	ms_free(obj->password);
	ms_free(obj);
}

typedef struct {
    int account_created;
    int confirmation_key_received;
    int account_activated;
} AccountCreatorState;

struct _AccountManager{
	char *unique_id;
	bctbx_list_t *accounts;
};

typedef struct _AccountManager AccountManager;

static AccountManager *the_am=NULL;

static void account_manager_generate_unique_id(AccountManager * am) {
	const int tokenLength = 6;
	if (am->unique_id)
		ms_free(am->unique_id);
	am->unique_id=sal_get_random_token(tokenLength);

	ms_message("Using lowercase random token for test username.");
	for (int i=0; i<tokenLength; i++) {
		am->unique_id[i] = tolower(the_am->unique_id[i]);
	}
}
AccountManager *account_manager_get(void){
	if (the_am==NULL){
		the_am=ms_new0(AccountManager,1);
		account_manager_generate_unique_id(the_am);
	}
	return the_am;
}

void account_manager_destroy(void){
	if (the_am){
		ms_free(the_am->unique_id);
		bctbx_list_free_with_data(the_am->accounts,(void(*)(void*))account_destroy);
		ms_free(the_am);
	}
	the_am=NULL;
	ms_message("Test account manager destroyed.");
}

Account *account_manager_get_account(AccountManager *m, const LinphoneAddress *identity){
	bctbx_list_t *it;

	for(it=m->accounts;it!=NULL;it=it->next){
		Account *a=(Account*)it->data;
		if (linphone_address_weak_equal(a->identity,identity)){
			return a;
		}
	}
	return NULL;
}

LinphoneAddress *account_manager_get_identity_with_modified_identity(const LinphoneAddress *modified_identity){
	AccountManager *m = account_manager_get();
	bctbx_list_t *it;

	for(it=m->accounts;it!=NULL;it=it->next){
		Account *a=(Account*)it->data;
		if (linphone_address_weak_equal(a->modified_identity,modified_identity)){
			return a->identity;
		}
	}
	return NULL;
}


static void account_created_on_server_cb(LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const char *info){
	Account *account=(Account*)linphone_core_get_user_data(lc);
	switch(state){
		case LinphoneRegistrationOk: {
			char * phrase = sal_op_get_error_info(linphone_proxy_config_get_sal_op(cfg))->full_string;
			if (phrase && strcasecmp("Test account created", phrase) == 0) {
				account->created=1;
			} else {
				account->registered=1;
			}
			break;
		}
		case LinphoneRegistrationCleared:
			account->done=1;
		break;
		default:
		break;
	}
}

void account_create_on_server(Account *account, const LinphoneProxyConfig *refcfg, const char* phone_alias){
	LinphoneCore *lc;
	LinphoneAddress *tmp_identity=linphone_address_clone(account->modified_identity);
	LinphoneProxyConfig *cfg;
	LinphoneAuthInfo *ai;
	char *tmp;
	LinphoneAddress *server_addr;
	LinphoneSipTransports tr;
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	linphone_core_cbs_set_registration_state_changed(cbs, account_created_on_server_cb);
	lc = configure_lc_from(cbs, bc_tester_get_resource_dir_prefix(), NULL, account);
	linphone_core_cbs_unref(cbs);
	tr.udp_port=LC_SIP_TRANSPORT_RANDOM;
	tr.tcp_port=LC_SIP_TRANSPORT_RANDOM;
	tr.tls_port=LC_SIP_TRANSPORT_RANDOM;
	linphone_core_set_sip_transports(lc,&tr);

	cfg=linphone_core_create_proxy_config(lc);
	linphone_address_set_secure(tmp_identity, FALSE);
	linphone_address_set_password(tmp_identity,account->password);
	linphone_address_set_header(tmp_identity,"X-Create-Account","yes");
	if (phone_alias) linphone_address_set_header(tmp_identity, "X-Phone-Alias", phone_alias);
	tmp=linphone_address_as_string(tmp_identity);
	linphone_proxy_config_set_identity(cfg,tmp);
	ms_free(tmp);
	linphone_address_unref(tmp_identity);

	server_addr=linphone_address_new(linphone_proxy_config_get_server_addr(refcfg));
	linphone_address_set_secure(server_addr, FALSE);
	linphone_address_set_transport(server_addr,LinphoneTransportTcp); // use tcp for account creation, we may not have certificates configured at this stage
	linphone_address_set_port(server_addr,0);
	tmp=linphone_address_as_string(server_addr);
	linphone_proxy_config_set_server_addr(cfg,tmp);
	ms_free(tmp);
	linphone_address_unref(server_addr);
	linphone_proxy_config_set_expires(cfg,3*3600); // accounts are valid 3 hours

	linphone_core_set_network_reachable_internal(lc, TRUE);

	linphone_core_add_proxy_config(lc,cfg);
	/*wait 25 seconds, since the DNS SRV resolution may take a while - and
	especially if router does NOT support DNS SRV and we have to wait its timeout*/
	if (wait_for_until(lc,NULL,&account->created,1,25000)==FALSE){
		ms_fatal("Account for %s could not be created on server.", linphone_proxy_config_get_identity(refcfg));
	}
	linphone_proxy_config_edit(cfg);
	tmp_identity=linphone_address_clone(account->modified_identity);
	linphone_address_set_secure(tmp_identity, FALSE);
	tmp=linphone_address_as_string(tmp_identity);
	linphone_proxy_config_set_identity(cfg,tmp); // remove the X-Create-Account header
	linphone_address_unref(tmp_identity);
	ms_free(tmp);
	linphone_proxy_config_done(cfg);

	ai=linphone_auth_info_new(linphone_address_get_username(account->modified_identity),
				NULL,
				account->password,NULL,NULL,linphone_address_get_domain(account->modified_identity));
	linphone_core_add_auth_info(lc,ai);
	linphone_auth_info_unref(ai);

	if (wait_for_until(lc,NULL,&account->registered,1,3000)==FALSE){
		ms_fatal("Account for %s is not working on server.", linphone_proxy_config_get_identity(refcfg));
	}
	linphone_core_remove_proxy_config(lc,cfg);
	linphone_proxy_config_unref(cfg);
	if (wait_for_until(lc,NULL,&account->done,1,3000)==FALSE){
		ms_error("Account creation could not clean the registration context.");
	}
	linphone_core_unref(lc);
}

static void account_created_in_db_cb(LinphoneAccountCreator *creator, LinphoneAccountCreatorStatus status, const char *resp){
	AccountCreatorState *state = linphone_account_creator_get_user_data(creator);
	switch(status){
		case LinphoneAccountCreatorStatusAccountCreated:
			state->account_created = TRUE;
			break;
		default:
			ms_fatal("Account not created on DB for %s.", linphone_account_creator_get_username(creator));
			state->account_created = FALSE;
			break;
	}
}

static void get_confirmation_key_cb(LinphoneAccountCreator *creator, LinphoneAccountCreatorStatus status, const char *resp){
	AccountCreatorState *state = linphone_account_creator_get_user_data(creator);
	switch(status){
		case LinphoneAccountCreatorStatusRequestOk:
			state->confirmation_key_received = TRUE;
			break;
		default:
			ms_warning("Confirmation key not received for %s.", linphone_account_creator_get_username(creator));
			state->confirmation_key_received = FALSE;
			break;
	}
}

static void account_activated_cb(LinphoneAccountCreator *creator, LinphoneAccountCreatorStatus status, const char *resp){
	AccountCreatorState *state = linphone_account_creator_get_user_data(creator);
	switch(status){
		case LinphoneAccountCreatorStatusAccountActivated:
		case LinphoneAccountCreatorStatusAccountAlreadyActivated:
			state->account_activated = TRUE;
			break;
		default:
			ms_message("Account not activated for %s.", linphone_account_creator_get_username(creator));
			state->account_activated = FALSE;
			break;
	}
}

void account_create_in_db(Account *account, LinphoneProxyConfig *cfg, const char *xmlrpc_url){
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_registration_state_changed(cbs, account_created_on_server_cb);
	LinphoneCore *lc = configure_lc_from(cbs, bc_tester_get_resource_dir_prefix(), NULL, account);
	linphone_core_cbs_unref(cbs);
	LinphoneSipTransports tr = {0};
	tr.udp_port = LC_SIP_TRANSPORT_RANDOM;
	tr.tcp_port = LC_SIP_TRANSPORT_RANDOM;
	tr.tls_port = LC_SIP_TRANSPORT_RANDOM;
	linphone_core_set_sip_transports(lc, &tr);

	LinphoneAccountCreator *creator = linphone_account_creator_new(lc, xmlrpc_url);
	LinphoneAccountCreatorCbs *creator_cbs = linphone_account_creator_get_callbacks(creator);

	LinphoneProxyConfig *default_cfg = linphone_core_get_default_proxy_config(lc);
	linphone_account_creator_set_proxy_config(creator, cfg);

	linphone_account_creator_service_set_user_data(linphone_account_creator_get_service(creator), (void*)LinphoneAccountCreatorStatusAccountCreated);

	const char *username = linphone_address_get_username(account->modified_identity);
	const char *password = account->password;
	const char *domain = linphone_proxy_config_get_domain(cfg);

	char *email = bctbx_strdup_printf("%s@%s", username, domain);

	AccountCreatorState state = {0};

	// create account
	linphone_account_creator_cbs_set_create_account(creator_cbs, account_created_in_db_cb);
	linphone_account_creator_set_username(creator, username);
	linphone_account_creator_set_password(creator, password);
	linphone_account_creator_set_domain(creator, domain);
	linphone_account_creator_set_email(creator, email);
	linphone_account_creator_set_user_data(creator, &state);

	if (account->phone_alias) {
		LinphoneAccountCreatorPhoneNumberStatusMask err = 0;
		const char* country_code = NULL;
		if (account->phone_alias[0] == '+') {
			const LinphoneDialPlan * ccc = linphone_dial_plan_by_ccc_as_int(linphone_dial_plan_lookup_ccc_from_e164(account->phone_alias));
			country_code  = linphone_dial_plan_get_country_calling_code(ccc);
		}
		if ((err = linphone_account_creator_set_phone_number(creator, account->phone_alias, country_code)) != LinphoneAccountCreatorPhoneNumberStatusOk) {
			ms_fatal("Could not set phone alias [%s] for account [%s], error [%i]"	, account->phone_alias
					 																, linphone_proxy_config_get_identity(cfg)
					 																, err);
		};
	}

	linphone_account_creator_create_account(creator);

	if (wait_for_until(lc, NULL, &state.account_created, TRUE, 15000) == FALSE)
		ms_fatal("Could not create account %s on db", linphone_proxy_config_get_identity(cfg));

	LinphoneAuthInfo *ai = linphone_auth_info_new(username, NULL, password, NULL, domain, domain);
	linphone_core_add_auth_info(lc, ai);
	linphone_auth_info_unref(ai);

	// get confirmation key
	linphone_account_creator_cbs_set_confirmation_key(creator_cbs, get_confirmation_key_cb);
	linphone_account_creator_get_confirmation_key(creator);

	if (wait_for_until(lc, NULL, &state.confirmation_key_received, TRUE, 15000) == FALSE)
		ms_fatal("Could not get confirmation key for account %s", linphone_proxy_config_get_identity(cfg));

	// activate account
	linphone_account_creator_cbs_set_activate_account(creator_cbs, account_activated_cb);
	if (linphone_account_creator_get_phone_number(creator))
		linphone_account_creator_activate_account_linphone(creator);
	else
		linphone_account_creator_activate_email_account_linphone(creator);

	if (wait_for_until(lc, NULL, &state.account_activated, TRUE, 15000) == FALSE)
		ms_fatal("Could not activate account %s", linphone_proxy_config_get_identity(cfg));

	linphone_account_creator_set_proxy_config(creator, default_cfg);

	bctbx_free(email);
	linphone_account_creator_unref(creator);
	linphone_core_unref(lc);
}

static LinphoneAddress *account_manager_check_account(AccountManager *m, LinphoneProxyConfig *cfg, LinphoneCoreManager *cm){
	LinphoneCore *lc = linphone_proxy_config_get_core(cfg);
	const char *identity = linphone_proxy_config_get_identity(cfg);
	LinphoneAddress *id_addr = linphone_address_new(identity);
	Account *account = account_manager_get_account(m, id_addr);
	LinphoneAuthInfo *ai;
	bool_t create_account = FALSE;
	const LinphoneAuthInfo *original_ai = linphone_core_find_auth_info(
		lc,
		NULL,
		linphone_address_get_username(id_addr),
		linphone_address_get_domain(id_addr)
	);
	const char *phone_alias = cm->phone_alias;

	if (!account || (phone_alias && (!account->phone_alias || strcmp(phone_alias, account->phone_alias) != 0))){
		if (account) {
			m->accounts = bctbx_list_remove(m->accounts, account);
			account_destroy(account);
		}
		account_manager_generate_unique_id(m); //change unique id to make sure we really create a new one
		account = account_new(id_addr, m->unique_id);
		account->phone_alias = ms_strdup(phone_alias);
		ms_message("No account for %s exists, going to create one.", identity);
		create_account = TRUE;
		m->accounts = bctbx_list_append(m->accounts, account);
	}
	// modify the username of the identity of the proxy config
	linphone_address_set_username(id_addr, linphone_address_get_username(account->modified_identity));
	linphone_proxy_config_set_identity_address(cfg, id_addr);

	// create account using account creator and flexisip-account-manager
	if (create_account){
		if (liblinphonetester_no_account_creator) {
			account_create_on_server(account, cfg, phone_alias);
		} else {
			const char *xmlrpc_url = linphone_config_get_string(
				linphone_core_get_config(lc),
				"misc",
				"xmlrpc_server_url",
				"http://subscribe.example.org:8082/flexisip-account-manager/xmlrpc.php"
			);
			account_create_in_db(account, cfg, xmlrpc_url);
		}
	}

	if (liblinphone_tester_keep_uuid) {
		// create and/or set uuid
		if (account->uuid == NULL) {
			char tmp[64];
			sal_create_uuid(linphone_core_get_sal(cm->lc), tmp, sizeof(tmp));
			account->uuid = bctbx_strdup(tmp);
		}
		sal_set_uuid(linphone_core_get_sal(cm->lc), account->uuid);
	}

	// remove previous auth info to avoid mismatching
	if (original_ai)
		linphone_core_remove_auth_info(lc,original_ai);

	ai = linphone_auth_info_new(
		linphone_address_get_username(account->modified_identity),
		NULL,
		account->password,
		NULL,
		linphone_address_get_domain(account->modified_identity),
		linphone_address_get_domain(account->modified_identity) // realm = domain
	);
	linphone_core_add_auth_info(lc, ai);
	linphone_auth_info_unref(ai);

	linphone_address_unref(id_addr);
	return account->modified_identity;
}

void linphone_core_manager_check_accounts(LinphoneCoreManager *m){
	const bctbx_list_t *it;
	AccountManager *am = account_manager_get();
	unsigned int logmask = linphone_core_get_log_level_mask();

	if (!liblinphonetester_show_account_manager_logs) linphone_core_set_log_level_mask(ORTP_ERROR|ORTP_FATAL);
	for(it = linphone_core_get_proxy_config_list(m->lc); it != NULL; it = it->next){
		LinphoneProxyConfig *cfg = (LinphoneProxyConfig *)it->data;
		account_manager_check_account(am, cfg, m);
	}
	if (!liblinphonetester_show_account_manager_logs) linphone_core_set_log_level_mask(logmask);
}
