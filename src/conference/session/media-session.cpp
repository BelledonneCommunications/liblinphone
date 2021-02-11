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
#include <algorithm>

#include "call/call.h"
#include "address/address.h"
#include "chat/chat-room/client-group-chat-room.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "core/core-p.h"
#include "c-wrapper/c-wrapper.h"
#include "sal/sal_stream_bundle.h"
#include "sal/sal_media_description.h"
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

LinphoneMediaEncryption MediaSessionPrivate::getEncryptionFromMediaDescription(const std::shared_ptr<SalMediaDescription> & md) const {
	L_Q();

	LinphoneMediaEncryption enc = LinphoneMediaEncryptionNone;
	if (atLeastOneStreamStarted() && (md->hasDtls() || allStreamsEncrypted())) {
		if (md->hasSrtp()) {
			enc = LinphoneMediaEncryptionSRTP;
		} else if (md->hasDtls()) {
			enc = LinphoneMediaEncryptionDTLS;
		} else if (md->hasZrtp()) {
			enc = LinphoneMediaEncryptionZRTP;
		} else {
			enc = LinphoneMediaEncryptionNone;
		}

		if (linphone_core_media_encryption_supported(q->getCore()->getCCore(), enc)) {
			return enc;
		}
	}

	// Do not change encryption if no stream is started or at least one is not encrypted or chosen encryption is not supported
	return getParams()->getMediaEncryption();
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
				makeLocalMediaDescription(op->getRemoteMediaDescription() ? false : true, q->isCapabilityNegotiationEnabled());
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
				updateLocalMediaDescriptionFromIce(localIsOfferer);
			}
		break;
		default:
		break;
	}

	/* Reset the internal call update flag, so it doesn't risk to be copied and used in further re-INVITEs */
	getParams()->getPrivate()->setInternalCallUpdate(false);
	std::shared_ptr<SalMediaDescription> rmd = op->getRemoteMediaDescription();
	std::shared_ptr<SalMediaDescription> & md = op->getFinalMediaDescription();
	if (!md && (prevState == CallSession::State::OutgoingEarlyMedia) && resultDesc) {
		lInfo() << "Using early media SDP since none was received with the 200 OK";
		md = resultDesc;
	}
	if (md && (md->isEmpty() || q->getCore()->incompatibleSecurity(md)))
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
				if (!localDesc->hasDir(SalStreamInactive)
					&& (md->hasDir(SalStreamRecvOnly) || md->hasDir(SalStreamInactive))) {
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
			if ((prevState  == CallSession::State::Connected) && (nextState == CallSession::State::StreamsRunning)) {
				// If capability negotiation is enabled, a second invite must be sent if the selected configuration is not the actual one.
				// It normally occurs after moving to state StreamsRunning. However, if ICE negotiations are not completed, then this action will be carried out together with the ICE re-INVITE
				if (localDesc->hasCapabilityNegotiation()) {
					// If no ICE session or checklist has completed, then send re-INVITE
					 if (!getStreamsGroup().getIceService().getSession() || (getStreamsGroup().getIceService().getSession() && getStreamsGroup().getIceService().hasCompletedCheckList())) {
						if (md->compareToActualConfiguration(*localDesc) != SAL_MEDIA_DESCRIPTION_UNCHANGED) {
							MediaSessionParams newParams(*getParams());
							q->update(&newParams);
						}
					}
				}
			}
		}
	} else { /* Invalid or no SDP */
		switch (prevState) {
			/* Send a bye only in case of early states */
			case CallSession::State::OutgoingInit:
			case CallSession::State::OutgoingProgress:
			case CallSession::State::OutgoingRinging:
			case CallSession::State::OutgoingEarlyMedia:
			case CallSession::State::IncomingReceived:
			case CallSession::State::PushIncomingReceived:
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
				setState(CallSession::State::UpdatedByRemote, "Updated by remote");
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
			if ((state == CallSession::State::OutgoingInit) 
				|| (state == CallSession::State::OutgoingProgress)
				|| (state == CallSession::State::OutgoingRinging) /* Push notification case */ 
				|| (state == CallSession::State::OutgoingEarlyMedia)) {
				bool mediaEncrptionSrtp = getParams()->getMediaEncryption() == LinphoneMediaEncryptionSRTP;
				bool avpfEnabled = getParams()->avpfEnabled();
				if (mediaEncrptionSrtp || avpfEnabled) {
					lInfo() << "Outgoing CallSession [" << q << "] failed with SRTP and/or AVPF enabled";
					string previousCallId = op->getCallId();
					for (auto & stream : localDesc->streams) {
						bool firstStream = (stream == localDesc->streams[0]);
						if (!stream.enabled())
							continue;
						if (mediaEncrptionSrtp) {
							if (avpfEnabled) {
								if (firstStream)
									lInfo() << "Retrying CallSession [" << q << "] with SAVP";
								getParams()->enableAvpf(false);
								restartInvite();
								linphone_core_notify_call_id_updated(q->getCore()->getCCore(), previousCallId.c_str(), op->getCallId().c_str());
								return true;
							} else if (!linphone_core_is_media_encryption_mandatory(q->getCore()->getCCore())) {
								if (firstStream)
									lInfo() << "Retrying CallSession [" << q << "] with AVP";
								getParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
								stream.crypto.clear();
								getParams()->enableAvpf(false);
								restartInvite();
								linphone_core_notify_call_id_updated(q->getCore()->getCCore(), previousCallId.c_str(), op->getCallId().c_str());
								return true;
							}
						} else if (avpfEnabled) {
							if (firstStream) 
								lInfo() << "Retrying CallSession [" << q << "] with AVP";
							getParams()->enableAvpf(false);
							getParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
							stream.cfgs[stream.getChosenConfigurationIndex()].crypto.clear();
							restartInvite();
							linphone_core_notify_call_id_updated(q->getCore()->getCCore(), previousCallId.c_str(), op->getCallId().c_str());
							return true;
						}
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
	q->getCore()->getPrivate()->getToneManager()->stop(q->getSharedFromThis());
	q->getCore()->getPrivate()->getToneManager()->startErrorTone(q->getSharedFromThis(), linphone_reason_from_sal(ei->reason));// Play an error tone if it is allowed

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
	if (linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "inactive_video_on_pause", 0))
		newParams.setVideoDirection(LinphoneMediaDirectionInactive);

	acceptUpdate(&newParams, CallSession::State::PausedByRemote, "Call paused by remote");

}

void MediaSessionPrivate::remoteRinging () {
	L_Q();
	/* Set privacy */
	getCurrentParams()->setPrivacy((LinphonePrivacyMask)op->getPrivacy());
	std::shared_ptr<SalMediaDescription> md = op->getFinalMediaDescription();
	if (md) {
		std::shared_ptr<SalMediaDescription> rmd = op->getRemoteMediaDescription();
		/* Initialize the remote call params by invoking linphone_call_get_remote_params(). This is useful as the SDP may not be present in the 200Ok */
		q->getRemoteParams();
		/* Accept early media */

		if (rmd && getStreamsGroup().isStarted()){
			OfferAnswerContext ctx;
			ctx.localMediaDescription = localDesc;
			ctx.resultMediaDescription = md;
			ctx.remoteMediaDescription = rmd;
			getStreamsGroup().tryEarlyMediaForking(ctx);
			return;
		}

		setState(CallSession::State::OutgoingEarlyMedia, "Early media");
		q->getCore()->getPrivate()->getToneManager()->stop(q->getSharedFromThis());
		lInfo() << "Doing early media...";
		updateStreams(md, state);

		if ((q->getCurrentParams()->getAudioDirection() == LinphoneMediaDirectionInactive)) {
			q->getCore()->getPrivate()->getToneManager()->startRingbackTone(q->getSharedFromThis());
		}
	} else {
		if (state == CallSession::State::OutgoingEarlyMedia) {
			/* Already doing early media */
			return;
		}

		// Start ringback tone before moving to next state as we need to retrieve the output device of the state we are currently in
		q->getCore()->getPrivate()->getToneManager()->startRingbackTone(q->getSharedFromThis());
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
	L_Q();
	stopStreams();
	CallSessionPrivate::terminated();
	q->getCore()->getPrivate()->getToneManager()->stop(q->getSharedFromThis());// Stop all tones and start limited one
	q->getCore()->getPrivate()->getToneManager()->startNamedTone(q->getSharedFromThis(), LinphoneToneCallEnd);
}

/* This callback is called when an incoming re-INVITE/ SIP UPDATE modifies the session */
void MediaSessionPrivate::updated (bool isUpdate) {
	const std::shared_ptr<SalMediaDescription> & rmd = op->getRemoteMediaDescription();
	switch (state) {
		case CallSession::State::PausedByRemote:
			if (rmd->hasDir(SalStreamSendRecv) || rmd->hasDir(SalStreamRecvOnly)) {
				resumed();
				return;
			}
			break;
		case CallSession::State::StreamsRunning:
		case CallSession::State::Connected:
		case CallSession::State::UpdatedByRemote: /* Can happen on UAC connectivity loss */
			if (rmd->hasDir(SalStreamSendOnly) || rmd->hasDir(SalStreamInactive)) {
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
	std::shared_ptr<SalMediaDescription> rmd = op->getRemoteMediaDescription();
	if (state != CallSession::State::Paused) {
		/* Refresh the local description, but in paused state, we don't change anything. */
		if (!rmd && linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "sdp_200_ack_follow_video_policy", 0)) {
			lInfo() << "Applying default policy for offering SDP on CallSession [" << q << "]";
			setParams(new MediaSessionParams());
			// Yes we init parameters as if we were in the case of an outgoing call, because it is a resume with no SDP.
			params->initDefault(q->getCore(), LinphoneCallOutgoing);
		}

		makeLocalMediaDescription((rmd == nullptr), false);
	}
	// Fix local parameter after creating new local media description
	fixCallParams(rmd, true);
	if (rmd) {
		SalErrorInfo sei;
		memset(&sei, 0, sizeof(sei));
		expectMediaInAck = false;
		std::shared_ptr<SalMediaDescription> & md = op->getFinalMediaDescription();
		if (md && (md->isEmpty() || q->getCore()->incompatibleSecurity(md))) {
			sal_error_info_set(&sei, SalReasonNotAcceptable, "SIP", 0, nullptr, nullptr);
			op->declineWithErrorInfo(&sei, nullptr);
			sal_error_info_reset(&sei);
			return;
		}
		std::shared_ptr<SalMediaDescription> & prevResultDesc = resultDesc;
		if (isUpdate && prevResultDesc && md){
			int diff = md->equal(*prevResultDesc);
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
	if (i) i->enableMic(!muted);
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
	if (getStreamsGroup().isStarted()) getStreamsGroup().stop();
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {
	L_Q();
	if (mediaNetworkReachable) {
		LinphoneConfig *config = linphone_core_get_config(q->getCore()->getCCore());
		if (linphone_config_get_int(config, "net", "recreate_sockets_when_network_is_up", 0))
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
	std::shared_ptr<SalMediaDescription> rmd = nullptr;
	switch (newState) {
		case CallSession::State::UpdatedByRemote:
			// Handle specifically the case of an incoming ICE-concluded reINVITE
			lInfo() << "Checking for ICE reINVITE";
			rmd = op->getRemoteMediaDescription();
			if (rmd && getIceService().reinviteNeedsDeferedResponse(rmd)) {
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


int MediaSessionPrivate::getFirstStreamWithType(const std::shared_ptr<SalMediaDescription> & md, SalStreamType type){
	for (size_t i = 0; i < md->streams.size(); ++i) {
		if (md->streams[i].type == type) return static_cast<int>(i);
	}
	return -1;
}

void MediaSessionPrivate::assignStreamsIndexes(bool localIsOfferer){
	if (biggestDesc && freeStreamIndex < static_cast<int>(biggestDesc->streams.size())) freeStreamIndex = static_cast<int>(biggestDesc->streams.size());

	/*Initialize stream indexes from potential incoming offer.*/
	std::shared_ptr<SalMediaDescription> rmd = op ? op->getRemoteMediaDescription() : nullptr;
	if (rmd) assignStreamsIndexesIncoming(rmd);
	
	if (!localIsOfferer) return;

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

void MediaSessionPrivate::assignStreamsIndexesIncoming(const std::shared_ptr<SalMediaDescription> & md) {
	if (mainAudioStreamIndex == -1){
		mainAudioStreamIndex = getFirstStreamWithType(md, SalAudio);
	}
	if (mainVideoStreamIndex == -1){
		mainVideoStreamIndex = getFirstStreamWithType(md, SalVideo);
	}
	if (mainTextStreamIndex == -1){
		mainTextStreamIndex = getFirstStreamWithType(md, SalText);
	}
	if (freeStreamIndex < static_cast<int>(md->streams.size())) freeStreamIndex = static_cast<int>(md->streams.size());
}

/*
 * This method needs to be called at each incoming reINVITE, in order to adjust various local parameters to what is being offered by remote:
 * - the stream indexes.
 * - the video enablement parameter according to what is offered and our local policy.
 * Fixing the params to proper values avoid request video by accident during internal call updates, pauses and resumes
 */
void MediaSessionPrivate::fixCallParams (std::shared_ptr<SalMediaDescription> & rmd, bool fromOffer) {
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

		bool isInLocalConference = getParams()->getPrivate()->getInConference();

		if (isInLocalConference) {
			// If the call is in a local conference, then check conference capabilities to know whether the video must be enabled or not
			bool isConferenceVideoCapabilityOn = false;
			LinphoneConference * conference = nullptr;
			if (listener) {
				conference = listener->getCallSessionConference(q->getSharedFromThis());
				if (conference) {
					const LinphoneConferenceParams * params = linphone_conference_get_current_params(conference);
					isConferenceVideoCapabilityOn = linphone_conference_params_video_enabled(params);
					if (rcp->videoEnabled() && linphone_core_video_enabled(q->getCore()->getCCore()) && !getParams()->videoEnabled()) {
						getParams()->enableVideo(isConferenceVideoCapabilityOn);
					}
				}
			}
		} else {
			if (rcp->videoEnabled() && q->getCore()->getCCore()->video_policy.automatically_accept && linphone_core_video_enabled(q->getCore()->getCCore()) && !getParams()->videoEnabled()) {
				lInfo() << "CallSession [" << q << "]: re-enabling video in our call params because the remote wants it and the policy allows to automatically accept";
				getParams()->enableVideo(true);
			}
		}
	}
}

void MediaSessionPrivate::initializeParamsAccordingToIncomingCallParams () {
	CallSessionPrivate::initializeParamsAccordingToIncomingCallParams();
	std::shared_ptr<SalMediaDescription> md = op->getRemoteMediaDescription();
	if (md) {
		assignStreamsIndexesIncoming(md);
		/* It is licit to receive an INVITE without SDP, in this case WE choose the media parameters according to policy */
		setCompatibleIncomingCallParams(md);
	}
}

bool MediaSessionPrivate::hasAvpf(const std::shared_ptr<SalMediaDescription> & md)const{
	/* We consider that AVPF is enabled if at least one of these condition is satisfied:
	 * - all the offered streams have AVPF
	 * - the video stream has AVPF.
	 * In practice, this means for a remote media description that AVPF is supported by the far end.
	 */
	bool hasAvpf = !!md->hasAvpf();
	if (mainVideoStreamIndex != -1 && (mainVideoStreamIndex < (int)md->streams.size()) && md->streams[static_cast<size_t>(mainVideoStreamIndex)].hasAvpf()){
		hasAvpf = true;
	}
	return hasAvpf;
}

/**
 * Fix call parameters on incoming call to eg. enable AVPF if the incoming call propose it and it is not enabled locally.
 */
void MediaSessionPrivate::setCompatibleIncomingCallParams (std::shared_ptr<SalMediaDescription> & md) {
	L_Q();
	LinphoneCore *lc = q->getCore()->getCCore();
	/* Handle AVPF, SRTP and DTLS */
	
	
	getParams()->enableAvpf(hasAvpf(md));
	if (destProxy)
		getParams()->setAvpfRrInterval(static_cast<uint16_t>(linphone_proxy_config_get_avpf_rr_interval(destProxy) * 1000));
	else
		getParams()->setAvpfRrInterval(static_cast<uint16_t>(linphone_core_get_avpf_rr_interval(lc) * 1000));
	bool_t mandatory = linphone_core_is_media_encryption_mandatory(lc);
	if (md->hasZrtp() && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionZRTP)) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionZRTP))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionZRTP);
	} else if (md->hasDtls() && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionDTLS)) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionDTLS))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionDTLS);
	} else if (md->hasSrtp() && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionSRTP)) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionSRTP))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionSRTP);
	} else if (getParams()->getMediaEncryption() != LinphoneMediaEncryptionZRTP) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionNone))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
	}
	if ((mainAudioStreamIndex != -1) && (mainAudioStreamIndex < static_cast<int>(md->streams.size()))){
		const SalStreamDescription & sd = md->streams[static_cast<size_t>(mainAudioStreamIndex)];
		const std::string & rtpAddr = (sd.rtp_addr.empty() == false) ? sd.rtp_addr : md->addr;
		if (ms_is_multicast(rtpAddr.c_str())){
			lInfo() << "Incoming offer has audio multicast, enabling it in local params.";
			getParams()->enableAudioMulticast(true);
		}else getParams()->enableAudioMulticast(false);
	}
	if ((mainVideoStreamIndex != -1) && (mainVideoStreamIndex < (int)md->streams.size())){
		const SalStreamDescription & sd = md->streams[static_cast<size_t>(mainVideoStreamIndex)];
		const std::string & rtpAddr = (sd.rtp_addr.empty() == false) ? sd.rtp_addr : md->addr;
		if (ms_is_multicast(rtpAddr.c_str())){
			lInfo() << "Incoming offer has video multicast, enabling it in local params.";
			getParams()->enableVideoMulticast(true);
		}else getParams()->enableVideoMulticast(false);
	}

	/* In case of nat64, even ipv4 addresses are reachable from v6. Should be enhanced to manage stream by stream connectivity (I.E v6 or v4) */
	/*if (!md->hasIpv6()){
		lInfo() << "The remote SDP doesn't seem to offer any IPv6 connectivity, so disabling IPv6 for this call";
		af = AF_INET;
	}*/
	fixCallParams(md, true);
}

void MediaSessionPrivate::updateBiggestDesc (std::shared_ptr<SalMediaDescription> & md) {
	if (!biggestDesc || (md->streams.size() > biggestDesc->streams.size())) {
		/* We have been offered and now are ready to proceed, or we added a new stream,
		 * store the media description to remember the mapping of streams within this call. */
		biggestDesc = md;
	}
}

void MediaSessionPrivate::updateRemoteSessionIdAndVer () {
	const std::shared_ptr<SalMediaDescription> & desc = op->getRemoteMediaDescription();
	if (desc) {
		remoteSessionId = desc->session_id;
		remoteSessionVer = desc->session_ver;
	}
}

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
	if (destProxy && linphone_proxy_config_get_op(destProxy)) {
		ip = linphone_proxy_config_get_op(destProxy)->getLocalAddress(nullptr);
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

int MediaSessionPrivate::portFromStreamIndex(int index){
	if (index != -1){
		auto stream = getStreamsGroup().getStream(mainAudioStreamIndex);
		if (stream) return stream->getPortConfig().rtpPort;
	}
	return 0;
}

/*
 * This is the deprecated basic STUN-based IP/port discovery. It is unreliable, we prefer using ICE.
 */
void MediaSessionPrivate::runStunTestsIfNeeded () {
	L_Q();
	if (linphone_nat_policy_stun_enabled(natPolicy) && !(linphone_nat_policy_ice_enabled(natPolicy) || linphone_nat_policy_turn_enabled(natPolicy))) {
		stunClient = makeUnique<StunClient>(q->getCore());
		int audioPort = portFromStreamIndex(mainAudioStreamIndex);
		int videoPort = portFromStreamIndex(mainVideoStreamIndex);
		int textPort = portFromStreamIndex(mainTextStreamIndex);
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
		if (destProxy && linphone_proxy_config_get_op(destProxy))
			af = linphone_proxy_config_get_op(destProxy)->getAddressFamily();
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
		if (destProxy && linphone_proxy_config_get_op(destProxy)) {
			// We can determine from the proxy connection whether IPv6 works - this is the most reliable
			af = linphone_proxy_config_get_op(destProxy)->getAddressFamily();
		} else if (sal_address_is_ipv6(L_GET_CPP_PTR_FROM_C_OBJECT(to)->getInternalAddress())) {
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

void MediaSessionPrivate::forceStreamsDirAccordingToState (std::shared_ptr<SalMediaDescription> & md) {
	L_Q();
	for (size_t i = 0; i < md->streams.size(); i++) {
		SalStreamDescription &sd = md->streams[i];
		CallSession::State stateToConsider = state;
		
		switch (stateToConsider){
			case CallSession::State::UpdatedByRemote:
				stateToConsider = prevState;
			break;
			default:
			break;
		}
		
		switch (stateToConsider) {
			case CallSession::State::Pausing:
			case CallSession::State::Paused:
				if (sd.getDirection() != SalStreamInactive) {
					sd.setDirection(SalStreamSendOnly);
					if ((sd.type == SalVideo) && linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "inactive_video_on_pause", 0))
						sd.setDirection(SalStreamInactive);
				}
				break;
			default:
				break;
		}
		/* Reflect the stream directions in the call params */
		if (static_cast<int>(i) == mainAudioStreamIndex)
			getCurrentParams()->setAudioDirection(MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd.getChosenConfiguration().dir));
		else if (static_cast<int>(i) == mainVideoStreamIndex)
			getCurrentParams()->setVideoDirection(MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd.getChosenConfiguration().dir));
	}
}

bool MediaSessionPrivate::generateB64CryptoKey (size_t keyLength, std::string & keyOut, size_t keyOutSize) const {
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
	// Initialize characters with \0
	char * key = new char [keyOutSize] {'\0'};
	b64Size = b64::b64_encode((const char *)tmp, keyLength, key, keyOutSize);
	if (b64Size == 0) {
		lError() << "Failed to b64 encode key";
		ms_free(tmp);
		delete[] key;
		return false;
	}
	keyOut.assign(key);
	delete[] key;
	ms_free(tmp);
	return true;
}

void MediaSessionPrivate::addStreamToBundle(std::shared_ptr<SalMediaDescription> & md, SalStreamDescription &sd, SalStreamConfiguration & cfg, const std::string mid){
	SalStreamBundle bundle;
	if (!md->bundles.empty()){
		bundle = md->bundles.front();
		// Delete first element
		md->bundles.erase(md->bundles.begin());
	}
	bundle.addStream(cfg, mid);
	cfg.mid_rtp_ext_header_id = rtpExtHeaderMidNumber;
	/* rtcp-mux must be enabled when bundle mode is proposed.*/
	cfg.rtcp_mux = TRUE;
	md->bundles.push_front(bundle);
}

/* This function is to authorize the downgrade from avpf to non-avpf, when avpf is enabled locally but the remote
 * offer doesn't offer it consistently for all streams.
 */
SalMediaProto MediaSessionPrivate::getAudioProto(const std::shared_ptr<SalMediaDescription> remote_md){
	SalMediaProto requested = getAudioProto();
	if (remote_md) {
		const SalStreamDescription &remote_stream = remote_md->streams[static_cast<size_t>(mainAudioStreamIndex)];
		if (!remote_stream.hasAvpf()) {
			switch(requested){
				case SalProtoRtpAvpf:
					requested = SalProtoRtpAvp;
				break;
				case SalProtoRtpSavpf:
					requested = SalProtoRtpSavp;
				break;
				default:
				break;
			}
		}
	}
	return requested;
}

SalMediaProto MediaSessionPrivate::getAudioProto(){
	L_Q();
	/*This property is mainly used for testing hybrid case where the SDP offer is made with AVPF only for video stream.*/
	SalMediaProto ret = getParams()->getMediaProto();
	
	if (linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "misc", "no_avpf_for_audio", false)){
		lInfo() << "Removing AVPF for audio mline.";
		switch (ret){
			case SalProtoRtpAvpf:
				ret = SalProtoRtpAvp;
			break;
			case SalProtoRtpSavpf:
				ret = SalProtoRtpSavp;
			break;
			default:
			break;
		}
	}
	return ret;
}

void MediaSessionPrivate::makeLocalStreamDecription(std::shared_ptr<SalMediaDescription> & md, const bool enabled, const std::string name, const size_t & idx, const SalStreamType type, const SalMediaProto proto, const SalStreamDir dir, const std::list<OrtpPayloadType*> & codecs, const std::string mid, const bool & multicastEnabled, const int & ttl, const SalCustomSdpAttribute *customSdpAttributes) {
	L_Q();
	SalStreamConfiguration cfg;
	cfg.proto = proto;
	md->streams[idx].type = type;
	if (enabled && !codecs.empty()) {
		md->streams[idx].name = name;
		cfg.dir = dir;
		const auto & core = q->getCore()->getCCore();
		bool rtcpMux = !!linphone_config_get_int(linphone_core_get_config(core), "rtp", "rtcp_mux", 0);
		cfg.rtcp_mux = rtcpMux;
		md->streams[idx].rtp_port = SAL_STREAM_DESCRIPTION_PORT_TO_BE_DETERMINED;

		cfg.replacePayloads(codecs);
		cfg.rtcp_cname = getMe()->getAddress().asString();
		if (getParams()->rtpBundleEnabled()) addStreamToBundle(md, md->streams[idx], cfg, mid);

		if (multicastEnabled) {
			cfg.ttl = ttl;
			md->streams[idx].multicast_role = (direction == LinphoneCallOutgoing) ? SalMulticastSender : SalMulticastReceiver;
		}

	} else {
		lInfo() << "Don't put stream of type " << sal_stream_type_to_string(type) << " on local offer for CallSession [" << q << "]";
		cfg.dir = SalStreamInactive;
		md->streams[idx].rtp_port = 0;
	}
	if (customSdpAttributes)
		cfg.custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);

	md->streams[idx].addActualConfiguration(cfg);
}

void MediaSessionPrivate::makeLocalMediaDescription(bool localIsOfferer, const bool addCapabilityNegotiationAttributes) {
	L_Q();

	const auto & core = q->getCore()->getCCore();
	std::shared_ptr<SalMediaDescription> md = std::make_shared<SalMediaDescription>(addCapabilityNegotiationAttributes);
	std::shared_ptr<SalMediaDescription> & oldMd = localDesc;

	this->localIsOfferer = localIsOfferer;
	assignStreamsIndexes(localIsOfferer);

	getParams()->getPrivate()->adaptToNetwork(core, pingTime);

	string subject = q->getParams()->getSessionName();
	if (!subject.empty()) {
		md->name = subject;
	}
	md->session_id = (oldMd ? oldMd->session_id : (bctbx_random() & 0xfff));
	md->session_ver = (oldMd ? (oldMd->session_ver + 1) : (bctbx_random() & 0xfff));

	md->accept_bundles = getParams()->rtpBundleEnabled() ||
		linphone_config_get_bool(linphone_core_get_config(core), "rtp", "accept_bundle", TRUE);

	/* Re-check local ip address each time we make a new offer, because it may change in case of network reconnection */
	{
		LinphoneAddress *address = (direction == LinphoneCallOutgoing ? log->to : log->from);
		getLocalIp(*L_GET_CPP_PTR_FROM_C_OBJECT(address));
	}

	md->addr = mediaLocalIp;

	LinphoneAddress *addr = nullptr;
	if (destProxy) {
		addr = linphone_address_clone(linphone_proxy_config_get_identity_address(destProxy));
	} else {
		addr = linphone_address_new(linphone_core_get_identity(core));
	}
	if (linphone_address_get_username(addr)) {/* Might be null in case of identity without userinfo */
		md->username = linphone_address_get_username(addr);
	}

	linphone_address_unref(addr);

	int bandwidth = getParams()->getPrivate()->getDownBandwidth();
	if (bandwidth)
		md->bandwidth = bandwidth;
	else
		md->bandwidth = linphone_core_get_download_bandwidth(core);

	SalCustomSdpAttribute *customSdpAttributes = getParams()->getPrivate()->getCustomSdpAttributes();
	if (customSdpAttributes)
		md->custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);

	PayloadTypeHandler pth(q->getCore());

	if (md->streams.size() <= static_cast<size_t>(freeStreamIndex)) {
		md->streams.resize(static_cast<size_t>(freeStreamIndex));
	}

	std::list<OrtpPayloadType*> codecs;
	// Declare here an empty list to give to the makeCodecsList if there is no valid already assigned payloads
	std::list<OrtpPayloadType*> emptyList;
	emptyList.clear();

	if (addCapabilityNegotiationAttributes) {
		bctbx_list_t * encs = linphone_core_get_supported_media_encryptions(core);

		for (decltype(encs) elem = encs; elem != NULL; elem = bctbx_list_next(elem)) {
			const char *enc = static_cast<const char *>(elem->data);
			const LinphoneMediaEncryption encEnum = static_cast<LinphoneMediaEncryption>(string_to_linphone_media_encryption(enc));
			const std::string mediaProto(sal_media_proto_to_string(getParams()->getMediaProto(encEnum, getParams()->avpfEnabled())));
			const auto & idx = md->getFreeTcapIdx();

			lInfo() << "Adding media protocol " << mediaProto << " at index " << idx;
			md->addTcap(idx, mediaProto);
		}

		if (encs) {
			bctbx_list_free_with_data(encs, (bctbx_list_free_func)bctbx_free);
		}
	}

	if (mainAudioStreamIndex != -1){
		size_t audioStreamIndex = static_cast<size_t>(mainAudioStreamIndex);

		auto audioCodecs = pth.makeCodecsList(SalAudio, getParams()->getAudioBandwidthLimit(), -1, (oldMd && (audioStreamIndex < oldMd->streams.size())) ? oldMd->streams[audioStreamIndex].already_assigned_payloads : emptyList);

		makeLocalStreamDecription(md, getParams()->audioEnabled(), "Audio", audioStreamIndex, SalAudio, getAudioProto(op ? op->getRemoteMediaDescription() : nullptr), getParams()->getPrivate()->getSalAudioDirection(), audioCodecs, "as", getParams()->audioMulticastEnabled(), linphone_core_get_audio_multicast_ttl(core), getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeAudio));

		auto & actualCfg = md->streams[audioStreamIndex].cfgs[md->streams[audioStreamIndex].getActualConfigurationIndex()];
		actualCfg.max_rate = pth.getMaxCodecSampleRate(audioCodecs);
		int downPtime = getParams()->getPrivate()->getDownPtime();
		if (downPtime)
			actualCfg.ptime = downPtime;
		else
			actualCfg.ptime = linphone_core_get_download_ptime(core);

		PayloadTypeHandler::clearPayloadList(audioCodecs);
	}
	if (mainVideoStreamIndex != -1){
		size_t videoStreamIndex = static_cast<size_t>(mainVideoStreamIndex);
		auto videoCodecs = pth.makeCodecsList(SalVideo, 0, -1,
			(oldMd && (videoStreamIndex < oldMd->streams.size())) ? oldMd->streams[videoStreamIndex].already_assigned_payloads : emptyList);

		makeLocalStreamDecription(md, getParams()->videoEnabled(), "Video", videoStreamIndex, SalVideo, getParams()->getMediaProto(), getParams()->getPrivate()->getSalVideoDirection(), videoCodecs, "vs", getParams()->videoMulticastEnabled(), linphone_core_get_video_multicast_ttl(core), getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeVideo));
		md->streams[videoStreamIndex].bandwidth = getParams()->getPrivate()->videoDownloadBandwidth;
		PayloadTypeHandler::clearPayloadList(videoCodecs);
	}

	if (mainTextStreamIndex != -1){
		size_t textStreamIndex = static_cast<size_t>(mainTextStreamIndex);
		auto textCodecs = pth.makeCodecsList(SalText, 0, -1,
				(oldMd && (textStreamIndex < oldMd->streams.size())) ? oldMd->streams[textStreamIndex].already_assigned_payloads : emptyList);

		makeLocalStreamDecription(md, getParams()->realtimeTextEnabled(), "Text", textStreamIndex, SalText, getParams()->getMediaProto(), SalStreamSendRecv, textCodecs, "ts", false, 0, getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeText));
		PayloadTypeHandler::clearPayloadList(textCodecs);
	}

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
	if (mainAudioStreamIndex != -1) getStreamsGroup().setStreamMain(static_cast<size_t>(mainAudioStreamIndex));
	if (mainVideoStreamIndex != -1) getStreamsGroup().setStreamMain(static_cast<size_t>(mainVideoStreamIndex));
	if (mainTextStreamIndex != -1) getStreamsGroup().setStreamMain(static_cast<size_t>(mainTextStreamIndex));
	/* Get the transport addresses filled in to the media description. */
	getStreamsGroup().fillLocalMediaDescription(ctx);

	updateLocalMediaDescriptionFromIce(localIsOfferer);
	if (oldMd) {
		transferAlreadyAssignedPayloadTypes(oldMd, md);
		localDescChanged = md->equal(*oldMd);
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

int MediaSessionPrivate::setupEncryptionKey (SalSrtpCryptoAlgo & crypto, MSCryptoSuite suite, unsigned int tag) const {
	crypto.tag = tag;
	crypto.algo = suite;
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
	if ((keylen == 0) || !generateB64CryptoKey(keylen, crypto.master_key, SAL_SRTP_KEY_SIZE)) {
		lError() << "Could not generate SRTP key";
		crypto.algo = MS_CRYPTO_SUITE_INVALID;
		return -1;
	}
	return 0;
}

void MediaSessionPrivate::setupRtcpFb (std::shared_ptr<SalMediaDescription> & md) {
	L_Q();
	for (auto & stream : md->streams) {
		stream.setupRtcpFb(!!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_fb_generic_nack_enabled", 0), !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_fb_tmmbr_enabled", 1), getParams()->getPrivate()->implicitRtcpFbEnabled());
		for (const auto & pt : stream.getPayloads()) {
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

void MediaSessionPrivate::setupRtcpXr (std::shared_ptr<SalMediaDescription> & md) {
	L_Q();
	md->rtcp_xr.enabled = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_enabled", 1);
	if (md->rtcp_xr.enabled) {
		const char *rcvr_rtt_mode = linphone_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_rcvr_rtt_mode", "all");
		if (strcasecmp(rcvr_rtt_mode, "all") == 0)
			md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttAll;
		else if (strcasecmp(rcvr_rtt_mode, "sender") == 0)
			md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttSender;
		else
			md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttNone;
		if (md->rtcp_xr.rcvr_rtt_mode != OrtpRtcpXrRcvrRttNone)
			md->rtcp_xr.rcvr_rtt_max_size = linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_rcvr_rtt_max_size", 10000);
		md->rtcp_xr.stat_summary_enabled = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_stat_summary_enabled", 1);
		if (md->rtcp_xr.stat_summary_enabled)
			md->rtcp_xr.stat_summary_flags = OrtpRtcpXrStatSummaryLoss | OrtpRtcpXrStatSummaryDup | OrtpRtcpXrStatSummaryJitt | OrtpRtcpXrStatSummaryTTL;
		md->rtcp_xr.voip_metrics_enabled = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_voip_metrics_enabled", 1);
	}
	for (auto & stream : md->streams) {
		stream.setupRtcpXr(md->rtcp_xr);
	}
}


void MediaSessionPrivate::setupImEncryptionEngineParameters (std::shared_ptr<SalMediaDescription> & md) {
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

void MediaSessionPrivate::setupEncryptionKeys (std::shared_ptr<SalMediaDescription> & md) {
	L_Q();
	std::shared_ptr<SalMediaDescription> & oldMd = localDesc;
	bool keepSrtpKeys = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "keep_srtp_keys", 1);
	for (size_t i = 0; i < md->streams.size(); i++) {
		if (md->streams[i].hasSrtp()) {
			auto & crypto = md->streams[i].cfgs[md->streams[i].getChosenConfigurationIndex()].crypto;
			if (keepSrtpKeys && oldMd && (i < oldMd->streams.size()) && oldMd->streams[i].enabled() && oldMd->streams[i].hasSrtp()) {
				lInfo() << "Keeping same crypto keys";
				crypto = oldMd->streams[i].getChosenConfiguration().crypto;
			} else {
				const MSCryptoSuite *suites = linphone_core_get_srtp_crypto_suites(q->getCore()->getCCore());
				for (size_t j = 0; (suites != nullptr) && (suites[j] != MS_CRYPTO_SUITE_INVALID); j++) {
					SalSrtpCryptoAlgo newCrypto;
					setupEncryptionKey(newCrypto, suites[j], static_cast<unsigned int>(j) + 1);
					crypto.emplace(std::next(crypto.begin(),static_cast<long>(j)),newCrypto);
				}
			}
		}
	}
}

void MediaSessionPrivate::transferAlreadyAssignedPayloadTypes (std::shared_ptr<SalMediaDescription> & oldMd, std::shared_ptr<SalMediaDescription> & md) {
	for (size_t i = 0; i < md->streams.size(); i++) {
		if (i < oldMd->streams.size()) {
			md->streams[i].already_assigned_payloads = std::move(oldMd->streams[i].already_assigned_payloads);
			oldMd->streams[i].already_assigned_payloads.clear();
		} else {
			md->streams[i].already_assigned_payloads.clear();
		}
	}
}

void MediaSessionPrivate::updateLocalMediaDescriptionFromIce (bool localIsOfferer) {
	OfferAnswerContext ctx;

	ctx.localMediaDescription = localDesc;
	ctx.remoteMediaDescription = op ? op->getRemoteMediaDescription() : nullptr;
	ctx.localIsOfferer = localIsOfferer;
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


void MediaSessionPrivate::queueIceCompletionTask(const std::function<void()> &lambda){
	iceDeferedTasks.push(lambda);
}

void MediaSessionPrivate::runIceCompletionTasks(){
	while(!iceDeferedTasks.empty()){
		iceDeferedTasks.front()();
		iceDeferedTasks.pop();
	}
}

/*
 * IceServiceListener implementation
 */
void MediaSessionPrivate::onGatheringFinished(IceService &service){
	runIceCompletionTasks();
}

void MediaSessionPrivate::onIceCompleted(IceService &service){
	L_Q();
	/* The ICE session has succeeded, so perform a call update */
	if (!getStreamsGroup().getIceService().hasCompletedCheckList()) return;
	if (getStreamsGroup().getIceService().isControlling() && getParams()->getPrivate()->getUpdateCallWhenIceCompleted()) {
		switch (state){
			case CallSession::State::StreamsRunning:
			case CallSession::State::Paused:
			case CallSession::State::PausedByRemote:
			{
				MediaSessionParams newParams(*getParams());
				newParams.getPrivate()->setInternalCallUpdate(true);
				q->update(&newParams);
			}
			break;
			default:
				lWarning() << "Cannot send reINVITE for ICE during state " << state;
			break;
		}
	}
	startDtlsOnAllStreams();
}

void MediaSessionPrivate::onLosingPairsCompleted(IceService &service){
	if (state == CallSession::State::UpdatedByRemote) {
		if (incomingIceReinvitePending){
			lInfo() << "Finished adding losing pairs, ICE re-INVITE can be answered.";
			startAcceptUpdate(prevState, Utils::toString(prevState));
			incomingIceReinvitePending = false;
		}
	}
}

void MediaSessionPrivate::onIceRestartNeeded(IceService & service){
	L_Q();
	getStreamsGroup().getIceService().restartSession(IR_Controlling);
	MediaSessionParams newParams(*getParams());
	q->update(&newParams);
}

void MediaSessionPrivate::tryEarlyMediaForking (std::shared_ptr<SalMediaDescription> & md) {
	OfferAnswerContext ctx;
	ctx.localMediaDescription = localDesc;
	ctx.remoteMediaDescription = md;
	ctx.resultMediaDescription = resultDesc;
	lInfo() << "Early media response received from another branch, checking if media can be forked to this new destination";
	getStreamsGroup().tryEarlyMediaForking(ctx);
}

void MediaSessionPrivate::updateStreamFrozenPayloads (SalStreamDescription &resultDesc, SalStreamDescription &localStreamDesc) {
	L_Q();
	for (const auto & pt : resultDesc.getPayloads()) {
		if (PayloadTypeHandler::isPayloadTypeNumberAvailable(localStreamDesc.already_assigned_payloads, payload_type_get_number(pt), nullptr)) {
			/* New codec, needs to be added to the list */
			localStreamDesc.already_assigned_payloads.push_back(payload_type_clone(pt));
			lInfo() << "CallSession[" << q << "] : payload type " << payload_type_get_number(pt) << " " << pt->mime_type << "/" << pt->clock_rate
				<< " fmtp=" << L_C_TO_STRING(pt->recv_fmtp) << " added to frozen list";
		}
	}
}

void MediaSessionPrivate::updateFrozenPayloads (std::shared_ptr<SalMediaDescription> & result) {
	for (size_t i = 0; i < result->streams.size(); i++) {
		updateStreamFrozenPayloads(result->streams[i], localDesc->streams[i]);
	}
}

void MediaSessionPrivate::updateStreams (std::shared_ptr<SalMediaDescription> & newMd, CallSession::State targetState) {
	L_Q();

	if (state == CallSession::State::Connected || state == CallSession::State::Resuming ||
		(state == CallSession::State::IncomingEarlyMedia && !linphone_core_get_ring_during_incoming_early_media(q->getCore()->getCCore()))) {
		q->getCore()->getPrivate()->getToneManager()->goToCall(q->getSharedFromThis());
	}

	if (!newMd) {
		lError() << "updateStreams() called with null media description";
		return;
	}

	updateBiggestDesc(localDesc);
	std::shared_ptr<SalMediaDescription> oldMd = resultDesc;
	resultDesc = newMd;

	OfferAnswerContext ctx;
	ctx.localMediaDescription = localDesc;
	ctx.remoteMediaDescription = op->getRemoteMediaDescription();
	ctx.resultMediaDescription = resultDesc;
	ctx.localIsOfferer = localIsOfferer;
	getStreamsGroup().render(ctx, targetState);

	bool isInLocalConference = getParams()->getPrivate()->getInConference();
	if ((state == CallSession::State::Pausing) && pausedByApp && (q->getCore()->getCallCount() == 1) && !isInLocalConference) {
		q->getCore()->getPrivate()->getToneManager()->startNamedTone(q->getSharedFromThis(), LinphoneToneCallOnHold);
	}

	// Capability negotiation may have changed the encryption of the streams hence call params must be updated
	if (q->isCapabilityNegotiationEnabled()) {
		const auto & enc = getEncryptionFromMediaDescription(newMd);
		lInfo() << "Changing call media encryption to " << linphone_media_encryption_to_string(enc) << " after capability negotiation are completed";
		getCurrentParams()->setMediaEncryption(enc);
	}

	updateFrozenPayloads(newMd);
	upBandwidth = linphone_core_get_upload_bandwidth(q->getCore()->getCCore());
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
	stopStreams();
	CallSessionPrivate::abort(errorMsg);
	q->getCore()->getPrivate()->getToneManager()->stop(q->getSharedFromThis());
	q->getCore()->getPrivate()->getToneManager()->startNamedTone(q->getSharedFromThis(), LinphoneToneCallEnd);
}

void MediaSessionPrivate::handleIncomingReceivedStateInIncomingNotification () {
	L_Q();

	/* Try to be best-effort in giving real local or routable contact address for 100Rel case */
	setContactOp();
	bool proposeEarlyMedia = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "incoming_calls_early_media", false);
	if (proposeEarlyMedia)
		q->acceptEarlyMedia();
	else
		op->notifyRinging(false);

	acceptOrTerminateReplacedSessionInIncomingNotification();
}


bool MediaSessionPrivate::canSoundResourcesBeFreed() const {
	return ((state == CallSession::State::StreamsRunning) || (state == CallSession::State::PausedByRemote));
}

LinphoneStatus MediaSessionPrivate::pause () {
	L_Q();
	if (state == CallSession::State::Paused) {
		lWarning() << "Media session (local addres " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") is in state " << Utils::toString(state) << " is already paused";
		return 0;
	} else if (state == CallSession::State::Pausing) {
		lWarning() << "Media session (local addres " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") is in state " << Utils::toString(state) << " is already in the process of being paused";
		return 0;
	} else if (!canSoundResourcesBeFreed()) {
		lWarning() << "Media session (local addres " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") is in state " << Utils::toString(state) << " hence it cannot be paused";
		return -1;
	}

	string subject;
	if (resultDesc->hasDir(SalStreamSendRecv))
		subject = "Call on hold";
	else if (resultDesc->hasDir(SalStreamRecvOnly)
				 || (resultDesc->hasDir(SalStreamInactive) && state == CallSession::State::PausedByRemote))	// Stream is inactive from Remote
		subject = "Call on hold for me too";
	else {
		lError() << "No reason to pause this call, it is already paused or inactive";
		return -1;
	}
	broken = false;
	setState(CallSession::State::Pausing, "Pausing call");
	makeLocalMediaDescription(true, q->isCapabilityNegotiationEnabled());
	op->update(subject.c_str(), false);

	shared_ptr<Call> currentCall = q->getCore()->getCurrentCall();
	// Reset current session if we are pausing the current call
	if (listener && (!currentCall || (currentCall->getActiveSession() == q->getSharedFromThis())))
		listener->onResetCurrentSession(q->getSharedFromThis());

	stopStreams();
	pausedByApp = false;
	return 0;
}

int MediaSessionPrivate::restartInvite () {
	L_Q();
	stopStreams();
	getStreamsGroup().clearStreams();
	makeLocalMediaDescription(true, q->isCapabilityNegotiationEnabled());
	return CallSessionPrivate::restartInvite();
}

void MediaSessionPrivate::setTerminated () {
	freeResources();
	CallSessionPrivate::setTerminated();
}

LinphoneStatus MediaSessionPrivate::startAcceptUpdate (CallSession::State nextState, const string &stateInfo) {

	op->accept();
	std::shared_ptr<SalMediaDescription> & md = op->getFinalMediaDescription();
	if (md && !md->isEmpty())
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
	if (q->isRecording()) {
		lInfo() << "Media session is being terminated, stop recording";
		q->stopRecording();
	}
	stopStreams();
	CallSessionPrivate::terminate();
	q->getCore()->getPrivate()->getToneManager()->stop(q->getSharedFromThis());
	q->getCore()->getPrivate()->getToneManager()->startNamedTone(q->getSharedFromThis(), LinphoneToneCallEnd);
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
	 * For both DTLS and ZRTP it is though necessary.
	 * But for all others the current_params->media_encryption state should reflect both what is agreed by the offer/answer
	 * mechanism and encryption status from media which is much stronger than only result of offer/answer.
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
	const std::shared_ptr<SalMediaDescription> & md = resultDesc;
	
	if (md) {
		getCurrentParams()->enableAvpf(hasAvpf(md));
		if (getCurrentParams()->avpfEnabled())
			getCurrentParams()->setAvpfRrInterval(getAvpfRrInterval());
		else
			getCurrentParams()->setAvpfRrInterval(0);
		if (mainAudioStreamIndex != -1) {
			if (mainAudioStreamIndex < (int)md->streams.size()){
				const SalStreamDescription &sd = md->streams[static_cast<size_t>(mainAudioStreamIndex)];
				getCurrentParams()->setAudioDirection(MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd.getDirection()));
				if (getCurrentParams()->getAudioDirection() != LinphoneMediaDirectionInactive) {
					const std::string & rtpAddr = (sd.getRtpAddress().empty() == false) ? sd.getRtpAddress() : md->addr;
					getCurrentParams()->enableAudioMulticast(!!ms_is_multicast(rtpAddr.c_str()));
				} else
					getCurrentParams()->enableAudioMulticast(false);
				getCurrentParams()->enableAudio(sd.enabled());
			} else {
				getCurrentParams()->setAudioDirection(LinphoneMediaDirectionInactive);
				getCurrentParams()->enableAudioMulticast(false);
				getCurrentParams()->enableAudio(false);
			}
		}
		if (mainVideoStreamIndex != -1) {
			if (mainVideoStreamIndex < (int)md->streams.size()) {
				const SalStreamDescription &sd = md->streams[static_cast<size_t>(mainVideoStreamIndex)];
				getCurrentParams()->getPrivate()->enableImplicitRtcpFb(sd.hasImplicitAvpf());
				getCurrentParams()->setVideoDirection(MediaSessionParamsPrivate::salStreamDirToMediaDirection(sd.getDirection()));
				if (getCurrentParams()->getVideoDirection() != LinphoneMediaDirectionInactive) {
					const std::string & rtpAddr = (sd.getRtpAddress().empty() == false) ? sd.getRtpAddress() : md->addr;
					getCurrentParams()->enableVideoMulticast(!!ms_is_multicast(rtpAddr.c_str()));
				} else
					getCurrentParams()->enableVideoMulticast(false);
				getCurrentParams()->enableVideo(sd.enabled());
			} else {
				getCurrentParams()->getPrivate()->enableImplicitRtcpFb(false);
				getCurrentParams()->setVideoDirection(LinphoneMediaDirectionInactive);
				getCurrentParams()->enableVideoMulticast(false);
				getCurrentParams()->enableVideo(false);
			}
		}
		if ((mainTextStreamIndex != -1) && (mainTextStreamIndex < (int)md->streams.size())){
		if ((mainTextStreamIndex != -1) && (mainTextStreamIndex < (int)md->streams.size())){
				const SalStreamDescription &sd = md->streams[static_cast<size_t>(mainTextStreamIndex)];
				// Direction and multicast are not supported for real-time text.
				getCurrentParams()->enableRealtimeText(sd.enabled());
			} else {
				getCurrentParams()->enableRealtimeText(false);
			}
		}
	}
	getCurrentParams()->getPrivate()->setUpdateCallWhenIceCompleted(getParams()->getPrivate()->getUpdateCallWhenIceCompleted());
}

// -----------------------------------------------------------------------------


void MediaSessionPrivate::startAccept(){
	L_Q();

	shared_ptr<Call> currentCall = q->getCore()->getCurrentCall();
	// If the core in a call, request to empty sound resources only if this call is not the call the core is currently in
	bool isThisNotCurrentMediaSession = currentCall && (currentCall->getActiveSession() != q->getSharedFromThis());

	bool isCoreInLocalConference = linphone_core_is_in_conference(q->getCore()->getCCore());
	LinphoneConference * callConference = nullptr;
	if (listener) {
		callConference = listener->getCallSessionConference(q->getSharedFromThis());
	}
	LinphoneConference * coreConference = linphone_core_get_conference(q->getCore()->getCCore());
	// If the core in a conference, request to empty sound resources only if the call is in a difference conference or the call is not part of a conference
	bool isThisNotCurrentConference = isCoreInLocalConference && (!callConference || (callConference != coreConference));

	// Try to preempt sound resources if the core is in a call or conference that are not the current ones
	if (isThisNotCurrentConference || isThisNotCurrentMediaSession) {
		if ((linphone_core_get_media_resource_mode(q->getCore()->getCCore()) == LinphoneExclusiveMediaResources) && linphone_core_preempt_sound_resources(q->getCore()->getCCore()) != 0) {
			lInfo() << "Delaying call to " << __func__ << " for media session (local addres " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") in state " << Utils::toString(state) << " because sound resources cannot be preempted";
			pendingActions.push([this] {this->startAccept();});
			return;
		}
	}

	/* Give a chance a set card prefered sampling frequency */
	if (localDesc->streams[0].getMaxRate() > 0) {
		lInfo() << "Configuring prefered card sampling rate to [" << localDesc->streams[0].getMaxRate() << "]";
		if (q->getCore()->getCCore()->sound_conf.play_sndcard)
			ms_snd_card_set_preferred_sample_rate(q->getCore()->getCCore()->sound_conf.play_sndcard, localDesc->streams[0].getMaxRate());
		if (q->getCore()->getCCore()->sound_conf.capt_sndcard)
			ms_snd_card_set_preferred_sample_rate(q->getCore()->getCCore()->sound_conf.capt_sndcard, localDesc->streams[0].getMaxRate());
	}

	/* We shall already have all the information to prepare the zrtp/lime mutual authentication */
	performMutualAuthentication();

	CallSessionPrivate::accept(nullptr);
	if (!getParams()->getPrivate()->getInConference() && listener){
		listener->onSetCurrentSession(q->getSharedFromThis());
	}

	std::shared_ptr<SalMediaDescription> & newMd = op->getFinalMediaDescription();
	if (newMd) {
		updateStreams(newMd, CallSession::State::StreamsRunning);
		setState(CallSession::State::StreamsRunning, "Connected (streams running)");
	} else
		expectMediaInAck = true;
}

void MediaSessionPrivate::accept (const MediaSessionParams *msp, bool wasRinging) {
	L_Q();
	if (msp) {
		setParams(new MediaSessionParams(*msp));
	}
	if (msp || localDesc == nullptr) makeLocalMediaDescription((op->getRemoteMediaDescription() ? false : true), q->isCapabilityNegotiationEnabled());

	// already accepting
	if (state == CallSession::State::IncomingReceived && params && localDesc == nullptr) makeLocalMediaDescription(false, q->isCapabilityNegotiationEnabled());

	updateRemoteSessionIdAndVer();

	auto acceptCompletionTask = [this](){
		updateLocalMediaDescriptionFromIce(op->getRemoteMediaDescription() == nullptr);
		startAccept();
	};
	if (getStreamsGroup().prepare()){
		queueIceCompletionTask(acceptCompletionTask);
		return; /* Deferred until completion of ICE gathering */
	}
	acceptCompletionTask();
}

LinphoneStatus MediaSessionPrivate::acceptUpdate (const CallSessionParams *csp, CallSession::State nextState, const string &stateInfo) {
	L_Q();
	const std::shared_ptr<SalMediaDescription> & desc = op->getRemoteMediaDescription();

	bool keepSdpVersion = !!linphone_config_get_int(
		linphone_core_get_config(q->getCore()->getCCore()),
		"sip",
		"keep_sdp_version",
		(op->getSal()->getSessionTimersExpire() > 0)
	);

	if (keepSdpVersion && (desc->session_id == remoteSessionId) && (desc->session_ver == remoteSessionVer)) {
		/* Remote has sent an INVITE with the same SDP as before, so send a 200 OK with the same SDP as before. */
		lInfo() << "SDP version has not changed, send same SDP as before or sessionTimersExpire=" << op->getSal()->getSessionTimersExpire();
		op->accept();
		setState(nextState, stateInfo);
		return 0;
	}

	if (csp) {
		setParams(new MediaSessionParams(*static_cast<const MediaSessionParams *>(csp)));
	} else {
		if (!op->isOfferer()) {
			/* Reset call params for multicast because this param is only relevant when offering */
			getParams()->enableAudioMulticast(false);
			getParams()->enableVideoMulticast(false);
		}
	}
	makeLocalMediaDescription((op->getRemoteMediaDescription() ? false : true), false);
	if (getParams()->videoEnabled() && !linphone_core_video_enabled(q->getCore()->getCCore())) {
		lWarning() << "Requested video but video support is globally disabled. Refusing video";
		getParams()->enableVideo(false);
	}
	updateRemoteSessionIdAndVer();
	makeLocalMediaDescription((op->getRemoteMediaDescription() ? false : true), false);

	auto acceptCompletionTask = [this, nextState, stateInfo](){
		updateLocalMediaDescriptionFromIce(op->getRemoteMediaDescription() == nullptr);
		startAcceptUpdate(nextState, stateInfo);
	};
	
	if (getStreamsGroup().prepare()){
		lInfo() << "Acceptance of incoming reINVITE is deferred to ICE gathering completion.";
		queueIceCompletionTask(acceptCompletionTask);
		return 0; /* Deferred until completion of ICE gathering */
	}
	acceptCompletionTask();
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

MediaSession::MediaSession (const shared_ptr<Core> &core, std::shared_ptr<Participant> me, const CallSessionParams *params, CallSessionListener *listener)
	: CallSession(*new MediaSessionPrivate, core) {
	L_D();
	d->me = me;
	d->listener = listener;

	if (params) {
		d->setParams(new MediaSessionParams(*(reinterpret_cast<const MediaSessionParams *>(params))));
	} else {
		d->setParams(new MediaSessionParams());
	}
	d->setCurrentParams(new MediaSessionParams());
	d->streamsGroup = makeUnique<StreamsGroup>(*this);
	d->streamsGroup->getIceService().setListener(d);

	lInfo() << "New MediaSession [" << this << "] initialized (liblinphone version: " << linphone_core_get_version() << ")";
}

MediaSession::~MediaSession () {
	L_D();
	cancelDtmfs();
	d->freeResources();
	if (d->natPolicy)
		linphone_nat_policy_unref(d->natPolicy);
	if (d->currentOutputAudioDevice)
		d->currentOutputAudioDevice->unref();
	if (d->currentInputAudioDevice)
		d->currentInputAudioDevice->unref();
}

// -----------------------------------------------------------------------------

void MediaSession::acceptDefault(){
	accept();
}

LinphoneStatus MediaSession::accept (const MediaSessionParams *msp) {
	L_D();
	if (!isOpConfigured()) {
		lInfo() << "CallSession accepting";
		if (msp)
			d->setParams(new MediaSessionParams(*msp));
		CallSession::accepting();
		return 0;
	}

	LinphoneStatus result = d->checkForAcceptation();
	if (result < 0) return result;

	bool wasRinging = false;
	if (d->listener)
		wasRinging = d->listener->onCallSessionAccepted(getSharedFromThis());

	d->accept(msp, wasRinging);
	lInfo() << "MediaSession accepted";
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
		d->makeLocalMediaDescription(false, isCapabilityNegotiationEnabled());
		d->op->setSentCustomHeaders(d->getParams()->getPrivate()->getCustomHeaders());
	}
	d->op->notifyRinging(true);
	d->setState(CallSession::State::IncomingEarlyMedia, "Incoming call early media");
	std::shared_ptr<SalMediaDescription> & md = d->op->getFinalMediaDescription();
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
			d->makeLocalMediaDescription(true, isCapabilityNegotiationEnabled());
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
		d->makeLocalMediaDescription((op->getRemoteMediaDescription() ? false : true), isCapabilityNegotiationEnabled());
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
			if (d->deferIncomingNotification) {
				auto incomingNotificationTask = [d](){
					/* There is risk that the call can be terminated before this task is executed, for example if offer/answer fails.*/
					if (d->state != State::Idle) return;
					d->deferIncomingNotification = false;
					d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
					d->startIncomingNotification();
				};
				d->queueIceCompletionTask(incomingNotificationTask);
			}else{
				d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
			}
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
				d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
			}else{
				d->queueIceCompletionTask([this]() {
					L_D();
					d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
					startInvite(nullptr, "");
				});
			}
			defer |= ice_needs_defer;
		}
	}
	return defer;
}

void MediaSession::iterate (time_t currentRealTime, bool oneSecondElapsed) {
	CallSession::iterate(currentRealTime, oneSecondElapsed);
}

LinphoneStatus MediaSession::pauseFromConference () {
	L_D();
	char * contactAddressStr = nullptr;
	if (d->destProxy && linphone_proxy_config_get_op(d->destProxy)) {
		contactAddressStr = sal_address_as_string(linphone_proxy_config_get_op(d->destProxy)->getContactAddress());
	} else {
		contactAddressStr = sal_address_as_string(d->op->getContactAddress());
	}
	Address contactAddress(contactAddressStr);
	ms_free(contactAddressStr);
	updateContactAddress (contactAddress);
	d->op->setContactAddress(contactAddress.getInternalAddress());

	return pause();
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
		lWarning() << "we cannot resume a call that has not been established and paused before. Current state: " << Utils::toString(d->state);
		return -1;
	}
	if (!d->getParams()->getPrivate()->getInConference()) {
		if (linphone_core_sound_resources_locked(getCore()->getCCore())) {
			lWarning() << "Cannot resume MediaSession " << this << " because another call is locking the sound resources";
			return -1;
		}
		linphone_core_preempt_sound_resources(getCore()->getCCore());
/*		if (linphone_core_preempt_sound_resources(getCore()->getCCore()) != 0) {
			lInfo() << "Delaying call to " << __func__ << " because sound resources cannot be preempted";
			d->pendingActions.push([this] {this->resume();});
			return -1;
		}*/
		lInfo() << "Resuming MediaSession " << this;
	}
	d->automaticallyPaused = false;
	d->broken = false;
	/* Stop playing music immediately. If remote side is a conference it
	 * prevents the participants to hear it while the 200OK comes back. */
	Stream *as = d->getStreamsGroup().lookupMainStream(SalAudio);
	if (as) as->stop();
	d->setState(CallSession::State::Resuming, "Resuming");
	d->makeLocalMediaDescription(true, isCapabilityNegotiationEnabled());
	d->localDesc->setDir(SalStreamSendRecv);

	if (getCore()->getCCore()->sip_conf.sdp_200_ack)
		d->op->setLocalMediaDescription(nullptr);
	string subject = "Call resuming";
	if (d->getParams()->getPrivate()->getInConference() && !getCurrentParams()->getPrivate()->getInConference()) {
		subject = "Conference";
	}

	char * contactAddressStr = nullptr;
	if (d->destProxy && linphone_proxy_config_get_op(d->destProxy)) {
		contactAddressStr = sal_address_as_string(linphone_proxy_config_get_op(d->destProxy)->getContactAddress());
	} else {
		contactAddressStr = sal_address_as_string(d->op->getContactAddress());
	}
	Address contactAddress(contactAddressStr);
	ms_free(contactAddressStr);
	updateContactAddress(contactAddress);
	d->op->setContactAddress(contactAddress.getInternalAddress());

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
		int delayMs = linphone_config_get_int(linphone_core_get_config(getCore()->getCCore()), "net", "dtmf_delay_ms", 200);
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

	if ((curParams->avpfEnabled() || curParams->getPrivate()->implicitRtcpFbEnabled())) { // || sal_media_description_has_implicit_avpf((const std::shared_ptr<SalMediaDescription> )call->resultdesc)
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

	std::shared_ptr<SalMediaDescription> & md = d->op->getFinalMediaDescription();
	if (md) {
		if (md->isEmpty() || getCore()->incompatibleSecurity(md)) {
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
		if (d->localDesc && (d->localDesc->streams.size() > 0) && (d->localDesc->streams[0].getMaxRate() > 0))
			ms_snd_card_set_preferred_sample_rate(getCore()->getCCore()->sound_conf.play_sndcard, d->localDesc->streams[0].getMaxRate());
		d->getStreamsGroup().prepare();
	}

	if (d->localDesc) {
		srand((unsigned int)time(NULL));
		for (auto & stream : d->localDesc->streams) {
			// In case of multicasting, choose a random port to send with the invite
			if (ms_is_multicast(L_STRING_TO_C(stream.rtp_addr))){
				pair<int, int> portRange = Stream::getPortRange(getCore()->getCCore(), stream.type);
				if (portRange.first <= 0) {
					portRange.first = 1024;
					lInfo() << "Setting minimum value of port range to " << portRange.first;
				}
				if (portRange.second <= 0) {
					// 2^16 - 1
					portRange.second = 65535;
					lInfo() << "Setting maximum value of port range to " << portRange.second;
				}
				if (portRange.second < portRange.first) {
					lError() << "Invalid port range provided for stream type " << Utils::toString(stream.type) << ": min=" << portRange.first << " max=" << portRange.second;
					continue;
				}
				int rtp_port = (rand() % abs(portRange.second - portRange.first)) + portRange.first;
				stream.rtp_port = rtp_port;
				stream.rtcp_port = stream.rtp_port + 1;
			}
		}
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
	if (i != nullptr) i->startRecording();
}

void MediaSession::stopRecording () {
	L_D();
	AudioControlInterface * i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i != nullptr) i->stopRecording();
}

bool MediaSession::isRecording () {
	L_D();
	AudioControlInterface * i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i != nullptr) return i->isRecording();
	return false;
}

void MediaSession::terminateBecauseOfLostMedia () {
	L_D();
	d->nonOpError = true;
	linphone_error_info_set(d->ei, nullptr, LinphoneReasonIOError, 503, "Media lost", nullptr);
	terminate();
}

LinphoneStatus MediaSession::updateFromConference (const MediaSessionParams *msp, const string &subject) {
	L_D();
	char * contactAddressStr = nullptr;
	if (d->destProxy && linphone_proxy_config_get_op(d->destProxy)) {
		contactAddressStr = sal_address_as_string(linphone_proxy_config_get_op(d->destProxy)->getContactAddress());
	} else {
		contactAddressStr = sal_address_as_string(d->op->getContactAddress());
	}
	Address contactAddress(contactAddressStr);
	ms_free(contactAddressStr);
	updateContactAddress (contactAddress);
	d->op->setContactAddress(contactAddress.getInternalAddress());

	return update(msp, subject);
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
			d->makeLocalMediaDescription(true, false);

		auto updateCompletionTask = [this, subject, initialState]() -> LinphoneStatus{
			L_D();
			d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
			LinphoneStatus res = d->startUpdate(subject);
			if (res && (d->state != initialState)) {
				/* Restore initial state */
				d->setState(initialState, "Restore initial state");
			}
			return res;
		};
		
		if (d->getStreamsGroup().prepare()) {
			lInfo() << "Defer CallSession update to gather ICE candidates";
			d->queueIceCompletionTask(updateCompletionTask);
			return 0;
		}
		result = updateCompletionTask();
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
			// Done directly by linphone_core_set_video_device().
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

const string &MediaSession::getAuthenticationToken () const {
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
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (iface) {
		return iface->getNativeWindowId();
	}
	return nullptr;
}

void MediaSession::setNativeVideoWindowId (void *id) {
	L_D();
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (iface) {
		iface->setNativeWindowId(id);
	}
}

void MediaSession::setNativePreviewWindowId(void *id){
	L_D();
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (iface) {
		iface->setNativePreviewWindowId(id);
	}
}

void * MediaSession::getNativePreviewVideoWindowId () const{
	L_D();
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (iface) {
		return iface->getNativePreviewWindowId();
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
		const std::shared_ptr<SalMediaDescription> & md = d->op->getRemoteMediaDescription();
		MediaSessionParams * params = nullptr;
		if (md) {
			params = new MediaSessionParams();

			if (d->mainAudioStreamIndex != -1 && d->mainAudioStreamIndex < static_cast<int>(md->streams.size())){
				size_t audioStreamIndex = static_cast<size_t>(d->mainAudioStreamIndex);
				const SalStreamDescription & sd = md->streams[audioStreamIndex];
				params->enableAudio(sd.enabled());
				params->setMediaEncryption(sd.hasSrtp() ? LinphoneMediaEncryptionSRTP : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeAudio, md->streams[audioStreamIndex].getCustomSdpAttributes());
			}else params->enableAudio(false);

			if (d->mainVideoStreamIndex != -1 && d->mainVideoStreamIndex < static_cast<int>(md->streams.size())){
				size_t videoStreamIndex = static_cast<size_t>(d->mainVideoStreamIndex);
				const SalStreamDescription & sd = md->streams[videoStreamIndex];
				params->enableVideo(sd.enabled());
				params->setMediaEncryption(sd.hasSrtp() ? LinphoneMediaEncryptionSRTP : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeVideo, md->streams[videoStreamIndex].getCustomSdpAttributes());
			}else params->enableVideo(false);

			if (d->mainTextStreamIndex != -1 && d->mainTextStreamIndex < static_cast<int>(md->streams.size())){
				size_t textStreamIndex = static_cast<size_t>(d->mainTextStreamIndex);
				const SalStreamDescription & sd = md->streams[textStreamIndex];
				params->enableRealtimeText(sd.enabled());
				params->setMediaEncryption(sd.hasSrtp() ? LinphoneMediaEncryptionSRTP : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeText, md->streams[textStreamIndex].getCustomSdpAttributes());
			}else params->enableRealtimeText(false);

			if (!params->videoEnabled()) {
				if ((md->bandwidth > 0) && (md->bandwidth <= linphone_core_get_edge_bw(getCore()->getCCore())))
					params->enableLowBandwidth(true);
			}
			if (md->name[0] != '\0')
				params->setSessionName(md->name);
			params->getPrivate()->setCustomSdpAttributes(md->custom_sdp_attributes);
			params->enableRtpBundle(!md->bundles.empty());
		}
		const SalCustomHeader *ch = d->op->getRecvCustomHeaders();
		if (ch) {
			if (!params) params = new MediaSessionParams();
			params->getPrivate()->setCustomHeaders(ch);
		}
		const list<Content> &additionnalContents = d->op->getAdditionalRemoteBodies();
		for (auto& content : additionnalContents){
			if (!params) params = new MediaSessionParams();
			params->addCustomContent(content);
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
	L_D();
	for(auto &stream : d->getStreamsGroup().getStreams()){
		LinphoneCallStats *stats = stream->getStats();
		if (stats && linphone_call_stats_get_ice_state(stats) == LinphoneIceStateInProgress){
			return true;
		}
	}
	return false;
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
		case CallSession::State::PushIncomingReceived:
			d->setParams(msp ? new MediaSessionParams(*msp) : nullptr);
			// Update the local media description.
			d->makeLocalMediaDescription((d->state == CallSession::State::OutgoingInit ?
				!getCore()->getCCore()->sip_conf.sdp_200_ack : false), isCapabilityNegotiationEnabled());
		break;
		default:
			lError() << "MediaSession::setParams(): Invalid state " << Utils::toString(d->state);
		break;
	}
}

StreamsGroup & MediaSession::getStreamsGroup()const{
	L_D();
	return d->getStreamsGroup();
}

void MediaSession::setInputAudioDevice(AudioDevice *audioDevice) {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	d->setCurrentInputAudioDevice(audioDevice);
	if (i) i->setInputDevice(audioDevice);
}

void MediaSession::setOutputAudioDevice(AudioDevice *audioDevice) {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	d->setCurrentOutputAudioDevice(audioDevice);
	if (i) i->setOutputDevice(audioDevice);
}

AudioDevice* MediaSession::getInputAudioDevice() const {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) return i->getInputDevice();
	return nullptr;
}

AudioDevice* MediaSession::getOutputAudioDevice() const {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) return i->getOutputDevice();
	return nullptr;
}

LINPHONE_END_NAMESPACE
