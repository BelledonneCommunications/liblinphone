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

#include "event-publish.h"

#include "account/account.h"
#include "core/core.h"
#include "core_private.h"
#include "linphone/api/c-event-cbs.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

LinphoneStatus EventPublish::sendPublish(const std::shared_ptr<const Content> &body, bool notifyErr) {
	SalBodyHandler *body_handler;
	int err;

	if (mSendCustomHeaders) {
		mOp->setSentCustomHeaders(mSendCustomHeaders);
		sal_custom_header_free(mSendCustomHeaders);
		mSendCustomHeaders = nullptr;
	} else mOp->setSentCustomHeaders(nullptr);

	fillOpFields();
	body_handler = sal_body_handler_from_content((body && !body->isEmpty()) ? body->toC() : nullptr);
	auto publishOp = dynamic_cast<SalPublishOp *>(mOp);
	err = publishOp->publish(mName, mExpires, body_handler);
	if (err == 0) {
		if (mPublishState == LinphonePublishOk) {
			setState(LinphonePublishRefreshing);
		} else {
			setState(LinphonePublishOutgoingProgress);
		}
	} else if (notifyErr) {
		setState(LinphonePublishError);
	}
	return err;
}

// -----------------------------------------------------------------------------

EventPublish::EventPublish(const shared_ptr<Core> &core) : Event(core) {
}

EventPublish::EventPublish(const shared_ptr<Core> &core, LinphonePrivate::SalPublishOp *op, const string &name)
    : Event(core) {
	mOp = op;
	mExpires = op->getExpires();
	mName = name;
	mOp->setUserPointer(this->toC());
}

EventPublish::EventPublish(const shared_ptr<Core> &core,
                           const shared_ptr<Account> &account,
                           const std::shared_ptr<const Address> &resourceAddr,
                           const string &event,
                           int expires)
    : EventPublish(core, new SalPublishOp(core->getCCore()->sal.get()), event) {
	auto resource = resourceAddr;
	if ((!resource || !resource->isValid()) && account) resource = account->getAccountParams()->getIdentityAddress();

	setExpires(expires);
	if (!account) {
		auto coreAccount = core->lookupKnownAccount(resource, true);
		linphone_configure_op_with_account(
		    core->getCCore(), mOp, resource->toC(), nullptr,
		    !!linphone_config_get_int(core->getCCore()->config, "sip", "publish_msg_with_contact", 0),
		    coreAccount->toC());
	} else {
		linphone_configure_op_with_account(
		    core->getCCore(), mOp, resource->toC(), nullptr,
		    !!linphone_config_get_int(core->getCCore()->config, "sip", "publish_msg_with_contact", 0), account->toC());
	}

	mOp->setManualRefresherMode(
	    !linphone_config_get_int(core->getCCore()->config, "sip", "refresh_generic_publish", 1));
}

EventPublish::EventPublish(const shared_ptr<Core> &core,
                           const std::shared_ptr<const Address> &resource,
                           const string &event,
                           int expires)
    : EventPublish(core, nullptr, resource, event, expires) {
}

EventPublish::EventPublish(const shared_ptr<Core> &core,
                           const std::shared_ptr<const Address> &resource,
                           const string &event)
    : EventPublish(core, resource, event, -1) {
	setOneshot(true);
	setUnrefWhenTerminated(true);
}

EventPublish::~EventPublish() {
	stopTimeoutHandling();
}

string EventPublish::toString() const {
	std::ostringstream ss;
	ss << "Publish of " << mName;
	return ss.str();
}

LinphoneStatus EventPublish::send(const std::shared_ptr<const Content> &body) {
	return sendPublish(body, TRUE);
}

LinphoneStatus EventPublish::update(const std::shared_ptr<const Content> &body) {
	return send(body);
}

LinphoneStatus EventPublish::refresh() {
	return mOp->refresh();
}

LinphoneStatus EventPublish::accept() {
	int err;
	if (mPublishState != LinphonePublishIncomingReceived && mPublishState != LinphonePublishRefreshing) {
		ms_error("EventPublish::accept(): cannot accept publish if subscription wasn't just received.");
		return -1;
	}
	fillOpFields();
	auto publishOp = dynamic_cast<SalPublishOp *>(mOp);
	err = publishOp->accept();
	if (err == 0) {
		setState(LinphonePublishOk);
		startTimeoutHandling();
	}
	return err;
}

LinphoneStatus EventPublish::deny(LinphoneReason reason) {
	int err;
	if (mPublishState != LinphonePublishIncomingReceived && mPublishState != LinphonePublishRefreshing) {
		ms_error("EventPublish::deny(): cannot deny publish if publish wasn't just received.");
		return -1;
	}
	auto publishOp = dynamic_cast<SalPublishOp *>(mOp);
	err = publishOp->decline(linphone_reason_to_sal(reason));
	setState(LinphonePublishCleared);
	return err;
}

void EventPublish::pause() {
	if (mOp) mOp->stopRefreshing();
}

void EventPublish::setOneshot(bool oneshot) {
	mOneshot = oneshot;
}

LinphonePublishState EventPublish::getState() const {
	return mPublishState;
}

void EventPublish::setState(LinphonePublishState state) {
	if (mPublishState != state) {
		ms_message("Event [%p] moving from [%s] to publish state [%s]", this,
		           linphone_publish_state_to_string(mPublishState), linphone_publish_state_to_string(state));
		mPublishState = state;

		ref();
		linphone_core_notify_publish_state_changed(getCore()->getCCore(), this->toC(), state);
		LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Event, this, linphone_event_cbs_get_publish_state_changed, state);
		switch (state) {
			case LinphonePublishNone: /*this state is probably trigered by a network state change to DOWN, we should
			                             release the op*/
				release();
				break;
			case LinphonePublishOk:
				if (mOneshot) release();
				break;
			case LinphonePublishCleared:
			case LinphonePublishError:
				release();
				break;
			case LinphonePublishOutgoingProgress:
			case LinphonePublishIncomingReceived:
			case LinphonePublishRefreshing:
			case LinphonePublishTerminating:
			case LinphonePublishExpiring:
				/*nothing special to do*/
				break;
		}
		unref();
	}
}

void EventPublish::unpublish() {
	setState(LinphonePublishTerminating);
	if (mOp) {
		auto op = dynamic_cast<SalPublishOp *>(mOp);
		op->unpublish();
	}
}

void EventPublish::terminate() {
	// if event was already terminated (including on error), we should not terminate it again
	// otherwise it will be unreffed twice.
	if (mPublishState == LinphonePublishError || mPublishState == LinphonePublishCleared) {
		return;
	}

	if (mPublishState != LinphonePublishNone) {
		if (mOp && (mPublishState == LinphonePublishOk) && (mExpires != -1)) {
			auto op = dynamic_cast<SalPublishOp *>(mOp);
			op->unpublish();
		}
		setState(LinphonePublishCleared);
		return;
	}

	setState(LinphonePublishTerminating);
}

void EventPublish::startTimeoutHandling() {
	stopTimeoutHandling();
	if (mExpires > 0) {
		mTimer = getCore()->createTimer(
		    [this]() {
			    lInfo() << "Publish event [" << this << "] has expired";
			    terminate();
			    return true;
		    },
		    static_cast<unsigned int>(mExpires) * 1000, "Publish timer");
	}
}

void EventPublish::stopTimeoutHandling() {
	if (mTimer) {
		lInfo() << "stopTimeoutHandling()";
		getCore()->destroyTimer(mTimer);
		mTimer = nullptr;
	}
}

LINPHONE_END_NAMESPACE
