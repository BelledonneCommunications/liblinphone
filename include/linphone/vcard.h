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

#ifndef LINPHONE_VCARD_H
#define LINPHONE_VCARD_H

#include "linphone/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup carddav_vcard
 * @{
 */

/**
 * Cast a belle_sip_object_t into LinphoneVcard.
 */
#define LINPHONE_VCARD BELLE_SIP_CAST(object, LinphoneVcard)

/**
 * Take a ref on a #LinphoneVcard.
 * @param vCard #LinphoneVcard object @notnil
 * @return the same #LinphoneVcard object @notnil
 */
LINPHONE_PUBLIC LinphoneVcard *linphone_vcard_ref(LinphoneVcard *vCard);

/**
 * Release a #LinphoneVcard.
 * @param vCard #LinphoneVcard object @notnil
 */
LINPHONE_PUBLIC void linphone_vcard_unref(LinphoneVcard *vCard);

/**
 * Clone a #LinphoneVcard.
 * @param vCard #LinphoneVcard object @notnil
 * @return a new #LinphoneVcard object @notnil
 */
LINPHONE_PUBLIC LinphoneVcard *linphone_vcard_clone(const LinphoneVcard *vCard);

/**
 * Returns the vCard4 representation of the LinphoneVcard.
 * @param vCard the #LinphoneVcard @notnil
 * @return a const char * that represents the vCard. @maybenil
 */
LINPHONE_PUBLIC const char* linphone_vcard_as_vcard4_string(LinphoneVcard *vCard);

/**
 * Sets the FN attribute of the vCard (which is mandatory).
 * @param vCard the #LinphoneVcard @notnil
 * @param name the display name to set for the vCard @maybenil
 */
LINPHONE_PUBLIC void linphone_vcard_set_full_name(LinphoneVcard *vCard, const char *name);

/**
 * Returns the FN attribute of the vCard, or NULL if it isn't set yet.
 * @param vCard the #LinphoneVcard @notnil
 * @return the display name of the vCard, or NULL. @maybenil
 */
LINPHONE_PUBLIC const char* linphone_vcard_get_full_name(const LinphoneVcard *vCard);

/**
 * Sets the skipFieldValidation property of the vcard
 * @param vCard the #LinphoneVcard @notnil
 * @param skip skipFieldValidation property of the vcard
 */
LINPHONE_PUBLIC void linphone_vcard_set_skip_validation(LinphoneVcard *vCard, bool_t skip);

/**
 * Returns the skipFieldValidation property of the vcard.
 * @param vCard the #LinphoneVcard @notnil
 * @return the skipFieldValidation property of the vcard
 */
LINPHONE_PUBLIC bool_t linphone_vcard_get_skip_validation(const LinphoneVcard *vCard);

/**
 * Sets the family name in the N attribute of the vCard.
 * @param vCard the #LinphoneVcard @notnil
 * @param name the family name to set for the vCard @maybenil
 */
LINPHONE_PUBLIC void linphone_vcard_set_family_name(LinphoneVcard *vCard, const char *name);

/**
 * Returns the family name in the N attribute of the vCard, or NULL if it isn't set yet.
 * @param vCard the #LinphoneVcard @notnil
 * @return the family name of the vCard, or NULL @maybenil
 */
LINPHONE_PUBLIC const char* linphone_vcard_get_family_name(const LinphoneVcard *vCard);

/**
 * Sets the given name in the N attribute of the vCard.
 * @param vCard the #LinphoneVcard @notnil
 * @param name the given name to set for the vCard @maybenil
 */
LINPHONE_PUBLIC void linphone_vcard_set_given_name(LinphoneVcard *vCard, const char *name);

/**
 * Returns the given name in the N attribute of the vCard, or NULL if it isn't set yet.
 * @param vCard the #LinphoneVcard @notnil
 * @return the given name of the vCard, or NULL @maybenil
 */
LINPHONE_PUBLIC const char* linphone_vcard_get_given_name(const LinphoneVcard *vCard);

/**
 * Adds a SIP address in the vCard, using the IMPP property
 * @param vCard the #LinphoneVcard @notnil
 * @param sip_address the SIP address to add @notnil
 */
LINPHONE_PUBLIC void linphone_vcard_add_sip_address(LinphoneVcard *vCard, const char *sip_address);

/**
 * Removes a SIP address in the vCard (if it exists), using the IMPP property
 * @param vCard the #LinphoneVcard @notnil
 * @param sip_address the SIP address to remove @notnil
 */
LINPHONE_PUBLIC void linphone_vcard_remove_sip_address(LinphoneVcard *vCard, const char *sip_address);

/**
 * Edits the preferred SIP address in the vCard (or the first one), using the IMPP property
 * @param vCard the #LinphoneVcard @notnil
 * @param sip_address the new SIP address @notnil
 */
LINPHONE_PUBLIC void linphone_vcard_edit_main_sip_address(LinphoneVcard *vCard, const char *sip_address);

/**
 * Returns the list of SIP addresses in the vCard (all the IMPP attributes that has an URI value starting by "sip:") or NULL
 * @param vCard the #LinphoneVcard @notnil
 * @return The SIP addresses. \bctbx_list{LinphoneAddress} @maybenil
 */
LINPHONE_PUBLIC const bctbx_list_t* linphone_vcard_get_sip_addresses(LinphoneVcard *vCard);

/**
 * Adds a phone number in the vCard, using the TEL property
 * @param vCard the #LinphoneVcard @notnil
 * @param phone the phone number to add @notnil
 */
LINPHONE_PUBLIC void linphone_vcard_add_phone_number(LinphoneVcard *vCard, const char *phone);

/**
 * Removes a phone number in the vCard (if it exists), using the TEL property
 * @param vCard the #LinphoneVcard @notnil
 * @param phone the phone number to remove @notnil
 */
LINPHONE_PUBLIC void linphone_vcard_remove_phone_number(LinphoneVcard *vCard, const char *phone);

/**
 * Returns the list of phone numbers in the vCard (all the TEL attributes) or NULL
 * @param vCard the #LinphoneVcard @notnil
 * @return The phone numbers as string. \bctbx_list{const char *} @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t* linphone_vcard_get_phone_numbers(const LinphoneVcard *vCard);

/**
 * Fills the Organization field of the vCard
 * @param vCard the #LinphoneVcard @notnil
 * @param organization the Organization. @maybenil
 */
LINPHONE_PUBLIC void linphone_vcard_set_organization(LinphoneVcard *vCard, const char *organization);

/**
 * Gets the Organization of the vCard
 * @param vCard the #LinphoneVcard @notnil
 * @return the Organization of the vCard or NULL. @maybenil
 */
LINPHONE_PUBLIC const char* linphone_vcard_get_organization(const LinphoneVcard *vCard);

/**
 * Removes the Organization field of the vCard
 * @param vCard the #LinphoneVcard @notnil
 */
LINPHONE_PUBLIC void linphone_vcard_remove_organization(LinphoneVcard *vCard);

/**
 * Generates a random unique id for the vCard.
 * If is required to be able to synchronize the vCard with a CardDAV server
 * @param vCard the #LinphoneVcard @notnil
 * @return TRUE if operation is successful, otherwise FALSE (for example if it already has an unique ID)
 */
LINPHONE_PUBLIC bool_t linphone_vcard_generate_unique_id(LinphoneVcard *vCard);

/**
 * Sets the unique ID of the vCard
 * @param vCard the #LinphoneVcard @notnil
 * @param uid the unique id @maybenil
 */
LINPHONE_PUBLIC void linphone_vcard_set_uid(LinphoneVcard *vCard, const char *uid);

/**
 * Gets the UID of the vCard
 * @param vCard the #LinphoneVcard @notnil
 * @return the UID of the vCard, otherwise NULL. @maybenil
 */
LINPHONE_PUBLIC const char* linphone_vcard_get_uid(const LinphoneVcard *vCard);

/**
 * Sets the eTAG of the vCard
 * @param vCard the #LinphoneVcard @notnil
 * @param etag the eTAG. @maybenil
 */
LINPHONE_PUBLIC void linphone_vcard_set_etag(LinphoneVcard *vCard, const char * etag);

/**
 * Gets the eTag of the vCard
 * @param vCard the #LinphoneVcard @notnil
 * @return the eTag of the vCard in the CardDAV server, otherwise NULL. @maybenil
 */
LINPHONE_PUBLIC const char* linphone_vcard_get_etag(const LinphoneVcard *vCard);

/**
 * Sets the URL of the vCard
 * @param vCard the #LinphoneVcard @notnil
 * @param url the URL. @maybenil
 */
LINPHONE_PUBLIC void linphone_vcard_set_url(LinphoneVcard *vCard, const char *url);

/**
 * Gets the URL of the vCard
 * @param vCard the #LinphoneVcard @notnil
 * @return the URL of the vCard in the CardDAV server, otherwise NULL. @maybenil
 */
LINPHONE_PUBLIC const char* linphone_vcard_get_url(const LinphoneVcard *vCard);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
