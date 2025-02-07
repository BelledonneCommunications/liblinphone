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

#include "linphone/core.h"
#include "linphone/core_utils.h"

int main(int argc, char *argv[]) {
	LinphoneAccount *account;
	char *normalized_number;
	if (argc < 2) {
		fprintf(stderr, "Usage:\n%s <phone number> [<country code>] [--escape-plus]\nReturns normalized number.",
		        argv[0]);
		return -1;
	}
	linphone_core_enable_logs(stderr);
	linphone_core_set_log_level(ORTP_DEBUG);
	LinphoneAccountParams *params = linphone_account_params_new(nullptr, FALSE);
	if (argc > 2) linphone_account_params_set_international_prefix(params, argv[2]);
	if (argc > 3 && strcmp(argv[3], "--escape-plus") == 0) linphone_account_params_set_dial_escape_plus(params, TRUE);
	account = linphone_core_create_account(NULL, params);
	linphone_account_params_unref(params);
	normalized_number = linphone_proxy_config_normalize_phone_number(account, argv[1]);

	if (!normalized_number) {
		printf("Invalid phone number: %s\n", argv[1]);
	} else {
		printf("Normalized number is %s\n", normalized_number);
		/*check extracted ccc*/
		const LinphoneAccountParams *account_params = linphone_account_get_params(account);
		if (linphone_account_params_get_international_prefix(account_params) != NULL) {
			if (linphone_dial_plan_lookup_ccc_from_e164(normalized_number) !=
			    atoi(linphone_account_params_get_international_prefix(account_params))) {
				printf("Error ccc [%i] not correctly parsed\n",
				       linphone_dial_plan_lookup_ccc_from_e164(normalized_number));
			} else {
				printf("Extracted ccc is [%i] \n", linphone_dial_plan_lookup_ccc_from_e164(normalized_number));
			}
		}
	}
	return 0;
}
