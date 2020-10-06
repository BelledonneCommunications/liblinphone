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

#ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_
#define _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_

#include "linphone/types.h"

#include "conference/conference-id.h"
#include "core/core-listener.h"
#include "remote-conference-event-handler-base.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceId;
class Conference;
class ConferenceListener;
class RemoteConferenceEventHandlerBase;

class LINPHONE_PUBLIC RemoteConferenceEventHandler : public RemoteConferenceEventHandlerBase, public CoreListener {
	friend class ClientGroupChatRoom;

public:
	RemoteConferenceEventHandler (Conference *remoteConference, ConferenceListener * listener);
	~RemoteConferenceEventHandler ();

	void subscribe (const ConferenceId &conferenceId);
	void notifyReceived (const std::string &xmlBody);
	void multipartNotifyReceived (const std::string &xmlBody);
	void unsubscribe () override;

	void invalidateSubscription () override;

	const ConferenceId &getConferenceId() const;
	unsigned int getLastNotify () const;

protected:
	void simpleNotifyReceived (const std::string &xmlBody);
	void subscribe () override;

	// CoreListener
	void onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) override;
	void onRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const std::string &message) override;
	void onEnteringBackground () override;
	void onEnteringForeground () override;

	Conference *conf = nullptr;
	ConferenceListener *confListener = nullptr;
	LinphoneEvent *lev = nullptr;

	bool subscriptionWanted = false;

private:
	void unsubscribePrivate ();
	L_DISABLE_COPY(RemoteConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_
