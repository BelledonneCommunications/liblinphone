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

#ifndef _L_CLIENT_GROUP_CHAT_ROOM_H_
#define _L_CLIENT_GROUP_CHAT_ROOM_H_

#include "chat/chat-room/chat-room.h"
#include "conference/remote-conference.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ClientGroupChatRoomPrivate;
enum class SecurityLevel;

class LINPHONE_PUBLIC ClientGroupChatRoom :
	public ConferenceListener,
	public ConferenceListenerInterface,
	public ChatRoom {
	friend class BasicToClientGroupChatRoomPrivate;
	friend class ClientGroupToBasicChatRoomPrivate;
	friend class CorePrivate;
	friend class LimeX3dhEncryptionEngine;
	friend class MainDb;

public:
	L_OVERRIDE_SHARED_FROM_THIS(ClientGroupChatRoom);

	~ClientGroupChatRoom ();

	std::shared_ptr<Core> getCore () const;

	void allowCpim (bool value) override;
	void allowMultipart (bool value) override;
	bool canHandleCpim () const override;
	bool canHandleMultipart () const override;

	CapabilitiesMask getCapabilities () const override;
	ChatRoom::SecurityLevel getSecurityLevel () const override;
	bool hasBeenLeft () const override;

	const ConferenceAddress getConferenceAddress () const override;

	void deleteFromDb () override;

	std::list<std::shared_ptr<EventLog>> getHistory (int nLast) const override;
	std::list<std::shared_ptr<EventLog>> getHistoryRange (int begin, int end) const override;
	int getHistorySize () const override;

	bool addParticipant (const IdentityAddress &participantAddress) override;
	bool addParticipant (std::shared_ptr<Call> call) override {return getConference()->addParticipant(call); };
	bool addParticipants (const std::list<IdentityAddress> &addresses) override;

	void join (const IdentityAddress &participantAddress) override { getConference()->join(participantAddress); };
	bool update(const ConferenceParamsInterface &newParameters) override { return getConference()->update(newParameters); };

	bool removeParticipant (const std::shared_ptr<Participant> &participant) override;

	std::shared_ptr<Participant> findParticipant (const IdentityAddress &addr) const override;

	std::shared_ptr<Participant> getMe () const override;
	int getParticipantCount () const override;
	const std::list<std::shared_ptr<Participant>> &getParticipants () const override;

	void setParticipantAdminStatus (const std::shared_ptr<Participant> &participant, bool isAdmin) override;

	const std::string &getSubject () const override;
	void setSubject (const std::string &subject) override;

	void join () override;
	void leave () override;
	
	void enableEphemeral (bool ephem, bool updateDb) override;
	bool ephemeralEnabled () const override;
	void setEphemeralLifetime (long lifetime, bool updateDb) override;
	long getEphemeralLifetime () const override;
	bool ephemeralSupportedByAllParticipants () const override;

	const ConferenceId &getConferenceId () const override { return getConference()->getConferenceId(); };

private:
	ClientGroupChatRoom (
		const std::shared_ptr<Core> &core,
		const IdentityAddress &focus,
		const ConferenceId &conferenceId,
		const std::string &subject,
		const Content &content,
		CapabilitiesMask capabilities,
		const std::shared_ptr<ChatRoomParams> &params
	);

	ClientGroupChatRoom (
		const std::shared_ptr<Core> &core,
		const std::string &factoryUri,
		const IdentityAddress &me,
		const std::string &subject,
		CapabilitiesMask capabilities,
		const std::shared_ptr<ChatRoomParams> &params
	);

	// Create a chat room from the main database.
	ClientGroupChatRoom (
		const std::shared_ptr<Core> &core,
		const ConferenceId &conferenceId,
		std::shared_ptr<Participant> &me,
		AbstractChatRoom::CapabilitiesMask capabilities,
		const std::shared_ptr<ChatRoomParams> &params,
		const std::string &subject,
		std::list<std::shared_ptr<Participant>> &&participants,
		unsigned int lastNotifyId,
		bool hasBeenLeft = false
	);

	// TODO: Move me in ClientGroupChatRoomPrivate.
	// ALL METHODS AFTER THIS POINT.

	void onConferenceCreated (const ConferenceAddress &addr) override;
	void onConferenceKeywordsChanged (const std::vector<std::string> &keywords) override;
	void onConferenceTerminated (const IdentityAddress &addr) override;
	void onSecurityEvent (const std::shared_ptr<ConferenceSecurityEvent> &event) override;
	void onFirstNotifyReceived (const IdentityAddress &addr) override;
	void onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event) override;
	void onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) override;
	void onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) override;
	void onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event) override;
	void onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event) override;
	void onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event) override;

	void onParticipantsCleared () override;

	L_DECLARE_PRIVATE(ClientGroupChatRoom);
	L_DISABLE_COPY(ClientGroupChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CLIENT_GROUP_CHAT_ROOM_H_
