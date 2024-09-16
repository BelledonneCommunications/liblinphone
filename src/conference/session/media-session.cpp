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

#include <algorithm>

#include <bctoolbox/defs.h>

#include <mediastreamer2/mediastream.h>
#include <mediastreamer2/msequalizer.h>
#include <mediastreamer2/mseventqueue.h>
#include <mediastreamer2/msfileplayer.h>
#include <mediastreamer2/msrtt4103.h>
#include <mediastreamer2/msvolume.h>

#include "account/account.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call-stats.h"
#include "call/call.h"
#include "chat/chat-room/client-chat-room.h"
#include "conference/conference.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "conference/session/streams.h"
#include "core/core-p.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/core.h"
#include "logger/logger.h"
#include "private.h"
#include "sal/call-op.h"
#include "sal/params/sal_media_description_params.h"
#include "sal/sal.h"
#include "sal/sal_media_description.h"
#include "sal/sal_stream_bundle.h"
#include "utils/payload-type-handler.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

#define STR_REASSIGN(dest, src)                                                                                        \
	{                                                                                                                  \
		if (dest) ms_free(dest);                                                                                       \
		dest = src;                                                                                                    \
	}

inline OrtpRtcpXrStatSummaryFlag operator|(OrtpRtcpXrStatSummaryFlag a, OrtpRtcpXrStatSummaryFlag b) {
	return static_cast<OrtpRtcpXrStatSummaryFlag>(static_cast<int>(a) | static_cast<int>(b));
}

// =============================================================================

const string MediaSessionPrivate::ecStateStore = ".linphone.ecstate";
const int MediaSessionPrivate::ecStateMaxLen = 1048576; /* 1Mo */

const string MediaSessionPrivate::DTXAudioContentAttribute = "DTX";
const string MediaSessionPrivate::ActiveSpeakerVideoContentAttribute = "speaker";
const string MediaSessionPrivate::GridVideoContentAttribute = "main";
const string MediaSessionPrivate::ThumbnailVideoContentAttribute = "thumbnail";
const string MediaSessionPrivate::ScreenSharingContentAttribute = "slides";

// =============================================================================

std::unique_ptr<LogContextualizer> MediaSessionPrivate::getLogContextualizer() const {
	if (listeners.size() > 0) {
		auto listener = listeners.front();
		if (listener) return listener->getLogContextualizer();
	}
	return nullptr;
}

void MediaSessionPrivate::setDtlsFingerprint(const std::string &fingerPrint) {
	dtlsCertificateFingerprint = fingerPrint;
}

const std::string &MediaSessionPrivate::getDtlsFingerprint() const {
	return dtlsCertificateFingerprint;
}

void MediaSessionPrivate::stunAuthRequestedCb(void *userData,
                                              const char *realm,
                                              const char *nonce,
                                              const char **username,
                                              const char **password,
                                              const char **ha1) {
	MediaSessionPrivate *msp = static_cast<MediaSessionPrivate *>(userData);
	msp->stunAuthRequestedCb(realm, nonce, username, password, ha1);
}

LinphoneMediaEncryption
MediaSessionPrivate::getEncryptionFromMediaDescription(const std::shared_ptr<SalMediaDescription> &md) const {
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

	// Do not change encryption if no stream is started or at least one is not encrypted or chosen encryption is not
	// supported
	return getParams()->getMediaEncryption();
}

bool MediaSessionPrivate::isMediaEncryptionAccepted(const LinphoneMediaEncryption enc) const {
	return ((getParams()->getMediaEncryption() == enc) || (getParams()->getPrivate()->isMediaEncryptionSupported(enc)));
}

LinphoneMediaEncryption MediaSessionPrivate::getNegotiatedMediaEncryption() const {

	switch (state) {
		case CallSession::State::Idle:
		case CallSession::State::IncomingReceived:
		case CallSession::State::OutgoingInit:
		case CallSession::State::OutgoingProgress:
		case CallSession::State::OutgoingRinging:
		case CallSession::State::OutgoingEarlyMedia:
		case CallSession::State::PushIncomingReceived:
			return getParams()->getMediaEncryption();
			break;
		case CallSession::State::Connected: {
			std::shared_ptr<SalMediaDescription> md = (op) ? op->getFinalMediaDescription() : nullptr;
			return md ? getEncryptionFromMediaDescription(md) : getParams()->getMediaEncryption();
		} break;
		default:
			return negotiatedEncryption;
			break;
	}

	return LinphoneMediaEncryptionNone;
}

// -----------------------------------------------------------------------------

bool MediaSessionPrivate::tryEnterConference() {
	L_Q();
	q->updateContactAddressInOp();
	const auto updatedContactAddress = q->getContactAddress();
	ConferenceId serverConferenceId = ConferenceId(updatedContactAddress, updatedContactAddress);
	shared_ptr<Conference> conference = q->getCore()->findConference(serverConferenceId, false);
	// If the call conference ID is not an empty string but no conference is linked to the call means that it was added
	// to the conference after the INVITE session was started but before its completition
	if (conference) {
		if (state == CallSession::State::Paused) {
			// Resume call as it was added to conference
			lInfo() << "Media session (local address " << *q->getLocalAddress() << " remote address "
			        << *q->getRemoteAddress() << ") was added to conference " << *conference->getConferenceAddress()
			        << " while the call was being paused. Resuming the session.";
			q->resume();
		} else {
			// Send update to notify that the call enters conference
			MediaSessionParams *newParams = q->getMediaParams()->clone();
			lInfo() << "Media session (local address " << *q->getLocalAddress() << " remote address "
			        << *q->getRemoteAddress() << ") was added to conference " << *conference->getConferenceAddress()
			        << " while the call was establishing. Sending update to notify remote participant.";
			q->update(newParams, CallSession::UpdateMethod::Default, q->isCapabilityNegotiationEnabled());
			delete newParams;
		}
		return true;
	}
	return false;
}

bool MediaSessionPrivate::rejectMediaSession(const std::shared_ptr<SalMediaDescription> &localMd,
                                             const std::shared_ptr<SalMediaDescription> &remoteMd,
                                             const std::shared_ptr<SalMediaDescription> &finalMd) const {
	L_Q();
	if (remoteMd && remoteMd->isEmpty() &&
	    linphone_core_zero_rtp_port_for_stream_inactive_enabled(q->getCore()->getCCore())) {
		return false;
	}

	if (!finalMd) return false;

	bool bundleOwnerRejected = false;
	if (!finalMd->bundles.empty()) {
		const auto ownerIndexes = finalMd->getTransportOwnerIndexes();
		bool isThereAnActiveOwner = false;
		if (!ownerIndexes.empty()) {
			for (const auto &idx : ownerIndexes) {
				const auto &sd = finalMd->getStreamAtIdx(static_cast<unsigned int>(idx));
				isThereAnActiveOwner |= sd.enabled();
			}
			bundleOwnerRejected = !isThereAnActiveOwner;
		}
	}
	auto securityCheckFailure = incompatibleSecurity(finalMd);
	auto reject = ((finalMd->isEmpty() && !(localIsOfferer ? localMd->isEmpty() : remoteMd->isEmpty())) ||
	               securityCheckFailure || bundleOwnerRejected);
	if (reject) {
		lWarning() << "Session [" << q << "] may be rejected: ";
		lWarning() << "- negotiated SDP is" << (finalMd->isEmpty() ? std::string() : std::string(" not")) << " empty";
		lWarning() << "- negotiated security is" << (securityCheckFailure ? std::string(" not") : std::string())
		           << " compatible with core settings";
		lWarning() << "- bundle owner has been "
		           << (bundleOwnerRejected ? std::string("rejected") : std::string("accepted"));
	}
	return reject;
}

void MediaSessionPrivate::accepted() {
	L_Q();
	auto logContext = getLogContextualizer();

	CallSessionPrivate::accepted();
	LinphoneTaskList tl;
	linphone_task_list_init(&tl);

	switch (state) {
		case CallSession::State::OutgoingProgress:
		case CallSession::State::OutgoingRinging:
		case CallSession::State::OutgoingEarlyMedia:
		case CallSession::State::Connected:
			if (q->getCore()->getCCore()->sip_conf.sdp_200_ack) {
				lInfo() << "Initializing local media description according to remote offer in 200Ok";
				if (!localIsOfferer) {
					// We were waiting for an incoming offer. Now prepare the local media description according to
					// remote offer.
					initializeParamsAccordingToIncomingCallParams();
					makeLocalMediaDescription(op->getRemoteMediaDescription() ? localIsOfferer : true,
					                          q->isCapabilityNegotiationEnabled(), false);
				}
				/*
				 * If ICE is enabled, we'll have to do the prepare() step, however since defering the sending of the ACK
				 * is complicated and confusing from a signaling standpoint, ICE we will skip the STUN gathering by not
				 * giving enough time for the gathering step. Only local candidates will be answered in the ACK.
				 */
				if (natPolicy && natPolicy->iceEnabled()) {
					if (getStreamsGroup().prepare()) {
						lWarning() << "Some gathering is needed for ICE, however since a defered sending of ACK is not "
						              "supported"
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
	std::shared_ptr<SalMediaDescription> lmd = op->getLocalMediaDescription();
	if (lmd) {
		std::shared_ptr<SalMediaDescription> rmd = op->getRemoteMediaDescription();
		std::shared_ptr<SalMediaDescription> &md = op->getFinalMediaDescription();
		if (!md && (prevState == CallSession::State::OutgoingEarlyMedia) && resultDesc) {
			lInfo() << "Using early media SDP since none was received with the 200 OK";
			md = resultDesc;
		}

		const auto conferenceInfo = (log) ? log->getConferenceInfo() : nullptr;
		bool updatingConference = conferenceInfo && (conferenceInfo->getState() == ConferenceInfo::State::Updated);
		// Do not reject media session if the client is trying to update a conference
		if (rejectMediaSession(lmd, rmd, md) && !updatingConference) {
			lInfo() << "Rejecting media session";
			md = nullptr;
		}

		if (md) {
			/* There is a valid SDP in the response, either offer or answer, and we're able to start/update the streams
			 */
			CallSession::State nextState = CallSession::State::Idle;
			string nextStateMsg;
			switch (state) {
				case CallSession::State::Resuming:
				case CallSession::State::Connected:
					if (referer) notifyReferState();
					BCTBX_NO_BREAK; /* Intentional no break */
				case CallSession::State::Updating:
				case CallSession::State::UpdatedByRemote:
					if (state == CallSession::State::Updating && prevState == CallSession::State::Paused) {
						// If previous state was paused and we are in Updating, there is no reason to set it to anything
						// else than paused
						nextState = CallSession::State::Paused;
						nextStateMsg = "Call paused";
					} else {
						// The call always enters state PausedByRemote if all streams are rejected. This is done to
						// support some clients who accept to stop the streams by setting the RTP port to 0 If the call
						// is part of a conference, then it shouldn't be paused if it is just trying to update the
						// conference
						if (isPausedByRemoteAllowed() && !localDesc->hasDir(SalStreamInactive) &&
						    (md->hasDir(SalStreamRecvOnly) || md->hasDir(SalStreamInactive) || md->isEmpty())) {
							nextState = CallSession::State::PausedByRemote;
							nextStateMsg = "Call paused by remote";
						} else {
							if (!getParams()->getPrivate()->getInConference()) {
								q->notifySetCurrentSession();
							}
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
					/* When we entered the pausing state, we always reach the paused state whatever the content of the
					 * remote SDP is. Our streams are all send-only (with music), soundcard and camera are never used.
					 */
					nextState = CallSession::State::Paused;
					nextStateMsg = "Call paused";
					if (referPending) linphone_task_list_add(&tl, &MediaSessionPrivate::startPendingRefer, q);
					break;
				default:
					lError() << "accepted(): don't know what to do in state [" << Utils::toString(state) << "]";
					break;
			}

			// A negative value of the counter may lead to unexpected behaviour, hence terminate here the execution in
			// order to analyze what leads to this scenario
			if (nbProcessingUpdates < 0) {
				lFatal() << "The number of updates under processing for media session (local address "
				         << q->getLocalAddress()->toString() << " remote address " << q->getRemoteAddress()->toString()
				         << ") should be greater than or equal to 0. Currently it is " << nbProcessingUpdates;
			}

			if (nextState == CallSession::State::Idle) {
				lError() << "BUG: nextState is not set in accepted(), current state is " << Utils::toString(state);
			} else {
				updateRemoteSessionIdAndVer();
				// getIceAgent().updateIceStateInCallStats();
				updateStreams(md, nextState);
				fixCallParams(rmd, false);

				setState(nextState, nextStateMsg);
				bool capabilityNegotiationReInviteSent = false;
				const bool capabilityNegotiationReInviteEnabled =
				    getParams()->getPrivate()->capabilityNegotiationReInviteEnabled();
				// If capability negotiation is enabled, a second invite must be sent if the selected configuration is
				// not the actual one. It normally occurs after moving to state StreamsRunning. However, if ICE
				// negotiations are not completed, then this action will be carried out together with the ICE re-INVITE
				if (localDesc->getParams().capabilityNegotiationSupported() &&
				    (nextState == CallSession::State::StreamsRunning) && localIsOfferer &&
				    capabilityNegotiationReInviteEnabled) {
					// If no ICE session or checklist has completed, then send re-INVITE
					// The reINVITE to notify intermediaries that do not support capability negotiations (RFC5939) is
					// sent in the following scenarios:
					// - no ICE session is found in th stream group
					// - an ICE sesson is found and its checklist has already completed
					// - an ICE sesson is found and ICE reINVITE is not sent upon completition if the checklist (This
					// case is the default one for DTLS SRTP negotiation as it was observed that webRTC gateway did not
					// correctly support SIP ICE reINVITEs)
					if (!getStreamsGroup().getIceService().getSession() ||
					    (getStreamsGroup().getIceService().getSession() &&
					     (!isUpdateSentWhenIceCompleted() ||
					      getStreamsGroup().getIceService().hasCompletedCheckList()))) {
						// Compare the chosen final configuration with the actual configuration in the local decription
						const auto diff = md->compareToActualConfiguration(*localDesc);
						const bool potentialConfigurationChosen = (diff & SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED);
						if (potentialConfigurationChosen) {
							lInfo() << "Sending a reINVITE because the actual configuraton was not chosen in the "
							           "capability negotiation procedure. Detected differences "
							        << SalMediaDescription::printDifferences(diff);
							MediaSessionParams newParams(*getParams());
							newParams.getPrivate()->setInternalCallUpdate(true);
							q->update(&newParams, CallSession::UpdateMethod::Default, true);
							capabilityNegotiationReInviteSent = true;
						} else {
							lInfo()
							    << "Using actual configuration after capability negotiation procedure, hence no need "
							       "to send a reINVITE";
						}
					} else {
						lInfo() << "Capability negotiation and ICE are both enabled hence wait for the end of ICE "
						           "checklist completion to send a reINVITE";
					}
				}

				if (getOp() && getOp()->getContactAddress()) {
					const auto contactAddress = q->getContactAddress();
					const auto &confId = getConferenceId();
					if (!confId.empty() && isInConference() && !contactAddress->hasParam("isfocus")) {
						// If the call was added to a conference after the last INVITE session was started, the reINVITE
						// to enter conference must be sent only if capability negotiation reINVITE was not sent
						if (!capabilityNegotiationReInviteSent) {
							// Add to conference if it was added after last INVITE message sequence started
							// It occurs if the local participant calls the remote participant and the call is added to
							// the conference when it is in state OutgoingInit, OutgoingProgress or OutgoingRinging
							q->getCore()->doLater([this]() {
								/* This has to be done outside of the accepted callback, because after the callback the
								 * SIP ACK is going to be sent. Despite it is not forbidden by RFC3261, it is preferable
								 * for the sake of clarity that the ACK for the current transaction is sent before the
								 * new INVITE that will be sent by tryEnterConference(). Some implementations (eg
								 * FreeSwitch) reply "500 Overlapped request" otherwise ( which is in fact a
								 * misunderstanding of RFC3261).
								 */
								tryEnterConference();
							});
						}
					}
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
					switch (state) {
						case CallSession::State::PausedByRemote:
						case CallSession::State::Paused:
						case CallSession::State::StreamsRunning:
							break;
						default:
							lInfo() << "Incompatible SDP answer received, restoring previous state ["
							        << Utils::toString(prevState) << "]";
							setState(prevState, "Incompatible media parameters.");
							break;
					}
					break;
			}
		}
	}

	getCurrentParams()->getPrivate()->setInConference(getParams()->getPrivate()->getInConference());

	linphone_task_list_run(&tl);
	linphone_task_list_free(&tl);
}

void MediaSessionPrivate::ackReceived(LinphoneHeaders *headers) {
	L_Q();
	auto logContext = getLogContextualizer();
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

void MediaSessionPrivate::dtmfReceived(char dtmf) {
	L_Q();
	q->notifyDtmfReceived(dtmf);
}

bool MediaSessionPrivate::failure() {
	L_Q();
	auto logContext = getLogContextualizer();
	if (CallSession::isEarlyState(state) && getStreamsGroup().isStarted()) {
		stopStreams();
	}

	const SalErrorInfo *ei = op->getErrorInfo();
	switch (ei->reason) {
		case SalReasonUnsupportedContent: /* This is for compatibility: linphone sent 415 because of SDP offer answer
		                                     failure */
		case SalReasonNotAcceptable:
			if ((linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
			                             "retry_invite_after_offeranswer_failure", 1)) &&
			    localDesc &&
			    ((state == CallSession::State::OutgoingInit) || (state == CallSession::State::OutgoingProgress) ||
			     (state == CallSession::State::OutgoingRinging) /* Push notification case */
			     || (state == CallSession::State::OutgoingEarlyMedia))) {
				bool mediaEncrptionSrtp = getParams()->getMediaEncryption() == LinphoneMediaEncryptionSRTP;
				bool avpfEnabled = getParams()->avpfEnabled();
				if (mediaEncrptionSrtp || avpfEnabled) {
					lInfo() << "Outgoing CallSession [" << q << "] failed with SRTP and/or AVPF enabled";
					string previousCallId = op->getCallId();
					for (auto &stream : localDesc->streams) {
						bool firstStream = (stream == localDesc->streams[0]);
						if (!stream.enabled()) continue;
						if (mediaEncrptionSrtp) {
							if (avpfEnabled) {
								if (firstStream) lInfo() << "Retrying CallSession [" << q << "] with SAVP";
								getParams()->enableAvpf(false);
								restartInvite();
								linphone_core_notify_call_id_updated(q->getCore()->getCCore(), previousCallId.c_str(),
								                                     op->getCallId().c_str());
								return true;
							} else if (!linphone_core_is_media_encryption_mandatory(q->getCore()->getCCore())) {
								if (firstStream) lInfo() << "Retrying CallSession [" << q << "] with AVP";
								getParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
								stream.cfgs[stream.getChosenConfigurationIndex()].crypto.clear();
								getParams()->enableAvpf(false);
								restartInvite();
								linphone_core_notify_call_id_updated(q->getCore()->getCCore(), previousCallId.c_str(),
								                                     op->getCallId().c_str());
								return true;
							}
						} else if (avpfEnabled) {
							if (firstStream) lInfo() << "Retrying CallSession [" << q << "] with AVP";
							getParams()->enableAvpf(false);
							getParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
							stream.cfgs[stream.getChosenConfigurationIndex()].crypto.clear();
							restartInvite();
							linphone_core_notify_call_id_updated(q->getCore()->getCCore(), previousCallId.c_str(),
							                                     op->getCallId().c_str());
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
	if (stop) return true;

	// Do not try to schedule resume of a call if it is ending
	if (referer && (state != CallSession::State::End) && (state != CallSession::State::Released)) {
		// Schedule automatic resume of the call. This must be done only after the notifications are completed due to
		// dialog serialization of requests
		linphone_core_queue_task(q->getCore()->getCCore(), &MediaSessionPrivate::resumeAfterFailedTransfer,
		                         referer.get(), "Automatic CallSession resuming after failed transfer");
	}

	return false;
}

void MediaSessionPrivate::pauseForTransfer() {
	L_Q();
	lInfo() << "Automatically pausing current MediaSession to accept transfer";
	q->pause();
	automaticallyPaused = true;
}

void MediaSessionPrivate::pausedByRemote() {
	L_Q();
	auto logContext = getLogContextualizer();
	MediaSessionParams newParams(*getParams());
	if (linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "inactive_audio_on_pause",
	                            0)) {
		lInfo() << "Media session [" << this << "] (local address " << *q->getLocalAddress() << " remote address "
		        << *q->getRemoteAddress()
		        << "): Setting audio direction to inactive when being paused as per user setting in the linphonerc";
		newParams.setAudioDirection(LinphoneMediaDirectionInactive);
	}
	if (linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "inactive_video_on_pause",
	                            0)) {
		lInfo() << "Media session [" << this << "] (local address " << *q->getLocalAddress() << " remote address "
		        << *q->getRemoteAddress()
		        << "): Setting video direction to inactive when being paused as per user setting in the linphonerc";
		newParams.setVideoDirection(LinphoneMediaDirectionInactive);
	}
	acceptUpdate(&newParams, CallSession::State::PausedByRemote, "Call paused by remote");
}

void MediaSessionPrivate::remoteRinging() {
	L_Q();
	auto logContext = getLogContextualizer();

	updateToFromAssertedIdentity();
	/* Set privacy */
	getCurrentParams()->setPrivacy((LinphonePrivacyMask)op->getPrivacy());
	std::shared_ptr<SalMediaDescription> md = op->getFinalMediaDescription();
	std::shared_ptr<SalMediaDescription> rmd = op->getRemoteMediaDescription();
	if (md && rmd) {
		/* Initialize the remote call params by invoking linphone_call_get_remote_params(). This is useful as the SDP
		 * may not be present in the 200Ok */
		q->getRemoteParams();
		/* Accept early media */

		if (rmd && getStreamsGroup().isStarted()) {
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
			if (!op->getNotifyAllRingings()) {
				/* Already doing early media */
				return;
			}
			/* stop the early media */
			stopStreams();
		}
		setState(CallSession::State::OutgoingRinging, "Remote ringing");
	}
}

void MediaSessionPrivate::replaceOp(SalCallOp *newOp) {
	CallSessionPrivate::replaceOp(newOp);
	updateStreams(newOp->getFinalMediaDescription(), state);
}

int MediaSessionPrivate::resumeAfterFailedTransfer() {
	L_Q();
	auto logContext = getLogContextualizer();
	if (automaticallyPaused && (state == CallSession::State::Pausing))
		return BELLE_SIP_CONTINUE; // Was still in pausing state
	if (automaticallyPaused && (state == CallSession::State::Paused)) {
		if (op->isIdle()) q->resume();
		else {
			lInfo() << "MediaSessionPrivate::resumeAfterFailedTransfer(), op was busy";
			return BELLE_SIP_CONTINUE;
		}
	}
	return BELLE_SIP_STOP;
}

void MediaSessionPrivate::resumed() {
	auto logContext = getLogContextualizer();
	acceptUpdate(nullptr, CallSession::State::StreamsRunning, "Connected (streams running)");
}

void MediaSessionPrivate::startPendingRefer() {
	L_Q();
	q->notifyCallSessionStartReferred();
}

void MediaSessionPrivate::telephoneEventReceived(int event) {
	static char dtmfTab[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'A', 'B', 'C', 'D'};
	if ((event < 0) || (event > 15)) {
		lWarning() << "Bad dtmf value " << event;
		return;
	}
	dtmfReceived(dtmfTab[event]);
}

void MediaSessionPrivate::terminated() {
	L_Q();
	stopStreams();
	q->getCore()->getPrivate()->getToneManager().stopSecurityAlert();
	CallSessionPrivate::terminated();
}

bool MediaSessionPrivate::isPausedByRemoteAllowed() {
	L_Q();
	const auto conferenceInfo = (log) ? log->getConferenceInfo() : nullptr;
	bool updatingConference = conferenceInfo && (conferenceInfo->getState() == ConferenceInfo::State::Updated);

	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	std::shared_ptr<Address> remoteContactAddress = Address::create();
	remoteContactAddress->setImpl(op->getRemoteContactAddress());
	// Paused by remote state is not allowed when the call is in a conference. In fact, a conference server is not
	// allowed to paused a call unilaterally. This assumption also aims at simplifying the management of the
	// PausedByRemote state as it is simply triggered by SIP messages without really knowing the will of the other party
	return !((conference && !isInConference() && remoteContactAddress && remoteContactAddress->hasParam("isfocus")) ||
	         updatingConference ||
	         ((prevState != CallSession::State::PausedByRemote) && (state == CallSession::State::Updating)));
}

/* This callback is called when an incoming re-INVITE/ SIP UPDATE modifies the session */
void MediaSessionPrivate::updated(bool isUpdate) {
	L_Q();
	auto logContext = getLogContextualizer();

	const std::shared_ptr<SalMediaDescription> &rmd = op->getRemoteMediaDescription();
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
				q->notifyRemoteRecording(rmd->record == SalMediaRecordOn);
			}
			if (isPausedByRemoteAllowed() && rmd &&
			    (rmd->hasDir(SalStreamSendOnly) || rmd->hasDir(SalStreamInactive))) {
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
		const bool acceptAnyEncryption = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()),
		                                                           "rtp", "accept_any_encryption", 0);
		// Verify that the negotiated encryption is not None if accepting any encryption
		if (acceptAnyEncryption) {
			if (negotiatedEncryption == LinphoneMediaEncryptionNone) {
				lError() << "Encryption is mandatory however the negotiated encryption is "
				         << linphone_media_encryption_to_string(negotiatedEncryption);
				return true;
			}
		} else {
			if ((negotiatedEncryption == LinphoneMediaEncryptionSRTP) && !md->hasSrtp()) {
				lError() << "Negotiated encryption is " << linphone_media_encryption_to_string(negotiatedEncryption)
				         << " however media description has no stream has been negotiated with it";
				return true;
			}
		}
	}
	return false;
}

void MediaSessionPrivate::updating(bool isUpdate) {
	L_Q();
	auto logContext = getLogContextualizer();
	if ((state == CallSession::State::End) || (state == CallSession::State::Released)) {
		lWarning() << "Session [" << q << "] is going to reject the reINVITE or UPDATE because it is already in state ["
		           << Utils::toString(state) << "]";
		SalErrorInfo sei;
		memset(&sei, 0, sizeof(sei));
		sal_error_info_set(&sei, SalReasonNoMatch, "SIP", 0, "Incompatible SDP", nullptr);
		op->declineWithErrorInfo(&sei, nullptr);
		sal_error_info_reset(&sei);
		return;
	}

	std::shared_ptr<SalMediaDescription> rmd = op->getRemoteMediaDescription();
	// Fix local parameter before creating new local media description in order to have it consistent with the offer.
	// Note that in some case such as if we are the offerer or transition from the state UpdateByRemote to
	// PausedByRemote, there may be further changes The goal of calling this method at such early stage is to given an
	// initial set of consistent parameters that can be used as such to generate the local media description to give to
	// the offer answer module and negotiate the final answer.
	fixCallParams(rmd, true);
	if (state != CallSession::State::Paused) {
		/* Refresh the local description, but in paused state, we don't change anything. */
		const bool makeOffer = (rmd == nullptr);
		if (makeOffer && linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
		                                         "sdp_200_ack_follow_video_policy", 0)) {
			lInfo() << "Applying default policy for offering SDP on CallSession [" << q << "]";
			setParams(new MediaSessionParams());
			// Yes we init parameters as if we were in the case of an outgoing call, because it is a resume with no SDP.
			params->initDefault(q->getCore(), LinphoneCallOutgoing);
		}

		// Reenable all streams if we are the offerer
		// This occurs with clients such as Avaya and FreeSwitch that put a call on hold by setting streams with
		// inactive direction and RTP port to 0 Scenario:
		// - client1 sends an INVITE without SDP
		// - client2 puts its offer down in the 200Ok
		// - client1 answers with inactive stream and RTP port set to 0
		// Without the workaround, a deadlock is created - client1 has inactive streams and client2 has audio/video/text
		// capabilities disabled in its local call parameters because the stream was rejected earlier on. Therefore it
		// would be impossible to resume the streams if we are asked to make an offer.
		if (localDesc && (makeOffer || ((state == CallSession::State::PausedByRemote) &&
		                                (prevState == CallSession::State::UpdatedByRemote)))) {
			for (const auto &stream : localDesc->streams) {
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
		bool enableCapabilityNegotiations = false;
		bool useNegotiatedMediaProtocol = true;
		// Add capability negotiation attribute during update if they are supported
		if (state == CallSession::State::StreamsRunning) {
			enableCapabilityNegotiations = q->isCapabilityNegotiationEnabled();
			useNegotiatedMediaProtocol = false;
		}
		makeLocalMediaDescription(makeOffer, enableCapabilityNegotiations, useNegotiatedMediaProtocol);
	}
	if (rmd) {
		SalErrorInfo sei;
		memset(&sei, 0, sizeof(sei));
		expectMediaInAck = false;
		std::shared_ptr<SalMediaDescription> lmd = op->getLocalMediaDescription();
		std::shared_ptr<SalMediaDescription> &md = op->getFinalMediaDescription();
		if (rejectMediaSession(lmd, rmd, md)) {
			lWarning() << "Session [" << q << "] is going to be rejected because of an incompatible negotiated SDP";
			sal_error_info_set(&sei, SalReasonNotAcceptable, "SIP", 0, "Incompatible SDP", nullptr);
			op->declineWithErrorInfo(&sei, nullptr);
			sal_error_info_reset(&sei);
			return;
		}
		std::shared_ptr<SalMediaDescription> &prevResultDesc = resultDesc;
		if (isUpdate && prevResultDesc && md) {
			int diff = md->equal(*prevResultDesc);
			if (diff & (SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED | SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED)) {
				lWarning() << "Cannot accept this update, it is changing parameters that require user approval";
				sal_error_info_set(&sei, SalReasonUnknown, "SIP", 504,
				                   "Cannot change the session parameters without prompting the user", nullptr);
				op->declineWithErrorInfo(&sei, nullptr);
				sal_error_info_reset(&sei);
				return;
			}
		}
		updated(isUpdate);
	} else {
		const auto audioEnabled = getParams()->audioEnabled();
		const auto videoEnabled = getParams()->videoEnabled();
		const auto remoteContactAddress = q->getRemoteContactAddress();
		const auto localAddress = q->getLocalAddress();
		const auto conference = q->getCore()->findConference(ConferenceId(localAddress, localAddress), false);
		// If the media session is in a conference, the remote media description is empty and audio video capabilities
		// are disabled, then just call end the update
		if ((((!!linphone_core_conference_server_enabled(q->getCore()->getCCore())) && conference) ||
		     (remoteContactAddress && remoteContactAddress->hasParam("isfocus"))) &&
		    !audioEnabled && !videoEnabled) {
			updated(isUpdate);
		} else {
			/* Case of a reINVITE or UPDATE without SDP */
			expectMediaInAck = true;
			op->accept(); /* Respond with an offer */
			              /* Don't do anything else in this case, wait for the ACK to receive to notify the app */
		}
	}
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::oglRender() {
#ifdef VIDEO_ENABLED
	MS2VideoStream *vs = getStreamsGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);
	if (vs) vs->oglRender();
#endif
}

void MediaSessionPrivate::sendVfu() {
	getStreamsGroup().forEach<VideoControlInterface>([](VideoControlInterface *i) { i->sendVfu(); });
}

// -----------------------------------------------------------------------------

bool MediaSessionPrivate::getSpeakerMuted() const {
	AudioControlInterface *i = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	return i ? !i->speakerEnabled() : false;
}

void MediaSessionPrivate::setSpeakerMuted(bool muted) {
	AudioControlInterface *i = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) i->enableSpeaker(!muted);
}

// -----------------------------------------------------------------------------

bool MediaSessionPrivate::getMicrophoneMuted() const {
	AudioControlInterface *i = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	return i ? !i->micEnabled() : false;
}

void MediaSessionPrivate::setMicrophoneMuted(bool muted) {
	AudioControlInterface *i = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) i->enableMic(!muted);
	// update local params as well:
	if (params) getParams()->enableMic(!muted);
}

// -----------------------------------------------------------------------------

MediaSessionParams *MediaSessionPrivate::createMediaSessionParams() {
	L_Q();
	MediaSessionParams *msp = new MediaSessionParams(*getParams());

	auto videoDir = computeNewVideoDirection(q->getCore()->getCCore()->video_policy->accept_media_direction);
	const auto &conference = q->getCore()->findConference(q->getSharedFromThis());
	if (conference) {
		videoDir = conference->verifyVideoDirection(q->getSharedFromThis(), videoDir);
	}
	msp->setVideoDirection(videoDir);

	return msp;
}

void MediaSessionPrivate::setCurrentParams(MediaSessionParams *msp) {
	if (currentParams) delete currentParams;
	currentParams = msp;
}

void MediaSessionPrivate::setParams(MediaSessionParams *msp) {
	// Pass the account used for the call to the local parameters.
	// It has been chosen at the start and it should not be changed anymore
	if (msp) msp->setAccount(getDestAccount());
	CallSessionPrivate::setParams(msp);
}

void MediaSessionPrivate::setRemoteParams(MediaSessionParams *msp) const {
	if (remoteParams) delete remoteParams;
	remoteParams = msp;
}

Stream *MediaSessionPrivate::getStream(LinphoneStreamType type) const {
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

shared_ptr<CallStats> MediaSessionPrivate::getStats(LinphoneStreamType type) const {
	Stream *s = getStream(type);
	if (s) return s->getStats();
	lError() << "There is no stats for main stream of type " << linphone_stream_type_to_string(type)
	         << " because this stream doesn't exist.";
	return nullptr;
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::stopStreams() {
	if (getStreamsGroup().isStarted()) getStreamsGroup().stop();
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::onNetworkReachable(bool sipNetworkReachable, bool mediaNetworkReachable) {
	L_Q();
	if (mediaNetworkReachable) {
		LinphoneConfig *config = linphone_core_get_config(q->getCore()->getCCore());
		if (linphone_config_get_int(config, "net", "recreate_sockets_when_network_is_up", 0)) refreshSockets();
	} else {
		setBroken();
	}
	CallSessionPrivate::onNetworkReachable(sipNetworkReachable, mediaNetworkReachable);
}

// -----------------------------------------------------------------------------

#ifdef TEST_EXT_RENDERER
void MediaSessionPrivate::extRendererCb(void *userData, const MSPicture *local, const MSPicture *remote) {
	lInfo() << "extRendererCb, local buffer=" << local ? local->planes[0]
	: nullptr << ", remote buffer=" << remote          ? remote->planes[0]
	                                                   : nullptr;
}
#endif

int MediaSessionPrivate::sendDtmf(void *data, BCTBX_UNUSED(unsigned int revents)) {
	MediaSession *session = static_cast<MediaSession *>(data);
	return session->getPrivate()->sendDtmf();
}

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

shared_ptr<Participant> MediaSessionPrivate::getMe() const {
	shared_ptr<Participant> participant = me.lock();
	if (!participant) {
		lWarning() << "Unable to get valid Participant instance";
		throw std::bad_weak_ptr();
	}
	return participant;
}

void MediaSessionPrivate::setState(CallSession::State newState, const string &message) {
	L_Q();
	q->getCore()->getPrivate()->getToneManager().notifyState(q->getSharedFromThis(), newState);
	// Take a ref on the session otherwise it might get destroyed during the call to setState
	shared_ptr<CallSession> sessionRef = q->getSharedFromThis();
	if ((newState != state) && (newState != CallSession::State::StreamsRunning)) q->cancelDtmfs();
	CallSessionPrivate::setState(newState, message);
	q->notifyCallSessionStateChangedForReporting();
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
				lInfo() << "CallSession [" << q
				        << "]: ICE reinvite received, but one or more check-lists are not completed. Response will be "
				           "sent later, when ICE has completed";
			}
			break;
		default:
			break;
	}
}

// -----------------------------------------------------------------------------

/*
 * This method needs to be called at each incoming reINVITE, in order to adjust various local parameters to what is
 * being offered by remote:
 * - the stream indexes.
 * - the video enablement parameter according to what is offered and our local policy.
 * Fixing the params to proper values avoid request video by accident during internal call updates, pauses and resumes
 */
void MediaSessionPrivate::fixCallParams(std::shared_ptr<SalMediaDescription> &rmd, bool fromOffer) {
	L_Q();
	if (!rmd) return;

	updateBiggestDesc(rmd);
	/* Why disabling implicit_rtcp_fb ? It is a local policy choice actually. It doesn't disturb to propose it again and
	 * again even if the other end apparently doesn't support it. The following line of code is causing trouble, while
	 * for example making an audio call, then adding video. Due to the 200Ok response of the audio-only offer where no
	 * rtcp-fb attribute is present, implicit_rtcp_fb is set to false, which is then preventing it to be eventually used
	 * when video is later added to the call. I did the choice of commenting it out.
	 */

	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	bool isInLocalConference = getParams()->getPrivate()->getInConference();
	bool isInRemoteConference = conference && !isInLocalConference;

	/*params.getPrivate()->enableImplicitRtcpFb(params.getPrivate()->implicitRtcpFbEnabled() &
	 * sal_media_description_has_implicit_avpf(rmd));*/
	const MediaSessionParams *rcp = q->getRemoteParams();
	if (rcp) {
		/*
		 * This is to avoid to re-propose again some streams that have just been declined.
		 */
		if (getParams()->audioEnabled() && !rcp->audioEnabled() && !isInRemoteConference) {
			lInfo() << "CallSession [" << q
			        << "]: disabling audio in our call params because the remote doesn't want it";
			getParams()->enableAudio(false);
		}
		if (getParams()->videoEnabled() && !rcp->videoEnabled() && !isInRemoteConference) {
			lInfo() << "CallSession [" << q
			        << "]: disabling video in our call params because the remote doesn't want it";
			getParams()->enableVideo(false);
		}
		if (getParams()->realtimeTextEnabled() && !rcp->realtimeTextEnabled() && !isInRemoteConference) {
			lInfo() << "CallSession [" << q << "]: disabling RTT in our call params because the remote doesn't want it";
			getParams()->enableRealtimeText(false);
		}
		// Real Time Text is always by default accepted when proposed.
		if (!getParams()->realtimeTextEnabled() && rcp->realtimeTextEnabled()) getParams()->enableRealtimeText(true);

		const auto &cCore = q->getCore()->getCCore();
		if (isInLocalConference) {
			// If the call is in a local conference, then check conference capabilities to know whether the video must
			// be enabled or not
			bool isConferenceVideoCapabilityOn = false;
			if (conference) {
				const auto &params = conference->getCurrentParams();
				isConferenceVideoCapabilityOn = params->videoEnabled();
				if (rcp->videoEnabled() && !!linphone_core_video_enabled(cCore) && !getParams()->videoEnabled()) {
					getParams()->enableVideo(isConferenceVideoCapabilityOn);
				}
			}
		} else {
			if (rcp->videoEnabled() && cCore->video_policy->automatically_accept &&
			    linphone_core_video_enabled(cCore) && !getParams()->videoEnabled()) {
				lInfo() << "CallSession [" << q
				        << "]: re-enabling video in our call params because the remote wants it and the policy allows "
				           "to automatically accept";
				getParams()->setVideoDirection(cCore->video_policy->accept_media_direction);
				getParams()->enableVideo(true);
			}
		}

		// Enable bundle mode in local parameters if remote offered it and core can accept bundle mode, or
		// turn it off if the remote doesn't offer it or rejects it.
		// In fact, we can have the scenario where bundle mode is only enabled by one of the cores in the call or
		// conference. If bundle mode has been accepted, then future reINVITEs or UPDATEs must reoffer bundle mode
		// unless the user has explicitely requested to disable it
		getParams()->enableRtpBundle(
		    fromOffer ? (rcp->rtpBundleEnabled() &&
		                 linphone_config_get_bool(linphone_core_get_config(cCore), "rtp", "accept_bundle", TRUE))
		              : rcp->rtpBundleEnabled());
	}
}

void MediaSessionPrivate::initializeParamsAccordingToIncomingCallParams() {
	L_Q();
	CallSessionPrivate::initializeParamsAccordingToIncomingCallParams();
	const auto remoteContactAddress = q->getRemoteContactAddress();
	const auto localAddress = q->getLocalAddress();
	const auto conference = q->getCore()->findConference(ConferenceId(localAddress, localAddress), false);
	std::shared_ptr<SalMediaDescription> md = op->getRemoteMediaDescription();
	if (md) {
		/* It is implicit to receive an INVITE without SDP, in this case WE choose the media parameters according to
		 * policy */
		setCompatibleIncomingCallParams(md);
	} else if (((!!linphone_core_conference_server_enabled(q->getCore()->getCCore())) && conference) ||
	           (remoteContactAddress && remoteContactAddress->hasParam("isfocus"))) {
		// We enter here when creating a group chat only conference
		lInfo() << "CallSession [" << q
		        << "]: disabling audio and video in our call params because the remote party didn't send a valid SDP";
		getParams()->enableAudio(false);
		getParams()->enableVideo(false);
		getParams()->getPrivate()->enableToneIndications(false);
	}
}

bool MediaSessionPrivate::hasAvpf(const std::shared_ptr<SalMediaDescription> &md) const {
	/* We consider that AVPF is enabled if at least one of these condition is satisfied:
	 * - all the offered streams have AVPF
	 * - the video stream has AVPF.
	 * In practice, this means for a remote media description that AVPF is supported by the far end.
	 */
	bool hasAvpf = !!md->hasAvpf();
	const auto &videoStream = md->findBestStream(SalVideo);
	if (videoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		hasAvpf = videoStream.hasAvpf();
	}
	return hasAvpf;
}

/**
 * Fix call parameters on incoming call to eg. enable AVPF if the incoming call propose it and it is not enabled
 * locally.
 */
void MediaSessionPrivate::setCompatibleIncomingCallParams(std::shared_ptr<SalMediaDescription> &md) {
	L_Q();
	LinphoneCore *lc = q->getCore()->getCCore();
	/* Handle AVPF, SRTP and DTLS */

	getParams()->enableAvpf(hasAvpf(md));
	const auto &account = getDestAccount();
	uint16_t avpfRrInterval;
	if (account) {
		const auto accountParams = account->getAccountParams();
		avpfRrInterval = static_cast<uint16_t>(accountParams->getAvpfRrInterval() * 1000);
	} else avpfRrInterval = static_cast<uint16_t>(linphone_core_get_avpf_rr_interval(lc) * 1000);
	getParams()->setAvpfRrInterval(avpfRrInterval);
	bool_t mandatory = linphone_core_is_media_encryption_mandatory(lc);
	bool_t acceptAllEncryptions = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp",
	                                                        "accept_any_encryption", 0);

	if (md->hasZrtp() && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionZRTP)) {
		if (!mandatory || (mandatory && (acceptAllEncryptions ||
		                                 linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionZRTP)))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionZRTP);
	} else if (md->hasDtls() && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionDTLS)) {
		if (!mandatory || (mandatory && (acceptAllEncryptions ||
		                                 linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionDTLS)))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionDTLS);
	} else if (md->hasSrtp() && linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionSRTP)) {
		if (!mandatory || (mandatory && (acceptAllEncryptions ||
		                                 linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionSRTP)))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionSRTP);
	} else if (getParams()->getMediaEncryption() != LinphoneMediaEncryptionZRTP) {
		if (!mandatory || (mandatory && linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionNone))
			getParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
	}

	const SalStreamDescription &audioStream = md->findBestStream(SalAudio);
	if (audioStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		const std::string &rtpAddr = (audioStream.rtp_addr.empty() == false) ? audioStream.rtp_addr : md->addr;
		if (ms_is_multicast(rtpAddr.c_str())) {
			lInfo() << "Incoming offer has audio multicast, enabling it in local params.";
			getParams()->enableAudioMulticast(true);
		} else getParams()->enableAudioMulticast(false);
	}
	const SalStreamDescription &videoStream = md->findBestStream(SalVideo);
	if (videoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
		const std::string &rtpAddr = (videoStream.rtp_addr.empty() == false) ? videoStream.rtp_addr : md->addr;
		if (ms_is_multicast(rtpAddr.c_str())) {
			lInfo() << "Incoming offer has video multicast, enabling it in local params.";
			getParams()->enableVideoMulticast(true);
		} else getParams()->enableVideoMulticast(false);
	}

	/* In case of nat64, even ipv4 addresses are reachable from v6. Should be enhanced to manage stream by stream
	 * connectivity (I.E v6 or v4) */
	/*if (!md->hasIpv6()){
	    lInfo() << "The remote SDP doesn't seem to offer any IPv6 connectivity, so disabling IPv6 for this call";
	    af = AF_INET;
	}*/
	fixCallParams(md, true);
}

void MediaSessionPrivate::updateBiggestDesc(std::shared_ptr<SalMediaDescription> &md) {
	if (!biggestDesc || (md->streams.size() > biggestDesc->streams.size())) {
		/* We have been offered and now are ready to proceed, or we added a new stream,
		 * store the media description to remember the mapping of streams within this call. */
		biggestDesc = md;
	}
}

void MediaSessionPrivate::updateRemoteSessionIdAndVer() {
	const std::shared_ptr<SalMediaDescription> &desc = op->getRemoteMediaDescription();
	if (desc) {
		remoteSessionId = desc->session_id;
		remoteSessionVer = desc->session_ver;
	}
}

// -----------------------------------------------------------------------------

const LinphoneStreamInternalStats *MediaSessionPrivate::getStreamInternalStats(LinphoneStreamType type) const {
	Stream *s = getStreamsGroup().lookupMainStream(linphone_stream_type_to_sal(type));
	return s ? &s->getInternalStats() : nullptr;
}

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

void MediaSessionPrivate::discoverMtu(const std::shared_ptr<Address> &remoteAddr) {
	L_Q();
	if (q->getCore()->getCCore()->net_conf.mtu == 0) {
		/* Attempt to discover mtu */
		int mtu = ms_discover_mtu(remoteAddr->getDomain().c_str());
		if (mtu > 0) {
			ms_factory_set_mtu(q->getCore()->getCCore()->factory, mtu);
			lInfo() << "Discovered mtu is " << mtu << ", RTP payload max size is "
			        << ms_factory_get_payload_max_size(q->getCore()->getCCore()->factory);
		}
	}
}

int MediaSessionPrivate::getAf() const {
	if (mediaLocalIp.empty()) return AF_UNSPEC;
	return ms_is_ipv6(mediaLocalIp.c_str()) ? AF_INET6 : AF_INET;
}

std::string MediaSessionPrivate::getLocalIpFromRemote(const std::string &remoteAddr) const {
	L_Q();
	string ret;
	/* attempt to convert to addrinfo to verify that we have a numeric ip address (not DNS hostname) */
	int anyport = 8888;
	struct addrinfo *res =
	    bctbx_ip_address_to_addrinfo(linphone_core_ipv6_enabled(q->getCore()->getCCore()) ? AF_INET6 : AF_INET,
	                                 SOCK_DGRAM, remoteAddr.c_str(), anyport);
	if (res) {
		char ipaddress[LINPHONE_IPADDR_SIZE] = {};
		if (bctbx_get_local_ip_for(AF_UNSPEC, remoteAddr.c_str(), anyport, ipaddress, sizeof(ipaddress)) == 0) {
			ret = ipaddress;
		}
		bctbx_freeaddrinfo(res);
	}
	return ret;
}

string MediaSessionPrivate::getLocalIpFromSignaling() const {
	const auto &account = getDestAccount();
	const auto &accountOp = account ? account->getOp() : nullptr;
	string localAddr;

	// If a known proxy was identified for this call, then we may have a chance to take the local ip address
	// from the socket that connects to this proxy
	if (accountOp) {
		const char *ip = accountOp->getLocalAddress(nullptr);
		if (ip) {
			lInfo() << "Found media local-ip from signaling connection: " << ip;
			return ip;
		}
	}
	return "";
}

std::string MediaSessionPrivate::getLocalIpFromMedia() const {
	L_Q();
	string guessedIpAddress;
	if (op && (q->getState() == CallSession::State::Idle || q->getState() == CallSession::State::IncomingReceived ||
	           q->getState() == CallSession::State::IncomingEarlyMedia ||
	           q->getState() == CallSession::State::UpdatedByRemote)) {
		auto remoteDesc = op->getRemoteMediaDescription();
		string remoteAddr;
		if (remoteDesc) {
			remoteAddr = remoteDesc->getConnectionAddress();
			if (remoteAddr.empty()) {
				if (remoteDesc->getNbStreams() > 0) {
					remoteAddr = remoteDesc->getStreamAtIdx(0).rtp_addr;
				}
			}
		}
		if (remoteAddr.empty()) return guessedIpAddress;
		guessedIpAddress = getLocalIpFromRemote(remoteAddr);
		if (!guessedIpAddress.empty()) lInfo() << "Local IP address guessed from SDP is: " << guessedIpAddress;
	}
	return guessedIpAddress;
}

std::string MediaSessionPrivate::getLocalIpFallback(bool preferIpv6) const {
	L_Q();
	char ipv4[LINPHONE_IPADDR_SIZE] = {};
	char ipv6[LINPHONE_IPADDR_SIZE] = {};
	bool haveIpv6 = false;
	bool haveIpv4 = false;
	string localAddr;

	/* as fallback, attempt to guess from the target SIP address */
	auto targetAddress = log->getRemoteAddress();
	if (targetAddress) {
		localAddr = getLocalIpFromRemote(targetAddress->getDomain());
		if (!localAddr.empty()) {
			lInfo() << "Found media local-ip from remote sip address: " << mediaLocalIp;
			return localAddr;
		}
	}
	if (linphone_core_get_local_ip_for(AF_INET, nullptr, ipv4) == 0) haveIpv4 = true;

	if (linphone_core_ipv6_enabled(q->getCore()->getCCore())) {
		if (linphone_core_get_local_ip_for(AF_INET6, nullptr, ipv6) == 0) haveIpv6 = true;
	}
	lInfo() << "Found media local ip address from default routes.";
	if (haveIpv4 && haveIpv6 && !preferIpv6) {
		// This is the case where IPv4 is to be prefered if both are available
		lInfo() << "prefer_ipv6 is set to false, as both IP versions are available we are going to use IPv4";
		return ipv4;
	} else if (haveIpv6) {
		return ipv6;
	}
	return ipv4;
}

std::string MediaSessionPrivate::overrideLocalIpFromConfig(const string &localIp) const {
	L_Q();
	// Legacy bind_address property handling.
	const char *ip =
	    linphone_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "bind_address", nullptr);
	if (ip) return ip;

	/*
	 * Use config-supplied addresses in SDP.
	 * This helps to workaround rare firewall issues (https://bugs.linphone.org/view.php?id=11800), by advertising
	 * wrong IP addresses (for example fd00:0bad:f00d::1 / 10.0.0.10 ).
	 */
	const char *mediaLocalIp4, *mediaLocalIp6;
	mediaLocalIp4 = linphone_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "rtp",
	                                           "ipv4_sdp_address", nullptr);
	mediaLocalIp6 = linphone_config_get_string(linphone_core_get_config(q->getCore()->getCCore()), "rtp",
	                                           "ipv6_sdp_address", nullptr);
	int af = ms_is_ipv6(localIp.c_str()) ? AF_INET6 : AF_INET;
	if (af == AF_INET && mediaLocalIp4) {
		lInfo() << "Local ipv4 address taken from configuration";
		return mediaLocalIp4;
	} else if (af == AF_INET6 && mediaLocalIp6) {
		lInfo() << "Local ipv6 address taken from configuration";
		return mediaLocalIp6;
	}
	return localIp;
}
/**
 * Guess a local IP address to be used for media (SDP).
 * This is "default" one, used when ICE is not activated.
 * It has to be guessed by heuristics, on a best effort basis.
 * Only ICE can determine the ones that really work.
 */
const std::string &MediaSessionPrivate::getMediaLocalIp() const {
	L_Q();
	string ip;

	if (needLocalIpRefresh) {
		needLocalIpRefresh = false;
	} else if (!mediaLocalIp.empty()) {
		return mediaLocalIp;
	}
	bool preferIpv6 =
	    !!linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "prefer_ipv6", TRUE);
	if (!preferIpv6) lInfo() << "rtp/prefer_ipv6 set to false.";
	preferIpv6 = preferIpv6 && linphone_core_ipv6_enabled(q->getCore()->getCCore());

	// Incoming SDP (if available) is the most accurate choice for guessing an appropriate local ip address
	ip = getLocalIpFromMedia();
	if (ip.empty()) {
		// As a secondary choice, use the ip address that was used for SIP connection, if available.
		ip = getLocalIpFromSignaling();
		if (ms_is_ipv6(ip.c_str()) && !preferIpv6) ip = "";
	}
	if (ip.empty()) {
		// Get ip address of default route.
		ip = getLocalIpFallback(preferIpv6);
	}
	// This choice may be ultimately overriden by config.
	ip = overrideLocalIpFromConfig(ip);

	mediaLocalIp = ip;
	lInfo() << "Guessed media local ip address is " << mediaLocalIp;

	return mediaLocalIp;
}

int MediaSessionPrivate::portFromStreamIndex(int index) {
	if (index != -1) {
		auto stream = getStreamsGroup().getStream(index);
		if (stream) return stream->getPortConfig().rtpPort;
	}
	return 0;
}

/*
 * This is the deprecated basic STUN-based IP/port discovery. It is unreliable, we prefer using ICE.
 */
void MediaSessionPrivate::runStunTestsIfNeeded() {
	L_Q();
	if (natPolicy && natPolicy->stunEnabled() && !(natPolicy->iceEnabled() || natPolicy->turnEnabled()) && op) {
		const std::shared_ptr<SalMediaDescription> &md = localIsOfferer ? localDesc : op->getRemoteMediaDescription();
		if (md) {
			const auto audioStreamIndex = md->findIdxBestStream(SalAudio);
			int audioPort = portFromStreamIndex(audioStreamIndex);

			const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
			bool isConferenceLayoutActiveSpeaker = false;
			if (conference) {
				bool isInLocalConference = getParams()->getPrivate()->getInConference();
				const auto &parameters = isInLocalConference ? getRemoteParams() : getParams();
				if (parameters) {
					const auto &confLayout = isInLocalConference ? getRemoteParams()->getConferenceVideoLayout()
					                                             : getParams()->getConferenceVideoLayout();
					isConferenceLayoutActiveSpeaker = (confLayout == ConferenceLayout::ActiveSpeaker);
				}
			}
			const auto mainStreamAttrValue = isConferenceLayoutActiveSpeaker
			                                     ? MediaSessionPrivate::ActiveSpeakerVideoContentAttribute
			                                     : MediaSessionPrivate::GridVideoContentAttribute;
			const auto videoStreamIndex =
			    conference ? md->findIdxStreamWithContent(mainStreamAttrValue) : md->findIdxBestStream(SalVideo);
			int videoPort = portFromStreamIndex(videoStreamIndex);
			const auto textStreamIndex = md->findIdxBestStream(SalText);
			int textPort = portFromStreamIndex(textStreamIndex);
			stunClient = makeUnique<StunClient>(q->getCore());
			int ret = stunClient->run(audioPort, videoPort, textPort);
			if (ret >= 0) pingTime = ret;
		}
	}
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::forceStreamsDirAccordingToState(std::shared_ptr<SalMediaDescription> &md) {
	L_Q();
	for (auto &sd : md->streams) {
		CallSession::State stateToConsider = state;

		switch (stateToConsider) {
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
					if ((sd.type == SalAudio) &&
					    linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
					                            "inactive_audio_on_pause", 0)) {
						lInfo() << "Media session [" << this << "] (local address " << *q->getLocalAddress()
						        << " remote address " << *q->getRemoteAddress()
						        << ") Setting audio direction to inactive when pausing the call as per user setting in "
						           "the linphonerc";

						sd.setDirection(SalStreamInactive);
					}
					if ((sd.type == SalVideo) &&
					    linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
					                            "inactive_video_on_pause", 0)) {
						lInfo() << "Media session [" << this << "] (local address " << *q->getLocalAddress()
						        << " remote address " << *q->getRemoteAddress()
						        << ") Setting video direction to inactive when pausing the call as per user setting in "
						           "the linphonerc";
						sd.setDirection(SalStreamInactive);
					}
				}
				break;
			default:
				break;
		}
		/* Reflect the stream directions in the call params */
		if (sd.getType() == SalAudio) {
			getCurrentParams()->setAudioDirection(sd.getDirection());
		} else if (sd.getType() == SalVideo) {
			const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
			SalStreamDir streamDir = SalStreamInactive;
			bool isInLocalConference = getParams()->getPrivate()->getInConference();
			if (conference) {
				if (isInLocalConference) {
					const auto &currentConfParams = conference->getCurrentParams();
					if (currentConfParams->videoEnabled()) {
						// At least receive the video streams of other participants if video is disabled in the call
						// params
						streamDir = (getParams()->videoEnabled()) ? SalStreamSendRecv : SalStreamRecvOnly;
					} else {
						streamDir = SalStreamInactive;
					}
				} else {
					// Don't check conference parameters for remote conferences because the NOTIFY full state may have
					// not been received yet At least receive the video streams of other participants if video is
					// disabled in the call params
					streamDir = (getParams()->videoEnabled()) ? SalStreamSendRecv : SalStreamRecvOnly;
				}
			} else {
				streamDir = sd.getDirection();
			}
			getCurrentParams()->setVideoDirection(streamDir);
		}
	}
}

bool MediaSessionPrivate::generateB64CryptoKey(size_t keyLength, std::string &keyOut) {
	vector<uint8_t> src = mRng.randomize(keyLength);
	keyOut = bctoolbox::encodeBase64(src);
	if (keyOut.empty()) {
		return false;
	}
	return true;
}

bool MediaSessionPrivate::mandatoryRtpBundleEnabled() const {
	if (!getParams()->rtpBundleEnabled()) return false;
	const auto &account = getDestAccount();
	if (account) {
		return account->getAccountParams()->rtpBundleAssumptionEnabled();
	}
	return false;
}

void MediaSessionPrivate::addStreamToBundle(const std::shared_ptr<SalMediaDescription> &md,
                                            SalStreamDescription &sd,
                                            SalStreamConfiguration &cfg,
                                            const std::string &mid) {
	if (cfg.dir != SalStreamInactive) {
		SalStreamBundle bundle;
		if (!md->bundles.empty()) {
			bundle = md->bundles.front();
			// Delete first element
			md->bundles.erase(md->bundles.begin());
		}
		bundle.addStream(cfg, mid);
		cfg.mid_rtp_ext_header_id = rtpExtHeaderMidNumber;
		/* rtcp-mux must be enabled when bundle mode is proposed.*/
		cfg.rtcp_mux = TRUE;
		if (mandatoryRtpBundleEnabled() || bundleModeAccepted) {
			// Bundle is offered inconditionally
			if (bundle.getMidOfTransportOwner() != mid) {
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
SalMediaProto MediaSessionPrivate::getAudioProto(const std::shared_ptr<SalMediaDescription> remote_md,
                                                 const bool useCurrentParams) const {
	SalMediaProto requested = getAudioProto(useCurrentParams);
	if (remote_md) {
		const SalStreamDescription &remote_stream = remote_md->findBestStream(SalAudio);
		if (!remote_stream.hasAvpf()) {
			switch (requested) {
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
	/*This property is mainly used for testing hybrid case where the SDP offer is made with AVPF only for video
	 * stream.*/
	SalMediaProto ret = useCurrentParams ? linphone_media_encryption_to_sal_media_proto(getNegotiatedMediaEncryption(),
	                                                                                    getParams()->avpfEnabled())
	                                     : getParams()->getMediaProto();
	if (linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "misc", "no_avpf_for_audio",
	                             false)) {
		lInfo() << "Removing AVPF for audio mline.";
		switch (ret) {
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

void MediaSessionPrivate::fillRtpParameters(SalStreamDescription &stream) const {
	L_Q();

	auto &cfg = stream.cfgs[stream.getActualConfigurationIndex()];
	if (cfg.dir != SalStreamInactive) {
		bool rtcpMux =
		    !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_mux", 0);
		/* rtcp-mux must be enabled when bundle mode is proposed or we're using DTLS-SRTP.*/
		cfg.rtcp_mux = rtcpMux || getParams()->rtpBundleEnabled() ||
		               (getNegotiatedMediaEncryption() == LinphoneMediaEncryptionDTLS);
		cfg.rtcp_cname = getMe()->getAddress()->toString();

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

void MediaSessionPrivate::fillConferenceParticipantStream(SalStreamDescription &newStream,
                                                          const std::shared_ptr<SalMediaDescription> &oldMd,
                                                          std::shared_ptr<SalMediaDescription> &md,
                                                          const std::shared_ptr<ParticipantDevice> &dev,
                                                          PayloadTypeHandler &pth,
                                                          const std::list<LinphoneMediaEncryption> &encs,
                                                          SalStreamType type,
                                                          const std::string &mid) {
	L_Q();

	// Declare here an empty list to give to the makeCodecsList if there is no valid already assigned payloads
	std::list<OrtpPayloadType *> emptyList;
	emptyList.clear();

	SalStreamConfiguration cfg;
	cfg.proto = getParams()->getMediaProto();

	newStream.type = type;

	bool bundle_enabled = getParams()->rtpBundleEnabled();
	bool success = false;
	if (dev) {
		bool isVideoStream = (type == SalVideo);
		bool useThumbnailStream = (isVideoStream && q->requestThumbnail(dev));
		const auto &label =
		    (useThumbnailStream) ? dev->getThumbnailStreamLabel() : dev->getLabel(sal_stream_type_to_linphone(type));
		const auto &previousParticipantStream =
		    oldMd ? oldMd->findStreamWithLabel(type, label) : Utils::getEmptyConstRefObject<SalStreamDescription>();
		const auto alreadyAssignedPayloads =
		    ((previousParticipantStream != Utils::getEmptyConstRefObject<SalStreamDescription>())
		         ? previousParticipantStream.already_assigned_payloads
		         : emptyList);

		std::list<OrtpPayloadType *> l = pth.makeCodecsList(type, 0, -1, alreadyAssignedPayloads, bundle_enabled);
		if (!l.empty()) {
			newStream.setLabel(label);
			const auto rtp_port = q->getRandomRtpPort(newStream);
			newStream.rtp_port = rtp_port;
			newStream.rtcp_port = newStream.rtp_port + 1;
			newStream.name = std::string(sal_stream_type_to_string(type)) + " " + dev->getAddress()->toString();
			const auto &content = newStream.getContent();

			const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
			const bool isInLocalConference = getParams()->getPrivate()->getInConference();
			const auto &participantDevice =
			    conference ? (isInLocalConference ? conference->findParticipantDevice(q->getSharedFromThis())
			                                      : conference->getMe()->findDevice(q->getSharedFromThis()))
			               : nullptr;

			auto dir = SalStreamInactive;
			// A participant device can only send a video stream if its video direction has the send component (i.e.
			// SendOnly or SendRecv)
			if (conference && (participantDevice == dev)) {
				const auto mediaDirection =
				    isVideoStream ? getParams()->getVideoDirection() : getParams()->getAudioDirection();
				// If the core is holding the conference, then the device is the one that is active in the session.
				// Otherwise it is me
				if (isInLocalConference) {
					if ((mediaDirection == LinphoneMediaDirectionRecvOnly) ||
					    (mediaDirection == LinphoneMediaDirectionSendRecv)) {
						dir = SalStreamRecvOnly;
					} else {
						dir = SalStreamInactive;
					}
				} else {
					if ((mediaDirection == LinphoneMediaDirectionSendOnly) ||
					    (mediaDirection == LinphoneMediaDirectionSendRecv)) {
						dir = SalStreamSendOnly;
					} else {
						dir = SalStreamInactive;
					}
				}
			} else {
				if (content.compare(MediaSessionPrivate::GridVideoContentAttribute) == 0) {
					dir = (isInLocalConference) ? SalStreamRecvOnly : SalStreamSendOnly;
				} else {
					const auto &mediaDir = dev->getStreamCapability(sal_stream_type_to_linphone(type));
					switch (mediaDir) {
						case LinphoneMediaDirectionSendRecv:
						case LinphoneMediaDirectionSendOnly:
							dir = (isInLocalConference) ? SalStreamSendOnly : SalStreamRecvOnly;
							break;
						case LinphoneMediaDirectionRecvOnly:
						case LinphoneMediaDirectionInactive:
						case LinphoneMediaDirectionInvalid:
							dir = (label.empty()) ? SalStreamInactive : SalStreamRecvOnly;
							break;
					}
				}
			}
			if (dir == SalStreamInactive) {
				lWarning() << *q << "Setting " << std::string(sal_stream_type_to_string(type))
				           << " stream of participant device " << dev->getAddress() << " to inactive (label " << label
				           << " and content " << content
				           << ") because he or she doesn't have the send component in its stream capabilities";
			}
			cfg.dir = dir;
			if (isVideoStream) {
				validateVideoStreamDirection(cfg);
			}
			if (getParams()->rtpBundleEnabled() && (dir != SalStreamInactive))
				addStreamToBundle(md, newStream, cfg, mid);
			cfg.replacePayloads(l);
			newStream.addActualConfiguration(cfg);
			newStream.setSupportedEncryptions(encs);
			fillRtpParameters(newStream);
			success = true;
		}
		PayloadTypeHandler::clearPayloadList(l);
	}

	if (!success) {
		lInfo() << "Don't put video stream for device in conference with address "
		        << (dev ? dev->getAddress()->toString() : "sip:unknown") << " on local offer for CallSession [" << q
		        << "] because no valid payload has been found or device is not valid (pointer " << dev << ")";
		cfg.dir = SalStreamInactive;
		newStream.disable();
		newStream.type = type;
		newStream.rtp_port = 0;
		newStream.rtcp_port = 0;
		newStream.addActualConfiguration(cfg);
	}
}

void MediaSessionPrivate::fillLocalStreamDescription(SalStreamDescription &stream,
                                                     std::shared_ptr<SalMediaDescription> &md,
                                                     const bool enabled,
                                                     const std::string name,
                                                     const SalStreamType type,
                                                     const SalMediaProto proto,
                                                     const SalStreamDir dir,
                                                     const std::list<OrtpPayloadType *> &codecs,
                                                     const std::string mid,
                                                     const SalCustomSdpAttribute *customSdpAttributes) {
	L_Q();

	const auto &dontCheckCodecs =
	    (type == SalAudio)
	        ? q->getCore()->getCCore()->codecs_conf.dont_check_audio_codec_support
	        : ((type == SalVideo) ? q->getCore()->getCCore()->codecs_conf.dont_check_video_codec_support : false);

	SalStreamConfiguration cfg;
	cfg.proto = proto;
	stream.type = type;

	if (enabled && (!codecs.empty() || dontCheckCodecs)) {
		stream.name = name;
		cfg.dir = dir;
		stream.rtp_port = SAL_STREAM_DESCRIPTION_PORT_TO_BE_DETERMINED;

		cfg.replacePayloads(codecs);
		cfg.rtcp_cname = getMe()->getAddress()->toString();

		const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
		if ((type == SalAudio) && isInConference()) {
			bool rtpVolumesAllowed =
			    !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "use_volumes", 1);
			if (rtpVolumesAllowed) {
				cfg.mixer_to_client_extension_id = RTP_EXTENSION_MIXER_TO_CLIENT_AUDIO_LEVEL;
				cfg.client_to_mixer_extension_id = RTP_EXTENSION_CLIENT_TO_MIXER_AUDIO_LEVEL;
			} else {
				lInfo() << "RTP client-to-mixer and mixer-to-client volumes disabled by configuration.";
			}
		} else if ((type == SalVideo) && conference) {
			validateVideoStreamDirection(cfg);

			cfg.frame_marking_extension_id = RTP_EXTENSION_FRAME_MARKING;
		}
		if (getParams()->rtpBundleEnabled()) addStreamToBundle(md, stream, cfg, mid);

		stream.addActualConfiguration(cfg);
		fillRtpParameters(stream);
	} else {
		const std::string streamType(sal_stream_type_to_string(type));
		lInfo() << "Don't put stream of type " << streamType << " on local offer for CallSession [" << q << "]: ";
		lInfo() << "- capability is " << (enabled ? "enabled" : "disabled");
		lInfo() << "- found " << codecs.size() << " codecs";
		lInfo() << "- codec check is " << (dontCheckCodecs ? "disabled" : "enabled");
		const auto &core = q->getCore()->getCCore();
		cfg.dir = linphone_core_get_keep_stream_direction_for_rejected_stream(core) ? dir : SalStreamInactive;
		stream.rtp_port = 0;
		stream.addActualConfiguration(cfg);
	}
	if (customSdpAttributes) stream.custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);
}

std::list<unsigned int> MediaSessionPrivate::getProtectedStreamNumbers(std::shared_ptr<SalMediaDescription> md) {
	L_Q();
	std::list<unsigned int> streamNumbers;
	// Protected streams are only meaningful when the call is in a conference
	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	// Protected streams are the first audio stream and the first 2 video streams as they handle local participant
	// medias
	if (conference) {
		auto firstAudioStream = md->findFirstStreamIdxOfType(SalAudio);
		if (firstAudioStream > -1) {
			streamNumbers.push_back(static_cast<unsigned int>(firstAudioStream));
		}

		const std::string activeSpeakerAttribute(MediaSessionPrivate::ActiveSpeakerVideoContentAttribute);
		const std::string gridAttribute(MediaSessionPrivate::GridVideoContentAttribute);
		const std::string screenSharingAttribute(MediaSessionPrivate::ScreenSharingContentAttribute);

		bool isInLocalConference = getParams()->getPrivate()->getInConference();
		auto participantDevice = isInLocalConference ? conference->findParticipantDevice(q->getSharedFromThis())
		                                             : conference->getMe()->findDevice(q->getSharedFromThis());

		std::string deviceLabel;
		if (participantDevice) {
			deviceLabel = participantDevice->getLabel(LinphoneStreamTypeVideo);
		}

		const auto screenSharingStreamLabel = md->findIdxStreamWithContent(screenSharingAttribute, deviceLabel);
		const auto mainActiveSpeakerVideoStreamLabel =
		    md->findIdxStreamWithContent(activeSpeakerAttribute, deviceLabel);
		const auto mainGridVideoStreamLabel = md->findIdxStreamWithContent(gridAttribute, deviceLabel);
		const auto mainActiveSpeakerVideoStreamNoLabel = md->findIdxStreamWithContent(activeSpeakerAttribute);
		const auto mainGridVideoStreamNoLabel = md->findIdxStreamWithContent(gridAttribute);
		const auto screenSharingStreamNoLabel = md->findIdxStreamWithContent(screenSharingAttribute);
		const auto mainVideoStreamLabel = (screenSharingStreamLabel > -1) ? screenSharingStreamLabel
		                                                                  : ((mainActiveSpeakerVideoStreamLabel > -1)
		                                                                         ? mainActiveSpeakerVideoStreamLabel
		                                                                         : mainGridVideoStreamLabel);
		const auto mainVideoStreamNoLabel =
		    (screenSharingStreamNoLabel > -1)
		        ? screenSharingStreamNoLabel
		        : ((mainActiveSpeakerVideoStreamNoLabel > -1) ? mainActiveSpeakerVideoStreamNoLabel
		                                                      : mainGridVideoStreamNoLabel);
		const auto mainVideoStream = (mainVideoStreamLabel > -1) ? mainVideoStreamLabel : mainVideoStreamNoLabel;
		if (mainVideoStream > -1) {
			streamNumbers.push_back(static_cast<unsigned int>(mainVideoStream));
			const auto thumbnailVideoStreamLabel =
			    md->findIdxStreamWithContent(MediaSessionPrivate::ThumbnailVideoContentAttribute, deviceLabel);
			const auto thumbnailVideoStreamNoLabel =
			    md->findIdxStreamWithContent(MediaSessionPrivate::ThumbnailVideoContentAttribute);
			const auto thumbnailVideoStream =
			    (thumbnailVideoStreamLabel > -1) ? thumbnailVideoStreamLabel : thumbnailVideoStreamNoLabel;
			if (thumbnailVideoStream > -1) {
				streamNumbers.push_back(static_cast<unsigned int>(thumbnailVideoStream));
			}
		}
	}

	return streamNumbers;
}

SalStreamDescription &MediaSessionPrivate::addStreamToMd(std::shared_ptr<SalMediaDescription> md,
                                                         int streamIdx,
                                                         const std::shared_ptr<SalMediaDescription> &oldMd) {
	L_Q();
	const auto currentMdSize = md->streams.size();
	const auto conference = q->getCore()->findConference(q->getSharedFromThis());
	std::list<unsigned int> protectedStreamNumbers = getProtectedStreamNumbers(md);
	std::list<unsigned int> protectedStreamNumbersOldMd;
	if (oldMd) {
		protectedStreamNumbersOldMd = getProtectedStreamNumbers(oldMd);
	}

	// Search for a free slot
	int freeSlot = -1;
	for (size_t mdStreamIdx = 0; mdStreamIdx < currentMdSize; mdStreamIdx++) {
		const auto &protectedIdx = (std::find(protectedStreamNumbers.cbegin(), protectedStreamNumbers.cend(),
		                                      mdStreamIdx) != protectedStreamNumbers.cend());
		auto stream = md->getStreamAtIdx(static_cast<unsigned int>(mdStreamIdx));
		if (!protectedIdx && (stream.getDirection() == SalStreamInactive)) {
			freeSlot = static_cast<int>(mdStreamIdx);
			break;
		}
	}

	if (streamIdx < 0) {
		if (freeSlot < 0) {
			md->streams.resize(currentMdSize + 1);
			return md->streams[currentMdSize];
		} else {
			return md->streams[static_cast<size_t>(freeSlot)];
		}
	} else {
		const auto &idx = static_cast<decltype(md->streams)::size_type>(streamIdx);
		try {
			auto stream = md->streams.at(idx);
			// If a stream at the index requested in the the function argument has already been allocated and it is
			// active, then it must be replaced.
			if ((stream.getDirection() != SalStreamInactive) && oldMd) {
				const auto oldMdSize = oldMd->streams.size();
				int idxOldMd = -1;
				for (size_t mdStreamIdx = 0; mdStreamIdx < oldMdSize; mdStreamIdx++) {
					const auto &protectedIdx =
					    (std::find(protectedStreamNumbersOldMd.cbegin(), protectedStreamNumbersOldMd.cend(),
					               mdStreamIdx) != protectedStreamNumbersOldMd.cend());
					auto oldStream = oldMd->getStreamAtIdx(static_cast<unsigned int>(mdStreamIdx));
					if (conference && !protectedIdx && (oldStream.getLabel() == stream.getLabel())) {
						idxOldMd = static_cast<int>(mdStreamIdx);
						break;
					}
				}
				// If the stream to replace was not in the previous media description, search an inactive stream or
				// append at the end
				if (idxOldMd < 0) {
					// If there no available slot to stored that stream being moved, then put at the end
					if (freeSlot < 0) {
						md->streams.push_back(stream);
					} else {
						md->streams[static_cast<size_t>(freeSlot)] = stream;
					}
				} else {
					auto &streamToFill = addStreamToMd(md, idxOldMd, oldMd);
					streamToFill = stream;
				}
			}
			return md->streams.at(idx);
		} catch (std::out_of_range &) {
			// If a stream at the index requested in the the function argument has not already been allocated, resize
			// the vector list
			lWarning() << "The current media description has only " << currentMdSize
			           << " streams and it has been requested to allocate a stream at index " << idx;
			md->streams.resize(idx + 1);
			if (oldMd) {
				const auto oldMdSize = oldMd->streams.size();
				lWarning() << "Keep the same type as in the previous media description for all newly allocate streams";
				for (decltype(md->streams)::size_type i = currentMdSize; i < idx; i++) {
					auto &c = md->streams[i];
					if (i < oldMdSize) {
						const auto &s = oldMd->streams[i];
						c.type = s.type;
					}
					lWarning() << "Setting " << std::string(sal_stream_type_to_string(c.type))
					           << " stream inactive at index " << i << " because of std::out_of_range.";
					c.setDirection(SalStreamInactive);
				}
			}
			return md->streams.at(idx);
		}
	}
}

void MediaSessionPrivate::addConferenceLocalParticipantStreams(bool add,
                                                               std::shared_ptr<SalMediaDescription> &md,
                                                               const std::shared_ptr<SalMediaDescription> &oldMd,
                                                               PayloadTypeHandler &pth,
                                                               const std::list<LinphoneMediaEncryption> &encs,
                                                               const SalStreamType type) {
	L_Q();
	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	if (conference) {
		if (!getParams()->rtpBundleEnabled()) {
			lWarning()
			    << "In an effort to ensure a satisfactory video conferencing quality, MediaSession [" << q
			    << "] (local address " << *q->getLocalAddress() << " remote address " << *q->getRemoteAddress()
			    << " in conference [" << conference << "] (address: " << *conference->getConferenceAddress()
			    << ") is not sending the thumbnail of the local participant because RTP bundle has been disabled";
			return;
		}
		const auto &currentConfParams = conference->getCurrentParams();
		bool isVideoConferenceEnabled = currentConfParams->videoEnabled();
		if (((type == SalVideo) && isVideoConferenceEnabled) || (type == SalAudio)) {
			std::list<OrtpPayloadType *> emptyList;
			bool isInLocalConference = getParams()->getPrivate()->getInConference();
			const auto &participantDevice = isInLocalConference
			                                    ? conference->findParticipantDevice(q->getSharedFromThis())
			                                    : conference->getMe()->findDevice(q->getSharedFromThis());
			if (participantDevice) {
				const auto &deviceState = participantDevice->getState();
				std::string content;
				std::string deviceLabel;
				if (type == SalAudio) {
					content = MediaSessionPrivate::DTXAudioContentAttribute;
					deviceLabel = participantDevice->getLabel(sal_stream_type_to_linphone(type));
				} else if (type == SalVideo) {
					content = MediaSessionPrivate::ThumbnailVideoContentAttribute;
					deviceLabel = participantDevice->getThumbnailStreamLabel();
				}
				bool isConferenceServer = !!linphone_core_conference_server_enabled(q->getCore()->getCCore());
				// A conference server is a passive element, therefore it must not add any stream that has not requested
				// by the client For this reason it always looks at the remote media description, if there is one. If
				// this media session is part of an ad hoc meeting, then the conference server will not request the
				// client thumbnail stream because the latter offered it
				const auto &refMd = (isConferenceServer) ? op->getRemoteMediaDescription() : oldMd;
				const auto &foundStreamIdx = refMd ? refMd->findIdxStreamWithContent(content, deviceLabel) : -1;
				const auto addStream =
				    ((foundStreamIdx != -1) ||
				     (localIsOfferer && !isConferenceServer && (deviceState == ParticipantDevice::State::Joining)) ||
				     (deviceState == ParticipantDevice::State::Present) ||
				     (deviceState == ParticipantDevice::State::OnHold));
				if (addStream) {
					SalStreamDescription &newStream = addStreamToMd(md, foundStreamIdx, oldMd);
					SalStreamConfiguration cfg;

					newStream.type = type;
					newStream.setContent(content);

					if (!deviceLabel.empty()) {
						newStream.setLabel(deviceLabel);
					}

					cfg.proto = getParams()->getMediaProto();

					bool bundle_enabled = getParams()->rtpBundleEnabled();
					std::list<OrtpPayloadType *> l = pth.makeCodecsList(
					    type, 0, -1,
					    (((foundStreamIdx >= 0) && localIsOfferer)
					         ? oldMd->streams[static_cast<decltype(oldMd->streams)::size_type>(foundStreamIdx)]
					               .already_assigned_payloads
					         : emptyList),
					    bundle_enabled);
					if (!l.empty()) {
						cfg.replacePayloads(l);
						newStream.name = "Thumbnail " + std::string(sal_stream_type_to_string(type)) + " " +
						                 participantDevice->getAddress()->toString();
						const auto mediaDirection =
						    (type == SalVideo) ? getParams()->getVideoDirection() : getParams()->getAudioDirection();
						auto dir = SalStreamInactive;
						if (add) {
							if (isInLocalConference) {
								if ((mediaDirection == LinphoneMediaDirectionRecvOnly) ||
								    (mediaDirection == LinphoneMediaDirectionSendRecv)) {
									dir = SalStreamRecvOnly;
								} else {
									dir = SalStreamInactive;
								}
							} else {
								// Do not offer own thumbnail if the camera is disabled
								if (q->getMediaParams()->cameraEnabled() &&
								    ((mediaDirection == LinphoneMediaDirectionSendOnly) ||
								     (mediaDirection == LinphoneMediaDirectionSendRecv))) {
									dir = SalStreamSendOnly;
								} else {
									dir = SalStreamInactive;
								}
							}
						}
						cfg.dir = dir;
						if (type == SalVideo) {
							validateVideoStreamDirection(cfg);

							if (!isInLocalConference) cfg.frame_marking_extension_id = RTP_EXTENSION_FRAME_MARKING;
						}
						if (bundle_enabled) {
							const std::string bundleNamePrefix((type == SalVideo) ? "vs" : "as");
							const std::string bundleName(bundleNamePrefix + std::string("Me") + content);
							addStreamToBundle(md, newStream, cfg, bundleName);
						}
					} else {
						lInfo() << "Don't put " << std::string(sal_stream_type_to_string(type))
						        << " stream for device in conference with address " << *participantDevice->getAddress()
						        << " on local offer for CallSession [" << q << "]";
						cfg.dir = SalStreamInactive;
					}
					PayloadTypeHandler::clearPayloadList(l);
					newStream.addActualConfiguration(cfg);
					newStream.setSupportedEncryptions(encs);
					fillRtpParameters(newStream);
				}
			}
		}
	}
}

void MediaSessionPrivate::addConferenceParticipantStreams(std::shared_ptr<SalMediaDescription> &md,
                                                          const std::shared_ptr<SalMediaDescription> &oldMd,
                                                          PayloadTypeHandler &pth,
                                                          const std::list<LinphoneMediaEncryption> &encs,
                                                          const SalStreamType type) {
	L_Q();
	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	if (conference) {
		const auto &currentConfParams = conference->getCurrentParams();
		bool isVideoConferenceEnabled = currentConfParams->videoEnabled();
		bool isVideoStream = (type == SalVideo);

		// Add additional video streams if required
		if ((isVideoStream && isVideoConferenceEnabled) || (type == SalAudio)) {
			bool isInLocalConference = getParams()->getPrivate()->getInConference();
			const auto &parameters = isInLocalConference ? getRemoteParams() : getParams();
			if (!parameters) {
				lInfo() << "Not adding streams of type " << std::string(sal_stream_type_to_string(type))
				        << " because the layout is not known yet";
				return;
			}
			const auto &confLayout = parameters->getConferenceVideoLayout();
			bool isConferenceLayoutActiveSpeaker = (confLayout == ConferenceLayout::ActiveSpeaker);
			const auto remoteContactAddress = q->getRemoteContactAddress();
			q->updateContactAddressInOp();
			const auto &participantDeviceAddress =
			    (isInLocalConference) ? remoteContactAddress : q->getContactAddress();

			if (localIsOfferer && !linphone_core_conference_server_enabled(q->getCore()->getCCore())) {
				bool request = true;
				if (isVideoStream) {
					request = conference->areThumbnailsRequested(false);
				}
				if (request) {
					const std::string participantContent(
					    (type == SalAudio)
					        ? MediaSessionPrivate::DTXAudioContentAttribute
					        : ((isConferenceLayoutActiveSpeaker) ? MediaSessionPrivate::ThumbnailVideoContentAttribute
					                                             : ""));
					const std::string bundleNameStreamPrefix((isVideoStream) ? "vs" : "as");
					for (const auto &p : conference->getParticipants()) {
						for (const auto &dev : p->getDevices()) {
							const auto &devAddress = dev->getAddress();
							const auto &devState = dev->getState();
							bool useThumbnailStream = (isVideoStream && q->requestThumbnail(dev));
							const auto mediaDirection =
							    useThumbnailStream ? dev->getThumbnailStreamCapability()
							                       : dev->getStreamCapability(sal_stream_type_to_linphone(type));
							const auto mediaAvailable = ((mediaDirection == LinphoneMediaDirectionSendOnly) ||
							                             (mediaDirection == LinphoneMediaDirectionSendRecv));
							const auto stateOk = (devState != ParticipantDevice::State::ScheduledForJoining) &&
							                     (devState != ParticipantDevice::State::Joining) &&
							                     (devState != ParticipantDevice::State::Alerting);
							// Add only streams of participants that have accepted to join the conference
							if ((participantDeviceAddress != devAddress) && mediaAvailable && stateOk) {
								const auto &devLabel = useThumbnailStream
								                           ? dev->getThumbnailStreamLabel()
								                           : dev->getLabel(sal_stream_type_to_linphone(type));
								// main stream has the same label as one of the thumbnail streams
								const auto &foundStreamIdx =
								    devLabel.empty() ? -1
								                     : oldMd->findIdxStreamWithContent(participantContent, devLabel);
								SalStreamDescription &newParticipantStream = addStreamToMd(md, foundStreamIdx, oldMd);
								if (isConferenceLayoutActiveSpeaker || (type == SalAudio)) {
									newParticipantStream.setContent(participantContent);
								}
								const auto mid(bundleNameStreamPrefix + devLabel);
								fillConferenceParticipantStream(newParticipantStream, oldMd, md, dev, pth, encs, type,
								                                mid);
								if (isVideoStream) {
									try {
										auto &cfg = newParticipantStream.cfgs.at(
										    newParticipantStream.getActualConfigurationIndex());
										cfg.frame_marking_extension_id = RTP_EXTENSION_FRAME_MARKING;
									} catch (std::out_of_range &) {
										lError()
										    << "Trying to set frame marking extension to incorrect configuration index";
									}
								}
							}
						}
					}
				} else {
					if (getParams()->rtpBundleEnabled()) {
						lWarning() << "MediaSession [" << q << "] (local address " << *q->getLocalAddress()
						           << " remote address " << *q->getRemoteAddress() << " in conference [" << conference
						           << "] (address: " << *conference->getConferenceAddress()
						           << ") is not requesting other participants' camera stream because their number "
						           << conference->getParticipantCount()
						           << " is bigger than the maximum allowed for this core "
						           << linphone_core_get_conference_max_thumbnails(q->getCore()->getCCore());
					} else {
						lWarning() << "In an effort to ensure a satisfactory video conferencing quality, MediaSession ["
						           << q << "] (local address " << *q->getLocalAddress() << " remote address "
						           << *q->getRemoteAddress() << " in conference [" << conference
						           << "] (address: " << *conference->getConferenceAddress()
						           << ") is not requesting other participants' camera stream because RTP bundle has "
						              "been disabled";
					}
				}
			} else {
				// The conference server is a passive core, therefore he must not add any stream to the SDP
				// Do not attempt to modify already existing streams
				const auto &refMd =
				    (linphone_core_conference_server_enabled(q->getCore()->getCCore()) && localIsOfferer)
				        ? oldMd
				        : op->getRemoteMediaDescription();

				// By default, every participant has a main stream and a thumbnail to send its video stream to the
				// conference server
				const unsigned int protectedStreams = (type == SalVideo) ? 2 : 1;
				unsigned int nbStream = 0;

				auto beginIt = refMd->streams.cbegin();
				for (auto sIt = beginIt; sIt != refMd->streams.end(); sIt++) {
					const auto &s = *sIt;
					if (s.getType() == type) {
						nbStream++;
						if (nbStream > protectedStreams) {

							const auto idx = std::distance(refMd->streams.cbegin(), sIt);
							const std::string contentAttrValue = s.getContent();
							const std::string participantsAttrValue = s.getLabel();

							std::shared_ptr<ParticipantDevice> dev = nullptr;
							if (!participantsAttrValue.empty()) {
								dev = conference->findParticipantDeviceByLabel(sal_stream_type_to_linphone(type),
								                                               participantsAttrValue);
								if (!dev && conference->getMe()) {
									// It might be me
									dev = conference->getMe()->findDevice(sal_stream_type_to_linphone(type),
									                                      participantsAttrValue, false);
								}
							}

							SalStreamDescription &newStream = addStreamToMd(md, static_cast<int>(idx), oldMd);
							if (dev) {
								newStream.setContent(contentAttrValue);
								fillConferenceParticipantStream(newStream, oldMd, md, dev, pth, encs, type, s.getMid());
							} else {
								const auto &s = refMd->streams[static_cast<size_t>(idx)];
								SalStreamConfiguration cfg;
								cfg.dir = SalStreamInactive;
								newStream.disable();
								newStream.type = s.type;
								newStream.rtp_port = 0;
								newStream.rtcp_port = 0;
								newStream.addActualConfiguration(cfg);
								lWarning() << *q
								           << ": New stream added as disabled and inactive because no device has been "
								              "found with label "
								           << participantsAttrValue << " in conference "
								           << *conference->getConferenceAddress();
							}
						}
					}
				}
			}
		}
	}
}

void MediaSessionPrivate::validateVideoStreamDirection(SalStreamConfiguration &cfg) const {
	L_Q();
	// Check core parameter to eventually change the media direction
	// video capture enables sending the stream
	// video display enables receiving the stream
	// capture | display | allowed directions
	//   false |  false  |  Inactive
	//   false |   true  |  RecvOnly, Inactive
	//   true  |  false  |  SendOnly, Inactive
	//   true  |   true  |  SendOnly, RecvOnly, SendRecv, Inactive
	const auto &cCore = q->getCore()->getCCore();
	const auto captureEnabled = !!linphone_core_video_capture_enabled(cCore);
	const auto displayEnabled = !!linphone_core_video_display_enabled(cCore);
	const auto oldVideoDir = cfg.dir;
	if (((oldVideoDir == SalStreamSendOnly) && !captureEnabled) ||
	    ((oldVideoDir == SalStreamRecvOnly) && !displayEnabled)) {
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
		lWarning() << "Video direction of a video stream has been changed from "
		           << std::string(sal_stream_dir_to_string(oldVideoDir)) << " to "
		           << std::string(sal_stream_dir_to_string(cfg.dir)) << " as video capture is "
		           << std::string(captureEnabled ? "enabled" : "disabled") << " and video display is "
		           << std::string(displayEnabled ? "enabled" : "disabled") << " in the core settings";
	}
}

void MediaSessionPrivate::copyOldStreams(std::shared_ptr<SalMediaDescription> &md,
                                         const std::shared_ptr<SalMediaDescription> &oldMd,
                                         const std::shared_ptr<SalMediaDescription> &refMd,
                                         const std::list<LinphoneMediaEncryption> &encs) {
	if (refMd) {
		// Declare here an empty list to give to the makeCodecsList if there is no valid already assigned payloads
		std::list<OrtpPayloadType *> emptyList;
		emptyList.clear();
		const auto noStreams = md->streams.size();
		const auto refNoStreams = refMd->streams.size();
		if (noStreams <= refNoStreams) {
			// Copy participant video streams from previous local description
			auto beginIt = refMd->streams.cbegin();
			std::advance(beginIt, static_cast<decltype(beginIt)::difference_type>(noStreams));

			for (auto sIt = beginIt; sIt != refMd->streams.end(); sIt++) {
				const auto &s = *sIt;
				const auto &idx = static_cast<int>(md->streams.size());
				SalStreamDescription &newStream = addStreamToMd(md, idx, oldMd);
				newStream.rtp_port = 0;
				newStream.rtcp_port = 0;
				newStream.type = s.type;
				newStream.name = s.name;
				newStream.disable();
				SalStreamConfiguration cfg;
				cfg.proto = s.getProto();
				cfg.dir = SalStreamInactive;
				lWarning() << "Disabling stream at index " << idx << " as it is no longer needed";

				newStream.setSupportedEncryptions(encs);
				newStream.addActualConfiguration(cfg);
			}
		}
	}
}

void MediaSessionPrivate::makeLocalMediaDescription(bool localIsOfferer,
                                                    const bool supportsCapabilityNegotiationAttributes,
                                                    const bool offerNegotiatedMediaProtocolOnly,
                                                    const bool forceCryptoKeyGeneration) {

	L_Q();
	const auto &core = q->getCore()->getCCore();
	bool isInLocalConference = getParams()->getPrivate()->getInConference();

	SalMediaDescriptionParams descParams(getParams());
	descParams.enableCapabilityNegotiationSupport(supportsCapabilityNegotiationAttributes);
	std::shared_ptr<SalMediaDescription> md = std::make_shared<SalMediaDescription>(descParams);
	std::shared_ptr<SalMediaDescription> &oldMd = localDesc;
	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	const std::shared_ptr<SalMediaDescription> &refMd = ((localIsOfferer) ? oldMd : op->getRemoteMediaDescription());

	this->localIsOfferer = localIsOfferer;

	getParams()->getPrivate()->adaptToNetwork(core, pingTime);

	string subject = q->getParams()->getSessionName();
	if (!subject.empty()) {
		md->name = subject;
	}
	md->session_id = (oldMd ? oldMd->session_id : (bctbx_random() & 0xfff));
	md->session_ver = (oldMd ? (oldMd->session_ver + 1) : (bctbx_random() & 0xfff));

	md->times.push_back(std::make_pair<time_t, time_t>(getParams()->getPrivate()->getStartTime(),
	                                                   getParams()->getPrivate()->getEndTime()));

	bool rtpBundleEnabled = getParams()->rtpBundleEnabled();
	md->accept_bundles = rtpBundleEnabled;

	if (getParams()->recordAwareEnabled() || linphone_core_is_record_aware_enabled(core)) {
		md->record = getParams()->getRecordingState();
	}

	md->origin_addr = getMediaLocalIp();
	md->addr = getMediaLocalIp();

	Address addr;
	const auto &account = getDestAccount();
	if (account) {
		const auto accountParams = account->getAccountParams();
		addr = *accountParams->getIdentityAddress();
	} else {
		addr = Address(linphone_core_get_identity(core));
	}
	if (!addr.getUsername().empty()) { /* Might be null in case of identity without userinfo */
		md->username = addr.getUsername();
	}

	int bandwidth = getParams()->getPrivate()->getDownBandwidth();
	if (bandwidth) md->bandwidth = bandwidth;
	else md->bandwidth = linphone_core_get_download_bandwidth(core);

	SalCustomSdpAttribute *customSdpAttributes = getParams()->getPrivate()->getCustomSdpAttributes();
	if (customSdpAttributes) md->custom_sdp_attributes = sal_custom_sdp_attribute_clone(customSdpAttributes);

	PayloadTypeHandler pth(q->getCore());

	// Declare here an empty list to give to the makeCodecsList if there is no valid already assigned payloads
	std::list<OrtpPayloadType *> emptyList;
	emptyList.clear();
	auto encList = q->getSupportedEncryptions();
	// Delete duplicates
	encList.unique();
	// Do not add capability negotiation attributes if encryption is mandatory
	const bool addCapabilityNegotiationAttributes =
	    supportsCapabilityNegotiationAttributes && !linphone_core_is_media_encryption_mandatory(core);
	if (addCapabilityNegotiationAttributes) {
		for (const auto &enc : encList) {
			const std::string mediaProto(sal_media_proto_to_string(
			    linphone_media_encryption_to_sal_media_proto(enc, (getParams()->avpfEnabled() ? TRUE : FALSE))));
			const auto &idx = md->getFreeTcapIdx();

			const auto &tcaps = md->getTcaps();
			const auto &tcapFoundIt = std::find_if(tcaps.cbegin(), tcaps.cend(), [&mediaProto](const auto &cap) {
				return (mediaProto.compare(cap.second) == 0);
			});

			if (tcapFoundIt == tcaps.cend()) {
				lInfo() << "Adding media protocol " << mediaProto << " at index " << idx << " for encryption "
				        << linphone_media_encryption_to_string(enc);
				md->addTcap(idx, mediaProto);
			} else {
				lInfo() << "Media protocol " << mediaProto << " is already found at " << tcapFoundIt->first
				        << " hence a duplicate will not be added to the tcap list";
			}
		}
	}

	encList.push_back(getParams()->getMediaEncryption());

	bool conferenceCreated = false;
	bool isConferenceLayoutActiveSpeaker = false;
	bool isVideoConferenceEnabled = false;
	bool isAudioConferenceEnabled = false;
	ConferenceLayout confLayout = ConferenceLayout::ActiveSpeaker;
	auto deviceState = ParticipantDevice::State::ScheduledForJoining;
	std::shared_ptr<ParticipantDevice> participantDevice = nullptr;
	if (conference) {
		const auto &currentConfParams = conference->getCurrentParams();
		const auto &conferenceState = conference->getState();

		participantDevice = isInLocalConference ? conference->findParticipantDevice(q->getSharedFromThis())
		                                        : conference->getMe()->findDevice(q->getSharedFromThis());
		if (participantDevice) {
			deviceState = participantDevice->getState();
		}

		isVideoConferenceEnabled = currentConfParams->videoEnabled();
		confLayout = (participantDevice && isInLocalConference && getRemoteParams())
		                 ? getRemoteParams()->getConferenceVideoLayout()
		                 : getParams()->getConferenceVideoLayout();
		isAudioConferenceEnabled = currentConfParams->audioEnabled();
		isConferenceLayoutActiveSpeaker = (confLayout == ConferenceLayout::ActiveSpeaker);
		if (isInLocalConference) {
			// If the conference is dialing out to participants and an internal update (i.e. ICE reINVITE or capability
			// negotiations reINVITE) occurs
			if (getParams()->getPrivate()->getInternalCallUpdate() && (direction == LinphoneCallOutgoing)) {
				conferenceCreated = (deviceState == ParticipantDevice::State::Present);
			} else {
				conferenceCreated = !((conferenceState == ConferenceInterface::State::Instantiated) ||
				                      (conferenceState == ConferenceInterface::State::CreationPending));
			}
		} else {
			conferenceCreated = !((conferenceState == ConferenceInterface::State::Instantiated) ||
			                      (conferenceState == ConferenceInterface::State::CreationPending));
		}
	}

	auto callAudioEnabled = (!conferenceCreated && conference && !CallSession::isEarlyState(q->getState()))
	                            ? getCurrentParams()->audioEnabled()
	                            : getParams()->audioEnabled();
	bool addAudioStream = callAudioEnabled;
	// Check if there was a main stream earlier on in the SDP.
	// It is necessary to check for both Grid and ActiveSpeaker layout in order to cover the case when the layout is
	// changed
	if (conference) {
		if (isInLocalConference) {
			addAudioStream &= isAudioConferenceEnabled;
		}
	}
	const SalStreamDescription &oldAudioStream =
	    refMd ? refMd->findBestStream(SalAudio) : Utils::getEmptyConstRefObject<SalStreamDescription>();

	if (addAudioStream || (oldAudioStream != Utils::getEmptyConstRefObject<SalStreamDescription>())) {
		auto audioCodecs = pth.makeCodecsList(SalAudio, getParams()->getAudioBandwidthLimit(), -1,
		                                      ((oldAudioStream != Utils::getEmptyConstRefObject<SalStreamDescription>())
		                                           ? oldAudioStream.already_assigned_payloads
		                                           : emptyList),
		                                      rtpBundleEnabled);

		const auto audioStreamIdx = (conference && refMd) ? refMd->findIdxBestStream(SalAudio) : -1;
		SalStreamDescription &audioStream = addStreamToMd(md, audioStreamIdx, oldMd);
		SalStreamDir audioDir = getParams()->getPrivate()->getSalAudioDirection();
		fillLocalStreamDescription(
		    audioStream, md, getParams()->audioEnabled(), "Audio", SalAudio,
		    getAudioProto(op ? op->getRemoteMediaDescription() : nullptr, offerNegotiatedMediaProtocolOnly), audioDir,
		    audioCodecs, "as", getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeAudio));

		auto &actualCfg = audioStream.cfgs[audioStream.getActualConfigurationIndex()];

		audioStream.setSupportedEncryptions(encList);
		actualCfg.max_rate = pth.getMaxCodecSampleRate(audioCodecs);
		int downPtime = getParams()->getPrivate()->getDownPtime();
		if (downPtime) actualCfg.ptime = downPtime;
		else actualCfg.ptime = linphone_core_get_download_ptime(core);

		PayloadTypeHandler::clearPayloadList(audioCodecs);

		if (conference) {
			// The call to getRemoteContactAddress updates CallSessionPrivate member remoteContactAddress
			const auto remoteContactAddress = q->getRemoteContactAddress();
			if (participantDevice && (isInLocalConference || (!isInLocalConference && remoteContactAddress &&
			                                                  remoteContactAddress->hasParam("isfocus")))) {
				audioStream.setLabel(participantDevice->getLabel(LinphoneStreamTypeAudio));
			}
		}
	}

	const auto params = isInLocalConference ? q->getRemoteParams() : q->getMediaParams();
	bool isScreenSharing = params ? params->screenSharingEnabled() : false;
	std::string mainStreamAttrValue;
	if (isScreenSharing) {
		mainStreamAttrValue = MediaSessionPrivate::ScreenSharingContentAttribute;
	} else if (isConferenceLayoutActiveSpeaker) {
		mainStreamAttrValue = MediaSessionPrivate::ActiveSpeakerVideoContentAttribute;
	} else {
		mainStreamAttrValue = MediaSessionPrivate::GridVideoContentAttribute;
	}

	auto callVideoEnabled =
	    (!conferenceCreated && conference) ? getCurrentParams()->videoEnabled() : getParams()->videoEnabled();
	bool addVideoStream = callVideoEnabled;
	// If the call is linked to a conference, search stream with content main first
	// Check if there was a main stream earlier on in the SDP.
	// It is necessary to check for both Grid and ActiveSpeaker layout in order to cover the case when the layout is
	// changed
	const SalStreamDescription &oldScreenSharingMainVideoStream =
	    refMd ? refMd->findStreamWithContent(MediaSessionPrivate::ScreenSharingContentAttribute)
	          : Utils::getEmptyConstRefObject<SalStreamDescription>();
	const SalStreamDescription &oldGridLayoutMainVideoStream =
	    refMd ? refMd->findStreamWithContent(MediaSessionPrivate::GridVideoContentAttribute)
	          : Utils::getEmptyConstRefObject<SalStreamDescription>();
	const SalStreamDescription &oldActiveSpeakerLayoutMainVideoStream =
	    refMd ? refMd->findStreamWithContent(MediaSessionPrivate::ActiveSpeakerVideoContentAttribute)
	          : Utils::getEmptyConstRefObject<SalStreamDescription>();
	const SalStreamDescription &oldMainVideoStream =
	    refMd ? refMd->findBestStream(SalVideo) : Utils::getEmptyConstRefObject<SalStreamDescription>();
	if (conference && isInLocalConference) {
		addVideoStream &= isVideoConferenceEnabled;
	}

	SalStreamDescription oldVideoStream;
	if (conference) {
		if (oldScreenSharingMainVideoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
			oldVideoStream = oldScreenSharingMainVideoStream;
		} else if (oldGridLayoutMainVideoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
			oldVideoStream = oldGridLayoutMainVideoStream;
		} else {
			oldVideoStream = oldActiveSpeakerLayoutMainVideoStream;
		}
	} else {
		oldVideoStream = oldMainVideoStream;
	}
	if (addVideoStream || (oldVideoStream != Utils::getEmptyConstRefObject<SalStreamDescription>())) {
		auto videoCodecs = pth.makeCodecsList(SalVideo, 0, -1,
		                                      ((oldVideoStream != Utils::getEmptyConstRefObject<SalStreamDescription>())
		                                           ? oldVideoStream.already_assigned_payloads
		                                           : emptyList),
		                                      rtpBundleEnabled);
		const auto proto = offerNegotiatedMediaProtocolOnly
		                       ? linphone_media_encryption_to_sal_media_proto(getNegotiatedMediaEncryption(),
		                                                                      getParams()->avpfEnabled())
		                       : getParams()->getMediaProto();

		SalStreamDir videoDir = SalStreamInactive;
		bool enableVideoStream = false;
		// Set direction appropriately to configuration
		if (conference) {
			auto [enableVideoStreamTmp, linphoneVideoDir] = conference->getMainStreamVideoDirection(
			    q->getSharedFromThis(), localIsOfferer, conferenceCreated || !isInLocalConference);
			enableVideoStream = enableVideoStreamTmp;
			videoDir = MediaSessionParamsPrivate::mediaDirectionToSalStreamDir(linphoneVideoDir);
			if (videoDir == SalStreamInactive) {
				lWarning() << *q << "Video may have been enable in the local media parameters but device "
				           << ((participantDevice) ? participantDevice->getAddress()->toString()
				                                   : std::string("sip:unknown"))
				           << " may not be able to send video streams or an old video stream is just being copied";
				enableVideoStream = false;
			}

		} else {
			videoDir = getParams()->getPrivate()->getSalVideoDirection();
			enableVideoStream = addVideoStream;
		}

		int videoStreamIdx = -1;
		if (refMd) {
			const auto gridStreamIdxWithContent =
			    refMd->findIdxStreamWithContent(MediaSessionPrivate::GridVideoContentAttribute);
			const auto activeSpeakerStreamIdxWithContent =
			    refMd->findIdxStreamWithContent(MediaSessionPrivate::ActiveSpeakerVideoContentAttribute);
			const auto screenSharingStreamIdxWithContent =
			    refMd->findIdxStreamWithContent(MediaSessionPrivate::ScreenSharingContentAttribute);
			if (conference) {
				if (screenSharingStreamIdxWithContent > -1) {
					videoStreamIdx = screenSharingStreamIdxWithContent;
				} else if (gridStreamIdxWithContent > -1) {
					videoStreamIdx = gridStreamIdxWithContent;
				} else if (activeSpeakerStreamIdxWithContent > -1) {
					videoStreamIdx = activeSpeakerStreamIdxWithContent;
				} else {
					videoStreamIdx = refMd->findIdxBestStream(SalVideo);
				}
			} else {
				videoStreamIdx = refMd->findIdxBestStream(SalVideo);
			}
		}

		SalStreamDescription &videoStream = addStreamToMd(md, videoStreamIdx, oldMd);
		fillLocalStreamDescription(videoStream, md, enableVideoStream, "Video", SalVideo, proto, videoDir, videoCodecs,
		                           "vs",
		                           getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeVideo));

		if (conference) {
			// The call to getRemoteContactAddress updates CallSessionPrivate member remoteContactAddress
			const auto remoteContactAddress = q->getRemoteContactAddress();
			if (participantDevice && (isInLocalConference || (!isInLocalConference && remoteContactAddress &&
			                                                  remoteContactAddress->hasParam("isfocus")))) {
				std::string label;
				// The stream label is not known yet when a participant starts screen sharing
				if (isInLocalConference) {
					label = participantDevice->getLabel(LinphoneStreamTypeVideo);
				}
				videoStream.setLabel(label);
				videoStream.setContent(mainStreamAttrValue);
			}
		}

		videoStream.setSupportedEncryptions(encList);

		PayloadTypeHandler::clearPayloadList(videoCodecs);

		addConferenceLocalParticipantStreams(addVideoStream, md, oldMd, pth, encList, SalVideo);
	}

	const SalStreamDescription &oldTextStream =
	    refMd ? refMd->findBestStream(SalText) : Utils::getEmptyConstRefObject<SalStreamDescription>();
	if ((localIsOfferer && getParams()->realtimeTextEnabled()) ||
	    (oldTextStream != Utils::getEmptyConstRefObject<SalStreamDescription>())) {
		auto textCodecs = pth.makeCodecsList(SalText, 0, -1,
		                                     ((oldTextStream != Utils::getEmptyConstRefObject<SalStreamDescription>())
		                                          ? oldTextStream.already_assigned_payloads
		                                          : emptyList),
		                                     rtpBundleEnabled);

		const auto proto = offerNegotiatedMediaProtocolOnly
		                       ? linphone_media_encryption_to_sal_media_proto(getNegotiatedMediaEncryption(),
		                                                                      getParams()->avpfEnabled())
		                       : getParams()->getMediaProto();

		const auto textStreamIdx = refMd ? refMd->findIdxBestStream(SalText) : -1;
		SalStreamDescription &textStream = addStreamToMd(md, textStreamIdx, oldMd);
		fillLocalStreamDescription(textStream, md, getParams()->realtimeTextEnabled(), "Text", SalText, proto,
		                           SalStreamSendRecv, textCodecs, "ts",
		                           getParams()->getPrivate()->getCustomSdpMediaAttributes(LinphoneStreamTypeText));
		textStream.setSupportedEncryptions(encList);
		PayloadTypeHandler::clearPayloadList(textCodecs);
	}

#ifdef HAVE_ADVANCED_IM
	bool eventLogEnabled = !!linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "misc",
	                                                  "conference_event_log_enabled", TRUE);
	if (conferenceCreated && eventLogEnabled && participantDevice &&
	    ((deviceState == ParticipantDevice::State::Joining) || (deviceState == ParticipantDevice::State::Present) ||
	     (deviceState == ParticipantDevice::State::OnHold))) {
		if (addVideoStream) {
			addConferenceParticipantStreams(md, oldMd, pth, encList, SalVideo);
		}
	}
#endif // HAVE_ADVANCED_IM
	copyOldStreams(md, oldMd, refMd, encList);

	setupEncryptionKeys(md, forceCryptoKeyGeneration, offerNegotiatedMediaProtocolOnly);
	setupImEncryptionEngineParameters(md);
	setupRtcpFb(md);
	setupRtcpXr(md);
	if (stunClient) stunClient->updateMediaDescription(md);
	if (md->streams.size() > 0) {
		localDesc = md;

		OfferAnswerContext ctx;
		ctx.localMediaDescription = localDesc;
		ctx.remoteMediaDescription = localIsOfferer ? nullptr : (op ? op->getRemoteMediaDescription() : nullptr);
		ctx.localIsOfferer = localIsOfferer;
		/* Now instanciate the streams according to the media description. */
		getStreamsGroup().createStreams(ctx);

		const auto &mdForMainStream = localIsOfferer ? md : refMd;
		const auto audioStreamIndex = mdForMainStream->findIdxBestStream(SalAudio);
		if (audioStreamIndex != -1) getStreamsGroup().setStreamMain(static_cast<size_t>(audioStreamIndex));
		const auto remoteContactAddress = q->getRemoteContactAddress();
		const auto videoStreamIndex =
		    (conference || (remoteContactAddress && remoteContactAddress->hasParam("isfocus")))
		        ? mdForMainStream->findIdxStreamWithContent(mainStreamAttrValue)
		        : mdForMainStream->findIdxBestStream(SalVideo);
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
				 * However, the localDesc may change between first INVITE and ICE reINVITE, for example if the remote
				 * party has declined a video stream. We use the internalCallUpdate flag to prevent trigger an
				 * unnecessary media restart.
				 */
				localDescChanged = 0;
			}
		}
		forceStreamsDirAccordingToState(md);
	} else {
		localDesc = nullptr;
	}
	forceStreamsDirAccordingToState(md);
	lInfo() << "makeLocalMediaDescription: address = " << (localDesc ? localDesc->addr : std::string("sip:unknown"));
	if (op) {
		lInfo() << "Local media description assigned to op " << op;
		op->setLocalMediaDescription(localDesc);
	}
}

int MediaSessionPrivate::setupEncryptionKey(SalSrtpCryptoAlgo &crypto, MSCryptoSuite suite, unsigned int tag) {
	crypto.tag = tag;
	crypto.algo = suite;
	size_t keylen = 0;
	switch (suite) {
		case MS_AES_128_SHA1_80:
		case MS_AES_128_SHA1_32:
		case MS_AES_128_SHA1_80_NO_AUTH:
		case MS_AES_128_SHA1_32_NO_AUTH:
		case MS_AES_128_SHA1_80_SRTP_NO_CIPHER:  /* Not sure for this one */
		case MS_AES_128_SHA1_80_SRTCP_NO_CIPHER: /* Not sure for this one */
		case MS_AES_128_SHA1_80_NO_CIPHER:       /* Not sure for this one */
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
	if ((keylen == 0) || !generateB64CryptoKey(keylen, crypto.master_key)) {
		lError() << "Could not generate SRTP key";
		crypto.algo = MS_CRYPTO_SUITE_INVALID;
		return -1;
	}
	return 0;
}

void MediaSessionPrivate::setupRtcpFb(std::shared_ptr<SalMediaDescription> &md) {
	L_Q();
	for (auto &stream : md->streams) {
		stream.setupRtcpFb(!!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp",
		                                             "rtcp_fb_generic_nack_enabled", 0),
		                   !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp",
		                                             "rtcp_fb_tmmbr_enabled", 1),
		                   getParams()->getPrivate()->implicitRtcpFbEnabled());
		for (const auto &pt : stream.getPayloads()) {
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

void MediaSessionPrivate::setupRtcpXr(std::shared_ptr<SalMediaDescription> &md) {
	L_Q();
	md->rtcp_xr.enabled =
	    !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "rtp", "rtcp_xr_enabled", 1);
	if (md->rtcp_xr.enabled) {
		const char *rcvr_rtt_mode = linphone_config_get_string(linphone_core_get_config(q->getCore()->getCCore()),
		                                                       "rtp", "rtcp_xr_rcvr_rtt_mode", "all");
		if (strcasecmp(rcvr_rtt_mode, "all") == 0) md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttAll;
		else if (strcasecmp(rcvr_rtt_mode, "sender") == 0) md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttSender;
		else md->rtcp_xr.rcvr_rtt_mode = OrtpRtcpXrRcvrRttNone;
		if (md->rtcp_xr.rcvr_rtt_mode != OrtpRtcpXrRcvrRttNone)
			md->rtcp_xr.rcvr_rtt_max_size = linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()),
			                                                        "rtp", "rtcp_xr_rcvr_rtt_max_size", 10000);
		md->rtcp_xr.stat_summary_enabled = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()),
		                                                             "rtp", "rtcp_xr_stat_summary_enabled", 1);
		if (md->rtcp_xr.stat_summary_enabled)
			md->rtcp_xr.stat_summary_flags = OrtpRtcpXrStatSummaryLoss | OrtpRtcpXrStatSummaryDup |
			                                 OrtpRtcpXrStatSummaryJitt | OrtpRtcpXrStatSummaryTTL;
		md->rtcp_xr.voip_metrics_enabled = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()),
		                                                             "rtp", "rtcp_xr_voip_metrics_enabled", 1);

		for (auto &stream : md->streams) {
			stream.setupRtcpXr(md->rtcp_xr);
		}
	}
}

void MediaSessionPrivate::setupImEncryptionEngineParameters(std::shared_ptr<SalMediaDescription> &md) {
	L_Q();
	auto encryptionEngine = q->getCore()->getEncryptionEngine();
	if (!encryptionEngine) return;

	list<EncryptionParameter> paramList = encryptionEngine->getEncryptionParameters(getDestAccount());

	// Loop over IM Encryption Engine parameters and append them to the SDP
	for (const auto &[name, value] : paramList) {
		lInfo() << "Appending " << name << " parameter to SDP attributes";
		md->custom_sdp_attributes =
		    sal_custom_sdp_attribute_append(md->custom_sdp_attributes, name.c_str(), value.c_str());
	}
}

void MediaSessionPrivate::setupEncryptionKeys(std::shared_ptr<SalMediaDescription> &md,
                                              const bool forceKeyGeneration,
                                              bool addOnlyAcceptedKeys) {
	L_Q();
	std::shared_ptr<SalMediaDescription> &oldMd = localDesc;
	bool keepSrtpKeys =
	    !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip", "keep_srtp_keys", 1);
	const std::string attrName("crypto");
	for (size_t i = 0; i < md->streams.size(); i++) {

		auto &newStream = md->streams[i];

		// Make best effort to keep same keys if user wishes so
		if (newStream.enabled()) {
			auto &newStreamActualCfg = newStream.cfgs[newStream.getActualConfigurationIndex()];
			auto &newStreamActualCfgCrypto = newStreamActualCfg.crypto;

			if (keepSrtpKeys && oldMd && (i < oldMd->streams.size()) && oldMd->streams[i].enabled()) {
				const auto &oldStream = oldMd->streams[i];
				const auto &oldStreamSupportedEncryptions = oldStream.getSupportedEncryptions();
				const bool oldStreamSupportsSrtp =
				    (std::find(oldStreamSupportedEncryptions.cbegin(), oldStreamSupportedEncryptions.cend(),
				               LinphoneMediaEncryptionSRTP) != oldStreamSupportedEncryptions.cend());
				const auto newStreamSupportedEncryptions = newStream.getSupportedEncryptions();
				const bool newStreamSupportsSrtp =
				    (std::find(newStreamSupportedEncryptions.cbegin(), newStreamSupportedEncryptions.cend(),
				               LinphoneMediaEncryptionSRTP) != newStreamSupportedEncryptions.cend());
				const auto &oldStreamActualCfg = oldStream.getActualConfiguration();
				const auto &oldStreamActualCfgCrypto = oldStreamActualCfg.crypto;

				// Actual configuration
				if (newStreamActualCfg.hasSrtp()) {
					if (forceKeyGeneration) {
						// Generate new crypto keys
						newStreamActualCfgCrypto = generateNewCryptoKeys();
					} else if (oldStreamActualCfg.hasSrtp()) {
						if ((state == CallSession::State::IncomingReceived) && params) {
							// Attempt to reuse the keys generated during the first offer answer execution that allowed
							// the remote to ring
							lInfo() << "Merging already created crypto suites with the ones of the call parameters";
							newStreamActualCfgCrypto = generateNewCryptoKeys(oldStreamActualCfgCrypto);
						} else {
							// If old stream actual configuration supported SRTP, then copy crypto parameters
							lInfo() << "Keeping same crypto keys when making new local stream description";
							newStreamActualCfgCrypto = oldStreamActualCfgCrypto;
						}
					} else if (oldMd->getParams().capabilityNegotiationSupported()) {
						const std::list<std::list<unsigned int>> acapIdx =
						    oldStream.getChosenConfiguration().getAcapIndexes();
						// Search crypto attributes in acaps if previous media description did support capability
						// negotiations Copy acap crypto attributes if old stream supports it as potential configuration
						for (const auto &[idx, nameValuePair] : oldStream.acaps) {
							// If only negotiated keys should be added, then check acaps in the chosen configuration
							if (addOnlyAcceptedKeys) {
								bool found = false;
								for (const auto &acapSet : acapIdx) {
									found |= (std::find(acapSet.cbegin(), acapSet.cend(), idx) != acapSet.end());
								}
								if (!found) {
									continue;
								}
							}
							const auto &[name, value] = nameValuePair;
							if (name.compare(attrName) == 0) {
								const auto keyEnc = SalStreamConfiguration::fillStrpCryptoAlgoFromString(value);
								if (keyEnc.algo != MS_CRYPTO_SUITE_INVALID) {
									newStreamActualCfgCrypto.push_back(keyEnc);
								}
							}
						}
					} else {
						newStreamActualCfgCrypto = generateNewCryptoKeys();
					}
				}

				// If capability negotiation is enabled, search keys among acaps
				if (md->getParams().capabilityNegotiationSupported()) {
					// If both old and new stream support SRTP as potential configuration
					if (newStreamSupportsSrtp && !forceKeyGeneration) {
						if (oldStreamSupportsSrtp && oldMd->getParams().capabilityNegotiationSupported()) {
							// Copy acap crypto attributes if old stream supports it as potential configuration
							for (const auto &[idx, nameValuePair] : oldStream.acaps) {
								const auto &[name, value] = nameValuePair;
								if (name.compare(attrName) == 0) {
									newStream.addAcap(idx, name, value);
								}
							}
						} else if (oldStreamActualCfg.hasSrtp()) {
							// Copy crypto attributes from actual configuration if old stream supports it as actual
							// configuration
							for (const auto &c : oldStreamActualCfgCrypto) {
								MSCryptoSuiteNameParams desc;
								if (ms_crypto_suite_to_name_params(c.algo, &desc) == 0) {
									const auto &idx = md->getFreeAcapIdx();
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
				lInfo() << "Don't put stream " << i << " on local offer for CallSession [" << q
				        << "] because it requires protocol " << sal_media_proto_to_string(newStreamActualCfg.getProto())
				        << " but no suitable crypto key has been found.";
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
		case LinphoneSrtpSuiteAESCM128HMACSHA180:
			return MS_AES_128_SHA1_80;
		case LinphoneSrtpSuiteAESCM128HMACSHA132:
			return MS_AES_128_SHA1_32;
		case LinphoneSrtpSuiteAES256CMHMACSHA180:
			return MS_AES_256_SHA1_80;
		case LinphoneSrtpSuiteAES256CMHMACSHA132:
			return MS_AES_256_SHA1_32;
		case LinphoneSrtpSuiteAEADAES128GCM:
			return MS_AEAD_AES_128_GCM;
		case LinphoneSrtpSuiteAEADAES256GCM:
			return MS_AEAD_AES_256_GCM;
		// all these case are not supported by the MS enumeration
		case LinphoneSrtpSuiteAES192CMHMACSHA180:
		case LinphoneSrtpSuiteAES192CMHMACSHA132:
		case LinphoneSrtpSuiteInvalid:
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
	for (const auto suite : suites) {
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
		case MS_AES_128_SHA1_32:
			return LinphoneSrtpSuiteAESCM128HMACSHA132;
		case MS_AES_128_SHA1_80:
			return LinphoneSrtpSuiteAESCM128HMACSHA180;
		case MS_AES_256_SHA1_32:
			return LinphoneSrtpSuiteAES256CMHMACSHA132;
		case MS_AES_256_SHA1_80:
		case MS_AES_CM_256_SHA1_80:
			return LinphoneSrtpSuiteAES256CMHMACSHA180;
		case MS_AEAD_AES_128_GCM:
			return LinphoneSrtpSuiteAEADAES128GCM;
		case MS_AEAD_AES_256_GCM:
			return LinphoneSrtpSuiteAEADAES256GCM;
		// all these cases are not supported by the linphone enumeration
		case MS_AES_128_SHA1_32_NO_AUTH:
		case MS_AES_128_SHA1_80_NO_AUTH:
		case MS_AES_128_SHA1_80_SRTP_NO_CIPHER:
		case MS_AES_128_SHA1_80_SRTCP_NO_CIPHER:
		case MS_AES_128_SHA1_80_NO_CIPHER:
		case MS_CRYPTO_SUITE_INVALID:
		default:
			return LinphoneSrtpSuiteInvalid;
	}
}

unsigned int MediaSessionPrivate::generateCryptoTag(const std::vector<SalSrtpCryptoAlgo> &cryptos) {
	L_Q();
	auto cryptoId = linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
	                                        "crypto_suite_tag_starting_value", 1);
	unsigned int cryptoTag = 0;
	// crypto tag lower than 1 is not valid
	if (cryptoId < 1) {
		lWarning() << "Trying to set initial value of the crypto tag suite to a value lower than 1: " << cryptoId
		           << ". Automatically fixing it by setting it to 1";
		cryptoTag = 1;
	} else {
		cryptoTag = static_cast<unsigned int>(cryptoId);
	}
	if (cryptos.empty()) {
		return cryptoTag;
	}
	bool found = false;
	do {
		const auto &crypto = std::find_if(cryptos.cbegin(), cryptos.cend(),
		                                  [&cryptoTag](const auto &crypto) { return (crypto.tag == cryptoTag); });

		found = (crypto != cryptos.cend());
		if (found) {
			cryptoTag++;
		}
	} while (found);
	return cryptoTag;
}

std::vector<SalSrtpCryptoAlgo>
MediaSessionPrivate::generateNewCryptoKeys(const std::vector<SalSrtpCryptoAlgo> oldCryptos) {
	L_Q();
	std::vector<SalSrtpCryptoAlgo> cryptos;
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
	if (suites != nullptr) {
		for (size_t j = 0; (suites != nullptr) && (suites[j] != MS_CRYPTO_SUITE_INVALID); j++) {
			suitesList.push_back(suites[j]);
		}
	}

	for (auto suite : suitesList) {
		const auto &oldCrypto = std::find_if(oldCryptos.cbegin(), oldCryptos.cend(),
		                                     [&suite](const auto &crypto) { return (crypto.algo == suite); });
		if (oldCrypto != oldCryptos.cend()) {
			cryptos.push_back(*oldCrypto);
		} else if (doNotUseParams || !isEncryptionMandatory() ||
		           (isEncryptionMandatory() && !ms_crypto_suite_is_unencrypted(suite))) {
			SalSrtpCryptoAlgo newCrypto;
			auto cryptoTag = generateCryptoTag(cryptos);
			setupEncryptionKey(newCrypto, suite, cryptoTag);
			cryptos.push_back(newCrypto);
		} else if (isEncryptionMandatory() && ms_crypto_suite_is_unencrypted(suite)) {
			lWarning() << "Not offering " << std::string(ms_crypto_suite_to_string(suite))
			           << " because either RTP or RTCP streams is not encrypted";
		} else {
			lWarning() << "Not offering " << std::string(ms_crypto_suite_to_string(suite));
		}
	}

	return cryptos;
}

void MediaSessionPrivate::transferAlreadyAssignedPayloadTypes(std::shared_ptr<SalMediaDescription> &oldMd,
                                                              std::shared_ptr<SalMediaDescription> &md) {
	for (size_t i = 0; i < md->streams.size(); i++) {
		if (i < oldMd->streams.size()) {
			md->streams[i].already_assigned_payloads = std::move(oldMd->streams[i].already_assigned_payloads);
			oldMd->streams[i].already_assigned_payloads.clear();
		} else {
			md->streams[i].already_assigned_payloads.clear();
		}
	}
}

void MediaSessionPrivate::updateLocalMediaDescriptionFromIce(bool localIsOfferer) {
	if (localDesc) {
		OfferAnswerContext ctx;
		ctx.localMediaDescription = localDesc;
		ctx.remoteMediaDescription = op ? op->getRemoteMediaDescription() : nullptr;
		ctx.localIsOfferer = localIsOfferer;
		getStreamsGroup().fillLocalMediaDescription(ctx);
	}
	if (op) op->setLocalMediaDescription(localDesc);
}

void MediaSessionPrivate::performMutualAuthentication() {
	L_Q();

	// Perform mutual authentication if instant messaging encryption is enabled
	auto encryptionEngine = q->getCore()->getEncryptionEngine();
	// Is call direction really relevant ? might be linked to offerer/answerer rather than call direction ?
	const std::shared_ptr<SalMediaDescription> &md =
	    localIsOfferer ? op->getLocalMediaDescription() : op->getRemoteMediaDescription();
	const auto audioStreamIndex = md->findIdxBestStream(SalAudio);
	Stream *stream = audioStreamIndex != -1 ? getStreamsGroup().getStream(audioStreamIndex) : nullptr;
	MS2AudioStream *ms2a = dynamic_cast<MS2AudioStream *>(stream);
	if (encryptionEngine && ms2a && ms2a->getZrtpContext()) {
		encryptionEngine->mutualAuthentication(ms2a->getZrtpContext(), op->getLocalMediaDescription(),
		                                       op->getRemoteMediaDescription(), q->getDirection());
	}
}

/*
 * Frees the media resources of the call.
 * This has to be done at the earliest, unlike signaling resources that sometimes need to be kept a bit more longer.
 * It is called by setTerminated() (for termination of calls signaled to the application), or directly by the destructor
 * of the session if it was never notified to the application.
 */
void MediaSessionPrivate::freeResources() {
	getStreamsGroup().finish();
}

void MediaSessionPrivate::queueIceCompletionTask(const std::function<LinphoneStatus()> &lambda) {
	iceDeferedCompletionTasks.push(lambda);
}

void MediaSessionPrivate::runIceCompletionTasks() {
	L_Q();
	while (!iceDeferedCompletionTasks.empty()) {
		const auto task = iceDeferedCompletionTasks.front();
		LinphoneStatus result = task();
		iceDeferedCompletionTasks.pop();
		if (result != 0) {
			q->addPendingAction(task);
		}
	}
}
void MediaSessionPrivate::queueIceGatheringTask(const std::function<LinphoneStatus()> &lambda) {
	iceDeferedGatheringTasks.push(lambda);
}

void MediaSessionPrivate::runIceGatheringTasks() {
	L_Q();
	while (!iceDeferedGatheringTasks.empty()) {
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
	return (getNegotiatedMediaEncryption() == LinphoneMediaEncryptionDTLS)
	           ? linphone_config_get_bool(linphone_core_get_config(core), "sip",
	                                      "update_call_when_ice_completed_with_dtls", false)
	           : !!linphone_config_get_int(linphone_core_get_config(core), "sip", "update_call_when_ice_completed",
	                                       true);
}

/*
 * IceServiceListener implementation
 */
void MediaSessionPrivate::onGatheringFinished(BCTBX_UNUSED(IceService &service)) {
	lInfo() << "Finished gathering candidates";
	runIceGatheringTasks();
}

void MediaSessionPrivate::onIceCompleted(BCTBX_UNUSED(IceService &service)) {
	L_Q();
	/* The ICE session has succeeded, so perform a call update */
	if (!getStreamsGroup().getIceService().hasCompletedCheckList()) return;
	if (getStreamsGroup().getIceService().isControlling() && isUpdateSentWhenIceCompleted()) {
		switch (state) {
			case CallSession::State::StreamsRunning:
			case CallSession::State::Paused:
			case CallSession::State::PausedByRemote: {
				lInfo() << "Sending reINVITE for Media session (local address " << *q->getLocalAddress()
				        << " remote address " << *q->getRemoteAddress()
				        << ") because ICE negotiation has completed successfully.";
				MediaSessionParams newParams(*getParams());
				newParams.getPrivate()->setInternalCallUpdate(true);
				q->update(&newParams, CallSession::UpdateMethod::Default, q->isCapabilityNegotiationEnabled());
			} break;
			default:
				lWarning() << "Cannot send reINVITE for ICE during state " << state;
				break;
		}
	}
	runIceCompletionTasks();
}

void MediaSessionPrivate::onLosingPairsCompleted(BCTBX_UNUSED(IceService &service)) {
	if (state == CallSession::State::UpdatedByRemote) {
		if (incomingIceReinvitePending) {
			lInfo() << "Finished adding losing pairs, ICE re-INVITE can be answered.";
			startAcceptUpdate(prevState, Utils::toString(prevState));
			incomingIceReinvitePending = false;
		}
	}
}

void MediaSessionPrivate::onIceRestartNeeded(BCTBX_UNUSED(IceService &service)) {
	L_Q();
	getStreamsGroup().getIceService().restartSession(IR_Controlling);
	MediaSessionParams newParams(*getParams());
	q->update(&newParams, CallSession::UpdateMethod::Default, q->isCapabilityNegotiationEnabled());
}

void MediaSessionPrivate::tryEarlyMediaForking(std::shared_ptr<SalMediaDescription> &md) {
	OfferAnswerContext ctx;
	ctx.localMediaDescription = op->getLocalMediaDescription();
	ctx.remoteMediaDescription = md;
	ctx.resultMediaDescription = resultDesc;
	lInfo()
	    << "Early media response received from another branch, checking if media can be forked to this new destination";
	getStreamsGroup().tryEarlyMediaForking(ctx);
}

void MediaSessionPrivate::updateStreamFrozenPayloads(SalStreamDescription &resultDesc,
                                                     SalStreamDescription &localStreamDesc) {
	L_Q();
	for (const auto &pt : resultDesc.getPayloads()) {
		if (PayloadTypeHandler::isPayloadTypeNumberAvailable(localStreamDesc.already_assigned_payloads,
		                                                     payload_type_get_number(pt), nullptr)) {
			/* New codec, needs to be added to the list */
			localStreamDesc.already_assigned_payloads.push_back(payload_type_clone(pt));
			lInfo() << "CallSession[" << q << "] : payload type " << payload_type_get_number(pt) << " " << pt->mime_type
			        << "/" << pt->clock_rate << " fmtp=" << L_C_TO_STRING(pt->recv_fmtp) << " added to frozen list";
		}
	}
}

void MediaSessionPrivate::updateFrozenPayloads(std::shared_ptr<SalMediaDescription> &result) {
	const auto localMediaDesc = op->getLocalMediaDescription();
	for (size_t i = 0; i < result->streams.size(); i++) {
		if (i < localMediaDesc->streams.size()) {
			updateStreamFrozenPayloads(result->streams[i], localMediaDesc->streams[i]);
		} else {
			lError() << "Local media description has " << localMediaDesc->streams.size() << " whereas result has "
			         << result->streams.size();
		}
	}
}

void MediaSessionPrivate::updateStreams(std::shared_ptr<SalMediaDescription> &newMd, CallSession::State targetState) {
	L_Q();

	if (!newMd) {
		lError() << "updateStreams() called with null media description";
		return;
	}

	if (q->getStreamsGroup().getAuthenticationTokenCheckDone() &&
	    !q->getStreamsGroup().getAuthenticationTokenVerified()) {
		lError() << "updateStreams() The ZRTP authentication token check failed, unable to update the streams";
		return;
	}

	updateBiggestDesc(localDesc);
	resultDesc = newMd;

	// Encryption may have changed during the offer answer process and not being the default one. Typical example of
	// this scenario is when capability negotiation is enabled and if ZRTP is only enabled on one side and the other
	// side supports it
	if (newMd->isEmpty()) {
		lInfo() << "All streams have been rejected, hence negotiated media encryption keeps being "
		        << linphone_media_encryption_to_string(negotiatedEncryption);
	} else {
		negotiatedEncryption = getEncryptionFromMediaDescription(newMd);
		lInfo() << "Negotiated media encryption is " << linphone_media_encryption_to_string(negotiatedEncryption);
		// There is no way to signal that ZRTP was enabled in the SDP and it is automatically accepted by Linphone even
		// if it was not offered in the first place Attribute zrtp-hash is not mandatory
		if (!q->isCapabilityNegotiationEnabled() && (negotiatedEncryption == LinphoneMediaEncryptionZRTP) &&
		    (getParams()->getMediaEncryption() == LinphoneMediaEncryptionNone)) {
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
	if (!getParams()->getPrivate()->isConferenceCreation()) {
		getStreamsGroup().render(ctx, targetState);
	}

	updateFrozenPayloads(newMd);
	upBandwidth = linphone_core_get_upload_bandwidth(q->getCore()->getCCore());
}

// -----------------------------------------------------------------------------

bool MediaSessionPrivate::allStreamsAvpfEnabled() const {
	return getStreamsGroup().avpfEnabled();
}

bool MediaSessionPrivate::allStreamsEncrypted() const {
	return getStreamsGroup().allStreamsEncrypted();
}

bool MediaSessionPrivate::atLeastOneStreamStarted() const {
	return getStreamsGroup().isStarted();
}

uint16_t MediaSessionPrivate::getAvpfRrInterval() const {
	return (uint16_t)getStreamsGroup().getAvpfRrInterval();
}

unsigned int MediaSessionPrivate::getNbActiveStreams() const {
	return (unsigned int)getStreamsGroup().getActiveStreamsCount();
}

bool MediaSessionPrivate::isEncryptionMandatory() const {
	L_Q();
	if (getNegotiatedMediaEncryption() == LinphoneMediaEncryptionDTLS) {
		lInfo() << "Forced encryption mandatory on CallSession [" << q << "] due to SRTP-DTLS";
		return true;
	}
	return getParams()->mandatoryMediaEncryptionEnabled();
}

void MediaSessionPrivate::propagateEncryptionChanged() {
	L_Q();

	auto oldEncryptionStatus = mEncryptionStatus;

	string authToken = getStreamsGroup().getAuthenticationToken();
	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	bool isInLocalConference = getParams()->getPrivate()->getInConference();
	bool isInRemoteConference = conference && !isInLocalConference;
	bool isInConference = (isInLocalConference || isInRemoteConference);
	// If the media session is part of a conference, the client has no way to check the token, hence do not pass it on
	// to the application
	string callbackAuthToken = (conference) ? std::string() : authToken;

	if (callbackAuthToken.empty() && !authToken.empty()) {
		getStreamsGroup().setAuthTokenVerified(true);
	}
	bool authTokenVerified = getStreamsGroup().getAuthenticationTokenVerified();
	bool cacheMismatch = getStreamsGroup().getZrtpCacheMismatch();
	bool zrtpCheckDone = getStreamsGroup().getAuthenticationTokenCheckDone();
	if (!getStreamsGroup().allStreamsEncrypted()) {
		lInfo() << "Some streams are not encrypted";
		getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
		q->notifyEncryptionChanged(false, callbackAuthToken);
	} else {
		if (!authToken.empty()) {
			/* ZRTP only is using auth_token */
			getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionZRTP);
			if (!isInConference) {
				q->getCore()->getPrivate()->getToneManager().stopSecurityAlert();
			}
			auto encryptionEngine = q->getCore()->getEncryptionEngine();
			if (encryptionEngine && authTokenVerified && !cacheMismatch) {
				const SalAddress *remoteAddress = getOp()->getRemoteContactAddress();
				if (remoteAddress) {
					char *peerDeviceId = sal_address_as_string_uri_only(remoteAddress);
					Stream *stream = getStreamsGroup().lookupMainStream(SalAudio);
					if (stream) {
						MS2Stream *ms2s = dynamic_cast<MS2Stream *>(stream);
						if (ms2s) {
							encryptionEngine->authenticationVerified(ms2s->getZrtpContext(),
							                                         op->getRemoteMediaDescription(), peerDeviceId);
						} else {
							lError() << "Could not dynamic_cast to MS2Stream in propagateEncryptionChanged().";
						}
					}
					ms_free(peerDeviceId);
				} else {
					/* This typically happens if the ZRTP session starts during early-media when receiving a 183
					 * response. Indeed the Contact header is not mandatory in 183 (and liblinphone does not set it). */
					lError() << "EncryptionEngine cannot be notified of verified status because remote contact address "
					            "is unknown.";
				}
				q->notifyEncryptionChanged(true, callbackAuthToken);
			} else {
				// ZRTP is enabled and we need to check the SAS, a security alert tones
				if (!isInConference) {
					q->getCore()->getPrivate()->getToneManager().notifySecurityAlert(q->getSharedFromThis());
				}
				if (!zrtpCheckDone) {
					q->notifyEncryptionChanged(true, callbackAuthToken);
				}
			}
		} else {
			/* Otherwise it must be DTLS as SDES doesn't go through this function */
			getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionDTLS);
		}

		lInfo() << "All streams are encrypted, key exchanged using "
		        << ((q->getCurrentParams()->getMediaEncryption() == LinphoneMediaEncryptionZRTP) ? "ZRTP"
		            : (q->getCurrentParams()->getMediaEncryption() == LinphoneMediaEncryptionDTLS)
		                ? "DTLS"
		                : "Unknown mechanism");
		if (q->getCurrentParams()->getMediaEncryption() != LinphoneMediaEncryptionZRTP) {
			q->notifyEncryptionChanged(true, callbackAuthToken);
		}

		Stream *videoStream = getStreamsGroup().lookupMainStream(SalVideo);
		if (isEncryptionMandatory() && videoStream && videoStream->getState() == Stream::Running) {
			/* Nothing could have been sent yet so generating key frame */
			VideoControlInterface *vc = dynamic_cast<VideoControlInterface *>(videoStream);
			if (vc) vc->sendVfu();
		}
	}

	mEncryptionStatus = getStreamsGroup().getEncryptionStatus();
	if (mEncryptionStatus.isDowngradedComparedTo(oldEncryptionStatus)) {
		lInfo() << __func__ << " : Security level downgraded";
		q->notifySecurityLevelDowngraded();
	}
}

void MediaSessionPrivate::skipZrtpAuthentication() {
	L_Q();
	q->getCore()->getPrivate()->getToneManager().stopSecurityAlert();
}

MSWebCam *MediaSessionPrivate::getVideoDevice() const {
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
	q->notifyLossOfMediaDetected();
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::abort(const string &errorMsg) {
	stopStreams();
	CallSessionPrivate::abort(errorMsg);
}

void MediaSessionPrivate::handleIncomingReceivedStateInIncomingNotification() {
	L_Q();
	auto logContext = getLogContextualizer();
	/* Try to be best-effort in giving real local or routable contact address for 100Rel case */
	setContactOp();
	if (notifyRinging) {
		bool proposeEarlyMedia = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
		                                                   "incoming_calls_early_media", false);
		if (proposeEarlyMedia) q->acceptEarlyMedia();
		else if (state != CallSession::State::IncomingEarlyMedia) {
			op->notifyRinging(false, linphone_core_get_tag_100rel_support_level(q->getCore()->getCCore()));
		}
	}

	acceptOrTerminateReplacedSessionInIncomingNotification();
}

bool MediaSessionPrivate::canSoundResourcesBeFreed() const {
	return ((state == CallSession::State::StreamsRunning) || (state == CallSession::State::PausedByRemote));
}

LinphoneStatus MediaSessionPrivate::pause() {
	L_Q();
	if (state == CallSession::State::Paused) {
		lWarning() << "Media session (local address " << *q->getLocalAddress() << " remote address "
		           << *q->getRemoteAddress() << ") is in state " << Utils::toString(state) << " is already paused";
		return 0;
	} else if (state == CallSession::State::Pausing) {
		lWarning() << "Media session (local address " << *q->getLocalAddress() << " remote address "
		           << *q->getRemoteAddress() << ") is in state " << Utils::toString(state)
		           << " is already in the process of being paused";
		return 0;
	} else if (!canSoundResourcesBeFreed()) {
		lWarning() << "Media session (local address " << *q->getLocalAddress() << " remote address "
		           << *q->getRemoteAddress() << ") is in state " << Utils::toString(state)
		           << " hence it cannot be paused";
		return -1;
	}

	bool isInLocalConference = getParams()->getPrivate()->getInConference();
	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	if (isInLocalConference) {
		const auto contactAddress = q->getContactAddress();

		if (!!linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "misc",
		                               "conference_event_log_enabled", TRUE) &&
		    contactAddress && contactAddress->hasParam("isfocus")) {
			if (conference) {
				if (conference->findParticipantDevice(q->getSharedFromThis())) {
					lWarning() << "Unable to pause media session (local address " << *q->getLocalAddress()
					           << " remote address " << *q->getRemoteAddress()
					           << ") because it is part of a conference. Please use the dedicated conference API to "
					              "execute the desired actions";
					return -1;
				}
			} else {
				lWarning() << "The contact address " << *contactAddress
				           << " of the call has isfocus attribute however it doesn't seems to be part of a conference.";
			}
		}

		params->getPrivate()->setInConference(false);
		q->updateContactAddressInOp();

		if (conference) {
			lInfo() << "Removing participant with session " << q << " (local addres " << *q->getLocalAddress()
			        << " remote address " << *q->getRemoteAddress() << ")  from conference "
			        << *conference->getConferenceAddress();
			// Do not preserve conference after removing the participant
			conference->removeParticipant(q->getSharedFromThis(), false);
			return 0;
		}
	}

	string subject;
	if (!conference) {
		if (resultDesc->hasDir(SalStreamSendRecv)) {
			subject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::CallOnHold);
		} else if (resultDesc->hasDir(SalStreamRecvOnly) ||
		           (resultDesc->hasDir(SalStreamInactive) &&
		            state == CallSession::State::PausedByRemote)) { // Stream is inactive from Remote
			subject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::BothPartiesOnHold);
		} else {
			lError() << "No reason to pause this call, it is already paused or inactive";
			return -1;
		}
	}
	broken = false;
	stopStreams();
	setState(CallSession::State::Pausing, "Pausing call");

	auto retryableAction = [this, subject]() {
		makeLocalMediaDescription(true, false, true);
		op->update(subject.c_str(), false);
	};
	op->setRetryFunction(retryableAction);
	retryableAction();

	shared_ptr<Call> currentCall = q->getCore()->getCurrentCall();
	// Reset current session if we are pausing the current call
	if (!currentCall || (currentCall->getActiveSession() == q->getSharedFromThis())) {
		q->notifyResetCurrentSession();
	}

	return 0;
}

int MediaSessionPrivate::restartInvite() {
	L_Q();
	stopStreams();
	getStreamsGroup().clearStreams();
	// Clear streams resets the ICE service therefore the pointer to its listener is now NULL and it must be set asap in
	// order to receive events.
	getStreamsGroup().getIceService().setListener(this);
	makeLocalMediaDescription(true, q->isCapabilityNegotiationEnabled(), false);
	const auto defer = CallSessionPrivate::restartInvite();
	if (!defer) {
		q->startInvite(nullptr, op->getSubject(), nullptr);
	}
	return defer;
}

void MediaSessionPrivate::setTerminated() {
	freeResources();
	CallSessionPrivate::setTerminated();
}

LinphoneStatus MediaSessionPrivate::startAcceptUpdate(CallSession::State nextState, const string &stateInfo) {
	op->accept();
	std::shared_ptr<SalMediaDescription> &md = op->getFinalMediaDescription();
	updateStreams(md, nextState);
	setState(nextState, stateInfo);
	getCurrentParams()->getPrivate()->setInConference(getParams()->getPrivate()->getInConference());

	return 0;
}

LinphoneStatus MediaSessionPrivate::startUpdate(const CallSession::UpdateMethod method, const string &subject) {
	L_Q();

	const bool doNotAddSdpToInvite =
	    q->getCore()->getCCore()->sip_conf.sdp_200_ack && !getParams()->getPrivate()->getInternalCallUpdate();
	if (doNotAddSdpToInvite) {
		op->setLocalMediaDescription(nullptr);
	}
	LinphoneStatus result = CallSessionPrivate::startUpdate(method, subject);
	op->setRetryFunction([this, subject, method]() { this->startUpdate(method, subject); });

	if (doNotAddSdpToInvite) {
		// We are NOT offering, set local media description after sending the call so that we are ready to
		// process the remote offer when it will arrive.
		op->setLocalMediaDescription(localDesc);
	}
	return result;
}

void MediaSessionPrivate::terminate() {
	L_Q();
	if (q->isRecording()) {
		lInfo() << "Media session is being terminated, stop recording";
		q->stopRecording();
	}
	stopStreams();
	q->getCore()->getPrivate()->getToneManager().stopSecurityAlert();
	localIsTerminator = true;
	CallSessionPrivate::terminate();
}

LinphoneMediaDirection MediaSessionPrivate::getDirFromMd(const std::shared_ptr<SalMediaDescription> &md,
                                                         const SalStreamType type) const {
	L_Q();
	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	if (conference) {
		const auto hasVideoSendRecvStream = (md->containsStreamWithDir(SalStreamSendRecv, type));
		const auto hasVideoSendOnlyStream = (md->containsStreamWithDir(SalStreamSendOnly, type));
		const auto hasVideoRecvOnlyStream = (md->containsStreamWithDir(SalStreamRecvOnly, type));
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
	const auto &videoStream = md->findBestStream(type);
	return MediaSessionParamsPrivate::salStreamDirToMediaDirection(videoStream.getDirection());
}

void MediaSessionPrivate::updateCurrentParams() const {
	L_Q();
	CallSessionPrivate::updateCurrentParams();

	VideoControlInterface *i = getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) {
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

	} else {
		LinphoneVideoDefinition *vdef =
		    linphone_video_definition_new(MS_VIDEO_SIZE_UNKNOWN_W, MS_VIDEO_SIZE_UNKNOWN_H, nullptr);
		getCurrentParams()->getPrivate()->setSentVideoDefinition(vdef);
		getCurrentParams()->getPrivate()->setReceivedVideoDefinition(vdef);
		linphone_video_definition_unref(vdef);
	}

	/* REVISITED
	 * Previous code was buggy.
	 * Relying on the mediastream's state (added by jehan: only) to know the current encryption is unreliable.
	 * For both DTLS and ZRTP it is though necessary.
	 * But for all others the current_params->media_encryption state should reflect both what is agreed by the
	 * offer/answer mechanism and encryption status from media which is much stronger than only result of offer/answer.
	 * Typically there can be inactive streams for which the media layer has no idea of whether they are encrypted or
	 * not.
	 */

	string authToken = getStreamsGroup().getAuthenticationToken();

	const std::shared_ptr<SalMediaDescription> &md = resultDesc;

	// In case capability negotiation is enabled, the actual encryption is the negotiated one
	const LinphoneMediaEncryption enc = getNegotiatedMediaEncryption();
	bool srtpEncryptionMatch = false;
	auto srtpSuite = LinphoneSrtpSuiteInvalid;
	const auto allStreamsAreEncrypted = allStreamsEncrypted();
	if (md) {
		srtpEncryptionMatch = true;
		bool srtpSuiteSet = false;
		for (size_t idx = 0; idx < md->getNbStreams(); idx++) {
			const auto &salStream = md->getStreamAtIdx(static_cast<unsigned int>(idx));
			if (salStream.enabled() && (salStream.getDirection() != SalStreamInactive) && salStream.hasSrtp()) {
				const auto &streamCryptos = salStream.getCryptos();
				const auto &stream = getStreamsGroup().getStream(idx);
				if (stream) {
					for (const auto &crypto : streamCryptos) {
						const auto &algo = crypto.algo;
						if (isEncryptionMandatory()) {
							srtpEncryptionMatch &= !ms_crypto_suite_is_unencrypted(algo) && stream->isEncrypted();
						} else {
							srtpEncryptionMatch &= ((ms_crypto_suite_is_unencrypted(algo)) ? !stream->isEncrypted()
							                                                               : stream->isEncrypted());
						}

						// To have a valid SRTP suite in the current call params, all streams must be encrypted and use
						// the same suite
						// TODO: get the stream status and SRTP encryption from mediastreamer so we can get the suite
						// even when using ZRTP or DTLS as key exchange
						if (srtpSuiteSet && (srtpSuite != LinphoneSrtpSuiteInvalid)) {
							if (srtpSuite != MSCryptoSuite2LinphoneSrtpSuite(algo)) {
								srtpSuite = LinphoneSrtpSuiteInvalid;
							}
						} else {
							srtpSuiteSet = true;
							srtpSuite = MSCryptoSuite2LinphoneSrtpSuite(algo);
						}
					}
				}
			} else { // No Srtp on this stream -> srtpSuite is set to invalid
				srtpSuite = LinphoneSrtpSuiteInvalid;
				srtpSuiteSet = true;
			}
		}
		getCurrentParams()->enableRtpBundle(!md->bundles.empty());
	} else {
		srtpEncryptionMatch = allStreamsAreEncrypted;
	}

	getCurrentParams()->setSrtpSuites(std::list<LinphoneSrtpSuite>{srtpSuite});

	bool updateEncryption = false;
	bool validNegotiatedEncryption = false;
	const auto activeStreams = getNbActiveStreams();

	switch (enc) {
		case LinphoneMediaEncryptionZRTP:
			updateEncryption = atLeastOneStreamStarted();
			validNegotiatedEncryption = (allStreamsAreEncrypted && !authToken.empty());
			break;
		case LinphoneMediaEncryptionSRTP:
			updateEncryption = atLeastOneStreamStarted();
			validNegotiatedEncryption = ((activeStreams == 0) || srtpEncryptionMatch);
			break;
		case LinphoneMediaEncryptionDTLS:
			updateEncryption = atLeastOneStreamStarted();
			validNegotiatedEncryption = ((activeStreams == 0) || allStreamsAreEncrypted);
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
			lDebug() << "Encryption was requested to be " << std::string(linphone_media_encryption_to_string(enc))
			         << ", but isn't effective (allStreamsEncrypted=" << allStreamsAreEncrypted
			         << ", auth_token=" << authToken << ")";
			getCurrentParams()->setMediaEncryption(LinphoneMediaEncryptionNone);
		}
	} /* else don't update the state if all streams are shutdown */

	const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
	if (md) {
		getCurrentParams()->enableAvpf(hasAvpf(md));
		if (getCurrentParams()->avpfEnabled()) {
			getCurrentParams()->setAvpfRrInterval(getAvpfRrInterval());
		} else {
			getCurrentParams()->setAvpfRrInterval(0);
		}
		const SalStreamDescription &audioStream = md->findBestStream(SalAudio);
		if (audioStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
			const auto audioDir = getDirFromMd(md, SalAudio);
			getCurrentParams()->setAudioDirection(audioDir);
			if (getCurrentParams()->getAudioDirection() != LinphoneMediaDirectionInactive) {
				const std::string &rtpAddr =
				    (audioStream.getRtpAddress().empty() == false) ? audioStream.getRtpAddress() : md->addr;
				getCurrentParams()->enableAudioMulticast(!!ms_is_multicast(rtpAddr.c_str()));
			} else getCurrentParams()->enableAudioMulticast(false);
			getCurrentParams()->enableAudio(audioStream.enabled());
		} else {
			getCurrentParams()->setAudioDirection(LinphoneMediaDirectionInactive);
			getCurrentParams()->enableAudioMulticast(false);
			getCurrentParams()->enableAudio(false);
		}

		const auto streamIdx = q->getThumbnailStreamIdx(md);
		const auto &videoStream =
		    (streamIdx == -1) ? md->findBestStream(SalVideo) : md->getStreamAtIdx(static_cast<unsigned int>(streamIdx));
		if (videoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
			getCurrentParams()->getPrivate()->enableImplicitRtcpFb(videoStream.hasImplicitAvpf());

			const auto videoDirection = getDirFromMd(md, SalVideo);
			getCurrentParams()->setVideoDirection(videoDirection);

			if (getCurrentParams()->getVideoDirection() != LinphoneMediaDirectionInactive) {
				const std::string &rtpAddr =
				    (videoStream.getRtpAddress().empty() == false) ? videoStream.getRtpAddress() : md->addr;
				getCurrentParams()->enableVideoMulticast(!!ms_is_multicast(rtpAddr.c_str()));
			} else {
				getCurrentParams()->enableVideoMulticast(false);
			}
			const auto enable =
			    (conference) ? (videoDirection != LinphoneMediaDirectionInactive) : videoStream.enabled();
			getCurrentParams()->enableVideo(enable);
		} else {
			getCurrentParams()->getPrivate()->enableImplicitRtcpFb(false);
			getCurrentParams()->setVideoDirection(LinphoneMediaDirectionInactive);
			getCurrentParams()->enableVideoMulticast(false);
			getCurrentParams()->enableVideo(false);
		}

		const SalStreamDescription &textStream = md->findBestStream(SalText);
		if (textStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
			// Direction and multicast are not supported for real-time text.
			getCurrentParams()->enableRealtimeText(textStream.enabled());
		} else {
			getCurrentParams()->enableRealtimeText(false);
		}
	}
	getCurrentParams()->getPrivate()->setUpdateCallWhenIceCompleted(isUpdateSentWhenIceCompleted());
	bool deviceIsScreenSharing = false;
	if (conference) {
		const auto meDevices = conference->getMe()->getDevices();
		const auto &participantDevice = ((meDevices.size() == 0) || isInConference())
		                                    ? conference->findParticipantDevice(q->getSharedFromThis())
		                                    : meDevices.front();
		if (participantDevice) {
			deviceIsScreenSharing = participantDevice->screenSharingEnabled();
		}
	}
	getCurrentParams()->enableScreenSharing(q->isScreenSharingNegotiated() && (!conference || deviceIsScreenSharing));
	getCurrentParams()->enableFec(getStreamsGroup().isFecEnabled());
}

// -----------------------------------------------------------------------------

LinphoneStatus MediaSessionPrivate::startAccept() {
	L_Q();

	shared_ptr<Call> currentCall = q->getCore()->getCurrentCall();
	// If the core in a call, request to empty sound resources only if this call is not the call the core is currently
	// in
	bool isThisNotCurrentMediaSession = currentCall && (currentCall->getActiveSession() != q->getSharedFromThis());

	bool isCoreInLocalConference = linphone_core_is_in_conference(q->getCore()->getCCore());
	const auto callConference = q->getCore()->findConference(q->getSharedFromThis(), false);
	auto coreConference = linphone_core_get_conference(q->getCore()->getCCore());
	// If the core in a conference, request to empty sound resources only if the call is in a different conference or
	// the call is not part of a conference
	bool isThisNotCurrentConference =
	    isCoreInLocalConference && (!callConference || (callConference->toC() != coreConference));

	// Try to preempt sound resources if the core is in a call or conference that are not the current ones
	if (isThisNotCurrentConference || isThisNotCurrentMediaSession) {
		if ((linphone_core_get_media_resource_mode(q->getCore()->getCCore()) == LinphoneExclusiveMediaResources) &&
		    linphone_core_preempt_sound_resources(q->getCore()->getCCore()) != 0) {
			lInfo() << "Delaying call to " << __func__ << " for media session (local addres " << *q->getLocalAddress()
			        << " remote address " << *q->getRemoteAddress() << ") in state " << Utils::toString(state)
			        << " because sound resources cannot be preempted";
			q->addPendingAction([this] {
				this->startAccept();
				return 0;
			});
			return -1;
		}
	}

	// It occurs if the remote participant calls the core hosting the conference and the call is added to the conference
	// when it is in state IncomingReceived
	// Do not do anything if the contact address is not yet known
	const auto &confId = getConferenceId();
	if (getOp() && getOp()->getContactAddress() && !confId.empty() && isInConference()) {
		q->updateContactAddressInOp();
	}

	/* Give a chance a set card prefered sampling frequency */
	if (localDesc->streams[0].getMaxRate() > 0) {
		lInfo() << "Configuring prefered card sampling rate to [" << localDesc->streams[0].getMaxRate() << "]";
		if (q->getCore()->getCCore()->sound_conf.play_sndcard)
			ms_snd_card_set_preferred_sample_rate(q->getCore()->getCCore()->sound_conf.play_sndcard,
			                                      localDesc->streams[0].getMaxRate());
		if (q->getCore()->getCCore()->sound_conf.capt_sndcard)
			ms_snd_card_set_preferred_sample_rate(q->getCore()->getCCore()->sound_conf.capt_sndcard,
			                                      localDesc->streams[0].getMaxRate());
	}

	/* We shall already have all the information to prepare the zrtp/lime mutual authentication */
	performMutualAuthentication();

	CallSessionPrivate::accept(nullptr);
	if (!getParams()->getPrivate()->getInConference()) {
		q->notifySetCurrentSession();
	}

	std::shared_ptr<SalMediaDescription> &newMd = op->getFinalMediaDescription();
	if (newMd) {
		// If negotiated media description doesn't contain a video stream after the first INVITE message sequence, then
		// disable video in the local call parameters
		if (getParams()->videoEnabled() &&
		    (newMd->findBestStream(SalVideo) == Utils::getEmptyConstRefObject<SalStreamDescription>())) {
			getParams()->enableVideo(false);
		}
		updateStreams(newMd, CallSession::State::StreamsRunning);
		setState(CallSession::State::StreamsRunning, "Connected (streams running)");
	} else expectMediaInAck = true;

	return 0;
}

LinphoneMediaDirection MediaSessionPrivate::computeNewVideoDirection(LinphoneMediaDirection acceptVideoDirection) {
	L_Q();
	auto paramsVideoDirection = getParams()->getVideoDirection();
	auto state = q->getState();
	if (op &&
	    ((op->getRemoteMediaDescription() &&
	      ((state == CallSession::State::IncomingReceived) || (state == CallSession::State::UpdatedByRemote) ||
	       (state == CallSession::State::EarlyUpdatedByRemote) || (state == CallSession::State::IncomingEarlyMedia))) ||
	     (state == CallSession::State::StreamsRunning && q->getCore()->getCCore()->sip_conf.sdp_200_ack))) {
		// We are accepting an offer
		auto videoEnabled = getParams()->videoEnabled();
		if (!videoEnabled) {
			return acceptVideoDirection;
		} else if (paramsVideoDirection == acceptVideoDirection) {
			return paramsVideoDirection;
		} else if (paramsVideoDirection == LinphoneMediaDirectionSendRecv ||
		           acceptVideoDirection == LinphoneMediaDirectionSendRecv) {
			return LinphoneMediaDirectionSendRecv;
		} else if ((paramsVideoDirection == LinphoneMediaDirectionSendOnly &&
		            acceptVideoDirection == LinphoneMediaDirectionRecvOnly) ||
		           (paramsVideoDirection == LinphoneMediaDirectionRecvOnly &&
		            acceptVideoDirection == LinphoneMediaDirectionSendOnly)) {
			return LinphoneMediaDirectionSendRecv;
		} else if (paramsVideoDirection == LinphoneMediaDirectionInactive) {
			return acceptVideoDirection;
		} else if (acceptVideoDirection == LinphoneMediaDirectionInactive) {
			return paramsVideoDirection;
		}
	} else {
		return paramsVideoDirection;
	}
	return LinphoneMediaDirectionInactive;
}

LinphoneStatus MediaSessionPrivate::accept(const MediaSessionParams *msp, BCTBX_UNUSED(bool wasRinging)) {
	L_Q();
	if (msp) {
		setParams(new MediaSessionParams(*msp));
	}

	const bool isOfferer = (op->getRemoteMediaDescription() ? false : true);

	if (msp || (localDesc == nullptr)) {
		makeLocalMediaDescription(isOfferer, q->isCapabilityNegotiationEnabled(), false);
	}

	// If call is going to be accepted, then recreate the local media description if there is no local description or
	// encryption is mandatory. The initial INVITE sequence goes through the offer answer negotiation process twice. The
	// first one generates the 180 Ringing and it ensures that the offer can be potentially accepted upon setting of a
	// compatible set of parameters. The second offer answer negotiation is more thorough as the set of parameters to
	// accept the call is known. In this case, if the encryption is mandatory a new local media description must be
	// generated in order to populate the crypto keys with the set actually ued in the call
	if ((state == CallSession::State::IncomingReceived) && params) {
		makeLocalMediaDescription(isOfferer, q->isCapabilityNegotiationEnabled(), false, false);
	}

	updateRemoteSessionIdAndVer();

	auto acceptCompletionTask = [this]() {
		updateLocalMediaDescriptionFromIce(op->getRemoteMediaDescription() == nullptr);
		return startAccept();
	};
	if (natPolicy && natPolicy->iceEnabled() && getStreamsGroup().prepare()) {
		queueIceGatheringTask(acceptCompletionTask);
		return 0; /* Deferred until completion of ICE gathering */
	}
	return acceptCompletionTask();
}

LinphoneStatus
MediaSessionPrivate::acceptUpdate(const CallSessionParams *csp, CallSession::State nextState, const string &stateInfo) {
	L_Q();
	const std::shared_ptr<SalMediaDescription> &desc = op->getRemoteMediaDescription();
	const bool isRemoteDescNull = (desc == nullptr);

	bool keepSdpVersion = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
	                                                "keep_sdp_version", (op->getSal()->getSessionTimersExpire() > 0));

	if (keepSdpVersion && desc && (desc->session_id == remoteSessionId) && (desc->session_ver == remoteSessionVer)) {
		/* Remote has sent an INVITE with the same SDP as before, so send a 200 OK with the same SDP as before. */
		lInfo() << "SDP version has not changed, send same SDP as before or sessionTimersExpire="
		        << op->getSal()->getSessionTimersExpire();
		op->accept();
		setState(nextState, stateInfo);
		return 0;
	}

	if (csp) {
		setParams(new MediaSessionParams(*static_cast<const MediaSessionParams *>(csp)));
	} else {
		auto msp = createMediaSessionParams();
		if (!op->isOfferer()) {
			/* Reset call params for multicast because this param is only relevant when offering */
			msp->enableAudioMulticast(false);
			msp->enableVideoMulticast(false);
		}
		setParams(msp);
	}
	if (getParams()->videoEnabled() && !linphone_core_video_enabled(q->getCore()->getCCore())) {
		lWarning() << "Requested video but video support is globally disabled. Refusing video";
		getParams()->enableVideo(false);
	}
	updateRemoteSessionIdAndVer();
	makeLocalMediaDescription(isRemoteDescNull, q->isCapabilityNegotiationEnabled(), false);

	auto acceptCompletionTask = [this, nextState, stateInfo, isRemoteDescNull]() {
		updateLocalMediaDescriptionFromIce(isRemoteDescNull);
		startAcceptUpdate(nextState, stateInfo);
		return 0;
	};

	if (natPolicy && natPolicy->iceEnabled() && getStreamsGroup().prepare()) {
		lInfo() << "Acceptance of incoming reINVITE is deferred to ICE gathering completion.";
		queueIceGatheringTask(acceptCompletionTask);
		return 0; /* Deferred until completion of ICE gathering */
	} else if (getStreamsGroup().getIceService().isRunning() && !isUpdateSentWhenIceCompleted()) {
		// ICE negotiations are ongoing hence the update cannot be accepted immediately - need to wait for the
		// completition of ICE negotiations
		lInfo() << "acceptance of incoming reINVITE is deferred to ICE completion completion.";
		queueIceCompletionTask(acceptCompletionTask);
		return 0;
	}
	acceptCompletionTask();
	return 0;
}

// -----------------------------------------------------------------------------

void MediaSessionPrivate::refreshSockets() {
	getStreamsGroup().refreshSockets();
}

void MediaSessionPrivate::reinviteToRecoverFromConnectionLoss() {
	L_Q();
	lInfo() << "MediaSession [" << q
	        << "] is going to be updated (reINVITE) in order to recover from lost connectivity";
	getStreamsGroup().getIceService().resetSession();
	MediaSessionParams newParams(*getParams());
	q->update(&newParams, CallSession::UpdateMethod::Invite, q->isCapabilityNegotiationEnabled());
}

void MediaSessionPrivate::repairByNewInvite(bool withReplaces) {
	if ((state == CallSession::State::IncomingEarlyMedia) || (state == CallSession::State::OutgoingEarlyMedia)) {
		stopStreams();
	}
	CallSessionPrivate::repairByNewInvite(withReplaces);
}

int MediaSessionPrivate::sendDtmf() {
	// There are two relevant Core settings here: "use_rfc2833" and "use_info"; Resulting in 4 cases.
	// (0)   If neither are enabled, don't send anything.
	// (1|2) If one is enabled but not the other, then send the DTMF using the one the that is enabled.
	// (3)   If both are enabled, use RFC 2833, then SIP INFO as fallback only if the media does not support telephone
	// events. (In that last sub-case, note that the DTMF will also be sent in-band through audio encoding)

	L_Q();
	LinphoneCore *lc = q->getCore()->getCCore();
	AudioControlInterface *audioInterface = nullptr;
	if (linphone_core_get_use_rfc2833_for_dtmf(lc)) { // (1|3)
		audioInterface = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
		if (audioInterface) audioInterface->sendDtmf(dtmfSequence.front());
		else {
			lError() << "Cannot send RFC2833 DTMF when we are not in communication";
			return FALSE;
		}
	}

	bool useSipInfo = linphone_core_get_use_info_for_dtmf(lc);
	if (audioInterface && useSipInfo) { // (3)
		useSipInfo = !audioInterface->supportsTelephoneEvents();
	}

	if (useSipInfo) { // (2|3)
		op->sendDtmf(dtmfSequence.front());
	}

	dtmfSequence.erase(0, 1);
	// Continue only if the dtmf sequence is not empty
	if (!dtmfSequence.empty()) return TRUE;
	else {
		q->cancelDtmfs();
		return FALSE;
	}
}

// -----------------------------------------------------------------------------

int MediaSessionPrivate::resumeAfterFailedTransfer(void *userData, unsigned int) {
	MediaSession *session = static_cast<MediaSession *>(userData);
	if (!session) {
		return -1;
	}
	return session->getPrivate()->resumeAfterFailedTransfer();
}

bool_t MediaSessionPrivate::startPendingRefer(void *userData) {
	MediaSession *session = static_cast<MediaSession *>(userData);
	if (!session) {
		return FALSE;
	}
	session->getPrivate()->startPendingRefer();
	return TRUE;
}

void MediaSessionPrivate::stunAuthRequestedCb(const char *realm,
                                              BCTBX_UNUSED(const char *nonce),
                                              const char **username,
                                              const char **password,
                                              const char **ha1) {
	L_Q();
	/* Get the username from the nat policy or the proxy config */
	std::shared_ptr<Account> stunAccount = nullptr;
	const auto &account = getDestAccount();
	if (account) stunAccount = account;
	else {
		stunAccount = q->getCore()->getDefaultAccount();
	}
	if (!stunAccount) return;
	const char *user = NULL;
	const auto &accountParams = stunAccount->getAccountParams();
	const auto &proxyNatPolicy = accountParams->getNatPolicy();
	if (proxyNatPolicy) user = L_STRING_TO_C(proxyNatPolicy->getStunServerUsername());
	else if (natPolicy) user = L_STRING_TO_C(natPolicy->getStunServerUsername());
	if (!user) {
		/* If the username has not been found in the nat_policy, take the username from the currently used proxy config
		 */
		const auto identityAddress = accountParams->getIdentityAddress();
		if (!identityAddress) return;
		user = L_STRING_TO_C(identityAddress->getUsername());
	}
	if (!user) return;

	const LinphoneAuthInfo *authInfo = linphone_core_find_auth_info(q->getCore()->getCCore(), realm, user, nullptr);
	if (!authInfo) {
		lWarning() << "No auth info found for STUN auth request";
		return;
	}
	const char *hash = linphone_auth_info_get_ha1(authInfo);
	if (hash) *ha1 = hash;
	else *password = linphone_auth_info_get_password(authInfo);
	*username = user;
}

IceSession *MediaSessionPrivate::getIceSession() const {
	return getIceService().getSession();
}

// =============================================================================
MediaSession::MediaSession(const shared_ptr<Core> &core,
                           std::shared_ptr<Participant> me,
                           const CallSessionParams *params,
                           CallSessionListener *listener)
    : CallSession(*new MediaSessionPrivate, core) {
	L_D();
	d->me = me;
	addListener(listener);

	if (params) {
		d->setParams(new MediaSessionParams(*(static_cast<const MediaSessionParams *>(params))));
	} else {
		d->setParams(new MediaSessionParams());
	}
	d->setCurrentParams(new MediaSessionParams());
	d->streamsGroup = makeUnique<StreamsGroup>(*this);
	d->streamsGroup->getIceService().setListener(d);

	lInfo() << "New MediaSession [" << this << "] initialized (liblinphone version: " << linphone_core_get_version()
	        << ")";
}

MediaSession::~MediaSession() {
	L_D();
	cancelDtmfs();
	d->freeResources();
}

// -----------------------------------------------------------------------------

ConferenceLayout MediaSession::computeConferenceLayout(const std::shared_ptr<SalMediaDescription> &md) const {
	L_D();
	ConferenceLayout layout = ConferenceLayout::ActiveSpeaker;
	if (md) {
		const bool isInLocalConference = d->getParams()->getPrivate()->getInConference();
		if (md->findIdxStreamWithContent(MediaSessionPrivate::GridVideoContentAttribute) != -1) {
			layout = ConferenceLayout::Grid;
		} else if (md->findIdxStreamWithContent(MediaSessionPrivate::ActiveSpeakerVideoContentAttribute) != -1) {
			layout = ConferenceLayout::ActiveSpeaker;
		} else if (md->findIdxStreamWithContent(MediaSessionPrivate::ScreenSharingContentAttribute) != -1) {
			const auto &params = (isInLocalConference) ? d->getRemoteParams() : d->getParams();
			layout = params->getConferenceVideoLayout();
		} else {
			layout = ConferenceLayout::ActiveSpeaker;
			lDebug() << "Unable to deduce layout from media description " << md
			         << " - Default it to: " << Utils::toString(layout);
		}
	}
	return layout;
}

void MediaSession::acceptDefault() {
	accept();
}

LinphoneStatus MediaSession::accept(const MediaSessionParams *msp) {
	L_D();
	if (!isOpConfigured()) {
		lInfo() << "CallSession accepting";
		if (msp) d->setParams(new MediaSessionParams(*msp));
		CallSession::accepting();
		return 0;
	}

	LinphoneStatus result = d->checkForAcceptation();
	if (result < 0) return result;

	bool wasRinging = false;
	notifyCallSessionAccepted();

	auto ret = d->accept(msp, wasRinging);
	if (ret == 0) {
		lInfo() << "MediaSession " << this << " (local address " << *getLocalAddress() << " remote address "
		        << *getRemoteAddress() << ") has been accepted";
	} else {
		lInfo() << "Unable to immediately accept session " << this << " (local address " << *getLocalAddress()
		        << " remote address " << *getRemoteAddress() << ")";
	}
	return ret;
}

LinphoneStatus MediaSession::acceptEarlyMedia(const MediaSessionParams *msp) {
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
	std::shared_ptr<SalMediaDescription> &md = d->op->getFinalMediaDescription();
	if (md) d->updateStreams(md, d->state);
	return 0;
}

LinphoneStatus MediaSession::acceptUpdate(const MediaSessionParams *msp) {
	L_D();
	if (d->expectMediaInAck) {
		lError() << "MediaSession::acceptUpdate() is not possible during a late offer incoming reINVITE (INVITE "
		            "without SDP)";
		return -1;
	}
	return CallSession::acceptUpdate(msp);
}

void MediaSession::cancelDtmfs() {
	L_D();
	if (!d->dtmfTimer) return;

	getCore()->getCCore()->sal->cancelTimer(d->dtmfTimer);
	belle_sip_object_unref(d->dtmfTimer);
	d->dtmfTimer = nullptr;
	d->dtmfSequence.clear();
}

void MediaSession::setNatPolicy(const std::shared_ptr<NatPolicy> &pol) {
	L_D();
	d->natPolicy = pol;
}

bool MediaSession::toneIndicationsEnabled() const {
	return getMediaParams()->getPrivate()->toneIndicationsEnabled();
}

void MediaSession::configure(LinphoneCallDir direction,
                             const std::shared_ptr<Account> &account,
                             SalCallOp *op,
                             const std::shared_ptr<const Address> &from,
                             const std::shared_ptr<const Address> &to) {
	L_D();
	std::shared_ptr<Address> remote;

	CallSession::configure(direction, account, op, from, to);

	if (direction == LinphoneCallOutgoing) {
		remote = to->clone()->toSharedPtr();
	} else if (direction == LinphoneCallIncoming) {
		/* Note that the choice of IP version for streams is later refined by setCompatibleIncomingCallParams() when
		 * examining the remote offer, if any. If the remote offer contains IPv4 addresses, we should propose IPv4 as
		 * well. */
		remote = from->clone()->toSharedPtr();
		remote->clean();
		d->setParams(new MediaSessionParams());
		d->params->initDefault(getCore(), LinphoneCallIncoming);
		d->initializeParamsAccordingToIncomingCallParams();
		/* For incoming calls, the bundle enablement is set according to remote call params and core's policy,
		 * in fixCallParams() */
	}

	assignAccount(account);
	// At this point, the account is set if found
	const auto &selectedAccount = d->params->getAccount();
	const auto &accountParams = selectedAccount ? selectedAccount->getAccountParams() : nullptr;

	if (direction == LinphoneCallOutgoing) {
		/* The enablement of rtp bundle is controlled at first by the Account, then the Core.
		 * Then the value is stored and later updated into MediaSessionParams. */
		bool rtpBundleEnabled = false;
		if (accountParams) {
			rtpBundleEnabled = accountParams->rtpBundleEnabled();
		} else {
			lInfo() << "No account set for this call, using rtp bundle enablement from LinphoneCore.";
			rtpBundleEnabled = linphone_core_rtp_bundle_enabled(getCore()->getCCore());
		}
		d->getParams()->enableRtpBundle(rtpBundleEnabled);
	}

	lInfo() << "Rtp bundle is " << (d->getParams()->rtpBundleEnabled() ? "enabled." : "disabled.");

	if (!d->natPolicy) {
		if (accountParams) {
			const auto accountNatPolicy = accountParams->getNatPolicy();
			if (accountNatPolicy) {
				d->natPolicy = accountNatPolicy;
			}
		}
		if (!d->natPolicy) {
			d->natPolicy = NatPolicy::toCpp(linphone_core_get_nat_policy(getCore()->getCCore()))->getSharedFromThis();
		}
	}

	if (d->natPolicy) d->runStunTestsIfNeeded();
	d->discoverMtu(remote);
}

LinphoneStatus MediaSession::deferUpdate() {
	L_D();
	if (d->state != CallSession::State::UpdatedByRemote) {
		lError() << "MediaSession::deferUpdate() not done in state CallSession::State::UpdatedByRemote";
		return -1;
	}
	if (d->expectMediaInAck) {
		lError()
		    << "MediaSession::deferUpdate() is not possible during a late offer incoming reINVITE (INVITE without SDP)";
		return -1;
	}
	d->deferUpdate = true;
	return 0;
}

void MediaSession::initiateIncoming() {
	L_D();
	CallSession::initiateIncoming();

	bool isOfferer = d->op->getRemoteMediaDescription() ? false : true;
	d->makeLocalMediaDescription(isOfferer, isCapabilityNegotiationEnabled(), false);

	if (d->natPolicy && d->natPolicy->iceEnabled()) {
		d->deferIncomingNotification = d->getStreamsGroup().prepare();
		/*
		 * If ICE gathering is done, we can update the local media description immediately.
		 * Otherwise, we'll get the ORTP_EVENT_ICE_GATHERING_FINISHED event later.
		 */
		if (d->deferIncomingNotification) {
			auto incomingNotificationTask = [d]() {
				/* There is risk that the call can be terminated before this task is executed, for example if
				 * offer/answer fails.*/
				if (d->state != State::Idle && d->state != State::PushIncomingReceived) return 0;
				d->deferIncomingNotification = false;
				d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
				d->startIncomingNotification();
				return 0;
			};
			d->queueIceGatheringTask(incomingNotificationTask);
		} else {
			d->updateLocalMediaDescriptionFromIce(d->localIsOfferer);
		}
	}
}

bool MediaSession::initiateOutgoing(const string &subject, const std::shared_ptr<const Content> content) {
	L_D();
	bool defer = CallSession::initiateOutgoing(subject, content);

	if (!d->op) d->createOp();
	if (!getCore()->getCCore()->sip_conf.sdp_200_ack) {
		d->makeLocalMediaDescription(true, isCapabilityNegotiationEnabled(), false);
		lInfo() << "Created local media description.";
	}

	if (d->natPolicy && d->natPolicy->iceEnabled()) {
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
			} else {
				auto toAddr = d->log->getToAddress();
				lInfo() << "Unable to initiate call to " << toAddr->toString()
				        << " because ICE candidates must be gathered first";
				d->queueIceGatheringTask([this, subject, content]() {
					L_D();
					if (d->state != CallSession::State::End) // Call has been terminated while gathering: avoid to
					                                         // update descriptions.
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

void MediaSession::iterate(time_t currentRealTime, bool oneSecondElapsed) {
	CallSession::iterate(currentRealTime, oneSecondElapsed);
}

LinphoneStatus MediaSession::pauseFromConference() {
	L_D();
	updateContactAddressInOp();

	int ret = 0;

	// Do not pause call if it is already in paused by remote state
	if (d->state != CallSession::State::PausedByRemote) {
		ret = pause();
	}

	return ret;
}

LinphoneStatus MediaSession::pause() {
	L_D();
	d->pausedByApp = true;
	LinphoneStatus result = d->pause();
	if (result != 0) d->pausedByApp = false;
	return result;
}

LinphoneStatus MediaSession::delayResume() {
	lInfo() << "Delaying call resume";
	addPendingAction([this] { return this->resume(); });
	return -1;
}

LinphoneStatus MediaSession::resume() {
	L_D();
	if (d->state == CallSession::State::Pausing) {
		lInfo() << "Call is currently in state " << Utils::toString(d->state)
		        << " and cannot be immediately resumed therefore this task will be scheduled";
		addPendingAction([this] { return this->resume(); });
		return -1;
	} else if (d->state != CallSession::State::Paused) {
		lWarning() << "we cannot resume a call that has not been established and paused before. Current state: "
		           << Utils::toString(d->state);
		return -1;
	}
	if (!d->getParams()->getPrivate()->getInConference()) {
		if (linphone_core_sound_resources_locked(getCore()->getCCore())) {
			lWarning() << "Cannot resume MediaSession " << this
			           << " because another call is locking the sound resources";
			return -1;
		}
		if (linphone_core_preempt_sound_resources(getCore()->getCCore()) != 0) {
			lInfo() << "Delaying call to " << __func__ << " because sound resources cannot be preempted";
			addPendingAction([this] { return this->resume(); });
			return -1;
		}
	}

	lInfo() << "Resuming MediaSession " << this;
	d->pausedByApp = false;
	d->automaticallyPaused = false;
	d->broken = false; // Set broken state to false in case the request is not being triggered by repairIfBroken(), so
	                   // that we don't the job twice.
	/* Stop playing music immediately. If remote side is a conference it
	 * prevents the participants to hear it while the 200OK comes back. */
	Stream *as = d->getStreamsGroup().lookupMainStream(SalAudio);
	if (as) as->stop();

	string subject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::Resuming);
	if (d->getParams()->getPrivate()->getInConference() && !getCurrentParams()->getPrivate()->getInConference()) {
		subject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::Conference);
	}

	updateContactAddressInOp();

	const auto isIceRunning = getStreamsGroup().getIceService().isRunning();

	auto retryableAction = [this, isIceRunning, subject]() -> int {
		L_D();
		auto updateCompletionTask = [this, subject]() -> int {
			L_D();

			CallSession::State previousState = d->state;
			// The state must be set before recreating the media description in order for the method
			// forceStreamsDirAccordingToState (called by makeLocalMediaDescription) to set the stream directions
			// accordingly
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
		if (d->natPolicy && d->natPolicy->iceEnabled() && preparingStreams) {
			lInfo() << "Defer CallSession " << this << " (local address " << getLocalAddress()->toString()
			        << " remote address " << getRemoteAddress()->toString() << ") resume to gather ICE candidates";
			d->queueIceGatheringTask(updateCompletionTask);
			return 0;
		} else if (isIceRunning) {
			// ICE negotiations are ongoing hence the update cannot be send right now
			lInfo() << "Ice negotiations are ongoing and resume once they complete, therefore defer CallSession "
			        << this << " (local address " << getLocalAddress()->toString() << " remote address "
			        << getRemoteAddress()->toString() << ") resume until Ice negotiations are completed.";
			d->queueIceCompletionTask(updateCompletionTask);
			return 0;
		}
		return updateCompletionTask();
	};
	d->op->setRetryFunction(retryableAction);
	if (retryableAction() == -1) return -1;

	if (!d->getParams()->getPrivate()->getInConference()) {
		notifySetCurrentSession();
	}

	return 0;
}

bool MediaSession::dtmfSendingAllowed() const {
	L_D();
	switch (d->state) {
		case CallSession::State::End:
		case CallSession::State::Released:
		case CallSession::State::Error:
			lWarning() << "Sending DTMF is not possible in state" << Utils::toString(d->state);
			return false;
			break;
		default:
			break;
	}
	return true;
}

LinphoneStatus MediaSession::sendDtmf(char dtmf) {
	L_D();
	if (!dtmfSendingAllowed()) return -1;
	d->dtmfSequence = dtmf;
	d->sendDtmf();
	return 0;
}

LinphoneStatus MediaSession::sendDtmfs(const std::string &dtmfs) {
	L_D();
	if (!dtmfSendingAllowed()) return -1;
	if (d->dtmfTimer) {
		lWarning() << "MediaSession::sendDtmfs(): a DTMF sequence is already in place, canceling DTMF sequence";
		return -2;
	}
	if (!dtmfs.empty()) {
		int delayMs =
		    linphone_config_get_int(linphone_core_get_config(getCore()->getCCore()), "net", "dtmf_delay_ms", 200);
		if (delayMs < 0) delayMs = 0;
		d->dtmfSequence = dtmfs;
		d->dtmfTimer = getCore()->getCCore()->sal->createTimer(
		    MediaSessionPrivate::sendDtmf, this, static_cast<unsigned int>(delayMs), "DTMF sequence timer");
	}
	return 0;
}

void MediaSession::sendVfuRequest() {
	L_D();
	MediaSessionParams *curParams = getCurrentParams();

	if ((curParams->avpfEnabled() ||
	     curParams->getPrivate()->implicitRtcpFbEnabled())) { // || sal_media_description_has_implicit_avpf((const
		                                                      // std::shared_ptr<SalMediaDescription> )call->resultdesc)
		lInfo() << "Request Full Intra Request on CallSession [" << this << "]";
		d->getStreamsGroup().forEach<VideoControlInterface>([](VideoControlInterface *i) { i->sendVfuRequest(); });
	} else if (getCore()->getCCore()->sip_conf.vfu_with_info) {
		lInfo() << "Request SIP INFO FIR on CallSession [" << this << "]";
		if (d->state == CallSession::State::StreamsRunning) d->op->sendVfuRequest();
	} else lInfo() << "vfu request using sip disabled from config [sip,vfu_with_info]";
}

// Try to search the local conference by first looking at the contact address and if it is unsuccesfull to the to
// address as a client may try to be calling a conference URI directly Typically, the seach using the contact address
// will succeed when a client creates a conference.
const std::shared_ptr<Conference> MediaSession::getLocalConference() const {
	L_D();

	ConferenceId serverConferenceId;
	shared_ptr<Conference> conference = nullptr;

	auto log = getLog();
	const auto conferenceInfo = (log) ? log->getConferenceInfo() : nullptr;
	if (conferenceInfo) {
		auto conferenceAddress = conferenceInfo->getUri();
		serverConferenceId = ConferenceId(conferenceAddress, conferenceAddress);
		conference = getCore()->findConference(serverConferenceId, false);
	}
	if (!conference) {
		auto contactAddress = getContactAddress();
		if (contactAddress) {
			updateContactAddress(*contactAddress);
			serverConferenceId = ConferenceId(contactAddress, contactAddress);
			conference = getCore()->findConference(serverConferenceId, false);
		}
	}
	if (!conference) {
		const auto to = Address::create(d->op->getTo());
		// Local conference
		if (to->hasUriParam("conf-id")) {
			serverConferenceId = ConferenceId(to, to);
			conference = getCore()->findConference(serverConferenceId, false);
		}
	}

	return conference;
}

void MediaSession::startIncomingNotification(bool notifyRinging) {
	L_D();
	auto logContext = d->getLogContextualizer();
	std::shared_ptr<SalMediaDescription> &md = d->op->getFinalMediaDescription();

	auto conference = getLocalConference();
	bool isLocalDialOutConferenceCreationPending = false;

	if (conference) {
		// Get state here as it may be changed if the conference dials participants out
		const auto conferenceState = conference->getState();
		const auto dialout =
		    (conference->getCurrentParams()->getJoiningMode() == ConferenceParams::JoiningMode::DialOut);
		isLocalDialOutConferenceCreationPending =
		    dialout && ((conferenceState == ConferenceInterface::State::Instantiated) ||
		                (conferenceState == ConferenceInterface::State::CreationPending));
	}

	// Do not send a 488 Not Acceptable here if the call is part of a local conference where the server will call out
	// participant This scenario occurs when a client tries to create a dial out conference but there are not common
	// codecs between the client and the server. In such a case, the conference is not created at all since the
	// organizer will not be able to take part to it
	auto securityCheckFailure = d->incompatibleSecurity(md);
	if (md && (md->isEmpty() || securityCheckFailure) &&
	    ((conference && isLocalDialOutConferenceCreationPending) || !conference)) {
		lWarning() << "Session [" << this << "] will be declined: ";
		lWarning() << "- negotiated SDP is" << (md->isEmpty() ? std::string() : std::string(" not")) << " empty";
		lWarning() << "- negotiated security is" << (securityCheckFailure ? std::string(" not") : std::string())
		           << " compatible with core settings";
		if (d->state != CallSession::State::PushIncomingReceived) {
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, nullptr, LinphoneReasonNotAcceptable, 488, "Not acceptable here", nullptr);
			/* When call state is PushIncomingReceived, not notify early failed.
			   Because the call is already added in core, need to be released. */
			notifyCallSessionEarlyFailed(ei);
			linphone_error_info_unref(ei);
		}
		d->op->decline(SalReasonNotAcceptable);
		if (conference) {
			conference->setState(ConferenceInterface::State::CreationFailed);
		}
		return;
	}

	CallSession::startIncomingNotification(notifyRinging);
}

int MediaSession::getRandomRtpPort(const SalStreamDescription &stream) const {
	auto [minPort, maxPort] = Stream::getPortRange(getCore()->getCCore(), stream.type);
	if (minPort <= 0) {
		minPort = 1024;
		lInfo() << "Setting minimum value of port range to " << minPort;
	}
	if (maxPort <= 0) {
		// 2^16 - 1
		maxPort = 65535;
		lInfo() << "Setting maximum value of port range to " << maxPort;
	}
	if (maxPort < minPort) {
		lError() << "Invalid port range provided for stream type " << Utils::toString(stream.type)
		         << ": min=" << minPort << " max=" << maxPort;
		return 0;
	} else if (maxPort == minPort) {
		lWarning() << "Port range provided for stream type " << Utils::toString(stream.type)
		           << " has minimum and maximum value set to " << minPort
		           << ". It will not be possible to have multiple streams of the same type in the SDP";
	}
	const int rtp_port =
	    (maxPort == minPort) ? minPort : ((int)(bctbx_random() % (unsigned int)abs(maxPort - minPort)) + minPort);
	if ((rtp_port > maxPort) && (rtp_port < minPort)) {
		lWarning() << "The chosen port " << rtp_port << " is not within the desired range (min=" << minPort
		           << ", max=" << maxPort << ")";
	}

	return rtp_port;
}

int MediaSession::startInvite(const std::shared_ptr<Address> &destination,
                              const string &subject,
                              const std::shared_ptr<const Content> content) {
	L_D();

	if (d->getOp() == nullptr) d->createOp();
	linphone_core_stop_dtmf_stream(getCore()->getCCore());
	if (getCore()->getCCore()->sound_conf.play_sndcard && getCore()->getCCore()->sound_conf.capt_sndcard) {
		/* Give a chance to set card prefered sampling frequency */
		if (d->localDesc && (d->localDesc->streams.size() > 0) && (d->localDesc->streams[0].getMaxRate() > 0))
			ms_snd_card_set_preferred_sample_rate(getCore()->getCCore()->sound_conf.play_sndcard,
			                                      d->localDesc->streams[0].getMaxRate());
		d->getStreamsGroup().prepare();
	}

	if (d->localDesc) {
		for (auto &stream : d->localDesc->streams) {
			// In case of multicasting, choose a random port to send with the invite
			if (ms_is_multicast(L_STRING_TO_C(stream.rtp_addr))) {
				const auto rtp_port = getRandomRtpPort(stream);
				stream.rtp_port = rtp_port;
				stream.rtcp_port = stream.rtp_port + 1;
			}
		}
	}

	d->op->setLocalMediaDescription(d->localDesc);

	int result = CallSession::startInvite(destination, subject, content);
	if (result < 0) {
		if (d->state == CallSession::State::Error) d->stopStreams();
		return result;
	}
	return result;
}

void MediaSession::setRecordPath(const std::string &path) {
	L_D();
	d->getParams()->setRecordFilePath(path);
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i != nullptr) i->setRecordPath(path);
	lInfo() << "MediaSession " << this << " set record file path " << path;
}

bool MediaSession::startRecording() {
	L_D();
	if (d->getParams()->getRecordFilePath().empty()) {
		lError()
		    << "MediaSession::startRecording(): no output file specified. Use MediaSessionParams::setRecordFilePath()";
		return false;
	}
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i != nullptr) return i->startRecording();

	return false;
}

void MediaSession::stopRecording() {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i != nullptr) i->stopRecording();
}

bool MediaSession::isRecording() {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i != nullptr) return i->isRecording();
	return false;
}

void MediaSession::terminateBecauseOfLostMedia() {
	L_D();
	d->nonOpError = true;
	lWarning() << "Session [" << this << "] is going to be terminated because the media has been lost";
	linphone_error_info_set(d->ei, nullptr, LinphoneReasonIOError, 503, "Media lost", nullptr);
	terminate();
}

LinphoneStatus MediaSession::updateFromConference(const MediaSessionParams *msp, const string &subject) {
	return update(msp, CallSession::UpdateMethod::Default, false, subject);
}

LinphoneStatus MediaSession::update(const MediaSessionParams *msp,
                                    const UpdateMethod method,
                                    const bool isCapabilityNegotiationUpdate,
                                    const string &subject) {
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
		// The state must be set before recreating the media description in order for the method
		// forceStreamsDirAccordingToState (called by makeLocalMediaDescription) to set the stream directions
		// accordingly
		d->setState(nextState, "Updating call");
		d->broken = false; // Set broken state to false in case the request is not being triggered by repairIfBroken(),
		                   // so that we don't the job twice.
		d->setParams(new MediaSessionParams(*msp));
		const auto isIceRunning = getStreamsGroup().getIceService().isRunning();
		// Add capability negotiation attributes if capability negotiation is enabled and it is not a reINVITE following
		// conclusion of the capability negotiation procedure
		bool addCapabilityNegotiationAttributesToLocalMd =
		    isCapabilityNegotiationEnabled() && !isCapabilityNegotiationUpdate;
		bool isCapabilityNegotiationReInvite = isCapabilityNegotiationEnabled() && isCapabilityNegotiationUpdate;
		bool isOfferer = isCapabilityNegotiationUpdate || !getCore()->getCCore()->sip_conf.sdp_200_ack;
		d->localIsOfferer = isOfferer;
		d->makeLocalMediaDescription(d->localIsOfferer, addCapabilityNegotiationAttributesToLocalMd,
		                             isCapabilityNegotiationReInvite);
		const auto &localDesc = d->localDesc;

		auto updateCompletionTask = [this, method, subject, localDesc]() -> LinphoneStatus {
			L_D();

			CallSession::State previousState = d->state;
			CallSession::State newState;
			if (!d->isUpdateAllowed(newState)) {
				return -1;
			}

			// Do not set state calling setState because we shouldn't call the callbacks as it may trigger an infinite
			// loop. For example, at every state change, the core tries to run pending tasks for each and every session
			// and this may lead to queue the same action multiple times
			if (d->state != newState) {
				d->state = newState;
			}

			// We may running this code after ICE candidates have been gathered or ICE released task completed,
			// therefore the local description must be updated to include ICE candidates for every stream
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
		if (d->natPolicy && d->natPolicy->iceEnabled() && preparingStreams) {
			lInfo() << "Defer CallSession " << this << " (local address " << *getLocalAddress() << " remote address "
			        << *getRemoteAddress() << ") update to gather ICE candidates";
			d->queueIceGatheringTask(updateCompletionTask);
			return 0;
		} else if (isIceRunning) {
			// ICE negotiations are ongoing hence the update cannot be send right now
			lInfo() << "Ice negotiations are ongoing and update once they complete, therefore defer CallSession "
			        << this << " (local address " << *getLocalAddress() << " remote address " << *getRemoteAddress()
			        << ") update until Ice negotiations are completed.";
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
			// Ideally this should use the same logic as video (See video_stream_change_camera)
			// I.E. reconstruct only ms2 graphs without destroying the streams.
			// For now, we just stop and restart audio stream with new playback/capture card
			as->stop();
			d->updateStreams(d->resultDesc, d->state);
		} else {
			// Done directly by linphone_core_set_video_device().
		}
	}
	return result;
}

// -----------------------------------------------------------------------------

void MediaSession::requestNotifyNextVideoFrameDecoded() {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) i->requestNotifyNextVideoFrameDecoded();
}

LinphoneStatus MediaSession::takePreviewSnapshot(const string &file) {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) return i->takePreviewSnapshot(file);
	return -1;
}

LinphoneStatus MediaSession::takeVideoSnapshot(const string &file) {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) return i->takeVideoSnapshot(file);
	return -1;
}

void MediaSession::zoomVideo(float zoomFactor, float *cx, float *cy) {
	zoomVideo(zoomFactor, *cx, *cy);
}

void MediaSession::zoomVideo(float zoomFactor, float cx, float cy) {
	L_D();
	VideoControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
	if (i) i->zoomVideo(zoomFactor, cx, cy);
}

// -----------------------------------------------------------------------------

MediaSessionParams *MediaSession::createMediaSessionParams() {
	L_D();
	return d->createMediaSessionParams();
}

bool MediaSession::cameraEnabled() const {
#ifdef VIDEO_ENABLED
	L_D();
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<MS2VideoControl>(SalVideo);
	if (iface) {
		auto vs = iface->getVideoStream();
		if (vs && video_stream_local_screen_sharing_enabled(vs)) {
			auto streamIdx = getLocalThumbnailStreamIdx();
			if (streamIdx >= 0) iface = dynamic_cast<MS2VideoControl *>(d->getStreamsGroup().getStream(streamIdx));
		}
	}
	if (iface) return iface->cameraEnabled();
#endif
	return false;
}

void MediaSession::enableCamera(BCTBX_UNUSED(bool value)) {
#ifdef VIDEO_ENABLED
	L_D();
	const_cast<MediaSessionParams *>(getMediaParams())->enableCamera(value);
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<MS2VideoControl>(SalVideo);
	if (iface) {
		auto vs = iface->getVideoStream();
		if (vs && video_stream_local_screen_sharing_enabled(vs)) {
			auto streamIdx = getLocalThumbnailStreamIdx();
			if (streamIdx >= 0) iface = dynamic_cast<MS2VideoControl *>(d->getStreamsGroup().getStream(streamIdx));
		}
	}
	if (iface) iface->enableCamera(value);
#endif
}

bool MediaSession::echoCancellationEnabled() const {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	return i ? i->echoCancellationEnabled() : false;
}

void MediaSession::enableEchoCancellation(bool value) {
	L_D();
	AudioControlInterface *i = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (i) i->enableEchoCancellation(value);
}

bool MediaSession::echoLimiterEnabled() const {
	lWarning() << "MediaSession::echoLimiterEnabled() unimplemented.";
	return false;
}

void MediaSession::enableEchoLimiter(BCTBX_UNUSED(bool value)) {
	lWarning() << "MediaSession::enableEchoLimiter() unimplemented.";
}

bool MediaSession::getAllMuted() const {
	L_D();
	return d->getStreamsGroup().isMuted();
}

shared_ptr<CallStats> MediaSession::getAudioStats() const {
	return getStats(LinphoneStreamTypeAudio);
}

const string &MediaSession::getAuthenticationToken() const {
	L_D();
	return d->getStreamsGroup().getAuthenticationToken();
}

void MediaSession::storeAndSortRemoteAuthToken(const string &remoteAuthToken) const {
	L_D();
	return d->getStreamsGroup().storeAndSortRemoteAuthToken(remoteAuthToken);
}

const list<string> &MediaSession::getRemoteAuthenticationTokens() const {
	L_D();
	return d->getStreamsGroup().getRemoteAuthenticationTokens();
}

const bctbx_list_t *MediaSession::getCListRemoteAuthenticationTokens() const {
	L_D();
	return d->getStreamsGroup().getCListRemoteAuthenticationTokens();
}

bool MediaSession::getAuthenticationTokenVerified() const {
	L_D();
	return d->getStreamsGroup().getAuthenticationTokenVerified();
}

bool MediaSession::getZrtpCacheMismatch() const {
	L_D();
	return d->getStreamsGroup().getZrtpCacheMismatch();
}

float MediaSession::getAverageQuality() const {
	L_D();
	return d->getStreamsGroup().getAverageQuality();
}

MediaSessionParams *MediaSession::getCurrentParams() const {
	L_D();
	d->updateCurrentParams();
	return d->getCurrentParams();
}

float MediaSession::getCurrentQuality() const {
	L_D();
	return d->getStreamsGroup().getCurrentQuality();
}

const MediaSessionParams *MediaSession::getMediaParams() const {
	L_D();
	return d->getParams();
}

RtpTransport *MediaSession::getMetaRtcpTransport(int streamIndex) const {
	MS2Stream *s = dynamic_cast<MS2Stream *>(getStreamsGroup().getStream(streamIndex));
	if (!s) {
		lError() << "MediaSession::getMetaRtcpTransport(): no stream with index " << streamIndex;
		return nullptr;
	}
	return s->getMetaRtpTransports().second;
}

RtpTransport *MediaSession::getMetaRtpTransport(int streamIndex) const {
	MS2Stream *s = dynamic_cast<MS2Stream *>(getStreamsGroup().getStream(streamIndex));
	if (!s) {
		lError() << "MediaSession::getMetaRtcpTransport(): no stream with index " << streamIndex;
		return nullptr;
	}
	return s->getMetaRtpTransports().first;
}

float MediaSession::getMicrophoneVolumeGain() const {
	AudioControlInterface *iface = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface) {
		return iface->getMicGain();
	} else {
		lError() << "Could not get record volume: no audio stream";
		return -1.0f;
	}
}

void MediaSession::setMicrophoneVolumeGain(float value) {
	AudioControlInterface *iface = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface) iface->setMicGain(value);
	else lError() << "Could not set record volume: no audio stream";
}

float MediaSession::getSpeakerVolumeGain() const {
	AudioControlInterface *iface = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface) return iface->getSpeakerGain();
	else {
		lError() << "Could not get playback volume: no audio stream";
		return -1.0f;
	}
}

void MediaSession::setSpeakerVolumeGain(float value) {
	AudioControlInterface *iface = getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface) iface->setSpeakerGain(value);
	else lError() << "Could not set playback volume: no audio stream";
}

#ifdef VIDEO_ENABLED
static bool_t compareFunc(Stream *s, const std::string &label, const bool isMe, const bool isThumbnail) {
	if (s->getType() == SalVideo) {
		return (!isMe || (static_cast<MS2VideoStream *>(s)->isThumbnail() == isThumbnail)) &&
		       (label.compare(s->getLabel()) == 0);
	}
	return false;
};
#endif

VideoControlInterface *MediaSession::getVideoControlInterface(BCTBX_UNUSED(const std::string label),
                                                              BCTBX_UNUSED(const bool isMe),
                                                              BCTBX_UNUSED(const bool isThumbnail)) const {
	VideoControlInterface *iface = nullptr;
#ifdef VIDEO_ENABLED
	if ((getState() != CallSession::State::End) && (getState() != CallSession::State::Released)) {
		if (label.empty()) {
			iface = getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
			if (!iface) {
				lError() << "Unable to find video control interface for main stream";
			}
		} else {
			auto s = getStreamsGroup().lookupStream(compareFunc, label, isMe, isThumbnail);
			if (s) {
				iface = dynamic_cast<VideoControlInterface *>(s);
				if (iface == nullptr) {
					lError() << "stream " << s << " with label " << label
					         << " cannot be casted to VideoControlInterface";
				}
			} else {
				lError() << "Unable to find stream with label " << label;
			}
		}
	}
#endif
	return iface;
}

/**
 * [-- Window ID distribution for screen sharing ---]
 *
 * Stream       Display !SS     SS_P      SS_me
 * -------------------------------------------------
 * Main :       Output  N       N       -
 *              Output2 P       P       N
 *
 * Thumbnail:   Output2 -       -       P
 *
 * Recv:        Output  PD_N    PD_N    PD_N
 * -------------------------------------------------
 *  N = Native
 *  P = Preview
 *  SS_P = Participant is sharing a screen
 *  SS_me = Me(local) is sharing a screen
 *  SS = SS_P + SS_me
 *  PD_N = Native for Participant Device
 *
 *Notes:
 *  - Updates only occured if Me on local is sharing a screen.
 *  - Each updates must clean previous window ID to avoid concurrency.
 *
 * Implementation remarks:
 *  - MSVideoControl controls the changes of N/P inside a stream = enableLocalScreenSharing() +
 *video_stream_local_screen_sharing_enabled()
 *  - MediaSession controls the call between Main and Thumbnail = set/get/create_window_id + render()
 */
void *MediaSession::getNativeVideoWindowId(const std::string label, const bool isMe, const bool isThumbnail) const {
	auto iface = getVideoControlInterface(label, isMe, isThumbnail);
	if (iface) {
		return iface->getNativeWindowId();
	}
	return nullptr;
}

void MediaSession::setNativeVideoWindowId(void *id, const std::string label, const bool isMe, const bool isThumbnail) {
	auto iface = getVideoControlInterface(label, isMe, isThumbnail);
	if (iface) {
		iface->setNativeWindowId(id);
	}
}

void MediaSession::setNativePreviewWindowId(BCTBX_UNUSED(void *id)) {
#ifdef VIDEO_ENABLED
	L_D();
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<MS2VideoControl>(SalVideo);
	if (iface) {
		auto vs = iface->getVideoStream();
		if (vs && video_stream_local_screen_sharing_enabled(vs)) {
			auto streamIdx = getLocalThumbnailStreamIdx();
			if (streamIdx < 0) return;
			auto videostream = dynamic_cast<VideoControlInterface *>(d->getStreamsGroup().getStream(streamIdx));
			videostream->setNativePreviewWindowId(id);
		} else iface->setNativePreviewWindowId(id);
	}
#endif
}

void *MediaSession::getNativePreviewVideoWindowId() const {
#ifdef VIDEO_ENABLED
	L_D();
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<MS2VideoControl>(SalVideo);
	if (iface) {
		auto vs = iface->getVideoStream();
		if (vs && video_stream_local_screen_sharing_enabled(vs)) {
			auto streamIdx = getLocalThumbnailStreamIdx();
			if (streamIdx < 0) return nullptr;
			auto videostream = dynamic_cast<VideoControlInterface *>(d->getStreamsGroup().getStream(streamIdx));
			return videostream->getNativePreviewWindowId();
		} else return iface->getNativePreviewWindowId();
	}
#endif
	return nullptr;
}

void *MediaSession::createNativePreviewVideoWindowId() const {
#ifdef VIDEO_ENABLED
	L_D();
	auto iface = d->getStreamsGroup().lookupMainStreamInterface<MS2VideoControl>(SalVideo);
	if (iface) {
		auto vs = iface->getVideoStream();
		if (vs && video_stream_local_screen_sharing_enabled(vs)) {
			auto streamIdx = getLocalThumbnailStreamIdx();
			if (streamIdx < 0) return nullptr;
			auto videostream = dynamic_cast<MS2VideoControl *>(d->getStreamsGroup().getStream(streamIdx));
			return videostream->createNativePreviewWindowId();
		} else return iface->createNativePreviewWindowId();
	}
#endif
	return nullptr;
}

void *MediaSession::createNativeVideoWindowId(const std::string label, const bool isMe, const bool isThumbnail) const {
	auto iface = getVideoControlInterface(label, isMe, isThumbnail);
	if (iface) {
		return iface->createNativeWindowId();
	}
	return nullptr;
}

const CallSessionParams *MediaSession::getParams() const {
	L_D();
	return d->params;
}

float MediaSession::getPlayVolume() const {
	L_D();
	AudioControlInterface *iface = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
	if (iface) return iface->getPlayVolume();
	return LINPHONE_VOLUME_DB_LOWEST;
}

float MediaSession::getRecordVolume() const {
	L_D();

	if (d->state == CallSession::State::StreamsRunning) {
		AudioControlInterface *iface = d->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
		if (iface) return iface->getRecordVolume();
	}
	return LINPHONE_VOLUME_DB_LOWEST;
}

const MediaSessionParams *MediaSession::getRemoteParams() const {
	L_D();
	if (d->op) {
		const std::shared_ptr<SalMediaDescription> &md = d->op->getRemoteMediaDescription();
		MediaSessionParams *params = nullptr;
		if (md) {
			params = new MediaSessionParams();

			bool screenSharingEnabled = false;
			const auto screenSharingStream =
			    md->findStreamWithContent(MediaSessionPrivate::ScreenSharingContentAttribute);
			if (screenSharingStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
				bool isServer = linphone_core_conference_server_enabled(getCore()->getCCore());
				SalStreamDir screenSharingDirectionStreamDir = screenSharingStream.getDirection();
				screenSharingEnabled =
				    (screenSharingDirectionStreamDir == ((isServer) ? SalStreamSendOnly : SalStreamRecvOnly)) ||
				    (screenSharingDirectionStreamDir == SalStreamSendRecv);
			}
			params->enableScreenSharing(screenSharingEnabled);
			if (d->isInConference()) {
				params->setConferenceVideoLayout(computeConferenceLayout(md));
			}

			const SalStreamDescription &audioStream = md->findBestStream(SalAudio);
			if (audioStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
				const auto audioDir = d->getDirFromMd(md, SalAudio);
				params->enableAudio(audioStream.enabled());
				params->setAudioDirection(audioDir);
				params->setMediaEncryption(audioStream.hasSrtp() ? LinphoneMediaEncryptionSRTP
				                                                 : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeAudio,
				                                                  audioStream.custom_sdp_attributes);
			} else params->enableAudio(false);

			const auto streamIdx = getThumbnailStreamIdx(md);
			const auto &videoStream = (streamIdx == -1) ? md->findBestStream(SalVideo)
			                                            : md->getStreamAtIdx(static_cast<unsigned int>(streamIdx));
			if (videoStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
				const auto videoDir = d->getDirFromMd(md, SalVideo);
				const auto &videoEnabled = videoStream.enabled();
				params->enableVideo(videoEnabled || (videoDir != LinphoneMediaDirectionInactive));
				params->setVideoDirection(videoDir);
				params->setMediaEncryption(videoStream.hasSrtp() ? LinphoneMediaEncryptionSRTP
				                                                 : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeVideo,
				                                                  videoStream.custom_sdp_attributes);
				// The camera is enabled if:
				// - the thumbnail stream is enabled
				// - the thumbnail stream's direction is sendrecv or sendonly
				const auto &thumbnailDirection = videoStream.getDirection();
				params->enableCamera(
				    ((thumbnailDirection == SalStreamSendRecv) || (thumbnailDirection == SalStreamSendOnly)) &&
				    videoEnabled);
			} else {
				params->enableVideo(false);
				params->enableCamera(false);
			}

			const SalStreamDescription &textStream = md->findBestStream(SalText);
			if (textStream != Utils::getEmptyConstRefObject<SalStreamDescription>()) {
				params->enableRealtimeText(textStream.enabled());
				params->setMediaEncryption(textStream.hasSrtp() ? LinphoneMediaEncryptionSRTP
				                                                : LinphoneMediaEncryptionNone);
				params->getPrivate()->setCustomSdpMediaAttributes(LinphoneStreamTypeText,
				                                                  textStream.custom_sdp_attributes);
			} else params->enableRealtimeText(false);

			if (!params->videoEnabled()) {
				if ((md->bandwidth > 0) && (md->bandwidth <= linphone_core_get_edge_bw(getCore()->getCCore())))
					params->enableLowBandwidth(true);
			}
			if (md->name[0] != '\0') params->setSessionName(md->name);
			params->getPrivate()->setCustomSdpAttributes(md->custom_sdp_attributes);
			params->enableRtpBundle(!md->bundles.empty());
			params->setRecordingState(md->record);

			const auto &times = md->times;
			if (times.size() > 0) {
				const auto [start, end] = times.front();
				params->getPrivate()->setStartTime(start);
				params->getPrivate()->setEndTime(end);
			}
		} else {
			lInfo() << "Unable to retrieve remote streams because op " << d->op
			        << " has not received yet a valid SDP from the other party";
		}
		const SalCustomHeader *ch = d->op->getRecvCustomHeaders();
		if (ch) {
			if (!params) params = new MediaSessionParams();
			params->getPrivate()->setCustomHeaders(ch);

			const char *supported = params->getCustomHeader("supported");
			params->enableRecordAware(supported && strstr(supported, "record-aware"));
		}
		const list<Content> &additionnalContents = d->op->getRemoteBodies();
		for (auto &content : additionnalContents) {
			if (!params) params = new MediaSessionParams();
			if (content.getContentType() != ContentType::Sdp) params->addCustomContent(Content::create(content));
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

shared_ptr<CallStats> MediaSession::getStats(LinphoneStreamType type) const {
	L_D();
	if (type == LinphoneStreamTypeUnknown) return nullptr;
	shared_ptr<CallStats> stats = nullptr;
	shared_ptr<CallStats> statsCopy = nullptr;
	Stream *s = d->getStream(type);
	if (s && (stats = s->getStats())) {
		statsCopy = stats->clone()->toSharedPtr();
	}
	return statsCopy;
}

int MediaSession::getStreamCount() const {
	L_D();
	return (int)d->getStreamsGroup().size();
}

MSFormatType MediaSession::getStreamType(int streamIndex) const {
	L_D();
	Stream *s = d->getStreamsGroup().getStream(streamIndex);
	if (s) {
		switch (s->getType()) {
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

shared_ptr<CallStats> MediaSession::getTextStats() const {
	return getStats(LinphoneStreamTypeText);
}

shared_ptr<CallStats> MediaSession::getVideoStats() const {
	return getStats(LinphoneStreamTypeVideo);
}

bool MediaSession::mediaInProgress() const {
	L_D();
	for (auto &stream : d->getStreamsGroup().getStreams()) {
		if (stream) {
			shared_ptr<CallStats> stats = stream->getStats();
			if (stats && stats->getIceState() == LinphoneIceStateInProgress) {
				return true;
			}
		}
	}
	return false;
}

void MediaSession::setAuthenticationTokenVerified(bool value) {
	L_D();
	d->getStreamsGroup().setAuthTokenVerified(value);
	if (!value) {
		char *peerDeviceId = nullptr;
		auto encryptionEngine = getCore()->getEncryptionEngine();
		if (encryptionEngine) { // inform lime that zrtp no longuer guaranty the trust
			const SalAddress *remoteAddress = d->getOp()->getRemoteContactAddress();
			peerDeviceId = sal_address_as_string_uri_only(remoteAddress);
			encryptionEngine->authenticationRejected(peerDeviceId);
			ms_free(peerDeviceId);
		}
	} else {
		d->getStreamsGroup().setZrtpCacheMismatch(false);
	}
	d->propagateEncryptionChanged();
}

void MediaSession::setAuthenticationTokenCheckDone(bool value) {
	L_D();
	d->getStreamsGroup().setAuthenticationTokenCheckDone(value);
}

void MediaSession::checkAuthenticationTokenSelected(const string &selectedValue, const string &halfAuthToken) {
	L_D();
	bool value = (selectedValue.compare(halfAuthToken) == 0) ? true : false;
	notifyAuthenticationTokenVerified(value);
	setAuthenticationTokenCheckDone(true);
	setAuthenticationTokenVerified(value);
	if (!value) d->stopStreams();
}

void MediaSession::skipZrtpAuthentication() {
	L_D();
	d->skipZrtpAuthentication();
}

void MediaSession::setParams(const MediaSessionParams *msp) {
	L_D();
	switch (d->state) {
		case CallSession::State::Idle:
		case CallSession::State::OutgoingInit:
		case CallSession::State::IncomingReceived:
		case CallSession::State::PushIncomingReceived:
			d->setParams(msp ? new MediaSessionParams(*msp) : nullptr);
			// Update the local media description.
			d->makeLocalMediaDescription(
			    (d->state == CallSession::State::OutgoingInit ? !getCore()->getCCore()->sip_conf.sdp_200_ack : false),
			    isCapabilityNegotiationEnabled(), false);
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

StreamsGroup &MediaSession::getStreamsGroup() const {
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

	const auto &currentInputDevice = getInputAudioDevice();
	// If pointer toward the new device has changed or at least one member of the audio device changed or no current
	// audio device is set, then return true
	bool change =
	    currentInputDevice ? ((audioDevice != currentInputDevice) || (*audioDevice != *currentInputDevice)) : true;

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

	const auto &currentOutputDevice = getOutputAudioDevice();
	// If pointer toward the new device has changed or at least one member of the audio device changed or no current
	// audio device is set, then return true
	bool change =
	    currentOutputDevice ? ((audioDevice != currentOutputDevice) || (*audioDevice != *currentOutputDevice)) : true;

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

bool MediaSession::pausedByApp() const {
	L_D();
	return d->pausedByApp;
}

void MediaSession::notifySpeakingDevice(uint32_t ssrc, bool isSpeaking) {
	const auto conference = getCore()->findConference(getSharedFromThis(), false);
	if (conference) {
		conference->notifySpeakingDevice(ssrc, isSpeaking);
	} else {
		lDebug() << "IsSpeaking: unable to notify speaking device because there is no conference.";
	}
}

void MediaSession::notifyMutedDevice(uint32_t ssrc, bool muted) {
	const auto conference = getCore()->findConference(getSharedFromThis(), false);
	if (conference) {
		conference->notifyMutedDevice(ssrc, muted);
	} else {
		lDebug() << "IsMuted: unable to notify muted device because there is no conference.";
	}
}

void MediaSession::onGoClearAckSent() {
	notifyGoClearAckSent();
}

std::shared_ptr<ParticipantDevice> MediaSession::getParticipantDevice(const LinphoneStreamType type,
                                                                      const std::string &label) {
	const auto conference = getCore()->findConference(getSharedFromThis(), false);
	if (conference) {
		return conference->findParticipantDeviceByLabel(type, label);
	}
	return nullptr;
}

void *MediaSession::getParticipantWindowId(const std::string &label) {
	auto participantDevice = getParticipantDevice(LinphoneStreamTypeVideo, label);
	if (participantDevice) return participantDevice->getWindowId();
	else return nullptr;
}

const std::shared_ptr<const VideoSourceDescriptor> MediaSession::getVideoSourceDescriptor() const {
	return mVideoSourceDescriptor;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void MediaSession::setVideoSource(const std::shared_ptr<const VideoSourceDescriptor> &descriptor) {
#ifdef VIDEO_ENABLED
	MS2VideoStream *stream = getStreamsGroup().lookupMainStreamInterface<MS2VideoStream>(SalVideo);
	stream->setVideoSource(descriptor);
	mVideoSourceDescriptor = descriptor;
#else
	lError() << "Cannot change the video source as video support is not enabled";
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

std::shared_ptr<const VideoSourceDescriptor> MediaSession::getVideoSource() const {
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

uint32_t MediaSession::getSsrc(std::string content) const {
	L_D();

	uint32_t ssrc = 0;
	const auto remoteMediaDesc = d->getOp()->getRemoteMediaDescription();
	if (remoteMediaDesc) {
		const auto idx = remoteMediaDesc->findIdxStreamWithContent(content);
		if (idx != -1) {
			const SalStreamDescription &videoStream = remoteMediaDesc->getStreamAtIdx(static_cast<unsigned int>(idx));
			const auto &videoStreamCfg = videoStream.getActualConfiguration();
			ssrc = videoStreamCfg.conference_ssrc;
		}
	}
	return ssrc;
}

uint32_t MediaSession::getSsrc(LinphoneStreamType type) const {
	L_D();

	uint32_t ssrc = 0;
	const auto remoteMediaDesc = d->getOp()->getRemoteMediaDescription();
	if (remoteMediaDesc) {
		switch (type) {
			case LinphoneStreamTypeAudio: {
				const auto &audioStream = remoteMediaDesc->getActiveStreamOfType(SalAudio, 0);
				const auto &audioStreamCfg = audioStream.getActualConfiguration();
				ssrc = audioStreamCfg.conference_ssrc;
			} break;
			case LinphoneStreamTypeVideo: {
				const auto &videoStreamIdx = getMainVideoStreamIdx(remoteMediaDesc);
				if ((videoStreamIdx >= 0) &&
				    (d->getOp()->getFinalMediaDescription()->nbActiveStreamsOfType(SalVideo) > 0)) {
					const SalStreamDescription &videoStream =
					    remoteMediaDesc->getStreamAtIdx(static_cast<unsigned int>(videoStreamIdx));
					const auto &videoStreamCfg = videoStream.getActualConfiguration();
					ssrc = videoStreamCfg.conference_ssrc;
				}
			} break;
			case LinphoneStreamTypeText:
			case LinphoneStreamTypeUnknown:
				ssrc = 0;
				break;
		}
	}

	return ssrc;
}

int MediaSession::getMainVideoStreamIdx(const std::shared_ptr<SalMediaDescription> &md) const {
	L_D();
	// In order to set properly the negotiated parameters, we must know if the client is sending video to the
	// conference, i.e. look at the thumbnail stream direction. In order to do so, we must know the label of the
	// searched thumbnail stream. The local case is quite straightforward because all labels are known, but for the
	// client is more difficult as the NOTIFY message may have not come or been processed. The algorithm below searches
	// for the label in the main stream and then reuses the label to look for the desired thumbnail stream
	auto streamIdx = -1;
	if (md) {
		const auto conference = getCore()->findConference(getSharedFromThis(), false);
		if (conference && d->op) {
			const bool isInLocalConference = d->getParams()->getPrivate()->getInConference();
			const auto &confLayout = computeConferenceLayout(isInLocalConference ? d->op->getRemoteMediaDescription()
			                                                                     : d->op->getLocalMediaDescription());
			const auto isConferenceLayoutActiveSpeaker = (confLayout == ConferenceLayout::ActiveSpeaker);
			const auto isConferenceLayoutGrid = (confLayout == ConferenceLayout::Grid);
			const auto &participantDevice = (isInLocalConference)
			                                    ? conference->findParticipantDevice(getSharedFromThis())
			                                    : conference->getMe()->findDevice(getSharedFromThis());

			// Try to find a stream with the screen sharing content attribute as it will be for sure the main video
			// stream. Indeed, screen sharing can be enabled regardless of the conference layout, it is needed to always
			// make this try.
			std::string mainStreamAttrValue = MediaSessionPrivate::ScreenSharingContentAttribute;
			streamIdx = md->findIdxStreamWithContent(mainStreamAttrValue);
			if (streamIdx == -1) {
				// If no stream with the screen sharing content is found, then try with regular attributes for each of
				// the different layouts
				if (isConferenceLayoutActiveSpeaker) {
					mainStreamAttrValue = MediaSessionPrivate::ActiveSpeakerVideoContentAttribute;
				} else if (isConferenceLayoutGrid) {
					mainStreamAttrValue = MediaSessionPrivate::GridVideoContentAttribute;
				} else {
					lError() << "Unable to determine attribute of main video stream of session " << this
					         << " (local addres " << *getLocalAddress() << " remote address " << *getRemoteAddress()
					         << ") in conference " << *conference->getConferenceAddress() << ":";
					lError() << " - grid layout: " << isConferenceLayoutGrid;
					lError() << " - active speaker layout: " << isConferenceLayoutActiveSpeaker;
					lError() << " - device is screen sharing: "
					         << (participantDevice && participantDevice->screenSharingEnabled());
				}
				streamIdx = md->findIdxStreamWithContent(mainStreamAttrValue);
			}
			if (streamIdx == -1) {
				// The stream index was not found despite all efforts
				lDebug() << "Unable to find main video stream of session " << this << " (local addres "
				         << *getLocalAddress() << " remote address " << *getRemoteAddress() << "):";
				lDebug() << " - no stream with content \"" << MediaSessionPrivate::ScreenSharingContentAttribute
				         << "\" is found";
				lDebug() << " - grid layout: " << isConferenceLayoutGrid;
				lDebug() << " - active speaker layout: " << isConferenceLayoutActiveSpeaker;
				lDebug() << " - device is screen sharing: "
				         << (participantDevice && participantDevice->screenSharingEnabled());
			}
		}
		if (streamIdx == -1) {
			streamIdx = md->findIdxBestStream(SalVideo);
		}
	}

	return streamIdx;
}

int MediaSession::getLocalThumbnailStreamIdx() const {
	L_D();
	return getThumbnailStreamIdx(d->op ? d->op->getLocalMediaDescription() : nullptr);
}

int MediaSession::getThumbnailStreamIdx(const std::shared_ptr<SalMediaDescription> &md) const {
	L_D();
	// In order to set properly the negotiated parameters, we must know if the client is sending video to the
	// conference, i.e. look at the thumbnail stream direction. In order to do so, we must know the label of the
	// searched thumbnail stream. The local case is quite straightforward because all labels are known, but for the
	// client is more difficult as the NOTIFY message may have not come or been processed. The algorithm below searches
	// for the label in the main stream and then reuses the label to look for the desired thumbnail stream
	auto streamIdx = -1;
	if (md) {
		const auto conference = getCore()->findConference(getSharedFromThis(), false);
		if (conference) {
			const auto content = MediaSessionPrivate::ThumbnailVideoContentAttribute;
			const bool isInLocalConference = d->getParams()->getPrivate()->getInConference();
			std::string label;
			if (isInLocalConference) {
				auto device = conference->findParticipantDevice(getSharedFromThis());
				if (device) {
					label = device->getThumbnailStreamLabel();
				}
			} else {
				auto device = conference->getMe()->findDevice(getSharedFromThis());
				if (device) {
					label = device->getThumbnailStreamLabel();
				}
			}
			streamIdx = md->findIdxStreamWithContent(content, label);
		}
	}
	return streamIdx;
}

void MediaSession::setEkt(const MSEKTParametersSet *ekt_params) const {
	getStreamsGroup().setEkt(ekt_params);
}

LinphoneMediaDirection MediaSession::getDirectionOfStream(BCTBX_UNUSED(const std::string content)) const {
	auto direction = LinphoneMediaDirectionInactive;
#ifdef VIDEO_ENABLED
	L_D();
	// If we are a conference server, we must look at the incoming INVITE as this SDP has the content attribute.
	// Look at the local description in all the other scenarions (remote conferece on a server or call)
	const auto &op = d->getOp();
	if (op) {
		bool isServer = linphone_core_conference_server_enabled(getCore()->getCCore());
		std::shared_ptr<SalMediaDescription> offer =
		    (isServer) ? op->getRemoteMediaDescription() : op->getLocalMediaDescription();
		if (offer) {
			// Look for the index of the stream containing the requested attribute in the offered SDP
			const auto idx = offer->findIdxStreamWithContent(content);
			if (idx != -1) {
				std::shared_ptr<SalMediaDescription> &md = op->getFinalMediaDescription();
				try {
					// Look the direction of the stream at the index previously found in the negotiated SDP to know
					// if the stream has been accepted.
					const auto &stream = md->streams.at(static_cast<size_t>(idx));
					direction = MediaSessionParamsPrivate::salStreamDirToMediaDirection(stream.getDirection());
					if (isServer) {
						if (direction == LinphoneMediaDirectionRecvOnly) {
							direction = LinphoneMediaDirectionSendOnly;
						} else if (direction == LinphoneMediaDirectionSendOnly) {
							direction = LinphoneMediaDirectionRecvOnly;
						}
					}
				} catch (std::out_of_range &) {
				}
			}
		}
	}
#endif
	return direction;
}

// This method return if the INVITE session has negotiated the screen sharing. In the case of a conference, the server
// is expected to confirm that a participant is actually screen sharing through the SUBSCRIBE/NOTIFY dialog. The
// conference server will use the value returned by this method to grant screen sharing to a participant. In fact, it is
// possible to create conferences that don't have the video capability and in suc a case screen sharing requests will be
// rejected all the time
bool MediaSession::isScreenSharingNegotiated() const {
#ifdef VIDEO_ENABLED
	L_D();
	const auto &isInConference = d->isInConference();
	bool sdpScreenSharingAccepted = false;
	bool isServer = linphone_core_conference_server_enabled(getCore()->getCCore());
	// If the session is in a local conference and the core is not a conference server, it means that the conference is
	// hosted on a device. No reINVITE is sent for the local participant that starts screen sharing
	if (!isServer && isInConference) {
		sdpScreenSharingAccepted = true;
	} else {
		const auto direction = getDirectionOfStream(MediaSessionPrivate::ScreenSharingContentAttribute);
		sdpScreenSharingAccepted =
		    ((direction == LinphoneMediaDirectionSendOnly) || (direction == LinphoneMediaDirectionSendRecv));
	}

	return sdpScreenSharingAccepted;
#else
	return false;
#endif
}

bool MediaSession::requestThumbnail(const std::shared_ptr<ParticipantDevice> &device) const {
	const bool isInLocalConference = getMediaParams()->getPrivate()->getInConference();
	const auto &confLayout = isInLocalConference ? getRemoteParams()->getConferenceVideoLayout()
	                                             : getMediaParams()->getConferenceVideoLayout();
	bool isGridLayout = (confLayout == ConferenceLayout::Grid);
	// Use thumbnail if:
	// - the stream is of type video
	// - the device is not screensharing or the layout is not grid
	// A client that is in a conference with Grid layout has to request the main stream of the participant that is
	// screensharing
	return (!device->screenSharingEnabled() || !isGridLayout);
}

LINPHONE_END_NAMESPACE
