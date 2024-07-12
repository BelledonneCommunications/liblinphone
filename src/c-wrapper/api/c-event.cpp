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

#ifndef LINPHONE_EVENT_H_
#define LINPHONE_EVENT_H_

#include "linphone/api/c-event.h"

#include "account/account.h"
#include "core/core.h"
#include "core_private.h"
#include "event/event-publish.h"
#include "event/event-subscribe.h"
#include "event/event.h"
#include "linphone/api/c-content.h"
#include "linphone/api/c-event-cbs.h"
#include "linphone/types.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

// =============================================================================
// Log function.
// =============================================================================

void log_bad_cast(const char *function_name) {
	lError() << function_name << " - the LinphoneEvent is not of the correct derived type";
}

// =============================================================================
// Core functions.
// =============================================================================

int _linphone_event_send_publish(LinphoneEvent *lev, LinphoneContent *body, bool_t notify_err) {
	auto evPub = dynamic_pointer_cast<EventPublish>(Event::toCpp(lev)->getSharedFromThis());
	if (!evPub) {
		log_bad_cast("linphone_event_update_publish");
		return -1;
	}
	const auto cppBody =
	    (body && (linphone_content_get_size(body) > 0)) ? Content::toCpp(body)->getSharedFromThis() : nullptr;
	return evPub->sendPublish(cppBody, notify_err);
}

LinphoneEvent *_linphone_core_create_publish(
    LinphoneCore *core, LinphoneAccount *account, LinphoneAddress *resource, const char *event, int expires) {
	return (new EventPublish(L_GET_CPP_PTR_FROM_C_OBJECT(core), Account::toCpp(account)->getSharedFromThis(),
	                         Address::toCpp(resource)->getSharedFromThis(), L_C_TO_STRING(event), expires))
	    ->toC();
}

LinphoneEvent *
linphone_core_create_publish(LinphoneCore *lc, LinphoneAddress *resource, const char *event, int expires) {
	CoreLogContextualizer logContextualizer(lc);
	return (new EventPublish(L_GET_CPP_PTR_FROM_C_OBJECT(lc), NULL, Address::toCpp(resource)->getSharedFromThis(),
	                         L_C_TO_STRING(event), expires))
	    ->toC();
}

LinphoneEvent *linphone_core_publish(
    LinphoneCore *lc, LinphoneAddress *resource, const char *event, int expires, LinphoneContent *body) {
	CoreLogContextualizer logContextualizer(lc);
	int err;
	auto ev = (new EventPublish(L_GET_CPP_PTR_FROM_C_OBJECT(lc), Address::toCpp(resource)->getSharedFromThis(),
	                            L_C_TO_STRING(event), expires));
	ev->setUnrefWhenTerminated(true);
	err = _linphone_event_send_publish(ev->toC(), body, FALSE);
	if (err == -1) {
		ev->unref();
		ev = nullptr;
	}
	return ev->toC();
}

LinphoneEvent *linphone_core_create_one_shot_publish(LinphoneCore *lc, LinphoneAddress *resource, const char *event) {
	CoreLogContextualizer logContextualizer(lc);
	return (new EventPublish(L_GET_CPP_PTR_FROM_C_OBJECT(lc), Address::toCpp(resource)->getSharedFromThis(),
	                         L_C_TO_STRING(event)))
	    ->toC();
}

LinphoneEvent *linphone_core_create_notify(LinphoneCore *lc, LinphoneAddress *resource, const char *event) {
	CoreLogContextualizer logContextualizer(lc);
	return (new EventSubscribe(L_GET_CPP_PTR_FROM_C_OBJECT(lc), Address::toCpp(resource)->getSharedFromThis(),
	                           L_C_TO_STRING(event)))
	    ->toC();
}

LinphoneEvent *
linphone_core_create_subscribe(LinphoneCore *lc, LinphoneAddress *resource, const char *event, int expires) {
	CoreLogContextualizer logContextualizer(lc);
	return (new EventSubscribe(L_GET_CPP_PTR_FROM_C_OBJECT(lc), Address::toCpp(resource)->getSharedFromThis(),
	                           L_C_TO_STRING(event), expires))
	    ->toC();
}

LinphoneEvent *linphone_core_subscribe(
    LinphoneCore *lc, LinphoneAddress *resource, const char *event, int expires, LinphoneContent *body) {
	CoreLogContextualizer logContextualizer(lc);
	auto ev = new EventSubscribe(L_GET_CPP_PTR_FROM_C_OBJECT(lc), Address::toCpp(resource)->getSharedFromThis(),
	                             L_C_TO_STRING(event), expires);
	ev->setUnrefWhenTerminated(true);
	const auto cppBody =
	    (body && (linphone_content_get_size(body) > 0)) ? Content::toCpp(body)->getSharedFromThis() : nullptr;
	ev->send(cppBody);
	return ev->toC();
}

LinphoneEvent *linphone_core_create_subscribe_2(
    LinphoneCore *lc, LinphoneAddress *resource, LinphoneProxyConfig *cfg, const char *event, int expires) {
	CoreLogContextualizer logContextualizer(lc);
	LinphoneAccount *account = nullptr;
	if (cfg) account = linphone_proxy_config_get_account(cfg);
	return (new EventSubscribe(L_GET_CPP_PTR_FROM_C_OBJECT(lc), Address::toCpp(resource)->getSharedFromThis(),
	                           Account::toCpp(account)->getSharedFromThis(), L_C_TO_STRING(event), expires))
	    ->toC();
}

// =============================================================================
// Private functions.
// =============================================================================

LINPHONE_PUBLIC const char *linphone_publish_state_to_string(LinphonePublishState state) {
	switch (state) {
		case LinphonePublishNone:
			return "LinphonePublishNone";
		case LinphonePublishOutgoingProgress:
			return "LinphonePublishOutgoingProgress";
		case LinphonePublishIncomingReceived:
			return "LinphonePublishIncomingReceived";
		case LinphonePublishRefreshing:
			return "LinphonePublishRefreshing";
		case LinphonePublishOk:
			return "LinphonePublishOk";
		case LinphonePublishError:
			return "LinphonePublishError";
		case LinphonePublishCleared:
			return "LinphonePublishCleared";
		case LinphonePublishExpiring:
			return "LinphonePublishExpiring";
		case LinphonePublishTerminating:
			return "LinphonePublishTerminating";
	}
	return NULL;
}

const char *linphone_subscription_state_to_string(LinphoneSubscriptionState state) {
	switch (state) {
		case LinphoneSubscriptionNone:
			return "LinphoneSubscriptionNone";
		case LinphoneSubscriptionIncomingReceived:
			return "LinphoneSubscriptionIncomingReceived";
		case LinphoneSubscriptionOutgoingProgress:
			return "LinphoneSubscriptionOutgoingProgress";
		case LinphoneSubscriptionPending:
			return "LinphoneSubscriptionPending";
		case LinphoneSubscriptionActive:
			return "LinphoneSubscriptionActive";
		case LinphoneSubscriptionTerminated:
			return "LinphoneSubscriptionTerminated";
		case LinphoneSubscriptionError:
			return "LinphoneSubscriptionError";
		case LinphoneSubscriptionExpiring:
			return "LinphoneSubscriptionExpiring";
	}
	return NULL;
}

LinphoneSubscriptionState linphone_subscription_state_from_sal(SalSubscribeStatus ss) {
	switch (ss) {
		case SalSubscribeNone:
			return LinphoneSubscriptionNone;
		case SalSubscribePending:
			return LinphoneSubscriptionPending;
		case SalSubscribeTerminated:
			return LinphoneSubscriptionTerminated;
		case SalSubscribeActive:
			return LinphoneSubscriptionActive;
	}
	return LinphoneSubscriptionNone;
}

const char *linphone_subscription_dir_to_string(LinphoneSubscriptionDir dir) {
	switch (dir) {
		case LinphoneSubscriptionIncoming:
			return "LinphoneSubscriptionIncoming";
		case LinphoneSubscriptionOutgoing:
			return "LinphoneSubscriptionOutgoing";
		case LinphoneSubscriptionInvalidDir:
			return "LinphoneSubscriptionInvalidDir";
	}
	return "INVALID";
}

void _linphone_event_notify_notify_response(LinphoneEvent *lev) {
	auto event_subscribe = dynamic_cast<EventSubscribe *>(Event::toCpp(lev));
	if (!event_subscribe) {
		log_bad_cast("_linphone_event_notify_notify_response");
		return;
	}
	event_subscribe->notifyNotifyResponse();
}

void linphone_event_set_internal(LinphoneEvent *linphone_event, bool_t internal) {
	Event::toCpp(linphone_event)->setInternal(internal);
}

bool_t linphone_event_is_internal(LinphoneEvent *linphone_event) {
	return Event::toCpp(linphone_event)->isInternal();
}

void linphone_event_set_state(LinphoneEvent *lev, LinphoneSubscriptionState state) {
	auto event_subscribe = dynamic_cast<EventSubscribe *>(Event::toCpp(lev));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_set_state");
		return;
	}
	event_subscribe->setState(state);
}

void linphone_event_set_publish_state(LinphoneEvent *lev, LinphonePublishState state) {
	auto event_publish = dynamic_cast<EventPublish *>(Event::toCpp(lev));
	if (!event_publish) {
		log_bad_cast("linphone_event_set_publish_state");
		return;
	}
	event_publish->setState(state);
}

void linphone_event_unpublish(LinphoneEvent *lev) {
	EventLogContextualizer logContextualizer(lev);
	Event::toCpp(lev)->unpublish();
}

void linphone_event_set_current_callbacks(LinphoneEvent *ev, LinphoneEventCbs *cbs) {
	Event::toCpp(ev)->setCurrentCallbacks(EventCbs::toCpp(cbs)->getSharedFromThis());
}

const bctbx_list_t *linphone_event_get_callbacks_list(const LinphoneEvent *ev) {
	return Event::toCpp(ev)->getCCallbacksList();
}

// =============================================================================
// Public functions.
// =============================================================================

LinphoneStatus linphone_event_send_subscribe(LinphoneEvent *linphone_event, const LinphoneContent *body) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_subscribe = dynamic_cast<EventSubscribe *>(Event::toCpp(linphone_event));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_send_subscribe");
		return -1;
	}
	const auto cppBody =
	    (body && (linphone_content_get_size(body) > 0)) ? Content::toCpp(body)->getSharedFromThis() : nullptr;
	return event_subscribe->send(cppBody);
}

LinphoneStatus linphone_event_update_subscribe(LinphoneEvent *linphone_event, const LinphoneContent *body) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_subscribe = dynamic_cast<EventSubscribe *>(Event::toCpp(linphone_event));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_update_subscribe");
		return -1;
	}
	const auto cppBody =
	    (body && (linphone_content_get_size(body) > 0)) ? Content::toCpp(body)->getSharedFromThis() : nullptr;
	return event_subscribe->update(cppBody);
}

LinphoneStatus linphone_event_refresh_subscribe(LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_subscribe = dynamic_cast<EventSubscribe *>(Event::toCpp(linphone_event));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_refresh_subscribe");
		return -1;
	}
	return event_subscribe->refresh();
}

LinphoneStatus linphone_event_accept_subscription(LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_subscribe = dynamic_cast<EventSubscribe *>(Event::toCpp(linphone_event));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_accept_subscription");
		return -1;
	}
	return event_subscribe->accept();
}

LinphoneStatus linphone_event_deny_subscription(LinphoneEvent *linphone_event, LinphoneReason reason) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_subscribe = dynamic_cast<EventSubscribe *>(Event::toCpp(linphone_event));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_deny_subscription");
		return -1;
	}
	return event_subscribe->deny(reason);
}

LinphoneStatus linphone_event_notify(LinphoneEvent *linphone_event, const LinphoneContent *body) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_subscribe = dynamic_cast<EventSubscribe *>(Event::toCpp(linphone_event));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_notify");
		return -1;
	}
	const auto cppBody =
	    (body && (linphone_content_get_size(body) > 0)) ? Content::toCpp(body)->getSharedFromThis() : nullptr;
	return event_subscribe->notify(cppBody);
}

LinphoneStatus linphone_event_send_publish(LinphoneEvent *linphone_event, const LinphoneContent *body) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_publish = dynamic_cast<EventPublish *>(Event::toCpp(linphone_event));
	if (!event_publish) {
		log_bad_cast("linphone_event_send_publish");
		return -1;
	}
	const auto cppBody =
	    (body && (linphone_content_get_size(body) > 0)) ? Content::toCpp(body)->getSharedFromThis() : nullptr;
	return event_publish->send(cppBody);
}

LinphoneStatus linphone_event_update_publish(LinphoneEvent *linphone_event, const LinphoneContent *body) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_publish = dynamic_cast<EventPublish *>(Event::toCpp(linphone_event));
	if (!event_publish) {
		log_bad_cast("linphone_event_update_publish");
		return -1;
	}
	const auto cppBody =
	    (body && (linphone_content_get_size(body) > 0)) ? Content::toCpp(body)->getSharedFromThis() : nullptr;
	return event_publish->update(cppBody);
}

LinphoneStatus linphone_event_refresh_publish(LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_publish = dynamic_cast<EventPublish *>(Event::toCpp(linphone_event));
	if (!event_publish) {
		log_bad_cast("linphone_event_refresh_publish");
		return -1;
	}
	return event_publish->refresh();
}

LinphoneStatus linphone_event_accept_publish(LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_publish = dynamic_cast<EventPublish *>(Event::toCpp(linphone_event));
	if (!event_publish) {
		log_bad_cast("linphone_event_accept_publish");
		return -1;
	}
	return event_publish->accept();
}

LinphoneStatus linphone_event_deny_publish(LinphoneEvent *linphone_event, LinphoneReason reason) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_publish = dynamic_cast<EventPublish *>(Event::toCpp(linphone_event));
	if (!event_publish) {
		log_bad_cast("linphone_event_deny_publish");
		return -1;
	}
	return event_publish->deny(reason);
}

void linphone_event_pause_publish(LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_publish = dynamic_cast<EventPublish *>(Event::toCpp(linphone_event));
	if (!event_publish) {
		log_bad_cast("linphone_event_pause_publish");
		return;
	}
	event_publish->pause();
}

LinphoneReason linphone_event_get_reason(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return Event::toCpp(linphone_event)->getReason();
}

const LinphoneErrorInfo *linphone_event_get_error_info(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return Event::toCpp(linphone_event)->getErrorInfo();
}

LinphoneSubscriptionState linphone_event_get_subscription_state(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_subscribe = dynamic_cast<const EventSubscribe *>(Event::toCpp(linphone_event));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_get_subscription_state");
		return LinphoneSubscriptionNone;
	}
	return event_subscribe->getState();
}

LinphonePublishState linphone_event_get_publish_state(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_publish = dynamic_cast<const EventPublish *>(Event::toCpp(linphone_event));
	if (!event_publish) {
		log_bad_cast("linphone_event_get_publish_state");
		return LinphonePublishNone;
	}
	return event_publish->getState();
}

LinphoneSubscriptionDir linphone_event_get_subscription_dir(LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	auto event_subscribe = dynamic_cast<EventSubscribe *>(Event::toCpp(linphone_event));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_get_subscription_dir");
		return LinphoneSubscriptionInvalidDir;
	}
	return event_subscribe->getDir();
}

bool_t linphone_event_is_out_of_dialog_op(const LinphoneEvent *linphone_event) {
	auto event_subscribe = dynamic_cast<const EventSubscribe *>(Event::toCpp(linphone_event));
	if (!event_subscribe) {
		log_bad_cast("linphone_event_is_out_of_dialog_op");
		return FALSE;
	}
	return event_subscribe->isOutOfDialogOp();
}

void linphone_event_add_custom_header(LinphoneEvent *linphone_event, const char *name, const char *value) {
	EventLogContextualizer logContextualizer(linphone_event);
	Event::toCpp(linphone_event)->addCustomHeader(L_C_TO_STRING(name), L_C_TO_STRING(value));
}

void linphone_event_remove_custom_header(LinphoneEvent *linphone_event, const char *name) {
	EventLogContextualizer logContextualizer(linphone_event);
	Event::toCpp(linphone_event)->removeCustomHeader(L_C_TO_STRING(name));
}

const char *linphone_event_get_custom_header(LinphoneEvent *linphone_event, const char *name) {
	EventLogContextualizer logContextualizer(linphone_event);
	return Event::toCpp(linphone_event)->getCustomHeaderCstr(L_C_TO_STRING(name));
}

void linphone_event_terminate(LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	Event::toCpp(linphone_event)->terminate();
}

const char *linphone_event_get_name(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return L_STRING_TO_C(Event::toCpp(linphone_event)->getName());
}

const LinphoneAddress *linphone_event_get_from(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return linphone_event_get_from_address(linphone_event);
}

const LinphoneAddress *linphone_event_get_to(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return linphone_event_get_to_address(linphone_event);
}

const LinphoneAddress *linphone_event_get_from_address(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return Event::toCpp(linphone_event)->getFrom()->toC();
}

const LinphoneAddress *linphone_event_get_to_address(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return Event::toCpp(linphone_event)->getTo()->toC();
}

const LinphoneAddress *linphone_event_get_resource(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return Event::toCpp(linphone_event)->getResource()->toC();
}

const LinphoneAddress *linphone_event_get_remote_contact(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return Event::toCpp(linphone_event)->getRemoteContact()->toC();
}

const LinphoneAddress *linphone_event_get_request_address(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return Event::toCpp(linphone_event)->getRequestAddress()->toC();
}

void linphone_event_set_request_address(LinphoneEvent *linphone_event, LinphoneAddress *request_address) {
	EventLogContextualizer logContextualizer(linphone_event);
	Event::toCpp(linphone_event)
	    ->setRequestAddress(request_address ? Address::toCpp(request_address)->getSharedFromThis() : nullptr);
}

const char *linphone_event_get_call_id(const LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	return L_STRING_TO_C(Event::toCpp(linphone_event)->getCallId());
}

LinphoneCore *linphone_event_get_core(const LinphoneEvent *linphone_event) {
	return Event::toCpp(linphone_event)->getCore()->getCCore();
}

void linphone_event_add_callbacks(LinphoneEvent *linphone_event, LinphoneEventCbs *cbs) {
	Event::toCpp(linphone_event)->addCallbacks(EventCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_event_remove_callbacks(LinphoneEvent *linphone_event, LinphoneEventCbs *cbs) {
	Event::toCpp(linphone_event)->removeCallbacks(EventCbs::toCpp(cbs)->getSharedFromThis());
}

LinphoneEventCbs *linphone_event_get_current_callbacks(const LinphoneEvent *linphone_event) {
	return Event::toCpp(linphone_event)->getCurrentCallbacks()->toC();
}

// =============================================================================
// Reference and user data handling functions.
// =============================================================================

LinphoneEvent *linphone_event_ref(LinphoneEvent *linphone_event) {
	Event::toCpp(linphone_event)->ref();
	return linphone_event;
}

void linphone_event_unref(LinphoneEvent *linphone_event) {
	EventLogContextualizer logContextualizer(linphone_event);
	Event::toCpp(linphone_event)->unref();
}

void linphone_event_set_user_data(LinphoneEvent *linphone_event, void *user_data) {
	Event::toCpp(linphone_event)->setUserData(user_data);
}

void *linphone_event_get_user_data(const LinphoneEvent *linphone_event) {
	return Event::toCpp(linphone_event)->getUserData();
}

// =============================================================================
// Constructor and destructor functions.
// =============================================================================

LinphoneEvent *linphone_event_new_subscribe_with_op(LinphoneCore *lc,
                                                    SalSubscribeOp *op,
                                                    LinphoneSubscriptionDir dir,
                                                    const char *name) {
	return EventSubscribe::createCObject<EventSubscribe>(L_GET_CPP_PTR_FROM_C_OBJECT(lc), op, dir, L_C_TO_STRING(name),
	                                                     false);
}

LinphoneEvent *linphone_event_new_subscribe_with_out_of_dialog_op(LinphoneCore *lc,
                                                                  SalSubscribeOp *op,
                                                                  LinphoneSubscriptionDir dir,
                                                                  const char *name) {
	return EventSubscribe::createCObject<EventSubscribe>(L_GET_CPP_PTR_FROM_C_OBJECT(lc), op, dir, L_C_TO_STRING(name),
	                                                     true);
}

LinphoneEvent *linphone_event_new_publish_with_op(LinphoneCore *lc, SalPublishOp *op, const char *name) {
	return EventPublish::createCObject<EventPublish>(L_GET_CPP_PTR_FROM_C_OBJECT(lc), op, L_C_TO_STRING(name));
}

#endif /* LINPHONE_EVENT_H_ */
