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

ConferenceParams::ConferenceParams(const LinphoneCore *core) {
	if (core) {
		const LinphoneVideoPolicy *policy = linphone_core_get_video_policy(core);
		enableVideo(policy->automatically_initiate);
		setParticipantListType(
		    static_cast<ParticipantListType>(linphone_core_get_conference_participant_list_type(core)));
		const auto defaultAccount = linphone_core_get_default_account(core);
		if (defaultAccount) {
			updateFromAccount(Account::toCpp(defaultAccount)->getSharedFromThis());
		}
	}
}

ConferenceParams::ConferenceParams(const ConferenceParams &params) : HybridObject(params), ConferenceParamsInterface() {
	m_enableVideo = params.m_enableVideo;
	m_enableAudio = params.m_enableAudio;
	m_enableChat = params.m_enableChat;
	m_localParticipantEnabled = params.m_localParticipantEnabled;
	m_allowOneParticipantConference = params.m_allowOneParticipantConference;
	m_participantListType = params.m_participantListType;
	m_joinMode = params.m_joinMode;
	m_conferenceAddress = params.m_conferenceAddress;
	m_factoryAddress = params.m_factoryAddress;
	m_subject = params.m_subject;
	m_description = params.m_description;
	m_me = params.m_me;
	m_startTime = params.m_startTime;
	m_endTime = params.m_endTime;
	m_account = params.m_account;
	m_static = params.m_static;
}

void ConferenceParams::setAccount(const shared_ptr<Account> &a) {
	m_account = a;
	updateFromAccount(m_account);
}

void ConferenceParams::updateFromAccount(
    const shared_ptr<Account> &account) { // Update Me and default factory from account.
	if (account) {
		auto accountParams = account->getAccountParams();
		if (accountParams) {
			auto identity = accountParams->getIdentityAddress();
			if (identity) {
				setMe(identity);
			} else {
				setMe(nullptr);
			}
			if (m_useDefaultFactoryAddress) {
				auto core = account->getCore();
				m_factoryAddress = accountParams->getAudioVideoConferenceFactoryAddress();
				if (m_factoryAddress && (linphone_core_get_global_state(core) != LinphoneGlobalStartup)) {
					lInfo() << "Update conference parameters from account, factory: " << m_factoryAddress->toString();
				}
			}
		} else lInfo() << "Update conference parameters from account: no account parameters";
	} else lInfo() << "Update conference parameters from account: no account";
}

void ConferenceParams::setUtf8Description(const std::string &description) {
	m_description = Utils::utf8ToLocale(description);
};

const std::string ConferenceParams::getUtf8Description() const {
	return Utils::localeToUtf8(m_description);
};

void ConferenceParams::setUtf8Subject(const std::string &subject) {
	m_subject = Utils::utf8ToLocale(subject);
};

const std::string ConferenceParams::getUtf8Subject() const {
	return Utils::localeToUtf8(m_subject);
};

void ConferenceParams::setConferenceAddress(const std::shared_ptr<Address> conferenceAddress) {
	m_conferenceAddress = Address::create(conferenceAddress->getUri());
};
LINPHONE_END_NAMESPACE
