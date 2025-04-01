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

#ifndef _L_CONFERENCE_PARAMS_H_
#define _L_CONFERENCE_PARAMS_H_

#include <string>

#include "belle-sip/object++.hh"

#include "chat/chat-room/chat-params.h"
#include "conference/conference-interface.h"
#include "conference/conference-params-interface.h"

LINPHONE_BEGIN_NAMESPACE

class Account;
class Address;
class Conference;
class Core;

class LINPHONE_PUBLIC ConferenceParams : public bellesip::HybridObject<LinphoneConferenceParams, ConferenceParams>,
                                         public ConferenceParamsInterface,
                                         public CoreAccessor {
	friend class ServerConference;
	friend class ClientConference;

public:
	ConferenceParams(const std::shared_ptr<Core> &core = nullptr);
	ConferenceParams(const ConferenceParams &other);

	void setAudioVideoDefaults();
	void setChatDefaults();

	ConferenceParams *clone() const override {
		return new ConferenceParams(*this);
	}

	virtual ~ConferenceParams() = default;

	bool isValid() const;
	void updateAccordingToCapabilities(AbstractChatRoom::CapabilitiesMask capabilities);

	virtual void setConferenceFactoryAddress(const std::shared_ptr<const Address> &address) override {
		mUseDefaultFactoryAddress = false;
		// Setting to nullptr means that we want to host a conference on the local device
		mFactoryAddress = address ? address->clone()->toSharedPtr() : nullptr;
	};

	std::shared_ptr<Address> getConferenceFactoryAddress() const {
		return mFactoryAddress;
	};

	void setHidden(bool enable) {
		mHidden = enable;
	}
	bool isHidden() const {
		return mHidden;
	}

	virtual void enableVideo(bool enable) override {
		mEnableVideo = enable;
	}
	bool videoEnabled() const {
		return mEnableVideo;
	}

	virtual void enableAudio(bool enable) override {
		mEnableAudio = enable;
	};
	bool audioEnabled() const {
		return mEnableAudio;
	}

	virtual void enableChat(bool enable) override;
	bool chatEnabled() const {
		return mEnableChat;
	}

	void enableLocalParticipant(bool enable) {
		mLocalParticipantEnabled = enable;
	}
	bool localParticipantEnabled() const {
		return mLocalParticipantEnabled;
	}

	void enableOneParticipantConference(bool enable) {
		mAllowOneParticipantConference = enable;
	}
	bool oneParticipantConferenceEnabled() const {
		return mAllowOneParticipantConference;
	}

	virtual void setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress) override;
	std::shared_ptr<Address> getConferenceAddress() const {
		return mConferenceAddress;
	};

	void setUtf8Description(const std::string &description) {
		mUtf8Description = description;
	}
	const std::string &getUtf8Description() const {
		return mUtf8Description;
	}
	void setDescription(const std::string &description);
	const std::string &getDescription() const;

	virtual void setUtf8Subject(const std::string &subject) override;
	void setSubject(const std::string &subject);
	const std::string &getUtf8Subject() const {
		return mUtf8Subject;
	}
	const std::string &getSubject() const;
	void setAccount(const std::shared_ptr<Account> &a);
	std::shared_ptr<Account> getAccount() const;

	void setExpiryTime(const time_t &expiryTime) {
		mExpiryTime = expiryTime;
	};
	time_t getExpiryTime() const {
		return mExpiryTime;
	};

	void setEarlierJoiningTime(const time_t &joinTime) {
		mEarlierJoiningTime = joinTime;
	};
	time_t getEarlierJoiningTime() const {
		return mEarlierJoiningTime;
	};

	virtual void setStartTime(const time_t &start) override {
		mStartTime = start;
	};
	time_t getStartTime() const {
		return mStartTime;
	};
	std::string getStartTimeString() const {
		return Utils::timeToIso8601(getStartTime());
	}

	virtual void setEndTime(const time_t &end) override {
		mEndTime = end;
	};
	time_t getEndTime() const {
		return mEndTime;
	};
	std::string getEndTimeString() const {
		return Utils::timeToIso8601(getEndTime());
	}

	virtual void setParticipantListType(const ParticipantListType &type) override {
		mParticipantListType = type;
	};
	ParticipantListType getParticipantListType() const {
		return mParticipantListType;
	};

	virtual void setJoiningMode(const JoiningMode &mode) override {
		mJoinMode = mode;
	};
	JoiningMode getJoiningMode() const {
		return mJoinMode;
	};

	virtual void setSecurityLevel(const SecurityLevel &level) override;
	const SecurityLevel &getSecurityLevel() const {
		return mSecurityLevel;
	};

	std::shared_ptr<ChatParams> getChatParams() const {
		return mChatParams;
	};

	bool isGroup() const;
	void setGroup(bool group);

	static ConferenceParams::SecurityLevel getSecurityLevelFromAttribute(const std::string &level);
	static std::string getSecurityLevelAttribute(const ConferenceParams::SecurityLevel &level);
	static AbstractChatRoom::CapabilitiesMask toCapabilities(const std::shared_ptr<ConferenceParams> &params);
	static std::shared_ptr<ConferenceParams> fromCapabilities(AbstractChatRoom::CapabilitiesMask capabilities,
	                                                          const std::shared_ptr<Core> &core = nullptr);

private:
	void updateFromAccount(const std::shared_ptr<Account> &account); // Update Me and default factory from account.

	bool mEnableVideo = false;
	bool mEnableAudio = false;
	bool mEnableChat = false;
	bool mLocalParticipantEnabled = true;
	bool mAllowOneParticipantConference = false;
	ParticipantListType mParticipantListType = ParticipantListType::Open;
	JoiningMode mJoinMode = JoiningMode::DialIn;
	std::shared_ptr<Address> mConferenceAddress = nullptr;
	std::shared_ptr<Address> mFactoryAddress = nullptr;
	SecurityLevel mSecurityLevel = SecurityLevel::None;
	bool mUseDefaultFactoryAddress = true;
	mutable std::string mSubject = "";
	std::string mUtf8Subject = "";
	mutable std::string mDescription = "";
	std::string mUtf8Description = "";
	std::shared_ptr<Address> mMe = nullptr;
	time_t mEarlierJoiningTime = -1;
	time_t mExpiryTime = -1;
	time_t mStartTime = -1;
	time_t mEndTime = -1;
	bool mGroup = true; // group chat
	std::weak_ptr<Account> mAccount;
	bool mHidden = false;
	mutable std::shared_ptr<ChatParams> mChatParams = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_PARAMS_H_
