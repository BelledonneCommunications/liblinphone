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

#include <iomanip>
#include <math.h>

#include "address/address-p.h"
#include "call/call-p.h"
#include "chat/chat-room/client-group-chat-room.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant-p.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "core/core-p.h"
#include "c-wrapper/c-wrapper.h"
#include "sal/call-op.h"
#include "sal/sal.h"
#include "utils/payload-type-handler.h"

#include "logger/logger.h"

#include "linphone/core.h"

#include <bctoolbox/defs.h>
#include <mediastreamer2/mediastream.h>
#include <mediastreamer2/msequalizer.h>
#include <mediastreamer2/mseventqueue.h>
#include <mediastreamer2/msfileplayer.h>
#include <mediastreamer2/msjpegwriter.h>
#include <mediastreamer2/msogl.h>
#include <mediastreamer2/msrtt4103.h>
#include <mediastreamer2/msvolume.h>
#include <ortp/b64.h>

#include "private.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

#define STR_REASSIGN(dest, src) { \
	if (dest) \
		ms_free(dest); \
	dest = src; \
}

inline OrtpRtcpXrStatSummaryFlag operator|(OrtpRtcpXrStatSummaryFlag a, OrtpRtcpXrStatSummaryFlag b) {
	return static_cast<OrtpRtcpXrStatSummaryFlag>(static_cast<int>(a) | static_cast<int>(b));
}

// =============================================================================

const string MediaSessionPrivate::ecStateStore = ".linphone.ecstate";
const int MediaSessionPrivate::ecStateMaxLen = 1048576; /* 1Mo */

// =============================================================================


void MediaSessionPrivate::setDtlsFingerprint(const std::string &fingerPrint){
	dtlsCertificateFingerprint = fingerPrint;
}

const std::string & MediaSessionPrivate::getDtlsFingerprint()const{
	return dtlsCertificateFingerprint;
}

void MediaSessionPrivate::stunAuthRequestedCb (void *userData, const char *realm, const char *nonce, const char **username, const char **password, const char **ha1) {
	MediaSessionPrivate *msp = reinterpret_cast<MediaSessionPrivate *>(userData);
	msp->stunAuthRequestedCb(realm, nonce, username, password, ha1);
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::accepted () {
	L_Q();
	CallSessionPrivate::accepted();
	LinphoneTaskList tl;
	linphone_task_list_init(&tl);
	/* Reset the internal call update flag, so it doesn't risk to be copied and used in further re-INVITEs */
	getParams()->getPrivate()->setInternalCallUpdate(false);
	SalMediaDescription *rmd = op->getRemoteMediaDescription();
	SalMediaDescription *md = op->getFinalMediaDescription();
	if (!md && (prevState == CallSession::State::OutgoingEarlyMedia) && resultDesc) {
		lInfo() << "Using early media SDP since none was received with the 200 OK";
		md = resultDesc;
	}
	if (md && (sal_media_description_empty(md) || linphone_core_incompatible_security(q->getCore()->getCCore(), md)))
		md = nullptr;
	if (md) {
		/* There is a valid SDP in the response, either offer or answer, and we're able to start/update the streams */
		if (rmd) {
			/* Handle remote ICE attributes if any. */
			getIceAgent().updateFromRemoteMediaDescription(localDesc, rmd, !op->isOfferer());
		}
		CallSession::State nextState = CallSession::State::Idle;
		string nextStateMsg;
		switch (state) {
			case CallSession::State::Resuming:
			case CallSession::State::Connected:
				if (referer)
					notifyReferState();
				BCTBX_NO_BREAK; /* Intentional no break */
			case CallSession::State::Updating:
			case CallSession::State::UpdatedByRemote:
				if (!sal_media_description_has_dir(localDesc, SalStreamInactive)
					&& (sal_media_description_has_dir(md, SalStreamRecvOnly) || sal_media_description_has_dir(md, SalStreamInactive))) {
					nextState = CallSession::State::PausedByRemote;
					nextStateMsg = "Call paused by remote";
				} else {
					if (!getParams()->getPrivate()->getInConference() && listener)
						listener->onSetCurrentSession(q->getSharedFromThis());
					nextState = CallSession::State::StreamsRunning;
					nextStateMsg = "Streams running";
				}
				break;
			case CallSession::State::EarlyUpdating:
				nextState = prevState;
				nextStateMsg = "Early update accepted";
				break;
			case CallSession::State::Pausing:
				/* When we entered the pausing state, we always reach the paused state whatever the content of the remote SDP is.
				 * Our streams are all send-only (with music), soundcard and camera are never used. */
				nextState = CallSession::State::Paused;
				nextStateMsg = "Call paused";
				if (referPending)
					linphone_task_list_add(&tl, &MediaSessionPrivate::startPendingRefer, q);
				break;
			default:
				lError() << "accepted(): don't know what to do in state [" << Utils::toString(state) << "]";
				break;
		}

		if (nextState == CallSession::State::Idle)
			lError() << "BUG: nextState is not set in accepted(), current state is " << Utils::toString(state);
		else {
			updateRemoteSessionIdAndVer();
			getIceAgent().updateIceStateInCallStats();
			updateStreams(md, nextState);
			fixCallParams(rmd);
			setState(nextState, nextStateMsg);
		}
	} else { /* Invalid or no SDP */
		switch (prevState) {
			/* Send a bye only in case of early states */
			case CallSession::State::OutgoingInit:
			case CallSession::State::OutgoingProgress:
			case CallSession::State::OutgoingRinging:
			case CallSession::State::OutgoingEarlyMedia:
			case CallSession::State::IncomingReceived:
			case CallSession::State::IncomingEarlyMedia:
				lError() << "Incompatible SDP answer received, need to abort the call";
				abort("Incompatible, check codecs or security settings...");
				break;
			/* Otherwise we are able to resume previous state */
			default:
				lError() << "Incompatible SDP answer received";
				switch(state) {
					case CallSession::State::PausedByRemote:
					case CallSession::State::Paused:
					case CallSession::State::StreamsRunning:
						break;
					default:
						lInfo() << "Incompatible SDP answer received, restoring previous state [" << Utils::toString(prevState) << "]";
						setState(prevState, "Incompatible media parameters.");
						break;
				}
				break;
		}
	}
	linphone_task_list_run(&tl);
	linphone_task_list_free(&tl);
}

void MediaSessionPrivate::ackReceived (LinphoneHeaders *headers) {
	L_Q();
	CallSessionPrivate::ackReceived(headers);
	if (expectMediaInAck) {
		switch (state) {
			case CallSession::State::StreamsRunning:
			case CallSession::State::PausedByRemote:
				setState(CallSession::State::UpdatedByRemote, "UpdatedByRemote");
				break;
			default:
				break;
		}
		accepted();
	}
	if (linphone_core_media_encryption_supported(q->getCore()->getCCore(), LinphoneMediaEncryptionZRTP)) {
		SalMediaDescription *remote = op->getRemoteMediaDescription();
		const SalStreamDescription *remoteStream = remote?sal_media_description_find_best_stream(remote, SalAudio):NULL;
		//Start zrtp if remote has not offered it but local is configured for zrtp and not offerer
		if (remoteStream && getParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP && !op->isOfferer() && remoteStream->haveZrtpHash == 0) {
			lInfo() << "Starting zrtp late";
			startZrtpPrimaryChannel(remoteStream);
		}
	}
}

void MediaSessionPrivate::dtmfReceived (char dtmf) {
	L_Q();
	if (listener)
		listener->onDtmfReceived(q->getSharedFromThis(), dtmf);
}

bool MediaSessionPrivate::failure () {
	L_Q();
	const SalErrorInfo *ei = op->getErrorInfo();
	switch (ei->reason) {
		case SalReasonRedirect:
			stopStreams();
			break;
		case SalReasonUnsupportedContent: /* This is for compatibility: linphone sent 415 because of SDP offer answer failure */
		case SalReasonNotAcceptable:
			lInfo() << "Outgoing CallSession [" << q << "] failed with SRTP and/or AVPF enabled";
			if ((state == CallSession::State::OutgoingInit) || (state == CallSession::State::OutgoingProgress)
				|| (state == CallSession::State::OutgoingRinging) /* Push notification case */ || (state == CallSession::State::OutgoingEarlyMedia)) {
				for (int i = 0; i < localDesc->nb_streams; i++) {
					if (!sal_stream_description_active(&localDesc->streams[i]))
						continue;
					if (getParams()->getMediaEncryption() == LinphoneMediaEncryptionSRTP) {
						if (getParams()->avpfEnabled()) {
							if (i == 0)
								lInfo() << "Retrying CallSession [" << q << "] with SAVP";
							getParams()->enableAvpf(false);
							restartInvite();
							return true;
						} else if (!linphone_core_is_media_encryption_mandatory(q->getCore()->getCCore())) {
							if (i == 0)
								lInfo() << "Retrying CallSession [" << q << "] with AVP";
							getParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
							memset(localDesc->streams[i].crypto, 0, sizeof(localDesc->streams[i].crypto));
							restartInvite();
							return true;
						}
					} else if (getParams()->avpfEnabled()) {
						if (i == 0)
							lInfo() << "Retrying CallSession [" << q << "] with AVP";
						getParams()->enableAvpf(false);
						restartInvite();
						return true;
					}
				}
			}
			break;
		default:
			break;
	}

	bool stop = CallSessionPrivate::failure();
	if (stop)
		return true;

	if (referer) {
		// Schedule automatic resume of the call. This must be done only after the notifications are completed due to dialog serialization of requests
		linphone_core_queue_task(q->getCore()->getCCore(),
			&MediaSessionPrivate::resumeAfterFailedTransfer, referer.get(),
			"Automatic CallSession resuming after failed transfer");
	}

	if (listener)
		listener->onStopRingingIfNeeded(q->getSharedFromThis());
	stopStreams();
	return false;
}

void MediaSessionPrivate::pauseForTransfer () {
	L_Q();
	lInfo() << "Automatically pausing current MediaSession to accept transfer";
	q->pause();
	automaticallyPaused = true;
}

void MediaSessionPrivate::pausedByRemote () {
	L_Q();
	MediaSessionParams newParams(*getParams());
	if (lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "inactive_video_on_pause", 0))
		newParams.setVideoDirection(LinphoneMediaDirectionInactive);
	acceptUpdate(&newParams, CallSession::State::PausedByRemote, "Call paused by remote");
}

void MediaSessionPrivate::remoteRinging () {
	L_Q();
	/* Set privacy */
	getCurrentParams()->setPrivacy((LinphonePrivacyMask)op->getPrivacy());
	SalMediaDescription *md = op->getFinalMediaDescription();
	if (md) {
		SalMediaDescription *rmd = op->getRemoteMediaDescription();
		/* Initialize the remote call params by invoking linphone_call_get_remote_params(). This is useful as the SDP may not be present in the 200Ok */
		q->getRemoteParams();
		/* Accept early media */
		
		if (getStreamsGroup().isStarted()){
			getStreamsGroup().tryEarlyMediaForking(md, rmd);
			return;
		}

		setState(CallSession::State::OutgoingEarlyMedia, "Early media");
		if (listener)
			listener->onStopRinging(q->getSharedFromThis());
		lInfo() << "Doing early media...";
		getIceAgent().updateFromRemoteMediaDescription(localDesc, rmd, !op->isOfferer());
		updateStreams(md, state);
		if ((q->getCurrentParams()->getAudioDirection() == LinphoneMediaDirectionInactive)) {
			if (listener)
				listener->onStartRinging(q->getSharedFromThis());
		}
	} else {
		linphone_core_stop_dtmf_stream(q->getCore()->getCCore());
		if (state == CallSession::State::OutgoingEarlyMedia) {
			/* Already doing early media */
			return;
		}
		if (listener)
			listener->onStartRinging(q->getSharedFromThis());
		lInfo() << "Remote ringing...";
		setState(CallSession::State::OutgoingRinging, "Remote ringing");
	}
}

void MediaSessionPrivate::replaceOp (SalCallOp *newOp) {
	CallSessionPrivate::replaceOp(newOp);
	updateStreams(newOp->getFinalMediaDescription(), state);
}

int MediaSessionPrivate::resumeAfterFailedTransfer () {
	L_Q();
	if (automaticallyPaused && (state == CallSession::State::Pausing))
		return BELLE_SIP_CONTINUE; // Was still in pausing state
	if (automaticallyPaused && (state == CallSession::State::Paused)) {
		if (op->isIdle())
			q->resume();
		else {
			lInfo() << "MediaSessionPrivate::resumeAfterFailedTransfer(), op was busy";
			return BELLE_SIP_CONTINUE;
		}
	}
	return BELLE_SIP_STOP;
}

void MediaSessionPrivate::resumed () {
	acceptUpdate(nullptr, CallSession::State::StreamsRunning, "Connected (streams running)");
}

void MediaSessionPrivate::startPendingRefer () {
	L_Q();
	if (listener)
		listener->onCallSessionStartReferred(q->getSharedFromThis());
}

void MediaSessionPrivate::telephoneEventReceived (int event) {
	static char dtmfTab[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'A', 'B', 'C', 'D' };
	if ((event < 0) || (event > 15)) {
		lWarning() << "Bad dtmf value " << event;
		return;
	}
	dtmfReceived(dtmfTab[event]);
}

void MediaSessionPrivate::terminated () {
	stopStreams();
	CallSessionPrivate::terminated();
}

/* This callback is called when an incoming re-INVITE/ SIP UPDATE modifies the session */
void MediaSessionPrivate::updated (bool isUpdate) {
	SalMediaDescription *rmd = op->getRemoteMediaDescription();
	switch (state) {
		case CallSession::State::PausedByRemote:
			if (sal_media_description_has_dir(rmd, SalStreamSendRecv) || sal_media_description_has_dir(rmd, SalStreamRecvOnly)) {
				resumed();
				return;
			}
			break;
		case CallSession::State::StreamsRunning:
		case CallSession::State::Connected:
		case CallSession::State::UpdatedByRemote: /* Can happen on UAC connectivity loss */
			if (sal_media_description_has_dir(rmd, SalStreamSendOnly) || sal_media_description_has_dir(rmd, SalStreamInactive)) {
				pausedByRemote();
				return;
			}
			break;
		default:
			/* The other cases are handled in CallSessionPrivate::updated */
			break;
	}
	CallSessionPrivate::updated(isUpdate);
}



void MediaSessionPrivate::updating (bool isUpdate) {
	L_Q();
	SalMediaDescription *rmd = op->getRemoteMediaDescription();
	fixCallParams(rmd);
	if (state != CallSession::State::Paused) {
		/* Refresh the local description, but in paused state, we don't change anything. */
		if (!rmd && lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "sdp_200_ack_follow_video_policy", 0)) {
			lInfo() << "Applying default policy for offering SDP on CallSession [" << q << "]";
			setParams(new MediaSessionParams());
			params->initDefault(q->getCore());
		}
		makeLocalMediaDescription();
	}
	if (rmd) {
		SalErrorInfo sei;
		memset(&sei, 0, sizeof(sei));
		expectMediaInAck = false;
		SalMediaDescription *md = op->getFinalMediaDescription();
		if (md && (sal_media_description_empty(md) || linphone_core_incompatible_security(q->getCore()->getCCore(), md))) {
			sal_error_info_set(&sei, SalReasonNotAcceptable, "SIP", 0, nullptr, nullptr);
			op->declineWithErrorInfo(&sei, nullptr);
			sal_error_info_reset(&sei);
			return;
		}
		SalMediaDescription *prevResultDesc = resultDesc;
		if (isUpdate && prevResultDesc && md){
			int diff = sal_media_description_equals(prevResultDesc, md);
			if (diff & (SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED | SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED)) {
				lWarning() << "Cannot accept this update, it is changing parameters that require user approval";
				sal_error_info_set(&sei, SalReasonUnknown, "SIP", 504, "Cannot change the session parameters without prompting the user", nullptr);
				op->declineWithErrorInfo(&sei, nullptr);
				sal_error_info_reset(&sei);
				return;
			}
		}
		updated(isUpdate);
	} else {
		/* Case of a reINVITE or UPDATE without SDP */
		expectMediaInAck = true;
		op->accept(); /* Respond with an offer */
		/* Don't do anything else in this case, wait for the ACK to receive to notify the app */
	}
}

// -----------------------------------------------------------------------------


void MediaSessionPrivate::oglRender () {
	if (mainVideoStreamIndex != -1){
		MS2VideoStream * vs = dynamic_cast<MS2VideoStream*>(getStreamsGroup().getStream(mainVideoStreamIndex));
		if (vs) vs->oglRender();
	}
}

void MediaSessionPrivate::sendVfu () {
	getStreamsGroup().forEach<VideoControlInterface>([](VideoControlInterface *i){ i->sendVfu(); });
}

// -----------------------------------------------------------------------------

#if 0
void MediaSessionPrivate::clearIceCheckList (IceCheckList *cl) {
	if (audioStream && audioStream->ms.ice_check_list == cl)
		audioStream->ms.ice_check_list = nullptr;
	if (videoStream && videoStream->ms.ice_check_list == cl)
		videoStream->ms.ice_check_list = nullptr;
	if (textStream && textStream->ms.ice_check_list == cl)
		textStream->ms.ice_check_list = nullptr;
}

void MediaSessionPrivate::deactivateIce () {
	if (audioStream)
		audioStream->ms.ice_check_list = nullptr;
	if (videoStream)
		videoStream->ms.ice_check_list = nullptr;
	if (textStream)
		textStream->ms.ice_check_list = nullptr;
	_linphone_call_stats_set_ice_state(audioStats, LinphoneIceStateNotActivated);
	_linphone_call_stats_set_ice_state(videoStats, LinphoneIceStateNotActivated);
	_linphone_call_stats_set_ice_state(textStats, LinphoneIceStateNotActivated);
	stopStreamsForIceGathering();
}
#endif

// -----------------------------------------------------------------------------

bool MediaSessionPrivate::getSpeakerMuted () const {
	AudioControlInterface *i = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	return i ? !i->speakerEnabled() : false;
}

void MediaSessionPrivate::setSpeakerMuted (bool muted) {
	AudioControlInterface *i = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) i->enableSpeaker(!muted);
}

// -----------------------------------------------------------------------------

bool MediaSessionPrivate::getMicrophoneMuted () const {
	AudioControlInterface *i = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	return i ? !i->micEnabled() : false;
}

void MediaSessionPrivate::setMicrophoneMuted (bool muted) {
	AudioControlInterface *i = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) i->enableMic(muted);
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::setCurrentParams (MediaSessionParams *msp) {
	if (currentParams)
		delete currentParams;
	currentParams = msp;
}

void MediaSessionPrivate::setParams (MediaSessionParams *msp) {
	if (params)
		delete params;
	params = msp;
}

void MediaSessionPrivate::setRemoteParams (MediaSessionParams *msp) {
	if (remoteParams)
		delete remoteParams;
	remoteParams = msp;
}

Stream *MediaSessionPrivate::getStream(LinphoneStreamType type)const{
	switch (type) {
		case LinphoneStreamTypeAudio:
			return getStreamsGroup().lookupMainStream(SalAudio);
		case LinphoneStreamTypeVideo:
			return getStreamsGroup().lookupMainStream(SalVideo);
		case LinphoneStreamTypeText:
			return getStreamsGroup().lookupMainStream(SalText);
		case LinphoneStreamTypeUnknown:
		break;
	}
	return nullptr;
}

LinphoneCallStats * MediaSessionPrivate::getStats(LinphoneStreamType type) const {
	Stream *s = getStream(type);
	if (s) return s->getStats();
	return nullptr;
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::initializeStreams () {
	assignStreamsIndexes();
	if (mainAudioStreamIndex != -1) initializeAudioStream();
	if (mainVideoStreamIndex != -1) initializeVideoStream();
	if (mainTextStreamIndex != -1) initializeTextStream();
}

void MediaSessionPrivate::stopStreams () {
	L_Q();
	getStreamsGroup().stop();
	q->getCore()->soundcardHintCheck();
}

void MediaSessionPrivate::restartStream (SalStreamDescription *streamDesc, int streamIndex, int sdChanged, CallSession::State targetState) {
	L_Q();
	string streamTypeName = sal_stream_description_get_type_as_string(streamDesc);

	stopStream(streamDesc);

	if (streamDesc->type == SalAudio) {
		if (sdChanged & SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED) {
			lInfo() << "Media ip type has changed, destroying sessions context on CallSession [" << q << "] for " << streamTypeName << " stream";
			ms_media_stream_sessions_uninit(&sessions[mainAudioStreamIndex]);
		}

		initializeAudioStream();
	} else if (streamDesc->type == SalVideo) {
		if (sdChanged & SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED) {
			lInfo() << "Media ip type has changed, destroying sessions context on CallSession [" << q << "] for " << streamTypeName << " stream";
			ms_media_stream_sessions_uninit(&sessions[mainVideoStreamIndex]);
		}

		initializeVideoStream();
	} else if (streamDesc->type == SalText) {
		if (sdChanged & SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED) {
			lInfo() << "Media ip type has changed, destroying sessions context on CallSession [" << q << "] for " << streamTypeName << " stream";
			ms_media_stream_sessions_uninit(&sessions[mainTextStreamIndex]);
		}

		initializeTextStream();
	}


	startStream(streamDesc, streamIndex, targetState);

	if (streamDesc->type == SalAudio && audioStream) {
		if ((state == CallSession::State::Pausing) && pausedByApp && (q->getCore()->getCallCount() == 1))
			linphone_core_play_named_tone(q->getCore()->getCCore(), LinphoneToneCallOnHold);
	}

	updateStreamFrozenPayloads(streamDesc, &localDesc->streams[streamIndex]);
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {
	L_Q();
	if (mediaNetworkReachable) {
		LinphoneConfig *config = linphone_core_get_config(q->getCore()->getCCore());
		if (lp_config_get_int(config, "net", "recreate_sockets_when_network_is_up", 0))
			refreshSockets();
	} else {
		setBroken();
	}
	CallSessionPrivate::onNetworkReachable(sipNetworkReachable, mediaNetworkReachable);
}

// -----------------------------------------------------------------------------


#ifdef TEST_EXT_RENDERER
void MediaSessionPrivate::extRendererCb (void *userData, const MSPicture *local, const MSPicture *remote) {
	lInfo() << "extRendererCb, local buffer=" << local ? local->planes[0] : nullptr
		<< ", remote buffer=" << remote ? remote->planes[0] : nullptr;
}
#endif


int MediaSessionPrivate::sendDtmf (void *data, unsigned int revents) {
	MediaSession *session = reinterpret_cast<MediaSession *>(data);
	return session->getPrivate()->sendDtmf();
}

// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------

shared_ptr<Participant> MediaSessionPrivate::getMe () const {
	shared_ptr<Participant> participant = me.lock();
	if (!participant) {
		lWarning() << "Unable to get valid Participant instance";
		throw std::bad_weak_ptr();
	}
	return participant;
}

void MediaSessionPrivate::setState (CallSession::State newState, const string &message) {
	L_Q();

	// Take a ref on the session otherwise it might get destroyed during the call to setState
	shared_ptr<CallSession> sessionRef = q->getSharedFromThis();
	if ((newState != state) && (newState != CallSession::State::StreamsRunning))
		q->cancelDtmfs();
	CallSessionPrivate::setState(newState, message);
	if (listener)
		listener->onCallSessionStateChangedForReporting(q->getSharedFromThis());
	SalMediaDescription *rmd = nullptr;
	switch (newState) {
		case CallSession::State::UpdatedByRemote:
			// Handle specifically the case of an incoming ICE-concluded reINVITE
			lInfo() << "Checking for ICE reINVITE";
			rmd = op->getRemoteMediaDescription();
			if (rmd && getIceAgent().checkIceReinviteNeedsDeferedResponse(rmd)) {
				deferUpdate = true;
				deferUpdateInternal = true;
				incomingIceReinvitePending = true;
				lInfo() << "CallSession [" << q << "]: ICE reinvite received, but one or more check-lists are not completed. Response will be sent later, when ICE has completed";
			}
			break;
		default:
			break;
	}
}

// -----------------------------------------------------------------------------


int MediaSessionPrivate::getFirstStreamWithType(const SalMediaDescription *md, SalStreamType type){
	int i;
	for (i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; ++i) {
		if (md->streams[i].type == type) return i;
	}
	return -1;
}

void MediaSessionPrivate::assignStreamsIndexes(){
	int freeIndex = 0;
	if (biggestDesc) freeIndex = biggestDesc->nb_streams;
	
	/*Initialize stream indexes from potential incoming offer.*/
	SalMediaDescription *rmd = op ? op->getRemoteMediaDescription() : nullptr;
	if (rmd) assignStreamsIndexesIncoming(rmd);
	
	/*Assign indexes for our streams, if no incoming offer was received, or if new streams are requested.*/
	if (getParams()->audioEnabled() && mainAudioStreamIndex == -1){
		mainAudioStreamIndex = freeIndex++;
	}
	if (getParams()->videoEnabled() && mainVideoStreamIndex == -1){
		mainVideoStreamIndex = freeIndex++;
	}
	if (getParams()->realtimeTextEnabled() && mainTextStreamIndex == -1){
		mainTextStreamIndex = freeIndex++;
	}
}

void MediaSessionPrivate::assignStreamsIndexesIncoming(const SalMediaDescription *md) {
	if (mainAudioStreamIndex == -1){
		mainAudioStreamIndex = getFirstStreamWithType(md, SalAudio);
	}
	if (mainVideoStreamIndex == -1){
		mainVideoStreamIndex = getFirstStreamWithType(md, SalVideo);
	}
	if (mainTextStreamIndex == -1){
		mainTextStreamIndex = getFirstStreamWithType(md, SalText);
	}
	
	lInfo() << "Stream indexes from incoming offer (-1 = unassigned): mainAudioStreamIndex=" << mainAudioStreamIndex <<
		", mainVideoStreamIndex=" << mainVideoStreamIndex << ", mainTextStreamIndex=" << mainTextStreamIndex;
}

/*
 * This method needs to be called at each incoming reINVITE, in order to adjust various local parameters to what is being offered by remote:
 * - the stream indexes.
 * - the video enablement parameter according to what is offered and our local policy.
 * Fixing the params to proper values avoid request video by accident during internal call updates, pauses and resumes
 */
void MediaSessionPrivate::fixCallParams (SalMediaDescription *rmd) {
	L_Q();
	if (rmd) {
		updateBiggestDesc(rmd);
		/* Why disabling implicit_rtcp_fb ? It is a local policy choice actually. It doesn't disturb to propose it again and again
		 * even if the other end apparently doesn't support it.
		 * The following line of code is causing trouble, while for example making an audio call, then adding video.
		 * Due to the 200Ok response of the audio-only offer where no rtcp-fb attribute is present, implicit_rtcp_fb is set to
		 * false, which is then preventing it to be eventually used when video is later added to the call.
		 * I did the choice of commenting it out.
		 */
		/*params.getPrivate()->enableImplicitRtcpFb(params.getPrivate()->implicitRtcpFbEnabled() & sal_media_description_has_implicit_avpf(rmd));*/
	}
	const MediaSessionParams *rcp = q->getRemoteParams();
	if (rcp) {
		if (getParams()->audioEnabled() && !rcp->audioEnabled()) {
			lInfo() << "CallSession [" << q << "]: disabling audio in our call params because the remote doesn't want it";
			getParams()->enableAudio(false);
		}
		if (getParams()->videoEnabled() && !rcp->videoEnabled()) {
			lInfo() << "CallSession [" << q << "]: disabling video in our call params because the remote doesn't want it";
			getParams()->enableVideo(false);
		}
		if (rcp->videoEnabled() && q->getCore()->getCCore()->video_policy.automatically_accept && linphone_core_video_enabled(q->getCore()->getCCore()) && !getParams()->videoEnabled()) {
			lInfo() << "CallSession [" << q << "]: re-enabling video in our call params because the remote wants it and the policy allows to automatically accept";
			getParams()->enableVideo(true);
		}
		if (rcp->realtimeTextEnabled() && !getParams()->realtimeTextEnabled())
			getParams()->enableRealtimeText(true);
	}
}

void MediaSessionPrivate::initializeParamsAccordingToIncomingCallParams () {
	L_Q();
	CallSessionPrivate::initializeParamsAccordingToIncomingCallParams();
	getCurrentParams()->getPrivate()->setUpdateCallWhenIceCompleted(getParams()->getPrivate()->getUpdateCallWhenIceCompleted());
	getParams()->enableVideo(linphone_core_video_enabled(q->getCore()->getCCore()) && q->getCore()->getCCore()->video_policy.automatically_accept);
	SalMediaDescription *md = op->getRemoteMediaDescription();
	if (md) {
		/* It is licit to receive an INVITE without SDP, in this case WE choose the media parameters according to policy */
		setCompatibleIncomingCallParams(md);
	}
}

/**
 * Fix call parameters on incoming call to eg. enable AVPF if the incoming call propose it and it is not enabled locally.
 */
void MediaSessionPrivate::setCompatibleIncomingCallParams (SalMediaDescription *md) {
	L_Q();
	LinphoneCore *lc = q->getCore()->getCCore();
	/* Handle AVPF, SRTP and DTLS */
	getParams()->enableAvpf(!!sal_media_description_has_avpf(md));
	if (destProxy)
		getParams()->setAvpfRrInterval(static_cast<uint16_t>(linphone_proxy_config_get_avpf_rr_interval(destProxy) * 1000));
	else
		getParams()->setAvpfRrInterval(static_cast<uint16_t>(linphone_core_get_avpf_rr_interval(lc) * 1000));
	bool_t mandatory = linphone_core_is_media_encryption_mandatory(lc);
	if (sal_media_description_has_zrtp(md) && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionZRTP)) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionZRTP))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionZRTP);
	} else if (sal_media_description_has_dtls(md) && media_stream_dtls_supported()) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionDTLS))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionDTLS);
	} else if (sal_media_description_has_srtp(md) && ms_srtp_supported()) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionSRTP))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionSRTP);
	} else if (getParams()->getMediaEncryption() != LinphoneMediaEncryptionZRTP) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionNone))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
	}
	/* In case of nat64, even ipv4 addresses are reachable from v6. Should be enhanced to manage stream by stream connectivity (I.E v6 or v4) */
	/*if (!sal_media_description_has_ipv6(md)){
		lInfo() << "The remote SDP doesn't seem to offer any IPv6 connectivity, so disabling IPv6 for this call";
		af = AF_INET;
	}*/
	fixCallParams(md);
}

void MediaSessionPrivate::updateBiggestDesc (SalMediaDescription *md) {
	if (!biggestDesc || (md->nb_streams > biggestDesc->nb_streams)) {
		/* We have been offered and now are ready to proceed, or we added a new stream,
		 * store the media description to remember the mapping of streams within this call. */
		if (biggestDesc) {
			sal_media_description_unref(biggestDesc);
			biggestDesc = nullptr;
		}
		biggestDesc = sal_media_description_ref(md);
	}
}

void MediaSessionPrivate::updateRemoteSessionIdAndVer () {
	SalMediaDescription *desc = op->getRemoteMediaDescription();
	if (desc) {
		remoteSessionId = desc->session_id;
		remoteSessionVer = desc->session_ver;
	}
}

// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------


unsigned int MediaSessionPrivate::getAudioStartCount () const {
	Stream *s = getStreamsGroup().lookupMainStream(SalAudio);
	return s ? (unsigned int)s->getStartCount() : 0;
}

unsigned int MediaSessionPrivate::getVideoStartCount () const {
	Stream *s = getStreamsGroup().lookupMainStream(SalVideo);
	return s ? (unsigned int)s->getStartCount() : 0;
}

unsigned int MediaSessionPrivate::getTextStartCount () const {
	Stream *s = getStreamsGroup().lookupMainStream(SalText);
	return s ? (unsigned int)s->getStartCount() : 0;
}

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

void MediaSessionPrivate::discoverMtu (const Address &remoteAddr) {
	L_Q();
	if (q->getCore()->getCCore()->net_conf.mtu == 0) {
		/* Attempt to discover mtu */
		int mtu = ms_discover_mtu(remoteAddr.getDomain().c_str());
		if (mtu > 0) {
			ms_factory_set_mtu(q->getCore()->getCCore()->factory, mtu);
			lInfo() << "Discovered mtu is " << mtu << ", RTP payload max size is " << ms_factory_get_payload_max_size(q->getCore()->getCCore()->factory);
		}
	}
}


/**
 * Fill the local ip that routes to the internet according to the destination, or guess it by other special means.
 */
void MediaSessionPrivate::getLocalIp (const Address &remoteAddr) {
	L_Q();
	// Next, sometime, override from config
	const char *ip = linphone_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "bind_address", nullptr);
	if (ip) {
		mediaLocalIp = ip;
		return;
	}

	// If a known proxy was identified for this call, then we may have a chance to take the local ip address
	// from the socket that connects to this proxy
	if (destProxy && destProxy->op) {
		ip = destProxy->op->getLocalAddress(nullptr);
		if (ip) {
			if (strchr(ip, ':') && (af == AF_INET)) {
				// Case where we've decided to use IPv4 in selectOutgoingIpVersion(), but the signaling local ip address is IPv6.
				// We'll use the default media localip
			} else {
				lInfo() << "Found media local-ip from signaling.";
				mediaLocalIp = ip;
				return;
			}
		}
	}

	// In last resort, attempt to find the local ip that routes to destination if given as an IP address,
	// or the default route (dest is empty)
	string dest;
	if (!destProxy) {
		struct addrinfo hints;
		struct addrinfo *res = nullptr;
		string host(remoteAddr.getDomain());
		int err;

		if (host[0] == '[')
			host = host.substr(1, host.size() - 2);
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_NUMERICHOST;
		err = getaddrinfo(host.c_str(), nullptr, &hints, &res);
		if (err == 0)
			dest = host; // The remote address host part is real ip address, use it
		if (res)
			freeaddrinfo(res);
	}

	if (mediaLocalIp.empty() || needLocalIpRefresh) {
		char tmp[LINPHONE_IPADDR_SIZE];
		linphone_core_get_local_ip(q->getCore()->getCCore(), af, dest.c_str(), tmp);
		mediaLocalIp.assign(tmp);
		needLocalIpRefresh = false;
		lInfo() << "Media local ip to reach " << (dest.empty() ? "default route" : dest) << " is :" << mediaLocalIp;
	}
}

string MediaSessionPrivate::getPublicIpForStream (int streamIndex) {
	Stream *s = getStreamsGroup().getStream(streamIndex);
	if (s){
		if (!s->getPortConfig().multicastIp.empty())
			return s->getPortConfig().multicastIp;
	}
	return mediaLocalIp;
}

void MediaSessionPrivate::runStunTestsIfNeeded () {
	L_Q();
	if (linphone_nat_policy_stun_enabled(natPolicy) && !(linphone_nat_policy_ice_enabled(natPolicy) || linphone_nat_policy_turn_enabled(natPolicy))) {
		stunClient = makeUnique<StunClient>(q->getCore());
		int audioPort = mainAudioStreamIndex ? getStreamsGroup().getStream(mainAudioStreamIndex)->getPortConfig().rtpPort : 0;
		int videoPort = mainVideoStreamIndex ? getStreamsGroup().getStream(mainVideoStreamIndex)->getPortConfig().rtpPort : 0;
		int textPort = mainTextStreamIndex ? getStreamsGroup().getStream(mainTextStreamIndex)->getPortConfig().rtpPort : 0;
		int ret = stunClient->run(audioPort, videoPort, textPort);
		if (ret >= 0)
			pingTime = ret;
	}
}

/*
 * Select IP version to use for advertising local addresses of RTP streams, for an incoming call.
 * If the call is received through a know proxy that is IPv6, use IPv6.
 * Otherwise check the remote contact address.
 * If later the resulting media description tells that we have to send IPv4, it won't be a problem because the RTP sockets
 * are dual stack.
 */
void MediaSessionPrivate::selectIncomingIpVersion () {
	L_Q();
	if (linphone_core_ipv6_enabled(q->getCore()->getCCore())) {
		if (destProxy && destProxy->op)
			af = destProxy->op->getAddressFamily();
		else
			af = op->getAddressFamily();
	} else
		af = AF_INET;
}

/*
 * Choose IP version we are going to use for RTP streams IP address advertised in SDP.
 * The algorithm is as follows:
 * - if ipv6 is disabled at the core level, it is always AF_INET
 * - Otherwise, if the call is done through a known proxy config, then use the information obtained during REGISTER
 * - Otherwise if the destination address for the call is an IPv6 address, use IPv6.
 * to know if IPv6 is supported by the server.
**/
void MediaSessionPrivate::selectOutgoingIpVersion () {
	L_Q();
	char ipv4[LINPHONE_IPADDR_SIZE];
	char ipv6[LINPHONE_IPADDR_SIZE];
	bool haveIpv6 = false;
	bool haveIpv4 = false;

	af = AF_UNSPEC;
	if (linphone_core_get_local_ip_for(AF_INET, nullptr, ipv4) == 0)
		haveIpv4 = true;
	if (linphone_core_ipv6_enabled(q->getCore()->getCCore())) {
		const LinphoneAddress *to = linphone_call_log_get_to_address(log);

		if (linphone_core_get_local_ip_for(AF_INET6, nullptr, ipv6) == 0)
			haveIpv6 = true;
		if (destProxy && destProxy->op) {
			// We can determine from the proxy connection whether IPv6 works - this is the most reliable
			af = destProxy->op->getAddressFamily();
		} else if (sal_address_is_ipv6(L_GET_PRIVATE_FROM_C_OBJECT(to)->getInternalAddress())) {
			af = AF_INET6;
		}

		if (!linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "prefer_ipv6", TRUE) && haveIpv4) {
			// This is the case where IPv4 is to be prefered if both are available
			af = AF_INET; // We'll use IPv4
			lInfo() << "prefer_ipv6 is set to false, as both IP versions are available we are going to use IPv4";
		}
		if (af == AF_UNSPEC)
			af = haveIpv6 ? AF_INET6 : AF_INET;
	} else {
		af = AF_INET;
	}
	// Fill the media_localip default value since we have it here
	mediaLocalIp = (af == AF_INET6) ? ipv6 : ipv4;
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::forceStreamsDirAccordingToState (SalMediaDescription *md) {
	L_Q();
	for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		SalStreamDescription *sd = &md->streams[i];
		switch (state) {
			case CallSession::State::Pausing:
			case CallSession::State::Paused:
				if (sd->dir != SalStreamInactive) {
					sd->dir = SalStreamSendOnly;
					if ((sd->type == SalVideo) && lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "inactive_video_on_pause", 0))
						sd->dir = SalStreamInactive;
				}
				break;
			default:
				break;
		}
		/* Reflect the stream directions in the call params */
		if (i == mainAudioStreamIndex)
			getCurrentParams()->setAudioDirection(MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd->dir));
		else if (i == mainVideoStreamIndex)
			getCurrentParams()->setVideoDirection(MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd->dir));
	}
}

bool MediaSessionPrivate::generateB64CryptoKey (size_t keyLength, char *keyOut, size_t keyOutSize) {
	uint8_t *tmp = (uint8_t *)ms_malloc0(keyLength);
	if (!sal_get_random_bytes(tmp, keyLength)) {
		lError() << "Failed to generate random key";
		ms_free(tmp);
		return false;
	}
	size_t b64Size = b64::b64_encode((const char *)tmp, keyLength, nullptr, 0);
	if (b64Size == 0) {
		lError() << "Failed to get b64 result size";
		ms_free(tmp);
		return false;
	}
	if (b64Size >= keyOutSize) {
		lError() << "Insufficient room for writing base64 SRTP key";
		ms_free(tmp);
		return false;
	}
	b64Size = b64::b64_encode((const char *)tmp, keyLength, keyOut, keyOutSize);
	if (b64Size == 0) {
		lError() << "Failed to b64 encode key";
		ms_free(tmp);
		return false;
	}
	keyOut[b64Size] = '\0';
	ms_free(tmp);
	return true;
}

void MediaSessionPrivate::makeLocalMediaDescription () {
	L_Q();
	int maxIndex = 0;
	bool rtcpMux = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_mux", 0);
	SalMediaDescription *md = sal_media_description_new();
	SalMediaDescription *oldMd = localDesc;
	
	//Initialize streams, because from now on we will need the allocated RTP port numbers to create the offer.
	initializeStreams();

	/* Multicast is only set in case of outgoing call */
	if (direction == LinphoneCallOutgoing) {
		if (mainAudioStreamIndex != -1 && getParams()->audioMulticastEnabled()) {
			md->streams[mainAudioStreamIndex].ttl = linphone_core_get_audio_multicast_ttl(q->getCore()->getCCore());
			md->streams[mainAudioStreamIndex].multicast_role = SalMulticastSender;
		}
		if (mainVideoStreamIndex != -1 && getParams()->videoMulticastEnabled()) {
			md->streams[mainVideoStreamIndex].ttl = linphone_core_get_video_multicast_ttl(q->getCore()->getCCore());
			md->streams[mainVideoStreamIndex].multicast_role = SalMulticastSender;
		}
	}

	getParams()->getPrivate()->adaptToNetwork(q->getCore()->getCCore(), pingTime);

	string subject = q->getParams()->getSessionName();
	if (!subject.empty()) {
		strncpy(md->name, subject.c_str(), sizeof(md->name));
		md->name[sizeof(md->name) - 1] = '\0';
	}
	md->session_id = (oldMd ? oldMd->session_id : (bctbx_random() & 0xfff));
	md->session_ver = (oldMd ? (oldMd->session_ver + 1) : (bctbx_random() & 0xfff));
	md->nb_streams = (biggestDesc ? biggestDesc->nb_streams : 1);

	/* Re-check local ip address each time we make a new offer, because it may change in case of network reconnection */
	{
		LinphoneAddress *address = (direction == LinphoneCallOutgoing ? log->to : log->from);
		getLocalIp(*L_GET_CPP_PTR_FROM_C_OBJECT(address));
	}

	strncpy(md->addr, mediaLocalIp.c_str(), sizeof(md->addr));
	md->addr[sizeof(md->addr) - 1] = '\0';

	LinphoneAddress *addr = nullptr;
	if (destProxy) {
		addr = linphone_address_clone(linphone_proxy_config_get_identity_address(destProxy));
	} else {
		addr = linphone_address_new(linphone_core_get_identity(q->getCore()->getCCore()));
	}
	if (linphone_address_get_username(addr)) {/* Might be null in case of identity without userinfo */
		strncpy(md->username, linphone_address_get_username(addr), sizeof(md->username));
		md->username[sizeof(md->username) - 1] = '\0';
	}
	linphone_address_unref(addr);

	int bandwidth = getParams()->getPrivate()->getDownBandwidth();
	if (bandwidth)
		md->bandwidth = bandwidth;
	else
		md->bandwidth = linphone_core_get_download_bandwidth(q->getCore()->getCCore());

	SalCustomSdpAttribute *customSdpAttributes = getParams()->getPrivate()->getCustomSdpAttributes();
	if (customSdpAttributes)
		md->custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);

	PayloadTypeHandler pth(q->getCore());

	bctbx_list_t *l = NULL;
	if (mainAudioStreamIndex != -1){
		l = pth.makeCodecsList(SalAudio, getParams()->getAudioBandwidthLimit(), -1,
		oldMd ? oldMd->streams[mainAudioStreamIndex].already_assigned_payloads : nullptr);
	
		if (l && getParams()->audioEnabled()) {
			strncpy(md->streams[mainAudioStreamIndex].rtp_addr, getPublicIpForStream(mainAudioStreamIndex).c_str(), sizeof(md->streams[mainAudioStreamIndex].rtp_addr));
			strncpy(md->streams[mainAudioStreamIndex].rtcp_addr, getPublicIpForStream(mainAudioStreamIndex).c_str(), sizeof(md->streams[mainAudioStreamIndex].rtcp_addr));
			strncpy(md->streams[mainAudioStreamIndex].name, "Audio", sizeof(md->streams[mainAudioStreamIndex].name) - 1);
			md->streams[mainAudioStreamIndex].rtp_port = mediaPorts[mainAudioStreamIndex].rtpPort;
			md->streams[mainAudioStreamIndex].rtcp_port = mediaPorts[mainAudioStreamIndex].rtcpPort;
			md->streams[mainAudioStreamIndex].proto = getParams()->getMediaProto();
			md->streams[mainAudioStreamIndex].dir = getParams()->getPrivate()->getSalAudioDirection();
			md->streams[mainAudioStreamIndex].type = SalAudio;
			md->streams[mainAudioStreamIndex].rtcp_mux = rtcpMux;
			int downPtime = getParams()->getPrivate()->getDownPtime();
			if (downPtime)
				md->streams[mainAudioStreamIndex].ptime = downPtime;
			else
				md->streams[mainAudioStreamIndex].ptime = linphone_core_get_download_ptime(q->getCore()->getCCore());
			md->streams[mainAudioStreamIndex].max_rate = pth.getMaxCodecSampleRate(l);
			md->streams[mainAudioStreamIndex].payloads = l;
			if (audioStream && audioStream->ms.sessions.rtp_session) {
				md->streams[mainAudioStreamIndex].rtp_ssrc = rtp_session_get_send_ssrc(audioStream->ms.sessions.rtp_session);
				strncpy(md->streams[mainAudioStreamIndex].rtcp_cname, getMe()->getAddress().asString().c_str(), sizeof(md->streams[mainAudioStreamIndex].rtcp_cname));
			}
			else
				lWarning() << "Cannot get audio local ssrc for CallSession [" << q << "]";
			if (mainAudioStreamIndex > maxIndex)
				maxIndex = mainAudioStreamIndex;
		} else {
			lInfo() << "Don't put audio stream on local offer for CallSession [" << q << "]";
			md->streams[mainAudioStreamIndex].dir = SalStreamInactive;
			if(l)
				l = bctbx_list_free_with_data(l, (bctbx_list_free_func)payload_type_destroy);
		}
		customSdpAttributes = getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeAudio);
		if (customSdpAttributes)
			md->streams[mainAudioStreamIndex].custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);
	}
	if (mainVideoStreamIndex != -1){
		md->streams[mainVideoStreamIndex].proto = getParams()->getMediaProto();
		md->streams[mainVideoStreamIndex].dir = getParams()->getPrivate()->getSalVideoDirection();
		md->streams[mainVideoStreamIndex].type = SalVideo;
		md->streams[mainVideoStreamIndex].rtcp_mux = rtcpMux;
		strncpy(md->streams[mainVideoStreamIndex].name, "Video", sizeof(md->streams[mainVideoStreamIndex].name) - 1);

		l = pth.makeCodecsList(SalVideo, 0, -1,
			oldMd ? oldMd->streams[mainVideoStreamIndex].already_assigned_payloads : nullptr);
		if (l && getParams()->videoEnabled()){
			strncpy(md->streams[mainVideoStreamIndex].rtp_addr, getPublicIpForStream(mainVideoStreamIndex).c_str(), sizeof(md->streams[mainVideoStreamIndex].rtp_addr));
			strncpy(md->streams[mainVideoStreamIndex].rtcp_addr, getPublicIpForStream(mainVideoStreamIndex).c_str(), sizeof(md->streams[mainVideoStreamIndex].rtcp_addr));
			md->streams[mainVideoStreamIndex].rtp_port = mediaPorts[mainVideoStreamIndex].rtpPort;
			md->streams[mainVideoStreamIndex].rtcp_port = mediaPorts[mainVideoStreamIndex].rtcpPort;
			md->streams[mainVideoStreamIndex].payloads = l;
			if (videoStream && videoStream->ms.sessions.rtp_session) {
				md->streams[mainVideoStreamIndex].rtp_ssrc = rtp_session_get_send_ssrc(videoStream->ms.sessions.rtp_session);
				strncpy(md->streams[mainVideoStreamIndex].rtcp_cname, getMe()->getAddress().asString().c_str(), sizeof(md->streams[mainVideoStreamIndex].rtcp_cname));
			} else
				lWarning() << "Cannot get video local ssrc for CallSession [" << q << "]";
			if (mainVideoStreamIndex > maxIndex)
				maxIndex = mainVideoStreamIndex;
		} else {
			lInfo() << "Don't put video stream on local offer for CallSession [" << q << "]";
			md->streams[mainVideoStreamIndex].dir = SalStreamInactive;
			if(l)
				l = bctbx_list_free_with_data(l, (bctbx_list_free_func)payload_type_destroy);
		}
		customSdpAttributes = getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeVideo);
		if (customSdpAttributes)
			md->streams[mainVideoStreamIndex].custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);
	}

	if (mainTextStreamIndex != -1){
		md->streams[mainTextStreamIndex].proto = getParams()->getMediaProto();
		md->streams[mainTextStreamIndex].dir = SalStreamSendRecv;
		md->streams[mainTextStreamIndex].type = SalText;
		md->streams[mainTextStreamIndex].rtcp_mux = rtcpMux;
		strncpy(md->streams[mainTextStreamIndex].name, "Text", sizeof(md->streams[mainTextStreamIndex].name) - 1);
		if (getParams()->realtimeTextEnabled()) {
			strncpy(md->streams[mainTextStreamIndex].rtp_addr, getPublicIpForStream(mainTextStreamIndex).c_str(), sizeof(md->streams[mainTextStreamIndex].rtp_addr));
			strncpy(md->streams[mainTextStreamIndex].rtcp_addr, getPublicIpForStream(mainTextStreamIndex).c_str(), sizeof(md->streams[mainTextStreamIndex].rtcp_addr));

			md->streams[mainTextStreamIndex].rtp_port = mediaPorts[mainTextStreamIndex].rtpPort;
			md->streams[mainTextStreamIndex].rtcp_port = mediaPorts[mainTextStreamIndex].rtcpPort;

			l = pth.makeCodecsList(SalText, 0, -1,
				oldMd ? oldMd->streams[mainTextStreamIndex].already_assigned_payloads : nullptr);
			md->streams[mainTextStreamIndex].payloads = l;
			if (textStream && textStream->ms.sessions.rtp_session) {
				md->streams[mainTextStreamIndex].rtp_ssrc = rtp_session_get_send_ssrc(textStream->ms.sessions.rtp_session);
				strncpy(md->streams[mainTextStreamIndex].rtcp_cname, getMe()->getAddress().asString().c_str(), sizeof(md->streams[mainTextStreamIndex].rtcp_cname));
			} else
				lWarning() << "Cannot get text local ssrc for CallSession [" << q << "]";
			if (mainTextStreamIndex > maxIndex)
				maxIndex = mainTextStreamIndex;
		} else {
			lInfo() << "Don't put text stream on local offer for CallSession [" << q << "]";
			md->streams[mainTextStreamIndex].dir = SalStreamInactive;
		}
		customSdpAttributes = getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeText);
		if (customSdpAttributes)
			md->streams[mainTextStreamIndex].custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);
	}

	md->nb_streams = MAX(md->nb_streams, maxIndex + 1);

	/* Deactivate unused streams */
	for (int i = md->nb_streams; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (md->streams[i].rtp_port == 0) {
			md->streams[i].dir = SalStreamInactive;
			if (biggestDesc && (i < biggestDesc->nb_streams)) {
				md->streams[i].proto = biggestDesc->streams[i].proto;
				md->streams[i].type = biggestDesc->streams[i].type;
			}
		}
	}
	setupEncryptionKeys(md);
	setupDtlsKeys(md);
	setupZrtpHash(md);
	setupImEncryptionEngineParameters(md);
	setupRtcpFb(md);
	setupRtcpXr(md);
	if (stunClient)
		stunClient->updateMediaDescription(md);
	localDesc = md;
	updateLocalMediaDescriptionFromIce();
	if (oldMd) {
		transferAlreadyAssignedPayloadTypes(oldMd, md);
		localDescChanged = sal_media_description_equals(md, oldMd);
		sal_media_description_unref(oldMd);
		if (getParams()->getPrivate()->getInternalCallUpdate()) {
			/*
			 * An internal call update (ICE reINVITE) is not expected to modify the actual media stream parameters.
			 * However, the localDesc may change between first INVITE and ICE reINVITE, for example if the remote party has declined a video stream.
			 * We use the internalCallUpdate flag to prevent trigger an unnecessary media restart.
			 */
			localDescChanged = 0;
		}
	}
	forceStreamsDirAccordingToState(md);
	if (op) op->setLocalMediaDescription(localDesc);
}

void MediaSessionPrivate::setupDtlsKeys (SalMediaDescription *md) {
	for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&md->streams[i]))
			continue;
		/* If media encryption is set to DTLS check presence of fingerprint in the call which shall have been set at stream init
		 * but it may have failed when retrieving certificate resulting in no fingerprint present and then DTLS not usable */
		if (sal_stream_description_has_dtls(&md->streams[i])) {
			/* Get the self fingerprint from call (it's computed at stream init) */
			strncpy(md->streams[i].dtls_fingerprint, dtlsCertificateFingerprint.c_str(), sizeof(md->streams[i].dtls_fingerprint));
			/* If we are offering, SDP will have actpass setup attribute when role is unset, if we are responding the result mediadescription will be set to SalDtlsRoleIsClient */
			md->streams[i].dtls_role = SalDtlsRoleUnset;
		} else {
			md->streams[i].dtls_fingerprint[0] = '\0';
			md->streams[i].dtls_role = SalDtlsRoleInvalid;
		}
	}
}

int MediaSessionPrivate::setupEncryptionKey (SalSrtpCryptoAlgo *crypto, MSCryptoSuite suite, unsigned int tag) {
	crypto->tag = tag;
	crypto->algo = suite;
	size_t keylen = 0;
	switch (suite) {
		case MS_AES_128_SHA1_80:
		case MS_AES_128_SHA1_32:
		case MS_AES_128_NO_AUTH:
		case MS_NO_CIPHER_SHA1_80: /* Not sure for this one */
			keylen = 30;
			break;
		case MS_AES_256_SHA1_80:
		case MS_AES_CM_256_SHA1_80:
		case MS_AES_256_SHA1_32:
			keylen = 46;
			break;
		case MS_CRYPTO_SUITE_INVALID:
			break;
	}
	if ((keylen == 0) || !generateB64CryptoKey(keylen, crypto->master_key, SAL_SRTP_KEY_SIZE)) {
		lError() << "Could not generate SRTP key";
		crypto->algo = MS_CRYPTO_SUITE_INVALID;
		return -1;
	}
	return 0;
}

void MediaSessionPrivate::setupRtcpFb (SalMediaDescription *md) {
	L_Q();
	for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&md->streams[i]))
			continue;
		md->streams[i].rtcp_fb.generic_nack_enabled = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_fb_generic_nack_enabled", 0);
		md->streams[i].rtcp_fb.tmmbr_enabled = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_fb_tmmbr_enabled", 1);
		md->streams[i].implicit_rtcp_fb = getParams()->getPrivate()->implicitRtcpFbEnabled();
		for (const bctbx_list_t *it = md->streams[i].payloads; it != nullptr; it = bctbx_list_next(it)) {
			OrtpPayloadType *pt = reinterpret_cast<OrtpPayloadType *>(bctbx_list_get_data(it));
			PayloadTypeAvpfParams avpf_params;
			if (!getParams()->avpfEnabled() && !getParams()->getPrivate()->implicitRtcpFbEnabled()) {
				payload_type_unset_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
				memset(&avpf_params, 0, sizeof(avpf_params));
			} else {
				payload_type_set_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
				avpf_params = payload_type_get_avpf_params(pt);
				avpf_params.trr_interval = getParams()->getAvpfRrInterval();
			}
			payload_type_set_avpf_params(pt, avpf_params);
		}
	}
}

void MediaSessionPrivate::setupRtcpXr (SalMediaDescription *md) {
	L_Q();
	md->rtcp_xr.enabled = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_enabled", 1);
	if (md->rtcp_xr.enabled) {
		const char *rcvr_rtt_mode = lp_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_rcvr_rtt_mode", "all");
		if (strcasecmp(rcvr_rtt_mode, "all") == 0)
			md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttAll;
		else if (strcasecmp(rcvr_rtt_mode, "sender") == 0)
			md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttSender;
		else
			md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttNone;
		if (md->rtcp_xr.rcvr_rtt_mode != OrtpRtcpXrRcvrRttNone)
			md->rtcp_xr.rcvr_rtt_max_size = lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_rcvr_rtt_max_size", 10000);
		md->rtcp_xr.stat_summary_enabled = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_stat_summary_enabled", 1);
		if (md->rtcp_xr.stat_summary_enabled)
			md->rtcp_xr.stat_summary_flags = OrtpRtcpXrStatSummaryLoss | OrtpRtcpXrStatSummaryDup | OrtpRtcpXrStatSummaryJitt | OrtpRtcpXrStatSummaryTTL;
		md->rtcp_xr.voip_metrics_enabled = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_voip_metrics_enabled", 1);
	}
	for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&md->streams[i]))
			continue;
		memcpy(&md->streams[i].rtcp_xr, &md->rtcp_xr, sizeof(md->streams[i].rtcp_xr));
	}
}

void MediaSessionPrivate::setupZrtpHash (SalMediaDescription *md) {
	L_Q();
	if (linphone_core_media_encryption_supported(q->getCore()->getCCore(), LinphoneMediaEncryptionZRTP)) {
		/* Set the hello hash for all streams */
		for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
			if (!sal_stream_description_active(&md->streams[i]))
				continue;
			Stream *s = getStreamsGroup().getStream(i);
			MSZrtpContext *zrtp_context = s->getZrtpContext();
			if (zrtp_context) {
				ms_zrtp_getHelloHash(zrtp_context, md->streams[i].zrtphash, 128);
				/* Turn on the flag to use it if ZRTP is set */
				md->streams[i].haveZrtpHash = (getParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP);
			} else
				md->streams[i].haveZrtpHash = 0;
		}
	}
}

void MediaSessionPrivate::setupImEncryptionEngineParameters (SalMediaDescription *md) {
	L_Q();
	auto encryptionEngine = q->getCore()->getEncryptionEngine();
	if (!encryptionEngine)
		return;

	list<EncryptionParameter> paramList = encryptionEngine->getEncryptionParameters();

	// Loop over IM Encryption Engine parameters and append them to the SDP
	for (const auto &param : paramList) {
		lInfo() << "Appending " << param.first << " parameter to SDP attributes";
		md->custom_sdp_attributes = sal_custom_sdp_attribute_append(md->custom_sdp_attributes, param.first.c_str(), param.second.c_str());
	}
}

void MediaSessionPrivate::setupEncryptionKeys (SalMediaDescription *md) {
	L_Q();
	SalMediaDescription *oldMd = localDesc;
	bool keepSrtpKeys = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "keep_srtp_keys", 1);
	for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&md->streams[i]))
			continue;
		if (sal_stream_description_has_srtp(&md->streams[i])) {
			if (keepSrtpKeys && oldMd && sal_stream_description_active(&oldMd->streams[i]) && sal_stream_description_has_srtp(&oldMd->streams[i])) {
				lInfo() << "Keeping same crypto keys";
				for (int j = 0; j < SAL_CRYPTO_ALGO_MAX; j++) {
					memcpy(&md->streams[i].crypto[j], &oldMd->streams[i].crypto[j], sizeof(SalSrtpCryptoAlgo));
				}
			} else {
				const MSCryptoSuite *suites = linphone_core_get_srtp_crypto_suites(q->getCore()->getCCore());
				for (int j = 0; (suites != nullptr) && (suites[j] != MS_CRYPTO_SUITE_INVALID) && (j < SAL_CRYPTO_ALGO_MAX); j++) {
					setupEncryptionKey(&md->streams[i].crypto[j], suites[j], static_cast<unsigned int>(j) + 1);
				}
			}
		}
	}
}

void MediaSessionPrivate::transferAlreadyAssignedPayloadTypes (SalMediaDescription *oldMd, SalMediaDescription *md) {
	for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		md->streams[i].already_assigned_payloads = oldMd->streams[i].already_assigned_payloads;
		oldMd->streams[i].already_assigned_payloads = nullptr;
	}
}

void MediaSessionPrivate::updateLocalMediaDescriptionFromIce () {
	getIceAgent().updateLocalMediaDescriptionFromIce(localDesc);
	getIceAgent().updateIceStateInCallStats();
	if (op) op->setLocalMediaDescription(localDesc);
}


void MediaSessionPrivate::performMutualAuthentication(){
	L_Q();
	
	// Perform mutual authentication if instant messaging encryption is enabled
	auto encryptionEngine = q->getCore()->getEncryptionEngine();
	// Is call direction really relevant ? might be linked to offerer/answerer rather than call direction ?
	Stream *stream = mainAudioStreamIndex != -1 ? getStreamsGroup().getStream(mainAudioStreamIndex) : nullptr;
	MS2AudioStream *ms2a = dynamic_cast<MS2AudioStream*>(stream);
	if (encryptionEngine && ms2a && ms2a->getZrtpContext()) {
		encryptionEngine->mutualAuthentication(
							ms2a->getZrtpContext(),
							op->getLocalMediaDescription(),
							op->getRemoteMediaDescription(),
							q->getDirection()
							);
	}
}


//might be the same interface as startDtls if audio_stream_start_zrtp is replaced by audio_streamsessions_start_zrtp
void MediaSessionPrivate::startZrtpPrimaryChannel(const SalStreamDescription *remote) {
	if (remote->type != SalAudio) {
		lError() << "Cannot start primary zrtp channel for stream type ["
		<< sal_stream_type_to_string(remote->type) << "]";
		return;
	}
	audio_stream_start_zrtp(audioStream);
	if (remote->haveZrtpHash == 1) {
		int retval = ms_zrtp_setPeerHelloHash(audioStream->ms.sessions.zrtp_context, (uint8_t *)remote->zrtphash, strlen((const char *)(remote->zrtphash)));
		if (retval != 0)
			lError() << "ZRTP hash mismatch 0x" << hex << retval;
	}
	return;
}

void MediaSessionPrivate::startDtlsOnAllStreams () {
	startDtlsOnAudioStream();
	startDtlsOnVideoStream();
	startDtlsOnTextStream();
}

void MediaSessionPrivate::updateStreamCryptoParameters (SalStreamDescription *oldStream, SalStreamDescription *newStream) {
	if (!oldStream || !newStream || oldStream->type != newStream->type)
		return;

	const SalStreamDescription *localStreamDesc = sal_media_description_find_secure_stream_of_type(localDesc, newStream->type);
	if (newStream->type == SalAudio) {
		if (audioStream && localStreamDesc) {
			updateCryptoParameters(localStreamDesc, oldStream, newStream, &audioStream->ms);
			startDtlsOnAudioStream();
		}
	}
#ifdef VIDEO_ENABLED
	else if (newStream->type == SalVideo) {
		if (videoStream && localStreamDesc) {
			updateCryptoParameters(localStreamDesc, oldStream, newStream, &videoStream->ms);
			startDtlsOnVideoStream();
		}
	}
#endif
	else if (newStream->type == SalText) {
		if (textStream && localStreamDesc) {
			updateCryptoParameters(localStreamDesc, oldStream, newStream, &textStream->ms);
			startDtlsOnTextStream();
		}
	}
}

void MediaSessionPrivate::updateStreamsCryptoParameters (SalMediaDescription *oldMd, SalMediaDescription *newMd) {
	const SalStreamDescription *localStreamDesc = sal_media_description_find_secure_stream_of_type(localDesc, SalAudio);
	SalStreamDescription *oldStream = sal_media_description_find_secure_stream_of_type(oldMd, SalAudio);
	SalStreamDescription *newStream = sal_media_description_find_secure_stream_of_type(newMd, SalAudio);
	if (audioStream && localStreamDesc && oldStream && newStream)
		updateCryptoParameters(localStreamDesc, oldStream, newStream, &audioStream->ms);
#ifdef VIDEO_ENABLED
	localStreamDesc = sal_media_description_find_secure_stream_of_type(localDesc, SalVideo);
	oldStream = sal_media_description_find_secure_stream_of_type(oldMd, SalVideo);
	newStream = sal_media_description_find_secure_stream_of_type(newMd, SalVideo);
	if (videoStream && localStreamDesc && oldStream && newStream)
		updateCryptoParameters(localStreamDesc, oldStream, newStream, &videoStream->ms);
#endif
	localStreamDesc = sal_media_description_find_secure_stream_of_type(localDesc, SalText);
	oldStream = sal_media_description_find_secure_stream_of_type(oldMd, SalText);
	newStream = sal_media_description_find_secure_stream_of_type(newMd, SalText);
	if (textStream && localStreamDesc && oldStream && newStream)
		updateCryptoParameters(localStreamDesc, oldStream, newStream, &textStream->ms);
	startDtlsOnAllStreams();
}

bool MediaSessionPrivate::updateCryptoParameters (const SalStreamDescription *localStreamDesc, SalStreamDescription *oldStream, SalStreamDescription *newStream, MediaStream *ms) {
	int cryptoIdx = Sal::findCryptoIndexFromTag(localStreamDesc->crypto, static_cast<unsigned char>(newStream->crypto_local_tag));
	if (cryptoIdx >= 0) {
		if (localDescChanged & SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED)
			ms_media_stream_sessions_set_srtp_send_key_b64(&ms->sessions, newStream->crypto[0].algo, localStreamDesc->crypto[cryptoIdx].master_key);
		if (strcmp(oldStream->crypto[0].master_key, newStream->crypto[0].master_key) != 0)
			ms_media_stream_sessions_set_srtp_recv_key_b64(&ms->sessions, newStream->crypto[0].algo, newStream->crypto[0].master_key);
		return true;
	} else
		lWarning() << "Failed to find local crypto algo with tag: " << newStream->crypto_local_tag;
	return false;
}

// -----------------------------------------------------------------------------

int MediaSessionPrivate::getIdealAudioBandwidth (const SalMediaDescription *md, const SalStreamDescription *desc) {
	L_Q();
	int remoteBandwidth = 0;
	if (desc->bandwidth > 0)
		remoteBandwidth = desc->bandwidth;
	else if (md->bandwidth > 0) {
		/* Case where b=AS is given globally, not per stream */
		remoteBandwidth = md->bandwidth;
	}
	int uploadBandwidth = 0;
	bool forced = false;
	if (getParams()->getPrivate()->getUpBandwidth() > 0) {
		forced = true;
		uploadBandwidth = getParams()->getPrivate()->getUpBandwidth();
	} else
		uploadBandwidth = linphone_core_get_upload_bandwidth(q->getCore()->getCCore());
	uploadBandwidth = PayloadTypeHandler::getMinBandwidth(uploadBandwidth, remoteBandwidth);
	if (!linphone_core_media_description_contains_video_stream(md) || forced)
		return uploadBandwidth;
	if (PayloadTypeHandler::bandwidthIsGreater(uploadBandwidth, 512))
		uploadBandwidth = 100;
	else if (PayloadTypeHandler::bandwidthIsGreater(uploadBandwidth, 256))
		uploadBandwidth = 64;
	else if (PayloadTypeHandler::bandwidthIsGreater(uploadBandwidth, 128))
		uploadBandwidth = 40;
	else if (PayloadTypeHandler::bandwidthIsGreater(uploadBandwidth, 0))
		uploadBandwidth = 24;
	return uploadBandwidth;
}

/*
 * Frees the media resources of the call.
 * This has to be done at the earliest, unlike signaling resources that sometimes need to be kept a bit more longer.
 * It is called by setTerminated() (for termination of calls signaled to the application), or directly by the destructor of the session
 * if it was never notified to the application.
 */
void MediaSessionPrivate::freeResources () {
	getStreamsGroup().clearStreams();
}

void MediaSessionPrivate::handleIceEvents (OrtpEvent *ev) {
	L_Q();
	OrtpEventType evt = ortp_event_get_type(ev);
	OrtpEventData *evd = ortp_event_get_data(ev);
	if (evt == ORTP_EVENT_ICE_SESSION_PROCESSING_FINISHED) {
		if (iceAgent->hasCompletedCheckList()) {
			/* The ICE session has succeeded, so perform a call update */
			if (iceAgent->isControlling() && q->getCurrentParams()->getPrivate()->getUpdateCallWhenIceCompleted()) {
				if (state == CallSession::State::StreamsRunning){
					MediaSessionParams newParams(*getParams());
					newParams.getPrivate()->setInternalCallUpdate(true);
					q->update(&newParams);
				}else{
					lWarning() << "Cannot send reINVITE for ICE during state " << state;
				}
			}else if (!iceAgent->isControlling() && incomingIceReinvitePending){
				q->acceptUpdate(nullptr);
				incomingIceReinvitePending = false;
			}
			getStreamsGroup().startDtls();
		}
		iceAgent->updateIceStateInCallStats();
	} else if (evt == ORTP_EVENT_ICE_GATHERING_FINISHED) {
		if (!evd->info.ice_processing_successful)
			lWarning() << "No STUN answer from [" << linphone_nat_policy_get_stun_server(q->getPrivate()->getNatPolicy()) << "], continuing without STUN";
		iceAgent->gatheringFinished();
		updateLocalMediaDescriptionFromIce();
		switch (state) {
			case CallSession::State::Updating:
				startUpdate();
				break;
			case CallSession::State::UpdatedByRemote:
				startAcceptUpdate(prevState, Utils::toString(prevState));
				break;
			case CallSession::State::OutgoingInit:
				stopStreamsForIceGathering();
				if (isReadyForInvite())
					q->startInvite(nullptr, "");
				break;
			case CallSession::State::Idle:
				stopStreamsForIceGathering();
				deferIncomingNotification = false;
				startIncomingNotification();
				break;
			default:
				break;
		}
	} else if (evt == ORTP_EVENT_ICE_LOSING_PAIRS_COMPLETED) {
		if (state == CallSession::State::UpdatedByRemote) {
			startAcceptUpdate(prevState, Utils::toString(prevState));
			iceAgent->updateIceStateInCallStats();
		}
	} else if (evt == ORTP_EVENT_ICE_RESTART_NEEDED) {
		iceAgent->restartSession(IR_Controlling);
		q->update(getCurrentParams());
	}
}


void MediaSessionPrivate::initializeAudioStream () {
	L_Q();
	
	if (audioStream)
		return;
	if (!sessions[mainAudioStreamIndex].rtp_session) {
		SalMulticastRole multicastRole = getMulticastRole(SalAudio);
		SalMediaDescription *remoteDesc = nullptr;
		SalStreamDescription *streamDesc = nullptr;
		if (op)
			remoteDesc = op->getRemoteMediaDescription();
		if (remoteDesc)
			streamDesc = sal_media_description_find_best_stream(remoteDesc, SalAudio);

		audioStream = audio_stream_new2(q->getCore()->getCCore()->factory, L_STRING_TO_C(getBindIpForStream(mainAudioStreamIndex)),
			(multicastRole ==  SalMulticastReceiver) ? streamDesc->rtp_port : mediaPorts[mainAudioStreamIndex].rtpPort,
			(multicastRole ==  SalMulticastReceiver) ? 0 /* Disabled for now */ : mediaPorts[mainAudioStreamIndex].rtcpPort);
		if (multicastRole == SalMulticastReceiver)
			joinMulticastGroup(mainAudioStreamIndex, &audioStream->ms);
		
		configureRtpSession(audioStream->ms.sessions.rtp_session, LinphoneStreamTypeAudio);
		setupDtlsParams(&audioStream->ms);

		/* Initialize zrtp even if we didn't explicitely set it, just in case peer offers it */
		if (linphone_core_media_encryption_supported(q->getCore()->getCCore(), LinphoneMediaEncryptionZRTP)) {
			LinphoneAddress *peerAddr = (direction == LinphoneCallIncoming) ? log->from : log->to;
			LinphoneAddress *selfAddr = (direction == LinphoneCallIncoming) ? log->to : log->from;
			char *peerUri = ms_strdup_printf("%s:%s@%s"	, linphone_address_get_scheme(peerAddr)
														, linphone_address_get_username(peerAddr)
														, linphone_address_get_domain(peerAddr));
			char *selfUri = ms_strdup_printf("%s:%s@%s"	, linphone_address_get_scheme(selfAddr)
														, linphone_address_get_username(selfAddr)
														, linphone_address_get_domain(selfAddr));

			MSZrtpParams params;
			zrtpCacheAccess zrtpCacheInfo = linphone_core_get_zrtp_cache_access(q->getCore()->getCCore());

			memset(&params, 0, sizeof(MSZrtpParams));
			/* media encryption of current params will be set later when zrtp is activated */
			params.zidCacheDB = zrtpCacheInfo.db;
			params.zidCacheDBMutex = zrtpCacheInfo.dbMutex;
			params.peerUri = peerUri;
			params.selfUri = selfUri;
			/* Get key lifespan from config file, default is 0:forever valid */
			params.limeKeyTimeSpan = bctbx_time_string_to_sec(lp_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "sip", "lime_key_validity", "0"));
			setZrtpCryptoTypesParameters(&params);
			audio_stream_enable_zrtp(audioStream, &params);
			if (peerUri)
				ms_free(peerUri);
			if (selfUri)
				ms_free(selfUri);
		}

		media_stream_reclaim_sessions(&audioStream->ms, &sessions[mainAudioStreamIndex]);
	} else {
		audioStream = audio_stream_new_with_sessions(q->getCore()->getCCore()->factory, &sessions[mainAudioStreamIndex]);
	}
	
	MSSndCard *playcard = q->getCore()->getCCore()->sound_conf.lsd_card ? q->getCore()->getCCore()->sound_conf.lsd_card : q->getCore()->getCCore()->sound_conf.play_sndcard;
	if (playcard) {
		// Set the stream type immediately, as on iOS AudioUnit is instanciated very early because it is 
		// otherwise too slow to start.
		ms_snd_card_set_stream_type(playcard, MS_SND_CARD_STREAM_VOICE);
	}

	if (mediaPorts[mainAudioStreamIndex].rtpPort == -1)
		setPortConfigFromRtpSession(mainAudioStreamIndex, audioStream->ms.sessions.rtp_session);
	int dscp = linphone_core_get_audio_dscp(q->getCore()->getCCore());
	if (dscp != -1)
		audio_stream_set_dscp(audioStream, dscp);
	if (linphone_core_echo_limiter_enabled(q->getCore()->getCCore())) {
		string type = lp_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "sound", "el_type", "mic");
		if (type == "mic")
			audio_stream_enable_echo_limiter(audioStream, ELControlMic);
		else if (type == "full")
			audio_stream_enable_echo_limiter(audioStream, ELControlFull);
	}

	// Equalizer location in the graph: 'mic' = in input graph, otherwise in output graph.
	// Any other value than mic will default to output graph for compatibility.
	string location = lp_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "sound", "eq_location", "hp");
	audioStream->eq_loc = (location == "mic") ? MSEqualizerMic : MSEqualizerHP;
	lInfo() << "Equalizer location: " << location;

	audio_stream_enable_gain_control(audioStream, true);
	if (linphone_core_echo_cancellation_enabled(q->getCore()->getCCore())) {
		int len = lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "ec_tail_len", 0);
		int delay = lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "ec_delay", 0);
		int framesize = lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "ec_framesize", 0);
		audio_stream_set_echo_canceller_params(audioStream, len, delay, framesize);
		if (audioStream->ec) {
			char *statestr=reinterpret_cast<char *>(ms_malloc0(ecStateMaxLen));
			if (lp_config_relative_file_exists(linphone_core_get_config(q->getCore()->getCCore()), ecStateStore.c_str())
				&& (lp_config_read_relative_file(linphone_core_get_config(q->getCore()->getCCore()), ecStateStore.c_str(), statestr, ecStateMaxLen) == 0)) {
				ms_filter_call_method(audioStream->ec, MS_ECHO_CANCELLER_SET_STATE_STRING, statestr);
			}
			ms_free(statestr);
		}
	}
	audio_stream_enable_automatic_gain_control(audioStream, linphone_core_agc_enabled(q->getCore()->getCCore()));
	bool_t enabled = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "noisegate", 0);
	audio_stream_enable_noise_gate(audioStream, enabled);
	audio_stream_set_features(audioStream, linphone_core_get_audio_features(q->getCore()->getCCore()));

	if (q->getCore()->getCCore()->rtptf) {
		RtpTransport *meta_rtp;
		RtpTransport *meta_rtcp;
		rtp_session_get_transports(audioStream->ms.sessions.rtp_session, &meta_rtp, &meta_rtcp);
		if (!meta_rtp_transport_get_endpoint(meta_rtp)) {
			lInfo() << "CallSession [" << q << "] using custom audio RTP transport endpoint";
			meta_rtp_transport_set_endpoint(meta_rtp, q->getCore()->getCCore()->rtptf->audio_rtp_func(q->getCore()->getCCore()->rtptf->audio_rtp_func_data, mediaPorts[mainAudioStreamIndex].rtpPort));
		}
		if (!meta_rtp_transport_get_endpoint(meta_rtcp))
			meta_rtp_transport_set_endpoint(meta_rtcp, q->getCore()->getCCore()->rtptf->audio_rtcp_func(q->getCore()->getCCore()->rtptf->audio_rtcp_func_data, mediaPorts[mainAudioStreamIndex].rtcpPort));
	}

	audioStreamEvQueue = ortp_ev_queue_new();
	rtp_session_register_event_queue(audioStream->ms.sessions.rtp_session, audioStreamEvQueue);
	iceAgent->prepareIceForStream(&audioStream->ms, false);
}

void MediaSessionPrivate::initializeTextStream () {
	L_Q();
	if (textStream)
		return;
	if (!sessions[mainTextStreamIndex].rtp_session) {
		SalMulticastRole multicastRole = getMulticastRole(SalText);
		SalMediaDescription *remoteDesc = nullptr;
		SalStreamDescription *streamDesc = nullptr;
		if (op)
			remoteDesc = op->getRemoteMediaDescription();
		if (remoteDesc)
			streamDesc = sal_media_description_find_best_stream(remoteDesc, SalText);

		textStream = text_stream_new2(q->getCore()->getCCore()->factory, L_STRING_TO_C(getBindIpForStream(mainTextStreamIndex)),
			(multicastRole ==  SalMulticastReceiver) ? streamDesc->rtp_port : mediaPorts[mainTextStreamIndex].rtpPort,
			(multicastRole ==  SalMulticastReceiver) ? 0 /* Disabled for now */ : mediaPorts[mainTextStreamIndex].rtcpPort);
		if (multicastRole == SalMulticastReceiver)
			joinMulticastGroup(mainTextStreamIndex, &textStream->ms);
		
		configureRtpSession(textStream->ms.sessions.rtp_session, LinphoneStreamTypeText);
		setupDtlsParams(&textStream->ms);
		media_stream_reclaim_sessions(&textStream->ms, &sessions[mainTextStreamIndex]);
	} else
		textStream = text_stream_new_with_sessions(q->getCore()->getCCore()->factory, &sessions[mainTextStreamIndex]);
	if (mediaPorts[mainTextStreamIndex].rtpPort == -1)
		setPortConfigFromRtpSession(mainTextStreamIndex, textStream->ms.sessions.rtp_session);

	if (q->getCore()->getCCore()->rtptf) {
		RtpTransport *meta_rtp;
		RtpTransport *meta_rtcp;
		rtp_session_get_transports(textStream->ms.sessions.rtp_session, &meta_rtp, &meta_rtcp);
		if (!meta_rtp_transport_get_endpoint(meta_rtp))
			meta_rtp_transport_set_endpoint(meta_rtp, q->getCore()->getCCore()->rtptf->audio_rtp_func(q->getCore()->getCCore()->rtptf->audio_rtp_func_data, mediaPorts[mainTextStreamIndex].rtpPort));
		if (!meta_rtp_transport_get_endpoint(meta_rtcp))
			meta_rtp_transport_set_endpoint(meta_rtcp, q->getCore()->getCCore()->rtptf->audio_rtcp_func(q->getCore()->getCCore()->rtptf->audio_rtcp_func_data, mediaPorts[mainTextStreamIndex].rtcpPort));
	}

	textStreamEvQueue = ortp_ev_queue_new();
	rtp_session_register_event_queue(textStream->ms.sessions.rtp_session, textStreamEvQueue);
	iceAgent->prepareIceForStream(&textStream->ms, false);
}

void MediaSessionPrivate::initializeVideoStream () {
#ifdef VIDEO_ENABLED
	L_Q();
	if (videoStream)
		return;
	if (!sessions[mainVideoStreamIndex].rtp_session) {
		SalMulticastRole multicastRole = getMulticastRole(SalVideo);
		SalMediaDescription *remoteDesc = nullptr;
		SalStreamDescription *streamDesc = nullptr;
		if (op)
			remoteDesc = op->getRemoteMediaDescription();
		if (remoteDesc)
			streamDesc = sal_media_description_find_best_stream(remoteDesc, SalVideo);

		videoStream = video_stream_new2(q->getCore()->getCCore()->factory, L_STRING_TO_C(getBindIpForStream(mainVideoStreamIndex)),
			(multicastRole ==  SalMulticastReceiver) ? streamDesc->rtp_port : mediaPorts[mainVideoStreamIndex].rtpPort,
			(multicastRole ==  SalMulticastReceiver) ?  0 /* Disabled for now */ : mediaPorts[mainVideoStreamIndex].rtcpPort);
		if (multicastRole == SalMulticastReceiver)
			joinMulticastGroup(mainVideoStreamIndex, &videoStream->ms);
		
		configureRtpSession(videoStream->ms.sessions.rtp_session, LinphoneStreamTypeVideo);
		setupDtlsParams(&videoStream->ms);
		/* Initialize zrtp even if we didn't explicitely set it, just in case peer offers it */
		if (linphone_core_media_encryption_supported(q->getCore()->getCCore(), LinphoneMediaEncryptionZRTP))
			video_stream_enable_zrtp(videoStream, audioStream);

		media_stream_reclaim_sessions(&videoStream->ms, &sessions[mainVideoStreamIndex]);
	} else
		videoStream = video_stream_new_with_sessions(q->getCore()->getCCore()->factory, &sessions[mainVideoStreamIndex]);

	if (mediaPorts[mainVideoStreamIndex].rtpPort == -1)
		setPortConfigFromRtpSession(mainVideoStreamIndex, videoStream->ms.sessions.rtp_session);
	int dscp = linphone_core_get_video_dscp(q->getCore()->getCCore());
	if (dscp!=-1)
		video_stream_set_dscp(videoStream, dscp);
	video_stream_enable_display_filter_auto_rotate(
		videoStream,
		!!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "video", "display_filter_auto_rotate", 0)
	);

	const char *displayFilter = linphone_core_get_video_display_filter(q->getCore()->getCCore());
	if (displayFilter)
		video_stream_set_display_filter_name(videoStream, displayFilter);
	video_stream_set_event_callback(videoStream, videoStreamEventCb, this);

	if (q->getCore()->getCCore()->rtptf) {
		RtpTransport *meta_rtp;
		RtpTransport *meta_rtcp;
		rtp_session_get_transports(videoStream->ms.sessions.rtp_session, &meta_rtp, &meta_rtcp);
		if (!meta_rtp_transport_get_endpoint(meta_rtp)) {
			lInfo() << "CallSession [" << q << "] using custom video RTP transport endpoint";
			meta_rtp_transport_set_endpoint(meta_rtp, q->getCore()->getCCore()->rtptf->video_rtp_func(q->getCore()->getCCore()->rtptf->video_rtp_func_data, mediaPorts[mainVideoStreamIndex].rtpPort));
		}
		if (!meta_rtp_transport_get_endpoint(meta_rtcp))
			meta_rtp_transport_set_endpoint(meta_rtcp, q->getCore()->getCCore()->rtptf->video_rtcp_func(q->getCore()->getCCore()->rtptf->video_rtcp_func_data, mediaPorts[mainVideoStreamIndex].rtcpPort));
	}
	videoStreamEvQueue = ortp_ev_queue_new();
	rtp_session_register_event_queue(videoStream->ms.sessions.rtp_session, videoStreamEvQueue);
	iceAgent->prepareIceForStream(&videoStream->ms, false);
#ifdef TEST_EXT_RENDERER
	video_stream_set_render_callback(videoStream, extRendererCb, nullptr);
#endif
#else
	videoStream = nullptr;
#endif
}

void MediaSessionPrivate::prepareEarlyMediaForking () {
	/* We need to disable symmetric rtp otherwise our outgoing streams will be switching permanently between the multiple destinations */
	if (audioStream)
		rtp_session_set_symmetric_rtp(audioStream->ms.sessions.rtp_session, false);
	if (videoStream)
		rtp_session_set_symmetric_rtp(videoStream->ms.sessions.rtp_session, false);
}

void MediaSessionPrivate::postConfigureAudioStreams (bool muted) {
	L_Q();
	q->getCore()->getPrivate()->postConfigureAudioStream(audioStream, muted);
	forceSpeakerMuted(speakerMuted);
	if (linphone_core_dtmf_received_has_listener(q->getCore()->getCCore()))
		audio_stream_play_received_dtmfs(audioStream, false);
	if (recordActive)
		q->startRecording();
}

void MediaSessionPrivate::setSymmetricRtp (bool value) {
	for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		setStreamSymmetricRtp(value, i);
	}
}

void MediaSessionPrivate::setStreamSymmetricRtp(bool value, int streamIndex) {
	MSMediaStreamSessions *mss = &sessions[streamIndex];
	if (mss->rtp_session)
		rtp_session_set_symmetric_rtp(mss->rtp_session, value);
}

void MediaSessionPrivate::setupRingbackPlayer () {
	L_Q();
	int pauseTime = 3000;
	audio_stream_play(audioStream, q->getCore()->getCCore()->sound_conf.ringback_tone);
	ms_filter_call_method(audioStream->soundread, MS_FILE_PLAYER_LOOP, &pauseTime);
}

void MediaSessionPrivate::startAudioStream (CallSession::State targetState) {
	L_Q();
	const SalStreamDescription *stream = sal_media_description_find_best_stream(resultDesc, SalAudio);
	if (stream && (stream->dir != SalStreamInactive) && (stream->rtp_port != 0)) {
		int usedPt = -1;
		onHoldFile = "";
		audioProfile = makeProfile(resultDesc, stream, &usedPt);
		if (usedPt == -1)
			lWarning() << "No audio stream accepted?";
		else {
			const char *rtpAddr = (stream->rtp_addr[0] != '\0') ? stream->rtp_addr : resultDesc->addr;
			bool isMulticast = !!ms_is_multicast(rtpAddr);
			bool ok = true;
			getCurrentParams()->getPrivate()->setUsedAudioCodec(rtp_profile_get_payload(audioProfile, usedPt));
			getCurrentParams()->enableAudio(true);
			MSSndCard *playcard = q->getCore()->getCCore()->sound_conf.lsd_card ? q->getCore()->getCCore()->sound_conf.lsd_card : q->getCore()->getCCore()->sound_conf.play_sndcard;
			if (!playcard)
				lWarning() << "No card defined for playback!";
			MSSndCard *captcard = q->getCore()->getCCore()->sound_conf.capt_sndcard;
			if (!captcard)
				lWarning() << "No card defined for capture!";
			string playfile = L_C_TO_STRING(q->getCore()->getCCore()->play_file);
			string recfile = L_C_TO_STRING(q->getCore()->getCCore()->rec_file);
			/* Don't use file or soundcard capture when placed in recv-only mode */
			if ((stream->rtp_port == 0) || (stream->dir == SalStreamRecvOnly) || ((stream->multicast_role == SalMulticastReceiver) && isMulticast)) {
				captcard = nullptr;
				playfile = "";
			}
			if (targetState == CallSession::State::Paused) {
				// In paused state, we never use soundcard
				playcard = captcard = nullptr;
				recfile = "";
				// And we will eventually play "playfile" if set by the user
			}
			if (listener && listener->isPlayingRingbackTone(q->getSharedFromThis())) {
				captcard = nullptr;
				playfile = ""; /* It is setup later */
				if (lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "send_ringback_without_playback", 0) == 1) {
					playcard = nullptr;
					recfile = "";
				}
			}
			// If playfile are supplied don't use soundcards
			bool useRtpIo = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "rtp_io", false);
			bool useRtpIoEnableLocalOutput = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sound", "rtp_io_enable_local_output", false);
			if (q->getCore()->getCCore()->use_files || (useRtpIo && !useRtpIoEnableLocalOutput)) {
				captcard = playcard = nullptr;
			}
			if (getParams()->getPrivate()->getInConference()) {
				// First create the graph without soundcard resources
				captcard = playcard = nullptr;
			}
			if (listener && !listener->areSoundResourcesAvailable(q->getSharedFromThis())) {
				lInfo() << "Sound resources are used by another CallSession, not using soundcard";
				captcard = playcard = nullptr;
			}

			if (playcard) {
				ms_snd_card_set_stream_type(playcard, MS_SND_CARD_STREAM_VOICE);
			}
			media_stream_set_max_network_bitrate(&audioStream->ms, linphone_core_get_upload_bandwidth(q->getCore()->getCCore()) * 1000);
			bool useEc = captcard && linphone_core_echo_cancellation_enabled(q->getCore()->getCCore());
			audio_stream_enable_echo_canceller(audioStream, useEc);
			if (playcard && (stream->max_rate > 0))
				ms_snd_card_set_preferred_sample_rate(playcard, stream->max_rate);
			if (captcard && (stream->max_rate > 0))
				ms_snd_card_set_preferred_sample_rate(captcard, stream->max_rate);
			rtp_session_enable_rtcp_mux(audioStream->ms.sessions.rtp_session, stream->rtcp_mux);
			if (!getParams()->getPrivate()->getInConference() && !getParams()->getRecordFilePath().empty()) {
				audio_stream_mixed_record_open(audioStream, getParams()->getRecordFilePath().c_str());
				getCurrentParams()->setRecordFilePath(getParams()->getRecordFilePath());
			}
			// Valid local tags are > 0
			if (sal_stream_description_has_srtp(stream)) {
				const SalStreamDescription *localStreamDesc = sal_media_description_find_stream(localDesc, stream->proto, SalAudio);
				int cryptoIdx = Sal::findCryptoIndexFromTag(localStreamDesc->crypto, static_cast<unsigned char>(stream->crypto_local_tag));
				if (cryptoIdx >= 0) {
					ms_media_stream_sessions_set_srtp_recv_key_b64(&audioStream->ms.sessions, stream->crypto[0].algo, stream->crypto[0].master_key);
					ms_media_stream_sessions_set_srtp_send_key_b64(&audioStream->ms.sessions, stream->crypto[0].algo, localStreamDesc->crypto[cryptoIdx].master_key);
				} else
					lWarning() << "Failed to find local crypto algo with tag: " << stream->crypto_local_tag;
			}
			configureRtpSessionForRtcpFb(stream);
			configureRtpSessionForRtcpXr(SalAudio);
			bool videoWillBeUsed = false;
#if defined(VIDEO_ENABLED)
			const SalStreamDescription *vstream = sal_media_description_find_best_stream(resultDesc, SalVideo);
			if (vstream && (vstream->dir != SalStreamInactive) && vstream->payloads) {
				/* When video is used, do not make adaptive rate control on audio, it is stupid */
				videoWillBeUsed = true;
			}
#endif
			configureAdaptiveRateControl(&audioStream->ms, getCurrentParams()->getUsedAudioCodec(), videoWillBeUsed);
			if (isMulticast)
				rtp_session_set_multicast_ttl(audioStream->ms.sessions.rtp_session, stream->ttl);
			MSMediaStreamIO io = MS_MEDIA_STREAM_IO_INITIALIZER;
			if (useRtpIo) {
				if (useRtpIoEnableLocalOutput) {
					io.input.type = MSResourceRtp;
					io.input.session = createAudioRtpIoSession();
					if (playcard) {
						io.output.type = MSResourceSoundcard;
						io.output.soundcard = playcard;
					} else {
						io.output.type = MSResourceFile;
						io.output.file = recfile.empty() ? nullptr : recfile.c_str();
					}
				} else {
					io.input.type = io.output.type = MSResourceRtp;
					io.input.session = io.output.session = createAudioRtpIoSession();
				}
				if (!io.input.session)
					ok = false;
			} else {
				if (playcard) {
					io.output.type = MSResourceSoundcard;
					io.output.soundcard = playcard;
				} else {
					io.output.type = MSResourceFile;
					io.output.file = recfile.empty() ? nullptr : recfile.c_str();
				}
				if (captcard) {
					io.input.type = MSResourceSoundcard;
					io.input.soundcard = captcard;
				} else {
					io.input.type = MSResourceFile;
					onHoldFile = playfile;
					io.input.file = nullptr; /* We prefer to use the remote_play api, that allows to play multimedia files */
				}
			}
			if (ok) {
				currentCaptureCard = ms_media_resource_get_soundcard(&io.input);
				currentPlayCard = ms_media_resource_get_soundcard(&io.output);

				int err = audio_stream_start_from_io(audioStream, audioProfile, rtpAddr, stream->rtp_port,
					(stream->rtcp_addr[0] != '\0') ? stream->rtcp_addr : resultDesc->addr,
					(linphone_core_rtcp_enabled(q->getCore()->getCCore()) && !isMulticast) ? (stream->rtcp_port ? stream->rtcp_port : stream->rtp_port + 1) : 0,
					usedPt, &io);
				if (err == 0)
					postConfigureAudioStreams((audioMuted || microphoneMuted) && (listener && !listener->isPlayingRingbackTone(q->getSharedFromThis())));
			}
			ms_media_stream_sessions_set_encryption_mandatory(&audioStream->ms.sessions, isEncryptionMandatory());
			if ((targetState == CallSession::State::Paused) && !captcard && !playfile.empty()) {
				int pauseTime = 500;
				ms_filter_call_method(audioStream->soundread, MS_FILE_PLAYER_LOOP, &pauseTime);
			}
			if (listener && listener->isPlayingRingbackTone(q->getSharedFromThis()))
				setupRingbackPlayer();
			if (getParams()->getPrivate()->getInConference() && listener) {
				// Transform the graph to connect it to the conference filter
				bool mute = (stream->dir == SalStreamRecvOnly);
				listener->onCallSessionConferenceStreamStarting(q->getSharedFromThis(), mute);
			}
			getCurrentParams()->getPrivate()->setInConference(getParams()->getPrivate()->getInConference());
			getCurrentParams()->enableLowBandwidth(getParams()->lowBandwidthEnabled());
			// Start ZRTP engine if needed : set here or remote have a zrtp-hash attribute
			SalMediaDescription *remote = op->getRemoteMediaDescription();
			const SalStreamDescription *remoteStream = sal_media_description_find_best_stream(remote, SalAudio);
			if (linphone_core_media_encryption_supported(q->getCore()->getCCore(), LinphoneMediaEncryptionZRTP)) {
				// Perform mutual authentication if instant messaging encryption is enabled
				auto encryptionEngine = q->getCore()->getEncryptionEngine();
				//Is call direction really relevant ? might be linked to offerer/answerer rather than call direction ?
				LinphoneCallDir direction = this->getPublic()->CallSession::getDirection();
				if (encryptionEngine && audioStream->ms.sessions.zrtp_context) {
					encryptionEngine->mutualAuthentication(
														   audioStream->ms.sessions.zrtp_context,
														   op->getLocalMediaDescription(),
														   op->getRemoteMediaDescription(),
														   direction
														   );
				}
				
				//Start zrtp if remote has offered it or if local is configured for zrtp and is the offerrer. If not, defered when ACK is received
				if ((getParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP && op->isOfferer()) || (remoteStream->haveZrtpHash == 1)) {
					startZrtpPrimaryChannel(remoteStream);
				}
			}
		}
	}
}

void MediaSessionPrivate::startStreams (CallSession::State targetState) {
	L_Q();
	switch (targetState) {
		case CallSession::State::IncomingEarlyMedia:
			if (listener)
				listener->onRingbackToneRequested(q->getSharedFromThis(), true);
			BCTBX_NO_BREAK;
		case CallSession::State::OutgoingEarlyMedia:
			if (!getParams()->earlyMediaSendingEnabled()) {
				audioMuted = true;
				videoMuted = true;
			}
			break;
		default:
			if (listener)
				listener->onRingbackToneRequested(q->getSharedFromThis(), false);
			audioMuted = false;
			videoMuted = false;
			break;
	}

	getCurrentParams()->getPrivate()->setUsedAudioCodec(nullptr);
	getCurrentParams()->getPrivate()->setUsedVideoCodec(nullptr);
	getCurrentParams()->getPrivate()->setUsedRealtimeTextCodec(nullptr);

	if (!audioStream && !videoStream) {
		lFatal() << "startStreams() called without prior init!";
		return;
	}
	if (iceAgent->hasSession()) {
		/* If there is an ICE session when we are about to start streams, then ICE will conduct the media path checking and authentication properly.
		 * Symmetric RTP must be turned off */
		setSymmetricRtp(false);
	}

	if (audioStream) audioStartCount++;
	if (videoStream) videoStartCount++;
	if (textStream) textStartCount++;

	lInfo() << "startStreams() CallSession=[" << q << "] local upload_bandwidth=[" << linphone_core_get_upload_bandwidth(q->getCore()->getCCore())
		<< "] kbit/s; local download_bandwidth=[" << linphone_core_get_download_bandwidth(q->getCore()->getCCore()) << "] kbit/s";
	getCurrentParams()->enableAudio(false);
	if (audioStream)
		startAudioStream(targetState);
	else
		lWarning() << "startStreams(): no audio stream!";
	getCurrentParams()->enableVideo(false);
	if (videoStream) {
		if (audioStream)
			audio_stream_link_video(audioStream, videoStream);
		startVideoStream(targetState);
	}
	/* The on-hold file is to be played once both audio and video are ready */
	if (!onHoldFile.empty() && !getParams()->getPrivate()->getInConference() && audioStream) {
		MSFilter *player = audio_stream_open_remote_play(audioStream, onHoldFile.c_str());
		if (player) {
			int pauseTime = 500;
			ms_filter_call_method(player, MS_PLAYER_SET_LOOP, &pauseTime);
			ms_filter_call_method_noarg(player, MS_PLAYER_START);
		}
	}
	if (getParams()->realtimeTextEnabled())
		startTextStream();

	setDtlsFingerprintOnAllStreams();
	if (!iceAgent->hasCompleted()) {
		iceAgent->startConnectivityChecks();
	} else {
		/* Should not start dtls until ice is completed */
		startDtlsOnAllStreams();
	}
}

void MediaSessionPrivate::startStream (SalStreamDescription *streamDesc, int streamIndex, CallSession::State targetState) {
	L_Q();
	string streamTypeName = sal_stream_description_get_type_as_string(streamDesc);

	if (streamDesc->type == SalAudio) {
		if (audioStream && audioStream->ms.state != MSStreamInitialized)
			audio_stream_unprepare_sound(audioStream);

		switch (targetState) {
			case CallSession::State::IncomingEarlyMedia:
				if (listener)
					listener->onRingbackToneRequested(q->getSharedFromThis(), true);
				BCTBX_NO_BREAK;
			case CallSession::State::OutgoingEarlyMedia:
				if (!getParams()->earlyMediaSendingEnabled())
					audioMuted = true;
				break;
			default:
				if (listener)
					listener->onRingbackToneRequested(q->getSharedFromThis(), false);
				audioMuted = false;
				break;
		}

		getCurrentParams()->getPrivate()->setUsedAudioCodec(nullptr);

		if (!audioStream) {
			lFatal() << "startStream() for audio stream called without prior init!";
			return;
		}
	} else if (streamDesc->type == SalVideo) {
#ifdef VIDEO_ENABLED
		if (videoStream && videoStream->ms.state != MSStreamInitialized)
			video_stream_unprepare_video(videoStream);

		switch (targetState) {
			case CallSession::State::OutgoingEarlyMedia:
				if (!getParams()->earlyMediaSendingEnabled())
					videoMuted = true;
				break;
			default:
				videoMuted = false;
				break;
		}

		getCurrentParams()->getPrivate()->setUsedVideoCodec(nullptr);

		if (!videoStream) {
			lFatal() << "startStream() for video stream called without prior init!";
			return;
		}
#endif
	} else if (streamDesc->type == SalText) {
		if (textStream && textStream->ms.state != MSStreamInitialized)
			text_stream_unprepare_text(textStream);

		getCurrentParams()->getPrivate()->setUsedRealtimeTextCodec(nullptr);
	}

	if (iceAgent->hasSession()) {
		/* If there is an ICE session when we are about to start streams, then ICE will conduct the media path checking and authentication properly.
		 * Symmetric RTP must be turned off */
		setStreamSymmetricRtp(false, streamIndex);
	}

	lInfo() << "startStream() for " << streamTypeName << " stream CallSession=[" << q << "] local upload_bandwidth=[" << linphone_core_get_upload_bandwidth(q->getCore()->getCCore())
		<< "] kbit/s; local download_bandwidth=[" << linphone_core_get_download_bandwidth(q->getCore()->getCCore()) << "] kbit/s";

	if (streamDesc->type == SalAudio) {
		audioStartCount++;

		getCurrentParams()->enableAudio(false);
		if (audioStream)
			startAudioStream(targetState);
		else
			lWarning() << "startStreams(): no audio stream!";

		postProcessHooks.push_back([this] {
			/* The on-hold file is to be played once both audio and video are ready */
			if (!onHoldFile.empty() && !getParams()->getPrivate()->getInConference() && audioStream) {
				MSFilter *player = audio_stream_open_remote_play(audioStream, onHoldFile.c_str());
				if (player) {
					int pauseTime = 500;
					ms_filter_call_method(player, MS_PLAYER_SET_LOOP, &pauseTime);
					ms_filter_call_method_noarg(player, MS_PLAYER_START);
				}
			}
		});

		setDtlsFingerprintOnAudioStream();
		if (iceAgent->hasCompleted())
			startDtlsOnAudioStream();
	} else if (streamDesc->type == SalVideo) {
		videoStartCount++;

		getCurrentParams()->enableVideo(false);
		if (videoStream) {
			if (audioStream)
				audio_stream_link_video(audioStream, videoStream);
			startVideoStream(targetState);
		}

		setDtlsFingerprintOnVideoStream();
		if (iceAgent->hasCompleted())
			startDtlsOnVideoStream();
	} else if (streamDesc->type == SalText) {
		textStartCount++;

		if (getParams()->realtimeTextEnabled())
			startTextStream();

		setDtlsFingerprintOnTextStream();
		if (iceAgent->hasCompleted())
			startDtlsOnTextStream();
	}
}

void MediaSessionPrivate::startTextStream () {
	L_Q();
	const SalStreamDescription *tstream = sal_media_description_find_best_stream(resultDesc, SalText);
	if (tstream && (tstream->dir != SalStreamInactive) && (tstream->rtp_port != 0)) {
		const char *rtpAddr = tstream->rtp_addr[0] != '\0' ? tstream->rtp_addr : resultDesc->addr;
		const char *rtcpAddr = tstream->rtcp_addr[0] != '\0' ? tstream->rtcp_addr : resultDesc->addr;
		const SalStreamDescription *localStreamDesc = sal_media_description_find_stream(localDesc, tstream->proto, SalText);
		int usedPt = -1;
		textProfile = makeProfile(resultDesc, tstream, &usedPt);
		if (usedPt == -1)
			lWarning() << "No text stream accepted";
		else {
			getCurrentParams()->getPrivate()->setUsedRealtimeTextCodec(rtp_profile_get_payload(textProfile, usedPt));
			getCurrentParams()->enableRealtimeText(true);
			if (sal_stream_description_has_srtp(tstream)) {
				int cryptoIdx = Sal::findCryptoIndexFromTag(localStreamDesc->crypto, static_cast<unsigned char>(tstream->crypto_local_tag));
				if (cryptoIdx >= 0) {
					ms_media_stream_sessions_set_srtp_recv_key_b64(&textStream->ms.sessions, tstream->crypto[0].algo, tstream->crypto[0].master_key);
					ms_media_stream_sessions_set_srtp_send_key_b64(&textStream->ms.sessions, tstream->crypto[0].algo, localStreamDesc->crypto[cryptoIdx].master_key);
				}
			}
			configureRtpSessionForRtcpFb(tstream);
			configureRtpSessionForRtcpXr(SalText);
			rtp_session_enable_rtcp_mux(textStream->ms.sessions.rtp_session, tstream->rtcp_mux);
			bool isMulticast = !!ms_is_multicast(rtpAddr);
			if (isMulticast)
				rtp_session_set_multicast_ttl(textStream->ms.sessions.rtp_session, tstream->ttl);
			text_stream_start(textStream, textProfile, rtpAddr, tstream->rtp_port, rtcpAddr,
				(linphone_core_rtcp_enabled(q->getCore()->getCCore()) && !isMulticast) ? (tstream->rtcp_port ? tstream->rtcp_port : tstream->rtp_port + 1) : 0, usedPt);
			ms_filter_add_notify_callback(textStream->rttsink, realTimeTextCharacterReceived, this, false);
			ms_media_stream_sessions_set_encryption_mandatory(&textStream->ms.sessions, isEncryptionMandatory());
		}
	} else
		lInfo() << "No valid text stream defined";
}

void MediaSessionPrivate::startVideoStream (CallSession::State targetState) {
#ifdef VIDEO_ENABLED
	L_Q();
	bool reusedPreview = false;
	/* Shutdown preview */
	MSFilter *source = nullptr;
	if (q->getCore()->getCCore()->previewstream) {
		if (q->getCore()->getCCore()->video_conf.reuse_preview_source)
			source = video_preview_stop_reuse_source(q->getCore()->getCCore()->previewstream);
		else
			video_preview_stop(q->getCore()->getCCore()->previewstream);
		q->getCore()->getCCore()->previewstream = nullptr;
	}
	const SalStreamDescription *vstream = sal_media_description_find_best_stream(resultDesc, SalVideo);
	if (vstream && (vstream->dir != SalStreamInactive) && (vstream->rtp_port != 0)) {
		int usedPt = -1;
		videoProfile = makeProfile(resultDesc, vstream, &usedPt);
		if (usedPt == -1)
			lWarning() << "No video stream accepted";
		else {
			getCurrentParams()->getPrivate()->setUsedVideoCodec(rtp_profile_get_payload(videoProfile, usedPt));
			getCurrentParams()->enableVideo(true);
			rtp_session_enable_rtcp_mux(videoStream->ms.sessions.rtp_session, vstream->rtcp_mux);
			media_stream_set_max_network_bitrate(&videoStream->ms, linphone_core_get_upload_bandwidth(q->getCore()->getCCore()) * 1000);
			if (q->getCore()->getCCore()->video_conf.preview_vsize.width != 0)
				video_stream_set_preview_size(videoStream, q->getCore()->getCCore()->video_conf.preview_vsize);
			video_stream_set_fps(videoStream, linphone_core_get_preferred_framerate(q->getCore()->getCCore()));
			if (lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "video", "nowebcam_uses_normal_fps", 0))
				videoStream->staticimage_webcam_fps_optimization = false;
			const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(q->getCore()->getCCore());
			MSVideoSize vsize;
			vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
			vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
			video_stream_set_sent_video_size(videoStream, vsize);
			video_stream_enable_self_view(videoStream, q->getCore()->getCCore()->video_conf.selfview);
			if (videoWindowId)
				video_stream_set_native_window_id(videoStream, videoWindowId);
			else if (q->getCore()->getCCore()->video_window_id)
				video_stream_set_native_window_id(videoStream, q->getCore()->getCCore()->video_window_id);
			if (q->getCore()->getCCore()->preview_window_id)
				video_stream_set_native_preview_window_id(videoStream, q->getCore()->getCCore()->preview_window_id);
			video_stream_use_preview_video_window(videoStream, q->getCore()->getCCore()->use_preview_window);
			const char *rtpAddr = (vstream->rtp_addr[0] != '\0') ? vstream->rtp_addr : resultDesc->addr;
			const char *rtcpAddr = (vstream->rtcp_addr[0] != '\0') ? vstream->rtcp_addr : resultDesc->addr;
			bool isMulticast = !!ms_is_multicast(rtpAddr);
			MediaStreamDir dir = MediaStreamSendRecv;
			bool isActive = true;
			if (isMulticast) {
				if (vstream->multicast_role == SalMulticastReceiver)
					dir = MediaStreamRecvOnly;
				else
					dir = MediaStreamSendOnly;
			} else if ((vstream->dir == SalStreamSendOnly) && q->getCore()->getCCore()->video_conf.capture)
				dir = MediaStreamSendOnly;
			else if ((vstream->dir == SalStreamRecvOnly) && q->getCore()->getCCore()->video_conf.display)
				dir = MediaStreamRecvOnly;
			else if (vstream->dir == SalStreamSendRecv) {
				if (q->getCore()->getCCore()->video_conf.display && q->getCore()->getCCore()->video_conf.capture)
					dir = MediaStreamSendRecv;
				else if (q->getCore()->getCCore()->video_conf.display)
					dir = MediaStreamRecvOnly;
				else
					dir = MediaStreamSendOnly;
			} else {
				lWarning() << "Video stream is inactive";
				/* Either inactive or incompatible with local capabilities */
				isActive = false;
			}
			MSWebCam *cam = getVideoDevice();
			if (isActive) {
				if (sal_stream_description_has_srtp(vstream)) {
					const SalStreamDescription *localStreamDesc = sal_media_description_find_stream(localDesc, vstream->proto, SalVideo);
					int cryptoIdx = Sal::findCryptoIndexFromTag(localStreamDesc->crypto, static_cast<unsigned char>(vstream->crypto_local_tag));
					if (cryptoIdx >= 0) {
						ms_media_stream_sessions_set_srtp_recv_key_b64(&videoStream->ms.sessions, vstream->crypto[0].algo, vstream->crypto[0].master_key);
						ms_media_stream_sessions_set_srtp_send_key_b64(&videoStream->ms.sessions, vstream->crypto[0].algo, localStreamDesc->crypto[cryptoIdx].master_key);
					}
				}
				configureRtpSessionForRtcpFb(vstream);
				configureRtpSessionForRtcpXr(SalVideo);
				configureAdaptiveRateControl(&videoStream->ms, getCurrentParams()->getUsedVideoCodec(), true);
				log->video_enabled = true;
				video_stream_set_direction(videoStream, dir);
				lInfo() << "startVideoStream: device_rotation=" << q->getCore()->getCCore()->device_rotation;
				video_stream_set_device_rotation(videoStream, q->getCore()->getCCore()->device_rotation);
				video_stream_set_freeze_on_error(videoStream, !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "video", "freeze_on_error", 1));
				if (isMulticast)
					rtp_session_set_multicast_ttl(videoStream->ms.sessions.rtp_session, vstream->ttl);
				video_stream_use_video_preset(videoStream, lp_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "video", "preset", nullptr));
				if (q->getCore()->getCCore()->video_conf.reuse_preview_source && source) {
					lInfo() << "video_stream_start_with_source kept: " << source;
					video_stream_start_with_source(videoStream, videoProfile, rtpAddr, vstream->rtp_port, rtcpAddr,
						linphone_core_rtcp_enabled(q->getCore()->getCCore()) ? (vstream->rtcp_port ? vstream->rtcp_port : vstream->rtp_port + 1) : 0,
						usedPt, -1, cam, source);
					reusedPreview = true;
				} else {
					bool ok = true;
					MSMediaStreamIO io = MS_MEDIA_STREAM_IO_INITIALIZER;
					if (linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "video", "rtp_io", FALSE)) {
						io.input.type = io.output.type = MSResourceRtp;
						io.input.session = io.output.session = createVideoRtpIoSession();
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
						video_stream_start_from_io(videoStream, videoProfile, rtpAddr, vstream->rtp_port, rtcpAddr,
							(linphone_core_rtcp_enabled(q->getCore()->getCCore()) && !isMulticast)  ? (vstream->rtcp_port ? vstream->rtcp_port : vstream->rtp_port + 1) : 0,
							usedPt, &io);
					}
				}

				ms_media_stream_sessions_set_encryption_mandatory(&videoStream->ms.sessions, isEncryptionMandatory());
				if (listener)
					listener->onResetFirstVideoFrameDecoded(q->getSharedFromThis());
				/* Start ZRTP engine if needed : set here or remote have a zrtp-hash attribute */
				SalMediaDescription *remote = op->getRemoteMediaDescription();
				const SalStreamDescription *remoteStream = sal_media_description_find_best_stream(remote, SalVideo);
				if ((getParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP) || (remoteStream->haveZrtpHash == 1)) {
					/* Audio stream is already encrypted and video stream is active */
					if (media_stream_secured(&audioStream->ms) && (media_stream_get_state(&videoStream->ms) == MSStreamStarted)) {
						video_stream_start_zrtp(videoStream);
						if (remoteStream->haveZrtpHash == 1) {
							int retval = ms_zrtp_setPeerHelloHash(videoStream->ms.sessions.zrtp_context, (uint8_t *)remoteStream->zrtphash, strlen((const char *)(remoteStream->zrtphash)));
							if (retval != 0)
								lError() << "Video stream ZRTP hash mismatch 0x" << hex << retval;
						}
					}
				}

				if (linphone_core_retransmission_on_nack_enabled(q->getCore()->getCCore())) {
					video_stream_enable_retransmission_on_nack(videoStream, TRUE);
				}
			}
		}
	} else
		lInfo() << "No valid video stream defined";
	if (!reusedPreview && source) {
		/* Destroy not-reused source filter */
		lWarning() << "Video preview (" << source << ") not reused: destroying it";
		ms_filter_destroy(source);
	}
#endif
}

void MediaSessionPrivate::stopAudioStream () {
	L_Q();
	if (!audioStream)
		return;

	if (listener)
		listener->onUpdateMediaInfoForReporting(q->getSharedFromThis(), LINPHONE_CALL_STATS_AUDIO);
	media_stream_reclaim_sessions(&audioStream->ms, &sessions[mainAudioStreamIndex]);
	if (audioStream->ec) {
		char *stateStr = nullptr;
		ms_filter_call_method(audioStream->ec, MS_ECHO_CANCELLER_GET_STATE_STRING, &stateStr);
		if (stateStr) {
			lInfo() << "Writing echo canceler state, " << (int)strlen(stateStr) << " bytes";
			lp_config_write_relative_file(linphone_core_get_config(q->getCore()->getCCore()), ecStateStore.c_str(), stateStr);
		}
	}
	audio_stream_get_local_rtp_stats(audioStream, &log->local_stats);
	fillLogStats(&audioStream->ms);
	if (listener)
		listener->onCallSessionConferenceStreamStopping(q->getSharedFromThis());
	ms_bandwidth_controller_remove_stream(q->getCore()->getCCore()->bw_controller, &audioStream->ms);
	audio_stream_stop(audioStream);
	updateRtpStats(audioStats, mainAudioStreamIndex);
	audioStream = nullptr;
	handleStreamEvents(mainAudioStreamIndex);
	rtp_session_unregister_event_queue(sessions[mainAudioStreamIndex].rtp_session, audioStreamEvQueue);
	ortp_ev_queue_flush(audioStreamEvQueue);
	ortp_ev_queue_destroy(audioStreamEvQueue);
	audioStreamEvQueue = nullptr;

	getCurrentParams()->getPrivate()->setUsedAudioCodec(nullptr);

	currentCaptureCard = nullptr;
	currentPlayCard = nullptr;

}

void MediaSessionPrivate::stopTextStream () {
	L_Q();
	if (textStream) {
		if (listener)
			listener->onUpdateMediaInfoForReporting(q->getSharedFromThis(), LINPHONE_CALL_STATS_TEXT);
		media_stream_reclaim_sessions(&textStream->ms, &sessions[mainTextStreamIndex]);
		fillLogStats(&textStream->ms);
		text_stream_stop(textStream);
		updateRtpStats(textStats, mainTextStreamIndex);
		textStream = nullptr;
		handleStreamEvents(mainTextStreamIndex);
		rtp_session_unregister_event_queue(sessions[mainTextStreamIndex].rtp_session, textStreamEvQueue);
		ortp_ev_queue_flush(textStreamEvQueue);
		ortp_ev_queue_destroy(textStreamEvQueue);
		textStreamEvQueue = nullptr;
		getCurrentParams()->getPrivate()->setUsedRealtimeTextCodec(nullptr);
	}
}

void MediaSessionPrivate::stopVideoStream () {
#ifdef VIDEO_ENABLED
	L_Q();
	if (videoStream) {
		if (listener)
			listener->onUpdateMediaInfoForReporting(q->getSharedFromThis(), LINPHONE_CALL_STATS_VIDEO);
		media_stream_reclaim_sessions(&videoStream->ms, &sessions[mainVideoStreamIndex]);
		fillLogStats(&videoStream->ms);
		ms_bandwidth_controller_remove_stream(q->getCore()->getCCore()->bw_controller, &videoStream->ms);
		video_stream_stop(videoStream);
		updateRtpStats(videoStats, mainVideoStreamIndex);
		videoStream = nullptr;
		handleStreamEvents(mainVideoStreamIndex);
		rtp_session_unregister_event_queue(sessions[mainVideoStreamIndex].rtp_session, videoStreamEvQueue);
		ortp_ev_queue_flush(videoStreamEvQueue);
		ortp_ev_queue_destroy(videoStreamEvQueue);
		videoStreamEvQueue = nullptr;
		getCurrentParams()->getPrivate()->setUsedVideoCodec(nullptr);
	}
#endif
}

void MediaSessionPrivate::tryEarlyMediaForking (SalMediaDescription *md) {
	L_Q();
	lInfo() << "Early media response received from another branch, checking if media can be forked to this new destination";
	getStreamsGroup().tryEarlyMediaForking(resultDesc, md);
}

void MediaSessionPrivate::updateStreamFrozenPayloads (SalStreamDescription *resultDesc, SalStreamDescription *localStreamDesc) {
	L_Q();
	for (bctbx_list_t *elem = resultDesc->payloads; elem != nullptr; elem = bctbx_list_next(elem)) {
		OrtpPayloadType *pt = reinterpret_cast<OrtpPayloadType *>(bctbx_list_get_data(elem));
		if (PayloadTypeHandler::isPayloadTypeNumberAvailable(localStreamDesc->already_assigned_payloads, payload_type_get_number(pt), nullptr)) {
			/* New codec, needs to be added to the list */
			localStreamDesc->already_assigned_payloads = bctbx_list_append(localStreamDesc->already_assigned_payloads, payload_type_clone(pt));
			lInfo() << "CallSession[" << q << "] : payload type " << payload_type_get_number(pt) << " " << pt->mime_type << "/" << pt->clock_rate
				<< " fmtp=" << L_C_TO_STRING(pt->recv_fmtp) << " added to frozen list";
		}
	}
}

void MediaSessionPrivate::updateFrozenPayloads (SalMediaDescription *result) {
	for (int i = 0; i < result->nb_streams; i++) {
		updateStreamFrozenPayloads(&result->streams[i], &localDesc->streams[i]);
	}
}

void MediaSessionPrivate::updateStreams (SalMediaDescription *newMd, CallSession::State targetState) {
	L_Q();

	if (!((state == CallSession::State::IncomingEarlyMedia) && linphone_core_get_ring_during_incoming_early_media(q->getCore()->getCCore())))
		linphone_core_stop_ringing(q->getCore()->getCCore());

	if (!newMd) {
		lError() << "updateStreams() called with null media description";
		return;
	}

	updateBiggestDesc(localDesc);
	sal_media_description_ref(newMd);
	SalMediaDescription *oldMd = resultDesc;
	resultDesc = newMd;

	if (getParams()->getMediaEncryption() == LinphoneMediaEncryptionDTLS) {
		getCurrentParams()->getPrivate()->setUpdateCallWhenIceCompleted(false);
		lInfo() << "Disabling update call when ice completed on call [" << q << "]";
	}

	if ((audioStream && (audioStream->ms.state == MSStreamStarted)) || (videoStream && (videoStream->ms.state == MSStreamStarted))) {
		clearEarlyMediaDestinations();

		/* We already started media: check if we really need to restart it */
		int mdChanged = 0;
		if (oldMd) {
			mdChanged = mediaParametersChanged(oldMd, newMd);
			/* Might not be mandatory to restart stream for each ice restart as it leads bad user experience, specially in video. See 0002495 for better background on this */
			if (mdChanged & (SAL_MEDIA_DESCRIPTION_CODEC_CHANGED
				| SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED
				| SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED
				| SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION)
			) {
				lInfo() << "Media descriptions are different, need to restart the streams";
			} else {
				for(int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; ++i) {
					if (!sal_stream_description_active(&oldMd->streams[i]) && !sal_stream_description_active(&newMd->streams[i])) continue;
					string streamTypeName = sal_stream_description_get_type_as_string(&newMd->streams[i]);

					/* If there was a change in the streams then newMd should have more streams */
					if (mdChanged & SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED && i >= oldMd->nb_streams) {
						lInfo() << "New " << streamTypeName << " stream detected, starting the stream";

						if (newMd->streams[i].type == SalAudio) {
							initializeAudioStream();
						} else if (newMd->streams[i].type == SalVideo) {
							initializeVideoStream();
						} else if (newMd->streams[i].type == SalText) {
							initializeTextStream();
						}

						if (getParams()->earlyMediaSendingEnabled() && (state == CallSession::State::OutgoingEarlyMedia)) {
							if (newMd->streams[i].type == SalAudio && audioStream)
								rtp_session_set_symmetric_rtp(audioStream->ms.sessions.rtp_session, false);
							else if (newMd->streams[i].type == SalVideo && videoStream)
								rtp_session_set_symmetric_rtp(videoStream->ms.sessions.rtp_session, false);
						}

						startStream(&newMd->streams[i], i, targetState);

						if (newMd->streams[i].type == SalAudio && audioStream) {
							if ((state == CallSession::State::Pausing) && pausedByApp && (q->getCore()->getCallCount() == 1))
								linphone_core_play_named_tone(q->getCore()->getCCore(), LinphoneToneCallOnHold);
						}

						updateStreamFrozenPayloads(&newMd->streams[i], &localDesc->streams[i]);

						continue;
					}

					int sdChanged = sal_stream_description_equals(&oldMd->streams[i], &newMd->streams[i]);

					if (newMd->streams[i].type == SalAudio && listener && listener->isPlayingRingbackTone(q->getSharedFromThis())) {
						lInfo() << "Playing ringback tone, will restart the audio stream";
						sdChanged |= SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION;
					}

					if (sdChanged & (SAL_MEDIA_DESCRIPTION_CODEC_CHANGED
						| SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED
						| SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED
						| SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION)
					) {
						lInfo() << "Stream descriptions are different, need to restart the " << streamTypeName << " stream";
						restartStream(&newMd->streams[i], i, sdChanged, targetState);
					} else {
						sdChanged |= mdChanged;

						if (newMd->streams[i].type == SalAudio && audioMuted && (targetState == CallSession::State::StreamsRunning)) {
							lInfo() << "Early media finished, unmuting audio input...";
							/* We were in early media, now we want to enable real media */
							audioMuted = false;

							if (audioStream) {
								linphone_core_enable_mic(q->getCore()->getCCore(), linphone_core_mic_enabled(q->getCore()->getCCore()));
							}
						}

#ifdef VIDEO_ENABLED
						if (newMd->streams[i].type == SalVideo && videoMuted && (targetState == CallSession::State::StreamsRunning)) {
							lInfo() << "Early media finished, unmuting video input...";
							/* We were in early media, now we want to enable real media */
							videoMuted = false;

							if (videoStream && cameraEnabled) {
								q->enableCamera(q->cameraEnabled());
							}
						}
#endif

						if (sdChanged == SAL_MEDIA_DESCRIPTION_UNCHANGED) {
							/* FIXME ZRTP, might be restarted in any cases? */
							lInfo() << "No need to restart the " << streamTypeName << " stream, SDP is unchanged";
						} else {
							if (sdChanged & SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED) {
								lInfo() << "Network parameters have changed for the " << streamTypeName << " stream, update it";
								if (newMd->streams[i].type == SalAudio || newMd->streams[i].type == SalVideo) {
									updateStreamDestination(newMd, &newMd->streams[i]);
								}
							}
							if (sdChanged & SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED) {
								lInfo() << "Crypto parameters have changed for the " << streamTypeName << " stream, update it";
								updateStreamCryptoParameters(&oldMd->streams[i], &newMd->streams[i]);
							}
						}
					}
				}

				for (const auto &hook : postProcessHooks) {
					hook();
				}
				postProcessHooks.clear();

				if (!iceAgent->hasCompleted()) {
					iceAgent->startConnectivityChecks();
				}

				if (oldMd)
					sal_media_description_unref(oldMd);

				return;
			}
		}

		stopStreams();
		if (mdChanged & SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED) {
			lInfo() << "Media ip type has changed, destroying sessions context on CallSession [" << q << "]";
			ms_media_stream_sessions_uninit(&sessions[mainAudioStreamIndex]);
			ms_media_stream_sessions_uninit(&sessions[mainVideoStreamIndex]);
			ms_media_stream_sessions_uninit(&sessions[mainTextStreamIndex]);
		}
		initializeStreams();
	}

	if (!audioStream) {
		/* This happens after pausing the call locally. The streams are destroyed and then we wait the 200Ok to recreate them */
		initializeStreams();
	}

	if (getParams()->earlyMediaSendingEnabled() && (state == CallSession::State::OutgoingEarlyMedia))
		prepareEarlyMediaForking();

	startStreams(targetState);

	if ((state == CallSession::State::Pausing) && pausedByApp && (q->getCore()->getCallCount() == 1))
		linphone_core_play_named_tone(q->getCore()->getCCore(), LinphoneToneCallOnHold);

	updateFrozenPayloads(newMd);

	upBandwidth = linphone_core_get_upload_bandwidth(q->getCore()->getCCore());

	if (oldMd)
		sal_media_description_unref(oldMd);
}

void MediaSessionPrivate::updateStreamDestination (SalMediaDescription *newMd, SalStreamDescription *newDesc) {
	if (!sal_stream_description_active(newDesc))
		return;

	if (newDesc && newDesc->type == SalAudio) {
		if (audioStream) {
			const char *rtpAddr = (newDesc->rtp_addr[0] != '\0') ? newDesc->rtp_addr : newMd->addr;
			const char *rtcpAddr = (newDesc->rtcp_addr[0] != '\0') ? newDesc->rtcp_addr : newMd->addr;
			lInfo() << "Change audio stream destination: RTP=" << rtpAddr << ":" << newDesc->rtp_port << " RTCP=" << rtcpAddr << ":" << newDesc->rtcp_port;
			rtp_session_set_remote_addr_full(audioStream->ms.sessions.rtp_session, rtpAddr, newDesc->rtp_port, rtcpAddr, newDesc->rtcp_port);
		}
	}
#ifdef VIDEO_ENABLED
	else if (newDesc && newDesc->type == SalVideo) {
		if (videoStream) {
			const char *rtpAddr = (newDesc->rtp_addr[0] != '\0') ? newDesc->rtp_addr : newMd->addr;
			const char *rtcpAddr = (newDesc->rtcp_addr[0] != '\0') ? newDesc->rtcp_addr : newMd->addr;
			lInfo() << "Change video stream destination: RTP=" << rtpAddr << ":" << newDesc->rtp_port << " RTCP=" << rtcpAddr << ":" << newDesc->rtcp_port;
			rtp_session_set_remote_addr_full(videoStream->ms.sessions.rtp_session, rtpAddr, newDesc->rtp_port, rtcpAddr, newDesc->rtcp_port);
		}
	}
#endif
}

void MediaSessionPrivate::updateStreamsDestinations (SalMediaDescription *oldMd, SalMediaDescription *newMd) {
	SalStreamDescription *newAudioDesc = nullptr;

	#ifdef VIDEO_ENABLED
		SalStreamDescription *newVideoDesc = nullptr;
	#endif

	for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		if (!sal_stream_description_active(&newMd->streams[i]))
			continue;
		if (newMd->streams[i].type == SalAudio)
			newAudioDesc = &newMd->streams[i];

		#ifdef VIDEO_ENABLED
			else if (newMd->streams[i].type == SalVideo)
				newVideoDesc = &newMd->streams[i];
		#endif
	}

	updateStreamDestination(newMd, newAudioDesc);
#ifdef VIDEO_ENABLED
	updateStreamDestination(newMd, newVideoDesc);
#endif
}

// -----------------------------------------------------------------------------

bool MediaSessionPrivate::allStreamsAvpfEnabled () const {
	int nbActiveStreams = 0;
	int nbAvpfEnabledStreams = 0;
	if (audioStream && media_stream_get_state(&audioStream->ms) == MSStreamStarted) {
		nbActiveStreams++;
		if (media_stream_avpf_enabled(&audioStream->ms))
			nbAvpfEnabledStreams++;
	}
	if (videoStream && media_stream_get_state(&videoStream->ms) == MSStreamStarted) {
		nbActiveStreams++;
		if (media_stream_avpf_enabled(&videoStream->ms))
			nbAvpfEnabledStreams++;
	}
	return (nbActiveStreams > 0) && (nbActiveStreams == nbAvpfEnabledStreams);
}

bool MediaSessionPrivate::allStreamsEncrypted () const {
	int numberOfEncryptedStreams = 0;
	int numberOfActiveStreams = 0;
	if (audioStream && (media_stream_get_state(&audioStream->ms) == MSStreamStarted)) {
		numberOfActiveStreams++;
		if (media_stream_secured(&audioStream->ms))
			numberOfEncryptedStreams++;
	}
	if (videoStream && (media_stream_get_state(&videoStream->ms) == MSStreamStarted)) {
		numberOfActiveStreams++;
		if (media_stream_secured(&videoStream->ms))
			numberOfEncryptedStreams++;
	}
	if (textStream && (media_stream_get_state(&textStream->ms) == MSStreamStarted)) {
		numberOfActiveStreams++;
		if (media_stream_secured(&textStream->ms))
			numberOfEncryptedStreams++;
	}
	return (numberOfActiveStreams > 0) && (numberOfActiveStreams == numberOfEncryptedStreams);
}

bool MediaSessionPrivate::atLeastOneStreamStarted () const {
	return (audioStream && (media_stream_get_state(&audioStream->ms) == MSStreamStarted))
		|| (videoStream && (media_stream_get_state(&videoStream->ms) == MSStreamStarted))
		|| (textStream && (media_stream_get_state(&textStream->ms) == MSStreamStarted));
}

void MediaSessionPrivate::audioStreamAuthTokenReady (const string &authToken, bool verified) {
	this->authToken = authToken;
	authTokenVerified = verified;
	lInfo() << "Authentication token is " << authToken << "(" << (verified ? "verified" : "unverified") << ")";
}

void MediaSessionPrivate::audioStreamEncryptionChanged (bool encrypted) {
	propagateEncryptionChanged();

	#ifdef VIDEO_ENABLED
		L_Q();
		/* Enable video encryption */
		if ((getParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP) && q->getCurrentParams()->videoEnabled()) {
			lInfo() << "Trying to start ZRTP encryption on video stream";
			video_stream_start_zrtp(videoStream);
		}
	#endif
}

uint16_t MediaSessionPrivate::getAvpfRrInterval () const {
	uint16_t rrInterval = 0;
	if (audioStream && (media_stream_get_state(&audioStream->ms) == MSStreamStarted)) {
		uint16_t streamRrInterval = media_stream_get_avpf_rr_interval(&audioStream->ms);
		if (streamRrInterval > rrInterval) rrInterval = streamRrInterval;
	}
	if (videoStream && (media_stream_get_state(&videoStream->ms) == MSStreamStarted)) {
		uint16_t streamRrInterval = media_stream_get_avpf_rr_interval(&videoStream->ms);
		if (streamRrInterval > rrInterval) rrInterval = streamRrInterval;
	}
	return rrInterval;
}

unsigned int MediaSessionPrivate::getNbActiveStreams () const {
	SalMediaDescription *md = nullptr;
	if (op)
		md = op->getRemoteMediaDescription();
	if (!md)
		return 0;
	return sal_media_description_nb_active_streams_of_type(md, SalAudio) + sal_media_description_nb_active_streams_of_type(md, SalVideo) + sal_media_description_nb_active_streams_of_type(md, SalText);
}

bool MediaSessionPrivate::isEncryptionMandatory () const {
	L_Q();
	if (getParams()->getMediaEncryption() == LinphoneMediaEncryptionDTLS) {
		lInfo() << "Forced encryption mandatory on CallSession [" << q << "] due to SRTP-DTLS";
		return true;
	}
	return getParams()->mandatoryMediaEncryptionEnabled();
}

int MediaSessionPrivate::mediaParametersChanged (SalMediaDescription *oldMd, SalMediaDescription *newMd) {
	L_Q();
	if (forceStreamsReconstruction) {
		forceStreamsReconstruction = false;
		return SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION;
	}
	if (getParams()->getPrivate()->getInConference() != getCurrentParams()->getPrivate()->getInConference())
		return SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION;
	if (upBandwidth != linphone_core_get_upload_bandwidth(q->getCore()->getCCore()))
		return SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION;
	if (localDescChanged) {
		char *differences = sal_media_description_print_differences(localDescChanged);
		lInfo() << "Local description has changed: " << differences;
		ms_free(differences);
	}
	int otherDescChanged = sal_media_description_global_equals(oldMd, newMd);
	if (otherDescChanged) {
		char *differences = sal_media_description_print_differences(otherDescChanged);
		lInfo() << "Other description has changed: " << differences;
		ms_free(differences);
	}
	return localDescChanged | otherDescChanged;
}

void MediaSessionPrivate::propagateEncryptionChanged () {
	L_Q();
	
	if (!getStreamsGroup().allStreamsEncrypted()) {
		lInfo() << "Some streams are not encrypted";
		getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
		if (listener)
			listener->onEncryptionChanged(q->getSharedFromThis(), false, authToken);
	} else {
		if (!authToken.empty()) {
			/* ZRTP only is using auth_token */
			getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionZRTP);
			char *peerDeviceId = nullptr;
			auto encryptionEngine = q->getCore()->getEncryptionEngine();
			if (encryptionEngine && authTokenVerified) {
				const SalAddress *remoteAddress = getOp()->getRemoteContactAddress();
				peerDeviceId = sal_address_as_string_uri_only(remoteAddress);
				Stream *stream = mainAudioStreamIndex != -1 ? getStreamsGroup().getStream(mainAudioStreamIndex) : nullptr;
				if (stream){
					MS2Stream *ms2s = dynamic_cast<MS2Stream*>(stream);
					if (ms2s){
						encryptionEngine->authenticationVerified(ms2s->getZrtpContext(), op->getRemoteMediaDescription(), peerDeviceId);
					}else{
						lError() << "Could not dynamic_cast to MS2Stream in propagateEncryptionChanged().";
					}
				}
				ms_free(peerDeviceId);
			}
		} else {
			/* Otherwise it must be DTLS as SDES doesn't go through this function */
			getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionDTLS);
		}

		lInfo() << "All streams are encrypted, key exchanged using "
			<< ((q->getCurrentParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP) ? "ZRTP"
				: (q->getCurrentParams()->getMediaEncryption() == LinphoneMediaEncryptionDTLS) ? "DTLS" : "Unknown mechanism");
		if (listener)
			listener->onEncryptionChanged(q->getSharedFromThis(), true, authToken);

		Stream *videoStream = mainVideoStreamIndex != -1 ? getStreamsGroup().getStream(mainVideoStreamIndex) : nullptr;
		if (isEncryptionMandatory() && videoStream && videoStream->getState() == Stream::Running) {
			/* Nothing could have been sent yet so generating key frame */
			VideoControlInterface *vc = dynamic_cast<VideoControlInterface*> (videoStream);
			if (vc) vc->sendVfu();
		}
	}
}

MSWebCam *MediaSessionPrivate::getVideoDevice()const{
	L_D();
	MS2VideoStream *vs = d->getStreamsGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);
	if (vs) return vs->getVideoDevice(state);
	return nullptr;
}

// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------

void MediaSessionPrivate::executeBackgroundTasks (bool oneSecondElapsed) {
	L_Q();
	switch (state) {
	case CallSession::State::StreamsRunning:
	case CallSession::State::OutgoingEarlyMedia:
	case CallSession::State::IncomingEarlyMedia:
	case CallSession::State::PausedByRemote:
	case CallSession::State::Paused:
		if (oneSecondElapsed) {
			float audioLoad = 0.f;
			float videoLoad = 0.f;
			float textLoad = 0.f;
			if (audioStream && audioStream->ms.sessions.ticker)
				audioLoad = ms_ticker_get_average_load(audioStream->ms.sessions.ticker);
			if (videoStream && videoStream->ms.sessions.ticker)
				videoLoad = ms_ticker_get_average_load(videoStream->ms.sessions.ticker);
			if (textStream && textStream->ms.sessions.ticker)
				textLoad = ms_ticker_get_average_load(textStream->ms.sessions.ticker);
			reportBandwidth();
			lInfo() << "Thread processing load: audio=" << audioLoad << "\tvideo=" << videoLoad << "\ttext=" << textLoad;
		}
		break;
	default:
		/* No stats for other states */
		break;
	}
	if (listener)
		listener->onNoMediaTimeoutCheck(q->getSharedFromThis(), oneSecondElapsed);
}

void MediaSessionPrivate::reportBandwidth () {
	L_Q();
	reportBandwidthForStream(&audioStream->ms, LinphoneStreamTypeAudio);
	reportBandwidthForStream(&videoStream->ms, LinphoneStreamTypeVideo);
	reportBandwidthForStream(&textStream->ms, LinphoneStreamTypeText);

	lInfo() << "Bandwidth usage for CallSession [" << q << "]:\n" << fixed << setprecision(2) <<
		"\tRTP  audio=[d=" << linphone_call_stats_get_download_bandwidth(audioStats) << ",u=" << linphone_call_stats_get_upload_bandwidth(audioStats) <<
		"], video=[d=" << linphone_call_stats_get_download_bandwidth(videoStats) << ",u=" << linphone_call_stats_get_upload_bandwidth(videoStats) << ",ed=" << linphone_call_stats_get_estimated_download_bandwidth(videoStats) <<
		"], text=[d=" << linphone_call_stats_get_download_bandwidth(textStats) << ",u=" << linphone_call_stats_get_upload_bandwidth(textStats) << "] kbits/sec\n" <<
		"\tRTCP audio=[d=" << linphone_call_stats_get_rtcp_download_bandwidth(audioStats) << ",u=" << linphone_call_stats_get_rtcp_upload_bandwidth(audioStats) <<
		"], video=[d=" << linphone_call_stats_get_rtcp_download_bandwidth(videoStats) << ",u=" << linphone_call_stats_get_rtcp_upload_bandwidth(videoStats) <<
		"], text=[d=" << linphone_call_stats_get_rtcp_download_bandwidth(textStats) << ",u=" << linphone_call_stats_get_rtcp_upload_bandwidth(textStats) << "] kbits/sec";
}

void MediaSessionPrivate::reportBandwidthForStream (MediaStream *ms, LinphoneStreamType type) {
	L_Q();
	LinphoneCallStats *stats = nullptr;
	if (type == LinphoneStreamTypeAudio) {
		stats = audioStats;
	} else if (type == LinphoneStreamTypeVideo) {
		stats = videoStats;
	} else if (type == LinphoneStreamTypeText) {
		stats = textStats;
	} else
		return;

	bool active = ms ? (media_stream_get_state(ms) == MSStreamStarted) : false;
	_linphone_call_stats_set_download_bandwidth(stats, active ? (float)(media_stream_get_down_bw(ms) * 1e-3) : 0.f);
	_linphone_call_stats_set_upload_bandwidth(stats, active ? (float)(media_stream_get_up_bw(ms) * 1e-3) : 0.f);
	_linphone_call_stats_set_rtcp_download_bandwidth(stats, active ? (float)(media_stream_get_rtcp_down_bw(ms) * 1e-3) : 0.f);
	_linphone_call_stats_set_rtcp_upload_bandwidth(stats, active ? (float)(media_stream_get_rtcp_up_bw(ms) * 1e-3) : 0.f);
	_linphone_call_stats_set_ip_family_of_remote(stats,
		active ? (ortp_stream_is_ipv6(&ms->sessions.rtp_session->rtp.gs) ? LinphoneAddressFamilyInet6 : LinphoneAddressFamilyInet) : LinphoneAddressFamilyUnspec);

	if (q->getCore()->getCCore()->send_call_stats_periodical_updates) {
		if (active)
			linphone_call_stats_update(stats, ms);
		_linphone_call_stats_set_updated(stats, _linphone_call_stats_get_updated(stats) | LINPHONE_CALL_STATS_PERIODICAL_UPDATE);
		if (listener)
			listener->onStatsUpdated(q->getSharedFromThis(), stats);
		_linphone_call_stats_set_updated(stats, 0);
	}
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::abort (const string &errorMsg) {
	L_Q();
	if (listener)
		listener->onStopRinging(q->getSharedFromThis());
	stopStreams();
	CallSessionPrivate::abort(errorMsg);
}

void MediaSessionPrivate::handleIncomingReceivedStateInIncomingNotification () {
	L_Q();

	/* Try to be best-effort in giving real local or routable contact address for 100Rel case */
	setContactOp();
	bool proposeEarlyMedia = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "incoming_calls_early_media", false);
	if (proposeEarlyMedia)
		q->acceptEarlyMedia();
	else
		op->notifyRinging(false);

	acceptOrTerminateReplacedSessionInIncomingNotification();
}

bool MediaSessionPrivate::isReadyForInvite () const {
	bool callSessionReady = CallSessionPrivate::isReadyForInvite();
	bool iceReady = false;
	if (iceAgent->hasSession()) {
		if (iceAgent->candidatesGathered())
			iceReady = true;
	} else
		iceReady = true;
	return callSessionReady && iceReady;
}

LinphoneStatus MediaSessionPrivate::pause () {
	L_Q();
	if ((state != CallSession::State::StreamsRunning) && (state != CallSession::State::PausedByRemote)) {
		lWarning() << "Cannot pause this MediaSession, it is not active";
		return -1;
	}
	string subject;
	if (sal_media_description_has_dir(resultDesc, SalStreamSendRecv))
		subject = "Call on hold";
	else if (sal_media_description_has_dir(resultDesc, SalStreamRecvOnly))
		subject = "Call on hold for me too";
	else {
		lError() << "No reason to pause this call, it is already paused or inactive";
		return -1;
	}
	broken = false;
	setState(CallSession::State::Pausing, "Pausing call");
	makeLocalMediaDescription();
	op->update(subject.c_str(), false);
	if (listener)
		listener->onResetCurrentSession(q->getSharedFromThis());
	if (audioStream || videoStream || textStream)
		stopStreams();
	pausedByApp = false;
	return 0;
}

int MediaSessionPrivate::restartInvite () {
	stopStreams();
	initializeStreams();
	return CallSessionPrivate::restartInvite();
}

void MediaSessionPrivate::setTerminated () {
	freeResources();
	CallSessionPrivate::setTerminated();
}

LinphoneStatus MediaSessionPrivate::startAcceptUpdate (CallSession::State nextState, const string &stateInfo) {
	if (iceAgent->hasSession() && (iceAgent->getNbLosingPairs() > 0)) {
		/* Defer the sending of the answer until there are no losing pairs left */
		return 0;
	}
	
	updateRemoteSessionIdAndVer();
	makeLocalMediaDescription();
	op->accept();
	SalMediaDescription *md = op->getFinalMediaDescription();
	iceAgent->stopIceForInactiveStreams(md);
	if (md && !sal_media_description_empty(md))
		updateStreams(md, nextState);
	setState(nextState, stateInfo);
	return 0;
}

LinphoneStatus MediaSessionPrivate::startUpdate (const string &subject) {
	L_Q();
	fillMulticastMediaAddresses();
	if (!getParams()->getPrivate()->getNoUserConsent())
		makeLocalMediaDescription();
	if (q->getCore()->getCCore()->sip_conf.sdp_200_ack)
		op->setLocalMediaDescription(nullptr);
	LinphoneStatus result = CallSessionPrivate::startUpdate(subject);
	if (q->getCore()->getCCore()->sip_conf.sdp_200_ack) {
		// We are NOT offering, set local media description after sending the call so that we are ready to
		// process the remote offer when it will arrive.
		op->setLocalMediaDescription(localDesc);
	}
	return result;
}

void MediaSessionPrivate::terminate () {
	L_Q();
	if (listener)
		listener->onStopRingingIfNeeded(q->getSharedFromThis());

	stopStreams();
	CallSessionPrivate::terminate();
}

void MediaSessionPrivate::updateCurrentParams () const {
	CallSessionPrivate::updateCurrentParams();

	LinphoneVideoDefinition *vdef = linphone_video_definition_new(MS_VIDEO_SIZE_UNKNOWN_W, MS_VIDEO_SIZE_UNKNOWN_H, nullptr);
	getCurrentParams()->getPrivate()->setSentVideoDefinition(vdef);
	getCurrentParams()->getPrivate()->setReceivedVideoDefinition(vdef);
	linphone_video_definition_unref(vdef);
#ifdef VIDEO_ENABLED
	if (videoStream) {
		MSVideoSize vsize = video_stream_get_sent_video_size(videoStream);
		vdef = linphone_video_definition_new(static_cast<unsigned int>(vsize.width), static_cast<unsigned int>(vsize.height), nullptr);
		getCurrentParams()->getPrivate()->setSentVideoDefinition(vdef);
		linphone_video_definition_unref(vdef);
		vsize = video_stream_get_received_video_size(videoStream);
		vdef = linphone_video_definition_new(static_cast<unsigned int>(vsize.width), static_cast<unsigned int>(vsize.height), nullptr);
		getCurrentParams()->getPrivate()->setReceivedVideoDefinition(vdef);
		linphone_video_definition_unref(vdef);
		getCurrentParams()->getPrivate()->setSentFps(video_stream_get_sent_framerate(videoStream));
		getCurrentParams()->getPrivate()->setReceivedFps(video_stream_get_received_framerate(videoStream));
	}
#endif

	/* REVISITED
	 * Previous code was buggy.
	 * Relying on the mediastream's state (added by jehan: only) to know the current encryption is unreliable.
	 * For (added by jehan: both DTLS and) ZRTP it is though necessary.
	 * But for all others the current_params->media_encryption state should reflect (added by jehan: both) what is agreed by the offer/answer
	 * mechanism  (added by jehan: and encryption status from media which is much stronger than only result of offer/answer )
	 * Typically there can be inactive streams for which the media layer has no idea of whether they are encrypted or not.
	 */

	switch (getParams()->getMediaEncryption()) {
		case LinphoneMediaEncryptionZRTP:
			if (atLeastOneStreamStarted()) {
				if (allStreamsEncrypted() && !authToken.empty())
					getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionZRTP);
				else {
					/* To avoid too many traces */
					lDebug() << "Encryption was requested to be " << linphone_media_encryption_to_string(getParams()->getMediaEncryption())
						<< ", but isn't effective (allStreamsEncrypted=" << allStreamsEncrypted() << ", auth_token=" << authToken << ")";
					getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
				}
			} /* else don't update the state if all streams are shutdown */
			break;
		case LinphoneMediaEncryptionDTLS:
		case LinphoneMediaEncryptionSRTP:
			if (atLeastOneStreamStarted()) {
				if ((getNbActiveStreams() == 0) || allStreamsEncrypted())
					getCurrentParams()->setMediaEncryption(getParams()->getMediaEncryption());
				else {
					/* To avoid to many traces */
					lDebug() << "Encryption was requested to be " << linphone_media_encryption_to_string(getParams()->getMediaEncryption())
						<< ", but isn't effective (allStreamsEncrypted=" << allStreamsEncrypted() << ")";
					getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
				}
			} /* else don't update the state if all streams are shutdown */
			break;
		case LinphoneMediaEncryptionNone:
			/* Check if we actually switched to ZRTP */
			if (atLeastOneStreamStarted() && allStreamsEncrypted() && !authToken.empty())
				getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionZRTP);
			else
				getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
			break;
	}
	SalMediaDescription *md = resultDesc;
	getCurrentParams()->enableAvpf(allStreamsAvpfEnabled() && sal_media_description_has_avpf(md));
	if (getCurrentParams()->avpfEnabled())
		getCurrentParams()->setAvpfRrInterval(getAvpfRrInterval());
	else
		getCurrentParams()->setAvpfRrInterval(0);
	if (md) {
		SalStreamDescription *sd = sal_media_description_find_best_stream(md, SalAudio);
		getCurrentParams()->setAudioDirection(sd ? MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd->dir) : LinphoneMediaDirectionInactive);
		if (getCurrentParams()->getAudioDirection() != LinphoneMediaDirectionInactive) {
			const char *rtpAddr = (sd->rtp_addr[0] != '\0') ? sd->rtp_addr : md->addr;
			getCurrentParams()->enableAudioMulticast(!!ms_is_multicast(rtpAddr));
		} else
			getCurrentParams()->enableAudioMulticast(false);
		sd = sal_media_description_find_best_stream(md, SalVideo);
		getCurrentParams()->getPrivate()->enableImplicitRtcpFb(sd && sal_stream_description_has_implicit_avpf(sd));
		getCurrentParams()->setVideoDirection(sd ? MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd->dir) : LinphoneMediaDirectionInactive);
		if (getCurrentParams()->getVideoDirection() != LinphoneMediaDirectionInactive) {
			const char *rtpAddr = (sd->rtp_addr[0] != '\0') ? sd->rtp_addr : md->addr;
			getCurrentParams()->enableVideoMulticast(!!ms_is_multicast(rtpAddr));
		} else
			getCurrentParams()->enableVideoMulticast(false);
	}
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::accept (const MediaSessionParams *msp, bool wasRinging) {
	L_Q();
	if (msp) {
		setParams(new MediaSessionParams(*msp));
		iceAgent->prepare(localDesc, true, false /*we don't allow gathering now, it must have been done before*/);
		makeLocalMediaDescription();
	}

	updateRemoteSessionIdAndVer();

	/* Give a chance a set card prefered sampling frequency */
	if (localDesc->streams[0].max_rate > 0) {
		lInfo() << "Configuring prefered card sampling rate to [" << localDesc->streams[0].max_rate << "]";
		if (q->getCore()->getCCore()->sound_conf.play_sndcard)
			ms_snd_card_set_preferred_sample_rate(q->getCore()->getCCore()->sound_conf.play_sndcard, localDesc->streams[0].max_rate);
		if (q->getCore()->getCCore()->sound_conf.capt_sndcard)
			ms_snd_card_set_preferred_sample_rate(q->getCore()->getCCore()->sound_conf.capt_sndcard, localDesc->streams[0].max_rate);
	}

	LinphoneCore *lc = q->getCore()->getCCore();
	if (!wasRinging && (audioStream->ms.state == MSStreamInitialized) && !lc->use_files) {
		audio_stream_prepare_sound(audioStream, lc->sound_conf.play_sndcard, lc->sound_conf.capt_sndcard);
	}

	CallSessionPrivate::accept(nullptr);

	SalMediaDescription *newMd = op->getFinalMediaDescription();
	iceAgent->stopIceForInactiveStreams(newMd);
	if (newMd) {
		updateStreams(newMd, CallSession::State::StreamsRunning);
		setState(CallSession::State::StreamsRunning, "Connected (streams running)");
	} else
		expectMediaInAck = true;
}

LinphoneStatus MediaSessionPrivate::acceptUpdate (const CallSessionParams *csp, CallSession::State nextState, const string &stateInfo) {
	L_Q();
	SalMediaDescription *desc = op->getRemoteMediaDescription();
	bool keepSdpVersion = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "keep_sdp_version", 0);
	if (keepSdpVersion && (desc->session_id == remoteSessionId) && (desc->session_ver == remoteSessionVer)) {
		/* Remote has sent an INVITE with the same SDP as before, so send a 200 OK with the same SDP as before. */
		lWarning() << "SDP version has not changed, send same SDP as before";
		op->accept();
		setState(nextState, stateInfo);
		return 0;
	}
	if (csp)
		setParams(new MediaSessionParams(*static_cast<const MediaSessionParams *>(csp)));
	else {
		if (!op->isOfferer()) {
			/* Reset call params for multicast because this param is only relevant when offering */
			getParams()->enableAudioMulticast(false);
			getParams()->enableVideoMulticast(false);
		}
	}
	if (getParams()->videoEnabled() && !linphone_core_video_enabled(q->getCore()->getCCore())) {
		lWarning() << "Requested video but video support is globally disabled. Refusing video";
		getParams()->enableVideo(false);
	}
	if (q->getCurrentParams()->getPrivate()->getInConference()) {
		lWarning() << "Video isn't supported in conference";
		getParams()->enableVideo(false);
	}
	/* Update multicast params according to call params */
	fillMulticastMediaAddresses();
	iceAgent->checkSession(IR_Controlled, true);
	if (iceAgent->prepare(localDesc, true))
		return 0; /* Deferred until completion of ICE gathering */
	startAcceptUpdate(nextState, stateInfo);
	return 0;
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::refreshSockets () {
	for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
		MSMediaStreamSessions *mss = &sessions[i];
		if (mss->rtp_session)
			rtp_session_refresh_sockets(mss->rtp_session);
	}
}

void MediaSessionPrivate::reinviteToRecoverFromConnectionLoss () {
	L_Q();
	lInfo() << "MediaSession [" << q << "] is going to be updated (reINVITE) in order to recover from lost connectivity";
	if (iceAgent->hasSession())
		iceAgent->resetSession(IR_Controlling);
	q->update(getParams());
}

void MediaSessionPrivate::repairByInviteWithReplaces () {
	if ((state == CallSession::State::IncomingEarlyMedia) || (state == CallSession::State::OutgoingEarlyMedia)) {
		stopStreams();
		initializeStreams();
	}
	CallSessionPrivate::repairByInviteWithReplaces();
}

// -----------------------------------------------------------------------------

#ifdef VIDEO_ENABLED
void MediaSessionPrivate::videoStreamEventCb (const MSFilter *f, const unsigned int eventId, const void *args) {
	L_Q();
	switch (eventId) {
		case MS_VIDEO_DECODER_DECODING_ERRORS:
			lWarning() << "MS_VIDEO_DECODER_DECODING_ERRORS";
			if (videoStream && video_stream_is_decoding_error_to_be_reported(videoStream, 5000)) {
				video_stream_decoding_error_reported(videoStream);
				q->sendVfuRequest();
			}
			break;
		case MS_VIDEO_DECODER_RECOVERED_FROM_ERRORS:
			lInfo() << "MS_VIDEO_DECODER_RECOVERED_FROM_ERRORS";
			if (videoStream)
				video_stream_decoding_error_recovered(videoStream);
			break;
		case MS_VIDEO_DECODER_FIRST_IMAGE_DECODED:
			lInfo() << "First video frame decoded successfully";
			if (listener)
				listener->onFirstVideoFrameDecoded(q->getSharedFromThis());
			break;
		case MS_VIDEO_DECODER_SEND_PLI:
		case MS_VIDEO_DECODER_SEND_SLI:
		case MS_VIDEO_DECODER_SEND_RPSI:
			/* Handled internally by mediastreamer2 */
			break;
		case MS_CAMERA_PREVIEW_SIZE_CHANGED: {
			MSVideoSize size = *(MSVideoSize *)args;
			lInfo() << "Camera video preview size changed: " << size.width << "x" << size.height;
			linphone_core_resize_video_preview(q->getCore()->getCCore(), size.width, size.height);
			break;
		}
		default:
			lWarning() << "Unhandled event " << eventId;
			break;
	}
}
#endif

void MediaSessionPrivate::realTimeTextCharacterReceived (MSFilter *f, unsigned int id, void *arg) {
	L_Q();
	if (id == MS_RTT_4103_RECEIVED_CHAR) {
		RealtimeTextReceivedCharacter *data = reinterpret_cast<RealtimeTextReceivedCharacter *>(arg);
		if (listener)
			listener->onRealTimeTextCharacterReceived(q->getSharedFromThis(), data);
	}
}

int MediaSessionPrivate::sendDtmf () {
	L_Q();
	LinphoneCore *lc = q->getCore()->getCCore();
	// By default we send DTMF RFC2833 if we do not have enabled SIP_INFO but we can also send RFC2833 and SIP_INFO
	if (linphone_core_get_use_rfc2833_for_dtmf(lc) || !linphone_core_get_use_info_for_dtmf(lc)) {
		// In Band DTMF
		if (audioStream)
			audio_stream_send_dtmf(audioStream, dtmfSequence.front());
		else {
			lError() << "Cannot send RFC2833 DTMF when we are not in communication";
			return FALSE;
		}
	}
	if (linphone_core_get_use_info_for_dtmf(lc)) {
		// Out of Band DTMF (use INFO method)
		op->sendDtmf(dtmfSequence.front());
	}

	dtmfSequence.erase(0, 1);
	// Continue only if the dtmf sequence is not empty
	if (!dtmfSequence.empty())
		return TRUE;
	else {
		q->cancelDtmfs();
		return FALSE;
	}
}

// -----------------------------------------------------------------------------

int MediaSessionPrivate::resumeAfterFailedTransfer (void *userData, unsigned int) {
	MediaSession *session = reinterpret_cast<MediaSession *>(userData);
	return session->getPrivate()->resumeAfterFailedTransfer();
}

bool_t MediaSessionPrivate::startPendingRefer (void *userData) {
	MediaSession *session = reinterpret_cast<MediaSession *>(userData);
	session->getPrivate()->startPendingRefer();
	return TRUE;
}

void MediaSessionPrivate::stunAuthRequestedCb (const char *realm, const char *nonce, const char **username, const char **password, const char **ha1) {
	L_Q();
	/* Get the username from the nat policy or the proxy config */
	LinphoneProxyConfig *proxy = nullptr;
	if (destProxy)
		proxy = destProxy;
	else
		proxy = linphone_core_get_default_proxy_config(q->getCore()->getCCore());
	if (!proxy)
		return;
	const char * user = nullptr;
	LinphoneNatPolicy *proxyNatPolicy = linphone_proxy_config_get_nat_policy(proxy);
	if (proxyNatPolicy)
		user = linphone_nat_policy_get_stun_server_username(proxyNatPolicy);
	else if (natPolicy)
		user = linphone_nat_policy_get_stun_server_username(natPolicy);
	if (!user) {
		/* If the username has not been found in the nat_policy, take the username from the currently used proxy config */
		const LinphoneAddress *addr = linphone_proxy_config_get_identity_address(proxy);
		if (!addr)
			return;
		user = linphone_address_get_username(addr);
	}
	if (!user)
		return;

	const LinphoneAuthInfo *authInfo = linphone_core_find_auth_info(q->getCore()->getCCore(), realm, user, nullptr);
	if (!authInfo) {
		lWarning() << "No auth info found for STUN auth request";
		return;
	}
	const char *hash = linphone_auth_info_get_ha1(authInfo);
	if (hash)
			*ha1 = hash;
	else
		*password = linphone_auth_info_get_passwd(authInfo);
	*username = user;
}

// =============================================================================

MediaSession::MediaSession (const shared_ptr<Core> &core, shared_ptr<Participant> me, const CallSessionParams *params, CallSessionListener *listener)
	: CallSession(*new MediaSessionPrivate, core) {
	L_D();
	d->me = me;
	d->listener = listener;

	if (params)
		d->setParams(new MediaSessionParams(*(reinterpret_cast<const MediaSessionParams *>(params))));
	else
		d->setParams(new MediaSessionParams());
	d->setCurrentParams(new MediaSessionParams());
	d->streamsGroup = makeUnique<StreamsGroup>(*this);

	lInfo() << "New MediaSession [" << this << "] initialized (LinphoneCore version: " << linphone_core_get_version() << ")";
}

MediaSession::~MediaSession () {
	L_D();
	cancelDtmfs();
	d->freeResources();
	if (d->natPolicy)
		linphone_nat_policy_unref(d->natPolicy);
	if (d->localDesc)
		sal_media_description_unref(d->localDesc);
	if (d->biggestDesc)
		sal_media_description_unref(d->biggestDesc);
	if (d->resultDesc)
		sal_media_description_unref(d->resultDesc);
}

// -----------------------------------------------------------------------------

void MediaSession::acceptDefault(){
	accept();
}

LinphoneStatus MediaSession::accept (const MediaSessionParams *msp) {
	L_D();
	LinphoneStatus result = d->checkForAcceptation();
	if (result < 0) return result;

	bool wasRinging = false;
	if (d->listener)
		wasRinging = d->listener->onCallSessionAccepted(getSharedFromThis());

	d->accept(msp, wasRinging);
	lInfo() << "CallSession accepted";
	return 0;
}

LinphoneStatus MediaSession::acceptEarlyMedia (const MediaSessionParams *msp) {
	L_D();
	if (d->state != CallSession::State::IncomingReceived) {
		lError() << "Bad state " << Utils::toString(d->state) << " for MediaSession::acceptEarlyMedia()";
		return -1;
	}
	/* Try to be best-effort in giving real local or routable contact address for 100Rel case */
	d->setContactOp();
	/* If parameters are passed, update the media description */
	if (msp) {
		d->setParams(new MediaSessionParams(*msp));
		d->makeLocalMediaDescription();
		d->op->setSentCustomHeaders(d->getParams()->getPrivate()->getCustomHeaders());
	}
	d->op->notifyRinging(true);
	d->setState(CallSession::State::IncomingEarlyMedia, "Incoming call early media");
	SalMediaDescription *md = d->op->getFinalMediaDescription();
	if (md)
		d->updateStreams(md, d->state);
	return 0;
}

LinphoneStatus MediaSession::acceptUpdate (const MediaSessionParams *msp) {
	L_D();
	if (d->expectMediaInAck) {
		lError() << "MediaSession::acceptUpdate() is not possible during a late offer incoming reINVITE (INVITE without SDP)";
		return -1;
	}
	return CallSession::acceptUpdate(msp);
}

void MediaSession::cancelDtmfs () {
	L_D();
	if (!d->dtmfTimer)
		return;

	getCore()->getCCore()->sal->cancelTimer(d->dtmfTimer);
	belle_sip_object_unref(d->dtmfTimer);
	d->dtmfTimer = nullptr;
	d->dtmfSequence.clear();
}

void MediaSession::configure (LinphoneCallDir direction, LinphoneProxyConfig *cfg, SalCallOp *op, const Address &from, const Address &to) {
	L_D();
	CallSession::configure (direction, cfg, op, from, to);

	if (d->destProxy)
		d->natPolicy = linphone_proxy_config_get_nat_policy(d->destProxy);
	if (!d->natPolicy)
		d->natPolicy = linphone_core_get_nat_policy(getCore()->getCCore());
	linphone_nat_policy_ref(d->natPolicy);

	if (direction == LinphoneCallOutgoing) {
		d->selectOutgoingIpVersion();
		d->getLocalIp(to);
		d->getCurrentParams()->getPrivate()->setUpdateCallWhenIceCompleted(d->getParams()->getPrivate()->getUpdateCallWhenIceCompleted());
		d->fillMulticastMediaAddresses();
		d->makeLocalMediaDescription();
		if (d->natPolicy && linphone_nat_policy_ice_enabled(d->natPolicy))
			d->iceAgent->checkSession(IR_Controlling, false);
		d->runStunTestsIfNeeded();
		d->discoverMtu(to);
	} else if (direction == LinphoneCallIncoming) {
		d->selectIncomingIpVersion();
		/* Note that the choice of IP version for streams is later refined by setCompatibleIncomingCallParams() when examining the
		 * remote offer, if any. If the remote offer contains IPv4 addresses, we should propose IPv4 as well. */
		Address cleanedFrom = from;
		cleanedFrom.clean();
		d->getLocalIp(cleanedFrom);
		d->setParams(new MediaSessionParams());
		d->params->initDefault(getCore());
		d->initializeParamsAccordingToIncomingCallParams();
		d->makeLocalMediaDescription();
		SalMediaDescription *md = d->op->getRemoteMediaDescription();
		if (d->natPolicy && linphone_nat_policy_ice_enabled(d->natPolicy)) {
			if (md) {
				/* Create the ice session now if ICE is required */
				d->iceAgent->checkSession(IR_Controlled, false);
			} else {
				linphone_nat_policy_unref(d->natPolicy);
				d->natPolicy = nullptr;
				lWarning() << "ICE not supported for incoming INVITE without SDP";
			}
		}
		if (d->natPolicy)
			d->runStunTestsIfNeeded();
		d->discoverMtu(cleanedFrom);
	}
}

LinphoneStatus MediaSession::deferUpdate () {
	L_D();
	if (d->state != CallSession::State::UpdatedByRemote) {
		lError() << "MediaSession::deferUpdate() not done in state CallSession::State::UpdatedByRemote";
		return -1;
	}
	if (d->expectMediaInAck) {
		lError() << "MediaSession::deferUpdate() is not possible during a late offer incoming reINVITE (INVITE without SDP)";
		return -1;
	}
	d->deferUpdate = true;
	return 0;
}

void MediaSession::initiateIncoming () {
	L_D();
	CallSession::initiateIncoming();
	if (d->natPolicy) {
		if (linphone_nat_policy_ice_enabled(d->natPolicy)){
			d->deferIncomingNotification = d->iceAgent->prepare(d->localDesc, true);
			/* 
			 * If ICE gathering is done, we can update the local media description immediately.
			 * Otherwise, we'll get the ORTP_EVENT_ICE_GATHERING_FINISHED event later.
			 */
			if (!d->deferIncomingNotification) d->updateLocalMediaDescriptionFromIce();
		}
	}
}

bool MediaSession::initiateOutgoing () {
	L_D();
	bool defer = CallSession::initiateOutgoing();
	if (linphone_nat_policy_ice_enabled(d->natPolicy)) {
		if (getCore()->getCCore()->sip_conf.sdp_200_ack)
			lWarning() << "ICE is not supported when sending INVITE without SDP";
		else {
			/* Defer the start of the call after the ICE gathering process */
			bool ice_needs_defer = d->iceAgent->prepare(d->localDesc, false);
			if (!ice_needs_defer) {
				/* 
				 * If ICE gathering is done, we can update the local media description immediately.
				 * Otherwise, we'll get the ORTP_EVENT_ICE_GATHERING_FINISHED event later.
				 */
				d->updateLocalMediaDescriptionFromIce();
			}
			defer |= ice_needs_defer;
		}
	}
	return defer;
}

void MediaSession::iterate (time_t currentRealTime, bool oneSecondElapsed) {
	L_D();
	d->executeBackgroundTasks(oneSecondElapsed);
	CallSession::iterate(currentRealTime, oneSecondElapsed);
}

LinphoneStatus MediaSession::pause () {
	L_D();
	LinphoneStatus result = d->pause();
	if (result == 0)
		d->pausedByApp = true;
	return result;
}

LinphoneStatus MediaSession::resume () {
	L_D();
	if (d->state != CallSession::State::Paused) {
		lWarning() << "we cannot resume a call that has not been established and paused before";
		return -1;
	}
	if (!d->getParams()->getPrivate()->getInConference()) {
		if (linphone_core_sound_resources_locked(getCore()->getCCore())) {
			lWarning() << "Cannot resume MediaSession " << this << " because another call is locking the sound resources";
			return -1;
		}
		linphone_core_preempt_sound_resources(getCore()->getCCore());
		lInfo() << "Resuming MediaSession " << this;
	}
	d->automaticallyPaused = false;
	d->broken = false;
	/* Stop playing music immediately. If remote side is a conference it
	 * prevents the participants to hear it while the 200OK comes back. */
	if (d->audioStream)
		audio_stream_play(d->audioStream, nullptr);
	d->makeLocalMediaDescription();
	sal_media_description_set_dir(d->localDesc, SalStreamSendRecv);
	if (getCore()->getCCore()->sip_conf.sdp_200_ack)
		d->op->setLocalMediaDescription(nullptr);
	string subject = "Call resuming";
	if (d->getParams()->getPrivate()->getInConference() && !getCurrentParams()->getPrivate()->getInConference())
		subject = "Conference";
	if (d->op->update(subject.c_str(), false) != 0)
		return -1;
	d->setState(CallSession::State::Resuming,"Resuming");
	if (!d->getParams()->getPrivate()->getInConference() && d->listener)
		d->listener->onSetCurrentSession(getSharedFromThis());
	if (getCore()->getCCore()->sip_conf.sdp_200_ack) {
		/* We are NOT offering, set local media description after sending the call so that we are ready to
		 * process the remote offer when it will arrive. */
		d->op->setLocalMediaDescription(d->localDesc);
	}
	return 0;
}

LinphoneStatus MediaSession::sendDtmf (char dtmf) {
	L_D();
	d->dtmfSequence = dtmf;
	d->sendDtmf();
	return 0;
}

LinphoneStatus MediaSession::sendDtmfs (const std::string &dtmfs) {
	L_D();
	if (d->dtmfTimer) {
		lWarning() << "MediaSession::sendDtmfs(): a DTMF sequence is already in place, canceling DTMF sequence";
		return -2;
	}
	if (!dtmfs.empty()) {
		int delayMs = lp_config_get_int(linphone_core_get_config(getCore()->getCCore()), "net", "dtmf_delay_ms", 200);
		if (delayMs < 0)
			delayMs = 0;
		d->dtmfSequence = dtmfs;
		d->dtmfTimer = getCore()->getCCore()->sal->createTimer(MediaSessionPrivate::sendDtmf, this, static_cast<unsigned int>(delayMs), "DTMF sequence timer");
	}
	return 0;
}

void MediaSession::sendVfuRequest () {
#ifdef VIDEO_ENABLED
	L_D();
	MediaSessionParams *curParams = getCurrentParams();

	if ((curParams->avpfEnabled() || curParams->getPrivate()->implicitRtcpFbEnabled())
		&& d->videoStream && media_stream_get_state(&d->videoStream->ms) == MSStreamStarted) { // || sal_media_description_has_implicit_avpf((const SalMediaDescription *)call->resultdesc)
		lInfo() << "Request Full Intra Request on CallSession [" << this << "]";
		video_stream_send_fir(d->videoStream);
	} else if (getCore()->getCCore()->sip_conf.vfu_with_info) {
		lInfo() << "Request SIP INFO FIR on CallSession [" << this << "]";
		if (d->state == CallSession::State::StreamsRunning)
			d->op->sendVfuRequest();
	} else
		lInfo() << "vfu request using sip disabled from config [sip,vfu_with_info]";
#endif
}

void MediaSession::startIncomingNotification (bool notifyRinging) {
	L_D();
	
	SalMediaDescription *md = d->op->getFinalMediaDescription();
	if (md) {
		if (sal_media_description_empty(md) || linphone_core_incompatible_security(getCore()->getCCore(), md)) {
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, nullptr, LinphoneReasonNotAcceptable, 488, "Not acceptable here", nullptr);
			if (d->listener)
				d->listener->onCallSessionEarlyFailed(getSharedFromThis(), ei);
			d->op->decline(SalReasonNotAcceptable);
			return;
		}
	}

	CallSession::startIncomingNotification(notifyRinging);
}

int MediaSession::startInvite (const Address *destination, const string &subject, const Content *content) {
	L_D();
	linphone_core_stop_dtmf_stream(getCore()->getCCore());
	if (!getCore()->getCCore()->ringstream && getCore()->getCCore()->sound_conf.play_sndcard && getCore()->getCCore()->sound_conf.capt_sndcard) {
		/* Give a chance to set card prefered sampling frequency */
		if (d->localDesc->streams[0].max_rate > 0)
			ms_snd_card_set_preferred_sample_rate(getCore()->getCCore()->sound_conf.play_sndcard, d->localDesc->streams[0].max_rate);
		if (!getCore()->getCCore()->use_files)
			audio_stream_prepare_sound(d->audioStream, getCore()->getCCore()->sound_conf.play_sndcard, getCore()->getCCore()->sound_conf.capt_sndcard);
	}
	if (getCore()->getCCore()->sip_conf.sdp_200_ack) {
		/* We don't want to make the offer, reset local media description before sending the call */
		d->op->setLocalMediaDescription(nullptr);
	}else{
		/* Otherwise set the local media description to the op, because previously makeLocalMediaDescription() could not do it because there was no op yet.*/
		d->op->setLocalMediaDescription(d->localDesc);
	}

	int result = CallSession::startInvite(destination, subject, content);
	if (result < 0) {
		if (d->state == CallSession::State::Error)
			d->stopStreams();
		return result;
	}
	if (getCore()->getCCore()->sip_conf.sdp_200_ack) {
		// We are NOT offering, set local media description after sending the call so that we are ready to
		// process the remote offer when it will arrive.
		d->op->setLocalMediaDescription(d->localDesc);
	}
	return result;
}

void MediaSession::startRecording () {
	L_D();
	if (d->getParams()->getRecordFilePath().empty()) {
		lError() << "MediaSession::startRecording(): no output file specified. Use MediaSessionParams::setRecordFilePath()";
		return;
	}
	if (d->audioStream && !d->getParams()->getPrivate()->getInConference())
		audio_stream_mixed_record_start(d->audioStream);
	d->recordActive = true;
}

void MediaSession::stopRecording () {
	L_D();
	if (d->audioStream && !d->getParams()->getPrivate()->getInConference())
		audio_stream_mixed_record_stop(d->audioStream);
	d->recordActive = false;
}

bool MediaSession::isRecording () {
	L_D();
	return d->recordActive;
}

void MediaSession::terminateBecauseOfLostMedia () {
	L_D();
	d->nonOpError = true;
	linphone_error_info_set(d->ei, nullptr, LinphoneReasonIOError, 503, "Media lost", nullptr);
	terminate();
}

LinphoneStatus MediaSession::update (const MediaSessionParams *msp, const string &subject) {
	L_D();
	CallSession::State nextState;
	CallSession::State initialState = d->state;
	LinphoneStatus result = 0;
	if (!d->isUpdateAllowed(nextState))
		return -1;
	if (d->getCurrentParams() == msp)
		lWarning() << "CallSession::update() is given the current params, this is probably not what you intend to do!";
	d->iceAgent->checkSession(IR_Controlling, true);
	if (msp) {
		d->broken = false;
		d->setState(nextState, "Updating call");
		d->setParams(new MediaSessionParams(*msp));
		if (d->iceAgent->prepare(d->localDesc, false)) {
			lInfo() << "Defer CallSession update to gather ICE candidates";
			return 0;
		}
		result = d->startUpdate(subject);
		if (result && (d->state != initialState)) {
			/* Restore initial state */
			d->setState(initialState, "Restore initial state");
		}
	} else if (d->state == CallSession::State::StreamsRunning) {
		const sound_config_t &soundConfig = getCore()->getCCore()->sound_conf;
		const MSSndCard *captureCard = soundConfig.capt_sndcard;
		const MSSndCard *playCard = soundConfig.lsd_card ? soundConfig.lsd_card : soundConfig.play_sndcard;

		if (captureCard != d->currentCaptureCard || playCard != d->currentPlayCard) {
			d->forceStreamsReconstruction = true;
		}

	#ifdef VIDEO_ENABLED
		else if (d->videoStream) {
			const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(getCore()->getCCore());
			MSVideoSize vsize;
			vsize.width = static_cast<int>(linphone_video_definition_get_width(vdef));
			vsize.height = static_cast<int>(linphone_video_definition_get_height(vdef));
			video_stream_set_sent_video_size(d->videoStream, vsize);
			video_stream_set_fps(d->videoStream, linphone_core_get_preferred_framerate(getCore()->getCCore()));
			if (d->cameraEnabled && (d->videoStream->cam != getCore()->getCCore()->video_conf.device))
				video_stream_change_camera(d->videoStream, getCore()->getCCore()->video_conf.device);
			else
				video_stream_update_video_params(d->videoStream);
		}
	#endif
	}
	return result;
}

// -----------------------------------------------------------------------------

void MediaSession::requestNotifyNextVideoFrameDecoded () {
	L_D();
	if (d->videoStream && d->videoStream->ms.decoder)
		ms_filter_call_method_noarg(d->videoStream->ms.decoder, MS_VIDEO_DECODER_RESET_FIRST_IMAGE_NOTIFICATION);
}

void MediaSessionPrivate::snapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg) {
#ifdef VIDEO_ENABLED
	L_Q();
	if (id == MS_JPEG_WRITER_SNAPSHOT_TAKEN) {
		const char *filepath = (const char *) arg;
		listener->onSnapshotTaken(q->getSharedFromThis(), filepath);
	}
#endif
}

#ifdef VIDEO_ENABLED
static void snapshot_taken(void *userdata, struct _MSFilter *f, unsigned int id, void *arg) {
	MediaSessionPrivate *d = (MediaSessionPrivate *)userdata;
	d->snapshotTakenCb(userdata, f, id, arg);
}
#endif

LinphoneStatus MediaSession::takePreviewSnapshot (const string& file) {
#ifdef VIDEO_ENABLED
	L_D();
	if (d->videoStream && d->videoStream->local_jpegwriter) {
		ms_filter_clear_notify_callback(d->videoStream->jpegwriter);
		const char *filepath = file.empty() ? nullptr : file.c_str();
		ms_filter_add_notify_callback(d->videoStream->local_jpegwriter, snapshot_taken, d, TRUE);
		return ms_filter_call_method(d->videoStream->local_jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void *)filepath);
	}
	lWarning() << "Cannot take local snapshot: no currently running video stream on this call";
#endif
	return -1;
}

LinphoneStatus MediaSession::takeVideoSnapshot (const string& file) {
#ifdef VIDEO_ENABLED
	L_D();
	if (d->videoStream && d->videoStream->jpegwriter) {
		ms_filter_clear_notify_callback(d->videoStream->jpegwriter);
		const char *filepath = file.empty() ? nullptr : file.c_str();
		ms_filter_add_notify_callback(d->videoStream->jpegwriter, snapshot_taken, d, TRUE);
		return ms_filter_call_method(d->videoStream->jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void *)filepath);
	}
	lWarning() << "Cannot take snapshot: no currently running video stream on this call";
#endif
	return -1;
}

void MediaSession::zoomVideo (float zoomFactor, float *cx, float *cy) {
	zoomVideo(zoomFactor, *cx, *cy);
}

void MediaSession::zoomVideo (float zoomFactor, float cx, float cy) {
	L_D();
	if (d->videoStream && d->videoStream->output) {
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
		ms_filter_call_method(d->videoStream->output, MS_VIDEO_DISPLAY_ZOOM, &zoom);
	} else
		lWarning() << "Could not apply zoom: video output wasn't activated";
}

// -----------------------------------------------------------------------------

bool MediaSession::cameraEnabled () const {
	L_D();
	return d->cameraEnabled;
}

bool MediaSession::echoCancellationEnabled () const {
	L_D();
	if (!d->audioStream || !d->audioStream->ec)
		return !!linphone_core_echo_cancellation_enabled(getCore()->getCCore());

	bool val;
	ms_filter_call_method(d->audioStream->ec, MS_ECHO_CANCELLER_GET_BYPASS_MODE, &val);
	return !val;
}

bool MediaSession::echoLimiterEnabled () const {
	L_D();
	if (d->audioStream)
		return d->audioStream->el_type !=ELInactive;
	return !!linphone_core_echo_limiter_enabled(getCore()->getCCore());
}

void MediaSession::enableCamera (bool value) {
#ifdef VIDEO_ENABLED
	L_D();
	d->cameraEnabled = value;
	switch (d->state) {
		case CallSession::State::StreamsRunning:
		case CallSession::State::OutgoingEarlyMedia:
		case CallSession::State::IncomingEarlyMedia:
		case CallSession::State::Connected:
			if (d->videoStream && video_stream_started(d->videoStream) && (video_stream_get_camera(d->videoStream) != d->getVideoDevice())) {
				string currentCam = video_stream_get_camera(d->videoStream) ? ms_web_cam_get_name(video_stream_get_camera(d->videoStream)) : "NULL";
				string newCam = d->getVideoDevice() ? ms_web_cam_get_name(d->getVideoDevice()) : "NULL";
				lInfo() << "Switching video cam from [" << currentCam << "] to [" << newCam << "] on CallSession [" << this << "]";
				video_stream_change_camera(d->videoStream, d->getVideoDevice());
			}
			break;
		default:
			break;
	}
#endif
}

void MediaSession::enableEchoCancellation (bool value) {
	L_D();
	if (d->audioStream && d->audioStream->ec) {
		bool bypassMode = !value;
		ms_filter_call_method(d->audioStream->ec, MS_ECHO_CANCELLER_SET_BYPASS_MODE, &bypassMode);
	}
}

void MediaSession::enableEchoLimiter (bool value) {
	L_D();
	if (d->audioStream) {
		if (value) {
			string type = lp_config_get_string(linphone_core_get_config(getCore()->getCCore()), "sound", "el_type", "mic");
			if (type == "mic")
				audio_stream_enable_echo_limiter(d->audioStream, ELControlMic);
			else if (type == "full")
				audio_stream_enable_echo_limiter(d->audioStream, ELControlFull);
		} else
			audio_stream_enable_echo_limiter(d->audioStream, ELInactive);
	}
}

bool MediaSession::getAllMuted () const {
	L_D();
	if (d->audioStream && d->videoStream) return d->audioMuted && d->videoMuted;
	if (d->audioStream) return d->audioMuted;
	return d->videoMuted;
}

LinphoneCallStats * MediaSession::getAudioStats () const {
	return getStats(LinphoneStreamTypeAudio);
}

string MediaSession::getAuthenticationToken () const {
	L_D();
	return d->authToken;
}

bool MediaSession::getAuthenticationTokenVerified () const {
	L_D();
	return d->authTokenVerified;
}

float MediaSession::getAverageQuality () const {
	L_D();
	return d->getStreamsGroup().getAverageQuality();
}

MediaSessionParams * MediaSession::getCurrentParams () const {
	L_D();
	d->updateCurrentParams();
	return d->getCurrentParams();
}

float MediaSession::getCurrentQuality () const {
	L_D();
	float audioRating = -1.f;
	float videoRating = -1.f;
	if (d->audioStream)
		audioRating = media_stream_get_quality_rating(&d->audioStream->ms) / 5.0f;
	if (d->videoStream)
		videoRating = media_stream_get_quality_rating(&d->videoStream->ms) / 5.0f;
	return MediaSessionPrivate::aggregateQualityRatings(audioRating, videoRating);
}

const MediaSessionParams * MediaSession::getMediaParams () const {
	L_D();
	return d->getParams();
}

RtpTransport * MediaSession::getMetaRtcpTransport (int streamIndex) const {
	L_D();
	if ((streamIndex < 0) || (streamIndex >= getStreamCount()))
		return nullptr;
	RtpTransport *metaRtp;
	RtpTransport *metaRtcp;
	rtp_session_get_transports(d->sessions[streamIndex].rtp_session, &metaRtp, &metaRtcp);
	return metaRtcp;
}

RtpTransport * MediaSession::getMetaRtpTransport (int streamIndex) const {
	L_D();
	if ((streamIndex < 0) || (streamIndex >= getStreamCount()))
		return nullptr;
	RtpTransport *metaRtp;
	RtpTransport *metaRtcp;
	rtp_session_get_transports(d->sessions[streamIndex].rtp_session, &metaRtp, &metaRtcp);
	return metaRtp;
}

float MediaSession::getMicrophoneVolumeGain () const {
	L_D();
	AudioControlInterface *iface = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface){
		return iface->getMicGain();
	} else {
		lError() << "Could not get record volume: no audio stream";
		return -1.0f;
	}
}

void MediaSession::setMicrophoneVolumeGain (float value) {
	L_D();
	AudioControlInterface *iface = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface)
		iface->setMicGain(value);
	else
		lError() << "Could not set record volume: no audio stream";
}

float MediaSession::getSpeakerVolumeGain () const {
	L_D();
	AudioControlInterface *iface = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface)
		return iface->getSpeakerGain();
	else {
		lError() << "Could not get playback volume: no audio stream";
		return -1.0f;
	}
}

void MediaSession::setSpeakerVolumeGain (float value) {
	L_D();
	AudioControlInterface *iface = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface)
		iface->setSpeakerGain(value);
	else
		lError() << "Could not set playback volume: no audio stream";
}

void * MediaSession::getNativeVideoWindowId () const {
	L_D();
	auto interface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (interface){
		interface->getNativeVideoWindowId();
	}
	return nullptr;
}

void MediaSession::setNativeVideoWindowId (void *id) {
	L_D();
	auto interface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (interface){
		return interface->setNativeVideoWindowId(id);
	}
}

const CallSessionParams * MediaSession::getParams () const {
	L_D();
	return d->params;
}

float MediaSession::getPlayVolume () const {
	L_D();
	AudioControlInterface *iface = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface) return iface->getPlayVolume();
	return LINPHONE_VOLUME_DB_LOWEST;
}

float MediaSession::getRecordVolume () const {
	L_D();
	
	if (d->state == CallSession::State::StreamsRunning){
		AudioControlInterface *iface = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
		if (iface) return iface->getRecordVolume();
	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

const MediaSessionParams * MediaSession::getRemoteParams () {
	L_D();
	if (d->op){
		SalMediaDescription *md = d->op->getRemoteMediaDescription();
		if (md) {
			d->setRemoteParams(new MediaSessionParams());
			unsigned int nbAudioStreams = sal_media_description_nb_active_streams_of_type(md, SalAudio);
			for (unsigned int i = 0; i < nbAudioStreams; i++) {
				SalStreamDescription *sd = sal_media_description_get_active_stream_of_type(md, SalAudio, i);
				if (sal_stream_description_has_srtp(sd))
					d->getRemoteParams()->setMediaEncryption(LinphoneMediaEncryptionSRTP);
			}
			unsigned int nbVideoStreams = sal_media_description_nb_active_streams_of_type(md, SalVideo);
			for (unsigned int i = 0; i < nbVideoStreams; i++) {
				SalStreamDescription *sd = sal_media_description_get_active_stream_of_type(md, SalVideo, i);
				if (sal_stream_description_active(sd))
					d->getRemoteParams()->enableVideo(true);
				if (sal_stream_description_has_srtp(sd))
					d->getRemoteParams()->setMediaEncryption(LinphoneMediaEncryptionSRTP);
			}
			unsigned int nbTextStreams = sal_media_description_nb_active_streams_of_type(md, SalText);
			for (unsigned int i = 0; i < nbTextStreams; i++) {
				SalStreamDescription *sd = sal_media_description_get_active_stream_of_type(md, SalText, i);
				if (sal_stream_description_has_srtp(sd))
					d->getRemoteParams()->setMediaEncryption(LinphoneMediaEncryptionSRTP);
				d->getRemoteParams()->enableRealtimeText(true);
			}
			if (!d->getRemoteParams()->videoEnabled()) {
				if ((md->bandwidth > 0) && (md->bandwidth <= linphone_core_get_edge_bw(getCore()->getCCore())))
					d->getRemoteParams()->enableLowBandwidth(true);
			}
			if (md->name[0] != '\0')
				d->getRemoteParams()->setSessionName(md->name);

			d->getRemoteParams()->getPrivate()->setCustomSdpAttributes(md->custom_sdp_attributes);
			if (d->mainAudioStreamIndex != -1)
				d->getRemoteParams()->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeAudio, md->streams[d->mainAudioStreamIndex].custom_sdp_attributes);
			if (d->mainVideoStreamIndex != -1)
				d->getRemoteParams()->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeVideo, md->streams[d->mainVideoStreamIndex].custom_sdp_attributes);
			if (d->mainTextStreamIndex != -1)
				d->getRemoteParams()->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeText, md->streams[d->mainTextStreamIndex].custom_sdp_attributes);
		}
		const SalCustomHeader *ch = d->op->getRecvCustomHeaders();
		if (ch) {
			/* Instanciate a remote_params only if a SIP message was received before (custom headers indicates this) */
			if (!d->remoteParams)
				d->setRemoteParams(new MediaSessionParams());
			d->getRemoteParams()->getPrivate()->setCustomHeaders(ch);
		}
		return d->getRemoteParams();
	}
	return nullptr;
}

LinphoneCallStats * MediaSession::getStats (LinphoneStreamType type) const {
	L_D();
	if (type == LinphoneStreamTypeUnknown)
		return nullptr;
	LinphoneCallStats *stats = nullptr;
	LinphoneCallStats *statsCopy = nullptr;
	Stream *s = d->getStream(type);
	if (s && (stats = s->getStats())) {
		statsCopy = _linphone_call_stats_new();
		_linphone_call_stats_clone(statsCopy, stats);
	}
	return statsCopy;
}

int MediaSession::getStreamCount () const {
	L_D();
	return d->getStreamGroup().size();
}

MSFormatType MediaSession::getStreamType (int streamIndex) const {
	L_D();
	Stream *s = d->getStreamsGroup().getStream(streamIndex);
	if (s){
		switch(s->getType()){
			case SalAudio:
				return MSAudio;
			case SalVideo:
				return MSVideo;
			case SalText:
				return MSText;
			case SalOther:
			break;
		}
	}
	return MSUnknownMedia;
}

LinphoneCallStats * MediaSession::getTextStats () const {
	return getStats(LinphoneStreamTypeText);
}

LinphoneCallStats * MediaSession::getVideoStats () const {
	return getStats(LinphoneStreamTypeVideo);
}

bool MediaSession::mediaInProgress () const {
	L_D();
	return !d->getIceAgent().hasCompleted();
}

void MediaSession::setAudioRoute (LinphoneAudioRoute route) {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) i->setRoute(route);
}

void MediaSession::setAuthenticationTokenVerified (bool value) {
	L_D();
	if (!d->audioStream || !media_stream_started(&d->audioStream->ms)) {
		lError() << "MediaSession::setAuthenticationTokenVerified(): No audio stream or not started";
		return;
	}
	if (!d->audioStream->ms.sessions.zrtp_context) {
		lError() << "MediaSession::setAuthenticationTokenVerified(): No zrtp context";
		return;
	}
	// SAS verified
	if (value) {
		ms_zrtp_sas_verified(d->audioStream->ms.sessions.zrtp_context);
	} else { // SAS rejected
		ms_zrtp_sas_reset_verified(d->audioStream->ms.sessions.zrtp_context);
		char *peerDeviceId = nullptr;
		auto encryptionEngine = getCore()->getEncryptionEngine();
		if (encryptionEngine) { //inform lime that zrtp no longuer guaranty the trust
			const SalAddress *remoteAddress = d->getOp()->getRemoteContactAddress();
			peerDeviceId = sal_address_as_string_uri_only(remoteAddress);
			encryptionEngine->authenticationRejected(peerDeviceId);
			ms_free(peerDeviceId);
		}
	}
	d->authTokenVerified = value;
	d->propagateEncryptionChanged();
}

void MediaSession::setParams (const MediaSessionParams *msp) {
	L_D();
	if ((d->state == CallSession::State::OutgoingInit) || (d->state == CallSession::State::IncomingReceived))
		d->setParams(msp ? new MediaSessionParams(*msp) : nullptr);
	else
		lError() << "MediaSession::setParams(): Invalid state %s", Utils::toString(d->state);
}



LINPHONE_END_NAMESPACE
