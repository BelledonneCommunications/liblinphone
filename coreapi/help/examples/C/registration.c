
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

/**
 * @defgroup registration_tutorials Basic registration
 * @ingroup tutorials
 *This program is a _very_ simple usage example of liblinphone.
 *Desmonstrating how to  initiate a SIP registration from a sip uri identity passed from the command line.
 *first argument must be like sip:jehan@sip.linphone.org , second must be password .
 *<br>
 *ex registration sip:jehan@sip.linphone.org secret
 *<br>Registration is cleared on SIGINT
 *<br>
 *@include registration.c

 *
 */

#include "linphone/core.h"

#include <signal.h>

static bool_t running = TRUE;

static void stop(int signum) {
	running = FALSE;
}

/**
 * Registration state notification callback
 */
static void account_registration_state_changed(struct _LinphoneCore *lc,
                                               LinphoneAccount *account,
                                               LinphoneRegistrationState cstate,
                                               const char *message) {
	LinphoneAccountParams *account_params = linphone_account_get_params(account);
	printf("New registration state %s for user id [%s] at proxy [%s]\n", linphone_registration_state_to_string(cstate),
	       linphone_account_params_get_identity(account_params), linphone_account_params_get_addr(account_params));
}

LinphoneCore *lc;
int main(int argc, char *argv[]) {
	LinphoneCoreVTable vtable = {0};
	LinphoneProxyConfig *proxy_cfg;
	LinphoneAddress *from;
	LinphoneAuthInfo *info;

	char *identity = NULL;
	char *password = NULL;
	const char *server_addr;

	/* takes   sip uri  identity from the command line arguments */
	if (argc > 1) {
		identity = argv[1];
	}

	/* takes   password from the command line arguments */
	if (argc > 2) {
		password = argv[2];
	}

	signal(SIGINT, stop);

#ifdef DEBUG_LOGS
	linphone_core_enable_logs(NULL); /*enable liblinphone logs.*/
#endif
	/*
	 Fill the LinphoneCoreVTable with application callbacks.
	 All are optional. Here we only use the account_registration_state_changed callbacks
	 in order to get notifications about the progress of the registration.
	 */
	vtable.account_registration_state_changed = account_registration_state_changed;

	/*
	 Instanciate a LinphoneCore object given the LinphoneCoreVTable
	*/
	lc = linphone_core_new(&vtable, NULL, NULL, NULL);

	/*create account parameters*/
	LinphoneAccountParams *params = linphone_account_params_new(nullptr, FALSE);
	/*parse identity*/
	LinphoneAddress *from = linphone_address_new(identity);
	LinphoneAuthInfo *info;
	if (from == NULL) {
		printf("%s not a valid sip uri, must be like sip:toto@sip.linphone.org \n", identity);
		goto end;
	}
	if (password != NULL) {
		info = linphone_auth_info_new(linphone_address_get_username(from), NULL, password, NULL, NULL,
		                              NULL);   /*create authentication structure from identity*/
		linphone_core_add_auth_info(lc, info); /*add authentication info to LinphoneCore*/
	}

	// configure proxy entries
	linphone_account_params_set_identity_address(params, from); /*set identity with user name and domain*/
	linphone_account_params_set_server_addr(
	    params, linphone_address_get_domain(from));        /* we assume domain = proxy server address*/
	linphone_account_params_enable_register(params, TRUE); /*activate registration for this account*/
	linphone_address_unref(from);                          /*release resource*/

	/*create account*/
	LinphoneAccount *account = linphone_core_create_account(NULL, params);

	linphone_core_add_account(lc, account);         /*add account to linphone core*/
	linphone_core_set_default_account(lc, account); /*set to default account*/

	linphone_account_params_unref(params); /*release resource*/

	/* main loop for receiving notifications and doing background linphonecore work: */
	while (running) {
		linphone_core_iterate(lc); /* first iterate initiates registration */
		ms_usleep(50000);
	}

	proxy_cfg = linphone_core_get_default_proxy_config(lc);  /* get default proxy config*/
	linphone_proxy_config_edit(proxy_cfg);                   /*start editing proxy configuration*/
	linphone_proxy_config_enable_register(proxy_cfg, FALSE); /*de-activate registration for this proxy config*/
	linphone_proxy_config_done(proxy_cfg);                   /*initiate REGISTER with expire = 0*/

	while (linphone_proxy_config_get_state(proxy_cfg) != LinphoneRegistrationCleared) {
		linphone_core_iterate(lc); /*to make sure we receive call backs before shutting down*/
		ms_usleep(50000);
	}

end:
	printf("Shutting down...\n");
	linphone_core_destroy(lc);
	printf("Exited\n");
	return 0;
}
