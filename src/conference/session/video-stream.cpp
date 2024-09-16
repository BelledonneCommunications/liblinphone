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

#include "bctoolbox/defs.h"

#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msitc.h"
#include "mediastreamer2/msjpegwriter.h"
#include "mediastreamer2/msogl.h"

#ifdef HAVE_LIME_X3DH
#include "bzrtp/bzrtp.h"
#endif

#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "conference/client-conference.h"
#include "conference/conference.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant.h"
#include "conference/server-conference.h"
#include "core/core.h"
#include "linphone/core.h"
#include "media-session-p.h"
#include "media-session.h"
#include "mixers.h"
#include "ms2-streams.h"

using namespace ::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * MS2VideoStream implemenation
 */

MS2VideoStream::MS2VideoStream(StreamsGroup &sg, const OfferAnswerContext &params)
    : MS2Stream(sg, params), MS2VideoControl(sg.getCore()) {
	string bindIp = getBindIp();

	const auto &localDesc = params.getLocalStreamDescription();

	if ((localDesc.getRtpPort() > 0) && (localDesc.getRtcpPort() > 0)) {
		// port already set in SDP
		mPortConfig.rtpPort = localDesc.getRtpPort();
		mPortConfig.rtcpPort = localDesc.getRtcpPort();
	}
	sg.installSharedService<ScreenSharingService>();

	mStream = video_stream_new2(getCCore()->factory, L_STRING_TO_C(bindIp), mPortConfig.rtpPort, mPortConfig.rtcpPort);

	initializeSessions(&mStream->ms);
}

void MS2VideoStream::configure(const OfferAnswerContext &params) {
	if (mStream) {
		const auto localIsInConference = getMediaSessionPrivate().getParams()->getPrivate()->getInConference();
		const auto &localDesc = params.getLocalStreamDescription();
		const auto &remoteDesc = params.getRemoteStreamDescription();

		// Content
		const auto &content = localIsInConference ? remoteDesc.getContent() : localDesc.getContent();
		MSVideoContent mscontent = MSVideoContentDefault;
		if (content == "thumbnail") {
			mscontent = MSVideoContentThumbnail;
		} else if (content == "speaker") {
			mscontent = MSVideoContentSpeaker;
		}
		if (getVideoMixer() != nullptr && mscontent == MSVideoContentDefault &&
		    media_stream_get_direction(&mStream->ms) == MediaStreamSendRecv) {
			/* When handling a conference call, in absence of content attribute and if stream is sendrecv,
			 * assume the target content is speaker (active speaker stream)*/
			lInfo() << "No content given, assuming active speaker mode.";
			mscontent = MSVideoContentSpeaker;
		}
		video_stream_set_content(mStream, mscontent);

		// Label
		const auto &label = localIsInConference ? localDesc.getLabel() : remoteDesc.getLabel();
		if (!label.empty()) {
			video_stream_set_label(mStream, label.c_str());
		}
	}
}

void MS2VideoStream::sVideoStreamEventCb(void *userData,
                                         const MSFilter *f,
                                         const unsigned int eventId,
                                         const void *args) {
	MS2VideoStream *zis = static_cast<MS2VideoStream *>(userData);
	zis->videoStreamEventCb(f, eventId, args);
}

void MS2VideoStream::videoStreamEventCb(BCTBX_UNUSED(const MSFilter *f), const unsigned int eventId, const void *args) {

	switch (eventId) {
		case MS_VIDEO_DECODER_DECODING_ERRORS:
			lWarning() << "MS_VIDEO_DECODER_DECODING_ERRORS";
			if (!media_stream_avpf_enabled(&mStream->ms)) {
				if (mStream && video_stream_is_decoding_error_to_be_reported(mStream, 5000)) {
					video_stream_decoding_error_reported(mStream);
					getMediaSession().sendVfuRequest();
				}
			} else {
				/* Decoders are not expected to throw MS_VIDEO_DECODER_DECODING_ERRORS if AVPF is enabled */
			}
			break;
		case MS_VIDEO_DECODER_RECOVERED_FROM_ERRORS:
			lInfo() << "MS_VIDEO_DECODER_RECOVERED_FROM_ERRORS";
			if (mStream) video_stream_decoding_error_recovered(mStream);
			break;
		case MS_VIDEO_DECODER_FIRST_IMAGE_DECODED:
			lInfo() << "First video frame decoded successfully";
			getMediaSession().notifyFirstVideoFrameDecoded();
			break;
		case MS_VIDEO_DECODER_SEND_PLI:
		case MS_VIDEO_DECODER_SEND_SLI:
		case MS_VIDEO_DECODER_SEND_RPSI:
			/* Handled internally by mediastreamer2 */
			break;
		case MS_CAMERA_PREVIEW_SIZE_CHANGED: {
			MSVideoSize size = *(MSVideoSize *)args;
			lInfo() << "Camera video preview size changed: " << size.width << "x" << size.height;
			linphone_core_resize_video_preview(getCCore(), size.width, size.height);
			break;
		}
		default:
			lWarning() << "Unhandled event " << eventId;
			break;
	}
}

void MS2VideoStream::sVideoStreamDisplayCb(void *userData, const unsigned int eventId, const void *args) {
	MS2VideoStream *zis = static_cast<MS2VideoStream *>(userData);
	zis->videoStreamDisplayCb(eventId, args);
}

void MS2VideoStream::videoStreamDisplayCb(const unsigned int eventId, const void *args) {
	auto participantDevice = getMediaSession().getParticipantDevice(LinphoneStreamTypeVideo, getLabel());

	switch (eventId) {
		case MS_VIDEO_DISPLAY_ERROR_OCCURRED:
			getMediaSession().notifyVideoDisplayErrorOccurred(*((int *)args));
			if (participantDevice) participantDevice->videoDisplayErrorOccurred(*((int *)args));
			break;
		default: {
		}
	}
}

void MS2VideoStream::sCameraNotWorkingCb(void *userData, const MSWebCam *oldWebcam) {
	MS2VideoStream *msp = static_cast<MS2VideoStream *>(userData);
	msp->cameraNotWorkingCb(oldWebcam->name);
}

void MS2VideoStream::cameraNotWorkingCb(const char *cameraName) {
	getMediaSession().notifyCameraNotWorking(cameraName);
}

void MS2VideoStream::csrcChangedCb(uint32_t new_csrc) {
	const auto conference = getCore().findConference(getMediaSession().getSharedFromThis(), false);
	if (conference) {
		const auto clientConference = dynamic_pointer_cast<ClientConference>(conference);
		if (clientConference) clientConference->notifyDisplayedSpeaker(new_csrc);
	}
}

void MS2VideoStream::sCsrcChangedCb(void *userData, uint32_t new_csrc) {
	MS2VideoStream *vs = static_cast<MS2VideoStream *>(userData);
	vs->csrcChangedCb(new_csrc);
}

MediaStream *MS2VideoStream::getMediaStream() const {
	if (mStream) return &mStream->ms;
	else return nullptr;
}

MSWebCam *MS2VideoStream::getVideoDevice(CallSession::State targetState) const {
	bool paused = (targetState == CallSession::State::Pausing) || (targetState == CallSession::State::Paused);
	if (paused || mMuted || !cameraEnabled())
		return ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(getCCore()->factory),
		                                  "StaticImage: Static picture");
	else return getCCore()->video_conf.device;
}

MSWebCam *MS2VideoStream::getVideoDevice() const {
	return getVideoDevice(getGroup().getCurrentSessionState());
}

void MS2VideoStream::initZrtp() {
	Stream *audioStream = getGroup().lookupMainStream(SalAudio);
	if (audioStream) {
		MS2AudioStream *msa = dynamic_cast<MS2AudioStream *>(audioStream);
		video_stream_enable_zrtp(mStream, (AudioStream *)msa->getMediaStream());
		// Copy newly created zrtp context into mSessions
		media_stream_reclaim_sessions((MediaStream *)mStream, &mSessions);
		if (mSessions.zrtp_context) {
			ms_zrtp_enable_go_clear(mSessions.zrtp_context, linphone_core_zrtp_go_clear_enabled(getCCore()));
		}
	} else {
		lError() << "Unable to initiate ZRTP session because no audio stream is attached to video stream " << this
		         << ".";
	}
}

void MS2VideoStream::startZrtp() {
	// If the bundle has been accepted by the remote party, there is no need to start the ZRTP video stream if it is not
	// the transport owner as all traffic will go through the transport owner of the bundle
	if (!isTransportOwner()) {
		lWarning() << "Do not start video stream " << *this << " because it isn't the transport owner of the bundle of "
		           << getGroup();
		return;
	}
	/* initialize ZRTP if it supported as default encryption or as optional encryption and capability negotiation is
	 * enabled */
	if (getMediaSessionPrivate().isMediaEncryptionAccepted(LinphoneMediaEncryptionZRTP)) {

		if (!mSessions.zrtp_context) {
			initZrtp();
		}

		if (mSessions.zrtp_context) {
			video_stream_start_zrtp(mStream);
		} else {
			lError() << "Error while enabling zrtp on video stream: ZRTP context is NULL";
		}
	}
}

bool MS2VideoStream::isThumbnail() const {
	return (video_stream_get_content(mStream) == MSVideoContentThumbnail);
}

std::string MS2VideoStream::getLabel() const {
	return L_C_TO_STRING(mStream->label);
}

bool MS2VideoStream::isFecEnabled() const {
	MediaStream *ms = getMediaStream();
	return media_stream_fec_enabled(ms);
}

bool MS2VideoStream::prepare() {

	MS2Stream::prepare();
	if (isTransportOwner()) {
		video_stream_prepare_video(mStream);
	}
	return false;
}

void MS2VideoStream::finishPrepare() {
	MS2Stream::finishPrepare();
	video_stream_unprepare_video(mStream);
}

MSVideoDisplayMode stringToVideoDisplayMode(const string &mode) {
	if (mode.compare("Hybrid") == 0) {
		return MSVideoDisplayHybrid;
	} else if (mode.compare("BlackBars") == 0) {
		return MSVideoDisplayBlackBars;
	} else if (mode.compare("OccupyAllSpace") == 0) {
		return MSVideoDisplayOccupyAllSpace;
	} else {
		lWarning() << "Video stream set display mode " << mode
		           << " failed, available values {Hybrid, BlackBars, OccupyAllSpace}.";
	}
	return MSVideoDisplayHybrid;
}

void MS2VideoStream::runAlertMonitors() {
	VideoStats sendStats, recvStats;
	getSendStats(&sendStats);
	getRecvStats(&recvStats);
	float fps = cameraEnabled() ? video_stream_get_sent_framerate(mStream) : -1;
	mVideoMonitor.check(&sendStats, &recvStats, fps);
	mBandwidthMonitor.check(mStats);
	mNetworkMonitor.checkNackQuality(mStream->ms.sessions.rtp_session);
	MS2Stream::runAlertMonitors();
}

// Do swap like specified in [Window ID distribution for screen sharing] table in MediaSession::getNativeVideoWindowId.
// It is needed to clean old window ID and avoid concurrency.
bool MS2VideoStream::enableLocalScreenSharing(bool value) {
	bool changed = false;
	auto service = getGroup().getSharedService<ScreenSharingService>();
	if (service->updateLocalScreenSharing(value)) {
		changed = true;
		VideoStream *vs = getVideoStream();
		if (vs) {
			video_stream_enable_local_screen_sharing(vs, value);
			if (value) { // Activation. Stop native and swap
				ms_debug("Enabling Screen sharing. Set preview ID to %x", mNativeWindowId);
				video_stream_set_native_window_id(vs, NULL); // Clean first to avoid concurrency.
				video_stream_set_native_preview_window_id(vs, mNativeWindowId);
			} else { // Deactivation : Swap end reuse old preview id
				ms_debug("Disabling Screen sharing. Set preview ID to %x and native ID to %x", mNativePreviewWindowId,
				         mNativeWindowId);
				video_stream_set_native_preview_window_id(vs, mNativePreviewWindowId);
				video_stream_set_native_window_id(vs, mNativeWindowId);
			}
		}
	}
	return changed;
}

void MS2VideoStream::updateWindowId(const std::shared_ptr<ParticipantDevice> &participantDevice,
                                    const std::string &label,
                                    bool isMe,
                                    bool isThumbnail,
                                    bool fallbackToCore) {
	void *windowId = nullptr;
	if (mNativeWindowId) {
		windowId = mNativeWindowId;
	} else if (isMe && isThumbnail) { // Preview thumbnail is always set by core.
		windowId = getCCore()->preview_window_id;
	} else if (!label.empty()) {
		if (participantDevice) windowId = participantDevice->getWindowId();
		else windowId = getMediaSession().getParticipantWindowId(label);
	} else if (fallbackToCore && getCCore()->video_window_id) {
		windowId = getCCore()->video_window_id;
	} else {
		lWarning() << " Video stream " << mStream << " does not set native window id";
	}
	// Only set for Main or me's thumbnail.
	if (isMe && isThumbnail) {
		setNativePreviewWindowId(windowId);
	} else {
		setNativeWindowId(windowId);
		if (mNativePreviewWindowId) setNativePreviewWindowId(mNativePreviewWindowId);
	}
}

void MS2VideoStream::render(const OfferAnswerContext &ctx, CallSession::State targetState) {
	bool reusedPreview = false;
	MS2VideoMixer *videoMixer = getVideoMixer();
	const auto localIsInConference = getMediaSessionPrivate().getParams()->getPrivate()->getInConference();
	const auto &localDesc = ctx.getLocalStreamDescription();
	const auto &remoteDesc = ctx.getRemoteStreamDescription();
	const auto &label = localIsInConference ? localDesc.getLabel() : remoteDesc.getLabel();
	const auto &content = localIsInConference ? remoteDesc.getContent() : localDesc.getContent();
	MSFilter *source = nullptr;
	MSWebCam *cam = nullptr;
	const MSWebCam *currentCam = video_stream_get_camera(mStream);
	bool cameraChanged = false;
	bool isScreenSharing = false;

	video_stream_set_fallback_to_dummy_codec(
	    mStream,
	    linphone_config_get_bool(linphone_core_get_config(getCCore()), "video", "fallback_to_dummy_codec", TRUE));

	/* Shutdown preview */
	if (getCCore()->previewstream) {
		if (getCCore()->video_conf.reuse_preview_source)
			source = video_preview_stop_reuse_source(getCCore()->previewstream);
		else video_preview_stop(getCCore()->previewstream);
		getCCore()->previewstream = nullptr;
	}
	bool basicChangesHandled = handleBasicChanges(ctx, targetState);
	bool contentIsThumbnail = (content == "thumbnail");
	bool isMe = false;
	bool isConferenceScreenSharing = false; // Used to check display mode for SendRecv stream (aka isMain())
	MS2VideoStream *auxStream = nullptr;

	const auto conference = getCore().findConference(getMediaSession().getSharedFromThis(), false);
	if (conference) {
		auto screenSharingDevice = conference->getScreenSharingDevice();
		if (screenSharingDevice) isConferenceScreenSharing = screenSharingDevice->screenSharingEnabled();
		if (conference->getCurrentParams()->getSecurityLevel() == ConferenceParamsInterface::SecurityLevel::EndToEnd) {
			if (getMediaSessionPrivate().getParams()->getPrivate()->getInConference()) {
				lInfo() << "MS2Videostream::render End2End encrypted local conference";
				setEktMode(MS_EKT_TRANSFER);
			} else {
				lInfo() << "MS2Videostream::render End2End encrypted remote conference";
				setEktMode(MS_EKT_ENABLED);
			}
		}
	}

	// Main output display mode depend of participant screen sharing state.
	bool localScreenSharingChanged = false, displayModeChanged = false;
	auto participantDevice = getMediaSession().getParticipantDevice(LinphoneStreamTypeVideo, label);
	if (!participantDevice) {
		if (conference) {
			participantDevice = conference->getMe()->findDevice(LinphoneStreamTypeVideo, label, false);
			// is Me. Q : Me is always local? (multi account)
			isScreenSharing = (participantDevice && participantDevice->screenSharingEnabled());
			isMe = !!participantDevice; // Just for safety. In normal case, if no device have been found in MediaSession
			                            // then it should be in getMe().
			if (isMe) {
				// Get auxiliary stream
				if (contentIsThumbnail) { // Get Main Stream
					auxStream = dynamic_cast<MS2VideoStream *>(getGroup().lookupMainStream(SalVideo));
					if (auxStream) {
						localScreenSharingChanged = auxStream->enableLocalScreenSharing(isScreenSharing);
						if (!localScreenSharingChanged && isScreenSharing) {
							auto auxVs = auxStream->getVideoSource();
							auto vs = getVideoStream();
							cameraChanged = auxVs && auxVs->getType() == VideoSourceDescriptor::Type::ScreenSharing &&
							                vs->cam != auxStream->getVideoDevice(targetState);
						}
					}
				} else { // Get Thumbnail Stream.
					int idx = getMediaSession().getLocalThumbnailStreamIdx();
					if (idx >= 0) auxStream = dynamic_cast<MS2VideoStream *>(getGroup().getStream(idx));
					localScreenSharingChanged = enableLocalScreenSharing(isScreenSharing);
				}
			}
		}
	} else isScreenSharing = participantDevice->screenSharingEnabled();

	// Display mode detection
	MSVideoDisplayMode displayPreviewMode;
	// Preview is forced to BlackBars only if the stream display a screen sharing.
	// (Reminder: for Me(), Thumbnail is camera)
	if (isScreenSharing && isMe && !contentIsThumbnail) displayPreviewMode = MSVideoDisplayBlackBars;
	else if (!label.empty() && !isMain())
		displayPreviewMode = stringToVideoDisplayMode(linphone_config_get_string(
		    linphone_core_get_config(getCCore()), "video", "other_display_mode", "OccupyAllSpace"));
	else
		displayPreviewMode = stringToVideoDisplayMode(
		    linphone_config_get_string(linphone_core_get_config(getCCore()), "video", "main_display_mode", "Hybrid"));
	// For normal display, BlackBars is forced on all case of screen sharing:
	//- Me() is screen sharing.
	//- The device is screen sharing and is coming from ActiveSpeaker.
	//- The stream is ActiveSpeaker and is screen sharing.
	MSVideoDisplayMode displayMode =
	    (isScreenSharing && (isMe || !contentIsThumbnail)) || (isMain() && isConferenceScreenSharing)
	        ? MSVideoDisplayBlackBars
	        : displayPreviewMode;
	displayModeChanged = (video_stream_get_display_mode(mStream) != displayMode) ||
	                     (video_stream_get_preview_display_mode(mStream) != displayPreviewMode);

	if (basicChangesHandled) {
		bool muted = mMuted;
		if (getState() == Running) {
			MS2Stream::render(ctx, targetState); // MS2Stream::render() may decide to unmute.
			if (muted && !mMuted) {
				lInfo() << "Early media finished, unmuting video input...";
				/* We were in early media, now we want to enable real media */
				mMuted = false;
				// when screen sharing, Camera is only used for thumbnail.
				if (!isScreenSharing || (isMe && contentIsThumbnail)) {
					enableCamera(cameraEnabled());
					currentCam = getVideoDevice(targetState);
				}
			}
			// Update video device
			cam = getVideoDevice(targetState);
			cameraChanged = cameraChanged || (currentCam && cam && (currentCam != cam));
		}

		if (!label.empty() && label.compare(getLabel()) != 0) {
			lInfo() << "Handling label change - previously it was " << getLabel() << " and now it is " << label;
			// If not Main, reset id with the participantDevice from label to avoid reusing an old window id that
			// doesn't exist anymore.
			if (!isMain()) setNativeWindowId(NULL);
			updateWindowId(participantDevice, label, isMe, contentIsThumbnail, false);
			video_stream_set_label(mStream, L_STRING_TO_C(label));
		}
		if (!cameraChanged && !localScreenSharingChanged &&
		    !displayModeChanged) { // Check only if mStream.State == Stopped?
			if (isMe) {
				updateWindowId(participantDevice, label, isMe, contentIsThumbnail, false);
			}
			return;
		}
	}

	if (localScreenSharingChanged) { // We need to stop this stream and the thumbnail stream because of
		                             // source filter concurrency, and
		                             // display mode replumbing
		lInfo() << "Screen sharing mode changed : ScreenSharing=" << isScreenSharing << ". Stopping current stream ("
		        << mStream << ") and auxiliary stream(" << auxStream << ")";
		stop();
		if (auxStream) auxStream->stop();
	} else if (displayModeChanged) {
		lInfo() << "Display mode changed.. Stopping current stream (" << mStream << ")";
		stop();
	} else if (cameraChanged) {
		if (currentCam && cam)
			lInfo() << "Camera device changed from " << ms_web_cam_get_name(currentCam) << " to "
			        << ms_web_cam_get_name(cam);
		stop();
	}

	const auto &vstream = ctx.getResultStreamDescription();

	if (vstream == Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		lError() << "Unable to find video stream";
		stop();
		return;
	}

	int usedPt = -1;
	RtpProfile *videoProfile = makeProfile(ctx.resultMediaDescription, vstream, &usedPt);
	if (usedPt == -1) {
		lError() << "No payload types accepted for video stream !";
		// Screen sharing cannot take place if no payload has been negotiated
		enableLocalScreenSharing(false);
		stop();
		return;
	}

	video_stream_enable_display_filter_auto_rotate(
	    mStream,
	    !!linphone_config_get_int(linphone_core_get_config(getCCore()), "video", "display_filter_auto_rotate", 0));

	const char *displayFilter = linphone_core_get_video_display_filter(getCCore());
	if (displayFilter) video_stream_set_display_filter_name(mStream, displayFilter);
	video_stream_set_event_callback(mStream, sVideoStreamEventCb, this);
	video_stream_set_display_callback(mStream, sVideoStreamDisplayCb, this);
	video_stream_set_camera_not_working_callback(mStream, sCameraNotWorkingCb, this);
	if (isMain()) {
		OrtpPayloadType *pt = rtp_profile_get_payload(videoProfile, usedPt);
		getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedVideoCodec(
		    pt ? PayloadType::create(getCore().getSharedFromThis(), pt) : nullptr);
	}

	if (getCCore()->video_conf.preview_vsize.width != 0)
		video_stream_set_preview_size(mStream, getCCore()->video_conf.preview_vsize);
	video_stream_set_fps(mStream, linphone_core_get_preferred_framerate(getCCore()));
	if (linphone_config_get_int(linphone_core_get_config(getCCore()), "video", "nowebcam_uses_normal_fps", 0))
		mStream->staticimage_webcam_fps_optimization = false;
	auto service = getGroup().getSharedService<ScreenSharingService>();
	auto screensharingEnabledInService = service->localScreenSharingEnabled();
	if (isMain()) {
		// Reset the main stream with the current state of screen sharing that could be lost on restart,
		// before using window ID.
		video_stream_enable_local_screen_sharing(mStream, screensharingEnabledInService);
	}

	LinphoneVideoDefinition *max_vdef = nullptr;

	if (conference) {
		if (!label.empty() && label.compare(getLabel()) != 0) {
			lInfo() << "Handling label change - previously it was " << getLabel() << " and now it is " << label;
			// If not Main, reset id with the participantDevice from label to avoid reusing an old window id that
			// doesn't exist anymore.
			if (!isMain()) setNativeWindowId(NULL);
			updateWindowId(participantDevice, label, isMe, contentIsThumbnail, false);

			video_stream_set_label(mStream, L_STRING_TO_C(label));
		}
		const char *str =
		    linphone_config_get_string(linphone_core_get_config(getCCore()), "video", "max_conference_size", nullptr);
		if (str != NULL && str[0] != 0) {
			max_vdef = linphone_factory_find_supported_video_definition_by_name(linphone_factory_get(), str);
			if (max_vdef == NULL) {
				lError() << "Cannot set max video size in mosaic (video definition '" << str << "' not supported)";
			} else {
				MSVideoSize max;
				max.width = static_cast<int>(linphone_video_definition_get_width(max_vdef));
				max.height = static_cast<int>(linphone_video_definition_get_height(max_vdef));
				video_stream_set_sent_video_size_max(mStream, max);
			}
		}
	}

	if (!contentIsThumbnail) {
		if (conference) video_stream_set_csrc_changed_callback(mStream, sCsrcChangedCb, this);
		else video_stream_set_csrc_changed_callback(mStream, nullptr, nullptr);
	}

	if (conference) {
		if (const auto &streamCfg = vstream.getActualConfiguration(); streamCfg.getFrameMarkingExtensionId() > 0) {
			video_stream_set_frame_marking_extension_id(mStream, streamCfg.getFrameMarkingExtensionId());
			rtp_session_set_ssrc_changed_threshold(mStream->ms.sessions.rtp_session, 1);
		}

		const auto serverConference = dynamic_pointer_cast<ServerConference>(conference);
		if (serverConference) {
			// when conference is local(we are a server), enable the transfer mode if needed
			LinphoneConfig *config = linphone_core_get_config(getCCore());
			if (static_cast<MSConferenceMode>(
			        linphone_config_get_int(config, "video", "conference_mode", MSConferenceModeRouterPayload)) ==
			    MSConferenceModeRouterFullPacket) {
				media_stream_enable_transfer_mode(&mStream->ms, TRUE);
			}
		} else {
			// If we are a client in a RemoteConference, enable the active speaker mode.
			// This mode will listen to any new incoming ssrc in the stream.
			video_stream_enable_active_speaker_mode(mStream, TRUE);
		}
	}

	const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(getCCore());
	if (vdef) {
		MSVideoSize vsize;
		// If max size is set and preferred size is superior then use max size
		if (max_vdef != nullptr && max_vdef->width < vdef->width && max_vdef->height < vdef->height) {
			vsize.width = static_cast<int>(linphone_video_definition_get_width(max_vdef));
			vsize.height = static_cast<int>(linphone_video_definition_get_height(max_vdef));
		} else {
			vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
			vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
		}
		video_stream_set_sent_video_size(mStream, vsize);
	}
	video_stream_enable_self_view(mStream, getCCore()->video_conf.selfview);
	updateWindowId(participantDevice, label, isMe, contentIsThumbnail);
	video_stream_use_preview_video_window(mStream, getCCore()->use_preview_window);

	MS2Stream::render(ctx, targetState);
	// Since MS2Stream::render() may affect the used VideoDevice, this assignment must be done here.
	cam = getVideoDevice(targetState);

	RtpAddressInfo dest;
	getRtpDestination(ctx, &dest);
	MediaStreamDir dir = MediaStreamSendRecv;

	if ((vstream.getDirection() == SalStreamSendOnly) && getCCore()->video_conf.capture) {
		dir = MediaStreamSendOnly;
	} else if ((vstream.getDirection() == SalStreamRecvOnly) && getCCore()->video_conf.display) {
		dir = MediaStreamRecvOnly;
	} else if (vstream.getDirection() == SalStreamSendRecv) {
		if (getCCore()->video_conf.display && getCCore()->video_conf.capture) {
			dir = MediaStreamSendRecv;
		} else if (getCCore()->video_conf.display) {
			dir = MediaStreamRecvOnly;
		} else {
			dir = MediaStreamSendOnly;
		}
	} else {
		// Screen sharing cannot take place on an inactive stream
		enableLocalScreenSharing(false);
		lWarning() << "Video stream is inactive";
		/* Either inactive or incompatible with local capabilities */
		stop();
		return;
	}

	if (vstream.multicast_role == SalMulticastReceiver) {
		dir = MediaStreamRecvOnly;
	} else if (vstream.multicast_role == SalMulticastSender) {
		dir = MediaStreamSendOnly;
	}

	getMediaSession().getLog()->setVideoEnabled(true);
	media_stream_set_direction(&mStream->ms, dir);
	lInfo() << "Device rotation =" << getCCore()->device_rotation;
	video_stream_set_device_rotation(mStream, getCCore()->device_rotation);
	video_stream_set_freeze_on_error(
	    mStream, !!linphone_config_get_int(linphone_core_get_config(getCCore()), "video", "freeze_on_error", 1));
	video_stream_use_video_preset(
	    mStream, linphone_config_get_string(linphone_core_get_config(getCCore()), "video", "preset", nullptr));
	video_stream_set_preview_display_mode(mStream, displayPreviewMode);
	video_stream_set_display_mode(mStream, displayMode);
	configure(ctx);

	if (getCCore()->video_conf.reuse_preview_source && source) {
		lInfo() << "video_stream_start_with_source kept: " << source;
		video_stream_start_with_source(mStream, videoProfile, dest.rtpAddr.c_str(), dest.rtpPort, dest.rtcpAddr.c_str(),
		                               dest.rtcpPort, usedPt, -1, cam, source);
		reusedPreview = true;
	} else {
		bool ok = true;
		MSMediaStreamIO io = MS_MEDIA_STREAM_IO_INITIALIZER;
		if (linphone_config_get_bool(linphone_core_get_config(getCCore()), "video", "rtp_io", FALSE)) {
			io.input.type = io.output.type = MSResourceRtp;
			io.input.session = io.output.session = createRtpIoSession();
			if (!io.input.session) {
				ok = false;
				lWarning() << "Cannot create video RTP IO session";
			}
		} else {
			// Set io from video source
			if (!videoMixer) {
				auto videoSource = getVideoSource();
				io.output.type = MSResourceDefault;
				if (videoSource && isScreenSharing &&
				    videoSource->getType() == VideoSourceDescriptor::Type::ScreenSharing) {
					io.input.type = MSResourceScreenSharing;
					io.input.screen_sharing = videoSource->getScreenSharingDesc();
				} else {
					io.input.type = MSResourceCamera;
					io.input.camera = cam;
				}
			} else {
				io.input.type = MSResourceVoid;
				io.input.camera = cam;
				io.output.type = MSResourceVoid;
			}
		}
		if (ok) {
			if (videoMixer == nullptr && !label.empty() && dir == MediaStreamSendOnly && contentIsThumbnail) {
				// The current stream is thumbnail, search for the main stream in order to get data on what we are using
				// for thumbnail (camera or Itc):
				MS2VideoStream *vs = dynamic_cast<MS2VideoStream *>(getGroup().lookupMainStream(SalVideo));
				MSWebCam *itcCamera = nullptr;
				MSFilter *itcFilter = nullptr;
				if (vs) {
					VideoStream *itcStream = vs->getVideoStream();
					itcFilter = itcStream->itcsink;
					itcCamera = vs->getVideoDevice(targetState);
					lInfo() << "[mix to all] this thumbnail stream " << mStream << " find itcStream " << itcStream
					        << " with label " << label;
				} else {
					lError() << "[mix to all] this thumbnail stream " << mStream
					         << " can not find itcStream with label " << label;
				}
				// Set the source of the Thumbnail
				if (isScreenSharing && isMe) {
					// In the case of screen sharing, we want to send the camera of vs if it exists.
					io.input.type = MSResourceCamera;
					io.input.camera = itcCamera;
				} else {
					io.input.type = MSResourceItc;
					io.input.itc = itcFilter;
				}
				media_stream_set_max_network_bitrate(&mStream->ms, 80000);

				MSVideoSize vsize = {160, 120};
				video_stream_set_content(mStream, MSVideoContentThumbnail);
				/* TODO The fps should be taken automatically from the main stream, however this is not implemented.
				 * Meanwhile, force a medium 20 fps so that video encoder gets configured with realistic values.
				 */
				video_stream_set_fps(mStream, 20);
				video_stream_set_sent_video_size(mStream, vsize);

				video_stream_start_from_io(mStream, videoProfile, dest.rtpAddr.c_str(), dest.rtpPort,
				                           dest.rtcpAddr.c_str(), dest.rtcpPort, usedPt, &io);
			} else {
				video_stream_start_from_io(mStream, videoProfile, dest.rtpAddr.c_str(), dest.rtpPort,
				                           dest.rtcpAddr.c_str(), dest.rtcpPort, usedPt, &io);

				if (videoMixer == nullptr && !label.empty() && dir != MediaStreamRecvOnly && !contentIsThumbnail &&
				    !screensharingEnabledInService) {
					getGroup().addPostRenderHook([this, &label] {
						link_video_stream_with_itc_sink(mStream);
						// Current stream is Main, search for the thumbnail to connect with ITC.
						MS2VideoStream *vs = nullptr;
						int idx = getMediaSession().getLocalThumbnailStreamIdx();
						if (idx >= 0) vs = dynamic_cast<MS2VideoStream *>(getGroup().getStream(idx));
						if (vs) {
							VideoStream *itcStream = vs->getVideoStream();
							lInfo() << "[mix to all] this normal stream " << mStream << " find itcStream " << itcStream
							        << " with label " << label;
							ms_filter_call_method(mStream->itcsink, MS_ITC_SINK_CONNECT, itcStream->source);
						}
					});
				}

				AudioStream *as = getPeerAudioStream();
				if (as) audio_stream_link_video(as, mStream);
			}
		}
	}
	mInternalStats.number_of_starts++;

	getMediaSession().notifyResetFirstVideoFrameDecoded();
	/* Start ZRTP engine if needed : set here or remote have a zrtp-hash attribute */
	const auto &remoteStream = ctx.getRemoteStreamDescription();
	if ((getMediaSessionPrivate().getNegotiatedMediaEncryption() == LinphoneMediaEncryptionZRTP) ||
	    (remoteStream.getChosenConfiguration().hasZrtpHash() == 1)) {
		Stream *audioStream = getGroup().lookupMainStream(SalAudio);
		/* Audio stream is already encrypted and video stream is active */
		if (audioStream && audioStream->isEncrypted()) {
			startZrtp();
			if (mSessions.zrtp_context && (remoteStream.getChosenConfiguration().hasZrtpHash() == 1)) {
				int retval = ms_zrtp_setPeerHelloHash(
				    mSessions.zrtp_context, (uint8_t *)remoteStream.getChosenConfiguration().getZrtpHash(),
				    strlen((const char *)(remoteStream.getChosenConfiguration().getZrtpHash())));
				if (retval != 0) lError() << "Video stream ZRTP hash mismatch 0x" << hex << retval;
			}
		}
	}

	if (linphone_core_retransmission_on_nack_enabled(getCCore())) {
		video_stream_enable_retransmission_on_nack(mStream, TRUE);
	}

	if (!reusedPreview && source) {
		/* Destroy not-reused source filter */
		lWarning() << "Video preview (" << source << ") not reused: destroying it";
		ms_filter_destroy(source);
	}

	if (videoMixer && (targetState == CallSession::State::StreamsRunning)) {
		mConferenceEndpoint = ms_video_endpoint_get_from_stream(mStream, true, videoMixer->getConferenceParams().mode);
		videoMixer->connectEndpoint(this, mConferenceEndpoint, isThumbnail());
	}
}

void MS2VideoStream::stop() {
	MS2Stream::stop();
	AudioStream *as = getPeerAudioStream();
	if (as) audio_stream_unlink_video(as, mStream);

	if (mConferenceEndpoint) {
		// First disconnect from the mixer before stopping the stream.
		getVideoMixer()->disconnectEndpoint(this, mConferenceEndpoint);
		ms_video_endpoint_release_from_stream(mConferenceEndpoint);
		mConferenceEndpoint = nullptr;
	}
	video_stream_stop(mStream);
	/* In mediastreamer2, stop actually stops and destroys. We immediately need to recreate the stream object for
	 * later use, keeping the sessions (for RTP, SRTP, ZRTP etc) that were setup at the beginning. */
	mStream = video_stream_new_with_sessions(getCCore()->factory, &mSessions);

	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedVideoCodec(nullptr);
}

void MS2VideoStream::handleEvent(const OrtpEvent *ev) {
	OrtpEventType evt = ortp_event_get_type(ev);
	OrtpEventData *evd = ortp_event_get_data(const_cast<OrtpEvent *>(ev));
	if (evt == ORTP_EVENT_NEW_VIDEO_BANDWIDTH_ESTIMATION_AVAILABLE) {
		if (isMain()) mStats->setEstimatedDownloadBandwidth((float)(evd->info.video_bandwidth_available * 1e-3));
	} else if (evt == ORTP_EVENT_JITTER_UPDATE_FOR_NACK) {
		mNetworkMonitor.confirmNackSent();
	}
}

void MS2VideoStream::zrtpStarted(Stream *mainZrtpStream) {
	MS2Stream::zrtpStarted(mainZrtpStream);
	if (getState() == Running) {
		if (getMediaSessionPrivate().isEncryptionMandatory()) {
			/* Nothing could have been sent yet so generating key frame */
			video_stream_send_vfu(mStream);
		}
	}
}

void MS2VideoStream::tryEarlyMediaForking(const OfferAnswerContext &ctx) {
	MS2Stream::tryEarlyMediaForking(ctx);
	sendVfu();
}

void MS2VideoStream::oglRender() {
	if (mStream && mStream->output && (ms_filter_get_id(mStream->output) == MS_OGL_ID))
		ms_filter_call_method(mStream->output, MS_OGL_RENDER, nullptr);
}

AudioStream *MS2VideoStream::getPeerAudioStream() {
	MS2AudioStream *as = getGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
	return as ? (AudioStream *)as->getMediaStream() : nullptr;
}

void MS2VideoStream::onSnapshotTaken(const string &filepath) {
	getMediaSession().notifySnapshotTaken(L_STRING_TO_C(filepath));
}

void MS2VideoStream::finish() {
	if (mStream) {
		video_stream_stop(mStream);
		mStream = nullptr;
	}
	MS2Stream::finish();
}

VideoStream *MS2VideoStream::getVideoStream() const {
	return (VideoStream *)mStream;
}

MS2VideoMixer *MS2VideoStream::getVideoMixer() {
	StreamMixer *mixer = getMixer();
	if (mixer) {
		MS2VideoMixer *videoMixer = dynamic_cast<MS2VideoMixer *>(mixer);
		if (!videoMixer) {
			lError() << *this << " does not have a mixer it is able to interface with.";
		}
		return videoMixer;
	}
	return nullptr;
}

void MS2VideoStream::setVideoSource(const std::shared_ptr<const VideoSourceDescriptor> &descriptor) {
	if (!descriptor) return;

	if (mStream == nullptr) {
		lError() << "Could not find video stream while attempting to change video source on MS2VideoStream [" << this
		         << "]";
		return;
	}
	if (!descriptor) {
		mVideoSourceDescriptor = nullptr;
		return;
	}
	switch (descriptor->getType()) {
		case VideoSourceDescriptor::Type::Call: {
			const auto &call = descriptor->getCall();

			MS2VideoStream *source_stream =
			    call->getMediaSession()->getStreamsGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);

			if (source_stream->mStream == nullptr) {
				lError() << "Could not find video stream of supplied call while attempting to change video source on "
				            "MS2VideoStream ["
				         << this << "]";
				return;
			}

			video_stream_forward_source_stream(mStream, source_stream->mStream);
		} break;
		case VideoSourceDescriptor::Type::Camera: {
			const auto &id = descriptor->getCameraId();

			MSFactory *factory = linphone_core_get_ms_factory(getCCore());
			MSWebCam *cam = ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(factory), id.c_str());

			if (cam == nullptr) {
				lError() << "Could not find camera id \"" << id
				         << "\" while attempting to change video source on MS2VideoStream [" << this << "]";
				return;
			}

			video_stream_change_camera(mStream, cam);
		} break;
		case VideoSourceDescriptor::Type::Image: {
			const auto &imagePath = descriptor->getImage();

			MSFactory *factory = linphone_core_get_ms_factory(getCCore());
			MSWebCam *cam =
			    ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(factory), "StaticImage: Static picture");
			MSFilter *imageFilter = ms_web_cam_create_reader(cam);

			if (imageFilter == nullptr) {
				lError()
				    << "Could not create filter for image while attempting to change video source on MS2VideoStream ["
				    << this << "]";
				return;
			}

			ms_filter_call_method(imageFilter, MS_STATIC_IMAGE_SET_IMAGE, (void *)(imagePath.c_str()));

			video_stream_change_source_filter(mStream, NULL, imageFilter, FALSE);
		} break;
		case VideoSourceDescriptor::Type::ScreenSharing: {
			if (mStream->source) {
				auto sourceDescriptor = descriptor->getScreenSharingDesc();
				if (ms_filter_get_id(mStream->source) == MS_SCREEN_SHARING_ID) {
					// The filter is not changing so the configuration should remains and there is no need to
					// replumb. If replumbing is really needed, the filter should send a
					// MS_FILTER_OUTPUT_FMT_CHANGED event.
					ms_filter_call_method(mStream->source, MS_SCREEN_SHARING_SET_SOURCE_DESCRIPTOR,
					                      (void *)&sourceDescriptor);
				} else {
					MSFactory *factory = linphone_core_get_ms_factory(getCCore());
					MSFilter *screenSharingFilter = ms_factory_create_filter(factory, MS_SCREEN_SHARING_ID);
					ms_filter_call_method(screenSharingFilter, MS_SCREEN_SHARING_SET_SOURCE_DESCRIPTOR,
					                      (void *)&sourceDescriptor);
					video_stream_change_source_filter(mStream, NULL, screenSharingFilter, FALSE);
				}
			}
		} break;
		case VideoSourceDescriptor::Type::Unknown:
			lError() << "Cannot change video source with an unknown video source type";
			mVideoSourceDescriptor = nullptr;
			return;
	}

	mVideoSourceDescriptor = descriptor;
}

std::shared_ptr<const VideoSourceDescriptor> MS2VideoStream::getVideoSource() const {
	return mVideoSourceDescriptor;
}

MS2VideoStream::~MS2VideoStream() {
	if (mStream) video_stream_stop(mStream);
}

/*
 * Here comes the utility class that implements usual controls on top of mediastreamer2's VideoStream.
 */

MS2VideoControl::MS2VideoControl(Core &core) : mCore(core) {
}

bool MS2VideoControl::cameraEnabled() const {
	return mCameraEnabled;
}

// Check [-- Window ID distribution for screen sharing ---] table in MediaSession::getNativeVideoWindowId
void *MS2VideoControl::createNativeWindowId() const {
	VideoStream *vs = getVideoStream();
	return vs ? video_stream_local_screen_sharing_enabled(vs) ? video_stream_create_native_preview_window_id(vs)
	                                                          : video_stream_create_native_window_id(vs)
	          : nullptr;
}

void MS2VideoControl::setNativeWindowId(void *w) {
	VideoStream *vs = getVideoStream();
	mNativeWindowId = w;
	if (vs) {
		if (video_stream_local_screen_sharing_enabled(vs)) {
			video_stream_set_native_preview_window_id(vs, w);
		} else {
			video_stream_set_native_window_id(vs, w);
		}
	}
}

void *MS2VideoControl::getNativeWindowId() const {
	VideoStream *vs = getVideoStream();
	if (mNativeWindowId) {
		return mNativeWindowId;
	}
	/* It was not set but we want to get the one automatically created by mediastreamer2 (desktop versions only) */
	return vs ? video_stream_local_screen_sharing_enabled(vs) ? video_stream_get_native_preview_window_id(vs)
	                                                          : video_stream_get_native_window_id(vs)
	          : nullptr;
}

// Preview API should not be call for Main stream if screen sharing is activated.
// MediaSession knows what stream to call:
// In this case, the check on 'video_stream_local_screen_sharing_enabled' is enough to know what preview the stream is
// using.
void *MS2VideoControl::createNativePreviewWindowId() const {
	VideoStream *vs = getVideoStream();
	if (vs) lInfo() << "Create " << this << " / " << vs << " -- " << vs->output2;
	return vs && !video_stream_local_screen_sharing_enabled(vs) ? video_stream_create_native_preview_window_id(vs)
	                                                            : nullptr;
}

void MS2VideoControl::setNativePreviewWindowId(void *w) {
	VideoStream *vs = getVideoStream();
	mNativePreviewWindowId = w;
	if (vs) {
		lInfo() << this << " / " << vs->output2 << " = " << mNativePreviewWindowId;
		if (!video_stream_local_screen_sharing_enabled(vs)) video_stream_set_native_preview_window_id(vs, w);
	}
}

void *MS2VideoControl::getNativePreviewWindowId() const {
	VideoStream *vs = getVideoStream();
	if (mNativePreviewWindowId) {
		return mNativePreviewWindowId;
	}
	/* It was not set but we want to get the one automatically created by mediastreamer2 (desktop versions only) */
	return vs && !video_stream_local_screen_sharing_enabled(vs) ? video_stream_get_native_preview_window_id(vs)
	                                                            : nullptr;
}

void MS2VideoControl::requestNotifyNextVideoFrameDecoded() {
	VideoStream *vs = getVideoStream();
	if (vs && vs->ms.decoder)
		ms_filter_call_method_noarg(vs->ms.decoder, MS_VIDEO_DECODER_RESET_FIRST_IMAGE_NOTIFICATION);
}

void MS2VideoControl::sSnapshotTakenCb(void *userdata, MSFilter *, unsigned int id, void *arg) {
	MS2VideoControl *d = (MS2VideoControl *)userdata;
	if (id == MS_JPEG_WRITER_SNAPSHOT_TAKEN) {
		MSJpegWriteEventData *data = static_cast<MSJpegWriteEventData *>(arg);
		d->onSnapshotTaken(data->filePath);
	}
}

int MS2VideoControl::takePreviewSnapshot(const string &file) {
	VideoStream *vs = getVideoStream();
	if (vs && vs->local_jpegwriter) {
		ms_filter_clear_notify_callback(vs->jpegwriter);
		const char *filepath = file.empty() ? nullptr : file.c_str();
		ms_filter_add_notify_callback(vs->local_jpegwriter, sSnapshotTakenCb, this, FALSE);
		return ms_filter_call_method(vs->local_jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void *)filepath);
	}
	lWarning() << "Cannot take local snapshot: no currently running video stream on this call";
	return -1;
}

int MS2VideoControl::takeVideoSnapshot(const string &file) {
	VideoStream *vs = getVideoStream();
	if (vs && vs->jpegwriter) {
		ms_filter_clear_notify_callback(vs->jpegwriter);
		const char *filepath = file.empty() ? nullptr : file.c_str();
		ms_filter_add_notify_callback(vs->jpegwriter, sSnapshotTakenCb, this, FALSE);
		return ms_filter_call_method(vs->jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void *)filepath);
	}
	lWarning() << "Cannot take snapshot: no currently running video stream on this call";
	return -1;
}

void MS2VideoControl::parametersChanged() {
	VideoStream *vs = getVideoStream();
	if (!vs) return;
	const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(mCore.getCCore());
	MSVideoSize vsize;
	vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
	vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
	video_stream_set_sent_video_size(vs, vsize);
	video_stream_set_fps(vs, linphone_core_get_preferred_framerate(mCore.getCCore()));
	if (cameraEnabled() && (vs->cam != mCore.getCCore()->video_conf.device))
		video_stream_change_camera(vs, mCore.getCCore()->video_conf.device);
	else video_stream_update_video_params(vs);
}

void MS2VideoControl::enableCamera(bool value) {
	mCameraEnabled = value;
	VideoStream *vs = getVideoStream();
	if (!vs) return;
	MSWebCam *videoDevice = getVideoDevice();
	if (video_stream_started(vs) && (video_stream_get_camera(vs) != videoDevice)) {
		string currentCam = video_stream_get_camera(vs) ? ms_web_cam_get_name(video_stream_get_camera(vs)) : "NULL";
		string newCam = videoDevice ? ms_web_cam_get_name(videoDevice) : "NULL";
		lInfo() << "Switching video cam from [" << currentCam << "] to [" << newCam << "]";
		video_stream_change_camera(vs, videoDevice);
	}
}

void MS2VideoControl::setDeviceRotation(int rotation) {
	VideoStream *vs = getVideoStream();
	if (vs) video_stream_set_device_rotation(vs, rotation);
}

void MS2VideoControl::getRecvStats(VideoStats *s) const {
	VideoStream *vs = getVideoStream();
	if (vs) {
		s->fps = video_stream_get_received_framerate(vs);
		MSVideoSize vsize = video_stream_get_received_video_size(vs);
		s->width = vsize.width;
		s->height = vsize.height;
	} else {
		s->fps = 0.0;
		s->width = s->height = 0;
	}
}

void MS2VideoControl::getSendStats(VideoStats *s) const {
	VideoStream *vs = getVideoStream();
	if (vs) {
		s->fps = video_stream_get_sent_framerate(vs);
		MSVideoSize vsize = video_stream_get_sent_video_size(vs);
		s->width = vsize.width;
		s->height = vsize.height;
	} else {
		s->fps = 0.0;
		s->width = s->height = 0;
	}
}

void MS2VideoControl::sendVfu() {
	VideoStream *vs = getVideoStream();
	if (vs) video_stream_send_vfu(vs);
}

void MS2VideoControl::sendVfuRequest() {
	VideoStream *vs = getVideoStream();
	if (vs) video_stream_send_fir(vs);
}

void MS2VideoControl::zoomVideo(float zoomFactor, float cx, float cy) {
	VideoStream *vs = getVideoStream();
	if (vs && vs->output) {
		if (zoomFactor < 1) zoomFactor = 1;
		float halfsize = 0.5f * 1.0f / zoomFactor;
		if ((cx - halfsize) < 0) cx = 0 + halfsize;
		if ((cx + halfsize) > 1) cx = 1 - halfsize;
		if ((cy - halfsize) < 0) cy = 0 + halfsize;
		if ((cy + halfsize) > 1) cy = 1 - halfsize;
		float zoom[3] = {zoomFactor, cx, cy};
		ms_filter_call_method(vs->output, MS_VIDEO_DISPLAY_ZOOM, &zoom);
	} else lWarning() << "Could not apply zoom: video output wasn't activated";
}

LINPHONE_END_NAMESPACE
