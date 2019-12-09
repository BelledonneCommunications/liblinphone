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

#include <bctoolbox/crypto.h>

#include <belcard/belcard_parser.hpp>
#include <belcard/belcard.hpp>

#include "linphone/factory.h"
#include "linphone/wrapper_utils.h"

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-sal.h"
#include "vcard_private.h"

#define VCARD_MD5_HASH_SIZE 16

using namespace std;

struct _LinphoneVcardContext {
	shared_ptr<belcard::BelCardParser> parser;
	void *user_data;
};

extern "C" {

LinphoneVcardContext* linphone_vcard_context_new(void) {
	LinphoneVcardContext* context = ms_new0(LinphoneVcardContext, 1);
	new (&context->parser) shared_ptr<belcard::BelCardParser>(belcard::BelCardParser::getInstance());
	context->user_data = NULL;
	return context;
}

void linphone_vcard_context_destroy(LinphoneVcardContext *context) {
	if (context) {
		context->parser.~shared_ptr<belcard::BelCardParser>();
		ms_free(context);
	}
}

void* linphone_vcard_context_get_user_data(const LinphoneVcardContext *context) {
	return context ? context->user_data : NULL;
}


void linphone_vcard_context_set_user_data(LinphoneVcardContext *context, void *data) {
	if (context) context->user_data = data;
}

} // extern "C"


struct _LinphoneVcard {
	belle_sip_object_t base;
	shared_ptr<belcard::BelCard> belCard;
	char *etag;
	char *url;
	unsigned char md5[VCARD_MD5_HASH_SIZE];
	bctbx_list_t *sip_addresses_cache;
};

extern "C" {

static void _linphone_vcard_uninit(LinphoneVcard *vCard) {
	if (vCard->etag) ms_free(vCard->etag);
	if (vCard->url) ms_free(vCard->url);
	linphone_vcard_clean_cache(vCard);
	vCard->belCard.~shared_ptr<belcard::BelCard>();
}

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneVcard);
BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneVcard);
BELLE_SIP_INSTANCIATE_VPTR(LinphoneVcard, belle_sip_object_t,
	_linphone_vcard_uninit, // destroy
	NULL, // clone
	NULL, // Marshall
	FALSE
);

LinphoneVcard* _linphone_vcard_new(void) {
	LinphoneVcard* vCard = belle_sip_object_new(LinphoneVcard);
	new (&vCard->belCard) shared_ptr<belcard::BelCard>(belcard::BelCardGeneric::create<belcard::BelCard>());
	return vCard;
}

LinphoneVcard *linphone_vcard_new(void) {
	return _linphone_vcard_new();
}

static LinphoneVcard* linphone_vcard_new_from_belcard(shared_ptr<belcard::BelCard> belcard) {
	LinphoneVcard* vCard = belle_sip_object_new(LinphoneVcard);
	new (&vCard->belCard) shared_ptr<belcard::BelCard>(belcard);
	return vCard;
}

void linphone_vcard_free(LinphoneVcard *vCard) {
	belle_sip_object_unref((belle_sip_object_t *)vCard);
}

LinphoneVcard *linphone_vcard_ref(LinphoneVcard *vCard) {
	return (LinphoneVcard *)belle_sip_object_ref((belle_sip_object_t *)vCard);
}

void linphone_vcard_unref(LinphoneVcard *vCard) {
	belle_sip_object_unref((belle_sip_object_t *)vCard);
}

LinphoneVcard *linphone_vcard_clone(const LinphoneVcard *vCard) {
	LinphoneVcard *copy = belle_sip_object_new(LinphoneVcard);

	new (&copy->belCard) shared_ptr<belcard::BelCard>(belcard::BelCardParser::getInstance()->parseOne(vCard->belCard->toFoldedString()));

	if (vCard->url) copy->url = ms_strdup(vCard->url);
	if (vCard->etag) copy->etag = ms_strdup(vCard->etag);

	memcpy(copy->md5, vCard->md5, sizeof *vCard->md5);

	return copy;
}

bctbx_list_t* linphone_vcard_context_get_vcard_list_from_file(LinphoneVcardContext *context, const char *filename) {
	bctbx_list_t *result = NULL;
	if (context && filename) {
		if (!context->parser) {
			context->parser = belcard::BelCardParser::getInstance();
		}
		shared_ptr<belcard::BelCardList> belCards = context->parser->parseFile(filename);
		if (belCards) {
			for (auto &belCard : belCards->getCards())
				result = bctbx_list_append(result, linphone_vcard_new_from_belcard(belCard));
		}
	}
	return result;
}

bctbx_list_t* linphone_vcard_context_get_vcard_list_from_buffer(LinphoneVcardContext *context, const char *buffer) {
	bctbx_list_t *result = NULL;
	if (context && buffer) {
		if (!context->parser) {
			context->parser = belcard::BelCardParser::getInstance();
		}
		shared_ptr<belcard::BelCardList> belCards = context->parser->parse(buffer);
		if (belCards) {
			for (auto &belCard : belCards->getCards())
				result = bctbx_list_append(result, linphone_vcard_new_from_belcard(belCard));
		}
	}
	return result;
}

LinphoneVcard* linphone_vcard_context_get_vcard_from_buffer(LinphoneVcardContext *context, const char *buffer) {
	LinphoneVcard *vCard = NULL;
	if (context && buffer) {
		if (!context->parser) {
			context->parser = belcard::BelCardParser::getInstance();
		}
		shared_ptr<belcard::BelCard> belCard = context->parser->parseOne(buffer);
		if (belCard) {
			vCard = linphone_vcard_new_from_belcard(belCard);
		} else {
			ms_error("Couldn't parse buffer %s", buffer);
		}
	}
	return vCard;
}

const char * linphone_vcard_as_vcard4_string(LinphoneVcard *vCard) {
	if (!vCard) return NULL;

	return vCard->belCard->toFoldedString().c_str();
}

void *linphone_vcard_get_belcard(LinphoneVcard *vcard) {
	return &vcard->belCard;
}

void linphone_vcard_set_full_name(LinphoneVcard *vCard, const char *name) {
	if (!vCard || !name) return;

	if (vCard->belCard->getFullName()) {
		vCard->belCard->getFullName()->setValue(name);
	} else {
		shared_ptr<belcard::BelCardFullName> fn = belcard::BelCardGeneric::create<belcard::BelCardFullName>();
		fn->setValue(name);
		vCard->belCard->setFullName(fn);
	}
}

const char* linphone_vcard_get_full_name(const LinphoneVcard *vCard) {
	if (!vCard) return NULL;

	const char *result = vCard->belCard->getFullName() ? vCard->belCard->getFullName()->getValue().c_str() : NULL;
	return result;
}

void linphone_vcard_set_skip_validation(LinphoneVcard *vCard, bool_t skip) {
	if (!vCard || !vCard->belCard) return;

	vCard->belCard->setSkipFieldValidation((skip == TRUE) ? true : false);
}

bool_t linphone_vcard_get_skip_validation(const LinphoneVcard *vCard) {
	if (!vCard) return FALSE;

	bool_t result = vCard->belCard->getSkipFieldValidation();
	return result;
}

void linphone_vcard_set_family_name(LinphoneVcard *vCard, const char *name) {
	if (!vCard || !name) return;

	if (vCard->belCard->getName()) {
		vCard->belCard->getName()->setFamilyName(name);
	} else {
		shared_ptr<belcard::BelCardName> n = belcard::BelCardGeneric::create<belcard::BelCardName>();
		n->setFamilyName(name);
		vCard->belCard->setName(n);
	}
}

const char* linphone_vcard_get_family_name(const LinphoneVcard *vCard) {
	if (!vCard) return NULL;

	const char *result = vCard->belCard->getName() ? vCard->belCard->getName()->getFamilyName().c_str() : NULL;
	return result;
}

void linphone_vcard_set_given_name(LinphoneVcard *vCard, const char *name) {
	if (!vCard || !name) return;

	if (vCard->belCard->getName()) {
		vCard->belCard->getName()->setGivenName(name);
	} else {
		shared_ptr<belcard::BelCardName> n = belcard::BelCardGeneric::create<belcard::BelCardName>();
		n->setGivenName(name);
		vCard->belCard->setName(n);
	}
}

const char* linphone_vcard_get_given_name(const LinphoneVcard *vCard) {
	if (!vCard) return NULL;

	const char *result = vCard->belCard->getName() ? vCard->belCard->getName()->getGivenName().c_str() : NULL;
	return result;
}

void linphone_vcard_add_sip_address(LinphoneVcard *vCard, const char *sip_address) {
	if (!vCard || !sip_address) return;

	shared_ptr<belcard::BelCardImpp> impp = belcard::BelCardGeneric::create<belcard::BelCardImpp>();
	impp->setValue(sip_address);
	if (!vCard->belCard->addImpp(impp)) {
		ms_error("Couldn't add IMPP value %s to vCard [%p]", sip_address, vCard);
	}
}

void linphone_vcard_remove_sip_address(LinphoneVcard *vCard, const char *sip_address) {
	if (!vCard) return;

	for (auto &impp : vCard->belCard->getImpp()) {
		const char *value = impp->getValue().c_str();
		if (strcmp(value, sip_address) == 0) {
			vCard->belCard->removeImpp(impp);
			break;
		}
	}
}

void linphone_vcard_edit_main_sip_address(LinphoneVcard *vCard, const char *sip_address) {
	if (!vCard || !sip_address) return;

	if (vCard->belCard->getImpp().size() > 0) {
		const shared_ptr<belcard::BelCardImpp> impp = vCard->belCard->getImpp().front();
		impp->setValue(sip_address);
	} else {
		shared_ptr<belcard::BelCardImpp> impp = belcard::BelCardGeneric::create<belcard::BelCardImpp>();
		impp->setValue(sip_address);
		if (!vCard->belCard->addImpp(impp)) {
			ms_error("Couldn't add IMPP value %s to vCard [%p]", sip_address, vCard);
		}
	}
}

const bctbx_list_t* linphone_vcard_get_sip_addresses(LinphoneVcard *vCard) {
	if (!vCard) return NULL;
	if (!vCard->sip_addresses_cache) {
		for (auto &impp : vCard->belCard->getImpp()) {
			LinphoneAddress* addr = linphone_address_new(impp->getValue().c_str());
			if (addr) {
				vCard->sip_addresses_cache = bctbx_list_append(vCard->sip_addresses_cache, addr);
			}
		}
	}
	return vCard->sip_addresses_cache;
}

void linphone_vcard_add_phone_number(LinphoneVcard *vCard, const char *phone) {
	if (!vCard || !phone) return;

	shared_ptr<belcard::BelCardPhoneNumber> phone_number = belcard::BelCardGeneric::create<belcard::BelCardPhoneNumber>();
	phone_number->setValue(phone);
	vCard->belCard->addPhoneNumber(phone_number);
}

void linphone_vcard_remove_phone_number(LinphoneVcard *vCard, const char *phone) {
	if (!vCard) return;

	shared_ptr<belcard::BelCardPhoneNumber> tel;
	for (auto &phoneNumber : vCard->belCard->getPhoneNumbers()) {
		const char *value = phoneNumber->getValue().c_str();
		if (strcmp(value, phone) == 0) {
			vCard->belCard->removePhoneNumber(phoneNumber);
			break;
		}
	}
}

bctbx_list_t* linphone_vcard_get_phone_numbers(const LinphoneVcard *vCard) {
	bctbx_list_t *result = NULL;
	if (!vCard) return NULL;

	for (auto &phoneNumber : vCard->belCard->getPhoneNumbers()) {
		const char *value = phoneNumber->getValue().c_str();
		result = bctbx_list_append(result, (char *)value);
	}
	return result;
}

void linphone_vcard_set_organization(LinphoneVcard *vCard, const char *organization) {
	if (!vCard) return;

	if (vCard->belCard->getOrganizations().size() > 0) {
		const shared_ptr<belcard::BelCardOrganization> org = vCard->belCard->getOrganizations().front();
		org->setValue(organization);
	} else {
		shared_ptr<belcard::BelCardOrganization> org = belcard::BelCardGeneric::create<belcard::BelCardOrganization>();
		org->setValue(organization);
		vCard->belCard->addOrganization(org);
	}
}

const char* linphone_vcard_get_organization(const LinphoneVcard *vCard) {
	if (vCard && vCard->belCard->getOrganizations().size() > 0) {
		const shared_ptr<belcard::BelCardOrganization> org = vCard->belCard->getOrganizations().front();
		return org->getValue().c_str();
	}

	return NULL;
}

bool_t linphone_vcard_generate_unique_id(LinphoneVcard *vCard) {
	if (vCard) {
		if (linphone_vcard_get_uid(vCard)) {
			return FALSE;
		}
		string uuid = "urn:" + LinphonePrivate::Sal::generateUuid();
		linphone_vcard_set_uid(vCard, uuid.c_str());
		return TRUE;
	}
	return FALSE;
}

void linphone_vcard_set_uid(LinphoneVcard *vCard, const char *uid) {
	if (!vCard || !uid) return;

	shared_ptr<belcard::BelCardUniqueId> uniqueId = belcard::BelCardGeneric::create<belcard::BelCardUniqueId>();
	uniqueId->setValue(uid);
	vCard->belCard->setUniqueId(uniqueId);
}

const char* linphone_vcard_get_uid(const LinphoneVcard *vCard) {
	if (vCard && vCard->belCard->getUniqueId()) {
		return vCard->belCard->getUniqueId()->getValue().c_str();
	}
	return NULL;
}

void linphone_vcard_set_etag(LinphoneVcard *vCard, const char * etag) {
	if (!vCard) {
		return;
	}
	if (vCard->etag) {
		ms_free(vCard->etag);
		vCard->etag = NULL;
	}
	vCard->etag = ms_strdup(etag);
}

const char* linphone_vcard_get_etag(const LinphoneVcard *vCard) {
	if (!vCard) return NULL;
	return vCard->etag;
}

void linphone_vcard_set_url(LinphoneVcard *vCard, const char * url) {
	if (!vCard) {
		return;
	}
	if (vCard->url) {
		ms_free(vCard->url);
		vCard->url = NULL;
	}
	vCard->url = ms_strdup(url);
}

const char* linphone_vcard_get_url(const LinphoneVcard *vCard) {
	if (!vCard) return NULL;
	return vCard->url;
}

void linphone_vcard_compute_md5_hash(LinphoneVcard *vCard) {
	const char *text = NULL;
	if (!vCard) return;
	text = linphone_vcard_as_vcard4_string(vCard);
	bctbx_md5((unsigned char *)text, strlen(text), vCard->md5);
}

bool_t linphone_vcard_compare_md5_hash(LinphoneVcard *vCard) {
	unsigned char previous_md5[VCARD_MD5_HASH_SIZE];
	memcpy(previous_md5, vCard->md5, VCARD_MD5_HASH_SIZE);
	linphone_vcard_compute_md5_hash(vCard);
	return !!memcmp(vCard->md5, previous_md5, VCARD_MD5_HASH_SIZE);
}

bool_t linphone_core_vcard_supported(void) {
	return TRUE;
}

void linphone_vcard_clean_cache(LinphoneVcard *vCard) {
	if (vCard->sip_addresses_cache) bctbx_list_free_with_data(vCard->sip_addresses_cache, (void (*)(void*))linphone_address_unref);
	vCard->sip_addresses_cache = NULL;
}

} // extern "C"
