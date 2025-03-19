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

#ifndef _L_BASIC_CHAT_ROOM_H_
#define _L_BASIC_CHAT_ROOM_H_

#include "chat/chat-room/chat-room.h"
#include "conference/conference-id.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC BasicChatRoom : public ChatRoom {
	friend class Core;
	friend class CorePrivate;

public:
	void allowCpim(bool value) override;
	void allowMultipart(bool value) override;
	bool canHandleCpim() const override;
	bool canHandleMultipart() const override;
	std::shared_ptr<ConferenceParams> getCurrentParams() const override;

	CapabilitiesMask getCapabilities() const override;
	bool hasBeenLeft() const override;
	bool isReadOnly() const override;

	const ConferenceId &getConferenceId() const override;
	void setConferenceId(const ConferenceId &conferenceId);
	std::optional<std::reference_wrapper<const std::string>> getIdentifier() const override;

	void invalidateAccount() override;
	std::shared_ptr<Account> getAccount() override;

	const std::string &getSubjectUtf8() const override;
	void setUtf8Subject(const std::string &subject) override;

	void setState(ConferenceInterface::State newState) override;
	ConferenceInterface::State getState() const override;

	bool isMe(const std::shared_ptr<Address> &address) const override;
	std::shared_ptr<Participant> getMe() const override;
	std::list<std::shared_ptr<Participant>> getParticipants() const override;
	std::list<std::shared_ptr<Address>> getParticipantAddresses() const override;

protected:
	explicit BasicChatRoom(const std::shared_ptr<Core> &core,
	                       const ConferenceId &conferenceId,
	                       const std::shared_ptr<ConferenceParams> &params);

private:
	ConferenceId mConferenceId;
	std::shared_ptr<ConferenceParams> mParams;
	std::shared_ptr<Participant> mMe;
	std::list<std::shared_ptr<Participant>> mParticipants;
	bool mCpimAllowed = false;
	bool mMultipartAllowed = false;
	std::string mSubject;

	ConferenceInterface::State mState = ConferenceInterface::State::None;

	L_DISABLE_COPY(BasicChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_BASIC_CHAT_ROOM_H_
