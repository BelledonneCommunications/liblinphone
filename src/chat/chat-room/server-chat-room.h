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

#ifndef _L_SERVER_GROUP_CHAT_ROOM_H_
#define _L_SERVER_GROUP_CHAT_ROOM_H_

#include "chat/chat-room/chat-room.h"
#include "conference/server-conference.h"

LINPHONE_BEGIN_NAMESPACE

class SalCallOp;

class ServerChatRoom : public ChatRoom {
	friend ServerConference;

public:
	struct RegistrationSubscriptionContext {
		void *context = nullptr; // TODO: unused currently, but can store a context pointer from the implementation of
		                         // reginfo subscription. This will remove the need for a map in conference server for
		                         // holding subscriptions.
	};

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

	ServerChatRoom(const std::shared_ptr<Core> &core, const std::shared_ptr<Conference> &conf);

	virtual ~ServerChatRoom();

	std::shared_ptr<Core> getCore() const;

	void allowCpim(bool value) override;
	void allowMultipart(bool value) override;
	bool canHandleCpim() const override;
	bool canHandleMultipart() const override;

	CapabilitiesMask getCapabilities() const override;
	bool hasBeenLeft() const override;
	bool isReadOnly() const override;

	// Find the other participant of a 1-1 chatroom.
	std::shared_ptr<Participant> getOtherParticipant(const std::shared_ptr<Participant> someParticipant) const;

	// we go here when receiving the first INVITE, the one that will redirect to newly allocated conference URI.
	void confirmCreation();
	void confirmRecreation(SalCallOp *op);

	LinphoneReason onSipMessageReceived(SalOp *op, const SalMessage *message) override;

	/*These are the two methods called by the registration subscription module*/
	void notifyParticipantDeviceRegistration(const std::shared_ptr<const Address> &participantDevice);

	Utils::Version getProtocolVersion() const {
		return mProtocolVersion;
	}

	void finalizeCreation();

	bool dispatchMessagesAfterFullState(const std::shared_ptr<ParticipantDevice> &device) const;
	bool dispatchMessagesAfterFullState(const std::shared_ptr<CallSession> &session) const;
	void dispatchQueuedMessages();
	static std::shared_ptr<Message> createMessage(const std::string &from,
	                                              const ContentType &contentType,
	                                              const std::string &text,
	                                              const SalCustomHeader *salCustomHeaders);

	void setJoiningPendingAfterCreation(bool value) {
		mJoiningPendingAfterCreation = value;
	}

	bool isJoiningPendingAfterCreation() const {
		return mJoiningPendingAfterCreation;
	}

	void setInitiatorDevice(const std::shared_ptr<ParticipantDevice> device) {
		mInitiatorDevice = device;
	}

	std::shared_ptr<ParticipantDevice> getInitiatorDevice() const {
		return mInitiatorDevice;
	}

	bool removeRegistrationSubscriptionParticipant(const std::shared_ptr<Address> &participantAddress);

	bool subscribeRegistrationForParticipants(const std::list<std::shared_ptr<const Address>> &participants,
	                                          bool newInvited);
	void unSubscribeRegistrationForParticipant(const std::shared_ptr<Address> &identAddresses);
	int getUnnotifiedRegistrationSubscriptions() const {
		return mUnnotifiedRegistrationSubscriptions;
	}

	const std::map<std::string, RegistrationSubscriptionContext> &getRegistrationSubscriptions() const {
		return mRegistrationSubscriptions;
	}
	void handleEphemeralSettingsChange(const std::shared_ptr<CallSession> &session);

	void setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress);

private:
	std::unordered_map<std::string, std::queue<std::shared_ptr<Message>>> mQueuedMessages;
	int mUnnotifiedRegistrationSubscriptions = 0; /*count of not-yet notified registration subscriptions*/

	std::map<std::string, RegistrationSubscriptionContext>
	    mRegistrationSubscriptions;                           /*map of mRegistrationSubscriptions for each participant*/
	std::list<Address> mRegistrationSubscriptionParticipants; // participants in the process of being added to the
	                                                          // chatroom, while for registration information.
	bool mJoiningPendingAfterCreation = false;

	/*pointer to the ParticipantDevice that is creating the chat room*/
	std::shared_ptr<ParticipantDevice> mInitiatorDevice;

	Utils::Version mProtocolVersion;

	void onBye(const std::shared_ptr<ParticipantDevice> &participantLeaving);
	void determineProtocolVersion();
	void updateProtocolVersionFromDevice(const std::shared_ptr<ParticipantDevice> &device);

	void sendMessage(const std::shared_ptr<Message> &message, const std::shared_ptr<Address> &deviceAddr);
	void queueMessage(const std::shared_ptr<Message> &message);
	void queueMessage(const std::shared_ptr<Message> &msg, const std::shared_ptr<Address> &deviceAddress);
	void removeQueuedParticipantMessages(const std::shared_ptr<Participant> &participant);

	void setEphemeralLifetimeForDevice(long time, const std::shared_ptr<CallSession> &session);
	void setEphemeralModeForDevice(AbstractChatRoom::EphemeralMode mode, const std::shared_ptr<CallSession> &session);

	static void copyMessageHeaders(const std::shared_ptr<Message> &fromMessage,
	                               const std::shared_ptr<ChatMessage> &toMessage);

	L_DISABLE_COPY(ServerChatRoom);
};

std::ostream &operator<<(std::ostream &stream, const ServerChatRoom *chatRoom);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SERVER_GROUP_CHAT_ROOM_H_
