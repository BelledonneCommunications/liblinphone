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

#ifndef LINPHONE_FRIEND_H_
#define LINPHONE_FRIEND_H_

#include "linphone/types.h"
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
 * return 0 if successfull, -1 otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_friend_set_address(LinphoneFriend *fr, const LinphoneAddress* address);

/**
 * Get address of this friend.
 * @note the #LinphoneAddress object returned is hold by the LinphoneFriend, however calling several time this function may return different objects.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return the #LinphoneAddress. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress * linphone_friend_get_address(const LinphoneFriend *linphone_friend);

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
LINPHONE_PUBLIC const bctbx_list_t* linphone_friend_get_addresses(const LinphoneFriend *linphone_friend);

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
 * Returns a list of phone numbers for this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return A list of phone numbers as string. \bctbx_list{const char *} @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t* linphone_friend_get_phone_numbers(const LinphoneFriend *linphone_friend);

/**
 * Returns whether a friend contains the given phone number
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param phone_number the phone number to search for @notnil
 * @return TRUE if found, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_friend_has_phone_number(const LinphoneFriend *linphone_friend, const char *phone_number);

/**
 * Removes a phone number in this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param phone_number number to remove @notnil
 */
LINPHONE_PUBLIC void linphone_friend_remove_phone_number(LinphoneFriend *linphone_friend, const char *phone_number);

/**
 * Set the display name for this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param name the display name to set @maybenil
 * @return 0 if successful, -1 otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_friend_set_name(LinphoneFriend *linphone_friend, const char *name);

/**
 * Get the display name for this friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return The display name of this friend. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_friend_get_name(const LinphoneFriend *linphone_friend);

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
LINPHONE_PUBLIC	LinphoneStatus linphone_friend_enable_subscribes(LinphoneFriend *fr, bool_t enable);
#define linphone_friend_send_subscribe linphone_friend_enable_subscribes

/**
 * Configure incoming subscription policy for this friend.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param policy #LinphoneSubscribePolicy policy to apply.
 * @return 0
 */
LINPHONE_PUBLIC LinphoneStatus linphone_friend_set_inc_subscribe_policy(LinphoneFriend *fr, LinphoneSubscribePolicy policy);

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
LINPHONE_PUBLIC	void linphone_friend_edit(LinphoneFriend *linphone_friend);

/**
 * Commits modification made to the friend configuration.
 * @param linphone_friend #LinphoneFriend object @notnil
**/
LINPHONE_PUBLIC	void linphone_friend_done(LinphoneFriend *linphone_friend);

/**
 * Get subscription state of a friend
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @return the #LinphoneSubscriptionState enum
 */
LINPHONE_PUBLIC LinphoneSubscriptionState linphone_friend_get_subscription_state(const LinphoneFriend *linphone_friend);

/**
 * Get the presence model of a friend
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @return A #LinphonePresenceModel object, or NULL if the friend do not have presence information (in which case he is considered offline). @maybenil
 */
LINPHONE_PUBLIC const LinphonePresenceModel * linphone_friend_get_presence_model(const LinphoneFriend *linphone_friend);

/**
 * Get the consolidated presence of a friend.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return The #LinphoneConsolidatedPresence of the friend
 */
LINPHONE_PUBLIC LinphoneConsolidatedPresence linphone_friend_get_consolidated_presence(const LinphoneFriend *linphone_friend);

/**
 * Get the presence model for a specific SIP URI or phone number of a friend
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @param uri_or_tel The SIP URI or phone number for which to get the presence model @notnil
 * @return A #LinphonePresenceModel object, or NULL if the friend do not have presence information for this SIP URI or phone number. @maybenil
 */
LINPHONE_PUBLIC const LinphonePresenceModel * linphone_friend_get_presence_model_for_uri_or_tel(const LinphoneFriend *linphone_friend, const char *uri_or_tel);

/**
 * Set the presence model of a friend
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @param presence The #LinphonePresenceModel object to set for the friend @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_presence_model(LinphoneFriend *linphone_friend, LinphonePresenceModel *presence);

/**
 * Set the presence model for a specific SIP URI or phone number of a friend
 * @param linphone_friend A #LinphoneFriend object @notnil
 * @param uri_or_tel The SIP URI or phone number for which to set the presence model @notnil
 * @param presence The #LinphonePresenceModel object to set @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_presence_model_for_uri_or_tel(LinphoneFriend *linphone_friend, const char *uri_or_tel, LinphonePresenceModel *presence);

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
LINPHONE_PUBLIC void* linphone_friend_get_user_data(const LinphoneFriend *linphone_friend);

LINPHONE_PUBLIC BuddyInfo * linphone_friend_get_info(const LinphoneFriend *linphone_friend);

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
 * Acquire a reference to the linphone friend.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @return The same #LinphoneFriend object @notnil
**/
LINPHONE_PUBLIC LinphoneFriend * linphone_friend_ref(LinphoneFriend *linphone_friend);

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
LINPHONE_PUBLIC LinphoneVcard* linphone_friend_get_vcard(const LinphoneFriend *linphone_friend);

/**
 * Binds a vCard object to a friend
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param vcard The #LinphoneVcard object to bind @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_set_vcard(LinphoneFriend *linphone_friend, LinphoneVcard *vcard);

/**
 * Creates a vCard object associated to this friend if there isn't one yet and if the full name is available, either by the parameter or the one in the friend's SIP URI
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param name The full name of the friend or NULL to use the one from the friend's SIP URI @maybenil
 * @return TRUE if the vCard has been created, FALSE if it wasn't possible (for exemple if name and the friend's SIP URI are null or if the friend's SIP URI doesn't have a display name), or if there is already one vcard
 */
LINPHONE_PUBLIC bool_t linphone_friend_create_vcard(LinphoneFriend *linphone_friend, const char *name);

/**
 * Contructor same as linphone_friend_new() + linphone_friend_set_address()
 * @param vcard a #LinphoneVcard object @notnil
 * @return a new #LinphoneFriend which has its vCard attribute initialized from the given vCard.
 * This can be get by linphone_friend_get_vcard(). @maybenil
 */
LINPHONE_PUBLIC	LinphoneFriend *linphone_friend_new_from_vcard(LinphoneVcard *vcard);

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
LINPHONE_PUBLIC bool_t linphone_friend_has_capability(const LinphoneFriend *linphone_friend, const LinphoneFriendCapability capability);

/**
 * Returns whether or not a friend has a capbility with a given version.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param capability #LinphoneFriendCapability object
 * @param version the version to test
 * @return whether or not a friend has a capbility with a given version or -1.0 if friend has not capability.
 */
LINPHONE_PUBLIC bool_t linphone_friend_has_capability_with_version(const LinphoneFriend *linphone_friend, const LinphoneFriendCapability capability, float version);

/**
 * Returns whether or not a friend has a capbility with a given version or more.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param capability #LinphoneFriendCapability object
 * @param version the version to test
 * @return whether or not a friend has a capbility with a given version or more.
 */
LINPHONE_PUBLIC bool_t linphone_friend_has_capability_with_version_or_more(const LinphoneFriend *linphone_friend, const LinphoneFriendCapability capability, float version);

/**
 * Returns the version of a friend's capbility.
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param capability #LinphoneFriendCapability object
 * @return the version of a friend's capbility.
 */
LINPHONE_PUBLIC float linphone_friend_get_capability_version(const LinphoneFriend *linphone_friend, const LinphoneFriendCapability capability);

/**
 * Removes a friend from it's friend list and from the rc if exists
 * @param linphone_friend #LinphoneFriend object to delete @notnil
 */
LINPHONE_PUBLIC void linphone_friend_remove(LinphoneFriend *linphone_friend);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Contructor
 * @return a new empty #LinphoneFriend
 * @deprecated 03/02/2016 use #linphone_core_create_friend() instead
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneFriend * linphone_friend_new(void);

/**
 * Contructor same as linphone_friend_new() + linphone_friend_set_address()
 * @param addr a buddy address, must be a sip uri like sip:joe@sip.linphone.org
 * @return a new #LinphoneFriend with an initialized address.
 * @deprecated 03/02/2016 use #linphone_core_create_friend_with_address() instead
 * @donotwrap
 */
LINPHONE_PUBLIC	LINPHONE_DEPRECATED LinphoneFriend *linphone_friend_new_with_address(const char *addr);

/**
 * Destroy a LinphoneFriend.
 * @param linphone_friend #LinphoneFriend object
 * @deprecated 31/03/2015 Use linphone_friend_unref() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_friend_destroy(LinphoneFriend *linphone_friend);

/**
 * Get the status of a friend
 * @param lf A #LinphoneFriend object
 * @return #LinphoneOnlineStatus
 * @deprecated 19/06/2013 Use linphone_friend_get_presence_model() instead
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneOnlineStatus linphone_friend_get_status(const LinphoneFriend *linphone_friend);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_FRIEND_H_ */
