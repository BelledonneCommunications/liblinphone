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
		const LinphoneVideoActivationPolicy *policy = linphone_core_get_video_activation_policy(core);
		enableVideo(linphone_video_activation_policy_get_automatically_initiate(policy));
		setParticipantListType(
		    static_cast<ParticipantListType>(linphone_core_get_conference_participant_list_type(core)));
		setAccount(L_GET_CPP_PTR_FROM_C_OBJECT(core)->getDefaultAccount());
	}
}

void ConferenceParams::setAccount(const shared_ptr<Account> &a) {
	m_account = a;
	updateFromAccount(a);
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
				if (m_factoryAddress &&
				    (linphone_core_get_global_state(L_GET_C_BACK_PTR(core)) != LinphoneGlobalStartup)) {
					lInfo() << "Update conference parameters from account, factory: " << *m_factoryAddress;
				}
			}
		} else lInfo() << "Update conference parameters from account: no account parameters";
	} else lInfo() << "Update conference parameters from account: no account";
}

void ConferenceParams::setUtf8Description(const std::string &description) {
	m_description = Utils::utf8ToLocale(description);
};

const std::string &ConferenceParams::getUtf8Description() const {
	m_utf8Description = Utils::localeToUtf8(m_description);
	return m_utf8Description;
};

void ConferenceParams::setUtf8Subject(const std::string &subject) {
	m_subject = Utils::utf8ToLocale(subject);
};

const std::string &ConferenceParams::getUtf8Subject() const {
	m_utf8Subject = Utils::localeToUtf8(m_subject);
	return m_utf8Subject;
};

void ConferenceParams::setConferenceAddress(const std::shared_ptr<Address> conferenceAddress) {
	m_conferenceAddress = Address::create(conferenceAddress->getUri());
};

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

LINPHONE_END_NAMESPACE
