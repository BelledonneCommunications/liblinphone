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

#include "linphone/api/c-participant-device-cbs.h"
#include "conference/participant-device.h"
#include "c-wrapper/c-wrapper.h"

using namespace LinphonePrivate;
// =============================================================================

LinphoneParticipantDeviceCbs * linphone_participant_device_cbs_new (void) {
	return ParticipantDeviceCbs::createCObject();
}

LinphoneParticipantDeviceCbs * linphone_participant_device_cbs_ref (LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDeviceCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_participant_device_cbs_unref (LinphoneParticipantDeviceCbs *cbs) {
	ParticipantDeviceCbs::toCpp(cbs)->unref();
}

void * linphone_participant_device_cbs_get_user_data (const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getUserData();
}

void linphone_participant_device_cbs_set_user_data (LinphoneParticipantDeviceCbs *cbs, void *ud) {
	ParticipantDeviceCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneParticipantDeviceCbsIsSpeakingChangedCb linphone_participant_device_cbs_get_is_speaking_changed (const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getIsSpeakingChanged();
}

void linphone_participant_device_cbs_set_is_speaking_changed (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsIsSpeakingChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setIsSpeakingChanged(cb);
}

LinphoneParticipantDeviceCbsConferenceJoinedCb linphone_participant_device_cbs_get_conference_joined (const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getConferenceJoined();
}

void linphone_participant_device_cbs_set_conference_joined (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsConferenceJoinedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setConferenceJoined(cb);
}

LinphoneParticipantDeviceCbsConferenceLeftCb linphone_participant_device_cbs_get_conference_left (const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getConferenceLeft();
}

void linphone_participant_device_cbs_set_conference_left (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsConferenceLeftCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setConferenceLeft(cb);
}

LinphoneParticipantDeviceCbsAudioDirectionChangedCb linphone_participant_device_cbs_get_audio_direction_changed (const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getAudioDirectionChanged();
}

void linphone_participant_device_cbs_set_audio_direction_changed (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsAudioDirectionChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setAudioDirectionChanged(cb);
}

LinphoneParticipantDeviceCbsVideoDirectionChangedCb linphone_participant_device_cbs_get_video_direction_changed (const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getVideoDirectionChanged();
}

void linphone_participant_device_cbs_set_video_direction_changed (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsVideoDirectionChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setVideoDirectionChanged(cb);
}

LinphoneParticipantDeviceCbsTextDirectionChangedCb linphone_participant_device_cbs_get_text_direction_changed (const LinphoneParticipantDeviceCbs *cbs) {
	return ParticipantDeviceCbs::toCpp(cbs)->getTextDirectionChanged();
}

void linphone_participant_device_cbs_set_text_direction_changed (LinphoneParticipantDeviceCbs *cbs, LinphoneParticipantDeviceCbsTextDirectionChangedCb cb) {
	ParticipantDeviceCbs::toCpp(cbs)->setTextDirectionChanged(cb);
}
