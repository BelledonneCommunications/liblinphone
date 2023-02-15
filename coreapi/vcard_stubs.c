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

#include <bctoolbox/defs.h>

#include "linphone/core.h"
#include "linphone/wrapper_utils.h"
#include "vcard_private.h"

struct _LinphoneVcardContext {
	void *user_data;
};

LinphoneVcardContext *linphone_vcard_context_new(void) {
	LinphoneVcardContext *context = ms_new0(LinphoneVcardContext, 1);
	context->user_data = NULL;
	return context;
}

void linphone_vcard_context_destroy(BCTBX_UNUSED(LinphoneVcardContext *context)) {
	if (context) {
		ms_free(context);
	}
}

void *linphone_vcard_context_get_user_data(BCTBX_UNUSED(const LinphoneVcardContext *context)) {
	return context ? context->user_data : NULL;
}

void linphone_vcard_context_set_user_data(BCTBX_UNUSED(LinphoneVcardContext *context), BCTBX_UNUSED(void *data)) {
	if (context) context->user_data = data;
}

struct _LinphoneVcard {
	void *dummy;
};

LinphoneVcard *_linphone_vcard_new(void) {
	return NULL;
}

LinphoneVcard *linphone_vcard_new(void) {
	return NULL;
}

void linphone_vcard_free(BCTBX_UNUSED(LinphoneVcard *vCard)) {
}

LinphoneVcard *linphone_vcard_ref(BCTBX_UNUSED(LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_unref(BCTBX_UNUSED(LinphoneVcard *vCard)) {
}

bctbx_list_t *linphone_vcard_context_get_vcard_list_from_file(BCTBX_UNUSED(LinphoneVcardContext *context),
                                                              BCTBX_UNUSED(const char *filename)) {
	return NULL;
}

bctbx_list_t *linphone_vcard_context_get_vcard_list_from_buffer(BCTBX_UNUSED(LinphoneVcardContext *context),
                                                                BCTBX_UNUSED(const char *buffer)) {
	return NULL;
}

LinphoneVcard *linphone_vcard_context_get_vcard_from_buffer(BCTBX_UNUSED(LinphoneVcardContext *context),
                                                            BCTBX_UNUSED(const char *buffer)) {
	return NULL;
}

const char *linphone_vcard_as_vcard4_string(BCTBX_UNUSED(LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_set_full_name(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *name)) {
}

const char *linphone_vcard_get_full_name(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_set_skip_validation(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(bool_t skip)) {
}

bool_t linphone_vcard_get_skip_validation(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return FALSE;
}

void linphone_vcard_set_family_name(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *name)) {
}

const char *linphone_vcard_get_family_name(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_set_given_name(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *name)) {
}

const char *linphone_vcard_get_given_name(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_add_sip_address(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *sip_address)) {
}

void linphone_vcard_remove_sip_address(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *sip_address)) {
}

void linphone_vcard_edit_main_sip_address(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *sip_address)) {
}

const bctbx_list_t *linphone_vcard_get_sip_addresses(BCTBX_UNUSED(LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_add_phone_number(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *phone)) {
}

void linphone_vcard_remove_phone_number(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *phone)) {
}

bctbx_list_t *linphone_vcard_get_phone_numbers(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_add_phone_number_with_label(BCTBX_UNUSED(LinphoneVcard *vCard),
                                                BCTBX_UNUSED(LinphoneFriendPhoneNumber *phoneNumber)) {
}

void linphone_vcard_remove_phone_number_with_label(BCTBX_UNUSED(LinphoneVcard *vCard),
                                                   BCTBX_UNUSED(const LinphoneFriendPhoneNumber *phoneNumber)) {
}

bctbx_list_t *linphone_vcard_get_phone_numbers_with_label(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_set_organization(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *organization)) {
}

const char *linphone_vcard_get_organization(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_remove_organization(BCTBX_UNUSED(LinphoneVcard *vCard)) {
}

void linphone_vcard_set_photo(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *picture)) {
}

void linphone_vcard_remove_photo(BCTBX_UNUSED(LinphoneVcard *vCard)) {
}

const char *linphone_vcard_get_photo(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

bool_t linphone_vcard_generate_unique_id(BCTBX_UNUSED(LinphoneVcard *vCard)) {
	return FALSE;
}

void linphone_vcard_set_uid(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *uid)) {
}

const char *linphone_vcard_get_uid(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_set_etag(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *etag)) {
}

const char *linphone_vcard_get_etag(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_set_url(BCTBX_UNUSED(LinphoneVcard *vCard), BCTBX_UNUSED(const char *url)) {
}

const char *linphone_vcard_get_url(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

void linphone_vcard_compute_md5_hash(BCTBX_UNUSED(LinphoneVcard *vCard)) {
}

bool_t linphone_vcard_compare_md5_hash(BCTBX_UNUSED(LinphoneVcard *vCard)) {
	return FALSE;
}

bool_t linphone_core_vcard_supported(void) {
	return FALSE;
}

void linphone_vcard_clean_cache(BCTBX_UNUSED(LinphoneVcard *vCard)) {
}

void *linphone_vcard_get_belcard(BCTBX_UNUSED(LinphoneVcard *vCard)) {
	return NULL;
}

LinphoneVcard *linphone_vcard_clone(BCTBX_UNUSED(const LinphoneVcard *vCard)) {
	return NULL;
}

bctbx_list_t *linphone_vcard_get_extended_properties_values_by_name(BCTBX_UNUSED(const LinphoneVcard *vCard),
                                                                    BCTBX_UNUSED(const char *name)) {
	return NULL;
}

void linphone_vcard_add_extended_property(BCTBX_UNUSED(LinphoneVcard *vCard),
                                          BCTBX_UNUSED(const char *name),
                                          BCTBX_UNUSED(const char *value)) {
}

void linphone_vcard_remove_extented_properties_by_name(BCTBX_UNUSED(LinphoneVcard *vCard),
                                                       BCTBX_UNUSED(const char *name)) {
}
