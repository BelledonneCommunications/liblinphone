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

#ifndef L_C_FRIEND_H_
#define L_C_FRIEND_H_

#include "linphone/api/c-types.h"
#include "linphone/callbacks.h"
#include "linphone/enums/c-enums.h"
#include "linphone/sipsetup.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup buddy_list
 * @{
 */

/**
 * Set #LinphoneAddress for this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param address the #LinphoneAddress to set @maybenil
 * return 0 if successful, -1 otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_friend_set_address(LinphoneFriend *linphone_friend,
                                                           const LinphoneAddress *address);

/**
 * Get address of this friend.
 * @note the #LinphoneAddress object returned is hold by the LinphoneFriend, however calling several time this function
 * may return different objects.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return the #LinphoneAddress. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_friend_get_address(const LinphoneFriend *linphone_friend);

/**
 * Adds an address in this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param address #LinphoneAddress object @notnil
 */
LINPHONE_PUBLIC void linphone_friend_add_address(LinphoneFriend *linphone_friend, const LinphoneAddress *address);

/**
 * Returns a list of #LinphoneAddress for this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return A list of #LinphoneAddress. \bctbx_list{LinphoneAddress} @maybenil
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_friend_get_addresses(const LinphoneFriend *linphone_friend);

/**
 * Returns a list of #LinphoneFriendDevice for this friend, for all known addresses.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return A list of #LinphoneFriendDevice. \bctbx_list{LinphoneFriendDevice} @tobefreed @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_friend_get_devices(const LinphoneFriend *linphone_friend);

/**
 * Returns a list of #LinphoneFriendDevice for this friend and a specific address.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param address #LinphoneAddress object @notnil
 * @return A list of #LinphoneFriendDevice. \bctbx_list{LinphoneFriendDevice} @tobefreed @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_friend_get_devices_for_address(const LinphoneFriend *linphone_friend,
                                                                      const LinphoneAddress *address);

/**
 * Returns the security level of a friend which is the lowest among all devices we know for it.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return A #LinphoneSecurityLevel, which is the lowest among all known devices.
 */
LINPHONE_PUBLIC LinphoneSecurityLevel linphone_friend_get_security_level(const LinphoneFriend *linphone_friend);

/**
 * Returns the security level of a friend for a given address which is the lowest among all devices we know for that
 * address.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param address #LinphoneAddress object @notnil
 * @return A #LinphoneSecurityLevel, which is the lowest among all known devices for that address.
 */
LINPHONE_PUBLIC LinphoneSecurityLevel
linphone_friend_get_security_level_for_address(const LinphoneFriend *linphone_friend, const LinphoneAddress *address);

/**
 * Removes an address in this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param address #LinphoneAddress object @notnil
 */
LINPHONE_PUBLIC void linphone_friend_remove_address(LinphoneFriend *linphone_friend, const LinphoneAddress *address);

/**
 * Adds a phone number in this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param phone_number number to add @notnil
 */
LINPHONE_PUBLIC void linphone_friend_add_phone_number(LinphoneFriend *linphone_friend, const char *phone_number);

/**
 * Adds a #LinphoneFriendPhoneNumber to this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param phone_number the #LinphoneFriendPhoneNumber to add @notnil
 */
LINPHONE_PUBLIC void linphone_friend_add_phone_number_with_label(LinphoneFriend *linphone_friend,
                                                                 LinphoneFriendPhoneNumber *phone_number);

/**
 * Returns a list of phone numbers for this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return A list of phone numbers as string. \bctbx_list{char *} @maybenil @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_friend_get_phone_numbers(const LinphoneFriend *linphone_friend);

/**
 * Returns a list of #LinphoneFriendPhoneNumber for this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return A list of phone numbers as string. \bctbx_list{LinphoneFriendPhoneNumber} @maybenil @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_friend_get_phone_numbers_with_label(const LinphoneFriend *linphone_friend);

/**
 * Returns whether a friend contains the given phone number
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param phone_number the phone number to search for @notnil
 * @return TRUE if found, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_friend_has_phone_number(const LinphoneFriend *linphone_friend,
                                                        const char *phone_number);

/**
 * Removes a phone number in this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param phone_number number to remove @notnil
 */
LINPHONE_PUBLIC void linphone_friend_remove_phone_number(LinphoneFriend *linphone_friend, const char *phone_number);

/**
 * Removes a #LinphoneFriendPhoneNumber from this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param phone_number the #LinphoneFriendPhoneNumber to remove @notnil
 */
LINPHONE_PUBLIC void linphone_friend_remove_phone_number_with_label(LinphoneFriend *linphone_friend,
                                                                    const LinphoneFriendPhoneNumber *phone_number);

/**
 * Sets the display name for this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param name the display name to set @maybenil
 * @return 0 if successful, -1 otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_friend_set_name(LinphoneFriend *linphone_friend, const char *name);

/**
 * Sets the last name for this friend if vCard is available
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param last_name the last name to set @maybenil
 * @return 0 if successful, -1 otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_friend_set_last_name(LinphoneFriend *linphone_friend, const char *last_name);

/**
 * Sets the first name for this friend is available
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param first_name the first name to set @maybenil
 * @return 0 if successful, -1 otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_friend_set_first_name(LinphoneFriend *linphone_friend, const char *first_name);

/**
 * Gets the display name for this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return The display name of this friend. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_get_name(const LinphoneFriend *linphone_friend);

/**
 * Gets the last name for this friend if vCard exists
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return The last name of this friend. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_get_last_name(const LinphoneFriend *linphone_friend);

/**
 * Gets the first name for this friend if vCard exists
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return The first name of this friend. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_get_first_name(const LinphoneFriend *linphone_friend);

/**
 * get subscription flag value
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return returns TRUE is subscription is activated for this friend
 */
LINPHONE_PUBLIC bool_t linphone_friend_subscribes_enabled(const LinphoneFriend *linphone_friend);
#define linphone_friend_get_send_subscribe linphone_friend_subscribes_enabled

/**
 * Configure #LinphoneFriend to subscribe to presence information
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param enable if TRUE this friend will receive subscription message
 * @return 0
 */
LINPHONE_PUBLIC LinphoneStatus linphone_friend_enable_subscribes(LinphoneFriend *linphone_friend, bool_t enable);
#define linphone_friend_send_subscribe linphone_friend_enable_subscribes

/**
 * Configure incoming subscription policy for this friend.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param policy #LinphoneSubscribePolicy policy to apply.
 * @return 0
 */
LINPHONE_PUBLIC LinphoneStatus linphone_friend_set_inc_subscribe_policy(LinphoneFriend *linphone_friend,
                                                                        LinphoneSubscribePolicy policy);

/**
 * get current subscription policy for this #LinphoneFriend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return the #LinphoneSubscribePolicy enum
 */
LINPHONE_PUBLIC LinphoneSubscribePolicy linphone_friend_get_inc_subscribe_policy(const LinphoneFriend *linphone_friend);

/**
 * Starts editing a friend configuration.
 *
 * Because friend configuration must be consistent, applications MUST
 * call linphone_friend_edit() before doing any attempts to modify
 * friend configuration (such as linphone_friend_set_address() or linphone_friend_set_inc_subscribe_policy()).
 * Once the modifications are done, then the application must call
 * linphone_friend_done() to commit the changes.
 * @param linphone_friend #LinphoneFriend object @notnil
 **/
LINPHONE_PUBLIC void linphone_friend_edit(LinphoneFriend *linphone_friend);

/**
 * Commits modification made to the friend configuration.
 * @param linphone_friend #LinphoneFriend object @notnil
 **/
LINPHONE_PUBLIC void linphone_friend_done(LinphoneFriend *linphone_friend);

/**
 * Get subscription state of a friend
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @return the #LinphoneSubscriptionState enum
 */
LINPHONE_PUBLIC LinphoneSubscriptionState linphone_friend_get_subscription_state(const LinphoneFriend *linphone_friend);

/**
 * Get the presence model of a friend. If a friend has more than one SIP address and phone number,
 * this method will return the most recent presence model using linphone_presence_model_get_timestamp().
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @return A #LinphonePresenceModel object, or NULL if the friend do not have presence information (in which case he is
 * considered offline). @maybenil
 */
LINPHONE_PUBLIC const LinphonePresenceModel *linphone_friend_get_presence_model(const LinphoneFriend *linphone_friend);

/**
 * Get the consolidated presence of a friend.
 * It will return the "most open" presence found if more than one presence model are found.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return The #LinphoneConsolidatedPresence of the friend
 */
LINPHONE_PUBLIC LinphoneConsolidatedPresence
linphone_friend_get_consolidated_presence(const LinphoneFriend *linphone_friend);

/**
 * Get the presence model for a specific SIP URI or phone number of a friend
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @param uri_or_tel The SIP URI or phone number for which to get the presence model @notnil
 * @return A #LinphonePresenceModel object, or NULL if the friend do not have presence information for this SIP URI or
 * phone number. @maybenil
 */
LINPHONE_PUBLIC const LinphonePresenceModel *
linphone_friend_get_presence_model_for_uri_or_tel(const LinphoneFriend *linphone_friend, const char *uri_or_tel);

/**
 * Set the presence model of a friend
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @param presence The #LinphonePresenceModel object to set for the friend @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_presence_model(LinphoneFriend *linphone_friend,
                                                        LinphonePresenceModel *presence);

/**
 * Set the presence model for a specific SIP URI or phone number of a friend
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @param uri_or_tel The SIP URI or phone number for which to set the presence model @notnil
 * @param presence The #LinphonePresenceModel object to set @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_presence_model_for_uri_or_tel(LinphoneFriend *linphone_friend,
                                                                       const char *uri_or_tel,
                                                                       LinphonePresenceModel *presence);

/**
 * Tells whether we already received presence information for a friend.
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @return TRUE if presence information has been received for the friend, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_friend_is_presence_received(const LinphoneFriend *linphone_friend);

/**
 * Store user pointer to friend object.
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @param user_data the user data to store. @maybenil
 **/
LINPHONE_PUBLIC void linphone_friend_set_user_data(LinphoneFriend *linphone_friend, void *user_data);

/**
 * Retrieve user data associated with friend.
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @return the user data pointer. @maybenil
 **/
LINPHONE_PUBLIC void *linphone_friend_get_user_data(const LinphoneFriend *linphone_friend);

LINPHONE_PUBLIC BuddyInfo *linphone_friend_get_info(const LinphoneFriend *linphone_friend);

/**
 * Set the reference key of a friend.
 * @param linphone_friend #LinphoneFriend object. @notnil
 * @param key The reference key to use for the friend. @maybenil
 **/
LINPHONE_PUBLIC void linphone_friend_set_ref_key(LinphoneFriend *linphone_friend, const char *key);

/**
 * Get the reference key of a friend.
 * @param linphone_friend #LinphoneFriend object. @notnil
 * @return The reference key of the friend. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_friend_get_ref_key(const LinphoneFriend *linphone_friend);

/**
 * Check that the given friend is in a friend list.
 * @param linphone_friend #LinphoneFriend object. @notnil
 * @return TRUE if the friend is in a friend list, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_friend_in_list(const LinphoneFriend *linphone_friend);

/**
 * Check that the given friend is in a friend list.
 * @param linphone_friend #LinphoneFriend object. @notnil
 * @return The #LinphoneFriendList the friend is in if any, NULL otherwise. @maybenil
 **/
LINPHONE_PUBLIC LinphoneFriendList *linphone_friend_get_friend_list(const LinphoneFriend *linphone_friend);

/**
 * Acquire a reference to the linphone friend.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return The same #LinphoneFriend object @notnil
 **/
LINPHONE_PUBLIC LinphoneFriend *linphone_friend_ref(LinphoneFriend *linphone_friend);

/**
 * Release a reference to the linphone friend.
 * @param linphone_friend #LinphoneFriend object @notnil
 **/
LINPHONE_PUBLIC void linphone_friend_unref(LinphoneFriend *linphone_friend);

/**
 * Returns the #LinphoneCore object managing this friend, if any.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return the #LinphoneCore object associated. @notnil
 */
LINPHONE_PUBLIC LinphoneCore *linphone_friend_get_core(const LinphoneFriend *linphone_friend);

/**
 * Returns the vCard object associated to this friend, if any
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return the #LinphoneVcard or NULL. @maybenil
 */
LINPHONE_PUBLIC LinphoneVcard *linphone_friend_get_vcard(const LinphoneFriend *linphone_friend);

/**
 * Returns the a string matching the vCard inside the friend, if any
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return the vCard as a string or NULL. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_dump_vcard(const LinphoneFriend *linphone_friend);

/**
 * Binds a vCard object to a friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param vcard The #LinphoneVcard object to bind @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_vcard(LinphoneFriend *linphone_friend, LinphoneVcard *vcard);

/**
 * Creates a vCard object associated to this friend if there isn't one yet and if the full name is available, either by
 * the parameter or the one in the friend's SIP URI
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param name The full name of the friend or NULL to use the one from the friend's SIP URI @maybenil
 * @return TRUE if the vCard has been created, FALSE if it wasn't possible (for exemple if name and the friend's SIP URI
 * are null or if the friend's SIP URI doesn't have a display name), or if there is already one vcard
 */
LINPHONE_PUBLIC bool_t linphone_friend_create_vcard(LinphoneFriend *linphone_friend, const char *name);

/**
 * Saves a friend either in database if configured, otherwise in linphonerc
 * @param linphone_friend the linphone friend to save @notnil
 * @param core the linphone core @notnil
 */
LINPHONE_PUBLIC void linphone_friend_save(LinphoneFriend *linphone_friend, LinphoneCore *core);

/**
 * Returns the capabilities associated to this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return an int representing the capabilities of the friend
 */
LINPHONE_PUBLIC int linphone_friend_get_capabilities(const LinphoneFriend *linphone_friend);

/**
 * Returns whether or not a friend has a capbility
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param capability #LinphoneFriendCapability object
 * @return whether or not a friend has a capbility
 */
LINPHONE_PUBLIC bool_t linphone_friend_has_capability(const LinphoneFriend *linphone_friend,
                                                      const LinphoneFriendCapability capability);

/**
 * Returns whether or not a friend has a capbility with a given version.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param capability #LinphoneFriendCapability object
 * @param version the version to test
 * @return whether or not a friend has a capbility with a given version or -1.0 if friend has not capability.
 */
LINPHONE_PUBLIC bool_t linphone_friend_has_capability_with_version(const LinphoneFriend *linphone_friend,
                                                                   const LinphoneFriendCapability capability,
                                                                   float version);

/**
 * Returns whether or not a friend has a capbility with a given version or more.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param capability #LinphoneFriendCapability object
 * @param version the version to test
 * @return whether or not a friend has a capbility with a given version or more.
 */
LINPHONE_PUBLIC bool_t linphone_friend_has_capability_with_version_or_more(const LinphoneFriend *linphone_friend,
                                                                           const LinphoneFriendCapability capability,
                                                                           float version);

/**
 * Returns the version of a friend's capbility.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param capability #LinphoneFriendCapability object
 * @return the version of a friend's capbility.
 */
LINPHONE_PUBLIC float linphone_friend_get_capability_version(const LinphoneFriend *linphone_friend,
                                                             const LinphoneFriendCapability capability);

/**
 * Removes a friend from it's friend list and from the rc if exists
 * @param linphone_friend #LinphoneFriend object to delete @notnil
 */
LINPHONE_PUBLIC void linphone_friend_remove(LinphoneFriend *linphone_friend);

/**
 * Sets the contact's picture URI
 * @param linphone_friend the #LinphoneFriend object.
 * @param picture_uri the picture URI to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_photo(LinphoneFriend *linphone_friend, const char *picture_uri);

/**
 * Gets the contact's picture URI
 * @param linphone_friend the #LinphoneFriend object.
 * @return the picture URI set if any, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_get_photo(const LinphoneFriend *linphone_friend);

/**
 * Sets if the friend is a user's favorite or important contact.
 * @param linphone_friend the #LinphoneFriend object.
 * @param is_starred TRUE if the friend is to be considered as important, FALSE otherwise.
 */
LINPHONE_PUBLIC void linphone_friend_set_starred(LinphoneFriend *linphone_friend, bool_t is_starred);

/**
 * Gets if the friend is to be considered as important for the user.
 * @param linphone_friend the #LinphoneFriend object.
 * @return TRUE if the contact is a user's favorite, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_friend_get_starred(const LinphoneFriend *linphone_friend);

/**
 * Sets the contact's native URI.
 * @param linphone_friend the #LinphoneFriend object.
 * @param native_uri the URI that matches the contact on the native system. @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_native_uri(LinphoneFriend *linphone_friend, const char *native_uri);

/**
 * Gets the contact's native URI
 * @param linphone_friend the #LinphoneFriend object.
 * @return the native URI set if any, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_get_native_uri(const LinphoneFriend *linphone_friend);

/**
 * Sets the contact's organization.
 * It's a shortcut to linphone_friend_get_vcard() and linphone_vcard_set_organization().
 * @param linphone_friend the #LinphoneFriend object.
 * @param organization the organization to store in Friend's vCard. @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_organization(LinphoneFriend *linphone_friend, const char *organization);

/**
 * Gets the contact's organization from it's vCard.
 * It's a shortcut to linphone_friend_get_vcard() and linphone_vcard_get_organization().
 * @param linphone_friend the #LinphoneFriend object.
 * @return the organization set if any & vCard is available, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_get_organization(const LinphoneFriend *linphone_friend);

/**
 * Sets the contact's job title.
 * It's a shortcut to linphone_friend_get_vcard() and linphone_vcard_set_job_title().
 * @param linphone_friend the #LinphoneFriend object.
 * @param job_title the job title to store in Friend's vCard. @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_job_title(LinphoneFriend *linphone_friend, const char *job_title);

/**
 * Gets the contact's job title from it's vCard.
 * It's a shortcut to linphone_friend_get_vcard() and linphone_vcard_get_job_title().
 * @param linphone_friend the #LinphoneFriend object.
 * @return the job_title set if any & vCard is available, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_get_job_title(const LinphoneFriend *linphone_friend);

/************ */
/* Friend CBS */
/* ********** */

/**
 * Adds the #LinphoneFriendCbs object associated with a LinphoneFriend.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param cbs The current #LinphoneFriendCbs object to be added to the LinphoneFriend. @notnil
 **/
LINPHONE_PUBLIC void linphone_friend_add_callbacks(LinphoneFriend *linphone_friend, LinphoneFriendCbs *cbs);

/**
 * Removes the #LinphoneFriendCbs object associated with a LinphoneFriend.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param cbs The current #LinphoneFriendCbs object to be remove from the LinphoneFriend. @notnil
 **/
LINPHONE_PUBLIC void linphone_friend_remove_callbacks(LinphoneFriend *linphone_friend, LinphoneFriendCbs *cbs);

/**
 * Get the current #LinphoneFriendCbs object associated with a LinphoneFriend.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return The current #LinphoneFriendCbs object associated with the LinphoneFriend. @maybenil
 **/
LINPHONE_PUBLIC LinphoneFriendCbs *linphone_friend_get_current_callbacks(const LinphoneFriend *linphone_friend);

/**
 * Acquire a reference to a #LinphoneFriendCbs object.
 * @param cbs #LinphoneFriendCbs object. @notnil
 * @return The same #LinphoneFriendCbs object.
 **/
LINPHONE_PUBLIC LinphoneFriendCbs *linphone_friend_cbs_ref(LinphoneFriendCbs *cbs);

/**
 * Release a reference to a #LinphoneFriendCbs object.
 * @param cbs #LinphoneFriendCbs object. @notnil
 **/
LINPHONE_PUBLIC void linphone_friend_cbs_unref(LinphoneFriendCbs *cbs);

/**
 * Retrieve the user pointer associated with a #LinphoneFriendCbs object.
 * @param cbs #LinphoneFriendCbs object. @notnil
 * @return The user pointer associated with the #LinphoneFriendCbs object. @maybenil
 **/
LINPHONE_PUBLIC void *linphone_friend_cbs_get_user_data(const LinphoneFriendCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneFriendCbs object.
 * @param cbs #LinphoneFriendCbs object. @notnil
 * @param user_data The user pointer to associate with the #LinphoneFriendCbs object. @maybenil
 **/
LINPHONE_PUBLIC void linphone_friend_cbs_set_user_data(LinphoneFriendCbs *cbs, void *user_data);

/**
 * Get the presence received callback.
 * @param cbs #LinphoneFriendCbs object. @notnil
 * @return The current presence received callback.
 **/
LINPHONE_PUBLIC LinphoneFriendCbsPresenceReceivedCb
linphone_friend_cbs_get_presence_received(const LinphoneFriendCbs *cbs);

/**
 * Set the presence received callback.
 * @param cbs #LinphoneFriendCbs object. @notnil
 * @param cb The presence received callback to be used.
 **/
LINPHONE_PUBLIC void linphone_friend_cbs_set_presence_received(LinphoneFriendCbs *cbs,
                                                               LinphoneFriendCbsPresenceReceivedCb cb);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Get the status of a friend
 * @param linphone_friend A #LinphoneFriend object
 * @return #LinphoneOnlineStatus
 * @deprecated 19/06/2013 Use linphone_friend_get_presence_model() instead
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneOnlineStatus
linphone_friend_get_status(const LinphoneFriend *linphone_friend);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* L_C_FRIEND_H_ */
