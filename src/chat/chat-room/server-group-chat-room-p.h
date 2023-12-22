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

#ifndef _L_SERVER_GROUP_CHAT_ROOM_P_H_
#define _L_SERVER_GROUP_CHAT_ROOM_P_H_

#include <chrono>
#include <map>
#include <queue>
#include <unordered_map>

#include "chat-room-p.h"
#include "server-group-chat-room.h"

#include "conference/participant-device.h"
#include "content/content-manager.h"
#include "object/clonable-object-p.h"
#include "object/clonable-object.h"

#include "linphone/utils/utils.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ParticipantDeviceIdentity
    : public bellesip::HybridObject<LinphoneParticipantDeviceIdentity, ParticipantDeviceIdentity> {
public:
	ParticipantDeviceIdentity(const std::shared_ptr<Address> &address, const std::string &name);
	void setCapabilityDescriptor(const std::string &capabilities);
	void setCapabilityDescriptor(const std::list<std::string> &capabilities);
	const std::shared_ptr<Address> &getAddress() const {
		return mDeviceAddress;
	}
	const std::string &getName() const {
		return mDeviceName;
	}
	const std::string &getCapabilityDescriptor() const;
	const std::list<std::string> getCapabilityDescriptorList() const;
	virtual ~ParticipantDeviceIdentity();

private:
	std::shared_ptr<Address> mDeviceAddress;
	std::string mDeviceName;
	mutable std::string mCapabilityDescriptorString;
	std::map<std::string, std::string> mCapabilityDescriptor; // +org.linphone.specs capability descriptor
};

class ServerGroupChatRoomPrivate : public ChatRoomPrivate {
public:
	ServerGroupChatRoomPrivate(void)
	    : ChatRoomPrivate(AbstractChatRoom::CapabilitiesMask({ChatRoom::Capabilities::Conference})){};
	ServerGroupChatRoomPrivate(AbstractChatRoom::CapabilitiesMask value)
	    : ChatRoomPrivate((value | ChatRoom::Capabilities::Conference)){};

	virtual ~ServerGroupChatRoomPrivate() = default;

	std::shared_ptr<Participant> addParticipant(const std::shared_ptr<Address> &participantAddress);
	void removeParticipant(const std::shared_ptr<Participant> &participant);

	void setParticipantDeviceState(const std::shared_ptr<ParticipantDevice> &device,
	                               ParticipantDevice::State state,
	                               bool notify = true);
	// Find the other participant of a 1-1 chatroom.
	std::shared_ptr<Participant> getOtherParticipant(const std::shared_ptr<Participant> someParticipant) const;

	// Make a search in the list of participants that are currently part of the chat room or in the process of leaving
	std::shared_ptr<ParticipantDevice>
	findCachedParticipantDevice(const std::shared_ptr<const CallSession> &session) const;

	void acceptSession(const std::shared_ptr<CallSession> &session);
	// we go here when receiving the first INVITE, the one that will redirect to newly allocated conference URI.
	void confirmCreation();
	void confirmJoining(SalCallOp *op);
	void confirmRecreation(SalCallOp *op);
	void declineSession(const std::shared_ptr<CallSession> &session, LinphoneReason reason);
	void dispatchQueuedMessages();

	void subscriptionStateChanged(const std::shared_ptr<EventSubscribe> &event, LinphoneSubscriptionState state);

	bool initializeParticipants(const std::shared_ptr<Participant> &initiator, SalCallOp *op);
	void resumeParticipant(const std::shared_ptr<Participant> &participant);
	bool subscribeRegistrationForParticipants(const std::list<std::shared_ptr<Address>> &participants, bool newInvited);
	void unSubscribeRegistrationForParticipant(const std::shared_ptr<Address> &identAddresses);
	void handleSubjectChange(SalCallOp *op);
	void handleEphemeralSettingsChange(const std::shared_ptr<CallSession> &session);
	void setEphemeralLifetime(long time, const std::shared_ptr<CallSession> &session);
	void setEphemeralMode(AbstractChatRoom::EphemeralMode mode, const std::shared_ptr<CallSession> &session);

	void setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress);
	void updateParticipantDevices(const std::shared_ptr<Address> &addr,
	                              const std::list<std::shared_ptr<ParticipantDeviceIdentity>> &devices);
	void setParticipantDevicesAtCreation(const std::shared_ptr<Address> &addr,
	                                     const std::list<std::shared_ptr<ParticipantDeviceIdentity>> &devices);
	void updateParticipantDeviceSession(const std::shared_ptr<ParticipantDevice> &device,
	                                    bool freslyRegistered = false);
	void updateParticipantsSessions();
	void conclude();
	void requestDeletion();

	LinphoneReason onSipMessageReceived(SalOp *op, const SalMessage *message) override;

	/*These are the two methods called by the registration subscription module*/
	void setParticipantDevices(const std::shared_ptr<Address> &addr,
	                           const std::list<std::shared_ptr<ParticipantDeviceIdentity>> &devices);
	void notifyParticipantDeviceRegistration(const std::shared_ptr<Address> &participantDevice);

private:
	struct Message {
		Message(const std::string &from,
		        const ContentType &contentType,
		        const std::string &text,
		        const SalCustomHeader *salCustomHeaders)
		    : fromAddr(Address::create(from)) {
			content.setContentType(contentType);
			if (!text.empty()) {
				content.setBodyFromUtf8(text);

				// If the message is encrypted, the server group chat room will send it to every participant in a
				// message where only the LIME key of the recipee is present.  Instead of parsing the content of the
				// received message for every outbound message to look for the section containing the LIME key that
				// causes important performance issues, the content is parsed once and the LIME encryption engine can
				// get the information it needs.
				if (contentType.isValid() && (contentType == ContentType::Encrypted)) {
					contentsList = ContentManager::multipartToContentList(content);
				}
			}
			if (salCustomHeaders) {
				customHeaders = sal_custom_header_clone(salCustomHeaders);
			}
		}

		~Message() {
			if (customHeaders) sal_custom_header_free(customHeaders);
		}

		std::shared_ptr<Address> fromAddr;
		Content content;
		std::list<Content> contentsList;
		std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
		SalCustomHeader *customHeaders = nullptr;
	};

	static void copyMessageHeaders(const std::shared_ptr<Message> &fromMessage,
	                               const std::shared_ptr<ChatMessage> &toMessage);
	static bool allDevicesLeft(const std::shared_ptr<Participant> &participant);
	void addParticipantDevice(const std::shared_ptr<Participant> &participant,
	                          const std::shared_ptr<ParticipantDeviceIdentity> &deviceInfo);
	void designateAdmin();
	void sendMessage(const std::shared_ptr<Message> &message, const std::shared_ptr<Address> &deviceAddr);
	void finalizeCreation();
	std::shared_ptr<CallSession> makeSession(const std::shared_ptr<ParticipantDevice> &device);
	void inviteDevice(const std::shared_ptr<ParticipantDevice> &device);
	void byeDevice(const std::shared_ptr<ParticipantDevice> &device);
	bool isAdminLeft() const;
	void queueMessage(const std::shared_ptr<Message> &message);
	void queueMessage(const std::shared_ptr<Message> &msg, const std::shared_ptr<Address> &deviceAddress);
	void removeParticipantDevice(const std::shared_ptr<Participant> &participant,
	                             const std::shared_ptr<Address> &deviceAddress);

	void removeCachedParticipant(const std::shared_ptr<Address> &address);
	void onParticipantDeviceLeft(const std::shared_ptr<ParticipantDevice> &device);
	void onBye(const std::shared_ptr<ParticipantDevice> &participantLeaving);
	void determineProtocolVersion();
	void updateProtocolVersionFromDevice(const std::shared_ptr<ParticipantDevice> &device);
	bool dispatchMessagesAfterFullState(const std::shared_ptr<ParticipantDevice> &device) const;
	bool dispatchMessagesAfterFullState(const std::shared_ptr<CallSession> &session) const;

	// ChatRoomListener
	void onChatRoomInsertRequested(const std::shared_ptr<AbstractChatRoom> &chatRoom) override;
	void onChatRoomInsertInDatabaseRequested(const std::shared_ptr<AbstractChatRoom> &chatRoom) override;
	void onChatRoomDeleteRequested(const std::shared_ptr<AbstractChatRoom> &chatRoom) override;

	// CallSessionListener
	void onCallSessionStateChanged(const std::shared_ptr<CallSession> &session,
	                               CallSession::State newState,
	                               const std::string &message) override;
	void onCallSessionSetReleased(const std::shared_ptr<CallSession> &session) override;
	void onAckReceived(const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) override;

	void moveDeviceToPresent(const std::shared_ptr<CallSession> &session);
	void moveDeviceToPresent(const std::shared_ptr<ParticipantDevice> &device);

	struct RegistrationSubscriptionContext {
		void *context = nullptr; // TODO: unused currently, but can store a context pointer from the implementation of
		                         // reginfo subscription. This will remove the need for a map in conference server for
		                         // holding subscriptions.
	};

	std::list<Address> invitedParticipants; // participants in the process of being added to the chatroom, while for
	                                        // registration information.
	ChatRoomListener *chatRoomListener = this;
	std::map<std::string, RegistrationSubscriptionContext>
	    registrationSubscriptions;               /*map of registrationSubscriptions for each participant*/
	int unnotifiedRegistrationSubscriptions = 0; /*count of not-yet notified registration subscriptions*/
	std::shared_ptr<ParticipantDevice>
	    mInitiatorDevice; /*pointer to the ParticipantDevice that is creating the chat room*/
	std::unordered_map<std::string, std::queue<std::shared_ptr<Message>>> queuedMessages;
	Utils::Version protocolVersion;
	bool joiningPendingAfterCreation = false;
	L_DECLARE_PUBLIC(ServerGroupChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SERVER_GROUP_CHAT_ROOM_P_H_
