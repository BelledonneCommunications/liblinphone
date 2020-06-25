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

#ifndef _L_SERVER_GROUP_CHAT_ROOM_P_H_
#define _L_SERVER_GROUP_CHAT_ROOM_P_H_

#include <chrono>
#include <queue>
#include <unordered_map>
#include <map>

#include "chat-room-p.h"
#include "server-group-chat-room.h"

#include "conference/participant-device.h"
#include "object/clonable-object.h"
#include "object/clonable-object-p.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ParticipantDevice;

class ParticipantDeviceIdentityPrivate : public ClonableObjectPrivate {
public:
	Address deviceAddress;
	std::string deviceName;
};

class ParticipantDeviceIdentity : public ClonableObject {
public:
	ParticipantDeviceIdentity (const Address &address, const std::string &name);
	ParticipantDeviceIdentity (const ParticipantDeviceIdentity &other);

	ParticipantDeviceIdentity* clone () const override {
		return new ParticipantDeviceIdentity(*this);
	}

	const Address &getAddress () const;
	const std::string &getName () const;
private:
	L_DECLARE_PRIVATE(ParticipantDeviceIdentity);
};

class ServerGroupChatRoomPrivate : public ChatRoomPrivate {
public:
	ServerGroupChatRoomPrivate(void) : ChatRoomPrivate(AbstractChatRoom::CapabilitiesMask({ChatRoom::Capabilities::Conference})) {};
	ServerGroupChatRoomPrivate(AbstractChatRoom::CapabilitiesMask value) : ChatRoomPrivate((value | ChatRoom::Capabilities::Conference)) {};

	
	std::shared_ptr<Participant> addParticipant (const IdentityAddress &participantAddress);
	void removeParticipant (const std::shared_ptr<const Participant> &participant);

	std::shared_ptr<Participant> findAuthorizedParticipant (const std::shared_ptr<const CallSession> &session) const;
	std::shared_ptr<Participant> findAuthorizedParticipant (const IdentityAddress &participantAddress) const;

	void setParticipantDeviceState (const std::shared_ptr<ParticipantDevice> &device, ParticipantDevice::State state);

	void acceptSession (const std::shared_ptr<CallSession> &session);
	// we go here when receiving the first INVITE, the one that will redirect to newly allocated conference URI.
	void confirmCreation ();
	void confirmJoining (SalCallOp *op);
	void confirmRecreation (SalCallOp *op);
	void declineSession (const std::shared_ptr<CallSession> &session, LinphoneReason reason);
	void dispatchQueuedMessages ();

	void subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state);

	bool initializeParticipants(const std::shared_ptr<Participant> & initiator, SalCallOp *op);
	void resumeParticipant(const std::shared_ptr<Participant> &participant);
	bool subscribeRegistrationForParticipants(const std::list<IdentityAddress> &participants, bool newInvited);
	void unSubscribeRegistrationForParticipant(const IdentityAddress &identAddresses);
	void handleSubjectChange(SalCallOp *op);

	void setConferenceAddress (const ConferenceAddress &conferenceAddress);
	void updateParticipantDevices (const IdentityAddress &addr, const std::list<ParticipantDeviceIdentity> &devices);
	void setParticipantDevicesAtCreation(const IdentityAddress &addr, const std::list<ParticipantDeviceIdentity> &devices);
	void updateParticipantDeviceSession(const std::shared_ptr<ParticipantDevice> &device, bool freslyRegistered = false);
	void updateParticipantsSessions();
	void conclude();
	void requestDeletion();

	LinphoneReason onSipMessageReceived (SalOp *op, const SalMessage *message) override;
	
	/*These are the two methods called by the registration subscription module*/
	void setParticipantDevices(const IdentityAddress &addr, const std::list<ParticipantDeviceIdentity> &devices);
	void notifyParticipantDeviceRegistration(const IdentityAddress &participantDevice);

private:
	struct Message {
		Message (const std::string &from, const ContentType &contentType, const std::string &text, const SalCustomHeader *salCustomHeaders)
			: fromAddr(from)
		{
			content.setContentType(contentType);
			if (!text.empty())
				content.setBodyFromUtf8(text);
			if (salCustomHeaders)
				customHeaders = sal_custom_header_clone(salCustomHeaders);
		}

		~Message () {
			if (customHeaders)
				sal_custom_header_free(customHeaders);
		}

		IdentityAddress fromAddr;
		Content content;
		std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
		SalCustomHeader *customHeaders = nullptr;
	};

	static void copyMessageHeaders (const std::shared_ptr<Message> &fromMessage, const std::shared_ptr<ChatMessage> &toMessage);
	static bool allDevicesLeft(const std::shared_ptr<Participant> &participant);
	void addParticipantDevice (const std::shared_ptr<Participant> &participant, const ParticipantDeviceIdentity &deviceInfo);
	void designateAdmin ();
	void sendMessage (const std::shared_ptr<Message> &message, const IdentityAddress &deviceAddr);
	void finalizeCreation ();
	std::shared_ptr<CallSession> makeSession(const std::shared_ptr<ParticipantDevice> &device);
	void inviteDevice (const std::shared_ptr<ParticipantDevice> &device);
	void byeDevice (const std::shared_ptr<ParticipantDevice> &device);
	bool isAdminLeft () const;
	void queueMessage (const std::shared_ptr<Message> &message);
	void queueMessage (const std::shared_ptr<Message> &msg, const IdentityAddress &deviceAddress);
	void removeParticipantDevice (const std::shared_ptr<Participant> &participant, const IdentityAddress &deviceAddress);

	void onParticipantDeviceLeft (const std::shared_ptr<ParticipantDevice> &device);

	// ChatRoomListener
	void onChatRoomInsertRequested (const std::shared_ptr<AbstractChatRoom> &chatRoom) override;
	void onChatRoomInsertInDatabaseRequested (const std::shared_ptr<AbstractChatRoom> &chatRoom) override;
	void onChatRoomDeleteRequested (const std::shared_ptr<AbstractChatRoom> &chatRoom) override;

	// CallSessionListener
	void onCallSessionStateChanged (
		const std::shared_ptr<CallSession> &session,
		CallSession::State newState,
		const std::string &message
	) override;
	void onCallSessionSetReleased (const std::shared_ptr<CallSession> &session) override;
	void onAckReceived (const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) override;
	struct RegistrationSubscriptionContext{
		void *context = nullptr; // TODO: unused currently, but can store a context pointer from the implementation of reginfo subscription.
					 // This will remove the need for a map in conference server for holding subscriptions.
	};
	
	std::list<std::shared_ptr<Participant>> authorizedParticipants; /*list of participant authorized to send messages to the chatroom.
					This typically excludes participants that in the process of being removed.*/
	std::list<IdentityAddress> invitedParticipants; // participants in the process of being added to the chatroom, while for registration information.
	ChatRoomListener *chatRoomListener = this;
	std::map<std::string, RegistrationSubscriptionContext> registrationSubscriptions; /*map of registrationSubscriptions for each participant*/
	int unnotifiedRegistrationSubscriptions = 0; /*count of not-yet notified registration subscriptions*/
	std::shared_ptr<ParticipantDevice> mInitiatorDevice; /*pointer to the ParticipantDevice that is creating the chat room*/
	bool joiningPendingAfterCreation = false;
	std::unordered_map<std::string, std::queue<std::shared_ptr<Message>>> queuedMessages;

	L_DECLARE_PUBLIC(ServerGroupChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SERVER_GROUP_CHAT_ROOM_P_H_
