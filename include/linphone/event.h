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

#ifndef LINPHONE_EVENT_H_
#define LINPHONE_EVENT_H_

#include "linphone/callbacks.h"
#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup event_api
 * @{
**/

/**
 * Send a subscription previously created by linphone_core_create_subscribe().
 * @param linphone_event the #LinphoneEvent
 * @param body optional content to attach with the subscription.
 * @return 0 if successful, -1 otherwise.
**/
LINPHONE_PUBLIC LinphoneStatus linphone_event_send_subscribe(LinphoneEvent *linphone_event, const LinphoneContent *body);

/**
 * Update (refresh) an outgoing subscription, changing the body.
 * @param linphone_event a #LinphoneEvent
 * @param body an optional body to include in the subscription update, may be NULL.
 * @maybenil
**/
LINPHONE_PUBLIC LinphoneStatus linphone_event_update_subscribe(LinphoneEvent *linphone_event, const LinphoneContent *body);

/**
 * Refresh an outgoing subscription keeping the same body.
 * @param linphone_event #LinphoneEvent object.
 * @return 0 if successful, -1 otherwise.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_event_refresh_subscribe(LinphoneEvent *linphone_event);


/**
 * Accept an incoming subcription.
 * @param linphone_event #LinphoneEvent object.
**/
LINPHONE_PUBLIC LinphoneStatus linphone_event_accept_subscription(LinphoneEvent *linphone_event);

/**
 * Deny an incoming subscription with given reason.
 * @param linphone_event #LinphoneEvent object.
**/
LINPHONE_PUBLIC LinphoneStatus linphone_event_deny_subscription(LinphoneEvent *linphone_event, LinphoneReason reason);

/**
 * Send a notification.
 * @param linphone_event a #LinphoneEvent corresponding to an incoming subscription previously received and accepted.
 * @param body an optional body containing the actual notification data.
 * @return 0 if successful, -1 otherwise.
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_event_notify(LinphoneEvent *linphone_event, const LinphoneContent *body);

/**
 * Send a publish created by linphone_core_create_publish().
 * @param linphone_event the #LinphoneEvent
 * @param body the new data to be published
**/
LINPHONE_PUBLIC LinphoneStatus linphone_event_send_publish(LinphoneEvent *linphone_event, const LinphoneContent *body);

/**
 * Update (refresh) a publish.
 * @param linphone_event the #LinphoneEvent
 * @param body the new data to be published
**/
LINPHONE_PUBLIC LinphoneStatus linphone_event_update_publish(LinphoneEvent *linphone_event, const LinphoneContent *body);

/**
 * Refresh an outgoing publish keeping the same body.
 * @param linphone_event #LinphoneEvent object.
 * @return 0 if successful, -1 otherwise.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_event_refresh_publish(LinphoneEvent *linphone_event);

/**
 * Prevent an event from refreshing its publish.
 * This is useful to let registrations to expire naturally (or) when the application wants to keep control on when
 * refreshes are sent.
 * The refreshing operations can be resumed with linphone_proxy_config_refresh_register().
 * @param[in] linphone_event #LinphoneEvent object.
 **/
LINPHONE_PUBLIC void linphone_event_pause_publish(LinphoneEvent *linphone_event);

/**
 * Return reason code (in case of error state reached).
 * @param linphone_event #LinphoneEvent object.
 * @return a #LinphoneReason object
**/
LINPHONE_PUBLIC LinphoneReason linphone_event_get_reason(const LinphoneEvent *linphone_event);

/**
 * Get full details about an error occured.
 * @param linphone_event #LinphoneEvent object.
 * @return a #LinphoneErrorInfo object.
**/
LINPHONE_PUBLIC const LinphoneErrorInfo *linphone_event_get_error_info(const LinphoneEvent *linphone_event);

/**
 * Get subscription state. If the event object was not created by a subscription mechanism, #LinphoneSubscriptionNone is returned.
 * @param linphone_event #LinphoneEvent object.
 * @return the current #LinphoneSubscriptionState
**/
LINPHONE_PUBLIC LinphoneSubscriptionState linphone_event_get_subscription_state(const LinphoneEvent *linphone_event);

/**
 * Get publish state. If the event object was not created by a publish mechanism, #LinphonePublishNone is returned.
 * @param linphone_event #LinphoneEvent object.
 * @return the current #LinphonePublishState
**/
LINPHONE_PUBLIC LinphonePublishState linphone_event_get_publish_state(const LinphoneEvent *linphone_event);

/**
 * Get subscription direction.
 * If the object wasn't created by a subscription mechanism, #LinphoneSubscriptionInvalidDir is returned.
 * @param linphone_event #LinphoneEvent object.
 * @return the #LinphoneSubscriptionDir
**/
LINPHONE_PUBLIC LinphoneSubscriptionDir linphone_event_get_subscription_dir(LinphoneEvent *linphone_event);

/**
 * Set a user (application) pointer.
 * @param linphone_event #LinphoneEvent object.
 * @param user_data The user data to set. @maybenil
**/
LINPHONE_PUBLIC void linphone_event_set_user_data(LinphoneEvent *linphone_event, void *user_data);

/**
 * Retrieve user pointer.
 * @param linphone_event #LinphoneEvent object.
 * @return the user_data pointer or NULL. @maybenil
**/
LINPHONE_PUBLIC void *linphone_event_get_user_data(const LinphoneEvent *linphone_event);

/**
 * Add a custom header to an outgoing susbscription or publish.
 * @param linphone_event the #LinphoneEvent
 * @param name header's name
 * @param value the header's value.
**/
LINPHONE_PUBLIC void linphone_event_add_custom_header(LinphoneEvent *linphone_event, const char *name, const char *value);

/**
 * Obtain the value of a given header for an incoming subscription.
 * @param linphone_event the #LinphoneEvent
 * @param name header's name
 * @return the header's value or NULL if such header doesn't exist.
**/
LINPHONE_PUBLIC const char *linphone_event_get_custom_header(LinphoneEvent *linphone_event, const char *name);

/**
 * Terminate an incoming or outgoing subscription that was previously acccepted, or a previous publication.
 * The #LinphoneEvent shall not be used anymore after this operation, unless the application explicitely took a reference on the object with
 * linphone_event_ref().
 * @param[in] linphone_event #LinphoneEvent object
**/
LINPHONE_PUBLIC void linphone_event_terminate(LinphoneEvent *linphone_event);

/**
 * Increase reference count of LinphoneEvent.
 * By default #LinphoneEvents created by the core are owned by the core only.
 * An application that wishes to retain a reference to it must call linphone_event_ref().
 * When this reference is no longer needed, linphone_event_unref() must be called.
 * @param[in] linphone_event #LinphoneEvent object
 * @return the same #LinphoneEvent object.
 *
**/
LINPHONE_PUBLIC LinphoneEvent *linphone_event_ref(LinphoneEvent *linphone_event);

/**
 * Decrease reference count.
 * @param[in] linphone_event #LinphoneEvent object
 * @see linphone_event_ref()
**/
LINPHONE_PUBLIC void linphone_event_unref(LinphoneEvent *linphone_event);

/**
 * Get the name of the event as specified in the event package RFC.
 * @param[in] linphone_event #LinphoneEvent object
 * @return the event name.
**/
LINPHONE_PUBLIC const char *linphone_event_get_name(const LinphoneEvent *linphone_event);

/**
 * Get the "from" address of the subscription.
 * @param[in] linphone_event #LinphoneEvent object
 * @return the from #LinphoneAddress.
**/
LINPHONE_PUBLIC const LinphoneAddress *linphone_event_get_from(const LinphoneEvent *linphone_event);

/**
 * Get the resource address of the subscription or publish.
 * @param[in] linphone_event #LinphoneEvent object
 * @return the resource #LinphoneAddress.
**/
LINPHONE_PUBLIC const LinphoneAddress *linphone_event_get_resource(const LinphoneEvent *linphone_event);

/**
 * Get the "contact" address of the subscription.
 * @param[in] linphone_event #LinphoneEvent object
 * @return The "contact" address of the subscription
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_event_get_remote_contact (const LinphoneEvent *linphone_event);

/**
 * Returns back pointer to the #LinphoneCore that created this #LinphoneEvent
 * @param[in] linphone_event #LinphoneEvent object
 * @return the #LinphoneCore object associated.
**/
LINPHONE_PUBLIC LinphoneCore *linphone_event_get_core(const LinphoneEvent *linphone_event);

/**
 * Get the LinphoneEventCbs object associated with a LinphoneEvent.
 * @param[in] linphone_event #LinphoneEvent object
 * @return The LinphoneEventCbs object associated with the LinphoneEvent.
 * @deprecated 19/02/2019 use add_callbacks / remove_callbacks instead
**/
LINPHONE_PUBLIC LinphoneEventCbs *linphone_event_get_callbacks(const LinphoneEvent *linphone_event);

/**
 * Adds a LinphoneEventCbs object to be associated with a LinphoneEvent.
 * @param[in] linphone_event #LinphoneEvent object
 * @param[in] cbs The LinphoneEventCbs object to add
**/
LINPHONE_PUBLIC void linphone_event_add_callbacks(LinphoneEvent *linphone_event, LinphoneEventCbs *cbs);

/**
 * Removes a LinphoneEventCbs object associated with a LinphoneEvent.
 * @param[in] linphone_event #LinphoneEvent object
 * @param[in] cbs The LinphoneEventCbs object to remove
**/
LINPHONE_PUBLIC void linphone_event_remove_callbacks(LinphoneEvent *linphone_event, LinphoneEventCbs *cbs);

/**
 * Get the current LinphoneEventCbs object associated with a LinphoneEvent.
 * @param[in] linphone_event #LinphoneEvent object
 * @return The current LinphoneEventCbs object associated with the LinphoneEvent.
**/
LINPHONE_PUBLIC LinphoneEventCbs *linphone_event_get_current_callbacks(const LinphoneEvent *linphone_event);

/**
 * Acquire a reference to a LinphoneEventCbs object.
 * @return The same LinphoneEventCbs object.
**/
LINPHONE_PUBLIC LinphoneEventCbs *linphone_event_cbs_ref(LinphoneEventCbs *cbs);

/**
 * Release a reference to a LinphoneEventCbs object.
 * @param[in] cbs LinphoneEventCbs object.
**/
LINPHONE_PUBLIC void linphone_event_cbs_unref(LinphoneEventCbs *cbs);

/**
 * Retrieve the user pointer associated with a LinphoneEventCbs object.
 * @param[in] cbs LinphoneEventCbs object.
 * @return The user pointer associated with the LinphoneEventCbs object. @maybenil
**/
LINPHONE_PUBLIC void *linphone_event_cbs_get_user_data(const LinphoneEventCbs *cbs);

/**
 * Assign a user pointer to a LinphoneEventCbs object.
 * @param[in] cbs LinphoneEventCbs object.
 * @param[in] user_data The user pointer to associate with the LinphoneEventCbs object. @maybenil
**/
LINPHONE_PUBLIC void linphone_event_cbs_set_user_data(LinphoneEventCbs *cbs, void *user_data);

/**
 * Get the notify response callback.
 * @param[in] cbs LinphoneEventCbs object.
 * @return The current notify response callback.
**/
LINPHONE_PUBLIC LinphoneEventCbsNotifyResponseCb linphone_event_cbs_get_notify_response(const LinphoneEventCbs *cbs);

/**
 * Set the notify response callback.
 * @param[in] cbs LinphoneEventCbs object.
 * @param[in] cb The notify response callback to be used.
**/
LINPHONE_PUBLIC void linphone_event_cbs_set_notify_response(LinphoneEventCbs *cbs, LinphoneEventCbsNotifyResponseCb cb);

/**
 * @}
**/

#ifdef __cplusplus
}
#endif


#endif /* LINPHONE_EVENT_H_ */
