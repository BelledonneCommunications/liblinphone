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
	} catch (const std::bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}

	unsubscribe();

	for (const auto &[address, timer] : mDelayMessageSendTimers) {
		stopDelayMessageSendTimer(address);
	}
}

void ClientConferenceEventHandlerBase::startDelayMessageSendTimer(const Address address) {
	stopDelayMessageSendTimer(address);
	bool sendMessagesAfterNotify = !!linphone_core_send_message_after_notify_enabled(getCore()->getCCore());
	if (!sendMessagesAfterNotify) {
		setDelayTimerExpired(false, address);
		int delayMessageSendS = linphone_core_get_message_sending_delay(getCore()->getCCore());
		lInfo() << "The core is configured not to wait for the NOTIFY full state before sending messages, hence start "
		           "timer to delay message sending by "
		        << delayMessageSendS << "s to ensure that chat messages in chatrooms associated to " << address
		        << " are sent to all participants";
		std::string backgroundTaskName("Delay message sending for " + address.toString());
		BackgroundTask bgTask{backgroundTaskName};
		bgTask.start(getCore());
		mDelayMessageSendBgTasks.insert_or_assign(address, bgTask);
		auto onDelayMessageSendTimerCleanup = [this, address]() -> bool {
			handleDelayMessageSendTimerExpired(address);
			return true;
		};
		std::string timerName("delay message sending timeout for " + address.toString());
		auto timer = getCore()->createTimer(onDelayMessageSendTimerCleanup,
		                                    static_cast<unsigned int>(delayMessageSendS) * 1000, timerName);
		mDelayMessageSendTimers.insert_or_assign(address, timer);
	}
}

void ClientConferenceEventHandlerBase::stopDelayMessageSendTimer(const Address address) {
	auto timerIt = mDelayMessageSendTimers.find(address);
	if (timerIt != mDelayMessageSendTimers.end()) {
		auto &[address, timer] = *timerIt;
		Core::destroyTimer(timer);
		timer = nullptr;
		mDelayMessageSendTimers.erase(timerIt);
	}

	auto bgTaskIt = mDelayMessageSendBgTasks.find(address);
	if (bgTaskIt != mDelayMessageSendBgTasks.end()) {
		auto &[address, bgTask] = *bgTaskIt;
		bgTask.stop();
		mDelayMessageSendBgTasks.erase(bgTaskIt);
	}
}

bool ClientConferenceEventHandlerBase::delayTimerExpired(const Address address) const {
	try {
		return mDelayTimersExpired.at(address);
	} catch (std::out_of_range &) {
		return false;
	}
}

void ClientConferenceEventHandlerBase::setDelayTimerExpired(bool expired, const Address address) {
	mDelayTimersExpired.insert_or_assign(address, expired);
}

bool ClientConferenceEventHandlerBase::delayMessageSendTimerStarted(const Address address) const {
	try {
		return (mDelayMessageSendTimers.at(address) != nullptr);
	} catch (std::out_of_range &) {
		return false;
	}
}

void ClientConferenceEventHandlerBase::unsubscribe() {
}

LINPHONE_END_NAMESPACE
