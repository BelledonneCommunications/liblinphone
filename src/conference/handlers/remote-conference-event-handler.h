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

#ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_
#define _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_

#include "linphone/types.h"

#include "xml/conference-info.h"
#include "xml/conference-info-linphone-extension.h"
#include "conference/conference-id.h"
#include "core/core-listener.h"
#include "remote-conference-event-handler-base.h"
#include "chat/chat-room/client-group-chat-room-p.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Content;
class ConferenceId;
class Conference;
class ConferenceListener;
class RemoteConferenceEventHandlerBase;

class LINPHONE_PUBLIC RemoteConferenceEventHandler : public RemoteConferenceEventHandlerBase, public CoreListener {
	friend class ClientGroupChatRoom;

public:

	static LinphoneMediaDirection mediaStatusToMediaDirection (Xsd::ConferenceInfo::MediaStatusType status);
	RemoteConferenceEventHandler (Conference *remoteConference, ConferenceListener * listener);
	~RemoteConferenceEventHandler ();

	void subscribe (const ConferenceId &conferenceId);
	bool alreadySubscribed() const;
	void notifyReceived (const Content &content);
	void notifyReceived (LinphoneEvent *notifyLev, const Content &content);
	void multipartNotifyReceived (const Content &content);
	void multipartNotifyReceived (LinphoneEvent *notifyLev, const Content &content);
	void unsubscribe () override;

	void invalidateSubscription () override;
	void subscriptionActive();

	const ConferenceId &getConferenceId() const;
	unsigned int getLastNotify () const;
	void requestFullState();

	void setInitialSubscriptionUnderWayFlag(bool on);
	bool getInitialSubscriptionUnderWayFlag() const;

protected:
	void conferenceInfoNotifyReceived (const std::string &xmlBody);
	void conferenceInfoLinphoneExtensionNotifyReceived (const std::string &xmlBody);
	void subscribe () override;

	// CoreListener
	void onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) override;
	void onAccountRegistrationStateChanged (std::shared_ptr<Account> account, LinphoneRegistrationState state, const std::string &message) override;
	void onEnteringBackground () override;
	void onEnteringForeground () override;

	LinphoneEvent *lev = nullptr;
	Conference *conf = nullptr;
	ConferenceListener *confListener = nullptr;

	bool subscriptionWanted = false;
	bool waitingFullState = false;
	bool fullStateRequested = false;
	bool initialSubscriptionUnderWay = false;

private:
	void unsubscribePrivate ();
	void updateInitialSubcriptionUnderWay (LinphoneEvent *notifyLev);
	L_DISABLE_COPY(RemoteConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_
