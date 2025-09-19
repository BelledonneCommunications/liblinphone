/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include "client-conference-event-handler-base.h"

#include "core/core-p.h"

LINPHONE_BEGIN_NAMESPACE

ClientConferenceEventHandlerBase::ClientConferenceEventHandlerBase(const std::shared_ptr<Core> &core)
    : CoreAccessor(core) {
	try {
		getCore()->getPrivate()->registerListener(this);
	} catch (const std::bad_weak_ptr &) {
		lError() << "ClientConferenceEventHandler [" << this
		         << "]: Unable to register listener as the core has already been destroyed";
	}
}

ClientConferenceEventHandlerBase::~ClientConferenceEventHandlerBase() {
	try {
		getCore()->getPrivate()->unregisterListener(this);
		stopWaitNotifyTimer();
	} catch (const std::bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}

	unsubscribe();
}

void ClientConferenceEventHandlerBase::unsubscribe() {
}

void ClientConferenceEventHandlerBase::startWaitNotifyTimer() {
	if (mWaitNotifyTimer) return;
	unsigned int timeout = (unsigned)linphone_core_get_message_sending_delay(getCore()->getCCore());
	if (timeout == 0) return;
	mWaitNotifyTimer = getCore()->createTimer(
	    [this]() -> bool {
		    stopWaitNotifyTimer();
		    onNotifyWaitExpired();
		    return false;
	    },
	    timeout * 1000, "Delay for waiting for NOTIFY");
}

void ClientConferenceEventHandlerBase::stopWaitNotifyTimer() {
	if (mWaitNotifyTimer) {
		getCore()->destroyTimer(mWaitNotifyTimer);
		mWaitNotifyTimer = nullptr;
	}
}

LINPHONE_END_NAMESPACE
