/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include <bctoolbox/crypto.h>

#include "friend/friend-phone-number.h"
#include "linphone/factory.h"
#include "linphone/wrapper_utils.h"
#include "vcard/vcard-context.h"
#include "vcard/vcard.h"

using namespace LinphonePrivate;

LinphoneVcard *linphone_vcard_context_get_vcard_from_buffer(LinphoneVcardContext *context, const char *buffer) {
	if (!context) return nullptr;
	std::shared_ptr<Vcard> vcard = VcardContext::toCpp(context)->getVcardFromBuffer(L_C_TO_STRING(buffer));
	return vcard ? linphone_vcard_ref(vcard->toC()) : nullptr;
}

LinphoneVcard *linphone_vcard_ref(LinphoneVcard *vCard) {
	Vcard::toCpp(vCard)->ref();
	return vCard;
}

void linphone_vcard_unref(LinphoneVcard *vCard) {
	Vcard::toCpp(vCard)->unref();
}

LinphoneVcard *linphone_vcard_clone(const LinphoneVcard *vCard) {
	return Vcard::toCpp(vCard)->clone()->toC();
}

const char *linphone_vcard_as_vcard4_string(LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->asVcard4String());
}

const char *linphone_vcard_as_vcard4_string_with_base_64_picture(LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->asVcard4StringWithBase64Picture());
}

void *linphone_vcard_get_belcard(LinphoneVcard *vCard) {
	return Vcard::toCpp(vCard)->getBelcard();
}

void linphone_vcard_set_full_name(LinphoneVcard *vCard, const char *name) {
	if (!vCard || !name) return;
	Vcard::toCpp(vCard)->setFullName(L_C_TO_STRING(name));
}

const char *linphone_vcard_get_full_name(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->getFullName());
}

void linphone_vcard_set_family_name(LinphoneVcard *vCard, const char *name) {
	if (!vCard || !name) return;
	Vcard::toCpp(vCard)->setFamilyName(L_C_TO_STRING(name));
}

const char *linphone_vcard_get_family_name(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->getFamilyName());
}

void linphone_vcard_set_given_name(LinphoneVcard *vCard, const char *name) {
	if (!vCard || !name) return;
	Vcard::toCpp(vCard)->setGivenName(L_C_TO_STRING(name));
}

const char *linphone_vcard_get_given_name(const LinphoneVcard *vCard) {
	if (!vCard) return NULL;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->getGivenName());
}

void linphone_vcard_add_sip_address(LinphoneVcard *vCard, const char *sip_address) {
	if (!vCard || !sip_address) return;
	Vcard::toCpp(vCard)->addSipAddress(L_C_TO_STRING(sip_address));
}

void linphone_vcard_remove_sip_address(LinphoneVcard *vCard, const char *sip_address) {
	if (!vCard || !sip_address) return;
	Vcard::toCpp(vCard)->removeSipAddress(L_C_TO_STRING(sip_address));
}

void linphone_vcard_edit_main_sip_address(LinphoneVcard *vCard, const char *sip_address) {
	if (!vCard || !sip_address) return;
	Vcard::toCpp(vCard)->editMainSipAddress(L_C_TO_STRING(sip_address));
}

const bctbx_list_t *linphone_vcard_get_sip_addresses(LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	Vcard::toCpp(vCard)->getSipAddresses();
	return Vcard::toCpp(vCard)->mSipAddresses.getCList();
}

void linphone_vcard_add_phone_number(LinphoneVcard *vCard, const char *phone) {
	if (!vCard || !phone) return;
	Vcard::toCpp(vCard)->addPhoneNumber(L_C_TO_STRING(phone));
}

void linphone_vcard_remove_phone_number(LinphoneVcard *vCard, const char *phone) {
	if (!vCard || !phone) return;
	Vcard::toCpp(vCard)->removePhoneNumber(L_C_TO_STRING(phone));
}

bctbx_list_t *linphone_vcard_get_phone_numbers(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	const std::list<std::string> phoneNumbers = Vcard::toCpp(vCard)->getPhoneNumbers();
	bctbx_list_t *result = nullptr;
	for (auto &phoneNumber : phoneNumbers)
		result = bctbx_list_append(result, bctbx_strdup(phoneNumber.c_str()));
	return result;
}

void linphone_vcard_add_phone_number_with_label(LinphoneVcard *vCard, const LinphoneFriendPhoneNumber *phoneNumber) {
	if (!vCard || !phoneNumber) return;
	Vcard::toCpp(vCard)->addPhoneNumberWithLabel(FriendPhoneNumber::getSharedFromThis(phoneNumber));
}

void linphone_vcard_remove_phone_number_with_label(LinphoneVcard *vCard, const LinphoneFriendPhoneNumber *phoneNumber) {
	if (!vCard || !phoneNumber) return;
	Vcard::toCpp(vCard)->removePhoneNumberWithLabel(FriendPhoneNumber::getSharedFromThis(phoneNumber));
}

bctbx_list_t *linphone_vcard_get_phone_numbers_with_label(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	std::list<std::shared_ptr<FriendPhoneNumber>> phoneNumbers = Vcard::toCpp(vCard)->getPhoneNumbersWithLabel();
	bctbx_list_t *result = nullptr;
	for (const auto &phoneNumber : phoneNumbers)
		result = bctbx_list_append(result, phoneNumber->toC());
	return result;
}

void linphone_vcard_set_organization(LinphoneVcard *vCard, const char *organization) {
	if (!vCard) return;
	Vcard::toCpp(vCard)->setOrganization(L_C_TO_STRING(organization));
}

const char *linphone_vcard_get_organization(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->getOrganization());
}

void linphone_vcard_remove_organization(LinphoneVcard *vCard) {
	if (!vCard) return;
	Vcard::toCpp(vCard)->removeOrganization();
}

void linphone_vcard_set_job_title(LinphoneVcard *vCard, const char *job_title) {
	if (!vCard) return;
	Vcard::toCpp(vCard)->setJobTitle(L_C_TO_STRING(job_title));
}

const char *linphone_vcard_get_job_title(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->getJobTitle());
}

void linphone_vcard_remove_job_title(LinphoneVcard *vCard) {
	if (!vCard) return;
	Vcard::toCpp(vCard)->removeJobTitle();
}

void linphone_vcard_set_photo(LinphoneVcard *vCard, const char *picture) {
	if (!vCard) return;
	Vcard::toCpp(vCard)->setPhoto(L_C_TO_STRING(picture));
}

void linphone_vcard_remove_photo(LinphoneVcard *vCard) {
	if (!vCard) return;
	Vcard::toCpp(vCard)->removePhoto();
}

const char *linphone_vcard_get_photo(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->getPhoto());
}

bool_t linphone_vcard_generate_unique_id(LinphoneVcard *vCard) {
	if (!vCard) return FALSE;
	return Vcard::toCpp(vCard)->generateUniqueId();
}

void linphone_vcard_set_uid(LinphoneVcard *vCard, const char *uid) {
	if (!vCard || !uid) return;
	Vcard::toCpp(vCard)->setUid(L_C_TO_STRING(uid));
}

const char *linphone_vcard_get_uid(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->getUid());
}

void linphone_vcard_set_etag(LinphoneVcard *vCard, const char *etag) {
	if (!vCard) return;
	Vcard::toCpp(vCard)->setEtag(L_C_TO_STRING(etag));
}

const char *linphone_vcard_get_etag(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->getEtag());
}

void linphone_vcard_set_url(LinphoneVcard *vCard, const char *url) {
	if (!vCard) return;
	Vcard::toCpp(vCard)->setUrl(L_C_TO_STRING(url));
}

const char *linphone_vcard_get_url(const LinphoneVcard *vCard) {
	if (!vCard) return nullptr;
	return L_STRING_TO_C(Vcard::toCpp(vCard)->getUrl());
}

bctbx_list_t *linphone_vcard_get_extended_properties_values_by_name(const LinphoneVcard *vCard, const char *name) {
	std::list<std::string> values = Vcard::toCpp(vCard)->getExtendedPropertiesValuesByName(L_C_TO_STRING(name));
	bctbx_list_t *result = nullptr;
	for (const auto &value : values)
		result = bctbx_list_append(result, bctbx_strdup(value.c_str()));
	return result;
}

void linphone_vcard_add_extended_property(LinphoneVcard *vCard, const char *name, const char *value) {
	if (!vCard || !name || !value) return;
	Vcard::toCpp(vCard)->addExtendedProperty(L_C_TO_STRING(name), L_C_TO_STRING(value));
}

void linphone_vcard_remove_extented_properties_by_name(LinphoneVcard *vCard, const char *name) {
	if (!vCard) return;
	Vcard::toCpp(vCard)->removeExtentedPropertiesByName(name);
}

bool_t linphone_core_vcard_supported(void) {
#ifdef VCARD_ENABLED
	return TRUE;
#else
	return FALSE;
#endif /* VCARD_ENABLED */
}
