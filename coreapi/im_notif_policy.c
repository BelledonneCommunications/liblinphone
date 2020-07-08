/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#include "linphone/core.h"

#include "c-wrapper/c-wrapper.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneImNotifPolicy);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneImNotifPolicy, belle_sip_object_t,
	NULL, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

static void load_im_notif_policy_from_config(LinphoneImNotifPolicy *policy) {
#ifdef HAVE_ADVANCED_IM
	bctbx_list_t *default_list = bctbx_list_append(NULL, (void *)"all");
	bctbx_list_t *values = linphone_config_get_string_list(policy->lc->config, "sip", "im_notif_policy", default_list);
	bctbx_list_t *elem;

	for (elem = values; elem != NULL; elem = bctbx_list_next(elem)) {
		const char *value = (const char *)bctbx_list_get_data(elem);
		if (strcasecmp(value, "all") == 0) {
			policy->send_is_composing = TRUE;
			policy->recv_is_composing = TRUE;
			policy->send_imdn_delivered = TRUE;
			policy->recv_imdn_delivered = TRUE;
			policy->send_imdn_displayed = TRUE;
			policy->recv_imdn_displayed = TRUE;
		} else if (strcasecmp(value, "none") == 0) {
			policy->send_is_composing = FALSE;
			policy->recv_is_composing = FALSE;
			policy->send_imdn_delivered = FALSE;
			policy->recv_imdn_delivered = FALSE;
			policy->send_imdn_displayed = FALSE;
			policy->recv_imdn_displayed = FALSE;
		} else if (strcasecmp(value, "send_is_comp") == 0) {
			policy->send_is_composing = TRUE;
		} else if (strcasecmp(value, "recv_is_comp") == 0) {
			policy->recv_is_composing = TRUE;
		} else if (strcasecmp(value, "send_imdn_delivered") == 0) {
			policy->send_imdn_delivered = TRUE;
		} else if (strcasecmp(value, "recv_imdn_delivered") == 0) {
			policy->recv_imdn_delivered = TRUE;
		} else if (strcasecmp(value, "send_imdn_displayed") == 0) {
			policy->send_imdn_displayed = TRUE;
		} else if (strcasecmp(value, "recv_imdn_displayed") == 0) {
			policy->recv_imdn_displayed = TRUE;
		}
	}
	if (values != default_list) {
		bctbx_list_free_with_data(values, ms_free);
	}
	bctbx_list_free(default_list);
#else
	policy->send_is_composing = FALSE;
	policy->recv_is_composing = FALSE;
	policy->send_imdn_delivered = FALSE;
	policy->recv_imdn_delivered = FALSE;
	policy->send_imdn_displayed = FALSE;
	policy->recv_imdn_displayed = FALSE;
#endif
}

static void save_im_notif_policy_to_config(LinphoneImNotifPolicy *policy) {
#ifdef HAVE_ADVANCED_IM
	bctbx_list_t *values = NULL;
	if ((policy->send_is_composing == TRUE)
		&& (policy->recv_is_composing == TRUE)
		&& (policy->send_imdn_delivered == TRUE)
		&& (policy->recv_imdn_delivered == TRUE)
		&& (policy->send_imdn_displayed == TRUE)
		&& (policy->recv_imdn_displayed == TRUE)) {
		/* Do not save anything, the default is everything enabled */
	} else if ((policy->send_is_composing == FALSE)
		&& (policy->recv_is_composing == FALSE)
		&& (policy->send_imdn_delivered == FALSE)
		&& (policy->recv_imdn_delivered == FALSE)
		&& (policy->send_imdn_displayed == FALSE)
		&& (policy->recv_imdn_displayed == FALSE)) {
		values = bctbx_list_append(values, (void *)"none");
	} else {
		if (policy->send_is_composing == TRUE)
			values = bctbx_list_append(values, (void *)"send_is_comp");
		if (policy->recv_is_composing == TRUE)
			values = bctbx_list_append(values, (void *)"recv_is_comp");
		if (policy->send_imdn_delivered == TRUE)
			values = bctbx_list_append(values, (void *)"send_imdn_delivered");
		if (policy->recv_imdn_delivered == TRUE)
			values = bctbx_list_append(values, (void *)"recv_imdn_delivered");
		if (policy->send_imdn_displayed == TRUE)
			values = bctbx_list_append(values, (void *)"send_imdn_displayed");
		if (policy->recv_imdn_displayed == TRUE)
			values = bctbx_list_append(values, (void *)"recv_imdn_displayed");
	}
	linphone_config_set_string_list(policy->lc->config, "sip", "im_notif_policy", values);
	if (values != NULL) bctbx_list_free(values);
#endif
}

LinphoneImNotifPolicy * linphone_im_notif_policy_ref(LinphoneImNotifPolicy *policy) {
	belle_sip_object_ref(policy);
	return policy;
}

void linphone_im_notif_policy_unref(LinphoneImNotifPolicy *policy) {
	belle_sip_object_unref(policy);
}

void *linphone_im_notif_policy_get_user_data(const LinphoneImNotifPolicy *policy) {
	return policy->user_data;
}

void linphone_im_notif_policy_set_user_data(LinphoneImNotifPolicy *policy, void *ud) {
	policy->user_data = ud;
}


void linphone_im_notif_policy_clear(LinphoneImNotifPolicy *policy) {
	policy->send_is_composing = FALSE;
	policy->recv_is_composing = FALSE;
	policy->send_imdn_delivered = FALSE;
	policy->recv_imdn_delivered = FALSE;
	policy->send_imdn_displayed = FALSE;
	policy->recv_imdn_displayed = FALSE;
	save_im_notif_policy_to_config(policy);
}

void linphone_im_notif_policy_enable_all(LinphoneImNotifPolicy *policy) {
#ifdef HAVE_ADVANCED_IM
	policy->send_is_composing = TRUE;
	policy->recv_is_composing = TRUE;
	policy->send_imdn_delivered = TRUE;
	policy->recv_imdn_delivered = TRUE;
	policy->send_imdn_displayed = TRUE;
	policy->recv_imdn_displayed = TRUE;
	save_im_notif_policy_to_config(policy);
#else
	ms_warning("Cannot enable im policy because ENABLE_ADVANCED_IM is OFF");
#endif
}

bool_t linphone_im_notif_policy_get_send_is_composing(const LinphoneImNotifPolicy *policy) {
	return policy->send_is_composing;
}

void linphone_im_notif_policy_set_send_is_composing(LinphoneImNotifPolicy *policy, bool_t enable) {
	policy->send_is_composing = enable;
	save_im_notif_policy_to_config(policy);
}

bool_t linphone_im_notif_policy_get_recv_is_composing(const LinphoneImNotifPolicy *policy) {
	return policy->recv_is_composing;
}

void linphone_im_notif_policy_set_recv_is_composing(LinphoneImNotifPolicy *policy, bool_t enable) {
	policy->recv_is_composing = enable;
	save_im_notif_policy_to_config(policy);
}

bool_t linphone_im_notif_policy_get_send_imdn_delivered(const LinphoneImNotifPolicy *policy) {
	return policy->send_imdn_delivered;
}

void linphone_im_notif_policy_set_send_imdn_delivered(LinphoneImNotifPolicy *policy, bool_t enable) {
	policy->send_imdn_delivered = enable;
	save_im_notif_policy_to_config(policy);
}

bool_t linphone_im_notif_policy_get_recv_imdn_delivered(const LinphoneImNotifPolicy *policy) {
	return policy->recv_imdn_delivered;
}

void linphone_im_notif_policy_set_recv_imdn_delivered(LinphoneImNotifPolicy *policy, bool_t enable) {
	policy->recv_imdn_delivered = enable;
	save_im_notif_policy_to_config(policy);
}

bool_t linphone_im_notif_policy_get_send_imdn_displayed(const LinphoneImNotifPolicy *policy) {
	return policy->send_imdn_displayed;
}

void linphone_im_notif_policy_set_send_imdn_displayed(LinphoneImNotifPolicy *policy, bool_t enable) {
	policy->send_imdn_displayed = enable;
	save_im_notif_policy_to_config(policy);
}

bool_t linphone_im_notif_policy_get_recv_imdn_displayed(const LinphoneImNotifPolicy *policy) {
	return policy->recv_imdn_displayed;
}

void linphone_im_notif_policy_set_recv_imdn_displayed(LinphoneImNotifPolicy *policy, bool_t enable) {
	policy->recv_imdn_displayed = enable;
	save_im_notif_policy_to_config(policy);
}

LinphoneImNotifPolicy * linphone_core_get_im_notif_policy(const LinphoneCore *lc) {
	return lc->im_notif_policy;
}

void linphone_core_create_im_notif_policy(LinphoneCore *lc) {
	if (lc->im_notif_policy)
		linphone_im_notif_policy_unref(lc->im_notif_policy);
	lc->im_notif_policy = belle_sip_object_new(LinphoneImNotifPolicy);
	lc->im_notif_policy->lc = lc;
	load_im_notif_policy_from_config(lc->im_notif_policy);
	save_im_notif_policy_to_config(lc->im_notif_policy);
}
