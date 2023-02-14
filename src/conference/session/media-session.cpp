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

//#include <iomanip>
//#include <math.h>
#include <algorithm>

#include "account/account.h"
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

#include "conference_private.h"
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
	MediaSessionPrivate *msp = static_cast<MediaSessionPrivate *>(userData);
	msp->stunAuthRequestedCb(realm, nonce, username, password, ha1);
}

LinphoneMediaEncryption MediaSessionPrivate::getEncryptionFromMediaDescription(const std::shared_ptr<SalMediaDescription> & md) const {
	LinphoneMediaEncryption enc = LinphoneMediaEncryptionNone;
	if (md->hasSrtp()) {
		enc = LinphoneMediaEncryptionSRTP;
	} else if (md->hasDtls()) {
		enc = LinphoneMediaEncryptionDTLS;
	} else if (md->hasZrtp()) {
		enc = LinphoneMediaEncryptionZRTP;
	} else {
		enc = LinphoneMediaEncryptionNone;
	}

	if (getParams()->getPrivate()->isMediaEncryptionSupported(enc)) {
		return enc;
	}

	// Do not change encryption if no stream is started or at least one is not encrypted or chosen encryption is not supported
	return getParams()->getMediaEncryption();
}

bool MediaSessionPrivate::isMediaEncryptionAccepted(const LinphoneMediaEncryption enc) const {
	return ((getParams()->getMediaEncryption() ==  enc) || (getParams()->getPrivate()->isMediaEncryptionSupported(enc)));
}

LinphoneMediaEncryption MediaSessionPrivate::getNegotiatedMediaEncryption() const {

	switch (state){
		case CallSession::State::Idle:
		case CallSession::State::IncomingReceived:
		case CallSession::State::OutgoingProgress:
		case CallSession::State::OutgoingRinging:
		case CallSession::State::OutgoingEarlyMedia:
		case CallSession::State::PushIncomingReceived:
			return getParams()->getMediaEncryption();
			break;
		case CallSession::State::Connected:
		{
			std::shared_ptr<SalMediaDescription> md = (op) ? op->getFinalMediaDescription() : nullptr;
			return md ? getEncryptionFromMediaDescription(md) : getParams()->getMediaEncryption();
		}
			break;
		default: 
			return negotiatedEncryption;
			break;
	}

	return LinphoneMediaEncryptionNone;
}

// -----------------------------------------------------------------------------

bool MediaSessionPrivate::tryEnterConference() {
	L_Q();

	if (getOp() && getOp()->getContactAddress()) {
		char * contactAddressStr = sal_address_as_string(getOp()->getContactAddress());
		Address contactAddress(contactAddressStr);
		ms_free(contactAddressStr);
		const auto & confId = getConferenceId();
		if (!confId.empty() && isInConference() && !contactAddress.hasParam("isfocus")) {
			q->updateContactAddress (contactAddress);
			ConferenceId localConferenceId = ConferenceId(contactAddress, contactAddress);
			shared_ptr<MediaConference::Conference> conference = q->getCore()->findAudioVideoConference(localConferenceId, false);
			// If the call conference ID is not an empty string but no conference is linked to the call means that it was added to the conference after the INVITE session was started but before its completition
			if (conference) {
				if (state == CallSession::State::Paused) {
					// Resume call as it was added to conference
					lInfo() << "Media session (local address " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") was added to conference " << conference->getConferenceAddress() << " while the call was being paused. Resuming the session.";
					q->resume();
				} else {
					// Send update to notify that the call enters conference
					MediaSessionParams *newParams = q->getMediaParams()->clone();
					const auto & videoEnabled = conference->getCurrentParams().videoEnabled();
					if (videoEnabled) {
						newParams->enableRtpBundle(true);
					}
					lInfo() << "Media session (local address " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") was added to conference " << conference->getConferenceAddress() << " while the call was establishing. Sending update to notify remote participant.";
					q->update(newParams, CallSession::UpdateMethod::Default, q->isCapabilityNegotiationEnabled());
					delete newParams;
				}
				return true;
			}
		}
	}
	return false;
}

bool MediaSessionPrivate::rejectMediaSession(const std::shared_ptr<SalMediaDescription> & remoteMd, const std::shared_ptr<SalMediaDescription> & finalMd) const {
	L_Q();
	if (remoteMd && remoteMd->isEmpty() && linphone_core_zero_rtp_port_for_stream_inactive_enabled(q->getCore()->getCCore())) {
		return false;
	}
	return (finalMd && (finalMd->isEmpty() || incompatibleSecurity(finalMd)));
}

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
				if (!localIsOfferer) {
					// We were waiting for an incoming offer. Now prepare the local media description according to remote offer.
					initializeParamsAccordingToIncomingCallParams();
					makeLocalMediaDescription(op->getRemoteMediaDescription() ? localIsOfferer : true, q->isCapabilityNegotiationEnabled(), false);
				}
				/*
				 * If ICE is enabled, we'll have to do the prepare() step, however since defering the sending of the ACK is complicated and
				 * confusing from a signaling standpoint, ICE we will skip the STUN gathering by not giving enough time
				 * for the gathering step. Only local candidates will be answered in the ACK.
				 */
				if (linphone_nat_policy_ice_enabled(natPolicy)){
					if (getStreamsGroup().prepare()){
						lWarning() << "Some gathering is needed for ICE, however since a defered sending of ACK is not supported"
							" the ICE gathering will only contain local candidates.";
					}
					getStreamsGroup().finishPrepare();
					updateLocalMediaDescriptionFromIce(localIsOfferer);
				}
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
	if (rejectMediaSession(rmd, md)) {
		lInfo() << "Rejecting media session";
		md = nullptr;
	}

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
				if (state == CallSession::State::Updating && prevState == CallSession::State::Paused) {
					// If previous state was paused and we are in Updating, there is no reason to set it to anything else than paused
					nextState = CallSession::State::Paused;
					nextStateMsg = "Call paused";
				} else {
					// The call always enters state PausedByRemote if all streams are rejected. This is done to support some clients who accept to stop the streams by setting the RTP port to 0
					if (!localDesc->hasDir(SalStreamInactive)
						&& (md->hasDir(SalStreamRecvOnly) || md->hasDir(SalStreamInactive) || md->isEmpty())) {
						nextState = CallSession::State::PausedByRemote;
						nextStateMsg = "Call paused by remote";
					} else {
						if (!getParams()->getPrivate()->getInConference() && listener)
							listener->onSetCurrentSession(q->getSharedFromThis());
						nextState = CallSession::State::StreamsRunning;
						nextStateMsg = "Streams running";
					}
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

		// A negative value of the counter may lead to unexpected behaviour, hence terminate here the execution in order to analyze what leads to this scenario
		if (nbProcessingUpdates < 0) {
			lFatal() << "The number of updates under processing for media session (local address " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") should be greater than or equal to 0. Currently it is " << nbProcessingUpdates;
		}

		if (nextState == CallSession::State::Idle) {
			lError() << "BUG: nextState is not set in accepted(), current state is " << Utils::toString(state);
		} else {
			updateRemoteSessionIdAndVer();
			//getIceAgent().updateIceStateInCallStats();
			updateStreams(md, nextState);
			fixCallParams(rmd, false);

			setState(nextState, nextStateMsg);
			bool capabilityNegotiationReInviteSent = false;
			const bool capabilityNegotiationReInviteEnabled = getParams()->getPrivate()->capabilityNegotiationReInviteEnabled();
			// If capability negotiation is enabled, a second invite must be sent if the selected configuration is not the actual one.
			// It normally occurs after moving to state StreamsRunning. However, if ICE negotiations are not completed, then this action will be carried out together with the ICE re-INVITE
			if (localDesc->supportCapabilityNegotiation() && (nextState == CallSession::State::StreamsRunning) && localIsOfferer && capabilityNegotiationReInviteEnabled) {
				// If no ICE session or checklist has completed, then send re-INVITE
				// The reINVITE to notify intermediaries that do not support capability negotiations (RFC5939) is sent in the following scenarions:
				// - no ICE session is found in th stream group
				// - an ICE sesson is found and its checklist has already completed
				// - an ICE sesson is found and ICE reINVITE is not sent upon completition if the checklist (This case is the default one for DTLS SRTP negotiation as it was observed that webRTC gateway did not correctly support SIP ICE reINVITEs)
				 if (!getStreamsGroup().getIceService().getSession() || (getStreamsGroup().getIceService().getSession() && (!isUpdateSentWhenIceCompleted() || getStreamsGroup().getIceService().hasCompletedCheckList()))) {
					// Compare the chosen final configuration with the actual configuration in the local decription
					const auto diff = md->compareToActualConfiguration(*localDesc);
					const bool potentialConfigurationChosen = (diff & SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED);
					if (potentialConfigurationChosen) {
						lInfo() << "Sending a reINVITE because the actual configuraton was not chosen in the capability negotiation procedure. Detected differences " << SalMediaDescription::printDifferences(diff);
						MediaSessionParams newParams(*getParams());
						newParams.getPrivate()->setInternalCallUpdate(true);
						q->update(&newParams, CallSession::UpdateMethod::Default, true);
						capabilityNegotiationReInviteSent = true;
					} else {
						lInfo() << "Using actual configuration after capability negotiation procedure, hence no need to send a reINVITE";
					}
				} else {
					lInfo() << "Capability negotiation and ICE are both enabled hence wait for the end of ICE checklist completion to send a reINVITE";
				}
			}

			// If the call was added to a conference after the last INVITE session was started, the reINVITE to enter conference must be sent only if capability negotiation reINVITE was not sent
			if (!capabilityNegotiationReInviteSent) {
				// Add to conference if it was added after last INVITE message sequence started
				// It occurs if the local participant calls the remote participant and the call is added to the conference when it is in state OutgoingInit, OutgoingProgress or OutgoingRinging
				q->getCore()->doLater( [this](){
					/* This has to be done outside of the accepted callback, because after the callback the SIP ACK is going to be sent.
					 * Despite it is not forbidden by RFC3261, it is preferable for the sake of clarity that the ACK for the current
					 * transaction is sent before the new INVITE that will be sent by tryEnterConference().
					 * Some implementations (eg FreeSwitch) reply "500 Overlapped request" otherwise ( which is 
					 * in fact a misunderstanding of RFC3261).
					 */
					tryEnterConference();
				});
			}
			bundleModeAccepted = q->getCurrentParams()->rtpBundleEnabled();
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

	getCurrentParams()->getPrivate()->setInConference(getParams()->getPrivate()->getInConference());

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
				setState(CallSession::State::UpdatedByRemote, "Updated by remote");
				break;
			default:
				break;
		}
		accepted();
	} else {
		bundleModeAccepted = q->getCurrentParams()->rtpBundleEnabled();
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

	if (CallSession::isEarlyState(state) && getStreamsGroup().isStarted()){
		stopStreams();
	}

	const SalErrorInfo *ei = op->getErrorInfo();
	switch (ei->reason) {
		case SalReasonUnsupportedContent: /* This is for compatibility: linphone sent 415 because of SDP offer answer failure */
		case SalReasonNotAcceptable:
			if ((linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "retry_invite_after_offeranswer_failure", 1)) && (
				(state == CallSession::State::OutgoingInit)
				|| (state == CallSession::State::OutgoingProgress)
				|| (state == CallSession::State::OutgoingRinging) /* Push notification case */ 
				|| (state == CallSession::State::OutgoingEarlyMedia))) {
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
								stream.cfgs[stream.getChosenConfigurationIndex()].crypto.clear();
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

	// Do not try to schedule resume of a call if it is ending
	if (referer && (state != CallSession::State::End) && (state != CallSession::State::Released)) {
		// Schedule automatic resume of the call. This must be done only after the notifications are completed due to dialog serialization of requests
		linphone_core_queue_task(q->getCore()->getCCore(),
			&MediaSessionPrivate::resumeAfterFailedTransfer, referer.get(),
			"Automatic CallSession resuming after failed transfer");
	}

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
		lInfo() << "Doing early media...";
		updateStreams(md, state);
	} else {
		if (state == CallSession::State::OutgoingEarlyMedia) {
			/* Already doing early media */
			return;
		}
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
	L_Q();

	const std::shared_ptr<SalMediaDescription> & rmd = op->getRemoteMediaDescription();
	char * remoteContactAddressStr = sal_address_as_string(op->getRemoteContactAddress());
	Address remoteContactAddress(remoteContactAddressStr);
	ms_free(remoteContactAddressStr);

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
			if (isUpdate && rmd->record != SalMediaRecordNone && lastRemoteRecordingState != rmd->record) {
				lastRemoteRecordingState = rmd->record;
				listener->onRemoteRecording(q->getSharedFromThis(), rmd->record == SalMediaRecordOn);
			}
			BCTBX_NO_BREAK;
		case CallSession::State::Updating:
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

bool MediaSessionPrivate::incompatibleSecurity(const std::shared_ptr<SalMediaDescription> &md) const {
	L_Q();
	if (isEncryptionMandatory()) {
		const auto negotiatedEncryption = getNegotiatedMediaEncryption();
		if (!!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "accept_any_encryption", 0)) {
			if (negotiatedEncryption==LinphoneMediaEncryptionNone) {
				lError() << "Encryption is mandatory however the negotiated encryption is " << linphone_media_encryption_to_string(negotiatedEncryption);
				return true;
			}
		} else {
			if ((negotiatedEncryption==LinphoneMediaEncryptionSRTP) && !md->hasSrtp()) {
				lError() << "Negotiated encryption is " << linphone_media_encryption_to_string(negotiatedEncryption) << " however media description has no stream has been negotiated with it";
				return true;
			}
		}
	}
	return false;
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

		bool enableCapabilityNegotiations = false;
		bool useNegotiatedMediaProtocol = true;
		// Add capability negotiation attribute during update if they are supported
		if (state == CallSession::State::StreamsRunning) {
			enableCapabilityNegotiations = q->isCapabilityNegotiationEnabled();
			useNegotiatedMediaProtocol = false;
		}

		// Reenable all streams if we are the offerer
		// This occurs with clients such as Avaya and FreeSwitch that put a call on hold by setting streams with inactive direction and RTP port to 0
		// Scenario:
		// - client1 sends an INVITE without SDP
		// - client2 puts its offer down in the 200Ok
		// - client1 answers with inactive stream and RTP port set to 0
		// Without the workaround, a deadlock is created - client1 has inactive streams and client2 has audio/video/text capabilities disabled in its local call parameters because the stream was rejected earlier on. Therefore it would be impossible to resume the streams if we are asked to make an offer.
		const bool makeOffer = (rmd == nullptr);
		if (makeOffer || ((state == CallSession::State::PausedByRemote) && (prevState == CallSession::State::UpdatedByRemote))) {
			for (const auto & stream : localDesc->streams) {
				switch (stream.getType()) {
					case SalAudio:
						getParams()->enableAudio(true);
						break;
					case SalVideo:
						getParams()->enableVideo(true);
						break;
					case SalText:
						getParams()->enableRealtimeText(true);
						break;
					case SalOther:
						break;
				}
			}
		}

		makeLocalMediaDescription(makeOffer, enableCapabilityNegotiations, useNegotiatedMediaProtocol);
	}
	// Fix local parameter after creating new local media description
	fixCallParams(rmd, true);
	if (rmd) {
		SalErrorInfo sei;
		memset(&sei, 0, sizeof(sei));
		expectMediaInAck = false;
		std::shared_ptr<SalMediaDescription> & md = op->getFinalMediaDescription();
		if (rejectMediaSession(rmd, md)) {
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
	MS2VideoStream *vs = getStreamsGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);
	if (vs) vs->oglRender();
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


int MediaSessionPrivate::sendDtmf (void *data, UNUSED(unsigned int revents)) {
	MediaSession *session = static_cast<MediaSession *>(data);
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
 	q->getCore()->getPrivate()->getToneManager().notifyState(q->getSharedFromThis(), newState);
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

	LinphoneConference * conference = nullptr;
	if (listener) {
		conference = listener->getCallSessionConference(q->getSharedFromThis());
	}
	bool isInLocalConference = getParams()->getPrivate()->getInConference();
	bool isInRemoteConference = conference && !isInLocalConference;

	/*params.getPrivate()->enableImplicitRtcpFb(params.getPrivate()->implicitRtcpFbEnabled() & sal_media_description_has_implicit_avpf(rmd));*/
	const MediaSessionParams *rcp = q->getRemoteParams();
	if (rcp) {

		/*
		 * This is to avoid to re-propose again some streams that have just been declined.
		 */
		if (getParams()->audioEnabled() && !rcp->audioEnabled() && !isInRemoteConference) {
			lInfo() << "CallSession [" << q << "]: disabling audio in our call params because the remote doesn't want it";
			getParams()->enableAudio(false);
		}
		if (getParams()->videoEnabled() && !rcp->videoEnabled() && !isInRemoteConference) {
			lInfo() << "CallSession [" << q << "]: disabling video in our call params because the remote doesn't want it";
			getParams()->enableVideo(false);
		}
		if (getParams()->realtimeTextEnabled() && !rcp->realtimeTextEnabled() && !isInRemoteConference) {
			lInfo() << "CallSession [" << q << "]: disabling RTT in our call params because the remote doesn't want it";
			getParams()->enableRealtimeText(false);
		}
		// Real Time Text is always by default accepted when proposed.
		if (!getParams()->realtimeTextEnabled() && rcp->realtimeTextEnabled())
			getParams()->enableRealtimeText(true);

		const auto & cCore = q->getCore()->getCCore();
		if (isInLocalConference) {
			// If the call is in a local conference, then check conference capabilities to know whether the video must be enabled or not
			bool isConferenceVideoCapabilityOn = false;
			if (conference) {
				const LinphoneConferenceParams * params = linphone_conference_get_current_params(conference);
				isConferenceVideoCapabilityOn = !!linphone_conference_params_video_enabled(params);
				if (rcp->videoEnabled() && !!linphone_core_video_enabled(cCore) && !getParams()->videoEnabled()) {
					getParams()->enableVideo(isConferenceVideoCapabilityOn);
				}
			}
		} else {
			if (rcp->videoEnabled() && cCore->video_policy.automatically_accept && linphone_core_video_enabled(cCore) && !getParams()->videoEnabled()) {
				lInfo() << "CallSession [" << q << "]: re-enabling video in our call params because the remote wants it and the policy allows to automatically accept";
				getParams()->enableVideo(true);
			}
		}

		// Enable bundle mode in local parameters if remote offered it and core can accept bundle mode, or
		// turn it off if the remote doesn't offer it or rejects it.
		// In fact, we can have the scenario where bundle mode is only enabled by one of the cores in the call or conference.
		// If bundle mode has been accepted, then future reINVITEs or UPDATEs must reoffer bundle mode unless the user has explicitely requested to disable it
		getParams()->enableRtpBundle(
			fromOffer ? (rcp->rtpBundleEnabled() && linphone_config_get_bool(linphone_core_get_config(cCore), "rtp", "accept_bundle", TRUE) )
			  : rcp->rtpBundleEnabled() );
	}
}

void MediaSessionPrivate::initializeParamsAccordingToIncomingCallParams () {
	CallSessionPrivate::initializeParamsAccordingToIncomingCallParams();
	std::shared_ptr<SalMediaDescription> md = op->getRemoteMediaDescription();
	if (md) {
		/* It is implicit to receive an INVITE without SDP, in this case WE choose the media parameters according to policy */
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
	const auto & videoStream = md->findBestStream(SalVideo);
	if (videoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()){
		hasAvpf = videoStream.hasAvpf();
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
	bool_t acceptAllEncryptions = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "accept_any_encryption", 0);

	if (md->hasZrtp() && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionZRTP)) {
		if (!mandatory || (mandatory && (acceptAllEncryptions || linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionZRTP)))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionZRTP);
	} else if (md->hasDtls() && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionDTLS)) {
		if (!mandatory || (mandatory && (acceptAllEncryptions || linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionDTLS)))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionDTLS);
	} else if (md->hasSrtp() && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionSRTP)) {
		if (!mandatory || (mandatory && (acceptAllEncryptions || linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionSRTP)))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionSRTP);
	} else if (getParams()->getMediaEncryption() != LinphoneMediaEncryptionZRTP) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionNone))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
	}

	const SalStreamDescription &audioStream = md->findBestStream(SalAudio);
	if (audioStream != Utils::getEmptyConstRefObject<SalStreamDescription>()){
		const std::string & rtpAddr = (audioStream.rtp_addr.empty() == false) ? audioStream.rtp_addr : md->addr;
		if (ms_is_multicast(rtpAddr.c_str())){
			lInfo() << "Incoming offer has audio multicast, enabling it in local params.";
			getParams()->enableAudioMulticast(true);
		}else getParams()->enableAudioMulticast(false);
	}
	const SalStreamDescription &videoStream = md->findBestStream(SalVideo);
	if (videoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()){
		const std::string & rtpAddr = (videoStream.rtp_addr.empty() == false) ? videoStream.rtp_addr : md->addr;
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

const LinphoneStreamInternalStats *MediaSessionPrivate::getStreamInternalStats(LinphoneStreamType type) const{
	Stream *s = getStreamsGroup().lookupMainStream(linphone_stream_type_to_sal(type));
	return s ? &s->getInternalStats() : nullptr;
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
		lInfo() << "Found media local-ip from configuration file: " << mediaLocalIp;
		return;
	}

	// If a known proxy was identified for this call, then we may have a chance to take the local ip address
	// from the socket that connects to this proxy
	if (destProxy && linphone_proxy_config_get_op(destProxy)) {
		ip = linphone_proxy_config_get_op(destProxy)->getLocalAddress(nullptr);
		if (ip) {
			if (needLocalIpRefresh) {
				af = strchr(ip, ':') ? AF_INET6 : AF_INET;
				lInfo() << "Address family for the local ip has changed to: " << (af == AF_INET6 ? "IPV6" : "IPV4");
			}
			if (strchr(ip, ':') && (af == AF_INET)) {
				// Case where we've decided to use IPv4 in selectOutgoingIpVersion(), but the signaling local ip address is IPv6.
				// We'll use the default media localip
			} else {
				mediaLocalIp = ip;
				lInfo() << "Found media local-ip from signaling: " << mediaLocalIp;
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
		auto stream = getStreamsGroup().getStream(index);
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
		const std::shared_ptr<SalMediaDescription> & md = localIsOfferer ? localDesc : op->getRemoteMediaDescription();
		const auto audioStreamIndex = md->findIdxBestStream(SalAudio);
		int audioPort = portFromStreamIndex(audioStreamIndex);

		LinphoneConference * conference = listener ? listener->getCallSessionConference(q->getSharedFromThis()) : nullptr;
		bool isConferenceLayoutActiveSpeaker = false;
		if (conference) {
			bool isInLocalConference = getParams()->getPrivate()->getInConference();
			const auto & confLayout = isInLocalConference ? getRemoteParams()->getConferenceVideoLayout() : getParams()->getConferenceVideoLayout();
			isConferenceLayoutActiveSpeaker = (confLayout == ConferenceLayout::ActiveSpeaker);
		}
		const auto mainStreamAttrValue = isConferenceLayoutActiveSpeaker ? "speaker" : "main";
		const auto videoStreamIndex = conference ? md->findIdxStreamWithContent(mainStreamAttrValue) : md->findIdxBestStream(SalVideo);
		int videoPort = portFromStreamIndex(videoStreamIndex);
		const auto textStreamIndex = md->findIdxBestStream(SalText);
		int textPort = portFromStreamIndex(textStreamIndex);
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
		const LinphoneAddress *to = log->getToAddress();

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
	lInfo() << "Media local-ip for streams advertised in SDP: " << mediaLocalIp;
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::forceStreamsDirAccordingToState (std::shared_ptr<SalMediaDescription> & md) {
	L_Q();
	for (auto & sd : md->streams) {
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
				if (sd.getDirection() == SalStreamRecvOnly) {
					sd.setDirection(SalStreamInactive);
				} else if (sd.getDirection() != SalStreamInactive) {
					sd.setDirection(SalStreamSendOnly);
					if ((sd.type == SalVideo) && linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "inactive_video_on_pause", 0))
						sd.setDirection(SalStreamInactive);
				}
				break;
			default:
				break;
		}
		/* Reflect the stream directions in the call params */
		if (sd.getType() == SalAudio) {
			getCurrentParams()->setAudioDirection(sd.getDirection());
		} else if (sd.getType() == SalVideo) {
			LinphoneConference * conference = listener ? listener->getCallSessionConference(q->getSharedFromThis()) : nullptr;
			SalStreamDir streamDir = SalStreamInactive;
			bool isInLocalConference = getParams()->getPrivate()->getInConference();
			if (conference) {
				if (isInLocalConference) {
					const auto cppConference = MediaConference::Conference::toCpp(conference)->getSharedFromThis();
					const auto & currentConfParams = cppConference->getCurrentParams();
					if (currentConfParams.videoEnabled()) {
						// At least receive the video streams of other participants if video is disabled in the call params
						streamDir = (getParams()->videoEnabled()) ? SalStreamSendRecv : SalStreamRecvOnly;
					} else {
						streamDir = SalStreamInactive;
					}
				} else {
					// Don't check conference parameters for remote conferences because the NOTIFY full state may have not been received yet
					// At least receive the video streams of other participants if video is disabled in the call params
					streamDir = (getParams()->videoEnabled()) ? SalStreamSendRecv : SalStreamRecvOnly;
				}
			} else {
				streamDir = sd.getDirection();
			}
			getCurrentParams()->setVideoDirection(streamDir);
		}
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


bool MediaSessionPrivate::mandatoryRtpBundleEnabled()const{
	if (!getParams()->rtpBundleEnabled()) return false;
	if (destProxy){
		return Account::toCpp(destProxy->account)->getAccountParams()->rtpBundleAssumptionEnabled();
	}
	return false;
}

void MediaSessionPrivate::addStreamToBundle(const std::shared_ptr<SalMediaDescription> & md, SalStreamDescription &sd, SalStreamConfiguration & cfg, const std::string & mid){
	if (cfg.dir != SalStreamInactive) {
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
		if (mandatoryRtpBundleEnabled() || bundleModeAccepted){
			// Bundle is offered inconditionnaly
			if (bundle.getMidOfTransportOwner() != mid){
				cfg.bundle_only = true;
				sd.rtp_port = 0;
			}
		}
		md->bundles.push_front(bundle);
	}
}

/* This function is to authorize the downgrade from avpf to non-avpf, when avpf is enabled locally but the remote
 * offer doesn't offer it consistently for all streams.
 */
SalMediaProto MediaSessionPrivate::getAudioProto(const std::shared_ptr<SalMediaDescription> remote_md, const bool useCurrentParams) const {
	SalMediaProto requested = getAudioProto(useCurrentParams);
	if (remote_md) {
		const SalStreamDescription &remote_stream = remote_md->findBestStream(SalAudio);
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

SalMediaProto MediaSessionPrivate::getAudioProto(const bool useCurrentParams) const {
	L_Q();
	/*This property is mainly used for testing hybrid case where the SDP offer is made with AVPF only for video stream.*/
	SalMediaProto ret = useCurrentParams ? linphone_media_encryption_to_sal_media_proto(getNegotiatedMediaEncryption(), getParams()->avpfEnabled()) : getParams()->getMediaProto();
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

void MediaSessionPrivate::fillRtpParameters(SalStreamDescription & stream) const {
	L_Q();

	auto & cfg = stream.cfgs[stream.getActualConfigurationIndex()];
	if (cfg.dir != SalStreamInactive)  {
		bool rtcpMux = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_mux", 0);
		/* rtcp-mux must be enabled when bundle mode is proposed.*/
		cfg.rtcp_mux = rtcpMux || getParams()->rtpBundleEnabled();
		cfg.rtcp_cname = getMe()->getAddress().asString();

		if (stream.rtp_port == 0 && !cfg.isBundleOnly()) {
			stream.rtp_port = SAL_STREAM_DESCRIPTION_PORT_TO_BE_DETERMINED;
		}

		if ((stream.type == SalAudio) && (getParams()->audioMulticastEnabled())) {
			cfg.ttl = linphone_core_get_audio_multicast_ttl(q->getCore()->getCCore());
			stream.multicast_role = (direction == LinphoneCallOutgoing) ? SalMulticastSender : SalMulticastReceiver;
		} else if ((stream.type == SalVideo) && (getParams()->videoMulticastEnabled())) {
			cfg.ttl = linphone_core_get_video_multicast_ttl(q->getCore()->getCCore());
			stream.multicast_role = (direction == LinphoneCallOutgoing) ? SalMulticastSender : SalMulticastReceiver;
		}

		if (stream.type == SalVideo) {
			/* this is a feature for tests only: */
			stream.bandwidth = getParams()->getPrivate()->videoDownloadBandwidth;
		}
	}
}

void MediaSessionPrivate::fillConferenceParticipantVideoStream(SalStreamDescription &newStream, const std::shared_ptr<SalMediaDescription> & oldMd, std::shared_ptr<SalMediaDescription> & md, const std::shared_ptr<ParticipantDevice> & dev, PayloadTypeHandler & pth, const std::list<LinphoneMediaEncryption> & encs) {
	L_Q();

	// Declare here an empty list to give to the makeCodecsList if there is no valid already assigned payloads
	std::list<OrtpPayloadType*> emptyList;
	emptyList.clear();

	SalStreamConfiguration cfg;
	cfg.proto = getParams()->getMediaProto();

	newStream.type = SalVideo;

	bool success = false;
	if (dev) {
		const auto & label = dev->getLabel();
		const auto & previousParticipantStream = oldMd ? oldMd->findStreamWithLabel(label) : Utils::getEmptyConstRefObject<SalStreamDescription>();
		const auto alreadyAssignedPayloads = ((previousParticipantStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) ? previousParticipantStream.already_assigned_payloads : emptyList);
		std::list<OrtpPayloadType*> l = pth.makeCodecsList(SalVideo, 0, -1, alreadyAssignedPayloads);
		if (!l.empty()) {
			newStream.setLabel(label);
			newStream.name = "Video " + dev->getAddress().asString();
			const auto & content = newStream.getContent();
			const bool isInLocalConference = getParams()->getPrivate()->getInConference();

			LinphoneConference * conference = listener ? listener->getCallSessionConference(q->getSharedFromThis()) : nullptr;
			const auto cppConference = (conference) ? MediaConference::Conference::toCpp(conference)->getSharedFromThis() : nullptr;
			const auto & participantDevice = cppConference ? (isInLocalConference ? cppConference->findParticipantDevice(q->getSharedFromThis()) : cppConference->getMe()->findDevice(q->getSharedFromThis())) : nullptr;

			auto dir = SalStreamInactive;
			if (cppConference && (participantDevice == dev)) {
				dir = (isInLocalConference) ? SalStreamRecvOnly : ((getParams()->getPrivate()->getSalVideoDirection() == SalStreamSendRecv) ? SalStreamSendOnly : SalStreamInactive);
			} else {
				if (content.compare("main") == 0) {
					dir = (isInLocalConference) ? SalStreamRecvOnly : SalStreamSendOnly;
				} else {
					if (isInLocalConference) {
						auto mediaDir = dev->getStreamCapability(LinphoneStreamTypeVideo);
						if ((mediaDir == LinphoneMediaDirectionSendRecv) || (mediaDir == LinphoneMediaDirectionSendOnly)) {
							dir = SalStreamSendOnly;
						} else if ((mediaDir == LinphoneMediaDirectionInactive) || (mediaDir == LinphoneMediaDirectionRecvOnly)) {
							dir = SalStreamInactive;
						}
					} else {
						dir = SalStreamRecvOnly;
					}
				}
			}
			if (dir == SalStreamInactive){
				lWarning() << *q << "Setting stream inactive for label " << label << " and content " << content;
			}
			cfg.dir = dir;
			validateVideoStreamDirection(cfg);
			if (getParams()->rtpBundleEnabled()) addStreamToBundle(md, newStream, cfg, "vs" + label);
			cfg.replacePayloads(l);
			newStream.addActualConfiguration(cfg);
			newStream.setSupportedEncryptions(encs);
			fillRtpParameters(newStream);
			success = true;
		}
		PayloadTypeHandler::clearPayloadList(l);
	}

	if (!success) {
		lInfo() << "Don't put video stream for device in conference with address " << (dev ? dev->getAddress().asString() : "<unknown>") << " on local offer for CallSession [" << q << "] because no valid payload has been found or device is not valid (pointer " << dev << ")";
		cfg.dir = SalStreamInactive;
		newStream.disable();
		newStream.rtp_port = 0;
		newStream.rtcp_port = 0;
		newStream.addActualConfiguration(cfg);
	}
}

void MediaSessionPrivate::fillLocalStreamDescription(SalStreamDescription & stream, std::shared_ptr<SalMediaDescription> & md, const bool enabled, const std::string name, const SalStreamType type, const SalMediaProto proto, const SalStreamDir dir, const std::list<OrtpPayloadType*> & codecs, const std::string mid, const SalCustomSdpAttribute *customSdpAttributes) {
	L_Q();

	const auto & dontCheckCodecs = (type == SalAudio) ? q->getCore()->getCCore()->codecs_conf.dont_check_audio_codec_support : ((type == SalVideo) ? q->getCore()->getCCore()->codecs_conf.dont_check_video_codec_support : false);

	SalStreamConfiguration cfg;
	cfg.proto = proto;
	stream.type = type;

	if (enabled && (!codecs.empty() || dontCheckCodecs)) {
		stream.name = name;
		cfg.dir = dir;
		stream.rtp_port = SAL_STREAM_DESCRIPTION_PORT_TO_BE_DETERMINED;

		cfg.replacePayloads(codecs);
		cfg.rtcp_cname = getMe()->getAddress().asString();


		LinphoneConference * conference = listener ? listener->getCallSessionConference(q->getSharedFromThis()) : nullptr;
		if ((type == SalAudio) && isInConference()) {
			cfg.mixer_to_client_extension_id = RTP_EXTENSION_MIXER_TO_CLIENT_AUDIO_LEVEL;
			cfg.client_to_mixer_extension_id = RTP_EXTENSION_CLIENT_TO_MIXER_AUDIO_LEVEL;
		} else if ((type == SalVideo) && conference) {
			validateVideoStreamDirection(cfg);
		}
		if (getParams()->rtpBundleEnabled()) addStreamToBundle(md, stream, cfg, mid);

		stream.addActualConfiguration(cfg);
		fillRtpParameters(stream);
	} else {
		lInfo() << "Don't put stream of type " << sal_stream_type_to_string(type) << " on local offer for CallSession [" << q << "]";
		const auto & core = q->getCore()->getCCore();
		cfg.dir = linphone_core_get_keep_stream_direction_for_rejected_stream(core) ? dir : SalStreamInactive;
		stream.rtp_port = 0;
		stream.addActualConfiguration(cfg);
	}
	if (customSdpAttributes)
		stream.custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);
}

SalStreamDescription & MediaSessionPrivate::addStreamToMd(std::shared_ptr<SalMediaDescription> md, int streamIdx, const std::shared_ptr<SalMediaDescription> & oldMd) {
	const auto oldSize = md->streams.size();
	const int protectedStreamNumber = 3;
	if (streamIdx < 0) {
		const auto inactiveStreamIt = (oldSize <= protectedStreamNumber) ? md->streams.cend() :
			std::find_if(md->streams.cbegin() + protectedStreamNumber, md->streams.cend(), [] (const auto s) {
				return (s.getDirection() == SalStreamInactive);
			});
		if (inactiveStreamIt == md->streams.cend()) {
			md->streams.resize(oldSize + 1);
			return md->streams[oldSize];
		} else {
			auto idx = static_cast<decltype(md->streams)::size_type>(std::distance(md->streams.cbegin(), inactiveStreamIt));
			return md->streams[idx];
		}
	} else {
		const auto & idx = static_cast<decltype(md->streams)::size_type>(streamIdx);
		try {
			auto stream = md->streams.at(idx);
			if ((stream.getDirection() != SalStreamInactive) && oldMd) {
				const auto oldMdSize = oldMd->streams.size();
				const auto streamInOldMdIt = ((oldMdSize <= (idx+2)) || (oldMdSize <= protectedStreamNumber)) ? oldMd->streams.cend() :
					std::find_if(oldMd->streams.cbegin() + protectedStreamNumber, oldMd->streams.cend(), [&stream] (const auto s) {
						return (s.getLabel() == stream.getLabel());
					});
				// If the stream to replace was not in the previous media description, search an inactive stream or append at the end
				if (streamInOldMdIt == oldMd->streams.cend()) {
					const auto inactiveStreamIt = (oldSize <= protectedStreamNumber) ? md->streams.cend() :
					std::find_if(md->streams.cbegin() + protectedStreamNumber, md->streams.cend(), [] (const auto s) {
						return (s.getDirection() == SalStreamInactive);
					});
					if (inactiveStreamIt == md->streams.cend()) {
						md->streams.push_back(stream);
					} else {
						auto inactiveStreamIdx = static_cast<decltype(md->streams)::size_type>(std::distance(md->streams.cbegin(), inactiveStreamIt));
						md->streams[inactiveStreamIdx] = stream;
					}
				} else {
					auto positionInOldMd = static_cast<decltype(oldMd->streams)::size_type>(std::distance(oldMd->streams.cbegin(), streamInOldMdIt));
					auto & streamToFill = addStreamToMd(md, static_cast<int>(positionInOldMd), oldMd);
					streamToFill = stream;
				}
			}
			return md->streams.at(idx);
		} catch (std::out_of_range&) {
			md->streams.resize(idx + 1);
			if (oldMd) {
				for (decltype(md->streams)::size_type i = oldSize; i < idx; i++) {
					const auto & s = oldMd->streams[i];
					auto & c = md->streams[i];
					c.type = s.type;
					/* FIXME: an explanation should be given for doing this, it is not obvious what this case is for.*/
					lWarning() << "Setting stream inactive at index " << i << " because of std::out_of_range.";
					c.setDirection(SalStreamInactive);
				}
			}
			return md->streams.at(idx);
		}
	}
}

void MediaSessionPrivate::addConferenceParticipantVideostreams(std::shared_ptr<SalMediaDescription> & md, const std::shared_ptr<SalMediaDescription> & oldMd, PayloadTypeHandler & pth, const std::list<LinphoneMediaEncryption> & encs) {
	L_Q();
	bool isInLocalConference = getParams()->getPrivate()->getInConference();
	LinphoneConference * conference = listener ? listener->getCallSessionConference(q->getSharedFromThis()) : nullptr;
	// Declare here an empty list to give to the makeCodecsList if there is no valid already assigned payloads
	std::list<OrtpPayloadType*> emptyList;
	emptyList.clear();
	if (conference) {
		const auto cppConference = MediaConference::Conference::toCpp(conference)->getSharedFromThis();
		const auto & currentConfParams = cppConference->getCurrentParams();
		bool isVideoConferenceEnabled = currentConfParams.videoEnabled();

		// Add additional video streams if required
		if (isVideoConferenceEnabled) {
			if (localIsOfferer && !linphone_core_conference_server_enabled(q->getCore()->getCCore())) {
				const auto & confLayout = isInLocalConference ? getRemoteParams()->getConferenceVideoLayout() : getParams()->getConferenceVideoLayout();

				bool isConferenceLayoutActiveSpeaker = (confLayout == ConferenceLayout::ActiveSpeaker);
				const auto & participantDeviceAddress = (isInLocalConference) ? remoteContactAddress : q->getContactAddress();

				const auto & participantDevice = isInLocalConference ? cppConference->findParticipantDevice(q->getSharedFromThis()) : cppConference->getMe()->findDevice(q->getSharedFromThis());
				const auto & devLabel = participantDevice->getLabel();
				const std::string content("thumbnail");
				const auto & foundStreamIdx = oldMd->findIdxStreamWithContent(content, devLabel);

				SalStreamDescription & newStream = addStreamToMd(md, foundStreamIdx, oldMd);
				newStream.setContent(content);
				fillConferenceParticipantVideoStream(newStream, oldMd, md, participantDevice, pth, encs);

				for (const auto & p : cppConference->getParticipants()) {
					for (const auto & dev : p->getDevices()) {
						const auto & devAddress = dev->getAddress().asAddress();
						const auto & devState = dev->getState();
						// Do not add stream for device matching the remote contact address if the chosen layout is active speaker
						if ((participantDeviceAddress != devAddress) && (devState != ParticipantDevice::State::ScheduledForJoining) && (devState != ParticipantDevice::State::Joining) && (devState != ParticipantDevice::State::Alerting))  {
							const auto & devLabel = dev->getLabel();
							// main stream has the same label as one of the minature streams
							const auto & foundStreamIdx = devLabel.empty() ? -1 : oldMd->findIdxStreamWithLabel(devLabel);
							SalStreamDescription & newParticipantStream = addStreamToMd(md, foundStreamIdx, oldMd);
							if (isConferenceLayoutActiveSpeaker) {
								newParticipantStream.setContent(content);
							}
							fillConferenceParticipantVideoStream(newParticipantStream, oldMd, md, dev, pth, encs);
						}
					}
				}

				if (cppConference->isIn() && isInLocalConference) {
					const auto & me = cppConference->getMe();
					for (const auto & dev : me->getDevices()) {
						const auto & devLabel = dev->getLabel();
						std::vector<std::pair<std::string, std::string>> attributes;
						const auto & foundStreamIdx = devLabel.empty() ? -1 : ((participantDeviceAddress == dev->getAddress().asAddress()) ? oldMd->findIdxStreamWithContent(content, devLabel) : md->findIdxStreamWithLabel(devLabel));
						SalStreamDescription & newMeStream = addStreamToMd(md, foundStreamIdx, oldMd);
						if (isConferenceLayoutActiveSpeaker) {
							newMeStream.setContent(content);
						}
						fillConferenceParticipantVideoStream(newMeStream, oldMd, md, dev, pth, encs);
					}
				}
			} else {
				// The conference server is a passive core, therefore he must not add any stream to the SDP
				const auto & refMd = (linphone_core_conference_server_enabled(q->getCore()->getCCore()) && localIsOfferer) ? oldMd : op->getRemoteMediaDescription();
				const auto noStreams = md->streams.size();
				const auto remoteNoStreams = refMd->streams.size();
				if (noStreams <= remoteNoStreams) {
					auto beginIt = refMd->streams.cbegin();
					std::advance(beginIt, static_cast<decltype(beginIt)::difference_type>(noStreams));

					for (auto sIt = beginIt; sIt != refMd->streams.end(); sIt++) {
						const auto & s = *sIt;
						if (s.getType() == SalVideo) {
							const std::string contentAttrValue = s.getContent();
							const std::string participantsAttrValue = s.getLabel();

							std::shared_ptr<ParticipantDevice> dev = nullptr;
							if (!participantsAttrValue.empty()) {
								dev = cppConference->findParticipantDeviceByLabel(participantsAttrValue);
								if (!dev && cppConference->getMe()) {
									// It might be me
									dev = cppConference->getMe()->findDevice(participantsAttrValue, false);
								}
							}

							const auto & idx = md->streams.size();
							SalStreamDescription & newStream = addStreamToMd(md, static_cast<int>(idx), oldMd);
							if (dev) {
								newStream.setContent(contentAttrValue);
								fillConferenceParticipantVideoStream(newStream, oldMd, md, dev, pth, encs);
							} else {
								const auto & s = refMd->streams[idx];
								SalStreamConfiguration cfg;
								cfg.dir = SalStreamInactive;
								newStream.disable();
								newStream.type = s.type;
								newStream.rtp_port = 0;
								newStream.rtcp_port = 0;
								newStream.addActualConfiguration(cfg);
								lWarning() << *q << "New stream added as disabled and inactive.";
							}
						}
					}
				}
			}
		}
	}
}

void MediaSessionPrivate::validateVideoStreamDirection(SalStreamConfiguration & cfg) const {
	L_Q();
	// Check core parameter to eventually change the media direction
	// video capture enables sending the stream
	// video display enables receiving the stream
	// capture | display | allowed directions
	//   false |  false  |  Inactive
	//   false |   true  |  RecvOnly, Inactive
	//   true  |  false  |  SendOnly, Inactive
	//   true  |   true  |  SendOnly, RecvOnly, SendRecv, Inactive
	const auto & cCore = q->getCore()->getCCore();
	const auto captureEnabled = !!linphone_core_video_capture_enabled(cCore);
	const auto displayEnabled = !!linphone_core_video_display_enabled(cCore);
	const auto oldVideoDir = cfg.dir;
	if (((oldVideoDir == SalStreamSendOnly) && !captureEnabled) || ((oldVideoDir == SalStreamRecvOnly) && !displayEnabled)) {
		cfg.dir = SalStreamInactive;
	} else if (oldVideoDir == SalStreamSendRecv) {
		if (!captureEnabled && !displayEnabled) {
			cfg.dir = SalStreamInactive;
		} else if (captureEnabled && !displayEnabled) {
			cfg.dir = SalStreamSendOnly;
		} else if (!captureEnabled && displayEnabled) {
			cfg.dir = SalStreamRecvOnly;
		}
	}

	if (oldVideoDir != cfg.dir) {
		lWarning() << "Video direction of a video stream has been changed from " << std::string(sal_stream_dir_to_string(oldVideoDir)) << " to " << std::string(sal_stream_dir_to_string(cfg.dir)) << " as video capture is " << std::string(captureEnabled ? "enabled" : "disabled") << " and video display is " << std::string(displayEnabled ? "enabled" : "disabled") << " in the core settings";
	}
}

void MediaSessionPrivate::copyOldStreams(std::shared_ptr<SalMediaDescription> & md, const std::shared_ptr<SalMediaDescription> & oldMd, const std::shared_ptr<SalMediaDescription> & refMd, PayloadTypeHandler & pth, const std::list<LinphoneMediaEncryption> & encs) {
	L_Q();

	if (refMd) {
		// Declare here an empty list to give to the makeCodecsList if there is no valid already assigned payloads
		std::list<OrtpPayloadType*> emptyList;
		emptyList.clear();
		std::list<OrtpPayloadType*> l;

		const auto noStreams = md->streams.size();
		const auto refNoStreams = refMd->streams.size();
		if (noStreams <= refNoStreams) {
			// Copy participant video streams from previous local description
			auto streamIdx = noStreams;
			auto beginIt = refMd->streams.cbegin();
			std::advance(beginIt, static_cast<decltype(beginIt)::difference_type>(noStreams));

			for (auto sIt = beginIt; sIt != refMd->streams.end(); sIt++) {
				const auto & s = *sIt;
				const auto & idx = static_cast<int>(md->streams.size());
				SalStreamDescription & newStream = addStreamToMd(md, idx, oldMd);
				newStream.type = s.type;
				newStream.name = s.name;
				newStream.disable();
				SalStreamConfiguration cfg;
				cfg.proto = s.getProto();
				cfg.dir = SalStreamInactive;

				const auto & previousParticipantStream = (oldMd) ? oldMd->getStreamIdx(static_cast<unsigned int>(streamIdx)) : Utils::getEmptyConstRefObject<SalStreamDescription>();
				l = pth.makeCodecsList(s.type, 0, -1, ((previousParticipantStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) ? previousParticipantStream.already_assigned_payloads : emptyList));
				if (!l.empty()){
					cfg.payloads = l;
				} else {
					lInfo() << "Don't put " << sal_stream_type_to_string(s.type) << " stream (index " << streamIdx << ") on local offer for CallSession [" << q << "] because no payload is found";
					PayloadTypeHandler::clearPayloadList(l);
					cfg.dir = SalStreamInactive;
					newStream.disable();
				}
				/* FIXME: need comment to explain what is being done here. Why calling disable() twice, even when there are payload types ? */
				lWarning() << "Disabling stream at index " << idx << " from copyOldStreams().";
				newStream.disable();
				newStream.rtp_port = 0;
				newStream.rtcp_port = 0;

				newStream.setSupportedEncryptions(encs);
				newStream.addActualConfiguration(cfg);
				fillRtpParameters(newStream);

				streamIdx++;
			}
		}
	}
}

void MediaSessionPrivate::makeLocalMediaDescription(bool localIsOfferer, const bool supportsCapabilityNegotiationAttributes, const bool offerNegotiatedMediaProtocolOnly, const bool forceCryptoKeyGeneration) {
	L_Q();
	const auto & core = q->getCore()->getCCore();
	bool isInLocalConference = getParams()->getPrivate()->getInConference();

	LinphoneConference * conference = listener ? listener->getCallSessionConference(q->getSharedFromThis()) : nullptr;
	std::shared_ptr<SalMediaDescription> md = std::make_shared<SalMediaDescription>(supportsCapabilityNegotiationAttributes, getParams()->getPrivate()->tcapLinesMerged());
	std::shared_ptr<SalMediaDescription> & oldMd = localDesc;
	const std::shared_ptr<SalMediaDescription> & refMd = (conference) ? 
		((isInLocalConference) ? oldMd : op->getRemoteMediaDescription()) :
		((localIsOfferer) ? oldMd : op->getRemoteMediaDescription())
	;

	this->localIsOfferer = localIsOfferer;

	getParams()->getPrivate()->adaptToNetwork(core, pingTime);

	string subject = q->getParams()->getSessionName();
	if (!subject.empty()) {
		md->name = subject;
	}
	md->session_id = (oldMd ? oldMd->session_id : (bctbx_random() & 0xfff));
	md->session_ver = (oldMd ? (oldMd->session_ver + 1) : (bctbx_random() & 0xfff));

	md->times.push_back(std::make_pair<time_t, time_t>(getParams()->getPrivate()->getStartTime(), getParams()->getPrivate()->getEndTime()));

	md->accept_bundles = getParams()->rtpBundleEnabled() ||
		linphone_config_get_bool(linphone_core_get_config(core), "rtp", "accept_bundle", TRUE);

	if (getParams()->recordAwareEnabled() || linphone_core_is_record_aware_enabled(core)) {
		md->record = getParams()->getRecordingState();
	}

	/* Re-check local ip address each time we make a new offer, because it may change in case of network reconnection */
	{
		const LinphoneAddress *address = (direction == LinphoneCallOutgoing ? log->getToAddress() : log->getFromAddress());
		getLocalIp(*L_GET_CPP_PTR_FROM_C_OBJECT(address));
	}

	md->origin_addr = mediaLocalIp;
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

	// Declare here an empty list to give to the makeCodecsList if there is no valid already assigned payloads
	std::list<OrtpPayloadType*> emptyList;
	emptyList.clear();

	auto encList = q->getSupportedEncryptions();
	// Delete duplicates
	encList.unique();
	// Do not add capability negotiation attributes if encryption is mandatory
	const bool addCapabilityNegotiationAttributes = supportsCapabilityNegotiationAttributes && !linphone_core_is_media_encryption_mandatory(core);
	if (addCapabilityNegotiationAttributes) {
		for (const auto & enc : encList) {
			const std::string mediaProto(sal_media_proto_to_string(linphone_media_encryption_to_sal_media_proto(enc, (getParams()->avpfEnabled() ? TRUE : FALSE))));
			const auto & idx = md->getFreeTcapIdx();

			const auto & tcaps = md->getTcaps();
			const auto & tcapFoundIt = std::find_if(tcaps.cbegin(), tcaps.cend(), [&mediaProto] (const auto & cap) {
				return (mediaProto.compare(cap.second) == 0);
			});

			if (tcapFoundIt == tcaps.cend()) {
				lInfo() << "Adding media protocol " << mediaProto << " at index " << idx << " for encryption " << linphone_media_encryption_to_string(enc);
				md->addTcap(idx, mediaProto);
			} else {
				lInfo() << "Media protocol " << mediaProto << " is already found at " << tcapFoundIt->first << " hence a duplicate will not be added to the tcap list";
			}
		}
	}

	encList.push_back(getParams()->getMediaEncryption());

	bool conferenceCreated = false;
	bool isConferenceLayoutActiveSpeaker = false;
	bool isVideoConferenceEnabled = false;
	ConferenceLayout confLayout = ConferenceLayout::ActiveSpeaker;
	auto deviceState = ParticipantDevice::State::ScheduledForJoining;
	std::shared_ptr<ParticipantDevice> participantDevice = nullptr;
	if (conference) {
		const auto cppConference = MediaConference::Conference::toCpp(conference)->getSharedFromThis();
		const auto & currentConfParams = cppConference->getCurrentParams();
		const auto & conferenceState = cppConference->getState();

		participantDevice = isInLocalConference ? cppConference->findParticipantDevice(q->getSharedFromThis()) : cppConference->getMe()->findDevice(q->getSharedFromThis());
		if (participantDevice) {
			deviceState = participantDevice->getState();
		}

		isVideoConferenceEnabled = currentConfParams.videoEnabled();
		confLayout = (participantDevice && isInLocalConference && getRemoteParams()) ? getRemoteParams()->getConferenceVideoLayout() : getParams()->getConferenceVideoLayout();
		isConferenceLayoutActiveSpeaker = (confLayout == ConferenceLayout::ActiveSpeaker);
		if (isInLocalConference) {
			// If the conference is dialing out to participants and an internal update (i.e. ICE reINVITE or capability negotiations reINVITE) occurs
			if (getParams()->getPrivate()->getInternalCallUpdate() && (direction == LinphoneCallOutgoing)) {
				conferenceCreated = (deviceState == ParticipantDevice::State::Present);
			} else {
				conferenceCreated = !(conferenceState == ConferenceInterface::State::Instantiated);
			}
		} else {
			conferenceCreated = !((conferenceState == ConferenceInterface::State::Instantiated) || (conferenceState == ConferenceInterface::State::CreationPending));
		}
	}

	const SalStreamDescription &oldAudioStream = refMd ? refMd->findBestStream(SalAudio) : Utils::getEmptyConstRefObject<SalStreamDescription>();

	if ((!conference || (localIsOfferer && getParams()->audioEnabled())) || (oldAudioStream != Utils::getEmptyConstRefObject<SalStreamDescription>())){
		auto audioCodecs = pth.makeCodecsList(SalAudio, getParams()->getAudioBandwidthLimit(), -1, ((oldAudioStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) ? oldAudioStream.already_assigned_payloads : emptyList));

		const auto audioStreamIdx = (conference && refMd) ? refMd->findIdxBestStream(SalAudio) : -1;
		SalStreamDescription & audioStream = addStreamToMd(md, audioStreamIdx, oldMd);
		fillLocalStreamDescription(audioStream, md, getParams()->audioEnabled(), "Audio", SalAudio, getAudioProto(op ? op->getRemoteMediaDescription() : nullptr, offerNegotiatedMediaProtocolOnly), getParams()->getPrivate()->getSalAudioDirection(), audioCodecs, "as", getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeAudio));

		auto & actualCfg = audioStream.cfgs[audioStream.getActualConfigurationIndex()];

		audioStream.setSupportedEncryptions(encList);
		actualCfg.max_rate = pth.getMaxCodecSampleRate(audioCodecs);
		int downPtime = getParams()->getPrivate()->getDownPtime();
		if (downPtime)
			actualCfg.ptime = downPtime;
		else
			actualCfg.ptime = linphone_core_get_download_ptime(core);

		PayloadTypeHandler::clearPayloadList(audioCodecs);
	}

	const auto mainStreamAttrValue = isConferenceLayoutActiveSpeaker ? "speaker" : "main";

	auto callVideoEnabled = (!conferenceCreated && conference) ? getCurrentParams()->videoEnabled() : getParams()->videoEnabled();
	bool addVideoStream = callVideoEnabled;
	// If the call is linked to a conference, search stream with content main first
	// Check if there was a main stream earlier on in the SDP.
	// It is necessary to check for both Grid and ActiveSpeaker layout in order to cover the case when the layout is changed
	const SalStreamDescription &oldGridLayoutMainVideoStream = refMd ? refMd->findStreamWithContent("main") : Utils::getEmptyConstRefObject<SalStreamDescription>();
	const SalStreamDescription &oldActiveSpeakerLayoutMainVideoStream = refMd ? refMd->findStreamWithContent("speaker") : Utils::getEmptyConstRefObject<SalStreamDescription>();
	const SalStreamDescription &oldMainVideoStream = refMd ? refMd->findBestStream(SalVideo) : Utils::getEmptyConstRefObject<SalStreamDescription>();
	if (conference && isInLocalConference) {
		addVideoStream &= isVideoConferenceEnabled;
	}

	const SalStreamDescription &oldVideoStream = (conference) ? ((oldGridLayoutMainVideoStream == Utils::getEmptyConstRefObject<SalStreamDescription>()) ? oldActiveSpeakerLayoutMainVideoStream : oldGridLayoutMainVideoStream) : oldMainVideoStream;
	if (addVideoStream || (oldVideoStream != Utils::getEmptyConstRefObject<SalStreamDescription>())) {
		auto videoCodecs = pth.makeCodecsList(SalVideo, 0, -1, ((oldVideoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) ? oldVideoStream.already_assigned_payloads : emptyList));
		const auto proto = offerNegotiatedMediaProtocolOnly ? linphone_media_encryption_to_sal_media_proto(getNegotiatedMediaEncryption(), getParams()->avpfEnabled()) : getParams()->getMediaProto();

		SalStreamDir videoDir = SalStreamInactive;
		bool enableVideoStream = false;
		// Set direction appropriately to configuration
		if (conference) {
			if (isInLocalConference) {
				if (!isVideoConferenceEnabled) {
					enableVideoStream = false;
					videoDir = SalStreamInactive;
				} else if ((deviceState == ParticipantDevice::State::Joining) || (deviceState == ParticipantDevice::State::Present) || (deviceState == ParticipantDevice::State::OnHold)) {
					enableVideoStream = (deviceState == ParticipantDevice::State::Joining) ? true : callVideoEnabled;
					// Enable video based on conference capabilities if:
					// - joining conference
					// - receiving an offer
					switch (confLayout) {
						case ConferenceLayout::ActiveSpeaker:
							videoDir = SalStreamSendRecv;
							break;
						case ConferenceLayout::Grid:
							videoDir = SalStreamRecvOnly;
							break;
					}
				} else if (deviceState == ParticipantDevice::State::ScheduledForJoining) {
					videoDir = getParams()->getPrivate()->getSalVideoDirection();
					enableVideoStream = true;
				} else {
					videoDir = MediaSessionParamsPrivate::mediaDirectionToSalStreamDir(participantDevice->getStreamCapability(LinphoneStreamTypeVideo));
					enableVideoStream = (localIsOfferer) ? callVideoEnabled : isVideoConferenceEnabled;
				}
			} else {
				enableVideoStream = callVideoEnabled;
				switch (confLayout) {
					case ConferenceLayout::ActiveSpeaker:
						videoDir = getParams()->getPrivate()->getSalVideoDirection();
						break;
					case ConferenceLayout::Grid:
						videoDir = (localIsOfferer) ? ((getParams()->getPrivate()->getSalVideoDirection() == SalStreamSendRecv) ? SalStreamSendOnly : SalStreamInactive) : getParams()->getPrivate()->getSalVideoDirection();
						break;
				}
			}
		} else {
			videoDir = getParams()->getPrivate()->getSalVideoDirection();
			enableVideoStream = addVideoStream;
		}

		const auto videoStreamIdx = refMd ? ((conference && refMd->findIdxStreamWithContent(mainStreamAttrValue)) ? refMd->findIdxStreamWithContent(mainStreamAttrValue) : refMd->findIdxBestStream(SalVideo)) : -1;

		SalStreamDescription & videoStream = addStreamToMd(md, videoStreamIdx, oldMd);
		if (videoDir == SalStreamInactive){
			lWarning() << *q << "Video stream added with inactive media direction.";
		}
		fillLocalStreamDescription(videoStream, md, enableVideoStream, "Video", SalVideo, proto, videoDir, videoCodecs, "vs", getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeVideo));

		if (conference) {
			// The call to getRemoteContactAddress updates CallSessionPrivate member remoteContactAddress
			const auto remoteContactAddress = q->getRemoteContactAddress();
			if (remoteContactAddress) {
				if (participantDevice && (isInLocalConference || (!isInLocalConference && remoteContactAddress->hasParam("isfocus")))) {
					videoStream.setLabel(participantDevice->getLabel());
					videoStream.setContent(mainStreamAttrValue);
				}
			}
		}

		videoStream.setSupportedEncryptions(encList);

		PayloadTypeHandler::clearPayloadList(videoCodecs);
	}

	const SalStreamDescription &oldTextStream = refMd ? refMd->findBestStream(SalText) : Utils::getEmptyConstRefObject<SalStreamDescription>();
	if ((localIsOfferer && getParams()->realtimeTextEnabled()) || (oldTextStream != Utils::getEmptyConstRefObject<SalStreamDescription>())){
		auto textCodecs = pth.makeCodecsList(SalText, 0, -1, ((oldTextStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) ? oldTextStream.already_assigned_payloads : emptyList));

		const auto proto = offerNegotiatedMediaProtocolOnly ? linphone_media_encryption_to_sal_media_proto(getNegotiatedMediaEncryption(), getParams()->avpfEnabled()) : getParams()->getMediaProto();

		const auto textStreamIdx = refMd ? refMd->findIdxBestStream(SalText) : -1;
		SalStreamDescription & textStream = addStreamToMd(md, textStreamIdx, oldMd);
		fillLocalStreamDescription(textStream, md, getParams()->realtimeTextEnabled(), "Text", SalText, proto, SalStreamSendRecv, textCodecs, "ts", getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeText));

		textStream.setSupportedEncryptions(encList);

		PayloadTypeHandler::clearPayloadList(textCodecs);
	}

#ifdef HAVE_ADVANCED_IM
	bool eventLogEnabled = !!linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "misc", "conference_event_log_enabled", TRUE );
	if (conferenceCreated && eventLogEnabled && addVideoStream && participantDevice && ((deviceState == ParticipantDevice::State::Joining) || (deviceState == ParticipantDevice::State::Present) || (deviceState == ParticipantDevice::State::OnHold))) {
		addConferenceParticipantVideostreams(md, oldMd, pth, encList);
	}
#endif // HAVE_ADVANCED_IM
	copyOldStreams(md, oldMd, refMd, pth, encList);

	setupEncryptionKeys(md, forceCryptoKeyGeneration);
	setupImEncryptionEngineParameters(md);
	setupRtcpFb(md);
	setupRtcpXr(md);
	if (stunClient)
		stunClient->updateMediaDescription(md);
	localDesc = md;

	if (!getParams()->getPrivate()->isConferenceCreation()) {
		OfferAnswerContext ctx;
		ctx.localMediaDescription = localDesc;
		ctx.remoteMediaDescription = localIsOfferer ? nullptr : ( op ? op->getRemoteMediaDescription() : nullptr);
		ctx.localIsOfferer = localIsOfferer;
		/* Now instanciate the streams according to the media description. */
		getStreamsGroup().createStreams(ctx);
	}

	const auto & mdForMainStream = localIsOfferer ? md : refMd;
	const auto audioStreamIndex = mdForMainStream->findIdxBestStream(SalAudio);
	if (audioStreamIndex != -1) getStreamsGroup().setStreamMain(static_cast<size_t>(audioStreamIndex));
	const auto videoStreamIndex = (conference || remoteContactAddress.hasParam("isfocus")) ? mdForMainStream->findIdxStreamWithContent(mainStreamAttrValue) : mdForMainStream->findIdxBestStream(SalVideo);
	if (videoStreamIndex != -1) getStreamsGroup().setStreamMain(static_cast<size_t>(videoStreamIndex));
	const auto textStreamIndex = mdForMainStream->findIdxBestStream(SalText);
	if (textStreamIndex != -1) getStreamsGroup().setStreamMain(static_cast<size_t>(textStreamIndex));
	/* Get the transport addresses filled in to the media description. */
	updateLocalMediaDescriptionFromIce(localIsOfferer);
	if (oldMd) {
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
		case MS_AES_128_SHA1_80_NO_AUTH:
		case MS_AES_128_SHA1_32_NO_AUTH:
		case MS_AES_128_SHA1_80_SRTP_NO_CIPHER: /* Not sure for this one */
		case MS_AES_128_SHA1_80_SRTCP_NO_CIPHER: /* Not sure for this one */
		case MS_AES_128_SHA1_80_NO_CIPHER: /* Not sure for this one */
			keylen = 30;
			break;
		case MS_AEAD_AES_128_GCM:
			keylen = 28;
			break;
		case MS_AEAD_AES_256_GCM:
			keylen = 44;
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

		for (auto & stream : md->streams) {
			stream.setupRtcpXr(md->rtcp_xr);
		}
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

void MediaSessionPrivate::setupEncryptionKeys (std::shared_ptr<SalMediaDescription> & md, const bool forceKeyGeneration) {
	L_Q();
	std::shared_ptr<SalMediaDescription> & oldMd = localDesc;
	bool keepSrtpKeys = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "keep_srtp_keys", 1);
	const std::string attrName("crypto");
	for (size_t i = 0; i < md->streams.size(); i++) {

		auto & newStream = md->streams[i];

		// Make best effort to keep same keys if user wishes so
		if (newStream.enabled()) {
			auto & newStreamActualCfg = newStream.cfgs[newStream.getActualConfigurationIndex()];
			auto & newStreamActualCfgCrypto = newStreamActualCfg.crypto;

			if (keepSrtpKeys && oldMd && (i < oldMd->streams.size()) && oldMd->streams[i].enabled()) {
				const auto & oldStream = oldMd->streams[i];
				const auto & oldStreamSupportedEncryptions = oldStream.getSupportedEncryptions();
				const bool oldStreamSupportsSrtp = (std::find(oldStreamSupportedEncryptions.cbegin(), oldStreamSupportedEncryptions.cend(), LinphoneMediaEncryptionSRTP) != oldStreamSupportedEncryptions.cend());
				const auto newStreamSupportedEncryptions = newStream.getSupportedEncryptions();
				const bool newStreamSupportsSrtp = (std::find(newStreamSupportedEncryptions.cbegin(), newStreamSupportedEncryptions.cend(), LinphoneMediaEncryptionSRTP) != newStreamSupportedEncryptions.cend());
				const auto & oldStreamActualCfg = oldStream.getActualConfiguration();
				const auto & oldStreamActualCfgCrypto = oldStreamActualCfg.crypto;

				// Actual configuration
				if (newStreamActualCfg.hasSrtp()) {
					if (forceKeyGeneration) {
						// Generate new crypto keys
						newStreamActualCfgCrypto = generateNewCryptoKeys();
					} else if (oldStreamActualCfg.hasSrtp()) {
						// If old stream actual configuration supported SRTP, then copy crypto parameters
						lInfo() << "Keeping same crypto keys when making new local stream description";
						newStreamActualCfgCrypto = oldStreamActualCfgCrypto;
					} else if (oldMd->supportCapabilityNegotiation()) {
						// Search crypto attributes in acaps if previous media description did support capability negotiations
						// Copy acap crypto attributes if old stream supports it as potential configuration
						for (const auto & cap : oldStream.acaps) {
							const auto & nameValuePair = cap.second;
							const auto & name = nameValuePair.first;
							if (name.compare(attrName) == 0) {
								const auto & attrValue = nameValuePair.second;

								const auto keyEnc = SalStreamConfiguration::fillStrpCryptoAlgoFromString(attrValue);
								if (keyEnc.algo!=MS_CRYPTO_SUITE_INVALID){
									newStreamActualCfgCrypto.push_back(keyEnc);
								}
							}
						}
					} else {
						newStreamActualCfgCrypto = generateNewCryptoKeys();
					}
				}

				// If capability negotiation is enabled, search keys among acaps
				if (md->supportCapabilityNegotiation()) {
					// If both old and new stream support SRTP as potential configuration
					if (newStreamSupportsSrtp && !forceKeyGeneration) {
						if (oldStreamSupportsSrtp && oldMd->supportCapabilityNegotiation()) {
							// Copy acap crypto attributes if old stream supports it as potential configuration
							for (const auto & cap : oldStream.acaps) {
								const auto & nameValuePair = cap.second;
								const auto & name = nameValuePair.first;
								if (name.compare(attrName) == 0) {
									const auto & value = nameValuePair.second;
									const auto & idx = cap.first;
									newStream.addAcap(idx, name, value);
								}
							}
						} else if (oldStreamActualCfg.hasSrtp()) {
							// Copy crypto attributes from actual configuration if old stream supports it as actual configuration
							for (const auto & c : oldStreamActualCfgCrypto) {
								MSCryptoSuiteNameParams desc;
								if (ms_crypto_suite_to_name_params(c.algo,&desc)==0){
									const auto & idx = md->getFreeAcapIdx();
									const auto value = SalStreamConfiguration::cryptoToSdpValue(c);
									newStream.addAcap(idx, attrName, value);
								}
							}
						}
					}
				}
			} else {
				if (newStreamActualCfg.hasSrtp()) {
					newStreamActualCfgCrypto = generateNewCryptoKeys();
				}
			}

			if (newStreamActualCfg.hasSrtp() && newStreamActualCfgCrypto.empty()) {
				lInfo() << "Don't put stream " << i << " on local offer for CallSession [" << q << "] because it requires protocol " << sal_media_proto_to_string(newStreamActualCfg.getProto()) << " but no suitable crypto key has been found.";
				newStreamActualCfg.dir = SalStreamInactive;
				md->streams[i].disable();
			}
		}
	}
}

/**
 * Convert enum LinphoneSrtpSuite into enum MSCryptoSuite
 * Enums definitions are not perferctly matching
 * any input without corresponding MSCryptoSuite value gives a MS_CRYPTO_SUITE_INVALID output
 *
 * @param[in]	suite	The LinphoneSrtpSuite to be converted
 * @return	the matching MSCryptoSuite value
 **/
MSCryptoSuite LinphoneSrtpSuite2MSCryptoSuite(const LinphoneSrtpSuite suite) {
	switch (suite) {
		case LinphoneSrtpSuiteAESCM128HMACSHA180 : return MS_AES_128_SHA1_80;
		case LinphoneSrtpSuiteAESCM128HMACSHA132 : return MS_AES_128_SHA1_32;
		case LinphoneSrtpSuiteAES256CMHMACSHA180 : return MS_AES_256_SHA1_80;
		case LinphoneSrtpSuiteAES256CMHMACSHA132 : return MS_AES_256_SHA1_32;
		case LinphoneSrtpSuiteAEADAES128GCM : return MS_AEAD_AES_128_GCM;
		case LinphoneSrtpSuiteAEADAES256GCM : return MS_AEAD_AES_256_GCM;
		// all these case are not supported by the MS enumeration
		case LinphoneSrtpSuiteAES192CMHMACSHA180 :
		case LinphoneSrtpSuiteAES192CMHMACSHA132 :
		case LinphoneSrtpSuiteInvalid :
		default:
			return MS_CRYPTO_SUITE_INVALID;
	}
}

/**
 * Convert a list of enum LinphoneSrtpSuite into a list enum MSCryptoSuite
 * Enums definitions are not perferctly matching
 * input giving MS_CRYPTO_SUITE_INVALID are skipped in the output list
 *
 * @param[in]	suite	The list of LinphoneSrtpSuite to be converted
 * @return	the matching MSCryptoSuite list, unconvertible input are skipped
 **/
std::list<MSCryptoSuite> LinphoneSrtpSuite2MSCryptoSuite(const std::list<LinphoneSrtpSuite> suites) {
	std::list<MSCryptoSuite> ret{};
	for (const auto suite:suites) {
		auto MSsuite = LinphoneSrtpSuite2MSCryptoSuite(suite);
		if (MSsuite != MS_CRYPTO_SUITE_INVALID) {
			ret.push_back(MSsuite);
		}
	}
	return ret;
}

/**
 * Convert enum MSCryptoSuite into enum LinphoneSrtpSuite
 * Enums definitions are not perferctly matching
 * any input without corresponding LinphoneCryptoSuite value gives a LinphoneSrtpSuiteInvalid output
 *
 * @param[in]	suite	The MSCryptoSuite to be converted
 * @return	the matching LinphoneSrtpSuite value
 **/
static LinphoneSrtpSuite MSCryptoSuite2LinphoneSrtpSuite(const MSCryptoSuite suite) {
	switch (suite) {
		case MS_AES_128_SHA1_32 : return LinphoneSrtpSuiteAESCM128HMACSHA132;
		case MS_AES_128_SHA1_80 : return LinphoneSrtpSuiteAESCM128HMACSHA180;
		case MS_AES_256_SHA1_32 : return LinphoneSrtpSuiteAES256CMHMACSHA132;
		case MS_AES_256_SHA1_80 :
		case MS_AES_CM_256_SHA1_80 : return LinphoneSrtpSuiteAES256CMHMACSHA180;
		case MS_AEAD_AES_128_GCM: return LinphoneSrtpSuiteAEADAES128GCM;
		case MS_AEAD_AES_256_GCM: return LinphoneSrtpSuiteAEADAES256GCM;
		// all these cases are not supported by the linphone enumeration
		case MS_AES_128_SHA1_32_NO_AUTH :
		case MS_AES_128_SHA1_80_NO_AUTH :
		case MS_AES_128_SHA1_80_SRTP_NO_CIPHER :
		case MS_AES_128_SHA1_80_SRTCP_NO_CIPHER :
		case MS_AES_128_SHA1_80_NO_CIPHER :
		case MS_CRYPTO_SUITE_INVALID :
		default:
			return LinphoneSrtpSuiteInvalid;
	}
}

std::vector<SalSrtpCryptoAlgo> MediaSessionPrivate::generateNewCryptoKeys() const {
	L_Q();
	std::vector<SalSrtpCryptoAlgo>  cryptos;
	const bool doNotUseParams = (direction == LinphoneCallIncoming) && (state == CallSession::State::Idle);
	const MSCryptoSuite *suites = nullptr;
	std::list<MSCryptoSuite> suitesList{};
	if (doNotUseParams) {
		suites = linphone_core_get_all_supported_srtp_crypto_suites(q->getCore()->getCCore());
	} else {
		// use settings from callParams if any, otherwise get them from the core configuration
		auto callParamSrtpSuites = getParams()->getSrtpSuites();
		if (callParamSrtpSuites.empty()) {
			suites = linphone_core_get_srtp_crypto_suites_array(q->getCore()->getCCore());
		} else {
			suitesList = LinphoneSrtpSuite2MSCryptoSuite(callParamSrtpSuites);
		}
	}
	if (suites!=nullptr) {
		for (size_t j = 0; (suites != nullptr) && (suites[j] != MS_CRYPTO_SUITE_INVALID); j++) {
			suitesList.push_back(suites[j]);
		}
	}

	auto cryptoId = linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "crypto_suite_tag_starting_value", 1);
	unsigned int cryptoTag = 0;
	// crypto tag lower than 1 is not valid
	if (cryptoId < 1) {
		lWarning() << "Trying to set initial value of the crypto tag suite to a value lower than 1: " << cryptoId << ". Automatically fixing it by setting it to 1";
		cryptoTag = 1;
	} else {
		cryptoTag = static_cast<unsigned int>(cryptoId);
	}
	for (auto suite:suitesList) {
		if (doNotUseParams || !isEncryptionMandatory() || (isEncryptionMandatory() && !ms_crypto_suite_is_unencrypted(suite))) {
			SalSrtpCryptoAlgo newCrypto;
			setupEncryptionKey(newCrypto, suite, cryptoTag);
			cryptos.push_back(newCrypto);
			cryptoTag++;
		} else if (isEncryptionMandatory() && ms_crypto_suite_is_unencrypted(suite)) {
			lWarning() << "Not offering " << std::string(ms_crypto_suite_to_string(suite)) << " because either RTP or RTCP streams is not encrypted";
		} else {
			lWarning() << "Not offering " << std::string(ms_crypto_suite_to_string(suite));
		}
	}

	return cryptos;
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
	const std::shared_ptr<SalMediaDescription> & md = localIsOfferer ? op->getLocalMediaDescription() : op->getRemoteMediaDescription();
	const auto audioStreamIndex = md->findIdxBestStream(SalAudio);
	Stream *stream = audioStreamIndex != -1 ? getStreamsGroup().getStream(audioStreamIndex) : nullptr;
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


/*
 * Frees the media resources of the call.
 * This has to be done at the earliest, unlike signaling resources that sometimes need to be kept a bit more longer.
 * It is called by setTerminated() (for termination of calls signaled to the application), or directly by the destructor of the session
 * if it was never notified to the application.
 */
void MediaSessionPrivate::freeResources () {
	getStreamsGroup().finish();
}

void MediaSessionPrivate::queueIceCompletionTask(const std::function<LinphoneStatus()> &lambda){
	iceDeferedCompletionTasks.push(lambda);
}

void MediaSessionPrivate::runIceCompletionTasks(){
	L_Q();
	while(!iceDeferedCompletionTasks.empty()){
		const auto task = iceDeferedCompletionTasks.front();
		LinphoneStatus result = task();
		iceDeferedCompletionTasks.pop();
		if (result != 0) {
			q->addPendingAction(task);
		}
	}
}
void MediaSessionPrivate::queueIceGatheringTask(const std::function<LinphoneStatus()> &lambda){
	iceDeferedGatheringTasks.push(lambda);
}

void MediaSessionPrivate::runIceGatheringTasks(){
	L_Q();
	while(!iceDeferedGatheringTasks.empty()){
		const auto task = iceDeferedGatheringTasks.front();
		LinphoneStatus result = task();
		iceDeferedGatheringTasks.pop();
		if (result != 0) {
			q->addPendingAction(task);
		}
	}
}

bool MediaSessionPrivate::isUpdateSentWhenIceCompleted() const {
	L_Q();

	const auto core = q->getCore()->getCCore();
	// In case of DTLS, the update is not sent after ICE completed due to interopability issues with webRTC
	return (getNegotiatedMediaEncryption() == LinphoneMediaEncryptionDTLS) ? linphone_config_get_bool(linphone_core_get_config(core), "sip", "update_call_when_ice_completed_with_dtls", false) : !!linphone_config_get_int(linphone_core_get_config(core), "sip", "update_call_when_ice_completed", true);
}

/*
 * IceServiceListener implementation
 */
void MediaSessionPrivate::onGatheringFinished(UNUSED(IceService &service)){
	lInfo() << "Finished gathering candidates";
	runIceGatheringTasks();
}

void MediaSessionPrivate::onIceCompleted(UNUSED(IceService &service)){
	L_Q();

	/* The ICE session has succeeded, so perform a call update */
	if (!getStreamsGroup().getIceService().hasCompletedCheckList()) return;
	if (getStreamsGroup().getIceService().isControlling() && isUpdateSentWhenIceCompleted()) {
		switch (state){
			case CallSession::State::StreamsRunning:
			case CallSession::State::Paused:
			case CallSession::State::PausedByRemote:
			{
				MediaSessionParams newParams(*getParams());
				newParams.getPrivate()->setInternalCallUpdate(true);
				q->update(&newParams, CallSession::UpdateMethod::Default, q->isCapabilityNegotiationEnabled());
			}
			break;
			default:
				lWarning() << "Cannot send reINVITE for ICE during state " << state;
			break;
		}
	}
	runIceCompletionTasks();
}

void MediaSessionPrivate::onLosingPairsCompleted(UNUSED(IceService &service)){
	if (state == CallSession::State::UpdatedByRemote) {
		if (incomingIceReinvitePending){
			lInfo() << "Finished adding losing pairs, ICE re-INVITE can be answered.";
			startAcceptUpdate(prevState, Utils::toString(prevState));
			incomingIceReinvitePending = false;
		}
	}
}

void MediaSessionPrivate::onIceRestartNeeded(UNUSED(IceService &service)){
	L_Q();
	getStreamsGroup().getIceService().restartSession(IR_Controlling);
	MediaSessionParams newParams(*getParams());
	q->update(&newParams, CallSession::UpdateMethod::Default, q->isCapabilityNegotiationEnabled());
}

void MediaSessionPrivate::tryEarlyMediaForking (std::shared_ptr<SalMediaDescription> & md) {
	OfferAnswerContext ctx;
	ctx.localMediaDescription = op->getLocalMediaDescription();
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
	const auto localMediaDesc = op->getLocalMediaDescription();
	for (size_t i = 0; i < result->streams.size(); i++) {
		if (i < localMediaDesc->streams.size()) {
			updateStreamFrozenPayloads(result->streams[i], localMediaDesc->streams[i]);
		} else {
			lError() << "Local media description has " << localMediaDesc->streams.size() << " whereas result has " << result->streams.size();
		}
	}
}

void MediaSessionPrivate::updateStreams (std::shared_ptr<SalMediaDescription> & newMd, CallSession::State targetState) {
	L_Q();

	if (!newMd) {
		lError() << "updateStreams() called with null media description";
		return;
	}

	updateBiggestDesc(localDesc);
	resultDesc = newMd;

	// Encryption may have changed during the offer answer process and not being the default one. Typical example of this scenario is when capability negotiation is enabled and if ZRTP is only enabled on one side and the other side supports it
	if (newMd->isEmpty()) {
		lInfo() << "All streams have been rejected, hence negotiated media encryption keeps being " << linphone_media_encryption_to_string(negotiatedEncryption);
	} else {
		negotiatedEncryption = getEncryptionFromMediaDescription(newMd);
		lInfo() << "Negotiated media encryption is " << linphone_media_encryption_to_string(negotiatedEncryption);
		// There is no way to signal that ZRTP was enabled in the SDP and it is automatically accepted by Linphone even if it was not offered in the first place
		// Attribute zrtp-hash is not mandatory
		if (!q->isCapabilityNegotiationEnabled() && (negotiatedEncryption == LinphoneMediaEncryptionZRTP) && (getParams()->getMediaEncryption() == LinphoneMediaEncryptionNone)) {
			getParams()->setMediaEncryption(negotiatedEncryption);
		}
	}
	
	/* Notify the tone manager that we're about to transition to a future state.
	 * This is important so that it can take stop pending tones, so that there is no audio resource conflict
	 * with the audio stream that is about to be started. */
	q->getCore()->getPrivate()->getToneManager().prepareForNextState(q->getSharedFromThis(), targetState);

	OfferAnswerContext ctx;
	ctx.localMediaDescription = localDesc;
	ctx.remoteMediaDescription = op->getRemoteMediaDescription();
	ctx.resultMediaDescription = resultDesc;
	ctx.localIsOfferer = localIsOfferer;
	getStreamsGroup().render(ctx, targetState);

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
	if (getNegotiatedMediaEncryption() == LinphoneMediaEncryptionDTLS) {
		lInfo() << "Forced encryption mandatory on CallSession [" << q << "] due to SRTP-DTLS";
		return true;
	}
	return getParams()->mandatoryMediaEncryptionEnabled();
}

void MediaSessionPrivate::propagateEncryptionChanged () {
	L_Q();

	string authToken = getStreamsGroup().getAuthenticationToken();
	LinphoneConference * conference = nullptr;
	if (listener) {
		conference = listener->getCallSessionConference(q->getSharedFromThis());
	}
	// If the media session is part of a conference, the client has no way to check the token, hence do not pass it on to the application
	string callbackAuthToken = (conference) ? std::string() : authToken;

	if (callbackAuthToken.empty() && !authToken.empty()) {
		getStreamsGroup().setAuthTokenVerified(true);
	}
	bool authTokenVerified = getStreamsGroup().getAuthenticationTokenVerified();
	if (!getStreamsGroup().allStreamsEncrypted()) {
		lInfo() << "Some streams are not encrypted";
		getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
		if (listener) {
			listener->onEncryptionChanged(q->getSharedFromThis(), false, callbackAuthToken);
		}
	} else {
		if (!authToken.empty()) {
			/* ZRTP only is using auth_token */
			getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionZRTP);
			auto encryptionEngine = q->getCore()->getEncryptionEngine();
			if (encryptionEngine && authTokenVerified) {
				const SalAddress *remoteAddress = getOp()->getRemoteContactAddress();
				if (remoteAddress){
					char *peerDeviceId = sal_address_as_string_uri_only(remoteAddress);
					Stream *stream = getStreamsGroup().lookupMainStream(SalAudio);
					if (stream){
						MS2Stream *ms2s = dynamic_cast<MS2Stream*>(stream);
						if (ms2s){
							encryptionEngine->authenticationVerified(ms2s->getZrtpContext(), op->getRemoteMediaDescription(), peerDeviceId);
						}else{
							lError() << "Could not dynamic_cast to MS2Stream in propagateEncryptionChanged().";
						}
					}
					ms_free(peerDeviceId);
				}else {
					/* This typically happens if the ZRTP session starts during early-media when receiving a 183 response.
					 * Indeed the Contact header is not mandatory in 183 (and liblinphone does not set it). */
					lError() << "EncryptionEngine cannot be notified of verified status because remote contact address is unknown.";
				}
			}
		} else {
			/* Otherwise it must be DTLS as SDES doesn't go through this function */
			getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionDTLS);
		}

		lInfo() << "All streams are encrypted, key exchanged using "
			<< ((q->getCurrentParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP) ? "ZRTP"
				: (q->getCurrentParams()->getMediaEncryption() == LinphoneMediaEncryptionDTLS) ? "DTLS" : "Unknown mechanism");
		if (listener) {
			listener->onEncryptionChanged(q->getSharedFromThis(), true, callbackAuthToken);
		}

		Stream *videoStream = getStreamsGroup().lookupMainStream(SalVideo);
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
	stopStreams();
	CallSessionPrivate::abort(errorMsg);
}

void MediaSessionPrivate::handleIncomingReceivedStateInIncomingNotification () {
	L_Q();

	/* Try to be best-effort in giving real local or routable contact address for 100Rel case */
	setContactOp();
	if (notifyRinging) {
		bool proposeEarlyMedia = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "incoming_calls_early_media", false);
		if (proposeEarlyMedia)
			q->acceptEarlyMedia();
		else if (state != CallSession::State::IncomingEarlyMedia) {
			op->notifyRinging(false, linphone_core_get_tag_100rel_support_level(q->getCore()->getCCore()));
		}
	}

	acceptOrTerminateReplacedSessionInIncomingNotification();
}


bool MediaSessionPrivate::canSoundResourcesBeFreed() const {
	return ((state == CallSession::State::StreamsRunning) || (state == CallSession::State::PausedByRemote));
}

LinphoneStatus MediaSessionPrivate::pause () {
	L_Q();
	if (state == CallSession::State::Paused) {
		lWarning() << "Media session (local address " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") is in state " << Utils::toString(state) << " is already paused";
		return 0;
	} else if (state == CallSession::State::Pausing) {
		lWarning() << "Media session (local address " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") is in state " << Utils::toString(state) << " is already in the process of being paused";
		return 0;
	} else if (!canSoundResourcesBeFreed()) {
		lWarning() << "Media session (local address " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") is in state " << Utils::toString(state) << " hence it cannot be paused";
		return -1;
	}

	bool isInLocalConference = getParams()->getPrivate()->getInConference();
	if (isInLocalConference) {
		char * contactAddressStr = NULL;
		const auto account = linphone_core_lookup_known_account(q->getCore()->getCCore(), L_GET_C_BACK_PTR(&(q->getLocalAddress())));
		if (op && op->getContactAddress()) {
			contactAddressStr = sal_address_as_string(op->getContactAddress());
		} else if (account && Account::toCpp(account)->getOp()) {
			contactAddressStr = sal_address_as_string(Account::toCpp(account)->getOp()->getContactAddress());
		} else {
			contactAddressStr = ms_strdup(linphone_core_get_identity(q->getCore()->getCCore()));
		}
		Address contactAddress(contactAddressStr);
		ms_free(contactAddressStr);

		if (!!linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "misc", "conference_event_log_enabled", TRUE) && contactAddress.hasParam("isfocus")) {
			if (listener) {
				auto callConference = listener->getCallSessionConference(q->getSharedFromThis());
				if (callConference) {
					auto conference = MediaConference::Conference::toCpp(callConference)->getSharedFromThis();
					if (conference->findParticipantDevice(q->getSharedFromThis())) {
						lWarning() << "Unable to pause media session (local address " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") because it is part of a conference. Please use the dedicated conference API to execute the desired actions";
						return -1;
					}
				} else {
					lWarning() << "The contact address " << contactAddress << " of the call has isfocus attribute however it doesn't seems to be part of a conference.";
				}
			}
		}

		params->getPrivate()->setInConference(false);
		q->updateContactAddress (contactAddress);
		op->setContactAddress(contactAddress.getInternalAddress());

		if (listener) {
			auto callConference = listener->getCallSessionConference(q->getSharedFromThis());
			if (callConference) {
				auto conference = MediaConference::Conference::toCpp(callConference)->getSharedFromThis();
				lInfo() << "Removing participant with session " << q << " (local addres " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ")  from conference " << conference->getConferenceAddress();
				// Do not preserve conference after removing the participant
				conference->removeParticipant(q->getSharedFromThis(), false);
				return 0;
			}
		}
	}

	string subject;

	LinphoneConference * conference = nullptr;
	if (listener) {
		conference = listener->getCallSessionConference(q->getSharedFromThis());
	}
	if (!conference) {
		if (resultDesc->hasDir(SalStreamSendRecv)) {
			subject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::CallOnHold);
		} else if (resultDesc->hasDir(SalStreamRecvOnly)
					 || (resultDesc->hasDir(SalStreamInactive) && state == CallSession::State::PausedByRemote)) {	// Stream is inactive from Remote
			subject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::BothPartiesOnHold);
		} else {
			lError() << "No reason to pause this call, it is already paused or inactive";
			return -1;
		}
	}
	broken = false;
	stopStreams();
	setState(CallSession::State::Pausing, "Pausing call");
	
	auto retryableAction = [this, subject](){
		makeLocalMediaDescription(true, false, true);
		op->update(subject.c_str(), false);
	};
	op->setRetryFunction(retryableAction);
	retryableAction();

	shared_ptr<Call> currentCall = q->getCore()->getCurrentCall();
	// Reset current session if we are pausing the current call
	if (listener && (!currentCall || (currentCall->getActiveSession() == q->getSharedFromThis())))
		listener->onResetCurrentSession(q->getSharedFromThis());

	return 0;
}

int MediaSessionPrivate::restartInvite () {
	L_Q();
	stopStreams();
	getStreamsGroup().clearStreams();
	// Clear streams resets the ICE service therefore the pointer to its listener is now NULL and it must be set asap in order to receive events.
	getStreamsGroup().getIceService().setListener(this);
	makeLocalMediaDescription(true, q->isCapabilityNegotiationEnabled(), false);
	const auto defer = CallSessionPrivate::restartInvite();
	if (!defer) {
		q->startInvite(nullptr, op->getSubject(), nullptr);
	}
	return defer;
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
	getCurrentParams()->getPrivate()->setInConference(getParams()->getPrivate()->getInConference());

	return 0;
}

LinphoneStatus MediaSessionPrivate::startUpdate (const CallSession::UpdateMethod method, const string &subject) {
	L_Q();

	const bool doNotAddSdpToInvite = q->getCore()->getCCore()->sip_conf.sdp_200_ack && !getParams()->getPrivate()->getInternalCallUpdate();
	if (doNotAddSdpToInvite) {
		op->setLocalMediaDescription(nullptr);
	}
	LinphoneStatus result = CallSessionPrivate::startUpdate(method, subject);
	op->setRetryFunction( [this, subject, method](){
		this->startUpdate(method, subject);
	});

	if (doNotAddSdpToInvite) {
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
	localIsTerminator = true;
	CallSessionPrivate::terminate();
}

LinphoneMediaDirection MediaSessionPrivate::getVideoDirFromMd (const std::shared_ptr<SalMediaDescription> & md) const {
	L_Q();
	LinphoneConference * conference = nullptr;
	if (listener) {
		conference = listener->getCallSessionConference(const_pointer_cast<CallSession>(q->getSharedFromThis()));
	}
	if (conference) {
		const auto cppConference = MediaConference::Conference::toCpp(conference)->getSharedFromThis();
		const auto hasVideoSendRecvStream = (md->containsStreamWithDir(SalStreamSendRecv, SalVideo));
		const auto hasVideoSendOnlyStream = (md->containsStreamWithDir(SalStreamSendOnly, SalVideo));
		const auto hasVideoRecvOnlyStream = (md->containsStreamWithDir(SalStreamRecvOnly, SalVideo));
		if (hasVideoSendRecvStream || (hasVideoSendOnlyStream && hasVideoRecvOnlyStream)) {
			return LinphoneMediaDirectionSendRecv;
		} else if (hasVideoSendOnlyStream && !hasVideoRecvOnlyStream) {
			return LinphoneMediaDirectionSendOnly;
		} else if (!hasVideoSendOnlyStream && hasVideoRecvOnlyStream) {
			return LinphoneMediaDirectionRecvOnly;
		} else {
			return LinphoneMediaDirectionInactive;
		}
	}
	const auto &videoStream = md->findBestStream(SalVideo);
	return MediaSessionParamsPrivate::salStreamDirToMediaDirection(videoStream.getDirection());
}

int MediaSessionPrivate::getMainVideoStreamIdx(const std::shared_ptr<SalMediaDescription> & md) const {
	L_Q();
	// In order to set properly the negotiated parameters, we must know if the client is sending video to the conference, i.e. look at the thumbnail stream direction. In order to do so, we must know the label of the searched thumbnail stream.
	// The local case is quite straightforward because all labels are known, but for the client is more difficult as the NOTIFY message may have not come or been processed.
	// The algorithm below searches for the label in the main stream and then reuses the label to look for the desired thumbnail stream
	auto streamIdx = -1;
	if (md) {
		LinphoneConference * conference = listener ? listener->getCallSessionConference(const_pointer_cast<CallSession>(q->getSharedFromThis())) : nullptr;
		if (conference && op) {
			const bool isInLocalConference = getParams()->getPrivate()->getInConference();
			const auto & confLayout = MediaSession::computeConferenceLayout(isInLocalConference ? op->getRemoteMediaDescription() : op->getLocalMediaDescription());
			const auto isConferenceLayoutActiveSpeaker = (confLayout == ConferenceLayout::ActiveSpeaker);
			const auto mainStreamAttrValue = isConferenceLayoutActiveSpeaker ? "speaker" : "main";
			streamIdx = md->findIdxStreamWithContent(mainStreamAttrValue);
		} else  {
			streamIdx = md->findIdxBestStream(SalVideo);
		}
	}
	return streamIdx;
}

int MediaSessionPrivate::getThumbnailStreamIdx(const std::shared_ptr<SalMediaDescription> & md) const {
	L_Q();
	// In order to set properly the negotiated parameters, we must know if the client is sending video to the conference, i.e. look at the thumbnail stream direction. In order to do so, we must know the label of the searched thumbnail stream.
	// The local case is quite straightforward because all labels are known, but for the client is more difficult as the NOTIFY message may have not come or been processed.
	// The algorithm below searches for the label in the main stream and then reuses the label to look for the desired thumbnail stream
	auto streamIdx = -1;
	if (md) {
		auto mainStreamIdx = getMainVideoStreamIdx(md);
		LinphoneConference * conference = listener ? listener->getCallSessionConference(const_pointer_cast<CallSession>(q->getSharedFromThis())) : nullptr;
		if (conference && op && (mainStreamIdx >= 0)) {
			const SalStreamDescription & mainVideoStream = md->getStreamIdx(static_cast<unsigned int>(mainStreamIdx));
			const auto cppConference = MediaConference::Conference::toCpp(conference)->getSharedFromThis();
			const auto meDevices = cppConference->getMe()->getDevices();
			const bool conferenceEventPackageEnabled = linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "misc", "conference_event_log_enabled", TRUE);
			const bool isInLocalConference = getParams()->getPrivate()->getInConference();
			// Devices don't have labels if conference event package is not enabled
			const auto label = (!conferenceEventPackageEnabled || isInLocalConference || (meDevices.size() == 0)) ? mainVideoStream.getLabel() : meDevices.front()->getLabel();
			if (!label.empty()) {
				if (md) {
					const auto content = "thumbnail";
					streamIdx = md->findIdxStreamWithContent(content, label);
				}
			}
		}
	}
	return streamIdx;
}

void MediaSessionPrivate::updateCurrentParams () const {
	L_Q();
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

	const std::shared_ptr<SalMediaDescription> & md = resultDesc;

	// In case capability negotiation is enabled, the actual encryption is the negotiated one
	const LinphoneMediaEncryption enc = getNegotiatedMediaEncryption();
	bool srtpEncryptionMatch = false;
	auto srtpSuite = LinphoneSrtpSuiteInvalid;
	if (md) {
		srtpEncryptionMatch = true;
		bool srtpSuiteSet = false;
		for (size_t idx = 0; idx < md->getNbStreams(); idx++) {
			const auto & salStream = md->getStreamIdx(static_cast<unsigned int>(idx));
			if (salStream.enabled() && (salStream.getDirection() != SalStreamInactive) && salStream.hasSrtp()) {
				const auto & streamCryptos = salStream.getCryptos();
				const auto & stream = getStreamsGroup().getStream(idx);
				for (const auto & crypto : streamCryptos) {
					const auto & algo = crypto.algo;
					if (isEncryptionMandatory()) {
						srtpEncryptionMatch &= !ms_crypto_suite_is_unencrypted(algo) && stream->isEncrypted();
					} else {
						srtpEncryptionMatch &= ((ms_crypto_suite_is_unencrypted(algo)) ? !stream->isEncrypted() : stream->isEncrypted());
					}

					// To have a valid SRTP suite in the current call params, all streams must be encrypted and use the same suite
					// TODO: get the stream status and SRTP encryption from mediastreamer so we can get the suite even when using ZRTP or DTLS as key exchange
					if (srtpSuiteSet) {
						if (srtpSuite != MSCryptoSuite2LinphoneSrtpSuite(algo)) {
							srtpSuite = LinphoneSrtpSuiteInvalid;
						}
					} else {
						srtpSuiteSet = true;
						srtpSuite = MSCryptoSuite2LinphoneSrtpSuite(algo);
					}
				}
			} else { // No Srtp on this stream -> srtpSuite is set to invalid
				srtpSuite = LinphoneSrtpSuiteInvalid;
				srtpSuiteSet = true;
			}
		}
		getCurrentParams()->enableRtpBundle(!md->bundles.empty());
	} else {
		srtpEncryptionMatch = allStreamsEncrypted();
	}

	getCurrentParams()->setSrtpSuites(std::list<LinphoneSrtpSuite>{srtpSuite});

	bool updateEncryption = false;
	bool validNegotiatedEncryption = false;

	switch (enc) {
		case LinphoneMediaEncryptionZRTP:
			updateEncryption = atLeastOneStreamStarted();
			validNegotiatedEncryption = (allStreamsEncrypted() && !authToken.empty());
			break;
		case LinphoneMediaEncryptionSRTP:
			updateEncryption = atLeastOneStreamStarted();
			validNegotiatedEncryption = ((getNbActiveStreams() == 0) || srtpEncryptionMatch);
			break;
		case LinphoneMediaEncryptionDTLS:
			updateEncryption = atLeastOneStreamStarted();
			validNegotiatedEncryption = ((getNbActiveStreams() == 0) || allStreamsEncrypted());
			break;
		case LinphoneMediaEncryptionNone:
			updateEncryption = true;
			validNegotiatedEncryption = true;
			break;
	}

	if (updateEncryption) {
		if (validNegotiatedEncryption) {
			getCurrentParams()->setMediaEncryption(enc);
		} else {
			/* To avoid too many traces */
			lDebug() << "Encryption was requested to be " << linphone_media_encryption_to_string(enc)
				<< ", but isn't effective (allStreamsEncrypted=" << allStreamsEncrypted() << ", auth_token=" << authToken << ")";
			getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
		}
	} /* else don't update the state if all streams are shutdown */

	if (md) {
		getCurrentParams()->enableAvpf(hasAvpf(md));
		if (getCurrentParams()->avpfEnabled()) {
			getCurrentParams()->setAvpfRrInterval(getAvpfRrInterval());
		} else {
			getCurrentParams()->setAvpfRrInterval(0);
		}
		const SalStreamDescription &audioStream = md->findBestStream(SalAudio);
		if (audioStream != Utils::getEmptyConstRefObject<SalStreamDescription>()){
			getCurrentParams()->setAudioDirection(audioStream.getDirection());
			if (getCurrentParams()->getAudioDirection() != LinphoneMediaDirectionInactive) {
				const std::string & rtpAddr = (audioStream.getRtpAddress().empty() == false) ? audioStream.getRtpAddress() : md->addr;
				getCurrentParams()->enableAudioMulticast(!!ms_is_multicast(rtpAddr.c_str()));
			} else
				getCurrentParams()->enableAudioMulticast(false);
			getCurrentParams()->enableAudio(audioStream.enabled());
		} else {
			getCurrentParams()->setAudioDirection(LinphoneMediaDirectionInactive);
			getCurrentParams()->enableAudioMulticast(false);
			getCurrentParams()->enableAudio(false);
		}

		const auto streamIdx = getThumbnailStreamIdx(md);
		const auto &videoStream = (streamIdx == -1) ? md->findBestStream(SalVideo) : md->getStreamIdx(static_cast<unsigned int>(streamIdx));
		if (videoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()){
			getCurrentParams()->getPrivate()->enableImplicitRtcpFb(videoStream.hasImplicitAvpf());

			const auto videoDirection = getVideoDirFromMd(md);
			getCurrentParams()->setVideoDirection(videoDirection);

			if (getCurrentParams()->getVideoDirection() != LinphoneMediaDirectionInactive) {
				const std::string & rtpAddr = (videoStream.getRtpAddress().empty() == false) ? videoStream.getRtpAddress() : md->addr;
				getCurrentParams()->enableVideoMulticast(!!ms_is_multicast(rtpAddr.c_str()));
			} else
				getCurrentParams()->enableVideoMulticast(false);
			LinphoneConference * conference = listener ? listener->getCallSessionConference(const_pointer_cast<CallSession>(q->getSharedFromThis())) : nullptr;
			const auto enable = (conference) ? (videoDirection != LinphoneMediaDirectionInactive) : videoStream.enabled();

			getCurrentParams()->enableVideo(enable);
		} else {
			getCurrentParams()->getPrivate()->enableImplicitRtcpFb(false);
			getCurrentParams()->setVideoDirection(LinphoneMediaDirectionInactive);
			getCurrentParams()->enableVideoMulticast(false);
			getCurrentParams()->enableVideo(false);
		}

		const SalStreamDescription &textStream = md->findBestStream(SalText);
		if (textStream != Utils::getEmptyConstRefObject<SalStreamDescription>()){
			// Direction and multicast are not supported for real-time text.
			getCurrentParams()->enableRealtimeText(textStream.enabled());
		} else {
			getCurrentParams()->enableRealtimeText(false);
		}
	}
	getCurrentParams()->getPrivate()->setUpdateCallWhenIceCompleted(isUpdateSentWhenIceCompleted());
}

// -----------------------------------------------------------------------------


LinphoneStatus MediaSessionPrivate::startAccept(){
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
	// If the core in a conference, request to empty sound resources only if the call is in a different conference or the call is not part of a conference
	bool isThisNotCurrentConference = isCoreInLocalConference && (!callConference || (callConference != coreConference));

	// Try to preempt sound resources if the core is in a call or conference that are not the current ones
	if (isThisNotCurrentConference || isThisNotCurrentMediaSession) {
		if ((linphone_core_get_media_resource_mode(q->getCore()->getCCore()) == LinphoneExclusiveMediaResources) && linphone_core_preempt_sound_resources(q->getCore()->getCCore()) != 0) {
			lInfo() << "Delaying call to " << __func__ << " for media session (local addres " << q->getLocalAddress().asString() << " remote address " << q->getRemoteAddress()->asString() << ") in state " << Utils::toString(state) << " because sound resources cannot be preempted";
			q->addPendingAction([this] {
				this->startAccept();
				return 0;
			});
			return -1;
		}
	}

	// It occurs if the remote participant calls the core hosting the conference and the call is added to the conference when it is in state IncomingReceived
	if (getOp() && getOp()->getContactAddress()) {
		char * contactAddressStr = sal_address_as_string(getOp()->getContactAddress());
		Address contactAddress(contactAddressStr);
		ms_free(contactAddressStr);
		const auto & confId = getConferenceId();
		if (!confId.empty() && isInConference() && !contactAddress.hasUriParam("conf-id")) {
			q->updateContactAddress(contactAddress);
			op->setContactAddress(contactAddress.getInternalAddress());
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
		// If negotiated media description doesn't contain a video stream after the first INVITE message sequence, then disable video in the local call parameters
		if (getParams()->videoEnabled() && (newMd->findBestStream(SalVideo) == Utils::getEmptyConstRefObject<SalStreamDescription>())) {
			getParams()->enableVideo(false);
		}
		updateStreams(newMd, CallSession::State::StreamsRunning);
		setState(CallSession::State::StreamsRunning, "Connected (streams running)");
	} else
		expectMediaInAck = true;

	return 0;
}

LinphoneStatus MediaSessionPrivate::accept (const MediaSessionParams *msp, UNUSED(bool wasRinging)) {
	L_Q();
	if (msp) {
		setParams(new MediaSessionParams(*msp));
	}

	const bool isOfferer = (op->getRemoteMediaDescription() ? false : true);

	if (msp || (localDesc == nullptr)) makeLocalMediaDescription(isOfferer, q->isCapabilityNegotiationEnabled(), false);

	// If call is going to be accepted, then recreate the local media description if there is no local description or encryption is mandatory.
	// The initial INVITE sequence goes through the offer answer negotiation process twice.
	// The first one generates the 180 Ringing and it ensures that the offer can be potentially accepted upon setting of a compatible set of parameters.
	// The second offer answer negotiation is more thorough as the set of parameters to accept the call is known. In this case, if the encryption is mandatory a new local media description must be generated in order to populate the crypto keys with the set actually ued in the call
	if ((state == CallSession::State::IncomingReceived) && params) makeLocalMediaDescription(isOfferer, q->isCapabilityNegotiationEnabled(), false, true);

	updateRemoteSessionIdAndVer();

	auto acceptCompletionTask = [this](){
		updateLocalMediaDescriptionFromIce(op->getRemoteMediaDescription() == nullptr);
		return startAccept();
	};
	if (linphone_nat_policy_ice_enabled(natPolicy) && getStreamsGroup().prepare()){
		queueIceGatheringTask(acceptCompletionTask);
		return 0; /* Deferred until completion of ICE gathering */
	}
	return acceptCompletionTask();
}

LinphoneStatus MediaSessionPrivate::acceptUpdate (const CallSessionParams *csp, CallSession::State nextState, const string &stateInfo) {
	L_Q();
	const std::shared_ptr<SalMediaDescription> & desc = op->getRemoteMediaDescription();
	const bool isRemoteDescNull = (desc == nullptr);

	bool keepSdpVersion = !!linphone_config_get_int(
		linphone_core_get_config(q->getCore()->getCCore()),
		"sip",
		"keep_sdp_version",
		(op->getSal()->getSessionTimersExpire() > 0)
	);

	if (keepSdpVersion && desc && (desc->session_id == remoteSessionId) && (desc->session_ver == remoteSessionVer)) {
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
	if (getParams()->videoEnabled() && !linphone_core_video_enabled(q->getCore()->getCCore())) {
		lWarning() << "Requested video but video support is globally disabled. Refusing video";
		getParams()->enableVideo(false);
	}
	updateRemoteSessionIdAndVer();
	makeLocalMediaDescription(isRemoteDescNull, q->isCapabilityNegotiationEnabled(), false);

	auto acceptCompletionTask = [this, nextState, stateInfo, isRemoteDescNull](){
		updateLocalMediaDescriptionFromIce(isRemoteDescNull);
		startAcceptUpdate(nextState, stateInfo);
		return 0;
	};

	if (linphone_nat_policy_ice_enabled(natPolicy) && getStreamsGroup().prepare()){
		lInfo() << "Acceptance of incoming reINVITE is deferred to ICE gathering completion.";
		queueIceGatheringTask(acceptCompletionTask);
		return 0; /* Deferred until completion of ICE gathering */
	} else if (getStreamsGroup().getIceService().isRunning() && !isUpdateSentWhenIceCompleted()) {
		// ICE negotiations are ongoing hence the update cannot be accepted immediately - need to wait for the completition of ICE negotiations
		lInfo() << "acceptance of incoming reINVITE is deferred to ICE completion completion.";
		queueIceCompletionTask(acceptCompletionTask);
		return 0;
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
	selectOutgoingIpVersion();
	getStreamsGroup().getIceService().resetSession();
	MediaSessionParams newParams(*getParams());
	q->update(&newParams, CallSession::UpdateMethod::Invite, q->isCapabilityNegotiationEnabled());
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
	if (!session) {
		return -1;
	}
	return session->getPrivate()->resumeAfterFailedTransfer();
}

bool_t MediaSessionPrivate::startPendingRefer (void *userData) {
	MediaSession *session = static_cast<MediaSession *>(userData);
	if (!session) {
		return FALSE;
	}
	session->getPrivate()->startPendingRefer();
	return TRUE;
}

void MediaSessionPrivate::stunAuthRequestedCb (const char *realm, UNUSED(const char *nonce), const char **username, const char **password, const char **ha1) {
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
		*password = linphone_auth_info_get_password(authInfo);
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
		d->setParams(new MediaSessionParams(*(static_cast<const MediaSessionParams *>(params))));
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
}

// -----------------------------------------------------------------------------

ConferenceLayout MediaSession::computeConferenceLayout(const std::shared_ptr<SalMediaDescription> & md) {
	ConferenceLayout layout = ConferenceLayout::ActiveSpeaker;
	if (md) {
		if (md->findIdxStreamWithContent("main") != -1) {
			layout = ConferenceLayout::Grid;
		} else if (md->findIdxStreamWithContent("speaker") != -1) {
			layout = ConferenceLayout::ActiveSpeaker;
		} else {
			lDebug() << "Unable to deduce layout from media description " << md;
		}
	}
	return layout;
}

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

	auto ret = d->accept(msp, wasRinging);
	if (ret == 0) {
		lInfo() << "MediaSession (local address " << getLocalAddress().asString() << " remote address " << getRemoteAddress()->asString() << ") has been accepted";
	} else {
		lInfo() << "Unable to immediately accept session " << this << " (local address " << getLocalAddress().asString() << " remote address " << getRemoteAddress()->asString() << ")";
	}
	return ret;
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
		d->makeLocalMediaDescription(false, isCapabilityNegotiationEnabled(), false);
		d->op->setSentCustomHeaders(d->getParams()->getPrivate()->getCustomHeaders());
	}
	d->op->notifyRinging(true, linphone_core_get_tag_100rel_support_level(getCore()->getCCore()));
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

void MediaSession::setNatPolicy(LinphoneNatPolicy *pol){
	L_D();
	if (pol){
		linphone_nat_policy_ref(pol);
	}
	if (d->natPolicy){
		linphone_nat_policy_unref(d->natPolicy);
	}
	d->natPolicy = pol;
}

void MediaSession::enableToneIndications(bool enabled){
	L_D();
	d->toneIndicationsEnabled = enabled;
}

bool MediaSession::toneIndicationsEnabled()const{
	L_D();
	return d->toneIndicationsEnabled;
}

void MediaSession::configure (LinphoneCallDir direction, LinphoneProxyConfig *cfg, SalCallOp *op, const Address &from, const Address &to) {
	L_D();
	bool makeLocalDescription = true;
	bool isOfferer = true;
	Address remote;
	
	CallSession::configure (direction, cfg, op, from, to);
	
	if (!d->natPolicy){
		if (d->destProxy)
			d->natPolicy = linphone_proxy_config_get_nat_policy(d->destProxy);
		if (!d->natPolicy)
			d->natPolicy = linphone_core_get_nat_policy(getCore()->getCCore());
		linphone_nat_policy_ref(d->natPolicy);
	}
	
	if (direction == LinphoneCallOutgoing) {
		d->selectOutgoingIpVersion();
		isOfferer = makeLocalDescription = !getCore()->getCCore()->sip_conf.sdp_200_ack;
		remote = to;
		/* The enablement of rtp bundle is controlled at first by the Account, then the Core.
		 * Then the value is stored and later updated into MediaSessionParams. */
		bool rtpBundleEnabled = false;
		if (d->destProxy){
			rtpBundleEnabled = Account::toCpp(d->destProxy->account)->getAccountParams()->rtpBundleEnabled();
		}else{
			lInfo() << "No account set for this call, using rtp bundle enablement from LinphoneCore.";
			rtpBundleEnabled = linphone_core_rtp_bundle_enabled(getCore()->getCCore());
		}
		d->getParams()->enableRtpBundle(rtpBundleEnabled);
	} else if (direction == LinphoneCallIncoming) {
		d->selectIncomingIpVersion();
		/* Note that the choice of IP version for streams is later refined by setCompatibleIncomingCallParams() when examining the
		 * remote offer, if any. If the remote offer contains IPv4 addresses, we should propose IPv4 as well. */
		remote = from;
		remote.clean();
		d->setParams(new MediaSessionParams());
		d->params->initDefault(getCore(), LinphoneCallIncoming);
		d->initializeParamsAccordingToIncomingCallParams();
		isOfferer = op->getRemoteMediaDescription() ? false : true;
		/* For incoming calls, the bundle enablement is set according to remote call params and core's policy,
		 * in fixCallParams() */
	}
		
	lInfo() << "Rtp bundle is " << (d->getParams()->rtpBundleEnabled() ? "enabled." : "disabled.");
	
	if (makeLocalDescription){
		/* Do not make a local media description when sending an empty INVITE. */
		d->makeLocalMediaDescription(isOfferer, isCapabilityNegotiationEnabled(), false);
	}
	
	if (d->natPolicy)
		d->runStunTestsIfNeeded();
	d->discoverMtu(remote);
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
					if (d->state != State::Idle && d->state != State::PushIncomingReceived) return 0;
					d->deferIncomingNotification = false;
					d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
					d->startIncomingNotification();
					return 0;
				};
				d->queueIceGatheringTask(incomingNotificationTask);
			}else{
				d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
			}
		}
	}
}

bool MediaSession::initiateOutgoing (const string &subject, const Content *content) {
	L_D();
	bool defer = CallSession::initiateOutgoing(subject, content);
	
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
				auto toAddr = linphone_address_as_string(d->log->getToAddress());
				lInfo() << "Unable to initiate call to " << std::string(toAddr) << " because ICE candidates must be gathered first";
				ms_free(toAddr);
				d->queueIceGatheringTask([this, subject, content]() {
					L_D();
					if(d->state != CallSession::State::End) // Call has been terminated while gathering: avoid to update descriptions.
						d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
					startInvite(nullptr, subject, content);
					return 0;
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
	if (d->destProxy) {
		if (linphone_proxy_config_get_op(d->destProxy)) {
			/* Give a chance to update the contact address if connectivity has changed */
			contactAddressStr = sal_address_as_string(linphone_proxy_config_get_op(d->destProxy)->getContactAddress());
		} else if (linphone_core_conference_server_enabled(getCore()->getCCore()) && linphone_proxy_config_get_contact(d->getDestProxy())) {
			contactAddressStr = linphone_address_as_string(linphone_proxy_config_get_contact(d->getDestProxy()));
		}
	} else if (d->op && d->op->getContactAddress()) {
		contactAddressStr = sal_address_as_string(d->op->getContactAddress());
	}

	if (contactAddressStr) {
		Address contactAddress(contactAddressStr);
		ms_free(contactAddressStr);
		updateContactAddress(contactAddress);
		d->op->setContactAddress(contactAddress.getInternalAddress());
	}

	int ret = 0;

	// Do not pause call if it is already in paused by remote state
	if (d->state != CallSession::State::PausedByRemote) {
		ret = pause();
	}

	return ret;
}

LinphoneStatus MediaSession::pause () {
	L_D();
	d->pausedByApp = true;
	LinphoneStatus result = d->pause();
	if (result != 0)
		d->pausedByApp = false;
	return result;
}

LinphoneStatus MediaSession::delayResume() {
	lInfo() << "Delaying call resume";
	addPendingAction([this] {return this->resume();});
	return -1;
}

LinphoneStatus MediaSession::resume () {
	L_D();
	if (d->state == CallSession::State::Pausing) {
		lInfo() << "Call is currently in state " << Utils::toString(d->state) << " and cannot be immediately resumed therefore this task will be scheduled";
		addPendingAction([this] {return this->resume();});
		return -1;
	} else if (d->state != CallSession::State::Paused) {
		lWarning() << "we cannot resume a call that has not been established and paused before. Current state: " << Utils::toString(d->state);
		return -1;
	}
	if (!d->getParams()->getPrivate()->getInConference()) {
		if (linphone_core_sound_resources_locked(getCore()->getCCore())) {
			lWarning() << "Cannot resume MediaSession " << this << " because another call is locking the sound resources";
			return -1;
		}
		if (linphone_core_preempt_sound_resources(getCore()->getCCore()) != 0) {
			lInfo() << "Delaying call to " << __func__ << " because sound resources cannot be preempted";
			addPendingAction([this] {
				return this->resume();
			});
			return -1;
		}

	}

	lInfo() << "Resuming MediaSession " << this;
	d->pausedByApp = false;
	d->automaticallyPaused = false;
	d->broken = false;
	/* Stop playing music immediately. If remote side is a conference it
	 * prevents the participants to hear it while the 200OK comes back. */
	Stream *as = d->getStreamsGroup().lookupMainStream(SalAudio);
	if (as) as->stop();
	
	string subject = "Call resuming";
	if (d->getParams()->getPrivate()->getInConference() && !getCurrentParams()->getPrivate()->getInConference()) {
		subject = "Conference";
	}
	char * contactAddressStr = nullptr;
	if (d->destProxy && linphone_proxy_config_get_op(d->destProxy)) {
		contactAddressStr = sal_address_as_string(linphone_proxy_config_get_op(d->destProxy)->getContactAddress());
	} else if (d->op && d->op->getContactAddress()) {
		contactAddressStr = sal_address_as_string(d->op->getContactAddress());
	}
	if (contactAddressStr) {
		Address contactAddress(contactAddressStr);
		ms_free(contactAddressStr);
		updateContactAddress(contactAddress);
		d->op->setContactAddress(contactAddress.getInternalAddress());
	}

	const auto isIceRunning = getStreamsGroup().getIceService().isRunning();

	auto retryableAction = [this, isIceRunning, subject]() -> int{
		L_D();
		auto updateCompletionTask = [this, subject]() -> int{
			L_D();

			CallSession::State previousState = d->state;
			// The state must be set before recreating the media description in order for the method forceStreamsDirAccordingToState (called by makeLocalMediaDescription) to set the stream directions accordingly
			d->setState(CallSession::State::Resuming, "Resuming");

			d->makeLocalMediaDescription(true, false, true);

			if (getCore()->getCCore()->sip_conf.sdp_200_ack) {
				d->op->setLocalMediaDescription(nullptr);
			} else {
				d->op->setLocalMediaDescription(d->localDesc);
			}

			const auto res = d->op->update(subject.c_str(), false);

			if (getCore()->getCCore()->sip_conf.sdp_200_ack) {
				/* We are NOT offering, set local media description after sending the call so that we are ready to
				* process the remote offer when it will arrive. */
				d->op->setLocalMediaDescription(d->localDesc);
			}

			if (res != 0) {
				d->setState(previousState, "Restore initial state");
				return -1;
			}

			return 0;
		};

		const auto preparingStreams = d->getStreamsGroup().prepare();
		if (linphone_nat_policy_ice_enabled(d->natPolicy) && preparingStreams) {
			lInfo() << "Defer CallSession " << this << " (local address " << getLocalAddress().asString() << " remote address " << getRemoteAddress()->asString() << ") resume to gather ICE candidates";
			d->queueIceGatheringTask(updateCompletionTask);
			return 0;
		} else if (isIceRunning) {
			// ICE negotiations are ongoing hence the update cannot be send right now
			lInfo() << "Ice negotiations are ongoing and resume once they complete, therefore defer CallSession " << this << " (local address " << getLocalAddress().asString() << " remote address " << getRemoteAddress()->asString() << ") resume until Ice negotiations are completed.";
			d->queueIceCompletionTask(updateCompletionTask);
			return 0;
		}
		return updateCompletionTask();
	};
	d->op->setRetryFunction(retryableAction);
	if (retryableAction() == -1) return -1;

	if (!d->getParams()->getPrivate()->getInConference() && d->listener)
		d->listener->onSetCurrentSession(getSharedFromThis());

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

	if (md && (md->isEmpty() || d->incompatibleSecurity(md))) {
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, nullptr, LinphoneReasonNotAcceptable, 488, "Not acceptable here", nullptr);
		/* When call state is PushIncomingReceived, not notify early failed.
		   Because the call is already added in core, need to be released. */
		if (d->state != CallSession::State::PushIncomingReceived &&  d->listener)
			d->listener->onCallSessionEarlyFailed(getSharedFromThis(), ei);
		d->op->decline(SalReasonNotAcceptable);
		return;
	}

	CallSession::startIncomingNotification(notifyRinging);
}

int MediaSession::getRandomRtpPort (const SalStreamDescription & stream) const {
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
		return 0;
	} else if (portRange.second == portRange.first) {
		lWarning() << "Port range provided for stream type " << Utils::toString(stream.type) << " has minimum and maximum value set to " << portRange.first << ". It will not be possible to have multiple streams of the same type in the SDP";
	}
	const int rtp_port = (portRange.second == portRange.first) ? portRange.first : ((rand() % abs(portRange.second - portRange.first)) + portRange.first);
	if ((rtp_port > portRange.second) && (rtp_port < portRange.first)) {
		lWarning() << "The chosen port " << rtp_port << " is not within the desired range (min=" << portRange.first << ", max=" << portRange.second << ")";
	}

	return rtp_port;
}

int MediaSession::startInvite (const Address *destination, const string &subject, const Content *content) {
	L_D();

	if (d->getOp() == nullptr) d->createOp();
	linphone_core_stop_dtmf_stream(getCore()->getCCore());
	if (getCore()->getCCore()->sound_conf.play_sndcard && getCore()->getCCore()->sound_conf.capt_sndcard) {
		/* Give a chance to set card prefered sampling frequency */
		if (d->localDesc && (d->localDesc->streams.size() > 0) && (d->localDesc->streams[0].getMaxRate() > 0))
			ms_snd_card_set_preferred_sample_rate(getCore()->getCCore()->sound_conf.play_sndcard, d->localDesc->streams[0].getMaxRate());
		d->getStreamsGroup().prepare();
	}

	if (d->localDesc) {
		for (auto & stream : d->localDesc->streams) {
			// In case of multicasting, choose a random port to send with the invite
			if (ms_is_multicast(L_STRING_TO_C(stream.rtp_addr))){
				const auto rtp_port = getRandomRtpPort(stream);
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

void MediaSession::setRecordPath(const std::string &path) {
	L_D();
	d->getParams()->setRecordFilePath(path);
	AudioControlInterface * i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i != nullptr) i->setRecordPath(path);
	lInfo() << "MediaSession " << this << " set record file path " << path;
}

bool MediaSession::startRecording () {
	L_D();
	if (d->getParams()->getRecordFilePath().empty()) {
		lError() << "MediaSession::startRecording(): no output file specified. Use MediaSessionParams::setRecordFilePath()";
		return false;
	}
	AudioControlInterface * i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i != nullptr) return i->startRecording();

	return false;
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
	return update(msp, CallSession::UpdateMethod::Default, false, subject);
}

LinphoneStatus MediaSession::update (const MediaSessionParams *msp, const UpdateMethod method, const bool isCapabilityNegotiationUpdate, const string &subject) {
	L_D();
	CallSession::State nextState;
	LinphoneStatus result = 0;
	if (!d->isUpdateAllowed(nextState)) {
		return -1;
	}

	bool noUserConsent = getMediaParams()->getPrivate()->getNoUserConsent();
	if (method != CallSession::UpdateMethod::Default) {
		noUserConsent = method == CallSession::UpdateMethod::Update;
	}

	if (!d->op->canSendRequest(noUserConsent, true)) {
		return -1;
	}

	if (d->getCurrentParams() == msp)
		lWarning() << "MediaSession::update() is given the current params, this is probably not what you intend to do!";

	if (msp) {
		// The state must be set before recreating the media description in order for the method forceStreamsDirAccordingToState (called by makeLocalMediaDescription) to set the stream directions accordingly
		d->setState(nextState, "Updating call");
		d->broken = false;
		d->setParams(new MediaSessionParams(*msp));

		LinphoneConference * conference = nullptr;
		if (d->listener) {
			conference = d->listener->getCallSessionConference(getSharedFromThis());
		}

		if (conference) {
			const auto cppConference = MediaConference::Conference::toCpp(conference)->getSharedFromThis();
			const auto & conferenceState = cppConference->getState();
			if ((conferenceState != ConferenceInterface::State::Instantiated) && (conferenceState != ConferenceInterface::State::CreationPending) && (!d->getParams()->rtpBundleEnabled())) {
				lInfo() << "Forcing RTP bundle in Media session (local address " << getLocalAddress() << " remote address " << getRemoteAddress()->asString() << ") was added to conference " << cppConference->getConferenceAddress();
				d->getParams()->enableRtpBundle(true);
			}
		}

		const auto isIceRunning = getStreamsGroup().getIceService().isRunning();
		// Add capability negotiation attributes if capability negotiation is enabled and it is not a reINVITE following conclusion of the capability negotiation procedure
		bool addCapabilityNegotiationAttributesToLocalMd = isCapabilityNegotiationEnabled() && !isCapabilityNegotiationUpdate;
		bool isCapabilityNegotiationReInvite = isCapabilityNegotiationEnabled() && isCapabilityNegotiationUpdate;
		bool isOfferer = isCapabilityNegotiationUpdate || !getCore()->getCCore()->sip_conf.sdp_200_ack;
		d->localIsOfferer = isOfferer;
		d->makeLocalMediaDescription(d->localIsOfferer, addCapabilityNegotiationAttributesToLocalMd, isCapabilityNegotiationReInvite);
		const auto & localDesc = d->localDesc;

		auto updateCompletionTask = [this, method, subject, localDesc]() -> LinphoneStatus{
			L_D();

			CallSession::State previousState = d->state;
			CallSession::State newState;
			if (!d->isUpdateAllowed(newState)) {
				return -1;
			}

			// Do not set state calling setState because we shouldn't call the callbacks as it may trigger an infinite loop.
			// For example, at every state change, the core tries to run pending tasks for each and every session and this may lead to queue the same action multiple times
			if (d->state != newState) {
				d->state = newState;
			}

			// We may running this code after ICE candidates have been gathered or ICE released task completed, therefore the local description must be updated to include ICE candidates for every stream
			const auto currentLocalDesc = d->localDesc;
			d->localDesc = localDesc;
			d->updateLocalMediaDescriptionFromIce(!getCore()->getCCore()->sip_conf.sdp_200_ack);

			if (getCore()->getCCore()->sip_conf.sdp_200_ack) {
				d->op->setLocalMediaDescription(nullptr);
			} else {
				d->op->setLocalMediaDescription(d->localDesc);
			}
			LinphoneStatus res = d->startUpdate(method, subject);

			d->localDesc = currentLocalDesc;
			if (getCore()->getCCore()->sip_conf.sdp_200_ack) {
				/* We are NOT offering, set local media description after sending the call so that we are ready to
				* process the remote offer when it will arrive. */
				d->op->setLocalMediaDescription(d->localDesc);
			}

			if (res && (d->state != previousState)) {
				/* Restore initial state */
				d->state = previousState;
			}
			return res;
		};

		const auto preparingStreams = d->getStreamsGroup().prepare();
		// reINVITE sent after full state must be sent after ICE negotiations are completed if ICE is enabled
		if (linphone_nat_policy_ice_enabled(d->natPolicy) && preparingStreams) {
			lInfo() << "Defer CallSession " << this << " (local address " << getLocalAddress().asString() << " remote address " << getRemoteAddress()->asString() << ") update to gather ICE candidates";
			d->queueIceGatheringTask(updateCompletionTask);
			return 0;
		} else if (isIceRunning) {
			// ICE negotiations are ongoing hence the update cannot be send right now
			lInfo() << "Ice negotiations are ongoing and update once they complete, therefore defer CallSession " << this << " (local address " << getLocalAddress().asString() << " remote address " << getRemoteAddress()->asString() << ") update until Ice negotiations are completed.";
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

void MediaSession::enableEchoLimiter (UNUSED(bool value)) {
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
	MS2Stream *s = dynamic_cast<MS2Stream*>(getStreamsGroup().getStream(streamIndex));
	if (!s){
		lError() << "MediaSession::getMetaRtcpTransport(): no stream with index " << streamIndex;
		return nullptr;
	}
	return s->getMetaRtpTransports().second;
}

RtpTransport * MediaSession::getMetaRtpTransport (int streamIndex) const {
	MS2Stream *s = dynamic_cast<MS2Stream*>(getStreamsGroup().getStream(streamIndex));
	if (!s){
		lError() << "MediaSession::getMetaRtcpTransport(): no stream with index " << streamIndex;
		return nullptr;
	}
	return s->getMetaRtpTransports().first;
}

float MediaSession::getMicrophoneVolumeGain () const {
	AudioControlInterface *iface = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface){
		return iface->getMicGain();
	} else {
		lError() << "Could not get record volume: no audio stream";
		return -1.0f;
	}
}

void MediaSession::setMicrophoneVolumeGain (float value) {
	AudioControlInterface *iface = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface)
		iface->setMicGain(value);
	else
		lError() << "Could not set record volume: no audio stream";
}

float MediaSession::getSpeakerVolumeGain () const {
	AudioControlInterface *iface = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface)
		return iface->getSpeakerGain();
	else {
		lError() << "Could not get playback volume: no audio stream";
		return -1.0f;
	}
}

void MediaSession::setSpeakerVolumeGain (float value) {
	AudioControlInterface *iface = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface)
		iface->setSpeakerGain(value);
	else
		lError() << "Could not set playback volume: no audio stream";
}

static bool_t compareFunc(Stream *s, const std::string & label) {
	return s->getType() == SalVideo && label.compare(s->getLabel())==0;
};

void * MediaSession::getNativeVideoWindowId(const std::string label) const {
	if ((getState() != CallSession::State::End) && (getState() != CallSession::State::Released)) {
		if (label.empty()) {
			auto iface = getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
			if (iface) {
				return iface->getNativeWindowId();
			}
		} else {
			auto s = getStreamsGroup().lookupStream(compareFunc, label);
			if (s) {
				VideoControlInterface * iface = dynamic_cast<VideoControlInterface*>(s);
				if (iface == nullptr){
					lError() << "stream " << s << " with label " << label << " cannot be casted to VideoControlInterface";
					return nullptr;
				}
				if (iface) {
					return iface->getNativeWindowId();
				}
			}
		}
	}
	return nullptr;
}

void MediaSession::setNativeVideoWindowId(void *id, const std::string label) {
	if ((getState() != CallSession::State::End) && (getState() != CallSession::State::Released)) {
		if (label.empty()) {
			auto iface = getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
			if (iface) {
				lInfo() << "Set native window ID " << id << " to video control interface " << iface;
				iface->setNativeWindowId(id);
			} else {
				lError() << "Unable to set window ID because video control interface cannot be found";
			}
		} else {
			auto s = getStreamsGroup().lookupStream(compareFunc, label);
			if (s) {
				VideoControlInterface * iface = dynamic_cast<VideoControlInterface*>(s);
				if (iface == nullptr){
					lError() << "stream " << s << " with label " << label << " cannot be casted to VideoControlInterface";
					return;
				}
				if (iface) {
					lInfo() << "Set window ID " << id << " to video stream with label " << label;
					iface->setNativeWindowId(id);
				}
			} else {
				lError() << "Unable to set window ID because no video stream has been found with label " << label;
			}
		}
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

void * MediaSession::createNativePreviewVideoWindowId () const{
	L_D();
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (iface) {
		return iface->createNativePreviewWindowId();
	}
	return nullptr;
}

void * MediaSession::createNativeVideoWindowId(const std::string label) const {
	if ((getState() != CallSession::State::End) && (getState() != CallSession::State::Released)) {
		if (label.empty()) {
			auto iface = getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
			if (iface) {
				return iface->createNativeWindowId();
			}
		} else {
			auto s = getStreamsGroup().lookupStream(compareFunc, label);
			if (s) {
				VideoControlInterface * iface = dynamic_cast<VideoControlInterface*>(s);
				if (iface == nullptr){
					lError() << "stream " << s << " with label " << label << " cannot be casted to VideoControlInterface";
					return nullptr;
				}
				if (iface) {
					return iface->createNativeWindowId();
				}
			}
		}
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

			if (d->isInConference()) {
				params->setConferenceVideoLayout(MediaSession::computeConferenceLayout(md));
			}

			const SalStreamDescription &audioStream = md->findBestStream(SalAudio);
			if (audioStream != Utils::getEmptyConstRefObject<SalStreamDescription>()){
				params->enableAudio(audioStream.enabled());
				params->setAudioDirection(audioStream.getDirection());
				params->setMediaEncryption(audioStream.hasSrtp() ? LinphoneMediaEncryptionSRTP : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeAudio, audioStream.custom_sdp_attributes);
			}else params->enableAudio(false);

			const auto streamIdx = d->getThumbnailStreamIdx(md);
			const auto &videoStream = (streamIdx == -1) ? md->findBestStream(SalVideo) : md->getStreamIdx(static_cast<unsigned int>(streamIdx));
			if (videoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()){
				const auto videoDir = d->getVideoDirFromMd(md);
				params->enableVideo(videoStream.enabled() || (videoDir != LinphoneMediaDirectionInactive));
				params->setVideoDirection(videoDir);
				params->setMediaEncryption(videoStream.hasSrtp() ? LinphoneMediaEncryptionSRTP : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeVideo, videoStream.custom_sdp_attributes);
			}else params->enableVideo(false);

			const SalStreamDescription &textStream = md->findBestStream(SalText);
			if (textStream != Utils::getEmptyConstRefObject<SalStreamDescription>()){
				params->enableRealtimeText(textStream.enabled());
				params->setMediaEncryption(textStream.hasSrtp() ? LinphoneMediaEncryptionSRTP : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeText, textStream.custom_sdp_attributes);
			}else params->enableRealtimeText(false);

			if (!params->videoEnabled()) {
				if ((md->bandwidth > 0) && (md->bandwidth <= linphone_core_get_edge_bw(getCore()->getCCore())))
					params->enableLowBandwidth(true);
			}
			if (md->name[0] != '\0')
				params->setSessionName(md->name);
			params->getPrivate()->setCustomSdpAttributes(md->custom_sdp_attributes);
			params->enableRtpBundle(!md->bundles.empty());
			params->setRecordingState(md->record);

			const auto & times = md->times;
			if (times.size() > 0) {
				const auto & timePair = times.front();
				params->getPrivate()->setStartTime(timePair.first);
				params->getPrivate()->setEndTime(timePair.second);
			}
		} else {
			lInfo() << "Unable to retrieve remote streams because op " << d->op << " has not received yet a valid SDP from the other party";
		}
		const SalCustomHeader *ch = d->op->getRecvCustomHeaders();
		if (ch) {
			if (!params) params = new MediaSessionParams();
			params->getPrivate()->setCustomHeaders(ch);

			const char* supported = params->getCustomHeader("supported");
			params->enableRecordAware(supported && strstr(supported, "record-aware"));
		}
		const list<Content> &additionnalContents = d->op->getAdditionalRemoteBodies();
		for (auto& content : additionnalContents){
			if (!params) params = new MediaSessionParams();
			params->addCustomContent(content);
		}
		d->setRemoteParams(params);
		if (!params) {
			lError() << "Failed to compute remote parameters";
		}
		return params;
	}
	lError() << "Failed to compute remote parameters because no op linked to session " << this << " has been found";
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
		if (stream) {
			LinphoneCallStats *stats = stream->getStats();
			if (stats && linphone_call_stats_get_ice_state(stats) == LinphoneIceStateInProgress){
				return true;
			}
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
				!getCore()->getCCore()->sip_conf.sdp_200_ack : false), isCapabilityNegotiationEnabled(), false);
		break;
		default:
			lError() << "MediaSession::setParams(): Invalid state " << Utils::toString(d->state);
		break;
	}
}

void MediaSession::queueIceCompletionTask(const std::function<LinphoneStatus()> &lambda) {
	L_D();
	d->queueIceCompletionTask(lambda);
}

StreamsGroup & MediaSession::getStreamsGroup()const{
	L_D();
	return d->getStreamsGroup();
}

bool MediaSession::isTerminator() const {
	L_D();
	return d->localIsTerminator;
}

bool MediaSession::setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	L_D();

	if (!audioDevice) {
		lError() << "Unable to use audio device [" << audioDevice << "] as recording device";
		return false;
	}

	const auto & currentInputDevice = getInputAudioDevice();
	// If pointer toward the new device has changed or at least one member of the audio device changed or no current audio device is set, then return true
	bool change = currentInputDevice ? ((audioDevice != currentInputDevice) || (*audioDevice != *currentInputDevice)) : true;

	if (change) {
		AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
		d->setCurrentInputAudioDevice(audioDevice);
		if (i) i->setInputDevice(audioDevice);
		return true;
	}

	return false;
}

bool MediaSession::setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	L_D();

	if (!audioDevice) {
		lError() << "Unable to use audio device [" << audioDevice << "] as playback device";
		return false;
	}

	const auto & currentOutputDevice = getOutputAudioDevice();
	// If pointer toward the new device has changed or at least one member of the audio device changed or no current audio device is set, then return true
	bool change = currentOutputDevice ? ((audioDevice != currentOutputDevice) || (*audioDevice != *currentOutputDevice)) : true;

	if (change) {
		AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
		d->setCurrentOutputAudioDevice(audioDevice);
		if (i) i->setOutputDevice(audioDevice);
		return true;
	}

	return false;
}

std::shared_ptr<AudioDevice> MediaSession::getInputAudioDevice() const {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) return i->getInputDevice();
	return nullptr;
}

std::shared_ptr<AudioDevice> MediaSession::getOutputAudioDevice() const {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) return i->getOutputDevice();
	return nullptr;
}

bool MediaSession::pausedByApp()const{
	L_D();
	return d->pausedByApp;
}

void MediaSession::notifySpeakingDevice(uint32_t ssrc, bool isSpeaking) {
	L_D();
	LinphoneConference * conference = nullptr;
	if (d->listener) {
		conference = d->listener->getCallSessionConference(getSharedFromThis());
		if (conference) {
			const auto cppConference = MediaConference::Conference::toCpp(conference)->getSharedFromThis();
			cppConference->notifySpeakingDevice(ssrc, isSpeaking);
		} else {
			lDebug() << "IsSpeaking: unable to notify speaking device because there is no conference.";
		}
	}
}

void MediaSession::notifyMutedDevice(uint32_t ssrc, bool muted) {
	L_D();
	if (d->listener) {
		LinphoneConference * conference = d->listener->getCallSessionConference(getSharedFromThis());
		if (conference) {
			const auto cppConference = MediaConference::Conference::toCpp(conference)->getSharedFromThis();
			cppConference->notifyMutedDevice(ssrc, muted);
		} else {
			lDebug() << "IsMuted: unable to notify muted device because there is no conference.";
		}
	}
}

void MediaSession::onGoClearAckSent() {
	L_D();
	if (d->listener)
		d->listener->onGoClearAckSent();
}

void * MediaSession::getParticipantWindowId(const std::string label) {
	L_D();

	LinphoneConference * conference = nullptr;
	if (d->listener) {
		conference = d->listener->getCallSessionConference(getSharedFromThis());
		if (conference) {
			const auto cppConference = MediaConference::Conference::toCpp(conference)->getSharedFromThis();
			const auto & device = cppConference->findParticipantDeviceByLabel(label);
			if (device) {
				return device->getWindowId();
			}
		}
	}

	return nullptr;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void MediaSession::setVideoSource (const std::shared_ptr<const VideoSourceDescriptor> &descriptor) {
#ifdef VIDEO_ENABLED
	MS2VideoStream *stream = getStreamsGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);
	stream->setVideoSource(descriptor);
#else
	lError() << "Cannot change the video source as video support is not enabled";
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

std::shared_ptr<const VideoSourceDescriptor> MediaSession::getVideoSource () const {
#ifdef VIDEO_ENABLED
	MS2VideoStream *stream = getStreamsGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);
	return stream ? stream->getVideoSource() : nullptr;
#else
	lError() << "Cannot retrieve the video source as video support is not enabled";
	return nullptr;
#endif
}

void MediaSession::confirmGoClear() {
	getStreamsGroup().confirmGoClear();
}

uint32_t MediaSession::getSsrc(LinphoneStreamType type) const {
	L_D();

	uint32_t ssrc = 0;
	const auto remoteMediaDesc = d->getOp()->getRemoteMediaDescription();
	if (remoteMediaDesc) {
		switch (type) {
			case LinphoneStreamTypeAudio:
			{
				const auto & audioStream = remoteMediaDesc->getActiveStreamOfType(SalAudio, 0);
				const auto & audioStreamCfg = audioStream.getActualConfiguration();
				ssrc = audioStreamCfg.conference_ssrc;
			}
				break;
			case LinphoneStreamTypeVideo:
			{
				const auto & videoStreamIdx = d->getMainVideoStreamIdx(remoteMediaDesc);
				if ((videoStreamIdx >= 0) && (d->getOp()->getFinalMediaDescription()->nbActiveStreamsOfType(SalVideo) > 0)) {
					const SalStreamDescription & videoStream = remoteMediaDesc->getStreamIdx(static_cast<unsigned int>(videoStreamIdx));
					const auto & videoStreamCfg = videoStream.getActualConfiguration();
					ssrc = videoStreamCfg.conference_ssrc;
				}
			}
				break;
			case LinphoneStreamTypeText:
			case LinphoneStreamTypeUnknown:
				ssrc = 0;
				break;
		}
	}

	return ssrc;
}


LINPHONE_END_NAMESPACE
