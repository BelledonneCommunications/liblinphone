/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "core/core.h"
#include "conference/conference-params.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceParams::ConferenceParams(const LinphoneCore *core) {
	if(core) {
		const LinphoneVideoPolicy *policy = linphone_core_get_video_policy(core);
		enableVideo(policy->automatically_initiate);
		setLayout((ConferenceLayout)linphone_core_get_default_conference_layout(core));
		setParticipantListType(static_cast<ParticipantListType>(linphone_core_get_conference_participant_list_type(core)));
		updateFromAccount(linphone_core_get_default_account(core));
	}
}

ConferenceParams::ConferenceParams(const ConferenceParams& params) : HybridObject(params), ConferenceParamsInterface()  {
	m_enableVideo = params.m_enableVideo;
	m_enableAudio = params.m_enableAudio;
	m_enableChat = params.m_enableChat;
	m_localParticipantEnabled = params.m_localParticipantEnabled;
	m_allowOneParticipantConference = params.m_allowOneParticipantConference;
	m_participantListType = params.m_participantListType;
	m_conferenceAddress = params.m_conferenceAddress;
	m_factoryAddress = params.m_factoryAddress;
	m_subject = params.m_subject;
	m_description = params.m_description;
	m_layout = params.m_layout;
	m_me = params.m_me;
	m_startTime = params.m_startTime;
	m_endTime = params.m_endTime;
	m_account = params.m_account;
	m_static = params.m_static;
}

void ConferenceParams::setAccount(LinphoneAccount * a) { 
	m_account = a;
	updateFromAccount(m_account);
}

void ConferenceParams::updateFromAccount(LinphoneAccount * account) {// Update Me and default factory from account.
	if(account){
		auto account_params = linphone_account_get_params( account);
		if( account_params){
			auto identity = linphone_account_params_get_identity_address(account_params);
			setMe(identity ? IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(identity)) : IdentityAddress());
			if(m_useDefaultFactoryAddress) {
				auto core = L_GET_CPP_PTR_FROM_C_OBJECT(linphone_account_get_core(account));
				auto factory_addr = Core::getAudioVideoConferenceFactoryAddress(core->getSharedFromThis(), account);
				m_factoryAddress = (factory_addr ? *L_GET_CPP_PTR_FROM_C_OBJECT(factory_addr) : Address());
				ms_message("Update conference parameters from account, factory:%s", factory_addr ? linphone_address_as_string(factory_addr) : "NULL");
				if(factory_addr)
					linphone_address_unref(factory_addr);
			}
		}else
			ms_message("Update conference parameters from account: no account parameters");
	}else
		ms_message("Update conference parameters from account: no account");
}
LINPHONE_END_NAMESPACE
