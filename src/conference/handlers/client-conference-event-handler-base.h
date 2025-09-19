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

	// virtual void publish() = 0;
	virtual bool subscribe() = 0;
	virtual void unsubscribe();
	virtual void invalidateSubscription() = 0;

	/*
	 * HACK
	 * The below three methods and mWaitNotifyTimer are there to workaround a signaling issue
	 * we have with Flexisip conference server <= 2.4, that may not send a NOTIFY for
	 * conference event SUBSCRIBE if there is nothing to notify.
	 * They should not do this: the NOTIFY is required in any case, at least to set the Subscription-state to active.
	 *
	 * The receiving of the NOTIFY is important, because it carries information about users and devices
	 * which are necessary to forge LIME parts for chatroom's outgoing messages.
	 * In order to avoid sending incomplete messages (messages for which not all recipients have their lime part),
	 * we always wait for the initial NOTIFY.
	 * Because of this Flexisip <= 2.4 bug, we may have cases where the NOTIFY will never happen.
	 * The purpose of the timer is to workaround this, by simulating the receival of the
	 * expected empty NOTIFY.
	 * This code will need to be removed when flexisip 2.5 is in production everywhere.
	 */
	void startWaitNotifyTimer();
	void stopWaitNotifyTimer();
	virtual void onNotifyWaitExpired() = 0;

private:
	belle_sip_source_t *mWaitNotifyTimer = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_BASE_H_
