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

#include "event-subscribe.h"

#include "core/core.h"
#include "core_private.h"
#include "linphone/api/c-event-cbs.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

EventSubscribe::EventSubscribe(const shared_ptr<Core> &core,
                               LinphoneSubscriptionDir dir,
                               const string &name,
                               LinphonePrivate::SalSubscribeOp *op)
    : Event(core) {
	mDir = dir;
	mOp = op;
	mName = name;
	mOp->setUserPointer(this->toC());
}

EventSubscribe::EventSubscribe(const shared_ptr<Core> &core,
                               LinphoneSubscriptionDir dir,
                               const string &name,
                               int expires)
    : EventSubscribe(core, dir, name, new SalSubscribeOp(core->getCCore()->sal.get())) {
	mExpires = expires;
}

EventSubscribe::EventSubscribe(const shared_ptr<Core> &core,
                               SalSubscribeOp *op,
                               LinphoneSubscriptionDir dir,
                               const string &name,
                               bool isOutOfDialog)
    : EventSubscribe(core, dir, name, op) {
	mIsOutOfDialogOp = isOutOfDialog;
}

EventSubscribe::EventSubscribe(const shared_ptr<Core> &core,
                               const std::shared_ptr<Address> resource,
                               const string &event)
    : EventSubscribe(core, LinphoneSubscriptionIncoming, event, -1) {
	linphone_configure_op(core->getCCore(), mOp, resource->toC(), NULL, TRUE);
	setState(LinphoneSubscriptionIncomingReceived);
	mOp->setEvent(event);
	setIsOutOfDialogOp(true);
}

EventSubscribe::EventSubscribe(const shared_ptr<Core> &core,
                               const std::shared_ptr<Address> resource,
                               const string &event,
                               int expires)
    : EventSubscribe(core, LinphoneSubscriptionOutgoing, event, expires) {
	linphone_configure_op(core->getCCore(), mOp, resource->toC(), NULL, TRUE);
	mOp->setManualRefresherMode(
	    !linphone_config_get_int(core->getCCore()->config, "sip", "refresh_generic_subscribe", 1));
}

EventSubscribe::EventSubscribe(const shared_ptr<Core> &core,
                               const std::shared_ptr<Address> resource,
                               const std::shared_ptr<Account> &account,
                               const string &event,
                               int expires)
    : EventSubscribe(core, LinphoneSubscriptionOutgoing, event, expires) {
	linphone_configure_op_with_account(core->getCCore(), mOp, resource->toC(), NULL, TRUE, account->toC());
	mOp->setManualRefresherMode(
	    !linphone_config_get_int(core->getCCore()->config, "sip", "refresh_generic_subscribe", 1));
}

string EventSubscribe::toString() const {
	std::ostringstream ss;
	ss << (mDir == LinphoneSubscriptionIncoming ? "Incoming Subscribe" : "Outgoing subscribe") << " of " << mName;
	return ss.str();
}

LinphoneStatus EventSubscribe::send(const std::shared_ptr<const Content> &body) {
	SalBodyHandler *body_handler;
	int err;

	if (mDir != LinphoneSubscriptionOutgoing) {
		ms_error("EventSubscribe::send(): cannot send or update something that is not an outgoing subscription.");
		return -1;
	}
	switch (mSubscriptionState) {
		case LinphoneSubscriptionIncomingReceived:
		case LinphoneSubscriptionTerminated:
		case LinphoneSubscriptionOutgoingProgress:
			ms_error("EventSubscribe::send(): cannot update subscription while in state [%s]",
			         linphone_subscription_state_to_string(mSubscriptionState));
			return -1;
			break;
		case LinphoneSubscriptionNone:
		case LinphoneSubscriptionActive:
		case LinphoneSubscriptionExpiring:
		case LinphoneSubscriptionError:
		case LinphoneSubscriptionPending:
			/*those states are ok*/
			break;
	}

	if (mSendCustomHeaders) {
		mOp->setSentCustomHeaders(mSendCustomHeaders);
		sal_custom_header_free(mSendCustomHeaders);
		mSendCustomHeaders = nullptr;
	} else mOp->setSentCustomHeaders(nullptr);
	if (mRequestAddress) {
		mOp->setRequestUri(mRequestAddress->asStringUriOnly());
	}

	const LinphoneContent *cBody = (body && !body->isEmpty()) ? body->toC() : nullptr;
	body_handler = sal_body_handler_from_content(cBody);
	auto subscribeOp = dynamic_cast<SalSubscribeOp *>(mOp);
	err = subscribeOp->subscribe(mName, mExpires, body_handler);
	if (err == 0) {
		if (mSubscriptionState == LinphoneSubscriptionNone) setState(LinphoneSubscriptionOutgoingProgress);
	}
	return err;
}

LinphoneStatus EventSubscribe::update(const std::shared_ptr<const Content> &body) {
	return send(body);
}

LinphoneStatus EventSubscribe::refresh() {
	return mOp->refresh();
}

LinphoneStatus EventSubscribe::accept() {
	int err;
	if (mSubscriptionState != LinphoneSubscriptionIncomingReceived) {
		ms_error("EventSubscribe::accept(): cannot accept subscription if subscription wasn't just received.");
		return -1;
	}
	auto subscribeOp = dynamic_cast<SalSubscribeOp *>(mOp);
	err = subscribeOp->accept();
	if (err == 0) {
		setState(LinphoneSubscriptionActive);
	}
	return err;
}

LinphoneStatus EventSubscribe::deny(LinphoneReason reason) {
	int err;
	if (mSubscriptionState != LinphoneSubscriptionIncomingReceived) {
		ms_error("EventSubscribe::deny(): cannot deny subscription if subscription wasn't just received.");
		return -1;
	}
	auto subscribeOp = dynamic_cast<SalSubscribeOp *>(mOp);
	err = subscribeOp->decline(linphone_reason_to_sal(reason));
	setState(LinphoneSubscriptionTerminated);
	return err;
}

LinphoneStatus EventSubscribe::notify(const std::shared_ptr<const Content> &body) {
	SalBodyHandler *body_handler;
	if (mSubscriptionState != LinphoneSubscriptionActive &&
	    mSubscriptionState != LinphoneSubscriptionIncomingReceived) {
		ms_error("EventSubscribe::notify(): cannot notify if subscription is not active.");
		return -1;
	}
	if (mDir != LinphoneSubscriptionIncoming) {
		ms_error("EventSubscribe::notify(): cannot notify if not an incoming subscription.");
		return -1;
	}
	const LinphoneContent *cBody = (body && !body->isEmpty()) ? body->toC() : nullptr;
	body_handler = sal_body_handler_from_content(cBody, false);
	auto subscribeOp = dynamic_cast<SalSubscribeOp *>(mOp);
	return subscribeOp->notify(body_handler);
}

void EventSubscribe::notifyNotifyResponse() {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Event, this, linphone_event_cbs_get_notify_response);
}

LinphoneSubscriptionState EventSubscribe::getState() const {
	return mSubscriptionState;
}

void EventSubscribe::setState(LinphoneSubscriptionState state) {
	if (mSubscriptionState != state) {
		ms_message("Event [%p] moving to subscription state %s", this, linphone_subscription_state_to_string(state));
		mSubscriptionState = state;
		ref();
		linphone_core_notify_subscription_state_changed(getCore()->getCCore(), this->toC(), state);
		LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Event, this, linphone_event_cbs_get_subscribe_state_changed, state);
		if (state == LinphoneSubscriptionTerminated || state == LinphoneSubscriptionError) {
			release();
		}
		unref();
	}
}

LinphoneSubscriptionDir EventSubscribe::getDir() {
	return mDir;
}

bool EventSubscribe::isOutOfDialogOp() const {
	return mIsOutOfDialogOp;
}

void EventSubscribe::setIsOutOfDialogOp(bool isOutOfDialogOp) {
	mIsOutOfDialogOp = isOutOfDialogOp;
}

void EventSubscribe::unpublish() {
	if (mOp) {
		auto op = dynamic_cast<SalPublishOp *>(mOp);
		op->unpublish();
	}
}

void EventSubscribe::terminate() {
	// if event was already terminated (including on error), we should not terminate it again
	// otherwise it will be unreffed twice.
	if (mSubscriptionState == LinphoneSubscriptionError || mSubscriptionState == LinphoneSubscriptionTerminated) {
		return;
	}

	if (mDir == LinphoneSubscriptionIncoming) {
		auto op = dynamic_cast<SalSubscribeOp *>(mOp);
		if (op) {
			op->closeNotify();
		}
	} else if (mDir == LinphoneSubscriptionOutgoing) {
		auto op = dynamic_cast<SalSubscribeOp *>(mOp);
		if (op) {
			op->unsubscribe();
		}
	}

	if (mSubscriptionState != LinphoneSubscriptionNone) {
		setState(LinphoneSubscriptionTerminated);
		return;
	}
}

LINPHONE_END_NAMESPACE
