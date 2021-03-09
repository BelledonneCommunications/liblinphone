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


#ifndef LINPHONE_FRIENDLIST_H_
#define LINPHONE_FRIENDLIST_H_


#include "linphone/types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup buddy_list
 * @{
 */

/**
 * Acquire a reference to the friend list.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @return The same #LinphoneFriendList object. @notnil
**/
LINPHONE_PUBLIC LinphoneFriendList * linphone_friend_list_ref(LinphoneFriendList *friend_list);

/**
 * Release reference to the friend list.
 * @param friend_list #LinphoneFriendList object. @notnil
**/
LINPHONE_PUBLIC void linphone_friend_list_unref(LinphoneFriendList *friend_list);

/**
 * Retrieve the user pointer associated with the friend list.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @return The user pointer associated with the friend list. @maybenil
**/
LINPHONE_PUBLIC void * linphone_friend_list_get_user_data(const LinphoneFriendList *friend_list);

/**
 * Assign a user pointer to the friend list.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param user_data The user pointer to associate with the friend list. @maybenil
**/
LINPHONE_PUBLIC void linphone_friend_list_set_user_data(LinphoneFriendList *friend_list, void *user_data);

/**
 * Get the display name of the friend list.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @return The display name of the friend list. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_friend_list_get_display_name(const LinphoneFriendList *friend_list);

/**
 * Set the display name of the friend list.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param display_name The new display name of the friend list. @maybenil
**/
LINPHONE_PUBLIC void linphone_friend_list_set_display_name(LinphoneFriendList *friend_list, const char *display_name);

/**
 * Get the RLS (Resource List Server) URI associated with the friend list to subscribe to these friends presence.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @return The RLS URI associated with the friend list. @maybenil
 * @deprecated 27/10/2020. Use linphone_friend_list_get_rls_address() instead.
**/
LINPHONE_PUBLIC const char * linphone_friend_list_get_rls_uri(const LinphoneFriendList *friend_list);

/**
 * Set the RLS (Resource List Server) URI associated with the friend list to subscribe to these friends presence.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param rls_uri The RLS URI to associate with the friend list. @maybenil
 * @deprecated 27/10/2020. Use linphone_friend_list_set_rls_address() instead.
**/
LINPHONE_PUBLIC void linphone_friend_list_set_rls_uri(LinphoneFriendList *friend_list, const char *rls_uri);

/**
 * Get the RLS (Resource List Server) URI associated with the friend list to subscribe to these friends presence.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @return The RLS URI as #LinphoneAddress associated with the friend list. @maybenil
**/
LINPHONE_PUBLIC const LinphoneAddress * linphone_friend_list_get_rls_address(const LinphoneFriendList *friend_list);

/**
 * Set the RLS (Resource List Server) URI associated with the friend list to subscribe to these friends presence.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param rls_addr The RLS URI to associate with the friend list. @maybenil
**/
LINPHONE_PUBLIC void linphone_friend_list_set_rls_address(LinphoneFriendList *friend_list, const LinphoneAddress *rls_addr);

/**
 * Add a friend to a friend list. If or when a remote CardDAV server will be attached to the list, the friend will be sent to the server.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param linphone_friend #LinphoneFriend object to add to the friend list. @notnil
 * @return #LinphoneFriendListOK if successfully added, #LinphoneFriendListInvalidFriend if the friend is not valid.
**/
LINPHONE_PUBLIC LinphoneFriendListStatus linphone_friend_list_add_friend(LinphoneFriendList *friend_list, LinphoneFriend *linphone_friend);

/**
 * Add a friend to a friend list. The friend will never be sent to a remote CardDAV server.
 * Warning! #LinphoneFriends added this way will be removed on the next synchronization, and the callback contact_deleted will be called.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param linphone_friend #LinphoneFriend object to add to the friend list. @notnil
 * @return #LinphoneFriendListOK if successfully added, #LinphoneFriendListInvalidFriend if the friend is not valid.
**/
LINPHONE_PUBLIC LinphoneFriendListStatus linphone_friend_list_add_local_friend(LinphoneFriendList *friend_list, LinphoneFriend *linphone_friend);

/**
 * Remove a friend from a friend list.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param linphone_friend #LinphoneFriend object to remove from the friend list. @notnil
 * @return #LinphoneFriendListOK if removed successfully, #LinphoneFriendListNonExistentFriend if the friend is not in the list.
**/
LINPHONE_PUBLIC LinphoneFriendListStatus linphone_friend_list_remove_friend(LinphoneFriendList *friend_list, LinphoneFriend *linphone_friend);

/**
 * Retrieves the list of #LinphoneFriend from this LinphoneFriendList.
 * @param friend_list #LinphoneFriendList object @notnil
 * @return A list of #LinphoneFriend \bctbx_list{LinphoneFriend} @maybenil
 */
LINPHONE_PUBLIC const bctbx_list_t * linphone_friend_list_get_friends(const LinphoneFriendList *friend_list);

/**
 * Find a friend in the friend list using a LinphoneAddress.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param address #LinphoneAddress object of the friend we want to search for. @notnil
 * @return A #LinphoneFriend if found, NULL otherwise. @maybenil
**/
LINPHONE_PUBLIC LinphoneFriend * linphone_friend_list_find_friend_by_address(const LinphoneFriendList *friend_list, const LinphoneAddress *address);

/**
 * Find a friend in the friend list using a phone number.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param phone_number a string of the phone number for which we want to find a friend. @notnil
 * @return A #LinphoneFriend if found, NULL otherwise. @maybenil
**/
LINPHONE_PUBLIC LinphoneFriend * linphone_friend_list_find_friend_by_phone_number(const LinphoneFriendList *friend_list, const char *phone_number);

/**
 * Find all friends in the friend list using a LinphoneAddress.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param address #LinphoneAddress object of the friends we want to search for. @notnil
 * @return A list of #LinphoneFriend if found, NULL otherwise. \bctbx_list{LinphoneFriend} @maybenil
**/
LINPHONE_PUBLIC bctbx_list_t * linphone_friend_list_find_friends_by_address(const LinphoneFriendList *friend_list, const LinphoneAddress *address);

/**
 * Find a friend in the friend list using an URI string.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param uri A string containing the URI of the friend we want to search for. @notnil
 * @return A #LinphoneFriend if found, NULL otherwise. @maybenil
**/
LINPHONE_PUBLIC LinphoneFriend * linphone_friend_list_find_friend_by_uri(const LinphoneFriendList *friend_list, const char *uri);

/**
 * Find all friends in the friend list using an URI string.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param uri A string containing the URI of the friends we want to search for. @notnil
 * @return A list of #LinphoneFriend if found, NULL otherwise. \bctbx_list{LinphoneFriend} @maybenil
**/
LINPHONE_PUBLIC bctbx_list_t * linphone_friend_list_find_friends_by_uri(const LinphoneFriendList *friend_list, const char *uri);

/**
 * Find a friend in the friend list using a ref key.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param ref_key The ref key string of the friend we want to search for. @notnil
 * @return A #LinphoneFriend if found, NULL otherwise. @maybenil
**/
LINPHONE_PUBLIC LinphoneFriend * linphone_friend_list_find_friend_by_ref_key(const LinphoneFriendList *friend_list, const char *ref_key);

/**
 * Update presence subscriptions for the entire list. Calling this function is necessary when list subscriptions are enabled,
 * ie when a RLS presence server is used.
 * @param friend_list the #LinphoneFriendList @notnil
**/
LINPHONE_PUBLIC void linphone_friend_list_update_subscriptions(LinphoneFriendList *friend_list);

/**
 * Notify our presence to all the friends in the friend list that have subscribed to our presence directly (not using a RLS).
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param presence #LinphonePresenceModel object. @notnil
**/
LINPHONE_PUBLIC void linphone_friend_list_notify_presence(LinphoneFriendList *friend_list, LinphonePresenceModel *presence);

/**
 * Get the URI associated with the friend list.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @return The URI associated with the friend list. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_friend_list_get_uri(const LinphoneFriendList *friend_list);

/**
 * Set the URI associated with the friend list.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param uri The URI to associate with the friend list. @maybenil
**/
LINPHONE_PUBLIC void linphone_friend_list_set_uri(LinphoneFriendList *friend_list, const char *uri);

/**
 * Get wheter the subscription of the friend list is bodyless or not.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @return Wheter the subscription of the friend list is bodyless or not.
**/
LINPHONE_PUBLIC bool_t linphone_friend_list_is_subscription_bodyless(LinphoneFriendList *friend_list);

/**
 * Set wheter the subscription of the friend list is bodyless or not.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param bodyless boolean telling if the subscription of the friend list is bodyless or not.
**/
LINPHONE_PUBLIC void linphone_friend_list_set_subscription_bodyless(LinphoneFriendList *friend_list, bool_t bodyless);

/**
 * Sets the revision from the last synchronization.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @param revision The revision
 */
LINPHONE_PUBLIC void linphone_friend_list_update_revision(LinphoneFriendList *friend_list, int revision);

/**
 * Adds the #LinphoneFriendListCbs object associated with a LinphoneFriendList.
 * @param friend_list #LinphoneFriendList object @notnil
 * @param cbs The current #LinphoneFriendListCbs object to be added to the LinphoneFriendList. @notnil
**/
LINPHONE_PUBLIC void linphone_friend_list_add_callbacks(LinphoneFriendList *friend_list, LinphoneFriendListCbs *cbs);

/**
 * Removes the #LinphoneFriendListCbs object associated with a LinphoneFriendList.
 * @param friend_list #LinphoneFriendList object @notnil
 * @param cbs The current #LinphoneFriendListCbs object to be remove from the LinphoneFriendList. @notnil
**/
LINPHONE_PUBLIC void linphone_friend_list_remove_callbacks(LinphoneFriendList *friend_list, LinphoneFriendListCbs *cbs);

/**
 * Get the current #LinphoneFriendListCbs object associated with a LinphoneFriendList.
 * @param friend_list #LinphoneFriendList object @notnil
 * @return The current #LinphoneFriendListCbs object associated with the LinphoneFriendList. @maybenil
**/
LINPHONE_PUBLIC LinphoneFriendListCbs *linphone_friend_list_get_current_callbacks(const LinphoneFriendList *friend_list);

/**
 * Acquire a reference to a #LinphoneFriendListCbs object.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @return The same #LinphoneFriendListCbs object.
**/
LINPHONE_PUBLIC LinphoneFriendListCbs * linphone_friend_list_cbs_ref(LinphoneFriendListCbs *cbs);

/**
 * Release a reference to a #LinphoneFriendListCbs object.
 * @param cbs #LinphoneFriendListCbs object. @notnil
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_unref(LinphoneFriendListCbs *cbs);

/**
 * Retrieve the user pointer associated with a #LinphoneFriendListCbs object.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @return The user pointer associated with the #LinphoneFriendListCbs object. @maybenil
**/
LINPHONE_PUBLIC void *linphone_friend_list_cbs_get_user_data(const LinphoneFriendListCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneFriendListCbs object.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @param user_data The user pointer to associate with the #LinphoneFriendListCbs object. @maybenil
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_user_data(LinphoneFriendListCbs *cbs, void *user_data);

/**
 * Get the contact created callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @return The current contact created callback.
**/
LINPHONE_PUBLIC LinphoneFriendListCbsContactCreatedCb linphone_friend_list_cbs_get_contact_created(const LinphoneFriendListCbs *cbs);

/**
 * Set the contact created callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @param cb The contact created to be used.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_contact_created(LinphoneFriendListCbs *cbs, LinphoneFriendListCbsContactCreatedCb cb);

/**
 * Get the contact deleted callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @return The current contact deleted callback.
**/
LINPHONE_PUBLIC LinphoneFriendListCbsContactDeletedCb linphone_friend_list_cbs_get_contact_deleted(const LinphoneFriendListCbs *cbs);

/**
 * Set the contact deleted callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @param cb The contact deleted to be used.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_contact_deleted(LinphoneFriendListCbs *cbs, LinphoneFriendListCbsContactDeletedCb cb);

/**
 * Get the contact updated callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @return The current contact updated callback.
**/
LINPHONE_PUBLIC LinphoneFriendListCbsContactUpdatedCb linphone_friend_list_cbs_get_contact_updated(const LinphoneFriendListCbs *cbs);

/**
 * Set the contact updated callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @param cb The contact updated to be used.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_contact_updated(LinphoneFriendListCbs *cbs, LinphoneFriendListCbsContactUpdatedCb cb);

/**
 * Get the sync status changed callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @return The current sync status changedcallback.
**/
LINPHONE_PUBLIC LinphoneFriendListCbsSyncStateChangedCb linphone_friend_list_cbs_get_sync_status_changed(const LinphoneFriendListCbs *cbs);

/**
 * Set the contact updated callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @param cb The sync status changed to be used.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_sync_status_changed(LinphoneFriendListCbs *cbs, LinphoneFriendListCbsSyncStateChangedCb cb);

/**
 * Get the presence received callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @return The current presence received callback.
**/
LINPHONE_PUBLIC LinphoneFriendListCbsPresenceReceivedCb linphone_friend_list_cbs_get_presence_received(const LinphoneFriendListCbs *cbs);

/**
 * Set the presence received callback.
 * @param cbs #LinphoneFriendListCbs object. @notnil
 * @param cb The presence received callback to be used.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_presence_received(LinphoneFriendListCbs *cbs, LinphoneFriendListCbsPresenceReceivedCb cb);

/**
 * Starts a CardDAV synchronization using value set using linphone_friend_list_set_uri.
 * @param friend_list #LinphoneFriendList object. @notnil
 */
LINPHONE_PUBLIC void linphone_friend_list_synchronize_friends_from_server(LinphoneFriendList *friend_list);

/**
 * Goes through all the #LinphoneFriend that are dirty and does a CardDAV PUT to update the server.
 * @param friend_list #LinphoneFriendList object. @notnil
 */
LINPHONE_PUBLIC void linphone_friend_list_update_dirty_friends(LinphoneFriendList *friend_list);

/**
 * Returns the #LinphoneCore object attached to this LinphoneFriendList.
 * @param friend_list #LinphoneFriendList object. @notnil
 * @return a #LinphoneCore object @notnil
 */
LINPHONE_PUBLIC LinphoneCore* linphone_friend_list_get_core(const LinphoneFriendList *friend_list);

/**
 * Creates and adds #LinphoneFriend objects to #LinphoneFriendList from a file that contains the vCard(s) to parse
 * @param friend_list the #LinphoneFriendList object @notnil
 * @param vcard_file the path to a file that contains the vCard(s) to parse @notnil
 * @return the amount of linphone friends created
 */
LINPHONE_PUBLIC int linphone_friend_list_import_friends_from_vcard4_file(LinphoneFriendList *friend_list, const char *vcard_file);

/**
 * Creates and adds #LinphoneFriend objects to #LinphoneFriendList from a buffer that contains the vCard(s) to parse
 * @param friend_list the #LinphoneFriendList object @notnil
 * @param vcard_buffer the buffer that contains the vCard(s) to parse @notnil
 * @return the amount of linphone friends created
 */
LINPHONE_PUBLIC int linphone_friend_list_import_friends_from_vcard4_buffer(LinphoneFriendList *friend_list, const char *vcard_buffer);

/**
 * Creates and export #LinphoneFriend objects from #LinphoneFriendList to a file using vCard 4 format
 * @param friend_list the #LinphoneFriendList object @notnil
 * @param vcard_file the path to a file that will contain the vCards @notnil
 */
LINPHONE_PUBLIC void linphone_friend_list_export_friends_as_vcard4_file(LinphoneFriendList *friend_list, const char *vcard_file);

/**
 * Enable subscription to NOTIFYes of all friends list
 * @param friend_list the #LinphoneFriendList object @notnil
 * @param enabled should subscription be enabled or not
 */
LINPHONE_PUBLIC void linphone_friend_list_enable_subscriptions(LinphoneFriendList *friend_list, bool_t enabled);

/**
 * Gets whether subscription to NOTIFYes of all friends list are enabled or not
 * @param friend_list the #LinphoneFriendList object @notnil
 * @return Whether subscriptions are enabled or not
 */
LINPHONE_PUBLIC bool_t linphone_friend_list_subscriptions_enabled(LinphoneFriendList *friend_list);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Get the #LinphoneFriendListCbs object associated with a LinphoneFriendList.
 * @param friend_list #LinphoneFriendList object @notnil
 * @return The #LinphoneFriendListCbs object associated with the LinphoneFriendList.
 * @deprecated 19/02/2019 use add_callbacks / remove_callbacks instead
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneFriendListCbs * linphone_friend_list_get_callbacks(const LinphoneFriendList *friend_list);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_FRIENDLIST_H_ */
