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

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceId;
class RemoteConference;

class LINPHONE_PUBLIC RemoteConferenceEventHandler : public CoreListener {
	friend class ClientGroupChatRoom;

public:
	RemoteConferenceEventHandler (RemoteConference *remoteConference);
	~RemoteConferenceEventHandler ();

	void subscribe (const ConferenceId &conferenceId);
	void notifyReceived (const std::string &xmlBody);
	void multipartNotifyReceived (const std::string &xmlBody);
	void unsubscribe ();

	void setConferenceId (ConferenceId conferenceId);
	const ConferenceId &getConferenceId () const;

	unsigned int getLastNotify () const;
	void setLastNotify (unsigned int lastNotify);
	void resetLastNotify ();
	void invalidateSubscription ();

protected:
	void simpleNotifyReceived (const std::string &xmlBody);
	void subscribe ();

	// CoreListener
	void onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) override;
	void onRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const std::string &message) override;
	void onEnteringBackground () override;
	void onEnteringForeground () override;

	ConferenceId conferenceId;

	RemoteConference *conf = nullptr;
	LinphoneEvent *lev = nullptr;

	unsigned int lastNotify = 0;
	bool subscriptionWanted = false;


private:
	void unsubscribePrivate ();
	L_DISABLE_COPY(RemoteConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_
