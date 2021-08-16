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

#include "c-wrapper/internal/c-tools.h"
#include "mediastreamer2/msitc.h"

LINPHONE_BEGIN_NAMESPACE

MS2VideoMixer::MS2VideoMixer(MixerSession & session) : StreamMixer(session), MS2VideoControl(session.getCore()){
	MSVideoConferenceParams paramsAlltoAll = {0};
	paramsAlltoAll.codec_mime_type = "VP8";
	paramsAlltoAll.all_to_all = 1;
	paramsAlltoAll.min_switch_interval = 3000;
	mConferenceAllToAll = ms_video_conference_new(mSession.getCCore()->factory, &paramsAlltoAll);

	MSVideoConferenceParams paramsOnetoAll = {0};
	paramsOnetoAll.codec_mime_type = "VP8";
	paramsOnetoAll.all_to_all = 0;
	paramsOnetoAll.min_switch_interval = 3000;
	mConferenceOnetoAll = ms_video_conference_new(mSession.getCCore()->factory, &paramsOnetoAll);
}

void MS2VideoMixer::connectEndpoint(Stream *vs, MSVideoEndpoint *endpoint, bool activeSpeaker){
	ms_video_endpoint_set_user_data(endpoint, &vs->getGroup());
	if (activeSpeaker) {
		ms_video_conference_add_member(mConferenceOnetoAll, endpoint);
	} else {
		ms_video_conference_add_member(mConferenceAllToAll, endpoint);
	}
}

void MS2VideoMixer::disconnectEndpoint(Stream *vs, MSVideoEndpoint *endpoint){
	ms_video_endpoint_set_user_data(endpoint, nullptr);
	// Try to remove endpoint from both MSConference
	ms_video_conference_remove_member(mConferenceOnetoAll, endpoint);
	ms_video_conference_remove_member(mConferenceAllToAll, endpoint);
}

void MS2VideoMixer::setFocus(StreamsGroup *sg){
	// used by mConferenceOnetoAll
	MSVideoEndpoint *ep = nullptr;
	
	if (sg == nullptr){
		ep = mMainLocalEndpoint;
	}else{
		const bctbx_list_t *elem = ms_video_conference_get_members(mConferenceOnetoAll);
		for (; elem != nullptr; elem = elem->next){
			MSVideoEndpoint *ep_it = (MSVideoEndpoint *)elem->data;
			if (ms_video_endpoint_get_user_data(ep_it) == sg){
				ep = ep_it;
				break;
			}
		}
	}
	if (ep){
		ms_video_conference_set_focus(mConferenceOnetoAll, ep);
	}else{
		MSVideoEndpoint *video_placeholder_ep = ms_video_conference_get_video_placeholder_member(mConferenceOnetoAll);
		if (video_placeholder_ep) {
			ms_video_conference_set_focus(mConferenceOnetoAll, video_placeholder_ep);
		} else {
			lError() << "MS2VideoMixer: cannot find endpoint requested for focus.";
		}
	}
}

RtpProfile *MS2VideoMixer::sMakeDummyProfile(){
	RtpProfile *prof = rtp_profile_new("dummy video");
	LinphonePayloadType *pt = linphone_core_get_payload_type(mSession.getCCore(), "VP8", 90000, -1);
	PayloadType *ortp_pt = payload_type_clone(linphone_payload_type_get_ortp_pt(pt));
	rtp_profile_set_payload(prof, sVP8PayloadTypeNumber, ortp_pt);
	return prof;
}

int MS2VideoMixer::getOutputBandwidth(){
	/* FIXME: it should take into account the remote bandwidth constraint (b=AS:) of other participants. */
	return linphone_core_get_upload_bandwidth(mSession.getCCore());
}

void MS2VideoMixer::addLocalParticipant(){
	if (mMainLocalEndpoint) return;
	LinphoneCore *core = getSession().getCCore();
	VideoStream *mainSt = video_stream_new(core->factory, 65002, 65003, FALSE);
	mLocalDummyProfile = sMakeDummyProfile();
	MSMediaStreamIO io;
	int outputBandwidth =  getOutputBandwidth() * 1000;
	PayloadType *pt;
	VideoStream *st;
	MSMediaStreamIO io2;
	
	memset(&io, 0, sizeof(io));
	io.input.type = MSResourceCamera;
	io.input.camera = getVideoDevice();
	io.output.type = MSResourceDefault;
	
	video_stream_set_native_preview_window_id(mainSt, linphone_core_get_native_preview_window_id(core));// Ensure to pass Window ID options (like not having to create a window)
	video_stream_set_native_window_id(mainSt, linphone_core_get_native_video_window_id(core));
	if (linphone_core_get_video_display_filter(core)) {
		video_stream_set_display_filter_name(mainSt, linphone_core_get_video_display_filter(core));	// Use the filter defined in configuration
	} /*else keep default one*/
	video_stream_use_preview_video_window(mainSt, linphone_core_video_preview_enabled(core));	// Allow preview on conference
	
	video_stream_set_device_rotation(mainSt, mSession.getCCore()->device_rotation);
	video_stream_set_freeze_on_error(mainSt, !!linphone_config_get_int(linphone_core_get_config(mSession.getCCore()), "video", "freeze_on_error", 1));
	video_stream_use_video_preset(mainSt, linphone_config_get_string(linphone_core_get_config(mSession.getCCore()), "video", "preset", nullptr));
	media_stream_set_max_network_bitrate(&mainSt->ms, outputBandwidth);
	pt = rtp_profile_get_payload(mLocalDummyProfile, sVP8PayloadTypeNumber);
	pt->normal_bitrate =  outputBandwidth; /* Is it really needed ?*/
	
	video_stream_set_fps(mainSt, linphone_core_get_preferred_framerate(mSession.getCCore()));
	const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(mSession.getCCore());
	if (vdef) {
		MSVideoSize vsize;
		vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
		vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
		video_stream_set_sent_video_size(mainSt, vsize);
	}
	
	if (video_stream_start_from_io(mainSt, mLocalDummyProfile, "127.0.0.1", 65002, "127.0.0.1", 65003, sVP8PayloadTypeNumber, &io) != 0){
		lError() << "Could not start video stream.";
		video_stream_stop(mainSt);
		return;
	}

	if (!mLocalParticipantLabel.empty()) {
		video_stream_set_label(mainSt, L_STRING_TO_C(mLocalParticipantLabel));
	}
	
	switch (mSession.getConferenceLayout()) {
		case ConferenceParams::Layout::None:
			lInfo() << "[one to all]:  add video endpoint";
			mLocalParticipantStream = mainSt;
			mMainLocalEndpoint = ms_video_endpoint_get_from_stream(mainSt, FALSE);
			ms_video_conference_add_member(mConferenceOnetoAll, mMainLocalEndpoint);
			break;
		case ConferenceParams::Layout::Grid:
			if (!mainSt->label) {
				lError() << "[all to all]: Can not add video endpoint with empty label";
			} else {
				lInfo() << "[all to all]:  add video endpoint with label " << mainSt->label;
				video_stream_enable_router(mainSt, true);
				mLocalParticipantStream = mainSt;
				mMainLocalEndpoint = ms_video_endpoint_get_from_stream(mainSt, FALSE);
				ms_video_conference_add_member(mConferenceAllToAll, mMainLocalEndpoint);
			}
			break;
		case ConferenceParams::Layout::ActiveSpeaker:
			lInfo() << "[mix to all]:  add video endpoint";
			mLocalParticipantStream = mainSt;
			mMainLocalEndpoint = ms_video_endpoint_get_from_stream(mainSt, FALSE);
			ms_video_conference_add_member(mConferenceOnetoAll, mMainLocalEndpoint);

			st = video_stream_new(core->factory, 65000, 65001, FALSE);
			// Stores the pointer to the newly created video stream to stop it upon local participant removal
			mLocalParticipantItcStream = st;
			memset(&io2, 0, sizeof(io2));
			io2.input.type = MSResourceItc;
			io2.output.type = MSResourceDefault;

			// todo set window id

			media_stream_set_max_network_bitrate(&st->ms, outputBandwidth);
			video_stream_set_fps(st, linphone_core_get_preferred_framerate(mSession.getCCore()));
			if (vdef) {
				// todo set size
				MSVideoSize vsize;
				vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
				vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
				video_stream_set_sent_video_size(st, vsize);
			}

			if (video_stream_start_from_io_and_sink(st, mLocalDummyProfile, "127.0.0.1", 65000, "127.0.0.1", 65001, sVP8PayloadTypeNumber, &io2, mainSt->itcsink) != 0){
				lError() << "Could not start video stream.";
				video_stream_stop(st);
				return;
			}
			link_video_stream_with_itc_sink(mainSt);
			ms_filter_call_method(mainSt->itcsink,MS_ITC_SINK_CONNECT,st->source);

			if (!mLocalParticipantLabel.empty()) {
				video_stream_set_label(st, L_STRING_TO_C(mLocalParticipantLabel));
			}

			if (!st->label) {
				lError() << "[mix to all]: Can not add video endpoint with empty label";
			} else {
				lInfo() << "[mix to all]:  add video endpoint with label" << st->label;
				video_stream_enable_router(st, true);
				mLocalEndpoint = ms_video_endpoint_get_from_stream(st, FALSE);
				ms_video_conference_add_member(mConferenceAllToAll, mLocalEndpoint);
			}
			break;
	}
	
}

void MS2VideoMixer::removeLocalParticipant(){
	if (mMainLocalEndpoint){
		ms_message("Conference: removing video local endpoint");
		ms_video_conference_remove_member(mConferenceAllToAll, mMainLocalEndpoint);
		ms_video_conference_remove_member(mConferenceOnetoAll, mMainLocalEndpoint);
		ms_video_endpoint_release_from_stream(mMainLocalEndpoint);
		mMainLocalEndpoint = nullptr;
		if (mLocalEndpoint) {
			ms_video_conference_remove_member(mConferenceAllToAll, mLocalEndpoint);
			ms_video_conference_remove_member(mConferenceOnetoAll, mLocalEndpoint);
			ms_video_endpoint_release_from_stream(mLocalEndpoint);
			mLocalEndpoint = nullptr;
		}

		if (mLocalParticipantItcStream) {
			video_stream_stop(mLocalParticipantItcStream);
			mLocalParticipantItcStream = nullptr;
		}
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
	ms_video_conference_destroy(mConferenceAllToAll);
	ms_video_conference_destroy(mConferenceOnetoAll);
}

void MS2VideoMixer::setLocalParticipantLabel(const std::string & label) {
	mLocalParticipantLabel = label;
}

std::string MS2VideoMixer::getLocalParticipantLabel() const {
	return mLocalParticipantLabel;
}

LINPHONE_END_NAMESPACE
