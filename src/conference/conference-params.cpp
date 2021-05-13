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

#include "core/core.h"
#include "conference/conference-params.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceParams::ConferenceParams(const LinphoneCore *core) {
	if(core) {
		const LinphoneVideoPolicy *policy = linphone_core_get_video_policy(core);
		if(policy->automatically_initiate) m_enableVideo = true;
	}
}

ConferenceParams::ConferenceParams(const ConferenceParams& params) : HybridObject(params), ConferenceParamsInterface()  {
	m_enableVideo = params.m_enableVideo;
	m_enableAudio = params.m_enableAudio;
	m_enableChat = params.m_enableChat;
	m_localParticipantEnabled = params.m_localParticipantEnabled;
	m_allowOneParticipantConference = params.m_allowOneParticipantConference;
	m_conferenceAddress = params.m_conferenceAddress;
	m_layout = params.m_layout;
	m_factoryAddress = params.m_factoryAddress;
	m_subject = params.m_subject;
	m_me = params.m_me;
}

LINPHONE_END_NAMESPACE
