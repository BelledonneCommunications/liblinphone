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

#include <bctoolbox/defs.h>

#include "mediastreamer2/msconference.h"
#include "mediastreamer2/msitc.h"

#include "c-wrapper/internal/c-tools.h"
#include "linphone/core.h"
#include "mixers.h"
#include "private.h"
#include "streams.h"

LINPHONE_BEGIN_NAMESPACE

MS2VideoMixer::MS2VideoMixer(MixerSession &session) : StreamMixer(session), MS2VideoControl(session.getCore()) {
	mConferenceParams.codec_mime_type = "VP8";
	mConferenceParams.min_switch_interval = 3000;
	mConferenceParams.security_level = StreamMixer::securityLevelToMsSecurityLevel(session.getSecurityLevel());
	LinphoneConfig *config = linphone_core_get_config(mSession.getCCore());
	mConferenceParams.mode = static_cast<MSConferenceMode>(
	    linphone_config_get_int(config, "video", "conference_mode", MSConferenceModeRouterPayload));
	mConferenceMix = ms_video_conference_new(mSession.getCCore()->factory, &mConferenceParams);
	mConferenceThumbnail = ms_video_conference_new(mSession.getCCore()->factory, &mConferenceParams);
}

void MS2VideoMixer::connectEndpoint(Stream *vs, MSVideoEndpoint *endpoint, bool thumbnail) {
	ms_video_endpoint_set_user_data(endpoint, &vs->getGroup());

	if (thumbnail) {
		lInfo() << *this << "Adding endpoint to thumbnail mixer.";
		ms_video_conference_add_member(mConferenceThumbnail, endpoint);
	} else {
		lInfo() << *this << "Adding endpoint to main mixer.";
		ms_video_conference_add_member(mConferenceMix, endpoint);
	}
}

void MS2VideoMixer::disconnectEndpoint(BCTBX_UNUSED(Stream *vs), MSVideoEndpoint *endpoint) {
	ms_video_endpoint_set_user_data(endpoint, nullptr);
	// Try to remove endpoint from both MSConference
	ms_video_conference_remove_member(mConferenceMix, endpoint);
	ms_video_conference_remove_member(mConferenceThumbnail, endpoint);
}

void MS2VideoMixer::setFocus(StreamsGroup *sg) {
	// used by mConferenceMix
	MSVideoEndpoint *ep = nullptr;

	lInfo() << *this << ": video focus requested for " << *sg;
	if (sg == nullptr) {
		ep = mMainLocalEndpoint;
	} else {
		const bctbx_list_t *elem = ms_video_conference_get_members(mConferenceMix);
		for (; elem != nullptr; elem = elem->next) {
			MSVideoEndpoint *ep_it = (MSVideoEndpoint *)elem->data;
			if (ms_video_endpoint_get_user_data(ep_it) == sg) {
				ep = ep_it;
				break;
			}
		}
	}
	if (ep) {
		ms_video_conference_set_focus(mConferenceMix, ep);
	} else {
		if (ms_video_conference_get_size(mConferenceMix) >= 2) {
			/* else this participant has no video, so set focus on a "no webcam" placeholder.
			 * However, if there is one or two participants, don't do this and let the ms2 mixer cross the streams.
			 */
			lInfo() << *this << "Showing video placeholder, participant has no video.";
			//			MSVideoEndpoint *video_placeholder_ep =
			// ms_video_conference_get_video_placeholder_member(mConferenceMix); 			if (video_placeholder_ep) {
			//				ms_video_conference_set_focus(mConferenceMix, video_placeholder_ep);
			//			}
		} else {
			lInfo() << *this << "Not using video placeholder, participant count <= 2.";
		}
	}
}

RtpProfile *MS2VideoMixer::sMakeDummyProfile() {
	RtpProfile *prof = rtp_profile_new("dummy video");
	LinphonePayloadType *pt = linphone_core_get_payload_type(mSession.getCCore(), "VP8", 90000, -1);
	OrtpPayloadType *ortp_pt = payload_type_clone(linphone_payload_type_get_ortp_pt(pt));
	rtp_profile_set_payload(prof, sVP8PayloadTypeNumber, ortp_pt);
	linphone_payload_type_unref(pt);
	return prof;
}

int MS2VideoMixer::getOutputBandwidth() {
	/* FIXME: it should take into account the remote bandwidth constraint (b=AS:) of other participants. */
	return linphone_core_get_upload_bandwidth(mSession.getCCore());
}

void MS2VideoMixer::addLocalParticipant() {
	if (mMainLocalEndpoint) return;
	createLocalMember(FALSE);
	createLocalMember(TRUE);
}

void MS2VideoMixer::createLocalMember(bool isThumbnail) {
	LinphoneCore *core = getSession().getCCore();
	VideoStream *vs = video_stream_new(core->factory, isThumbnail ? 65000 : 65002, isThumbnail ? 65001 : 65003, FALSE);
	video_stream_set_fallback_to_dummy_codec(
	    vs, linphone_config_get_bool(linphone_core_get_config(core), "video", "fallback_to_dummy_codec", TRUE));

	if (!mLocalDummyProfile) mLocalDummyProfile = sMakeDummyProfile();
	MSMediaStreamIO io;
	int outputBandwidth = getOutputBandwidth() * 1000;
	OrtpPayloadType *pt;

	memset(&io, 0, sizeof(io));

	video_stream_set_content(vs, isThumbnail ? MSVideoContentThumbnail : MSVideoContentDefault);
	const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(mSession.getCCore());
	pt = rtp_profile_get_payload(mLocalDummyProfile, sVP8PayloadTypeNumber);
	pt->normal_bitrate = outputBandwidth; /* Is it really needed ?*/
	video_stream_set_fps(vs, linphone_core_get_preferred_framerate(mSession.getCCore()));
	if (isThumbnail) {
		io.input.type = MSResourceItc;
		io.output.type = MSResourceDefault;
		io.input.itc = mLocalParticipantStream->itcsink;

		media_stream_set_max_network_bitrate(&vs->ms, 80000);
		if (vdef) {
			MSVideoSize vsize = {160, 120};
			video_stream_set_sent_video_size(vs, vsize);
		}

		if (video_stream_start_from_io(vs, mLocalDummyProfile, "127.0.0.1", 65000, "127.0.0.1", 65001,
		                               sVP8PayloadTypeNumber, &io) != 0) {
			lError() << "Could not start video stream.";
			video_stream_stop(vs);
			return;
		}

		media_stream_set_direction(&vs->ms, MediaStreamSendOnly);
	} else {
		io.input.type = MSResourceCamera;
		io.input.camera = getVideoDevice();
		io.output.type = MSResourceDefault;

		video_stream_set_native_preview_window_id(
		    vs, linphone_core_get_native_preview_window_id(
		            core)); // Ensure to pass Window ID options (like not having to create a window)
		video_stream_set_native_window_id(vs, linphone_core_get_native_video_window_id(core));
		if (linphone_core_get_video_display_filter(core)) {
			video_stream_set_display_filter_name(
			    vs, linphone_core_get_video_display_filter(core)); // Use the filter defined in configuration
		}                                                          /*else keep default one*/
		video_stream_use_preview_video_window(vs,
		                                      linphone_core_video_preview_enabled(core)); // Allow preview on conference
		video_stream_set_device_rotation(vs, mSession.getCCore()->device_rotation);
		video_stream_set_freeze_on_error(vs, !!linphone_config_get_int(linphone_core_get_config(mSession.getCCore()),
		                                                               "video", "freeze_on_error", 1));
		video_stream_use_video_preset(
		    vs, linphone_config_get_string(linphone_core_get_config(mSession.getCCore()), "video", "preset", nullptr));

		media_stream_set_max_network_bitrate(&vs->ms, outputBandwidth);
		if (vdef) {
			MSVideoSize vsize;
			vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
			vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
			video_stream_set_sent_video_size(vs, vsize);
		}
		if (video_stream_start_from_io(vs, mLocalDummyProfile, "127.0.0.1", 65002, "127.0.0.1", 65003,
		                               sVP8PayloadTypeNumber, &io) != 0) {
			lError() << "Could not start video stream.";
			video_stream_stop(vs);
			return;
		}
		if (linphone_core_get_default_conference_layout(mSession.getCCore()) == LinphoneConferenceLayoutGrid) {
			media_stream_set_direction(&vs->ms, MediaStreamSendOnly);
		}
		link_video_stream_with_itc_sink(vs);
	}

	if (!mLocalParticipantLabel.empty()) {
		video_stream_set_label(vs, L_STRING_TO_C(mLocalParticipantLabel));
	}

	lInfo() << "[mix to all]: add local video endpoint label " << mLocalParticipantLabel << " is thumbnail "
	        << isThumbnail;

	if (isThumbnail) {
		mLocalParticipantItcStream = vs;
		MSConferenceMode mode = ms_video_conference_get_params(mConferenceThumbnail)->mode;
		mLocalEndpoint = ms_video_endpoint_get_from_stream(mLocalParticipantItcStream, FALSE, mode);
		ms_video_conference_add_member(mConferenceThumbnail, mLocalEndpoint);
	} else {
		mLocalParticipantStream = vs;
		MSConferenceMode mode = ms_video_conference_get_params(mConferenceMix)->mode;
		mMainLocalEndpoint = ms_video_endpoint_get_from_stream(mLocalParticipantStream, FALSE, mode);
		ms_video_conference_add_member(mConferenceMix, mMainLocalEndpoint);
	}
}

void MS2VideoMixer::removeLocalParticipant() {
	if (mMainLocalEndpoint) {
		ms_message("Conference: removing video local endpoint");
		ms_video_conference_remove_member(mConferenceMix, mMainLocalEndpoint);
		ms_video_endpoint_release_from_stream(mMainLocalEndpoint);
		mMainLocalEndpoint = nullptr;
		if (mLocalEndpoint) {
			ms_video_conference_remove_member(mConferenceThumbnail, mLocalEndpoint);
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

void MS2VideoMixer::enableLocalParticipant(bool enabled) {
	if (enabled) addLocalParticipant();
	else removeLocalParticipant();
}

VideoStream *MS2VideoMixer::getVideoStream() const {
	return mLocalParticipantStream;
}

MSWebCam *MS2VideoMixer::getVideoDevice() const {
	return mCore.getCCore()->video_conf.device;
}

void MS2VideoMixer::onSnapshotTaken(BCTBX_UNUSED(const std::string &filepath)) {
	// TODO
}

MS2VideoMixer::~MS2VideoMixer() {
	removeLocalParticipant();
	ms_video_conference_destroy(mConferenceMix);
	ms_video_conference_destroy(mConferenceThumbnail);
}

void MS2VideoMixer::setLocalParticipantLabel(const std::string &label) {
	mLocalParticipantLabel = label;
}

std::string MS2VideoMixer::getLocalParticipantLabel() const {
	return mLocalParticipantLabel;
}

const MSVideoConferenceParams &MS2VideoMixer::getConferenceParams() const {
	return mConferenceParams;
}

LINPHONE_END_NAMESPACE
