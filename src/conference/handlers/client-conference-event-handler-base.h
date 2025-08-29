/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_BASE_H_
#define _L_REMOTE_CONFERENCE_EVENT_HANDLER_BASE_H_

// =============================================================================

#include <belle-sip/mainloop.h>

#include "address/address.h"
#include "core/core-accessor.h"
#include "core/core-listener.h"
#include "linphone/utils/general.h"
#include "utils/background-task.h"

LINPHONE_BEGIN_NAMESPACE

class Core;

class LINPHONE_PUBLIC ClientConferenceEventHandlerBase : public CoreAccessor, public CoreListener {
	friend class ClientChatRoom;

public:
	ClientConferenceEventHandlerBase(const std::shared_ptr<Core> &core);
	virtual ~ClientConferenceEventHandlerBase();

	void startDelayMessageSendTimer(const Address address);
	void stopDelayMessageSendTimer(const Address address);
	bool delayMessageSendTimerStarted(const Address address) const;
	void setDelayTimerExpired(bool expired, const Address address);
	bool delayTimerExpired(const Address address) const;

	// virtual void publish() = 0;
	virtual bool subscribe() = 0;
	virtual void unsubscribe();
	virtual void invalidateSubscription() = 0;

protected:
	virtual void handleDelayMessageSendTimerExpired(const Address address) = 0;
	std::map<const Address, bool, Address::LessWithoutGruu> mDelayTimersExpired;
	std::map<const Address, BackgroundTask, Address::LessWithoutGruu> mDelayMessageSendBgTasks;
	std::map<const Address, belle_sip_source_t *, Address::LessWithoutGruu> mDelayMessageSendTimers;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_BASE_H_
