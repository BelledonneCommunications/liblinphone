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
#include "linphone/account_creator.h"
#include "linphone/core.h"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

using namespace LinphonePrivate;

const char *linphone_account_creator_get_domain_with_fallback_to_proxy_domain(LinphoneAccountCreator *creator) {
	if (creator->domain) return creator->domain;
	else if (creator->proxy_cfg) return linphone_proxy_config_get_domain(creator->proxy_cfg);
	return NULL;
}

char *linphone_account_creator_get_identity(const LinphoneAccountCreator *creator) {
	char *identity = NULL;
	const char *username = creator->username ? creator->username : creator->phone_number;
	if (username) {
		// we must escape username
		LinphoneProxyConfig *proxy = linphone_core_create_proxy_config(creator->core);
		LinphoneAddress *addr = linphone_proxy_config_normalize_sip_uri(proxy, username);
		const char *addr_domain = addr ? linphone_address_get_domain(addr) : nullptr;
		const char *creator_domain = creator->domain;

		if (addr_domain == nullptr || (creator_domain != nullptr && strcmp(addr_domain, creator_domain) != 0)) {
			if (creator_domain != nullptr) {
				char *url = ms_strdup_printf("sip:%s", creator_domain);
				if (addr) linphone_address_unref(addr);
				addr = linphone_address_new(url);
				ms_free(url);

				if (addr) {
					linphone_address_set_username(addr, username);
				} else {
					goto end;
				}
			} else {
				goto end;
			}
		}

		identity = linphone_address_as_string(addr);
	end:
		if (addr) linphone_address_unref(addr);
		linphone_proxy_config_unref(proxy);
	}
	return identity;
}

void linphone_account_creator_fill_domain_and_algorithm_if_needed(LinphoneAccountCreator *creator) {
	if (creator->domain == NULL) {
		const char *domain =
		    linphone_config_get_string(linphone_core_get_config(creator->core), "assistant", "domain", NULL);
		if (domain) {
			linphone_account_creator_set_domain(creator, domain);
		}
	}
	if (creator->algorithm == NULL) {
		const char *algorithm =
		    linphone_config_get_string(linphone_core_get_config(creator->core), "assistant", "algorithm", NULL);
		if (algorithm) {
			linphone_account_creator_set_algorithm(creator, algorithm);
		}
	}
}
