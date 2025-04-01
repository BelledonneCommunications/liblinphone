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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "account/account.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "conference/conference-params.h"
#include "core/core.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceParams::ConferenceParams(const std::shared_ptr<Core> &core) : CoreAccessor(core) {
	if (core) {
		setHidden(
		    !!linphone_config_get_bool(linphone_core_get_config(core->getCCore()), "misc", "hide_conferences", 0));
	}
}

ConferenceParams::ConferenceParams(const ConferenceParams &other)
    : HybridObject<LinphoneConferenceParams, ConferenceParams>(other), CoreAccessor(nullptr) {
	try {
		setCore(other.getCore());
	} catch (const bad_weak_ptr &) {
	}
	mEnableAudio = other.mEnableAudio;
	mEnableVideo = other.mEnableVideo;
	mEnableChat = other.mEnableChat;
	mLocalParticipantEnabled = other.mLocalParticipantEnabled;
	mAllowOneParticipantConference = other.mAllowOneParticipantConference;
	mParticipantListType = other.mParticipantListType;
	mJoinMode = other.mJoinMode;
	mConferenceAddress = other.mConferenceAddress ? other.mConferenceAddress->clone()->toSharedPtr() : nullptr;
	mFactoryAddress = other.mFactoryAddress ? other.mFactoryAddress->clone()->toSharedPtr() : nullptr;
	mSecurityLevel = other.mSecurityLevel;
	mUseDefaultFactoryAddress = other.mUseDefaultFactoryAddress;
	mSubject = other.mSubject;
	mUtf8Subject = other.mUtf8Subject;
	mDescription = other.mDescription;
	mUtf8Description = other.mUtf8Description;
	mMe = other.mMe ? other.mMe->clone()->toSharedPtr() : nullptr;
	mStartTime = other.mStartTime;
	mEndTime = other.mEndTime;
	mEarlierJoiningTime = other.mEarlierJoiningTime;
	mExpiryTime = other.mExpiryTime;
	mGroup = other.mGroup;
	mAccount = other.mAccount;
	mHidden = other.mHidden;
	mChatParams = other.mChatParams ? other.mChatParams->clone()->toSharedPtr() : nullptr;
}

void ConferenceParams::setAudioVideoDefaults() {
	try {
		auto cCore = getCore()->getCCore();
		if (cCore) {
			enableAudio(true);
			const LinphoneVideoActivationPolicy *policy = linphone_core_get_video_activation_policy(cCore);
			enableVideo(linphone_video_activation_policy_get_automatically_initiate(policy));
			setParticipantListType(
			    static_cast<ParticipantListType>(linphone_core_get_conference_participant_list_type(cCore)));
		}
	} catch (const bad_weak_ptr &) {
	}
}

void ConferenceParams::setChatDefaults() {
	enableChat(true);
	mChatParams->setChatDefaults(getCore());
}

void ConferenceParams::enableChat(bool enable) {
	mEnableChat = enable;
	if (!mChatParams) {
		std::shared_ptr<Core> core;
		try {
			core = getCore();
		} catch (const bad_weak_ptr &) {
		}
		mChatParams = ChatParams::create(core);
	}
};

std::shared_ptr<Account> ConferenceParams::getAccount() const {
	return mAccount.lock();
}

void ConferenceParams::setAccount(const shared_ptr<Account> &a) {
	mAccount = a;
	updateFromAccount(a);
}

void ConferenceParams::updateFromAccount(
    const shared_ptr<Account> &account) { // Update Me and default factory from account.
	if (account) {
		auto accountParams = account->getAccountParams();
		if (accountParams) {
			if (mUseDefaultFactoryAddress) {
				const auto &conferenceFactory = (audioEnabled() || videoEnabled())
				                                    ? accountParams->getAudioVideoConferenceFactoryAddress()
				                                    : accountParams->getConferenceFactoryAddress();
				mFactoryAddress = conferenceFactory ? conferenceFactory->clone()->toSharedPtr() : nullptr;
				if (mFactoryAddress &&
				    (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalStartup)) {
					lInfo() << "Update conference parameters from account, factory: " << *mFactoryAddress;
				}
			}
		} else lInfo() << "Update conference parameters from account: no account parameters";
	} else lDebug() << "Update conference parameters from account: no account";
}

const std::string &ConferenceParams::getDescription() const {
	mDescription = Utils::utf8ToLocale(getUtf8Description());
	return mDescription;
}

void ConferenceParams::setDescription(const std::string &description) {
	setUtf8Description(Utils::localeToUtf8(description));
};

void ConferenceParams::setSubject(const std::string &subject) {
	setUtf8Subject(Utils::localeToUtf8(subject));
}

const std::string &ConferenceParams::getSubject() const {
	mSubject = Utils::utf8ToLocale(getUtf8Subject());
	return mSubject;
}

void ConferenceParams::setUtf8Subject(const std::string &subject) {
	mUtf8Subject = subject;
}

void ConferenceParams::setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress) {
	auto cCore = getCore()->getCCore();
	bool keepGruu = !!linphone_core_gruu_in_conference_address_enabled(cCore);
	if (keepGruu) {
		mConferenceAddress = Address::create(conferenceAddress->getUri());
	} else {
		mConferenceAddress = Address::create(conferenceAddress->getUriWithoutGruu());
	}
};

bool ConferenceParams::isGroup() const {
	return mGroup;
}

void ConferenceParams::setGroup(bool group) {
	mGroup = group;
	if (group && mChatParams) {
		mChatParams->setBackend(ChatParams::Backend::FlexisipChat);
	}
}

void ConferenceParams::setSecurityLevel(const SecurityLevel &level) {
	mSecurityLevel = level;
	if (mChatParams) {
		mChatParams->updateSecurityParams((mSecurityLevel == SecurityLevel::EndToEnd));
	}
};

void ConferenceParams::updateAccordingToCapabilities(AbstractChatRoom::CapabilitiesMask capabilities) {
	if (capabilities & AbstractChatRoom::CapabilitiesMask(AbstractChatRoom::Capabilities::Basic)) {
		mChatParams->setBackend(ChatParams::Backend::Basic);
	}
	if (capabilities & AbstractChatRoom::CapabilitiesMask(AbstractChatRoom::Capabilities::Conference)) {
		mChatParams->setBackend(ChatParams::Backend::FlexisipChat);
	}
	if (capabilities & AbstractChatRoom::CapabilitiesMask(AbstractChatRoom::Capabilities::Encrypted)) {
		setSecurityLevel(SecurityLevel::EndToEnd);
	} else {
		setSecurityLevel(SecurityLevel::None);
	}
	if (capabilities & AbstractChatRoom::CapabilitiesMask(AbstractChatRoom::Capabilities::Ephemeral)) {
		mChatParams->setEphemeralMode(AbstractChatRoom::EphemeralMode::AdminManaged);
	}
	setGroup((!(capabilities & AbstractChatRoom::CapabilitiesMask(AbstractChatRoom::Capabilities::OneToOne))) &&
	         (capabilities & AbstractChatRoom::CapabilitiesMask(AbstractChatRoom::Capabilities::Conference)));
	mChatParams->setRealTimeText(
	    (capabilities & AbstractChatRoom::CapabilitiesMask(AbstractChatRoom::Capabilities::RealTimeText)));
}

ConferenceParams::SecurityLevel ConferenceParams::getSecurityLevelFromAttribute(const string &level) {
	if (level.compare("point-to-point") == 0) {
		return ConferenceParams::SecurityLevel::PointToPoint;
	} else if (level.compare("end-to-end") == 0) {
		return ConferenceParams::SecurityLevel::EndToEnd;
	} else {
		return ConferenceParams::SecurityLevel::None;
	}
	return ConferenceParams::SecurityLevel::None;
}

string ConferenceParams::getSecurityLevelAttribute(const ConferenceParams::SecurityLevel &level) {
	switch (level) {
		case ConferenceParams::SecurityLevel::None:
			return "none";
		case ConferenceParams::SecurityLevel::PointToPoint:
			return "point-to-point";
		case ConferenceParams::SecurityLevel::EndToEnd:
			return "end-to-end";
	}
	return "none";
}

shared_ptr<ConferenceParams> ConferenceParams::fromCapabilities(AbstractChatRoom::CapabilitiesMask capabilities,
                                                                const std::shared_ptr<Core> &core) {
	auto params = ConferenceParams::create(core);
	params->enableAudio(false);
	params->enableVideo(false);
	params->enableChat(true);
	params->updateAccordingToCapabilities(capabilities);
	return params;
}

AbstractChatRoom::CapabilitiesMask ConferenceParams::toCapabilities(const std::shared_ptr<ConferenceParams> &params) {
	AbstractChatRoom::CapabilitiesMask mask;
	const auto chatBackend = params->getChatParams()->getBackend();
	if (chatBackend == ChatParams::Backend::Basic) {
		mask |= AbstractChatRoom::Capabilities::Basic;
		mask |= AbstractChatRoom::Capabilities::OneToOne;
	} else if (chatBackend == ChatParams::Backend::FlexisipChat) {
		mask |= AbstractChatRoom::Capabilities::Conference;
		if (!params->isGroup()) {
			mask |= AbstractChatRoom::Capabilities::OneToOne;
		}
		if (params->getChatParams()->ephemeralAllowed() &&
		    (params->getChatParams()->getEphemeralMode() == AbstractChatRoom::EphemeralMode::AdminManaged)) {
			mask |= AbstractChatRoom::Capabilities::Ephemeral;
		}
	}
	if (params->getChatParams()->isEncrypted() &&
	    params->getChatParams()->getEncryptionBackend() != ChatParams::EncryptionBackend::None) {
		mask |= AbstractChatRoom::Capabilities::Encrypted;
	}
	if (params->getChatParams()->isRealTimeText()) {
		mask |= AbstractChatRoom::Capabilities::RealTimeText;
	}
	return mask;
}

// Returns false	if there are any inconsistencies between parameters
bool ConferenceParams::isValid() const {
	auto ret = mChatParams->isValid();
	if (mGroup && mChatParams->getBackend() != ChatParams::Backend::FlexisipChat) {
		lError() << "FlexisipChat backend must be used when group is enabled";
		return false;
	}
	if (mUtf8Subject.empty() && mChatParams->getBackend() == ChatParams::Backend::FlexisipChat) {
		lError() << "You must set a non empty subject when using the FlexisipChat backend";
		return false;
	}
	return ret;
}

LINPHONE_END_NAMESPACE
