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

#include "streams.h"
#include "media-session.h"
#include "media-session-p.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "call/call-p.h"
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

MS2VideoStream::MS2VideoStream(StreamsGroup &sg, const OfferAnswerContext &params) : MS2Stream(sg, params){
	mStream = video_stream_new2(getCCore()->factory, getBindIp().c_str(), mPortConfig.rtpPort, mPortConfig.rtcpPort);
	initializeSessions(&mStream->ms);
	
	video_stream_enable_display_filter_auto_rotate(mStream,
		!!lp_config_get_int(linphone_core_get_config(getCCore()), "video", "display_filter_auto_rotate", 0)
	);

	const char *displayFilter = linphone_core_get_video_display_filter(getCCore());
	if (displayFilter)
		video_stream_set_display_filter_name(mStream, displayFilter);
	video_stream_set_event_callback(mStream, sVideoStreamEventCb, this);
	
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
			if (mStream && video_stream_is_decoding_error_to_be_reported(mStream, 5000)) {
				video_stream_decoding_error_reported(mStream);
				sendVfu();
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

MediaStream *MS2VideoStream::getMediaStream()const{
	return &mStream->ms;
}

void MS2VideoStream::sendVfu(){
	video_stream_send_vfu(mStream);
}

void MS2VideoStream::sendVfuRequest(){
	video_stream_send_fir(mStream);
}

void MS2VideoStream::zoomVideo (float zoomFactor, float cx, float cy){
	if (mStream->output) {
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
		ms_filter_call_method(mStream->output, MS_VIDEO_DISPLAY_ZOOM, &zoom);
	} else
		lWarning() << "Could not apply zoom: video output wasn't activated";
}

void MS2VideoStream::parametersChanged(){
	if (getState() != Stream::Running) return;
	const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(getCCore());
	MSVideoSize vsize;
	vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
	vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
	video_stream_set_sent_video_size(mStream, vsize);
	video_stream_set_fps(mStream, linphone_core_get_preferred_framerate(getCCore()));
	if (mCameraEnabled && (mStream->cam != getCCore()->video_conf.device))
		video_stream_change_camera(mStream, getCCore()->video_conf.device);
	else
		video_stream_update_video_params(mStream);
}

void MS2VideoStream::setNativeWindowId(void *w){
	mNativeWindowId = w;
	video_stream_set_native_window_id(mStream, w);
}

void * MS2VideoStream::getNativeWindowId() const{
	if (mNativeWindowId){
		return mNativeWindowId;
	}
	/* It was not set but we want to get the one automatically created by mediastreamer2 (desktop versions only) */
	return video_stream_get_native_window_id(mStream);
}

void MS2VideoStream::setNativePreviewWindowId(void *w){
	mNativePreviewWindowId = w;
	video_stream_set_native_preview_window_id(mStream, w);
}

void * MS2VideoStream::getNativePreviewWindowId() const{
	return mNativePreviewWindowId;
}

void MS2VideoStream::enableCamera(bool value){
	mCameraEnabled = value;
	MSWebCam *videoDevice = getVideoDevice(getMediaSession().getState());
	if (video_stream_started(mStream) && (video_stream_get_camera(mStream) != videoDevice)) {
		string currentCam = video_stream_get_camera(mStream) ? ms_web_cam_get_name(video_stream_get_camera(mStream)) : "NULL";
		string newCam = videoDevice ? ms_web_cam_get_name(videoDevice) : "NULL";
		lInfo() << "Switching video cam from [" << currentCam << "] to [" << newCam << "]";
		video_stream_change_camera(mStream, videoDevice);
	}
}

MSWebCam * MS2VideoStream::getVideoDevice(CallSession::State targetState) const {
	bool paused = (targetState == CallSession::State::Pausing) || (targetState == CallSession::State::Paused);
	if (paused || mMuted || !mCameraEnabled)
#ifdef VIDEO_ENABLED
		return ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(getCCore()->factory),
			"StaticImage: Static picture");
#else
		return nullptr;
#endif
	else
		return getCCore()->video_conf.device;
}


void MS2VideoStream::prepare(){
	if (linphone_core_media_encryption_supported(getCCore(), LinphoneMediaEncryptionZRTP)){
		Stream *audioStream = getGroup().lookupMainStream(SalAudio);
		if (audioStream){
			MS2AudioStream *msa = dynamic_cast<MS2AudioStream*>(audioStream);
			video_stream_enable_zrtp(mStream, (AudioStream*)msa->getMediaStream());
		}else{
			lError() << "Error while enabling zrtp on video stream: the audio stream isn't known. This is unsupported.";
		}
	}
	MS2Stream::prepare();
	video_stream_prepare_video(mStream);
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
	
	if (vstream->dir == SalStreamInactive || vstream->rtp_port == 0){
		stop();
		return;
	}

	int usedPt = -1;
	RtpProfile *videoProfile = makeProfile(ctx.resultMediaDescription, vstream, &usedPt);
	if (usedPt == -1){
		lError() << "No payload types accepted for video stream !";
		stop();
		return;
	}
	
	if (mMuted && targetState == CallSession::State::StreamsRunning){
		lInfo() << "Early media finished, unmuting video input...";
		/* We were in early media, now we want to enable real media */
		mMuted = false;
		enableCamera(mCameraEnabled);
	}

	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedVideoCodec(rtp_profile_get_payload(videoProfile, usedPt));
	getMediaSessionPrivate().getCurrentParams()->enableVideo(true);

	if (getCCore()->video_conf.preview_vsize.width != 0)
		video_stream_set_preview_size(mStream, getCCore()->video_conf.preview_vsize);
	video_stream_set_fps(mStream, linphone_core_get_preferred_framerate(getCCore()));
	if (lp_config_get_int(linphone_core_get_config(getCCore()), "video", "nowebcam_uses_normal_fps", 0))
		mStream->staticimage_webcam_fps_optimization = false;
	const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(getCCore());
	MSVideoSize vsize;
	vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
	vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
	video_stream_set_sent_video_size(mStream, vsize);
	video_stream_enable_self_view(mStream, getCCore()->video_conf.selfview);
	if (mNativeWindowId)
		video_stream_set_native_window_id(mStream, mNativeWindowId);
	else if (getCCore()->video_window_id)
		video_stream_set_native_window_id(mStream, getCCore()->video_window_id);
	if (getCCore()->preview_window_id)
		video_stream_set_native_preview_window_id(mStream, getCCore()->preview_window_id);
	video_stream_use_preview_video_window(mStream, getCCore()->use_preview_window);
	const char *rtpAddr = (vstream->rtp_addr[0] != '\0') ? vstream->rtp_addr : ctx.resultMediaDescription->addr;
	const char *rtcpAddr = (vstream->rtcp_addr[0] != '\0') ? vstream->rtcp_addr : ctx.resultMediaDescription->addr;
	bool isMulticast = !!ms_is_multicast(rtpAddr);
	MediaStreamDir dir = MediaStreamSendRecv;
	if (isMulticast) {
		if (vstream->multicast_role == SalMulticastReceiver)
			dir = MediaStreamRecvOnly;
		else
			dir = MediaStreamSendOnly;
	} else if ((vstream->dir == SalStreamSendOnly) && getCCore()->video_conf.capture)
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
	} else {
		lWarning() << "Video stream is inactive";
		/* Either inactive or incompatible with local capabilities */
		stop();
		return;
	}
	MSWebCam *cam = getVideoDevice(targetState);
	MS2Stream::render(ctx, targetState);
	
	getMediaSession().getLog()->video_enabled = true;
	video_stream_set_direction(mStream, dir);
	lInfo() << "Device rotation =" << getCCore()->device_rotation;
	video_stream_set_device_rotation(mStream, getCCore()->device_rotation);
	video_stream_set_freeze_on_error(mStream, !!lp_config_get_int(linphone_core_get_config(getCCore()), "video", "freeze_on_error", 1));
	video_stream_use_video_preset(mStream, lp_config_get_string(linphone_core_get_config(getCCore()), "video", "preset", nullptr));
	if (getCCore()->video_conf.reuse_preview_source && source) {
		lInfo() << "video_stream_start_with_source kept: " << source;
		video_stream_start_with_source(mStream, videoProfile, rtpAddr, vstream->rtp_port, rtcpAddr,
			linphone_core_rtcp_enabled(getCCore()) ? (vstream->rtcp_port ? vstream->rtcp_port : vstream->rtp_port + 1) : 0,
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
			io.input.type = MSResourceCamera;
			io.input.camera = cam;
			io.output.type = MSResourceDefault;
		}
		if (ok) {
			AudioStream *as = getPeerAudioStream();
			if (as) audio_stream_link_video(as, mStream);
			video_stream_start_from_io(mStream, videoProfile, rtpAddr, vstream->rtp_port, rtcpAddr,
				(linphone_core_rtcp_enabled(getCCore()) && !isMulticast)  ? (vstream->rtcp_port ? vstream->rtcp_port : vstream->rtp_port + 1) : 0,
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
			video_stream_start_zrtp(mStream);
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
	
}

void MS2VideoStream::stop(){
	MS2Stream::stop();
	AudioStream *as = getPeerAudioStream();
	if (as) audio_stream_link_video(as, nullptr);
	video_stream_stop(mStream);
	/* In mediastreamer2, stop actually stops and destroys. We immediately need to recreate the stream object for later use, keeping the 
	 * sessions (for RTP, SRTP, ZRTP etc) that were setup at the beginning. */
	mStream = video_stream_new_with_sessions(getCCore()->factory, &mSessions);
	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedAudioCodec(nullptr);
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
		video_stream_start_zrtp(mStream);
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
	if (mStream->output && (ms_filter_get_id(mStream->output) == MS_OGL_ID))
		ms_filter_call_method(mStream->output, MS_OGL_RENDER, nullptr);
}

AudioStream *MS2VideoStream::getPeerAudioStream(){
	MS2AudioStream *as = getGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
	return as ? (AudioStream*)as->getMediaStream() : nullptr;
}

void MS2VideoStream::requestNotifyNextVideoFrameDecoded () {
	if (mStream->ms.decoder)
		ms_filter_call_method_noarg(mStream->ms.decoder, MS_VIDEO_DECODER_RESET_FIRST_IMAGE_NOTIFICATION);
}


void MS2VideoStream::snapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg) {
	if (id == MS_JPEG_WRITER_SNAPSHOT_TAKEN) {
		CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
		const char *filepath = (const char *) arg;
		listener->onSnapshotTaken(getMediaSession().getSharedFromThis(), filepath);
	}
}

void MS2VideoStream::sSnapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg) {
	MS2VideoStream *d = (MS2VideoStream *)userdata;
	d->snapshotTakenCb(userdata, f, id, arg);
}

int MS2VideoStream::takePreviewSnapshot (const string& file) {
	if (mStream && mStream->local_jpegwriter) {
		ms_filter_clear_notify_callback(mStream->jpegwriter);
		const char *filepath = file.empty() ? nullptr : file.c_str();
		ms_filter_add_notify_callback(mStream->local_jpegwriter, sSnapshotTakenCb, this, TRUE);
		return ms_filter_call_method(mStream->local_jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void *)filepath);
	}
	lWarning() << "Cannot take local snapshot: no currently running video stream on this call";
	return -1;
}

int MS2VideoStream::takeVideoSnapshot (const string& file) {
	if (mStream && mStream->jpegwriter) {
		ms_filter_clear_notify_callback(mStream->jpegwriter);
		const char *filepath = file.empty() ? nullptr : file.c_str();
		ms_filter_add_notify_callback(mStream->jpegwriter, sSnapshotTakenCb, this, TRUE);
		return ms_filter_call_method(mStream->jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void *)filepath);
	}
	lWarning() << "Cannot take snapshot: no currently running video stream on this call";
	return -1;
}

bool MS2VideoStream::cameraEnabled() const{
	return mCameraEnabled;
}

void MS2VideoStream::getRecvStats(VideoStats *s) const{
	s->fps = video_stream_get_received_framerate(mStream);
	MSVideoSize vsize = video_stream_get_received_video_size(mStream);
	s->width = vsize.width;
	s->height = vsize.height;
}

void MS2VideoStream::getSendStats(VideoStats *s) const{
	s->fps = video_stream_get_sent_framerate(mStream);
	MSVideoSize vsize = video_stream_get_sent_video_size(mStream);
	s->width = vsize.width;
	s->height = vsize.height;
}

MS2VideoStream::~MS2VideoStream(){
	video_stream_stop(mStream);
}

LINPHONE_END_NAMESPACE

