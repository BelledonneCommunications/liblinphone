/*
friendlist.h
Copyright (C) 2010-2015 Belledonne Communications SARL

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#ifndef LINPHONE_FRIENDLIST_H_
#define LINPHONE_FRIENDLIST_H_


#ifdef IN_LINPHONE
#include "linphonefriend.h"
#include "linphonepresence.h"
#else
#include "linphone/linphonefriend.h"
#include "linphone/linphonepresence.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup buddy_list
 * @{
 */

/**
* Enum describing the status of a LinphoneFriendList operation.
**/
typedef enum _LinphoneFriendListStatus {
	LinphoneFriendListOK,
	LinphoneFriendListNonExistentFriend,
	LinphoneFriendListInvalidFriend
} LinphoneFriendListStatus;

/**
 * Enum describing the status of a CardDAV synchronization
 */
typedef enum _LinphoneFriendListSyncStatus {
	LinphoneFriendListSyncStarted,
	LinphoneFriendListSyncSuccessful,
	LinphoneFriendListSyncFailure
} LinphoneFriendListSyncStatus;

/**
 * The LinphoneFriendList object representing a list of friends.
**/
typedef struct _LinphoneFriendList LinphoneFriendList;

/**
 * Create a new empty LinphoneFriendList object.
 * @param[in] lc LinphoneCore object.
 * @return A new LinphoneFriendList object.
**/
LINPHONE_PUBLIC LinphoneFriendList * linphone_core_create_friend_list(LinphoneCore *lc);

/**
 * Add a friend list.
 * @param[in] lc LinphoneCore object
 * @param[in] list LinphoneFriendList object
 */
LINPHONE_PUBLIC void linphone_core_add_friend_list(LinphoneCore *lc, LinphoneFriendList *list);

/**
 * Removes a friend list.
 * @param[in] lc LinphoneCore object
 * @param[in] list LinphoneFriendList object
 */
LINPHONE_PUBLIC void linphone_core_remove_friend_list(LinphoneCore *lc, LinphoneFriendList *list);

/**
 * Retrieves the list of LinphoneFriendList from the core.
 * @param[in] lc LinphoneCore object
 * @return \mslist{LinphoneFriendList} a list of LinphoneFriendList
 */
LINPHONE_PUBLIC const MSList * linphone_core_get_friends_lists(const LinphoneCore *lc);

/**
 * Retrieves the first list of LinphoneFriend from the core.
 * @param[in] lc LinphoneCore object
 * @return the first LinphoneFriendList object or NULL
 */
LINPHONE_PUBLIC LinphoneFriendList * linphone_core_get_default_friend_list(const LinphoneCore *lc);

/**
 * Acquire a reference to the friend list.
 * @param[in] list LinphoneFriendList object.
 * @return The same LinphoneFriendList object.
**/
LINPHONE_PUBLIC LinphoneFriendList * linphone_friend_list_ref(LinphoneFriendList *list);

/**
 * Release reference to the friend list.
 * @param[in] list LinphoneFriendList object.
**/
LINPHONE_PUBLIC void linphone_friend_list_unref(LinphoneFriendList *list);

/**
 * Retrieve the user pointer associated with the friend list.
 * @param[in] list LinphoneFriendList object.
 * @return The user pointer associated with the friend list.
**/
LINPHONE_PUBLIC void * linphone_friend_list_get_user_data(const LinphoneFriendList *list);

/**
 * Assign a user pointer to the friend list.
 * @param[in] list LinphoneFriendList object.
 * @param[in] ud The user pointer to associate with the friend list.
**/
LINPHONE_PUBLIC void linphone_friend_list_set_user_data(LinphoneFriendList *list, void *ud);

/**
 * Get the display name of the friend list.
 * @param[in] list LinphoneFriendList object.
 * @return The display name of the friend list.
**/
LINPHONE_PUBLIC const char * linphone_friend_list_get_display_name(const LinphoneFriendList *list);

/**
 * Set the display name of the friend list.
 * @param[in] list LinphoneFriendList object.
 * @param[in] display_name The new display name of the friend list.
**/
LINPHONE_PUBLIC void linphone_friend_list_set_display_name(LinphoneFriendList *list, const char *display_name);

/**
 * Get the RLS (Resource List Server) URI associated with the friend list to subscribe to these friends presence.
 * @param[in] list LinphoneFriendList object.
 * @return The RLS URI associated with the friend list.
**/
LINPHONE_PUBLIC const char * linphone_friend_list_get_rls_uri(const LinphoneFriendList *list);

/**
 * Set the RLS (Resource List Server) URI associated with the friend list to subscribe to these friends presence.
 * @param[in] list LinphoneFriendList object.
 * @param[in] rls_uri The RLS URI to associate with the friend list.
**/
LINPHONE_PUBLIC void linphone_friend_list_set_rls_uri(LinphoneFriendList *list, const char *rls_uri);

/**
 * Add a friend to a friend list. If or when a remote CardDAV server will be attached to the list, the friend will be sent to the server.
 * @param[in] list LinphoneFriendList object.
 * @param[in] friend LinphoneFriend object to add to the friend list.
 * @return LinphoneFriendListOK if successfully added, LinphoneFriendListInvalidFriend if the friend is not valid.
**/
LINPHONE_PUBLIC LinphoneFriendListStatus linphone_friend_list_add_friend(LinphoneFriendList *list, LinphoneFriend *lf);

/**
 * Add a friend to a friend list. The friend will never be sent to a remote CardDAV server.
 * Warning! LinphoneFriends added this way will be removed on the next synchronization, and the callback contact_deleted will be called.
 * @param[in] list LinphoneFriendList object.
 * @param[in] friend LinphoneFriend object to add to the friend list.
 * @return LinphoneFriendListOK if successfully added, LinphoneFriendListInvalidFriend if the friend is not valid.
**/
LINPHONE_PUBLIC LinphoneFriendListStatus linphone_friend_list_add_local_friend(LinphoneFriendList *list, LinphoneFriend *lf);

/**
 * Remove a friend from a friend list.
 * @param[in] list LinphoneFriendList object.
 * @param[in] friend LinphoneFriend object to remove from the friend list.
 * @return LinphoneFriendListOK if removed successfully, LinphoneFriendListNonExistentFriend if the friend is not in the list.
**/
LINPHONE_PUBLIC LinphoneFriendListStatus linphone_friend_list_remove_friend(LinphoneFriendList *list, LinphoneFriend *afriend);

/**
 * Retrieves the list of LinphoneFriend from this LinphoneFriendList.
 * @param[in] list LinphoneFriendList object
 * @return \mslist{LinphoneFriend} a list of LinphoneFriend
 */
LINPHONE_PUBLIC const MSList * linphone_friend_list_get_friends(const LinphoneFriendList *list);

/**
 * Find a friend in the friend list using a LinphoneAddress.
 * @param[in] list LinphoneFriendList object.
 * @param[in] address LinphoneAddress object of the friend we want to search for.
 * @return A LinphoneFriend if found, NULL otherwise.
**/
LINPHONE_PUBLIC LinphoneFriend * linphone_friend_list_find_friend_by_address(const LinphoneFriendList *list, const LinphoneAddress *address);

/**
 * Find a friend in the friend list using an URI string.
 * @param[in] list LinphoneFriendList object.
 * @param[in] uri A string containing the URI of the friend we want to search for.
 * @return A LinphoneFriend if found, NULL otherwise.
**/
LINPHONE_PUBLIC LinphoneFriend * linphone_friend_list_find_friend_by_uri(const LinphoneFriendList *list, const char *uri);

/**
 * Find a frient in the friend list using a ref key.
 * @param[in] list LinphoneFriendList object.
 * @param[in] ref_key The ref key string of the friend we want to search for.
 * @return A LinphoneFriend if found, NULL otherwise.
**/
LINPHONE_PUBLIC LinphoneFriend * linphone_friend_list_find_friend_by_ref_key(const LinphoneFriendList *list, const char *ref_key);

LINPHONE_PUBLIC void linphone_friend_list_close_subscriptions(LinphoneFriendList *list);

LINPHONE_PUBLIC void linphone_friend_list_update_subscriptions(LinphoneFriendList *list, LinphoneProxyConfig *cfg, bool_t only_when_registered);

/**
 * Notify our presence to all the friends in the friend list that have subscribed to our presence directly (not using a RLS).
 * @param[in] list LinphoneFriendList object.
 * @param[in] presence LinphonePresenceModel object.
**/
LINPHONE_PUBLIC void linphone_friend_list_notify_presence(LinphoneFriendList *list, LinphonePresenceModel *presence);

/**
 * Get the URI associated with the friend list.
 * @param[in] list LinphoneFriendList object.
 * @return The URI associated with the friend list.
**/
LINPHONE_PUBLIC const char * linphone_friend_list_get_uri(const LinphoneFriendList *list);

/**
 * Set the URI associated with the friend list.
 * @param[in] list LinphoneFriendList object.
 * @param[in] rls_uri The URI to associate with the friend list.
**/
LINPHONE_PUBLIC void linphone_friend_list_set_uri(LinphoneFriendList *list, const char *uri);

/**
 * Sets the revision from the last synchronization.
 * @param[in] list LinphoneFriendList object.
 * @param[in] rev The revision
 */
void linphone_friend_list_update_revision(LinphoneFriendList *list, int rev);

/**
 * An object to handle the callbacks for LinphoneFriend synchronization.
**/
typedef struct _LinphoneFriendListCbs LinphoneFriendListCbs;

/**
 * Callback used to notify a new contact has been created on the CardDAV server and downloaded locally
 * @param list The LinphoneFriendList object the new contact is added to
 * @param lf The LinphoneFriend object that has been created
**/
typedef void (*LinphoneFriendListCbsContactCreatedCb)(LinphoneFriendList *list, LinphoneFriend *lf);

/**
 * Callback used to notify a contact has been deleted on the CardDAV server
 * @param list The LinphoneFriendList object a contact has been removed from
 * @param lf The LinphoneFriend object that has been deleted
**/
typedef void (*LinphoneFriendListCbsContactDeletedCb)(LinphoneFriendList *list, LinphoneFriend *lf);

/**
 * Callback used to notify a contact has been updated on the CardDAV server
 * @param list The LinphoneFriendList object in which a contact has been updated
 * @param new_friend The new LinphoneFriend object corresponding to the updated contact
 * @param old_friend The old LinphoneFriend object before update
**/
typedef void (*LinphoneFriendListCbsContactUpdatedCb)(LinphoneFriendList *list, LinphoneFriend *new_friend, LinphoneFriend *old_friend);

/**
 * Callback used to notify the status of the synchronization has changed
 * @param list The LinphoneFriendList object for which the status has changed
 * @param status The new synchronisation status
 * @param msg An additional information on the status update
**/
typedef void (*LinphoneFriendListCbsSyncStateChangedCb)(LinphoneFriendList *list, LinphoneFriendListSyncStatus status, const char *msg);

/**
 * Get the LinphoneFriendListCbs object associated with a LinphoneFriendList.
 * @param[in] request LinphoneXmlRpcRequest object
 * @return The LinphoneFriendListCbs object associated with the LinphoneFriendList.
**/
LINPHONE_PUBLIC LinphoneFriendListCbs * linphone_friend_list_get_callbacks(const LinphoneFriendList *list);

/**
 * Acquire a reference to a LinphoneFriendListCbs object.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @return The same LinphoneFriendListCbs object.
**/
LINPHONE_PUBLIC LinphoneFriendListCbs * linphone_friend_list_cbs_ref(LinphoneFriendListCbs *cbs);

/**
 * Release a reference to a LinphoneFriendListCbs object.
 * @param[in] cbs LinphoneFriendListCbs object.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_unref(LinphoneFriendListCbs *cbs);

/**
 * Retrieve the user pointer associated with a LinphoneFriendListCbs object.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @return The user pointer associated with the LinphoneFriendListCbs object.
**/
LINPHONE_PUBLIC void *linphone_friend_list_cbs_get_user_data(const LinphoneFriendListCbs *cbs);

/**
 * Assign a user pointer to a LinphoneFriendListCbs object.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @param[in] ud The user pointer to associate with the LinphoneFriendListCbs object.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_user_data(LinphoneFriendListCbs *cbs, void *ud);

/**
 * Get the contact created callback.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @return The current contact created callback.
**/
LINPHONE_PUBLIC LinphoneFriendListCbsContactCreatedCb linphone_friend_list_cbs_get_contact_created(const LinphoneFriendListCbs *cbs);

/**
 * Set the contact created callback.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @param[in] cb The contact created to be used.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_contact_created(LinphoneFriendListCbs *cbs, LinphoneFriendListCbsContactCreatedCb cb);

/**
 * Get the contact deleted callback.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @return The current contact deleted callback.
**/
LINPHONE_PUBLIC LinphoneFriendListCbsContactDeletedCb linphone_friend_list_cbs_get_contact_deleted(const LinphoneFriendListCbs *cbs);

/**
 * Set the contact deleted callback.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @param[in] cb The contact deleted to be used.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_contact_deleted(LinphoneFriendListCbs *cbs, LinphoneFriendListCbsContactDeletedCb cb);

/**
 * Get the contact updated callback.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @return The current contact updated callback.
**/
LINPHONE_PUBLIC LinphoneFriendListCbsContactUpdatedCb linphone_friend_list_cbs_get_contact_updated(const LinphoneFriendListCbs *cbs);

/**
 * Set the contact updated callback.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @param[in] cb The contact updated to be used.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_contact_updated(LinphoneFriendListCbs *cbs, LinphoneFriendListCbsContactUpdatedCb cb);

/**
 * Get the sync status changed callback.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @return The current sync status changedcallback.
**/
LINPHONE_PUBLIC LinphoneFriendListCbsSyncStateChangedCb linphone_friend_list_cbs_get_sync_status_changed(const LinphoneFriendListCbs *cbs);

/**
 * Set the contact updated callback.
 * @param[in] cbs LinphoneFriendListCbs object.
 * @param[in] cb The sync status changed to be used.
**/
LINPHONE_PUBLIC void linphone_friend_list_cbs_set_sync_status_changed(LinphoneFriendListCbs *cbs, LinphoneFriendListCbsSyncStateChangedCb cb);

/**
 * Starts a CardDAV synchronization using value set using linphone_friend_list_set_uri.
 * @param[in] list LinphoneFriendList object.
 */
LINPHONE_PUBLIC void linphone_friend_list_synchronize_friends_from_server(LinphoneFriendList *list);

/**
 * Goes through all the LinphoneFriend that are dirty and does a CardDAV PUT to update the server.
 * @param[in] list LinphoneFriendList object.
 */
void linphone_friend_list_update_dirty_friends(LinphoneFriendList *list);

/**
 * Returns the LinphoneCore object attached to this LinphoneFriendList.
 * @param[in] list LinphoneFriendList object.
 * @return a LinphoneCore object
 */
LINPHONE_PUBLIC LinphoneCore* linphone_friend_list_get_core(LinphoneFriendList *list);

/**
 * Creates and adds LinphoneFriend objects to LinphoneFriendList from a file that contains the vCard(s) to parse
 * @param[in] list the LinphoneFriendList object
 * @param[in] vcard_file the path to a file that contains the vCard(s) to parse
 * @return the amount of linphone friends created
 */
LINPHONE_PUBLIC int linphone_friend_list_import_friends_from_vcard4_file(LinphoneFriendList *list, const char *vcard_file);

/**
 * Creates and adds LinphoneFriend objects to LinphoneFriendList from a buffer that contains the vCard(s) to parse
 * @param[in] list the LinphoneFriendList object
 * @param[in] vcard_buffer the buffer that contains the vCard(s) to parse
 * @return the amount of linphone friends created
 */
LINPHONE_PUBLIC int linphone_friend_list_import_friends_from_vcard4_buffer(LinphoneFriendList *list, const char *vcard_buffer);

/**
 * Creates and export LinphoneFriend objects from LinphoneFriendList to a file using vCard 4 format
 * @param[in] list the LinphoneFriendList object
 * @param[in] vcard_file the path to a file that will contain the vCards
 */
LINPHONE_PUBLIC void linphone_friend_list_export_friends_as_vcard4_file(LinphoneFriendList *list, const char *vcard_file);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_FRIENDLIST_H_ */
