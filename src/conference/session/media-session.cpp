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

//#include <iomanip>
//#include <math.h>

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
	
	switch (state){
		case CallSession::State::OutgoingProgress:
		case CallSession::State::OutgoingRinging:
		case CallSession::State::OutgoingEarlyMedia:
		case CallSession::State::Connected:
			if (q->getCore()->getCCore()->sip_conf.sdp_200_ack){
				lInfo() << "Initializing local media description according to remote offer in 200Ok";
				// We were waiting for an incoming offer. Now prepare the local media description according to remote offer.
				initializeParamsAccordingToIncomingCallParams();
				makeLocalMediaDescription(op->getRemoteMediaDescription() ? false : true);
				/*
				 * If ICE is enabled, we'll have to do the prepare() step, however since defering the sending of the ACK is complicated and 
				 * confusing from a signaling standpoint, ICE we will skip the STUN gathering by not giving enough time
				 * for the gathering step. Only local candidates will be answered in the ACK.
				 */
				if (getStreamsGroup().prepare()){
					lWarning() << "Some gathering is needed for ICE, however since a defered sending of ACK is not supported"
						" the ICE gathering will only contain local candidates.";
				}
				getStreamsGroup().finishPrepare();
				updateLocalMediaDescriptionFromIce();
			}
		break;
		default:
		break;
	}
	
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
			//getIceAgent().updateIceStateInCallStats();
			updateStreams(md, nextState);
			fixCallParams(rmd, false);
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
	getStreamsGroup().sessionConfirmed(getStreamsGroup().getCurrentOfferAnswerContext());
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
					if (!sal_stream_description_enabled(&localDesc->streams[i]))
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
			OfferAnswerContext ctx;
			ctx.localMediaDescription = localDesc;
			ctx.resultMediaDescription = md;
			ctx.remoteMediaDescription = rmd;
			getStreamsGroup().tryEarlyMediaForking(ctx);
			return;
		}

		setState(CallSession::State::OutgoingEarlyMedia, "Early media");
		if (listener)
			listener->onStopRinging(q->getSharedFromThis());
		lInfo() << "Doing early media...";
		/* FIXME */
		//getIceAgent().updateFromRemoteMediaDescription(localDesc, rmd, !op->isOfferer());
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



void MediaSessionPrivate::updating(bool isUpdate) {
	L_Q();
	SalMediaDescription *rmd = op->getRemoteMediaDescription();
	fixCallParams(rmd, true);
	if (state != CallSession::State::Paused) {
		/* Refresh the local description, but in paused state, we don't change anything. */
		if (!rmd && lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "sdp_200_ack_follow_video_policy", 0)) {
			lInfo() << "Applying default policy for offering SDP on CallSession [" << q << "]";
			setParams(new MediaSessionParams());
			// Yes we init parameters as if we were in the case of an outgoing call, because it is a resume with no SDP.
			params->initDefault(q->getCore(), LinphoneCallOutgoing);
		}
		makeLocalMediaDescription(false);
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
#ifdef VIDEO_ENABLED
	if (mainVideoStreamIndex != -1){
		MS2VideoStream * vs = dynamic_cast<MS2VideoStream*>(getStreamsGroup().getStream(mainVideoStreamIndex));
		if (vs) vs->oglRender();
	}
#endif
}

void MediaSessionPrivate::sendVfu () {
	getStreamsGroup().forEach<VideoControlInterface>([](VideoControlInterface *i){ i->sendVfu(); });
}


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
	lError() << "There is no stats for main stream of type " << linphone_stream_type_to_string(type) << " because this stream doesn't exist.";
	return nullptr;
}

// -----------------------------------------------------------------------------


void MediaSessionPrivate::stopStreams () {
	L_Q();
	if (getStreamsGroup().isStarted()) getStreamsGroup().stop();
	q->getCore()->soundcardHintCheck();
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
			//FIXME 
			if (rmd && false /*getIceAgent().checkIceReinviteNeedsDeferedResponse(rmd) */) {
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
	for (i = 0; i < md->nb_streams; ++i) {
		if (md->streams[i].type == type) return i;
	}
	return -1;
}

void MediaSessionPrivate::assignStreamsIndexes(){
	if (biggestDesc && freeStreamIndex < biggestDesc->nb_streams) freeStreamIndex = biggestDesc->nb_streams;
	
	/*Initialize stream indexes from potential incoming offer.*/
	SalMediaDescription *rmd = op ? op->getRemoteMediaDescription() : nullptr;
	if (rmd) assignStreamsIndexesIncoming(rmd);
	
	/*Assign indexes for our streams, if no incoming offer was received, or if new streams are requested.*/
	if (getParams()->audioEnabled() && mainAudioStreamIndex == -1){
		mainAudioStreamIndex = freeStreamIndex++;
	}
	if (getParams()->videoEnabled() && mainVideoStreamIndex == -1){
		mainVideoStreamIndex = freeStreamIndex++;
	}
	if (getParams()->realtimeTextEnabled() && mainTextStreamIndex == -1){
		mainTextStreamIndex = freeStreamIndex++;
	}
	lInfo() << "Stream indexes selected (-1 = unassigned): mainAudioStreamIndex=" << mainAudioStreamIndex <<
		", mainVideoStreamIndex=" << mainVideoStreamIndex << ", mainTextStreamIndex=" << mainTextStreamIndex;
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
	if (freeStreamIndex < md->nb_streams) freeStreamIndex = md->nb_streams;
}

/*
 * This method needs to be called at each incoming reINVITE, in order to adjust various local parameters to what is being offered by remote:
 * - the stream indexes.
 * - the video enablement parameter according to what is offered and our local policy.
 * Fixing the params to proper values avoid request video by accident during internal call updates, pauses and resumes
 */
void MediaSessionPrivate::fixCallParams (SalMediaDescription *rmd, bool fromOffer) {
	L_Q();
	if (!rmd) return;
	
	updateBiggestDesc(rmd);
	/* Why disabling implicit_rtcp_fb ? It is a local policy choice actually. It doesn't disturb to propose it again and again
		* even if the other end apparently doesn't support it.
		* The following line of code is causing trouble, while for example making an audio call, then adding video.
		* Due to the 200Ok response of the audio-only offer where no rtcp-fb attribute is present, implicit_rtcp_fb is set to
		* false, which is then preventing it to be eventually used when video is later added to the call.
		* I did the choice of commenting it out.
		*/
	/*params.getPrivate()->enableImplicitRtcpFb(params.getPrivate()->implicitRtcpFbEnabled() & sal_media_description_has_implicit_avpf(rmd));*/
	const MediaSessionParams *rcp = q->getRemoteParams();
	if (rcp) {
		if (!fromOffer){
			/*
			 * This is to avoid to re-propose again some streams that have just been declined.
			 */
			if (getParams()->audioEnabled() && !rcp->audioEnabled()) {
				lInfo() << "CallSession [" << q << "]: disabling audio in our call params because the remote doesn't want it";
				getParams()->enableAudio(false);
			}
			if (getParams()->videoEnabled() && !rcp->videoEnabled()) {
				lInfo() << "CallSession [" << q << "]: disabling video in our call params because the remote doesn't want it";
				getParams()->enableVideo(false);
			}
			if (getParams()->realtimeTextEnabled() && !rcp->realtimeTextEnabled()) {
				lInfo() << "CallSession [" << q << "]: disabling RTT in our call params because the remote doesn't want it";
				getParams()->enableRealtimeText(false);
			}
		}
		// Real Time Text is always by default accepted when proposed.
		if (!getParams()->realtimeTextEnabled() && rcp->realtimeTextEnabled())
			getParams()->enableRealtimeText(true);
		
		if (rcp->videoEnabled() && q->getCore()->getCCore()->video_policy.automatically_accept && linphone_core_video_enabled(q->getCore()->getCCore()) && !getParams()->videoEnabled()) {
			lInfo() << "CallSession [" << q << "]: re-enabling video in our call params because the remote wants it and the policy allows to automatically accept";
			getParams()->enableVideo(true);
		}
		
	}
}

void MediaSessionPrivate::initializeParamsAccordingToIncomingCallParams () {
	CallSessionPrivate::initializeParamsAccordingToIncomingCallParams();
	SalMediaDescription *md = op->getRemoteMediaDescription();
	if (md) {
		assignStreamsIndexesIncoming(md);
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
	if (mainAudioStreamIndex != -1){
		SalStreamDescription *sd = &md->streams[mainAudioStreamIndex];
		const char *rtpAddr = (sd->rtp_addr[0] != '\0') ? sd->rtp_addr : md->addr;
		if (ms_is_multicast(rtpAddr)){
			lInfo() << "Incoming offer has audio multicast, enabling it in local params.";
			getParams()->enableAudioMulticast(true);
		}else getParams()->enableAudioMulticast(false);
	}
	if (mainVideoStreamIndex != -1){
		SalStreamDescription *sd = &md->streams[mainVideoStreamIndex];
		const char *rtpAddr = (sd->rtp_addr[0] != '\0') ? sd->rtp_addr : md->addr;
		if (ms_is_multicast(rtpAddr)){
			lInfo() << "Incoming offer has video multicast, enabling it in local params.";
			getParams()->enableVideoMulticast(true);
		}else getParams()->enableVideoMulticast(false);
	}
	
	/* In case of nat64, even ipv4 addresses are reachable from v6. Should be enhanced to manage stream by stream connectivity (I.E v6 or v4) */
	/*if (!sal_media_description_has_ipv6(md)){
		lInfo() << "The remote SDP doesn't seem to offer any IPv6 connectivity, so disabling IPv6 for this call";
		af = AF_INET;
	}*/
	fixCallParams(md, true);
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
	for (int i = 0; i < md->nb_streams; i++) {
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

void MediaSessionPrivate::addStreamToBundle(SalMediaDescription *md, SalStreamDescription *sd, const char *mid){
	SalStreamBundle *bundle;
	if (md->bundles == nullptr){
		bundle = sal_media_description_add_new_bundle(md);
	}else{
		bundle = (SalStreamBundle*) md->bundles->data;
	}
	sal_stream_bundle_add_stream(bundle, sd, mid);
	sd->mid_rtp_ext_header_id = rtpExtHeaderMidNumber;
	/* rtcp-mux must be enabled when bundle mode is proposed.*/
	sd->rtcp_mux = TRUE;
}

void MediaSessionPrivate::makeLocalMediaDescription(bool localIsOfferer) {
	L_Q();
	bool rtcpMux = !!lp_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_mux", 0);
	SalMediaDescription *md = sal_media_description_new();
	SalMediaDescription *oldMd = localDesc;
	
	assignStreamsIndexes();
	
	getParams()->getPrivate()->adaptToNetwork(q->getCore()->getCCore(), pingTime);

	string subject = q->getParams()->getSessionName();
	if (!subject.empty()) {
		strncpy(md->name, subject.c_str(), sizeof(md->name));
		md->name[sizeof(md->name) - 1] = '\0';
	}
	md->session_id = (oldMd ? oldMd->session_id : (bctbx_random() & 0xfff));
	md->session_ver = (oldMd ? (oldMd->session_ver + 1) : (bctbx_random() & 0xfff));
	md->nb_streams = (biggestDesc ? biggestDesc->nb_streams : 1);
	
	md->accept_bundles = getParams()->rtpBundleEnabled() || 
		linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "accept_bundle", TRUE);

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
		l = nullptr;
		md->streams[mainAudioStreamIndex].proto = getParams()->getMediaProto();
		md->streams[mainAudioStreamIndex].dir = getParams()->getPrivate()->getSalAudioDirection();
		md->streams[mainAudioStreamIndex].type = SalAudio;
		if (getParams()->audioEnabled() && (l = pth.makeCodecsList(SalAudio, getParams()->getAudioBandwidthLimit(), -1,
		oldMd ? oldMd->streams[mainAudioStreamIndex].already_assigned_payloads : nullptr))) {
			strncpy(md->streams[mainAudioStreamIndex].name, "Audio", sizeof(md->streams[mainAudioStreamIndex].name) - 1);
			md->streams[mainAudioStreamIndex].rtcp_mux = rtcpMux;
			md->streams[mainAudioStreamIndex].rtp_port = SAL_STREAM_DESCRIPTION_PORT_TO_BE_DETERMINED;
			int downPtime = getParams()->getPrivate()->getDownPtime();
			if (downPtime)
				md->streams[mainAudioStreamIndex].ptime = downPtime;
			else
				md->streams[mainAudioStreamIndex].ptime = linphone_core_get_download_ptime(q->getCore()->getCCore());
			md->streams[mainAudioStreamIndex].max_rate = pth.getMaxCodecSampleRate(l);
			md->streams[mainAudioStreamIndex].payloads = l;
			strncpy(md->streams[mainAudioStreamIndex].rtcp_cname, getMe()->getAddress().asString().c_str(), sizeof(md->streams[mainAudioStreamIndex].rtcp_cname));
			if (getParams()->rtpBundleEnabled()) addStreamToBundle(md, &md->streams[mainAudioStreamIndex], "as");
			
			if (getParams()->audioMulticastEnabled()) {
				md->streams[mainAudioStreamIndex].ttl = linphone_core_get_audio_multicast_ttl(q->getCore()->getCCore());
				md->streams[mainAudioStreamIndex].multicast_role = (direction == LinphoneCallOutgoing) ? SalMulticastSender : SalMulticastReceiver;
			}
			
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
		l = nullptr;
		md->streams[mainVideoStreamIndex].proto = getParams()->getMediaProto();
		md->streams[mainVideoStreamIndex].dir = getParams()->getPrivate()->getSalVideoDirection();
		md->streams[mainVideoStreamIndex].type = SalVideo;
		
		if (getParams()->videoEnabled() && (l = pth.makeCodecsList(SalVideo, 0, -1,
			oldMd ? oldMd->streams[mainVideoStreamIndex].already_assigned_payloads : nullptr)) ){
			md->streams[mainVideoStreamIndex].rtcp_mux = rtcpMux;
			md->streams[mainVideoStreamIndex].rtp_port = SAL_STREAM_DESCRIPTION_PORT_TO_BE_DETERMINED;
			strncpy(md->streams[mainVideoStreamIndex].name, "Video", sizeof(md->streams[mainVideoStreamIndex].name) - 1);
			md->streams[mainVideoStreamIndex].payloads = l;
			strncpy(md->streams[mainVideoStreamIndex].rtcp_cname, getMe()->getAddress().asString().c_str(), sizeof(md->streams[mainVideoStreamIndex].rtcp_cname));
			if (getParams()->rtpBundleEnabled()) addStreamToBundle(md, &md->streams[mainVideoStreamIndex], "vs");
			
			if (getParams()->videoMulticastEnabled()) {
				md->streams[mainVideoStreamIndex].ttl = linphone_core_get_video_multicast_ttl(q->getCore()->getCCore());
				md->streams[mainVideoStreamIndex].multicast_role = (direction == LinphoneCallOutgoing) ? SalMulticastSender : SalMulticastReceiver;
			}
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
		l = nullptr;
		md->streams[mainTextStreamIndex].proto = getParams()->getMediaProto();
		md->streams[mainTextStreamIndex].dir = SalStreamSendRecv;
		md->streams[mainTextStreamIndex].type = SalText;
		if (getParams()->realtimeTextEnabled() && (l = pth.makeCodecsList(SalText, 0, -1,
				oldMd ? oldMd->streams[mainTextStreamIndex].already_assigned_payloads : nullptr)) ) {
			md->streams[mainTextStreamIndex].rtcp_mux = rtcpMux;
			md->streams[mainTextStreamIndex].rtp_port = getParams()->realtimeTextEnabled() ? SAL_STREAM_DESCRIPTION_PORT_TO_BE_DETERMINED : 0;
			strncpy(md->streams[mainTextStreamIndex].name, "Text", sizeof(md->streams[mainTextStreamIndex].name) - 1);
			md->streams[mainTextStreamIndex].payloads = l;
			strncpy(md->streams[mainTextStreamIndex].rtcp_cname, getMe()->getAddress().asString().c_str(), sizeof(md->streams[mainTextStreamIndex].rtcp_cname));
			if (getParams()->rtpBundleEnabled()) addStreamToBundle(md, &md->streams[mainTextStreamIndex], "ts");
		} else {
			lInfo() << "Don't put text stream on local offer for CallSession [" << q << "]";
			md->streams[mainTextStreamIndex].dir = SalStreamInactive;
			if(l)
				l = bctbx_list_free_with_data(l, (bctbx_list_free_func)payload_type_destroy);
		}
		customSdpAttributes = getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeText);
		if (customSdpAttributes)
			md->streams[mainTextStreamIndex].custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);
	}

	md->nb_streams = freeStreamIndex;

	setupEncryptionKeys(md);
	setupImEncryptionEngineParameters(md);
	setupRtcpFb(md);
	setupRtcpXr(md);
	if (stunClient)
		stunClient->updateMediaDescription(md);
	localDesc = md;
	
	OfferAnswerContext ctx;
	ctx.localMediaDescription = localDesc;
	ctx.remoteMediaDescription = localIsOfferer ? nullptr : ( op ? op->getRemoteMediaDescription() : nullptr);
	ctx.localIsOfferer = localIsOfferer;
	/* Now instanciate the streams according to the media description. */
	getStreamsGroup().createStreams(ctx);
	if (mainAudioStreamIndex != -1) getStreamsGroup().setStreamMain((size_t)mainAudioStreamIndex);
	if (mainVideoStreamIndex != -1) getStreamsGroup().setStreamMain((size_t)mainVideoStreamIndex);
	if (mainTextStreamIndex != -1) getStreamsGroup().setStreamMain((size_t)mainTextStreamIndex);
	/* Get the transport addresses filled in to the media description. */
	getStreamsGroup().fillLocalMediaDescription(ctx);
	
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
	for (int i = 0; i < md->nb_streams; i++) {
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
	for (int i = 0; i < md->nb_streams; i++) {
		memcpy(&md->streams[i].rtcp_xr, &md->rtcp_xr, sizeof(md->streams[i].rtcp_xr));
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
	for (int i = 0; i < md->nb_streams; i++) {
		if (sal_stream_description_has_srtp(&md->streams[i])) {
			if (keepSrtpKeys && oldMd && sal_stream_description_enabled(&oldMd->streams[i]) && sal_stream_description_has_srtp(&oldMd->streams[i])) {
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
	OfferAnswerContext ctx;
	ctx.localMediaDescription = localDesc;
	ctx.remoteMediaDescription = op ? op->getRemoteMediaDescription() : nullptr;
	getStreamsGroup().fillLocalMediaDescription(ctx);
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


void MediaSessionPrivate::startDtlsOnAllStreams () {
	OfferAnswerContext params;
	params.localMediaDescription = localDesc;
	params.remoteMediaDescription = op->getRemoteMediaDescription();
	params.resultMediaDescription = resultDesc;
	if (params.remoteMediaDescription && params.resultMediaDescription){
		getStreamsGroup().startDtls(params);
	}
}


/*
 * Frees the media resources of the call.
 * This has to be done at the earliest, unlike signaling resources that sometimes need to be kept a bit more longer.
 * It is called by setTerminated() (for termination of calls signaled to the application), or directly by the destructor of the session
 * if it was never notified to the application.
 */
void MediaSessionPrivate::freeResources () {
	getStreamsGroup().finish();
}

/* 
 * IceServiceListener implementation
 */
void MediaSessionPrivate::onGatheringFinished(IceService &service){
	L_Q();
	updateLocalMediaDescriptionFromIce();
	switch (state) {
		case CallSession::State::IncomingReceived:
		case CallSession::State::IncomingEarlyMedia:
			if (callAcceptanceDefered) startAccept();
			break;
		case CallSession::State::Updating:
			startUpdate();
			break;
		case CallSession::State::UpdatedByRemote:
			startAcceptUpdate(prevState, Utils::toString(prevState));
			break;
		case CallSession::State::OutgoingInit:
			q->startInvite(nullptr, "");
			break;
		case CallSession::State::Idle:
			deferIncomingNotification = false;
			startIncomingNotification();
			break;
		default:
			break;
	}
}

void MediaSessionPrivate::onIceCompleted(IceService &service){
	L_Q();
	/* The ICE session has succeeded, so perform a call update */
	if (!getStreamsGroup().getIceService().hasCompletedCheckList()) return;
	if (getStreamsGroup().getIceService().isControlling() && getParams()->getPrivate()->getUpdateCallWhenIceCompleted()) {
		if (state == CallSession::State::StreamsRunning){
			MediaSessionParams newParams(*getParams());
			newParams.getPrivate()->setInternalCallUpdate(true);
			q->update(&newParams);
		}else{
			lWarning() << "Cannot send reINVITE for ICE during state " << state;
		}
	}else if (!getStreamsGroup().getIceService().isControlling() && incomingIceReinvitePending){
		q->acceptUpdate(nullptr);
		incomingIceReinvitePending = false;
	}
	startDtlsOnAllStreams();
}

void MediaSessionPrivate::onLosingPairsCompleted(IceService &service){
	if (state == CallSession::State::UpdatedByRemote) {
		lInfo() << "Finished adding losing pairs, ICE re-INVITE can be answered.";
		startAcceptUpdate(prevState, Utils::toString(prevState));
	}
}

void MediaSessionPrivate::onIceRestartNeeded(IceService & service){
	L_Q();
	getStreamsGroup().getIceService().restartSession(IR_Controlling);
	MediaSessionParams newParams(*getParams());
	q->update(&newParams);
}

void MediaSessionPrivate::tryEarlyMediaForking (SalMediaDescription *md) {
	OfferAnswerContext ctx;
	ctx.localMediaDescription = localDesc;
	ctx.remoteMediaDescription = md;
	ctx.resultMediaDescription = resultDesc;
	lInfo() << "Early media response received from another branch, checking if media can be forked to this new destination";
	getStreamsGroup().tryEarlyMediaForking(ctx);
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
	
	OfferAnswerContext ctx;
	ctx.localMediaDescription = localDesc;
	ctx.remoteMediaDescription = op->getRemoteMediaDescription();
	ctx.resultMediaDescription = resultDesc;
	getStreamsGroup().render(ctx, targetState);

	if ((state == CallSession::State::Pausing) && pausedByApp && (q->getCore()->getCallCount() == 1))
		linphone_core_play_named_tone(q->getCore()->getCCore(), LinphoneToneCallOnHold);

	updateFrozenPayloads(newMd);
	upBandwidth = linphone_core_get_upload_bandwidth(q->getCore()->getCCore());

	if (oldMd)
		sal_media_description_unref(oldMd);
}

// -----------------------------------------------------------------------------

bool MediaSessionPrivate::allStreamsAvpfEnabled () const {
	return getStreamsGroup().avpfEnabled();
}

bool MediaSessionPrivate::allStreamsEncrypted () const {
	return getStreamsGroup().allStreamsEncrypted();
}

bool MediaSessionPrivate::atLeastOneStreamStarted () const {
	return getStreamsGroup().isStarted();
}

uint16_t MediaSessionPrivate::getAvpfRrInterval () const {
	return (uint16_t)getStreamsGroup().getAvpfRrInterval();
}

unsigned int MediaSessionPrivate::getNbActiveStreams () const {
	return (unsigned int)getStreamsGroup().getActiveStreamsCount();
}

bool MediaSessionPrivate::isEncryptionMandatory () const {
	L_Q();
	if (getParams()->getMediaEncryption() == LinphoneMediaEncryptionDTLS) {
		lInfo() << "Forced encryption mandatory on CallSession [" << q << "] due to SRTP-DTLS";
		return true;
	}
	return getParams()->mandatoryMediaEncryptionEnabled();
}

void MediaSessionPrivate::propagateEncryptionChanged () {
	L_Q();
	
	string authToken = getStreamsGroup().getAuthenticationToken();
	bool authTokenVerified = getStreamsGroup().getAuthenticationTokenVerified();
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
#ifdef VIDEO_ENABLED
	MS2VideoStream *vs = getStreamsGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);
	if (vs) return vs->getVideoDevice(state);
#endif
	return nullptr;
}

// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------

void MediaSessionPrivate::lossOfMediaDetected() {
	L_Q();
	if (listener)
		listener->onLossOfMediaDetected(q->getSharedFromThis());
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
	makeLocalMediaDescription(true);
	op->update(subject.c_str(), false);
	if (listener)
		listener->onResetCurrentSession(q->getSharedFromThis());
	stopStreams();
	pausedByApp = false;
	return 0;
}

int MediaSessionPrivate::restartInvite () {
	stopStreams();
	getStreamsGroup().clearStreams();
	makeLocalMediaDescription(true);
	return CallSessionPrivate::restartInvite();
}

void MediaSessionPrivate::setTerminated () {
	freeResources();
	CallSessionPrivate::setTerminated();
}

LinphoneStatus MediaSessionPrivate::startAcceptUpdate (CallSession::State nextState, const string &stateInfo) {
	op->accept();
	SalMediaDescription *md = op->getFinalMediaDescription();
	if (md && !sal_media_description_empty(md))
		updateStreams(md, nextState);
	setState(nextState, stateInfo);
	return 0;
}

LinphoneStatus MediaSessionPrivate::startUpdate (const string &subject) {
	L_Q();
	
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

	
	VideoControlInterface *i = getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i){
		VideoControlInterface::VideoStats st;
		LinphoneVideoDefinition *vdef;
		
		i->getRecvStats(&st);
		vdef = linphone_video_definition_new((unsigned)st.width, (unsigned)st.height, nullptr);
		getCurrentParams()->getPrivate()->setReceivedVideoDefinition(vdef);
		linphone_video_definition_unref(vdef);
		getCurrentParams()->getPrivate()->setReceivedFps(st.fps);
		
		i->getSendStats(&st);
		vdef = linphone_video_definition_new((unsigned)st.width, (unsigned)st.height, nullptr);
		getCurrentParams()->getPrivate()->setSentVideoDefinition(vdef);
		linphone_video_definition_unref(vdef);
		getCurrentParams()->getPrivate()->setSentFps(st.fps);
		
	}else{
		LinphoneVideoDefinition *vdef = linphone_video_definition_new(MS_VIDEO_SIZE_UNKNOWN_W, MS_VIDEO_SIZE_UNKNOWN_H, nullptr);
		getCurrentParams()->getPrivate()->setSentVideoDefinition(vdef);
		getCurrentParams()->getPrivate()->setReceivedVideoDefinition(vdef);
		linphone_video_definition_unref(vdef);
	}

	/* REVISITED
	 * Previous code was buggy.
	 * Relying on the mediastream's state (added by jehan: only) to know the current encryption is unreliable.
	 * For (added by jehan: both DTLS and) ZRTP it is though necessary.
	 * But for all others the current_params->media_encryption state should reflect (added by jehan: both) what is agreed by the offer/answer
	 * mechanism  (added by jehan: and encryption status from media which is much stronger than only result of offer/answer )
	 * Typically there can be inactive streams for which the media layer has no idea of whether they are encrypted or not.
	 */
	string authToken = getStreamsGroup().getAuthenticationToken();
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
		if (mainAudioStreamIndex != -1){
			SalStreamDescription *sd = &md->streams[mainAudioStreamIndex];
			getCurrentParams()->setAudioDirection(sd ? MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd->dir) : LinphoneMediaDirectionInactive);
			if (getCurrentParams()->getAudioDirection() != LinphoneMediaDirectionInactive) {
				const char *rtpAddr = (sd->rtp_addr[0] != '\0') ? sd->rtp_addr : md->addr;
				getCurrentParams()->enableAudioMulticast(!!ms_is_multicast(rtpAddr));
			} else
				getCurrentParams()->enableAudioMulticast(false);
			getCurrentParams()->enableAudio(sal_stream_description_enabled(sd));
		}
		if (mainVideoStreamIndex != -1){
			SalStreamDescription *sd = &md->streams[mainVideoStreamIndex];
			getCurrentParams()->getPrivate()->enableImplicitRtcpFb(sd && sal_stream_description_has_implicit_avpf(sd));
			getCurrentParams()->setVideoDirection(sd ? MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd->dir) : LinphoneMediaDirectionInactive);
			if (getCurrentParams()->getVideoDirection() != LinphoneMediaDirectionInactive) {
				const char *rtpAddr = (sd->rtp_addr[0] != '\0') ? sd->rtp_addr : md->addr;
				getCurrentParams()->enableVideoMulticast(!!ms_is_multicast(rtpAddr));
			} else
				getCurrentParams()->enableVideoMulticast(false);
			getCurrentParams()->enableVideo(sal_stream_description_enabled(sd));
		}
		if (mainTextStreamIndex != -1){
			SalStreamDescription *sd = &md->streams[mainTextStreamIndex];
			// Direction and multicast are not supported for real-time text.
			getCurrentParams()->enableRealtimeText(sal_stream_description_enabled(sd));
		}
	}
	getCurrentParams()->getPrivate()->setUpdateCallWhenIceCompleted(getParams()->getPrivate()->getUpdateCallWhenIceCompleted());
}

// -----------------------------------------------------------------------------


void MediaSessionPrivate::startAccept(){
	L_Q();
	/* Give a chance a set card prefered sampling frequency */
	if (localDesc->streams[0].max_rate > 0) {
		lInfo() << "Configuring prefered card sampling rate to [" << localDesc->streams[0].max_rate << "]";
		if (q->getCore()->getCCore()->sound_conf.play_sndcard)
			ms_snd_card_set_preferred_sample_rate(q->getCore()->getCCore()->sound_conf.play_sndcard, localDesc->streams[0].max_rate);
		if (q->getCore()->getCCore()->sound_conf.capt_sndcard)
			ms_snd_card_set_preferred_sample_rate(q->getCore()->getCCore()->sound_conf.capt_sndcard, localDesc->streams[0].max_rate);
	}

	CallSessionPrivate::accept(nullptr);

	SalMediaDescription *newMd = op->getFinalMediaDescription();
	if (newMd) {
		updateStreams(newMd, CallSession::State::StreamsRunning);
		setState(CallSession::State::StreamsRunning, "Connected (streams running)");
	} else
		expectMediaInAck = true;
	if (callAcceptanceDefered) callAcceptanceDefered = false;
}

void MediaSessionPrivate::accept (const MediaSessionParams *msp, bool wasRinging) {
	if (msp) {
		setParams(new MediaSessionParams(*msp));
	}
	if (msp || localDesc == nullptr) makeLocalMediaDescription(op->getRemoteMediaDescription() ? false : true);

	updateRemoteSessionIdAndVer();

	if (getStreamsGroup().prepare()){
		callAcceptanceDefered = true;
		return; /* Deferred until completion of ICE gathering */
	}
	startAccept();
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
	updateRemoteSessionIdAndVer();
	makeLocalMediaDescription(op->getRemoteMediaDescription() ? false : true);

	if (getStreamsGroup().prepare())
		return 0; /* Deferred until completion of ICE gathering */
	startAcceptUpdate(nextState, stateInfo);
	return 0;
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::refreshSockets () {
	getStreamsGroup().refreshSockets();
}

void MediaSessionPrivate::reinviteToRecoverFromConnectionLoss () {
	L_Q();
	lInfo() << "MediaSession [" << q << "] is going to be updated (reINVITE) in order to recover from lost connectivity";
	getStreamsGroup().getIceService().resetSession();
	q->update(getParams());
}

void MediaSessionPrivate::repairByInviteWithReplaces () {
	if ((state == CallSession::State::IncomingEarlyMedia) || (state == CallSession::State::OutgoingEarlyMedia)) {
		stopStreams();
	}
	CallSessionPrivate::repairByInviteWithReplaces();
}

int MediaSessionPrivate::sendDtmf () {
	L_Q();
	LinphoneCore *lc = q->getCore()->getCCore();
	// By default we send DTMF RFC2833 if we do not have enabled SIP_INFO but we can also send RFC2833 and SIP_INFO
	if (linphone_core_get_use_rfc2833_for_dtmf(lc) || !linphone_core_get_use_info_for_dtmf(lc)) {
		AudioControlInterface *i = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
		// In Band DTMF
		if (i)
			i->sendDtmf(dtmfSequence.front());
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
	MediaSession *session = static_cast<MediaSession *>(userData);
	return session->getPrivate()->resumeAfterFailedTransfer();
}

bool_t MediaSessionPrivate::startPendingRefer (void *userData) {
	MediaSession *session = static_cast<MediaSession *>(userData);
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

IceSession *MediaSessionPrivate::getIceSession()const{
	return getIceService().getSession();
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
	d->streamsGroup->getIceService().setListener(d);

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
		d->makeLocalMediaDescription(false);
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
		if (!getCore()->getCCore()->sip_conf.sdp_200_ack){
			/* Do not make a local media description when sending an empty INVITE. */
			d->makeLocalMediaDescription(true);
		}
		d->runStunTestsIfNeeded();
		d->discoverMtu(to);
	} else if (direction == LinphoneCallIncoming) {
		d->selectIncomingIpVersion();
		/* Note that the choice of IP version for streams is later refined by setCompatibleIncomingCallParams() when examining the
		 * remote offer, if any. If the remote offer contains IPv4 addresses, we should propose IPv4 as well. */
		Address cleanedFrom = from;
		cleanedFrom.clean();
		d->setParams(new MediaSessionParams());
		d->params->initDefault(getCore(), LinphoneCallIncoming);
		d->initializeParamsAccordingToIncomingCallParams();
		d->makeLocalMediaDescription(op->getRemoteMediaDescription() ? false : true);
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
			
			d->deferIncomingNotification = d->getStreamsGroup().prepare();
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
			bool ice_needs_defer = d->getStreamsGroup().prepare();
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
	Stream *as = d->getStreamsGroup().lookupMainStream(SalAudio);
	if (as) as->stop();
	d->setState(CallSession::State::Resuming,"Resuming");
	d->makeLocalMediaDescription(true);
	sal_media_description_set_dir(d->localDesc, SalStreamSendRecv);
	if (getCore()->getCCore()->sip_conf.sdp_200_ack)
		d->op->setLocalMediaDescription(nullptr);
	string subject = "Call resuming";
	if (d->getParams()->getPrivate()->getInConference() && !getCurrentParams()->getPrivate()->getInConference())
		subject = "Conference";
	if (d->op->update(subject.c_str(), false) != 0)
		return -1;
	
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
	L_D();
	MediaSessionParams *curParams = getCurrentParams();

	if ((curParams->avpfEnabled() || curParams->getPrivate()->implicitRtcpFbEnabled())) { // || sal_media_description_has_implicit_avpf((const SalMediaDescription *)call->resultdesc)
		lInfo() << "Request Full Intra Request on CallSession [" << this << "]";
		d->getStreamsGroup().forEach<VideoControlInterface>([](VideoControlInterface *i){ i->sendVfuRequest(); });
	} else if (getCore()->getCCore()->sip_conf.vfu_with_info) {
		lInfo() << "Request SIP INFO FIR on CallSession [" << this << "]";
		if (d->state == CallSession::State::StreamsRunning)
			d->op->sendVfuRequest();
	} else
		lInfo() << "vfu request using sip disabled from config [sip,vfu_with_info]";
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
		if (d->localDesc && d->localDesc->streams[0].max_rate > 0)
			ms_snd_card_set_preferred_sample_rate(getCore()->getCCore()->sound_conf.play_sndcard, d->localDesc->streams[0].max_rate);
		d->getStreamsGroup().prepare();
	}

	d->op->setLocalMediaDescription(d->localDesc);

	int result = CallSession::startInvite(destination, subject, content);
	if (result < 0) {
		if (d->state == CallSession::State::Error)
			d->stopStreams();
		return result;
	}
	return result;
}

void MediaSession::startRecording () {
	L_D();
	if (d->getParams()->getRecordFilePath().empty()) {
		lError() << "MediaSession::startRecording(): no output file specified. Use MediaSessionParams::setRecordFilePath()";
		return;
	}
	AudioControlInterface * i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	i->startRecording();
}

void MediaSession::stopRecording () {
	L_D();
	AudioControlInterface * i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	i->stopRecording();
}

bool MediaSession::isRecording () {
	L_D();
	AudioControlInterface * i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	return i->isRecording();
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
	if (msp) {
		d->broken = false;
		d->setState(nextState, "Updating call");
		d->setParams(new MediaSessionParams(*msp));
		if (!d->getParams()->getPrivate()->getNoUserConsent())
			d->makeLocalMediaDescription(true);
		
		if (d->getStreamsGroup().prepare()) {
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
		
		MS2AudioStream *as = d->getStreamsGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
		if (as && ((captureCard != as->getCurrentCaptureCard()) || playCard != as->getCurrentPlaybackCard())) {
			//Ideally this should use the same logic as video (See video_stream_change_camera)
			//I.E. reconstruct only ms2 graphs without destroying the streams.
			//For now, we just stop and restart audio stream with new playback/capture card
			as->stop();
			d->updateStreams(d->resultDesc, d->state);
		}else{
			VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
			if (i) i->parametersChanged();
		}
	}
	return result;
}

// -----------------------------------------------------------------------------

void MediaSession::requestNotifyNextVideoFrameDecoded () {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) i->requestNotifyNextVideoFrameDecoded();
}


LinphoneStatus MediaSession::takePreviewSnapshot (const string& file) {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) return i->takePreviewSnapshot(file);
	return -1;
}

LinphoneStatus MediaSession::takeVideoSnapshot (const string& file) {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) return i->takeVideoSnapshot(file);
	return -1;
}

void MediaSession::zoomVideo (float zoomFactor, float *cx, float *cy) {
	zoomVideo(zoomFactor, *cx, *cy);
}

void MediaSession::zoomVideo (float zoomFactor, float cx, float cy) {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) i->zoomVideo(zoomFactor, cx, cy);
}

// -----------------------------------------------------------------------------

bool MediaSession::cameraEnabled () const {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) return i->cameraEnabled();
	return false;
}

void MediaSession::enableCamera (bool value) {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) i->enableCamera(value);
}

bool MediaSession::echoCancellationEnabled () const {
	L_D();
	AudioControlInterface * i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	return i ? i->echoCancellationEnabled() : false;
}

void MediaSession::enableEchoCancellation (bool value) {
	L_D();
	AudioControlInterface * i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) i->enableEchoCancellation(value);
}

bool MediaSession::echoLimiterEnabled () const {
	lWarning() << "MediaSession::echoLimiterEnabled() unimplemented.";
	return false;
}

void MediaSession::enableEchoLimiter (bool value) {
	lWarning() << "MediaSession::enableEchoLimiter() unimplemented.";
}

bool MediaSession::getAllMuted () const {
	L_D();
	return d->getStreamsGroup().isMuted();
}

LinphoneCallStats * MediaSession::getAudioStats () const {
	return getStats(LinphoneStreamTypeAudio);
}

string MediaSession::getAuthenticationToken () const {
	L_D();
	return d->getStreamsGroup().getAuthenticationToken();
}

bool MediaSession::getAuthenticationTokenVerified () const {
	L_D();
	return d->getStreamsGroup().getAuthenticationTokenVerified();
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
	return d->getStreamsGroup().getCurrentQuality();
}

const MediaSessionParams * MediaSession::getMediaParams () const {
	L_D();
	return d->getParams();
}

RtpTransport * MediaSession::getMetaRtcpTransport (int streamIndex) const {
	L_D();
	MS2Stream *s = dynamic_cast<MS2Stream*>(d->getStreamsGroup().getStream(streamIndex));
	if (!s){
		lError() << "MediaSession::getMetaRtcpTransport(): no stream with index " << streamIndex;
		return nullptr;
	}
	return s->getMetaRtpTransports().second;
}

RtpTransport * MediaSession::getMetaRtpTransport (int streamIndex) const {
	L_D();
	MS2Stream *s = dynamic_cast<MS2Stream*>(d->getStreamsGroup().getStream(streamIndex));
	if (!s){
		lError() << "MediaSession::getMetaRtcpTransport(): no stream with index " << streamIndex;
		return nullptr;
	}
	return s->getMetaRtpTransports().first;
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
		interface->getNativeWindowId();
	}
	return nullptr;
}

void MediaSession::setNativeVideoWindowId (void *id) {
	L_D();
	auto interface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (interface){
		interface->setNativeWindowId(id);
	}
}

void MediaSession::setNativePreviewWindowId(void *id){
	L_D();
	auto interface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (interface){
		interface->setNativePreviewWindowId(id);
	}
}

void * MediaSession::getNativePreviewVideoWindowId () const{
	L_D();
	auto interface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (interface){
		interface->getNativePreviewWindowId();
	}
	return nullptr;
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
		MediaSessionParams * params = nullptr;
		if (md) {
			SalStreamDescription *sd;
			params = new MediaSessionParams();
			
			if (d->mainAudioStreamIndex != -1 && d->mainAudioStreamIndex < md->nb_streams){
				sd = &md->streams[d->mainAudioStreamIndex];
				params->enableAudio(sal_stream_description_enabled(sd));
				params->setMediaEncryption(sal_stream_description_has_srtp(sd) ? LinphoneMediaEncryptionSRTP : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeAudio, md->streams[d->mainAudioStreamIndex].custom_sdp_attributes);
			}else params->enableAudio(false);
			
			if (d->mainVideoStreamIndex != -1 && d->mainVideoStreamIndex < md->nb_streams){
				sd = &md->streams[d->mainVideoStreamIndex];
				params->enableVideo(sal_stream_description_enabled(sd));
				params->setMediaEncryption(sal_stream_description_has_srtp(sd) ? LinphoneMediaEncryptionSRTP : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeVideo, md->streams[d->mainVideoStreamIndex].custom_sdp_attributes);
			}else params->enableVideo(false);
			
			if (d->mainTextStreamIndex != -1 && d->mainTextStreamIndex < md->nb_streams){
				sd = &md->streams[d->mainTextStreamIndex];
				params->enableRealtimeText(sal_stream_description_enabled(sd));
				params->setMediaEncryption(sal_stream_description_has_srtp(sd) ? LinphoneMediaEncryptionSRTP : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeText, md->streams[d->mainTextStreamIndex].custom_sdp_attributes);
			}else params->enableRealtimeText(false);
			
			if (!params->videoEnabled()) {
				if ((md->bandwidth > 0) && (md->bandwidth <= linphone_core_get_edge_bw(getCore()->getCCore())))
					params->enableLowBandwidth(true);
			}
			if (md->name[0] != '\0')
				params->setSessionName(md->name);
			params->getPrivate()->setCustomSdpAttributes(md->custom_sdp_attributes);
			params->enableRtpBundle(md->bundles != nullptr);
		}
		const SalCustomHeader *ch = d->op->getRecvCustomHeaders();
		if (ch) {
			if (!params) params = new MediaSessionParams();
			params->getPrivate()->setCustomHeaders(ch);
		}
		d->setRemoteParams(params);
		return params;
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
		statsCopy = (LinphoneCallStats*) belle_sip_object_clone((belle_sip_object_t*)stats);
	}
	return statsCopy;
}

int MediaSession::getStreamCount () const {
	L_D();
	return (int)d->getStreamsGroup().size();
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
	//L_D();
	/* FIXME */
	return false;
}

void MediaSession::setAudioRoute (LinphoneAudioRoute route) {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) i->setRoute(route);
}

void MediaSession::setAuthenticationTokenVerified (bool value) {
	L_D();
	d->getStreamsGroup().setAuthTokenVerified(value);
	if (!value) {
		char *peerDeviceId = nullptr;
		auto encryptionEngine = getCore()->getEncryptionEngine();
		if (encryptionEngine) { //inform lime that zrtp no longuer guaranty the trust
			const SalAddress *remoteAddress = d->getOp()->getRemoteContactAddress();
			peerDeviceId = sal_address_as_string_uri_only(remoteAddress);
			encryptionEngine->authenticationRejected(peerDeviceId);
			ms_free(peerDeviceId);
		}
	}
	d->propagateEncryptionChanged();
}

void MediaSession::setParams (const MediaSessionParams *msp) {
	L_D();
		
	switch(d->state){
		case CallSession::State::OutgoingInit:
		case CallSession::State::IncomingReceived:
			d->setParams(msp ? new MediaSessionParams(*msp) : nullptr);
			// Update the local media description.
			d->makeLocalMediaDescription(d->state == CallSession::State::OutgoingInit ? 
				!getCore()->getCCore()->sip_conf.sdp_200_ack : false);
		break;
		default:
			lError() << "MediaSession::setParams(): Invalid state %s", Utils::toString(d->state);
		break;
	}
}



LINPHONE_END_NAMESPACE
