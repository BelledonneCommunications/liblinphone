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

#ifndef _L_CLIENT_GROUP_CHAT_ROOM_H_
#define _L_CLIENT_GROUP_CHAT_ROOM_H_

#include "chat/chat-room/chat-room.h"
#include "conference/client-conference.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ClientConferenceEventHandler;
enum class SecurityLevel;

class LINPHONE_PUBLIC ClientChatRoom : public ConferenceListener, public ChatRoom {
	friend class ClientConference;
	friend class LimeX3dhEncryptionEngine;
	friend class MainDb;
	friend class ClientConferenceEventHandler;

public:
	virtual ~ClientChatRoom() = default;

	std::shared_ptr<Core> getCore() const;

	void allowCpim(bool value) override;
	void allowMultipart(bool value) override;
	bool canHandleCpim() const override;
	bool canHandleMultipart() const override;

	CapabilitiesMask getCapabilities() const override;
	ChatRoom::SecurityLevel getSecurityLevel() const override;
	bool hasBeenLeft() const override;
	bool isReadOnly() const override;

	void deleteFromDb() override;

	std::list<std::shared_ptr<EventLog>> getHistory(int nLast) const override;
	std::list<std::shared_ptr<EventLog>> getHistory(int nLast, HistoryFilterMask filters) const override;
	std::list<std::shared_ptr<EventLog>> getHistoryRange(int begin, int end) const override;
	std::list<std::shared_ptr<EventLog>> getHistoryRange(int begin, int end, HistoryFilterMask filters) const override;
	int getHistorySize() const override;
	int getHistorySize(HistoryFilterMask filters) const override;
	void exhume();

	void enableEphemeral(bool ephem, bool updateDb) override;
	bool ephemeralEnabled() const override;
	void setEphemeralLifetime(long lifetime, bool updateDb) override;
	long getEphemeralLifetime() const override;
	void setEphemeralMode(AbstractChatRoom::EphemeralMode mode, bool updateDb) override;
	AbstractChatRoom::EphemeralMode getEphemeralMode() const override;
	bool ephemeralSupportedByAllParticipants() const override;

	void stopBgTask() {
		mBgTask.stop();
	}

	void setDeletionOnTerminationEnabled(bool enable) {
		mDeletionOnTerminationEnabled = enable;
	}

	bool getDeletionOnTerminationEnabled() const {
		return mDeletionOnTerminationEnabled;
	}

	const std::list<std::shared_ptr<ChatMessage>> &getPendingCreationMessages() const {
		return mPendingCreationMessages;
	}

	void clearPendingCreationMessages() {
		mPendingCreationMessages.clear();
	}

	std::pair<bool, std::shared_ptr<AbstractChatRoom>> needToMigrate() const;

	unsigned int getLastNotifyId() const;

	void onChatRoomCreated(const std::shared_ptr<Address> &remoteContact);
	void sendChatMessage(const std::shared_ptr<ChatMessage> &chatMessage) override;

	virtual void addPendingMessage(const std::shared_ptr<ChatMessage> &chatMessage) override;
	virtual void handleMessageRejected(const std::shared_ptr<ChatMessage> &chatMessage) override;

	void onExhumedConference(const ConferenceId &oldConfId, const ConferenceId &newConfId);
	void onLocallyExhumedConference(const std::shared_ptr<Address> &remoteContact);
	void onRemotelyExhumedConference(SalCallOp *op);
	void removeConferenceIdFromPreviousList(const ConferenceId &confId);
	void addConferenceIdToPreviousList(const ConferenceId &confId) {
		mPreviousConferenceIds.push_back(confId);
	}
	const std::list<ConferenceId> &getPreviousConferenceIds() const {
		return mPreviousConferenceIds;
	};
	void addExhumeMessage(const std::shared_ptr<ChatMessage> msg);
	bool isLocalExhumePending() const {
		return mLocalExhumePending;
	}

private:
	ClientChatRoom(const std::shared_ptr<Core> &core, const std::shared_ptr<Conference> &conf);

	virtual void sendPendingMessages() override;

	void sendEphemeralUpdate();

	bool mDeletionOnTerminationEnabled = false;
	bool mListHandlerUsed = false;
	BackgroundTask mBgTask{"Subscribe/notify of full state conference"};

	std::list<std::shared_ptr<ChatMessage>> mPendingCreationMessages;

	// 1-1 exhume related
	bool mLocalExhumePending = false;
	std::list<std::shared_ptr<ChatMessage>> mPendingExhumeMessages;
	std::list<ConferenceId> mPreviousConferenceIds;

	L_DISABLE_COPY(ClientChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CLIENT_GROUP_CHAT_ROOM_H_
