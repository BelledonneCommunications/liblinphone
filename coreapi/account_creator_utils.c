#include "linphone/account_creator.h"
#include "linphone/core.h"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

using namespace LinphonePrivate;

/************************** Start Misc **************************/
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

void resetField (char **field) {
	if (*field) {
		bctbx_free(*field);
		*field = nullptr;
	}
}

LinphoneProxyConfig * linphone_account_creator_create_proxy_config(const LinphoneAccountCreator *creator) {
	LinphoneAuthInfo *info;
	LinphoneProxyConfig *cfg = linphone_core_create_proxy_config(creator->core);
	char *identity_str = _get_identity(creator);
	LinphoneAddress *identity = linphone_address_new(identity_str);

	ms_free(identity_str);
	if (creator->display_name) {
		linphone_address_set_display_name(identity, creator->display_name);
	}
	linphone_proxy_config_set_identity_address(cfg, identity);
	if (creator->phone_country_code) {
		linphone_proxy_config_set_dial_prefix(cfg, creator->phone_country_code);
	} else if (creator->phone_number) {
		int dial_prefix_number = DialPlan::lookupCccFromE164(creator->phone_number);
		char buff[4];
		snprintf(buff, sizeof(buff), "%d", dial_prefix_number);
		linphone_proxy_config_set_dial_prefix(cfg, buff);
	}
	if (linphone_proxy_config_get_server_addr(cfg) == NULL && creator->domain != NULL) {
		char *url = ms_strdup_printf("sip:%s", creator->domain);
		LinphoneAddress *proxy_addr = linphone_address_new(url);
		if (proxy_addr) {
			linphone_address_set_transport(proxy_addr, creator->transport);
			linphone_proxy_config_set_server_addr(cfg, linphone_address_as_string_uri_only(proxy_addr));
			linphone_address_unref(proxy_addr);
		} else {
			linphone_proxy_config_set_server_addr(cfg, creator->domain);
		}
		ms_free(url);
	}

	linphone_proxy_config_enable_register(cfg, TRUE);

	info = linphone_auth_info_new_for_algorithm(linphone_address_get_username(identity), // username
								NULL, //user id
								creator->password, // passwd
								creator->password ? NULL : creator->ha1,  // ha1
								!creator->password && creator->ha1 ? linphone_address_get_domain(identity) : NULL,  // realm - assumed to be domain
								linphone_address_get_domain(identity), // domain
								creator->password ? NULL : creator->algorithm //if clear text password is given, allow its usage with all algorithms.
	);
	linphone_core_add_auth_info(creator->core, info);
	linphone_address_unref(identity);

	if (linphone_core_add_proxy_config(creator->core, cfg) != -1) {
		if (creator->set_as_default) {
			linphone_core_set_default_proxy_config(creator->core, cfg);
		}
		return cfg;
	}

	linphone_core_remove_auth_info(creator->core, info);
	linphone_auth_info_unref(info);
	return NULL;
}

LinphoneProxyConfig * linphone_account_creator_configure(const LinphoneAccountCreator *creator) {
	return linphone_account_creator_create_proxy_config(creator);
}
/************************** End Misc **************************/