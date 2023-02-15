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

#include "linphone/api/c-digest-authentication-policy.h"
#include "private.h"

#define FROM_BELLE_SIP_DIGEST_AUTHENTICATION_POLICY(obj) ((LinphoneDigestAuthenticationPolicy *)obj)
#define TO_BELLE_SIP_DIGEST_AUTHENTICATION_POLICY(obj) ((belle_sip_digest_authentication_policy_t *)obj)

#define DIGEST_AUTHENTICATION_POLICY_USER_DATA_KEY "_linphone_user_data"

LinphoneDigestAuthenticationPolicy *linphone_digest_authentication_policy_new(void) {
	belle_sip_digest_authentication_policy_t *policy = belle_sip_digest_authentication_policy_new();
	belle_sip_object_ref(policy); // This object is initially unowned in belle-sip.
	return FROM_BELLE_SIP_DIGEST_AUTHENTICATION_POLICY(policy);
}

static constexpr const char *digest_authentication_policy_key = "digest_authentication_policy";

LinphoneDigestAuthenticationPolicy *linphone_digest_authentication_policy_new_from_config(LinphoneConfig *config) {
	LinphoneDigestAuthenticationPolicy *policy = linphone_digest_authentication_policy_new();
	linphone_digest_authentication_policy_set_allow_md5(
	    policy, linphone_config_get_bool(config, digest_authentication_policy_key, "allow_md5", TRUE));
	linphone_digest_authentication_policy_set_allow_no_qop(
	    policy, linphone_config_get_bool(config, digest_authentication_policy_key, "allow_no_qop", TRUE));
	return policy;
}

void linphone_digest_authentication_policy_save(const LinphoneDigestAuthenticationPolicy *policy,
                                                LinphoneConfig *config) {
	linphone_config_set_bool(config, digest_authentication_policy_key, "allow_md5",
	                         linphone_digest_authentication_policy_get_allow_md5(policy));
	linphone_config_set_bool(config, digest_authentication_policy_key, "allow_no_qop",
	                         linphone_digest_authentication_policy_get_allow_no_qop(policy));
}

LinphoneDigestAuthenticationPolicy *
linphone_digest_authentication_policy_ref(LinphoneDigestAuthenticationPolicy *policy) {
	belle_sip_object_ref(TO_BELLE_SIP_DIGEST_AUTHENTICATION_POLICY(policy));
	return policy;
}

void linphone_digest_authentication_policy_unref(LinphoneDigestAuthenticationPolicy *policy) {
	belle_sip_object_unref(TO_BELLE_SIP_DIGEST_AUTHENTICATION_POLICY(policy));
}

void linphone_digest_authentication_policy_set_allow_md5(LinphoneDigestAuthenticationPolicy *policy, bool_t value) {
	belle_sip_digest_authentication_policy_set_allow_md5(TO_BELLE_SIP_DIGEST_AUTHENTICATION_POLICY(policy), value);
}

bool_t linphone_digest_authentication_policy_get_allow_md5(const LinphoneDigestAuthenticationPolicy *policy) {
	return belle_sip_digest_authentication_policy_get_allow_md5(TO_BELLE_SIP_DIGEST_AUTHENTICATION_POLICY(policy));
}

void linphone_digest_authentication_policy_set_allow_no_qop(LinphoneDigestAuthenticationPolicy *policy, bool_t value) {
	belle_sip_digest_authentication_policy_set_allow_no_qop(TO_BELLE_SIP_DIGEST_AUTHENTICATION_POLICY(policy), value);
}

bool_t linphone_digest_authentication_policy_get_allow_no_qop(const LinphoneDigestAuthenticationPolicy *policy) {
	return belle_sip_digest_authentication_policy_get_allow_no_qop(TO_BELLE_SIP_DIGEST_AUTHENTICATION_POLICY(policy));
}

void *linphone_digest_authentication_policy_get_user_data(const LinphoneDigestAuthenticationPolicy *policy) {
	return belle_sip_object_data_get(BELLE_SIP_OBJECT(policy), DIGEST_AUTHENTICATION_POLICY_USER_DATA_KEY);
}

void linphone_digest_authentication_policy_set_user_data(LinphoneDigestAuthenticationPolicy *policy, void *user_data) {
	belle_sip_object_data_set(BELLE_SIP_OBJECT(policy), DIGEST_AUTHENTICATION_POLICY_USER_DATA_KEY, user_data, NULL);
}
