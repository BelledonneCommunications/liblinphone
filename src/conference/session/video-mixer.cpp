/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#include "streams.h"
#include "mixers.h"

#include "linphone/core.h"
#include "private.h"


LINPHONE_BEGIN_NAMESPACE

MS2VideoMixer::MS2VideoMixer(MixerSession & session) : StreamMixer(session), MS2VideoControl(session.getCore()){
	MSVideoConferenceParams params = {0};
	params.codec_mime_type = "VP8";
	params.min_switch_interval = 3000;
	mConference = ms_video_conference_new(mSession.getCCore()->factory, &params);
}

void MS2VideoMixer::connectEndpoint(Stream *vs, MSVideoEndpoint *endpoint, bool muted){
	ms_video_endpoint_set_user_data(endpoint, &vs->getGroup());
	ms_video_conference_add_member(mConference, endpoint);
}

void MS2VideoMixer::disconnectEndpoint(Stream *vs, MSVideoEndpoint *endpoint){
	ms_video_endpoint_set_user_data(endpoint, nullptr);
	ms_video_conference_remove_member(mConference, endpoint);
}

void MS2VideoMixer::setFocus(StreamsGroup *sg){
	MSVideoEndpoint *ep = nullptr;
	
	if (sg == nullptr){
		ep = mLocalEndpoint;
	}else{
		const bctbx_list_t *elem = ms_video_conference_get_members(mConference);
		for (; elem != nullptr; elem = elem->next){
			MSVideoEndpoint *ep_it = (MSVideoEndpoint *)elem->data;
			if (ms_video_endpoint_get_user_data(ep_it) == sg){
				ep = ep_it;
				break;
			}
		}
	}
	if (ep){
		ms_video_conference_set_focus(mConference, ep);
	}else{
		lError() << "MS2VideoMixer: cannot find endpoint requested for focus.";
	}
}

RtpProfile *MS2VideoMixer::sMakeDummyProfile(){
	RtpProfile *prof = rtp_profile_new("dummy video");
	LinphonePayloadType *pt = linphone_core_get_payload_type(mSession.getCCore(), "VP8", 90000, -1);
	PayloadType *ortp_pt = payload_type_clone(linphone_payload_type_get_ortp_pt(pt));
	rtp_profile_set_payload(prof, sVP8PayloadTypeNumber, ortp_pt);
	return prof;
}

void MS2VideoMixer::addLocalParticipant(){
	if (mLocalEndpoint) return;
	LinphoneCore *core = getSession().getCCore();
	VideoStream *st = video_stream_new(core->factory, 65002, 65003, FALSE);
	mLocalDummyProfile = sMakeDummyProfile();
	MSMediaStreamIO io;
	
	memset(&io, 0, sizeof(io));
	io.input.type = MSResourceCamera;
	io.input.camera = getVideoDevice();
	io.output.type = MSResourceDefault;
	
	video_stream_set_device_rotation(st, mSession.getCCore()->device_rotation);
	video_stream_set_freeze_on_error(st, !!lp_config_get_int(linphone_core_get_config(mSession.getCCore()), "video", "freeze_on_error", 1));
	video_stream_use_video_preset(st, lp_config_get_string(linphone_core_get_config(mSession.getCCore()), "video", "preset", nullptr));
	
	if (video_stream_start_from_io(st, mLocalDummyProfile, "127.0.0.1", 65002, "127.0.0.1", 65003, sVP8PayloadTypeNumber, &io) != 0){
		lError() << "Could not start video stream.";
		video_stream_stop(st);
		return;
	}
	
	mLocalParticipantStream = st;
	mLocalEndpoint = ms_video_endpoint_get_from_stream(st, FALSE);
	ms_message("Conference: adding video local endpoint");
	ms_video_conference_add_member(mConference, mLocalEndpoint);
}

void MS2VideoMixer::removeLocalParticipant(){
	if (mLocalEndpoint){
		ms_video_conference_remove_member(mConference, mLocalEndpoint);
		ms_video_endpoint_release_from_stream(mLocalEndpoint);
		mLocalEndpoint = nullptr;
		video_stream_stop(mLocalParticipantStream);
		mLocalParticipantStream = nullptr;
		rtp_profile_destroy(mLocalDummyProfile);
		mLocalDummyProfile = nullptr;
	}
}

void MS2VideoMixer::enableLocalParticipant(bool enabled){
	if (enabled) addLocalParticipant();
	else removeLocalParticipant();
}

VideoStream *MS2VideoMixer::getVideoStream()const{
	return mLocalParticipantStream;
}

MSWebCam *MS2VideoMixer::getVideoDevice()const{
	return mCore.getCCore()->video_conf.device;
}

void MS2VideoMixer::onSnapshotTaken(const std::string &filepath){
	// TODO
}

MS2VideoMixer::~MS2VideoMixer(){
	removeLocalParticipant();
	ms_video_conference_destroy(mConference);
}

LINPHONE_END_NAMESPACE
