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

#ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_
#define _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_

#include "linphone/types.h"

#include "chat/chat-room/client-chat-room.h"
#include "client-conference-event-handler-base.h"
#include "conference/conference-id.h"
#include "core/core-listener.h"
#include "xml/conference-info-linphone-extension.h"
#include "xml/conference-info.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Content;
class ConferenceId;
class Conference;
class ConferenceListener;
class EventSubscribe;
class ClientConferenceEventHandlerBase;

class LINPHONE_PUBLIC ClientConferenceEventHandler : public std::enable_shared_from_this<ClientConferenceEventHandler>,
                                                     public CoreAccessor,
                                                     public ClientConferenceEventHandlerBase,
                                                     public CoreListener {
public:
	ClientConferenceEventHandler(const std::shared_ptr<Core> &core,
	                             const std::shared_ptr<Conference> &clientConference,
	                             ConferenceListener *listener);
	~ClientConferenceEventHandler();

	bool subscribe(const ConferenceId &conferenceId);
	bool notAlreadySubscribed() const;
	bool needToSubscribe() const;
	void notifyReceived(const Content &content);
	void notifyReceived(std::shared_ptr<Event> notifyLev, const Content &content);
	void multipartNotifyReceived(const Content &content);
	void multipartNotifyReceived(std::shared_ptr<Event> notifyLev, const Content &content);
	void unsubscribe() override;

	void invalidateSubscription() override;
	void subscriptionActive();

	const ConferenceId &getConferenceId() const;
	std::shared_ptr<Conference> getConference() const;
	unsigned int getLastNotify() const;
	void requestFullState();

	void setInitialSubscriptionUnderWayFlag(bool on);
	bool getInitialSubscriptionUnderWayFlag() const;

	void setManagedByListEventhandler(bool managed);

	static void subscribeStateChangedCb(LinphoneEvent *lev, LinphoneSubscriptionState state);

protected:
	void conferenceInfoNotifyReceived(const std::string &xmlBody);
	void conferenceInfoLinphoneExtensionNotifyReceived(const std::string &xmlBody);
	bool subscribe() override;

	// CoreListener
	void onNetworkReachable(bool sipNetworkReachable, bool mediaNetworkReachable) override;
	void onAccountRegistrationStateChanged(std::shared_ptr<Account> account,
	                                       LinphoneRegistrationState state,
	                                       const std::string &message) override;
	void onEnteringBackground() override;
	void onEnteringForeground() override;

	std::shared_ptr<EventSubscribe> ev = nullptr;
	std::weak_ptr<Conference> conf;
	ConferenceListener *confListener = nullptr;

	bool managedByListEventhandler = false;
	bool subscriptionWanted = false;
	bool waitingFullState = false;
	bool fullStateRequested = false;
	bool initialSubscriptionUnderWay = false;

private:
	void setSubscriptionWanted(bool wanted);
	void unsubscribePrivate();
	void updateInitialSubcriptionUnderWay(std::shared_ptr<Event> notifyLev);
	time_t dateTimeToTimeT(const Xsd::XmlSchema::DateTime &xsdTime) const;
	void fillParticipantAttributes(std::shared_ptr<Participant> &participant,
	                               xsd::cxx::tree::optional<LinphonePrivate::Xsd::ConferenceInfo::UserRolesType> &roles,
	                               Xsd::ConferenceInfo::StateType state,
	                               bool isFullState,
	                               bool notify) const;
	L_DISABLE_COPY(ClientConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_H_
