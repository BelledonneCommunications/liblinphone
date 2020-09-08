/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#include "bctoolbox/defs.h"

#include "ms2-streams.h"
#include "mixers.h"

#include "media-session.h"
#include "media-session-p.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "conference/participant.h"
#include "conference/params/media-session-params-p.h"

#include "mediastreamer2/msjpegwriter.h"
#include "mediastreamer2/msogl.h"

#include "linphone/core.h"

using namespace::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * MS2VideoStream implemenation
 */

MS2VideoStream::MS2VideoStream(StreamsGroup &sg, const OfferAnswerContext &params) : MS2Stream(sg, params), MS2VideoControl(sg.getCore()) {
	string bindIp = getBindIp();
	mStream = video_stream_new2(getCCore()->factory, bindIp.empty() ? nullptr : bindIp.c_str(), mPortConfig.rtpPort, mPortConfig.rtcpPort);
	initializeSessions(&mStream->ms);
}

void MS2VideoStream::sVideoStreamEventCb (void *userData, const MSFilter *f, const unsigned int eventId, const void *args) {
	MS2VideoStream *zis = static_cast<MS2VideoStream*>(userData);
	zis->videoStreamEventCb(f, eventId, args);
}

void MS2VideoStream::videoStreamEventCb (const MSFilter *f, const unsigned int eventId, const void *args) {
	CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
	
	switch (eventId) {
		case MS_VIDEO_DECODER_DECODING_ERRORS:
			lWarning() << "MS_VIDEO_DECODER_DECODING_ERRORS";
			if (!media_stream_avpf_enabled(&mStream->ms)){
				if (mStream && video_stream_is_decoding_error_to_be_reported(mStream, 5000)) {
					video_stream_decoding_error_reported(mStream);
					getMediaSession().sendVfuRequest();
				}
			}else{
				/* Decoders are not expected to throw MS_VIDEO_DECODER_DECODING_ERRORS if AVPF is enabled */
			}
			break;
		case MS_VIDEO_DECODER_RECOVERED_FROM_ERRORS:
			lInfo() << "MS_VIDEO_DECODER_RECOVERED_FROM_ERRORS";
			if (mStream)
				video_stream_decoding_error_recovered(mStream);
			break;
		case MS_VIDEO_DECODER_FIRST_IMAGE_DECODED:
			lInfo() << "First video frame decoded successfully";
			if (listener)
				listener->onFirstVideoFrameDecoded(getMediaSession().getSharedFromThis());
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

void MS2VideoStream::sCameraNotWorkingCb (void *userData, const MSWebCam *oldWebcam) {
	MS2VideoStream *msp = static_cast<MS2VideoStream *>(userData);
	msp->cameraNotWorkingCb(oldWebcam->name);
}

void MS2VideoStream::cameraNotWorkingCb (const char *cameraName) {
	CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();

	if (listener) {
		listener->onCameraNotWorking(getMediaSession().getSharedFromThis(), cameraName);
	}
}

MediaStream *MS2VideoStream::getMediaStream()const{
	if(mStream)
		return &mStream->ms;
	else
		return nullptr;
}

MSWebCam * MS2VideoStream::getVideoDevice(CallSession::State targetState) const {
	bool paused = (targetState == CallSession::State::Pausing) || (targetState == CallSession::State::Paused);
	if (paused || mMuted || !mCameraEnabled)
		return ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(getCCore()->factory),
			"StaticImage: Static picture");
	else
		return getCCore()->video_conf.device;
}

MSWebCam *MS2VideoStream::getVideoDevice()const{
	return getVideoDevice(getGroup().getCurrentSessionState());
	
}

void MS2VideoStream::activateZrtp(){
	if (linphone_core_media_encryption_supported(getCCore(), LinphoneMediaEncryptionZRTP)){
		Stream *audioStream = getGroup().lookupMainStream(SalAudio);
		if (audioStream){
			MS2AudioStream *msa = dynamic_cast<MS2AudioStream*>(audioStream);
			video_stream_enable_zrtp(mStream, (AudioStream*)msa->getMediaStream());
			// Since the zrtp session is now initialized, make sure it is retained for future use.
			media_stream_reclaim_sessions((MediaStream*)mStream, &mSessions);
			video_stream_start_zrtp(mStream);
		}else{
			lError() << "Error while enabling zrtp on video stream: the audio stream isn't known. This is unsupported.";
		}
	}
}


bool MS2VideoStream::prepare(){
	
	MS2Stream::prepare();
	video_stream_prepare_video(mStream);
	return false;
}

void MS2VideoStream::finishPrepare(){
	MS2Stream::finishPrepare();
	video_stream_unprepare_video(mStream);
}

void MS2VideoStream::render(const OfferAnswerContext & ctx, CallSession::State targetState){
	bool reusedPreview = false;
	CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
	
	/* Shutdown preview */
	MSFilter *source = nullptr;
	if (getCCore()->previewstream) {
		if (getCCore()->video_conf.reuse_preview_source)
			source = video_preview_stop_reuse_source(getCCore()->previewstream);
		else
			video_preview_stop(getCCore()->previewstream);
		getCCore()->previewstream = nullptr;
	}
	const SalStreamDescription *vstream = ctx.resultStreamDescription;
	
	bool basicChangesHandled = handleBasicChanges(ctx, targetState);
	
	if (basicChangesHandled) {
		bool muted = mMuted;
		if (getState() == Running) {
			MS2Stream::render(ctx, targetState); // MS2Stream::render() may decide to unmute.
			if (muted && !mMuted) {
				lInfo() << "Early media finished, unmuting video input...";
				/* We were in early media, now we want to enable real media */
				mMuted = false;
				enableCamera(mCameraEnabled);
			}
		}
		return;
	}

	int usedPt = -1;
	RtpProfile *videoProfile = makeProfile(ctx.resultMediaDescription, vstream, &usedPt);
	if (usedPt == -1){
		lError() << "No payload types accepted for video stream !";
		stop();
		return;
	}
	
	
	video_stream_enable_display_filter_auto_rotate(mStream,
		!!linphone_config_get_int(linphone_core_get_config(getCCore()), "video", "display_filter_auto_rotate", 0)
	);

	const char *displayFilter = linphone_core_get_video_display_filter(getCCore());
	if (displayFilter)
		video_stream_set_display_filter_name(mStream, displayFilter);
	video_stream_set_event_callback(mStream, sVideoStreamEventCb, this);
	video_stream_set_camera_not_working_callback(mStream, sCameraNotWorkingCb, this);
	if (isMain()){
		getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedVideoCodec(rtp_profile_get_payload(videoProfile, usedPt));
	}

	if (getCCore()->video_conf.preview_vsize.width != 0)
		video_stream_set_preview_size(mStream, getCCore()->video_conf.preview_vsize);
	video_stream_set_fps(mStream, linphone_core_get_preferred_framerate(getCCore()));
	if (linphone_config_get_int(linphone_core_get_config(getCCore()), "video", "nowebcam_uses_normal_fps", 0))
		mStream->staticimage_webcam_fps_optimization = false;
	const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(getCCore());
	if (vdef) {
		MSVideoSize vsize;
		vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
		vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
		video_stream_set_sent_video_size(mStream, vsize);
	}
	video_stream_enable_self_view(mStream, getCCore()->video_conf.selfview);
	if (mNativeWindowId)
		video_stream_set_native_window_id(mStream, mNativeWindowId);
	else if (getCCore()->video_window_id)
		video_stream_set_native_window_id(mStream, getCCore()->video_window_id);
	if (getCCore()->preview_window_id)
		video_stream_set_native_preview_window_id(mStream, getCCore()->preview_window_id);
	video_stream_use_preview_video_window(mStream, getCCore()->use_preview_window);
	
	MS2Stream::render(ctx, targetState);
	
	RtpAddressInfo dest;
	getRtpDestination(ctx, &dest);
	MediaStreamDir dir = MediaStreamSendRecv;
		
	if ((vstream->dir == SalStreamSendOnly) && getCCore()->video_conf.capture)
		dir = MediaStreamSendOnly;
	else if ((vstream->dir == SalStreamRecvOnly) && getCCore()->video_conf.display)
		dir = MediaStreamRecvOnly;
	else if (vstream->dir == SalStreamSendRecv) {
		if (getCCore()->video_conf.display && getCCore()->video_conf.capture)
			dir = MediaStreamSendRecv;
		else if (getCCore()->video_conf.display)
			dir = MediaStreamRecvOnly;
		else
			dir = MediaStreamSendOnly;
	}else {
		lWarning() << "Video stream is inactive";
		/* Either inactive or incompatible with local capabilities */
		stop();
		return;
	}
	if (vstream->multicast_role == SalMulticastReceiver){
			dir = MediaStreamRecvOnly;
	}else if (vstream->multicast_role == SalMulticastSender){
		dir = MediaStreamSendOnly;
	}
	
	MSWebCam *cam = getVideoDevice(targetState);
	MS2VideoMixer * videoMixer = getVideoMixer();
	
	getMediaSession().getLog()->video_enabled = true;
	media_stream_set_direction(&mStream->ms, dir);
	lInfo() << "Device rotation =" << getCCore()->device_rotation;
	video_stream_set_device_rotation(mStream, getCCore()->device_rotation);
	video_stream_set_freeze_on_error(mStream, !!linphone_config_get_int(linphone_core_get_config(getCCore()), "video", "freeze_on_error", 1));
	video_stream_use_video_preset(mStream, linphone_config_get_string(linphone_core_get_config(getCCore()), "video", "preset", nullptr));
	if (getCCore()->video_conf.reuse_preview_source && source) {
		lInfo() << "video_stream_start_with_source kept: " << source;
		video_stream_start_with_source(mStream, videoProfile, dest.rtpAddr.c_str(), dest.rtpPort, dest.rtcpAddr.c_str(),
			dest.rtcpPort,
			usedPt, -1, cam, source);
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
			io.input.type = (videoMixer == nullptr) ? MSResourceCamera : MSResourceVoid;
			io.input.camera = cam;
			io.output.type = (videoMixer == nullptr) ? MSResourceDefault : MSResourceVoid;
		}
		if (ok) {
			AudioStream *as = getPeerAudioStream();
			if (as) audio_stream_link_video(as, mStream);
			video_stream_start_from_io(mStream, videoProfile, dest.rtpAddr.c_str(), dest.rtpPort, dest.rtcpAddr.c_str(), dest.rtcpPort,
				usedPt, &io);
		}
	}
	mStartCount++;

	if (listener)
		listener->onResetFirstVideoFrameDecoded(getMediaSession().getSharedFromThis());
	/* Start ZRTP engine if needed : set here or remote have a zrtp-hash attribute */
	const SalStreamDescription *remoteStream = ctx.remoteStreamDescription;
	if ((getMediaSessionPrivate().getParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP) || (remoteStream->haveZrtpHash == 1)) {
		Stream *audioStream = getGroup().lookupMainStream(SalAudio);
		/* Audio stream is already encrypted and video stream is active */
		if (audioStream && audioStream->isEncrypted()) {
			activateZrtp();
			if (remoteStream->haveZrtpHash == 1) {
				int retval = ms_zrtp_setPeerHelloHash(mSessions.zrtp_context, (uint8_t *)remoteStream->zrtphash, strlen((const char *)(remoteStream->zrtphash)));
				if (retval != 0)
					lError() << "Video stream ZRTP hash mismatch 0x" << hex << retval;
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
	if (videoMixer){
		mConferenceEndpoint = ms_video_endpoint_get_from_stream(mStream, TRUE);
		videoMixer->connectEndpoint(this, mConferenceEndpoint, (vstream->dir == SalStreamRecvOnly));
	}
}

void MS2VideoStream::stop(){
	MS2Stream::stop();
	AudioStream *as = getPeerAudioStream();
	if (as) audio_stream_unlink_video(as, mStream);
	
	if (mConferenceEndpoint){
		// First disconnect from the mixer before stopping the stream.
		getVideoMixer()->disconnectEndpoint(this, mConferenceEndpoint);
		ms_video_endpoint_release_from_stream(mConferenceEndpoint);
		mConferenceEndpoint = nullptr;
	}
	video_stream_stop(mStream);
	/* In mediastreamer2, stop actually stops and destroys. We immediately need to recreate the stream object for later use, keeping the 
	 * sessions (for RTP, SRTP, ZRTP etc) that were setup at the beginning. */
	mStream = video_stream_new_with_sessions(getCCore()->factory, &mSessions);
	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedVideoCodec(nullptr);
}

void MS2VideoStream::handleEvent(const OrtpEvent *ev){
	OrtpEventType evt = ortp_event_get_type(ev);
	OrtpEventData *evd = ortp_event_get_data(const_cast<OrtpEvent*>(ev));
	
	if (evt == ORTP_EVENT_NEW_VIDEO_BANDWIDTH_ESTIMATION_AVAILABLE) {
		lInfo() << "Video bandwidth estimation is " << (int)(evd->info.video_bandwidth_available / 1000.) << " kbit/s";
		if (isMain())
			linphone_call_stats_set_estimated_download_bandwidth(mStats, (float)(evd->info.video_bandwidth_available*1e-3));
	}
}

void MS2VideoStream::zrtpStarted(Stream *mainZrtpStream){
	if (getState() == Running){
		lInfo() << "Trying to start ZRTP encryption on video stream";
		activateZrtp();
		if (getMediaSessionPrivate().isEncryptionMandatory()) {
			/* Nothing could have been sent yet so generating key frame */
			video_stream_send_vfu(mStream);
		}
	}
}

void MS2VideoStream::tryEarlyMediaForking(const OfferAnswerContext &ctx){
	MS2Stream::tryEarlyMediaForking(ctx);
	sendVfu();
}

void MS2VideoStream::oglRender(){
	if (mStream && mStream->output && (ms_filter_get_id(mStream->output) == MS_OGL_ID))
		ms_filter_call_method(mStream->output, MS_OGL_RENDER, nullptr);
}

AudioStream *MS2VideoStream::getPeerAudioStream(){
	MS2AudioStream *as = getGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
	return as ? (AudioStream*)as->getMediaStream() : nullptr;
}

void MS2VideoStream::onSnapshotTaken(const string &filepath) {
	CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
	listener->onSnapshotTaken(getMediaSession().getSharedFromThis(), filepath.c_str());
}

void MS2VideoStream::finish(){
	if (mStream) {
		video_stream_stop(mStream);
		mStream = nullptr;
	}
	MS2Stream::finish();
}

VideoStream *MS2VideoStream::getVideoStream()const{
	return (VideoStream*)mStream;
}

MS2VideoMixer *MS2VideoStream::getVideoMixer(){
	StreamMixer *mixer = getMixer();
	if (mixer){
		MS2VideoMixer * videoMixer = dynamic_cast<MS2VideoMixer*>(mixer);
		if (!videoMixer){
			lError() << *this << " does not have a mixer it is able to interface with.";
		}
		return videoMixer;
	}
	return nullptr;
}

MS2VideoStream::~MS2VideoStream(){
	if (mStream) video_stream_stop(mStream);
}

/*
 * Here comes the utility class that implements usual controls on top of mediastreamer2's VideoStream.
 */

MS2VideoControl::MS2VideoControl(Core &core) : mCore(core){
}

bool MS2VideoControl::cameraEnabled() const{
	return mCameraEnabled;
}

void MS2VideoControl::setNativeWindowId(void *w){
	VideoStream *vs = getVideoStream();
	mNativeWindowId = w;
	if (vs) video_stream_set_native_window_id(vs, w);
}

void * MS2VideoControl::getNativeWindowId() const{
	VideoStream *vs = getVideoStream();
	if (mNativeWindowId){
		return mNativeWindowId;
	}
	/* It was not set but we want to get the one automatically created by mediastreamer2 (desktop versions only) */
	return vs ? video_stream_get_native_window_id(vs) : nullptr;
}

void MS2VideoControl::setNativePreviewWindowId(void *w){
	VideoStream *vs = getVideoStream();
	mNativePreviewWindowId = w;
	if (vs) video_stream_set_native_preview_window_id(vs, w);
}

void * MS2VideoControl::getNativePreviewWindowId() const{
	return mNativePreviewWindowId;
}

void MS2VideoControl::requestNotifyNextVideoFrameDecoded () {
	VideoStream *vs = getVideoStream();
	if (vs && vs->ms.decoder)
		ms_filter_call_method_noarg(vs->ms.decoder, MS_VIDEO_DECODER_RESET_FIRST_IMAGE_NOTIFICATION);
}

void MS2VideoControl::sSnapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg) {
	MS2VideoControl *d = (MS2VideoControl *)userdata;
	if (id == MS_JPEG_WRITER_SNAPSHOT_TAKEN) {
		const char *filepath = (const char *) arg;
		d->onSnapshotTaken(filepath);
	}
}

int MS2VideoControl::takePreviewSnapshot (const string& file) {
	VideoStream *vs = getVideoStream();
	if (vs && vs->local_jpegwriter) {
		ms_filter_clear_notify_callback(vs->jpegwriter);
		const char *filepath = file.empty() ? nullptr : file.c_str();
		ms_filter_add_notify_callback(vs->local_jpegwriter, sSnapshotTakenCb, this, TRUE);
		return ms_filter_call_method(vs->local_jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void *)filepath);
	}
	lWarning() << "Cannot take local snapshot: no currently running video stream on this call";
	return -1;
}

int MS2VideoControl::takeVideoSnapshot (const string& file) {
	VideoStream *vs = getVideoStream();
	if (vs && vs->jpegwriter) {
		ms_filter_clear_notify_callback(vs->jpegwriter);
		const char *filepath = file.empty() ? nullptr : file.c_str();
		ms_filter_add_notify_callback(vs->jpegwriter, sSnapshotTakenCb, this, TRUE);
		return ms_filter_call_method(vs->jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void *)filepath);
	}
	lWarning() << "Cannot take snapshot: no currently running video stream on this call";
	return -1;
}


void MS2VideoControl::parametersChanged(){
	VideoStream *vs = getVideoStream();
	if (!vs) return;
	const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(mCore.getCCore());
	MSVideoSize vsize;
	vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
	vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
	video_stream_set_sent_video_size(vs, vsize);
	video_stream_set_fps(vs, linphone_core_get_preferred_framerate(mCore.getCCore()));
	if (mCameraEnabled && (vs->cam != mCore.getCCore()->video_conf.device))
		video_stream_change_camera(vs, mCore.getCCore()->video_conf.device);
	else
		video_stream_update_video_params(vs);
}

void MS2VideoControl::enableCamera(bool value){
	VideoStream *vs = getVideoStream();
	mCameraEnabled = value;
	if (!vs) return;
	MSWebCam *videoDevice = getVideoDevice();
	if (video_stream_started(vs) && (video_stream_get_camera(vs) != videoDevice)) {
		string currentCam = video_stream_get_camera(vs) ? ms_web_cam_get_name(video_stream_get_camera(vs)) : "NULL";
		string newCam = videoDevice ? ms_web_cam_get_name(videoDevice) : "NULL";
		lInfo() << "Switching video cam from [" << currentCam << "] to [" << newCam << "]";
		video_stream_change_camera(vs, videoDevice);
	}
}

void MS2VideoControl::setDeviceRotation(int rotation){
	VideoStream *vs = getVideoStream();
	if (vs) video_stream_set_device_rotation(vs, rotation);
}

void MS2VideoControl::getRecvStats(VideoStats *s) const{
	VideoStream *vs = getVideoStream();
	if (vs){
		s->fps = video_stream_get_received_framerate(vs);
		MSVideoSize vsize = video_stream_get_received_video_size(vs);
		s->width = vsize.width;
		s->height = vsize.height;
	}else{
		s->fps = 0.0;
		s->width = s->height = 0;
	}
}

void MS2VideoControl::getSendStats(VideoStats *s) const{
	VideoStream *vs = getVideoStream();
	if (vs){
		s->fps = video_stream_get_sent_framerate(vs);
		MSVideoSize vsize = video_stream_get_sent_video_size(vs);
		s->width = vsize.width;
		s->height = vsize.height;
	}else{
		s->fps = 0.0;
		s->width = s->height = 0;
	}
}

void MS2VideoControl::sendVfu(){
	VideoStream *vs = getVideoStream();
	if (vs) video_stream_send_vfu(vs);
}

void MS2VideoControl::sendVfuRequest(){
	VideoStream *vs = getVideoStream();
	if (vs) video_stream_send_fir(vs);
}

void MS2VideoControl::zoomVideo (float zoomFactor, float cx, float cy){
	VideoStream *vs = getVideoStream();
	if (vs && vs->output) {
		if (zoomFactor < 1)
			zoomFactor = 1;
		float halfsize = 0.5f * 1.0f / zoomFactor;
		if ((cx - halfsize) < 0)
			cx = 0 + halfsize;
		if ((cx + halfsize) > 1)
			cx = 1 - halfsize;
		if ((cy - halfsize) < 0)
			cy = 0 + halfsize;
		if ((cy + halfsize) > 1)
			cy = 1 - halfsize;
		float zoom[3] = { zoomFactor, cx, cy };
		ms_filter_call_method(vs->output, MS_VIDEO_DISPLAY_ZOOM, &zoom);
	} else
		lWarning() << "Could not apply zoom: video output wasn't activated";
}

LINPHONE_END_NAMESPACE



