#include "linphone/account_creator.h"
#include "linphone/core.h"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

using namespace LinphonePrivate;

const char *_get_domain(LinphoneAccountCreator *creator) {
	if (creator->domain)
		return creator->domain;
	else if (creator->proxy_cfg)
		return linphone_proxy_config_get_domain(creator->proxy_cfg);
	return NULL;
}

const char* ha1_for_passwd(const char* username, const char* realm, const char* passwd, const char* algo) {
	if(algo==NULL || strcmp(algo, "MD5")==0) {
		static char ha1[33];
		sal_auth_compute_ha1(username, realm, passwd, ha1);
		return ha1;
	}
	else if(strcmp(algo, "SHA-256")==0) {
		static char ha1[65];
		sal_auth_compute_ha1_for_algorithm(username, realm, passwd, ha1, 65, algo);
		return ha1;
	}
	else {
		return NULL;
	}
}

unsigned int validate_uri(const char* username, const char* domain, const char* display_name) {
	LinphoneAddress* addr;
	unsigned int status = 0;
	LinphoneProxyConfig* proxy = linphone_core_create_proxy_config(NULL);
	addr = linphone_address_new("sip:?@domain.com");
	linphone_proxy_config_set_identity_address(proxy, addr);
	if (addr) linphone_address_unref(addr);

	if (username) {
		addr = linphone_proxy_config_normalize_sip_uri(proxy, username);
	} else {
		addr = linphone_address_clone(linphone_proxy_config_get_identity_address(proxy));
	}

	if (addr == NULL) {
		status = 1;
		goto end;
	}

	if (domain && linphone_address_set_domain(addr, domain) != 0) {
		status = 1;
	}

	if (display_name && (!strlen(display_name) || linphone_address_set_display_name(addr, display_name) != 0)) {
		status = 1;
	}
	linphone_address_unref(addr);
end:
	linphone_proxy_config_unref(proxy);
	return status;
}

char* _get_identity(const LinphoneAccountCreator *creator) {
	char *identity = NULL;
	if ((creator->username || creator->phone_number)) {
		//we must escape username
		LinphoneProxyConfig* proxy = linphone_core_create_proxy_config(creator->core);
		LinphoneAddress* addr;

		addr = linphone_proxy_config_normalize_sip_uri(proxy, creator->username ? creator->username : creator->phone_number);
		if (addr == NULL || (creator->domain && strcmp(linphone_address_get_domain(addr), creator->domain) != 0)) {
			if ((creator->username || creator->phone_number) && creator->domain) {
				char *url = ms_strdup_printf("sip:%s", creator->domain);
				addr = linphone_address_new(url);
				ms_free(url);

				if (addr) {
					linphone_address_set_username(addr, creator->username ? creator->username : creator->phone_number);
				} else {
					goto end;
				}
			} else {
				goto end;
			}
		}

		identity = linphone_address_as_string(addr);
		linphone_address_unref(addr);
		end:
		linphone_proxy_config_unref(proxy);
	}
	return identity;
}

void reset_field (char **field) {
	if (*field) {
		bctbx_free(*field);
		*field = nullptr;
	}
}

void fill_domain_and_algorithm_if_needed(LinphoneAccountCreator *creator) {
	if (creator->domain == NULL) {
		const char* domain = linphone_config_get_string(linphone_core_get_config(creator->core), "assistant", "domain", NULL);
		if (domain) {
			linphone_account_creator_set_domain(creator, domain);
		}
	}
	if (creator->algorithm == NULL) {
		const char* algorithm = linphone_config_get_string(linphone_core_get_config(creator->core), "assistant", "algorithm", NULL);
		if (algorithm) {
			linphone_account_creator_set_algorithm(creator, algorithm);
		}
	}
}
