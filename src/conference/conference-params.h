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

#include "conference/conference-interface.h"
#include "conference/conference-params-interface.h"
#include "linphone/core.h"

LINPHONE_BEGIN_NAMESPACE

class Account;
class Address;
namespace MediaConference { // They are in a special namespace because of conflict of generic Conference classes in
	                        // src/conference/*
class Conference;
class LocalConference;
class RemoteConference;
} // namespace MediaConference

class LINPHONE_PUBLIC ConferenceParams : public bellesip::HybridObject<LinphoneConferenceParams, ConferenceParams>,
                                         public ConferenceParamsInterface {
	friend class MediaConference::Conference;
	friend class MediaConference::LocalConference;
	friend class MediaConference::RemoteConference;

public:
	ConferenceParams(const LinphoneCore *core = NULL);

	ConferenceParams *clone() const override {
		return new ConferenceParams(*this);
	}

	virtual void setConferenceFactoryAddress(const std::shared_ptr<Address> &address) override {
		m_useDefaultFactoryAddress = false;
		m_factoryAddress = address; // Setting to nullptr means that we want to host a conference on the local device
	};

	const std::shared_ptr<Address> &getConferenceFactoryAddress() const {
		return m_factoryAddress;
	};

	void setHidden(bool enable) {
		m_hidden = enable;
	}
	bool isHidden() const {
		return m_hidden;
	}

	void setStatic(bool enable) {
		m_static = enable;
	}
	bool isStatic() const {
		return m_static;
	}

	virtual void enableVideo(bool enable) override {
		m_enableVideo = enable;
	}
	bool videoEnabled() const {
		return m_enableVideo;
	}

	virtual void enableAudio(bool enable) override {
		m_enableAudio = enable;
	};
	bool audioEnabled() const {
		return m_enableAudio;
	}

	virtual void enableChat(bool enable) override {
		m_enableChat = enable;
	};
	bool chatEnabled() const {
		return m_enableChat;
	}

	void enableLocalParticipant(bool enable) {
		m_localParticipantEnabled = enable;
	}
	bool localParticipantEnabled() const {
		return m_localParticipantEnabled;
	}

	void enableOneParticipantConference(bool enable) {
		m_allowOneParticipantConference = enable;
	}
	bool oneParticipantConferenceEnabled() const {
		return m_allowOneParticipantConference;
	}

	virtual void setConferenceAddress(const std::shared_ptr<Address> conferenceAddress) override;
	const std::shared_ptr<Address> &getConferenceAddress() const {
		return m_conferenceAddress;
	};

	void setUtf8Description(const std::string &description);
	void setDescription(const std::string &description) {
		m_description = description;
	};
	const std::string &getDescription() const {
		return m_description;
	};
	const std::string &getUtf8Description() const;

	virtual void setUtf8Subject(const std::string &subject) override;
	virtual void setSubject(const std::string &subject) override {
		m_subject = subject;
	};
	const std::string &getUtf8Subject() const;
	const std::string &getSubject() const {
		return m_subject;
	};

	virtual void setMe(const std::shared_ptr<Address> &participantAddress) override {
		m_me = participantAddress ? participantAddress->clone()->toSharedPtr() : nullptr;
	};
	const std::shared_ptr<Address> &getMe() const {
		return m_me;
	};

	void setAccount(const std::shared_ptr<Account> &a);
	const std::shared_ptr<Account> getAccount() const {
		return m_account.lock();
	};

	virtual void setStartTime(const time_t &start) override {
		m_startTime = start;
	};
	const time_t &getStartTime() const {
		return m_startTime;
	};

	virtual void setEndTime(const time_t &end) override {
		m_endTime = end;
	};
	const time_t &getEndTime() const {
		return m_endTime;
	};

	virtual void setParticipantListType(const ParticipantListType &type) override {
		m_participantListType = type;
	};
	const ParticipantListType &getParticipantListType() const {
		return m_participantListType;
	};

	virtual void setJoiningMode(const JoiningMode &mode) override {
		m_joinMode = mode;
	};
	const JoiningMode &getJoiningMode() const {
		return m_joinMode;
	};

	virtual void setSecurityLevel(const SecurityLevel &level) override {
		m_securityLevel = level;
	};
	const SecurityLevel &getSecurityLevel() const {
		return m_securityLevel;
	};

	static ConferenceParams::SecurityLevel getSecurityLevelFromAttribute(const std::string &level);
	static std::string getSecurityLevelAttribute(const ConferenceParams::SecurityLevel &level);

private:
	void updateFromAccount(const std::shared_ptr<Account> &account); // Update Me and default factory from account.

	bool m_enableVideo = false;
	bool m_enableAudio = false;
	bool m_enableChat = false;
	bool m_localParticipantEnabled = true;
	bool m_allowOneParticipantConference = false;
	ParticipantListType m_participantListType = ParticipantListType::Open;
	JoiningMode m_joinMode = JoiningMode::DialIn;
	std::shared_ptr<Address> m_conferenceAddress = nullptr;
	std::shared_ptr<Address> m_factoryAddress = nullptr;
	SecurityLevel m_securityLevel = SecurityLevel::None;
	bool m_useDefaultFactoryAddress = true;
	std::string m_subject = "";
	mutable std::string m_utf8Subject = "";
	std::string m_description = "";
	mutable std::string m_utf8Description = "";
	std::shared_ptr<Address> m_me = nullptr;
	time_t m_startTime = (time_t)-1;
	time_t m_endTime = (time_t)-1;
	std::weak_ptr<Account> m_account;
	bool m_static = false;
	bool m_hidden = false;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_PARAMS_H_
