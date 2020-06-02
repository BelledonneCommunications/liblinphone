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

#ifndef _L_CONFERENCE_H_
#define _L_CONFERENCE_H_

#include "linphone/types.h"

#include "conference/conference-interface.h"
#include "conference/conference-listener.h"
#include "core/core-accessor.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSession;
class CallSessionListener;
class CallSessionPrivate;
class Content;
class ParticipantDevice;

class LINPHONE_PUBLIC Conference :
	public ConferenceInterface,
	public ConferenceListener,
	public CoreAccessor {
	friend class CallSessionPrivate;

public:
	~Conference();

	std::shared_ptr<Participant> getActiveParticipant () const;

	std::shared_ptr<Participant> findParticipant (const std::shared_ptr<const CallSession> &session) const;
	std::shared_ptr<ParticipantDevice> findParticipantDevice (const std::shared_ptr<const CallSession> &session) const;

	/* ConferenceInterface */
	bool addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) override;
	bool addParticipants (const std::list<IdentityAddress> &addresses, const CallSessionParams *params, bool hasMedia) override;
	bool canHandleParticipants () const override;
	std::shared_ptr<Participant> findParticipant (const IdentityAddress &addr) const override;
	const ConferenceAddress &getConferenceAddress () const override;
	std::shared_ptr<Participant> getMe () const override;
	int getParticipantCount () const override;
	const std::list<std::shared_ptr<Participant>> &getParticipants () const override;
	const std::string &getSubject () const override;
	void join () override;
	void leave () override;
	bool removeParticipant (const std::shared_ptr<Participant> &participant) override;
	bool removeParticipants (const std::list<std::shared_ptr<Participant>> &participants) override;
	void setParticipantAdminStatus (const std::shared_ptr<Participant> &participant, bool isAdmin) override;
	void setSubject (const std::string &subject) override;

	std::string getResourceLists (const std::list<IdentityAddress> &addresses) const;
	static std::list<IdentityAddress> parseResourceLists (const Content &content);

	void addListener(std::shared_ptr<ConferenceListenerInterface> listener) override {
		confListeners.push_back(listener);
	}

	const ConferenceId &getConferenceId () const;
	inline unsigned int getLastNotify () const { return lastNotify; };

	void subscribeReceived (LinphoneEvent *event);

	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantAdded (time_t creationTime, const bool isFullState, const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantRemoved (time_t creationTime, const bool isFullState, const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantSetAdmin (time_t creationTime, const bool isFullState, const Address &addr, bool isAdmin);
	std::shared_ptr<ConferenceSubjectEvent> notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject);
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceAdded (time_t creationTime, const bool isFullState, const Address &addr, const Address &gruu, const std::string name = "");
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceRemoved (time_t creationTime, const bool isFullState, const Address &addr, const Address &gruu);

	void notifyFullState ();

protected:
	explicit Conference (
		const std::shared_ptr<Core> &core,
		const IdentityAddress &myAddress,
		CallSessionListener *listener
	);

	bool isMe (const IdentityAddress &addr) const;

	ConferenceAddress conferenceAddress;
	std::list<std::shared_ptr<Participant>> participants;
	std::string subject;

	std::shared_ptr<Participant> activeParticipant;
	std::shared_ptr<Participant> me;

	std::list<std::shared_ptr<ConferenceListenerInterface>> confListeners;

	CallSessionListener *listener = nullptr;

	void setLastNotify (unsigned int lastNotify);
	void resetLastNotify ();
	void setConferenceId (const ConferenceId &conferenceId);

	ConferenceId conferenceId;

	// lastNotify belongs to the conference and not the the event handler.
	// The event handler can access it using the getter
	unsigned int lastNotify;

private:
	L_DISABLE_COPY(Conference);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_H_
