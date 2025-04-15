/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "account/account.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call-log.h"
#include "call/call.h"
#include "conference/conference-scheduler.h"
#include "conference/conference.h"
#include "conference/params/call-session-params-p.h"
#include "conference/participant.h"
#include "conference/session/call-session-p.h"
#include "conference/session/call-session.h"
#include "core/core-p.h"
#include "factory/factory.h"
#include "linphone/api/c-content.h"
#include "linphone/core.h"
#include "logger/logger.h"
#include "private.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const std::map<CallSession::PredefinedSubjectType, std::string> CallSession::predefinedSubject{
    {CallSession::PredefinedSubjectType::Conference, "Conference"},
    {CallSession::PredefinedSubjectType::InternalUpdate, "ICE processing concluded"},
    {CallSession::PredefinedSubjectType::Refresh, "Refreshing"},
    {CallSession::PredefinedSubjectType::MediaChange, "Media change"},
    {CallSession::PredefinedSubjectType::CallOnHold, "Call on hold"},
    {CallSession::PredefinedSubjectType::BothPartiesOnHold, "Call on hold for me too"},
    {CallSession::PredefinedSubjectType::Resuming, "Call resuming"}};

// =============================================================================

int CallSessionPrivate::computeDuration() const {
	if (log->getConnectedTime() == 0) {
		if (log->getStartTime() == 0) return 0;
		return (int)(ms_time(nullptr) - log->getStartTime());
	}
	return (int)(ms_time(nullptr) - log->getConnectedTime());
}

/*
 * Initialize call parameters according to incoming call parameters. This is to avoid to ask later (during reINVITEs)
 * for features that the remote end apparently does not support. This features are: privacy, video...
 */
void CallSessionPrivate::initializeParamsAccordingToIncomingCallParams() {
	currentParams->setPrivacy((LinphonePrivacyMask)op->getPrivacy());
}

void CallSessionPrivate::notifyReferState() {
	SalCallOp *refererOp = referer->getPrivate()->getOp();
	if (refererOp) refererOp->notifyReferState(op);
}

void CallSessionPrivate::restorePreviousState() {
	setState(prevState, prevMessageState);
}

void CallSessionPrivate::setState(CallSession::State newState, const string &message) {
	L_Q();
	// Keep a ref on the CallSession, otherwise it might get destroyed before the end of the method
	shared_ptr<CallSession> ref = q->getSharedFromThis();
	if (state != newState) {
		prevState = state;
		prevMessageState = messageState;
		if ((state == CallSession::State::StreamsRunning) || (state == CallSession::State::Paused)) {
			lastStableState = state;
			lastStableMessageState = messageState;
		}
		// Make sanity checks with call state changes. Any bad transition can result in unpredictable results
		// or irrecoverable errors in the application.
		if ((state == CallSession::State::End) || (state == CallSession::State::Error)) {
			if (newState != CallSession::State::Released) {
				lFatal() << "Abnormal resurection from " << Utils::toString(state) << " to "
				         << Utils::toString(newState) << " of call session [" << q << "], aborting";
				return;
			}
		} else if ((newState == CallSession::State::Released) && (prevState != CallSession::State::Error) &&
		           (prevState != CallSession::State::End)) {
			lFatal() << "Attempt to move CallSession [" << q
			         << "] to Released state while it was not previously in Error or End state, aborting";
			return;
		}

		lInfo() << "CallSession [" << q << "] moving from state " << Utils::toString(state) << " to "
		        << Utils::toString(newState);

		if (newState != CallSession::State::Referred) {
			// CallSession::State::Referred is rather an event, not a state.
			// Indeed it does not change the state of the call (still paused or running).
			state = newState;
			messageState = message;
		}

		auto core = q->getCore();
		switch (newState) {
			case CallSession::State::IncomingReceived: {
				if (op) {
					auto call = core->getCallByCallId(op->getCallId());
					// If there is an active call with the same call ID as the session, then this session may belong to
					// a conference
					if (call) {
						const std::shared_ptr<Address> to = Address::create(op->getTo());
						// Server conference
						if (to->hasUriParam(Conference::ConfIdParameter)) {
							shared_ptr<Conference> conference = core->findConference(
							    ConferenceId(to, to, q->getCore()->createConferenceIdParams()), false);

							if (conference) {
								// The call is for a conference stored in the core
								ref->addListener(conference);
							}
						} else if (op->getRemoteContactAddress()) {
							if (sal_address_has_param(op->getRemoteContactAddress(),
							                          Conference::IsFocusParameter.c_str())) {
								const auto &conferenceInfo = Utils::createConferenceInfoFromOp(op, true);
								if (conferenceInfo->getUri()->isValid()) {
#ifdef HAVE_DB_STORAGE
									auto &mainDb = core->getPrivate()->mainDb;
									if (mainDb) {
										lInfo() << "Inserting conference information to database related to conference "
										        << *conferenceInfo->getUri();
										mainDb->insertConferenceInfo(conferenceInfo);
									}
#endif // HAVE_DB_STORAGE
									auto log = q->getLog();
									log->setConferenceInfo(conferenceInfo);
								}
							}
						}
					}
				}
			} break;
			case CallSession::State::End:
			case CallSession::State::Error:
				switch (linphone_error_info_get_reason(q->getErrorInfo())) {
					case LinphoneReasonDeclined:
						if (log->getStatus() !=
						    LinphoneCallMissed) // Do not re-change the status of a call if it's already set
							log->setStatus(LinphoneCallDeclined);
						break;
					case LinphoneReasonNotAnswered:
						if (log->getDirection() == LinphoneCallIncoming) log->setStatus(LinphoneCallMissed);
						break;
					case LinphoneReasonNone:
						if (log->getDirection() == LinphoneCallIncoming) {
							if (ei) {
								int code = linphone_error_info_get_protocol_code(ei);
								if ((code >= 200) && (code < 300)) log->setStatus(LinphoneCallAcceptedElsewhere);
								else if (code == 487) log->setStatus(LinphoneCallMissed);
							}
						}
						break;
					case LinphoneReasonDoNotDisturb:
						if (log->getDirection() == LinphoneCallIncoming) {
							if (ei) {
								int code = linphone_error_info_get_protocol_code(ei);
								if ((code >= 600) && (code < 700)) log->setStatus(LinphoneCallDeclinedElsewhere);
							}
						}
						break;
					default:
						break;
				}
				break;
			case CallSession::State::Connected:
				log->setStatus(LinphoneCallSuccess);
				log->setConnectedTime(ms_time(nullptr));
				if ((q->sdpFoundInRemoteBody() || q->sdpFoundInLocalBody()) && reportEvents()) {
					core->reportConferenceCallEvent(EventLog::Type::ConferenceCallConnected, log, nullptr);
				}
				break;
			default:
				break;
		}

		if (message.empty()) {
			lError() << "You must fill a reason when changing call state (from " << Utils::toString(prevState) << " to "
			         << Utils::toString(state) << ")";
		}

		// So call log duration will be available in those states in the app listener
		if ((newState == CallSession::State::End) || (newState == CallSession::State::Error)) {
			setTerminated();
		}

		q->notifyCallSessionStateChanged(newState, message);

		if (newState == CallSession::State::Released) {
			setReleased(); /* Shall be performed after app notification */
		}
	}
}

void CallSessionPrivate::onCallStateChanged(BCTBX_UNUSED(LinphoneCall *call),
                                            BCTBX_UNUSED(LinphoneCallState state),
                                            BCTBX_UNUSED(const std::string &message)) {
	L_Q();
	auto zis = q->getSharedFromThis();
	q->getCore()->doLater([zis, this] {
		(void)zis;
		this->executePendingActions();
	});
}

void CallSessionPrivate::executePendingActions() {
	L_Q();
	bool_t networkReachable = linphone_core_is_network_reachable(q->getCore()->getCCore());
	if (networkReachable && (state != CallSession::State::End) && (state != CallSession::State::Released) &&
	    (state != CallSession::State::Error)) {
		std::queue<std::function<LinphoneStatus()>> unsuccessfulActions;
		std::queue<std::function<LinphoneStatus()>> copyPendingActions;
		copyPendingActions.swap(pendingActions);

		while (copyPendingActions.empty() == false) {
			// Store std::function in a temporary variable in order to take it out of the queue before executing it
			const auto f = copyPendingActions.front();
			copyPendingActions.pop();
			// Execute method
			const auto result = f();
			if (result != 0) {
				unsuccessfulActions.push(f);
			}
		}
		while (!unsuccessfulActions.empty()) {
			pendingActions.push(unsuccessfulActions.front());
			unsuccessfulActions.pop();
		}
	}
}

void CallSessionPrivate::setTransferState(CallSession::State newState) {
	L_Q();
	if (newState == transferState) {
		lError() << "Unable to change transfer state for CallSession [" << q << "] from ["
		         << Utils::toString(transferState) << "] to [" << Utils::toString(newState) << "]";
		return;
	}
	lInfo() << "Transfer state for CallSession [" << q << "] changed from [" << Utils::toString(transferState)
	        << "] to [" << Utils::toString(newState) << "]";

	transferState = newState;
	q->notifyCallSessionTransferStateChanged(newState);

	if (newState == LinphonePrivate::CallSession::State::Connected) {
		if (linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
		                            "terminate_call_upon_transfer_completion", 1)) {
			lInfo() << "Automatically terminating call " << *q << " because transfer has completed successfully.";
			q->terminate();
		}
	}
}

void CallSessionPrivate::handleIncoming(bool tryStartRingtone) {
	L_Q();

	if (tryStartRingtone) { // The state is still in IncomingReceived state. Start ringing if it was needed
		q->notifyStartRingtone();
	}

	handleIncomingReceivedStateInIncomingNotification();
}

void CallSessionPrivate::startIncomingNotification() {
	L_Q();
	bool_t tryStartRingtone =
	    TRUE; // Try to start a tone if this notification is not a PushIncomingReceived and have listener
	if (state != CallSession::State::PushIncomingReceived) {
		q->notifyIncomingCallSessionStarted(); // Can set current call to this sessions
	} else tryStartRingtone = FALSE;

	setState(CallSession::State::IncomingReceived, "Incoming call received"); // Change state and notify listeners

	// From now on, the application is aware of the call and supposed to take background task or already submitted
	// notification to the user. We can then drop our background task.
	q->notifyBackgroundTaskToBeStopped();

	if ((state == CallSession::State::IncomingReceived &&
	     linphone_core_auto_send_ringing_enabled(q->getCore()->getCCore())) ||
	    state == CallSession::State::IncomingEarlyMedia) { // If early media was accepted during setState callback above
		handleIncoming(tryStartRingtone);
	}

	if (q->mIsAccepting) {
		lInfo() << "CallSession [" << q << "] is accepted early.";
		q->notifyCallSessionAccepting();
	}
}

bool CallSessionPrivate::startPing() {
	L_Q();
	if (q->getCore()->getCCore()->sip_conf.ping_with_options) {
		/* Defer the start of the call after the OPTIONS ping for outgoing call or
		 * send an option request back to the caller so that we get a chance to discover our nat'd address
		 * before answering for incoming call */
		pingReplied = false;
		pingOp = new SalOp(q->getCore()->getCCore()->sal.get());
		if (direction == LinphoneCallIncoming) {
			auto *from = sal_address_clone(pingOp->getFromAddress());
			auto *to = sal_address_clone(pingOp->getToAddress());
			linphone_configure_op(q->getCore()->getCCore(), pingOp, log->getFromAddress()->toC(), nullptr, false);
			pingOp->setRouteAddress(op->getNetworkOriginAddress());
			pingOp->ping(from, to);
			sal_address_unref(from);
			sal_address_unref(to);
		} else if (direction == LinphoneCallOutgoing) {
			pingOp->ping(log->getFromAddress()->getImpl(), log->getToAddress()->getImpl());
		}
		pingOp->setUserPointer(this);
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------------

void CallSessionPrivate::setParams(CallSessionParams *csp) {
	if (csp) csp->assertNoReuse();
	if (params) delete params;
	params = csp;
}

void CallSessionPrivate::createOp() {
	createOpTo(log->getToAddress());
}

bool CallSessionPrivate::isInConference() const {
	return params->getPrivate()->getInConference();
}

const std::string CallSessionPrivate::getConferenceId() const {
	return params->getPrivate()->getConferenceId();
}

void CallSessionPrivate::setConferenceId(const std::string id) {
	params->getPrivate()->setConferenceId(id);
}

// -----------------------------------------------------------------------------

void CallSessionPrivate::abort(const string &errorMsg) {
	L_Q();
	op->terminate();
	lWarning() << "Session [" << q << "] is being aborted with message " << errorMsg;
	setState(CallSession::State::Error, errorMsg);
}

void CallSessionPrivate::accepted() {
	/* Immediately notify the connected state, even if errors occur after */
	switch (state) {
		case CallSession::State::OutgoingProgress:
		case CallSession::State::OutgoingRinging:
		case CallSession::State::OutgoingEarlyMedia:
			/* Immediately notify the connected state */
			setState(CallSession::State::Connected, "Connected");
			break;
		default:
			break;
	}
	currentParams->setPrivacy((LinphonePrivacyMask)op->getPrivacy());
}

void CallSessionPrivate::ackBeingSent(LinphoneHeaders *headers) {
	L_Q();
	q->notifyAckBeingSent(headers);
}

void CallSessionPrivate::ackReceived(LinphoneHeaders *headers) {
	L_Q();
	q->notifyAckReceived(headers);
}

void CallSessionPrivate::cancelDone() {
	if (reinviteOnCancelResponseRequested) {
		reinviteOnCancelResponseRequested = false;
		if (op->hasDialog()) {
			reinviteToRecoverFromConnectionLoss();
		} else {
			repairByNewInvite(false);
		}
	}
}

bool CallSessionPrivate::failure() {
	L_Q();
	const SalErrorInfo *ei = op->getErrorInfo();
	switch (ei->reason) {
		case SalReasonRedirect:
			if ((state == CallSession::State::OutgoingInit) || (state == CallSession::State::OutgoingProgress) ||
			    (state == CallSession::State::OutgoingRinging) /* Push notification case */ ||
			    (state == CallSession::State::OutgoingEarlyMedia)) {
				const SalAddress *redirectionTo = op->getRemoteContactAddress();
				if (redirectionTo) {
					std::shared_ptr<Address> redirectAddress = Address::create();
					redirectAddress->setImpl(redirectionTo);
					lWarning() << "Redirecting CallSession [" << q << "] to " << redirectAddress->toString();
					log->setToAddress(redirectAddress);
					restartInvite();
					return true;
				}
			}
			break;
		default:
			break;
	}

	/* Some call errors are not fatal */
	switch (state) {
		case CallSession::State::Updating:
		case CallSession::State::Pausing:
		case CallSession::State::Resuming:
		case CallSession::State::StreamsRunning:
			if (ei->reason == SalReasonRequestPending) {
				/* there will be a retry. Keep this state. */
				if (op->hasRetryFunction()) {
					lInfo() << "Call error on state [" << Utils::toString(state)
					        << "], keeping this state until scheduled retry.";
				} else {
					lInfo() << "Call error on state [" << Utils::toString(state)
					        << "], no retry function has been found therefore bringing call to last known stable state "
					        << Utils::toString(lastStableState);
					setState(lastStableState, "Restore stable state because no retry function has been set");
				}
				return true;
			}
			if (ei->reason != SalReasonNoMatch) {
				lInfo() << "Call error on state [" << Utils::toString(state) << "], restoring previous state ["
				        << Utils::toString(prevState) << "]";
				setState(prevState, ei->full_string);
				return true;
			}
		default:
			break;
	}

	if ((state != CallSession::State::End) && (state != CallSession::State::Error)) {
		if (ei->reason == SalReasonDeclined) setState(CallSession::State::End, "Call declined");
		else {
			std::string errorString(ei->full_string ? ei->full_string : "");
			if (CallSession::isEarlyState(state)) {
				lInfo() << "Call error on state [" << Utils::toString(state) << "] has failed with error "
				        << (errorString.empty() ? std::string("Unknown error") : errorString);
				setState(CallSession::State::Error, errorString);
			} else setState(CallSession::State::End, errorString);
		}
	}
	if (referer) {
		// Notify referer of the failure
		notifyReferState();
	}
	return false;
}

void CallSessionPrivate::infoReceived(SalBodyHandler *bodyHandler) {
	L_Q();
	LinphoneInfoMessage *info = linphone_core_create_info_message(q->getCore()->getCCore());
	linphone_info_message_set_headers(info, op->getRecvCustomHeaders());
	if (bodyHandler) {
		LinphoneContent *content = linphone_content_from_sal_body_handler(bodyHandler);
		linphone_info_message_set_content(info, content);
		linphone_content_unref(content);
	}
	q->notifyInfoReceived(info);
	linphone_info_message_unref(info);
}

void CallSessionPrivate::pingReply() {
	L_Q();
	if (state == CallSession::State::OutgoingInit) {
		pingReplied = true;
		if (isReadyForInvite()) q->startInvite(nullptr, "");
	}
}

void CallSessionPrivate::referred(const std::shared_ptr<Address> &referToAddr) {
	if (referToAddr) referToAddress = referToAddr;
	referPending = true;
	setState(CallSession::State::Referred, "Referred");
}

void CallSessionPrivate::updateToFromAssertedIdentity() {
	L_Q();
	LinphoneCore *lc = L_GET_C_BACK_PTR(q->getCore());
	/* We have the possibility to update the call's log remote identity based on P-Asserted-Identity */
	const char *pAssertedId = sal_custom_header_find(op->getRecvCustomHeaders(), "P-Asserted-Identity");
	/* In some situation, better to trust the network rather than the UAC */
	if (pAssertedId &&
	    linphone_config_get_int(linphone_core_get_config(lc), "sip", "call_logs_use_asserted_id_instead_of_from", 0)) {
		auto pAssertedIdAddr = Address::create(pAssertedId);
		if (pAssertedIdAddr) {
			lInfo() << "Using P-Asserted-Identity [" << pAssertedId << "] instead of to [" << op->getTo().c_str()
			        << "].";
			log->setToAddress(pAssertedIdAddr);

#ifdef HAVE_DB_STORAGE
			auto &mainDb = q->getCore()->getPrivate()->mainDb;
			if (mainDb != nullptr) mainDb->updateCallLog(log);
#endif // HAVE_DB_STORAGE
		} else {
			lWarning() << "Unsupported P-Asserted-Identity header";
		}
	}
}

void CallSessionPrivate::remoteRinging() {
	/* Set privacy */
	updateToFromAssertedIdentity();
	currentParams->setPrivacy((LinphonePrivacyMask)op->getPrivacy());
	setState(CallSession::State::OutgoingRinging, "Remote ringing");
}

void CallSessionPrivate::replaceOp(SalCallOp *newOp) {
	L_Q();
	SalCallOp *oldOp = op;
	CallSession::State oldState = state;
	op = newOp;
	op->setUserPointer(q);
	op->setLocalMediaDescription(oldOp->getLocalMediaDescription());
	// Replace the call ID in the call log
	log->setCallId(op->getCallId());
#ifdef HAVE_DB_STORAGE
	auto &mainDb = q->getCore()->getPrivate()->mainDb;
	if (mainDb != nullptr) mainDb->updateCallLog(log);
#endif // HAVE_DB_STORAGE
	switch (state) {
		case CallSession::State::IncomingEarlyMedia:
		case CallSession::State::IncomingReceived:
			op->notifyRinging((state == CallSession::State::IncomingEarlyMedia) ? true : false,
			                  linphone_core_get_tag_100rel_support_level(q->getCore()->getCCore()));
			break;
		case CallSession::State::Connected:
		case CallSession::State::StreamsRunning:
			op->accept();
			break;
		case CallSession::State::PushIncomingReceived:
			break;
		default:
			lWarning() << "CallSessionPrivate::replaceOp(): don't know what to do in state [" << Utils::toString(state)
			           << "]";
			break;
	}
	switch (oldState) {
		case CallSession::State::IncomingEarlyMedia:
		case CallSession::State::IncomingReceived:
			oldOp->setUserPointer(
			    nullptr); // In order for the call session to not get terminated by terminating this op
			// Do not terminate a forked INVITE
			lInfo() << "CallSessionPrivate::replaceOp(): terminating old session in early state.";
			if (op->getReplaces()) {
				oldOp->terminate();
			} else {
				oldOp->killDialog();
			}
			break;
		case CallSession::State::Connected:
		case CallSession::State::StreamsRunning:
			lInfo() << "CallSessionPrivate::replaceOp(): terminating old session in running state.";
			oldOp->terminate();
			oldOp->killDialog();
			break;
		default:
			break;
	}
	oldOp->release();
}

void CallSessionPrivate::terminated() {
	L_Q();
	switch (state) {
		case CallSession::State::End:
		case CallSession::State::Error:
			lWarning() << "terminated: already terminated, ignoring";
			return;
		case CallSession::State::IncomingReceived:
		case CallSession::State::IncomingEarlyMedia:
			if (!op->getReasonErrorInfo()->protocol || strcmp(op->getReasonErrorInfo()->protocol, "") == 0) {
				lWarning() << "Session [" << q << "] has not been answered by the remote party";
				linphone_error_info_set(ei, nullptr, LinphoneReasonNotAnswered, 0, "Incoming call cancelled", nullptr);
				nonOpError = true;
			}
			break;
		default:
			break;
	}
	setState(CallSession::State::End, "Call ended");
}

void CallSessionPrivate::updated(bool isUpdate) {
	L_Q();
	deferUpdate = !!linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
	                                        "defer_update_default", FALSE);
	SalErrorInfo sei;
	memset(&sei, 0, sizeof(sei));
	CallSession::State localState = state; // Member variable "state" may be changed within this function

	switch (localState) {
		case CallSession::State::PausedByRemote:
			updatedByRemote();
			break;
		/* SIP UPDATE CASE */
		case CallSession::State::OutgoingRinging:
		case CallSession::State::OutgoingEarlyMedia:
		case CallSession::State::IncomingEarlyMedia:
			if (isUpdate) {
				setState(CallSession::State::EarlyUpdatedByRemote, "EarlyUpdatedByRemote");
				acceptUpdate(nullptr, prevState, Utils::toString(prevState));
			}
			break;
		case CallSession::State::StreamsRunning:
		case CallSession::State::Connected:
		case CallSession::State::UpdatedByRemote: /* Can happen on UAC connectivity loss */
			updatedByRemote();
			break;
		case CallSession::State::Paused:
			/* We'll remain in pause state but accept the offer anyway according to default parameters */
			setState(CallSession::State::UpdatedByRemote, "Call updated by remote (while in Paused)");
			acceptUpdate(nullptr, CallSession::State::Paused, "Paused");
			break;
		case CallSession::State::Pausing:
		case CallSession::State::Updating:
		case CallSession::State::Resuming:
			/* Notify UpdatedByRemote state, then return to the original state, so that retryable transaction can
			 * complete.*/
			setState(CallSession::State::UpdatedByRemote,
			         "Call updated by remote while in transcient state (Pausing/Updating/Resuming)");
			acceptUpdate(nullptr, localState, Utils::toString(localState));
			break;
		case CallSession::State::End:
		case CallSession::State::Released:
			lWarning() << "Session [" << q
			           << "] is going to reject the reINVITE or UPDATE because it is already in state ["
			           << Utils::toString(state) << "]";
			sal_error_info_set(&sei, SalReasonNoMatch, "SIP", 0, "Incompatible SDP", nullptr);
			op->replyWithErrorInfo(&sei, nullptr);
			sal_error_info_reset(&sei);
			break;
		case CallSession::State::Idle:
		case CallSession::State::OutgoingInit:
		case CallSession::State::IncomingReceived:
		case CallSession::State::PushIncomingReceived:
		case CallSession::State::OutgoingProgress:
		case CallSession::State::Referred:
		case CallSession::State::Error:
		case CallSession::State::EarlyUpdatedByRemote:
		case CallSession::State::EarlyUpdating:
			lWarning() << "Receiving reINVITE or UPDATE while in state [" << Utils::toString(state)
			           << "], should not happen";
			break;
	}
}

void CallSessionPrivate::refreshed() {
	/* Briefly notifies the application that we received an UPDATE thanks to UpdatedByRemote state .*/
	setState(CallSession::State::UpdatedByRemote, "Session refresh");
	/* And immediately get back to previous state, since the actual call state doesn't change.*/
	restorePreviousState();
}

void CallSessionPrivate::updatedByRemote() {
	L_Q();

	setState(CallSession::State::UpdatedByRemote, "Call updated by remote");
	if (deferUpdate || deferUpdateInternal) {
		if (state == CallSession::State::UpdatedByRemote && !deferUpdateInternal) {
			lInfo() << "CallSession [" << q
			        << "]: UpdatedByRemoted was signaled but defered. LinphoneCore expects the application to call "
			           "linphone_call_accept_update() later";
		}
	} else {
		if (state == CallSession::State::UpdatedByRemote) q->acceptUpdate(nullptr);
		else {
			// Otherwise it means that the app responded by CallSession::acceptUpdate() within the callback,
			// so job is already done
		}
	}
}

void CallSessionPrivate::updating(bool isUpdate) {
	updated(isUpdate);
}

// -----------------------------------------------------------------------------

void CallSessionPrivate::init() {
	currentParams = new CallSessionParams();
	ei = linphone_error_info_new();
}

// -----------------------------------------------------------------------------

void CallSessionPrivate::accept(const CallSessionParams *csp) {
	L_Q();
	/* Try to be best-effort in giving real local or routable contact address */
	setContactOp();
	if (csp) setParams(new CallSessionParams(*csp));
	if (params) {
		op->enableCapabilityNegotiation(q->isCapabilityNegotiationEnabled());
		op->setSentCustomHeaders(params->getPrivate()->getCustomHeaders());
	}

	op->accept();
	setState(CallSession::State::Connected, "Connected");
}

void CallSessionPrivate::acceptOrTerminateReplacedSessionInIncomingNotification() {
	L_Q();
	CallSession *replacedSession = nullptr;
	if (linphone_config_get_int(linphone_core_get_config(q->getCore()->getCCore()), "sip",
	                            "auto_answer_replacing_calls", 1)) {
		if (op->getReplaces()) replacedSession = static_cast<CallSession *>(op->getReplaces()->getUserPointer());
		if (replacedSession) {
			switch (replacedSession->getState()) {
				/* If the replaced call is already accepted, then accept automatic replacement. */
				case CallSession::State::StreamsRunning:
				case CallSession::State::Connected:
				case CallSession::State::Paused:
				case CallSession::State::PausedByRemote:
				case CallSession::State::Pausing:
					lInfo() << " auto_answer_replacing_calls is true, replacing call is going to be accepted and "
					           "replaced call terminated.";
					q->acceptDefault();
					break;
				default:
					break;
			}
		}
	}
}

LinphoneStatus CallSessionPrivate::acceptUpdate(BCTBX_UNUSED(const CallSessionParams *csp),
                                                CallSession::State nextState,
                                                const string &stateInfo) {
	return startAcceptUpdate(nextState, stateInfo);
}

LinphoneStatus CallSessionPrivate::checkForAcceptation() {
	L_Q();
	switch (state) {
		case CallSession::State::IncomingReceived:
		case CallSession::State::IncomingEarlyMedia:
		case CallSession::State::PushIncomingReceived:
			break;
		default:
			lError() << "checkForAcceptation() CallSession [" << q << "] is in state [" << Utils::toString(state)
			         << "], operation not permitted";
			return -1;
	}
	q->notifyCheckForAcceptation();

	/* Check if this call is supposed to replace an already running one */
	SalOp *replaced = op->getReplaces();
	if (replaced) {
		CallSession *session = static_cast<CallSession *>(replaced->getUserPointer());
		if (session) {
			lInfo() << "CallSession " << q << " replaces CallSession " << session
			        << ". This last one is going to be terminated automatically";
			session->terminate();
		}
	}
	return 0;
}

void CallSessionPrivate::handleIncomingReceivedStateInIncomingNotification() {
	L_Q();
	/* Try to be best-effort in giving real local or routable contact address for 100Rel case */
	setContactOp();
	if (notifyRinging && state != CallSession::State::IncomingEarlyMedia)
		op->notifyRinging(false, linphone_core_get_tag_100rel_support_level(q->getCore()->getCCore()));
	acceptOrTerminateReplacedSessionInIncomingNotification();
}

bool CallSessionPrivate::isReadyForInvite() const {
	bool pingReady = false;
	if (pingOp) {
		if (pingReplied) pingReady = true;
	} else pingReady = true;
	return pingReady;
}

bool CallSessionPrivate::isUpdateAllowed(CallSession::State &nextState) const {
	L_Q();
	if (op->hasRetryFunction()) {
		lWarning() << "Unable to send reINVITE or UPDATE request right now because " << *q << " (local address "
		           << *q->getLocalAddress() << " remote address "
		           << (q->getRemoteAddress() ? q->getRemoteAddress()->toString() : "sip:")
		           << ") needs to execute the request which was replied by a 491 Request Pending first.";
		return false;
	}

	switch (state) {
		case CallSession::State::IncomingReceived:
		case CallSession::State::PushIncomingReceived:
		case CallSession::State::IncomingEarlyMedia:
		case CallSession::State::OutgoingRinging:
		case CallSession::State::OutgoingEarlyMedia:
			nextState = CallSession::State::EarlyUpdating;
			break;
		case CallSession::State::Connected:
		case CallSession::State::StreamsRunning:
		case CallSession::State::PausedByRemote:
		case CallSession::State::UpdatedByRemote:
			nextState = CallSession::State::Updating;
			break;
		case CallSession::State::Paused:
			nextState = CallSession::State::Pausing;
			break;
		case CallSession::State::OutgoingProgress:
		case CallSession::State::Pausing:
		case CallSession::State::Resuming:
		case CallSession::State::Updating:
		case CallSession::State::EarlyUpdating:
			nextState = state;
			break;
		default:
			lError() << *q << " (local address " << *q->getLocalAddress() << " remote address "
			         << (q->getRemoteAddress() ? q->getRemoteAddress()->toString() : "sip:")
			         << "): Update is not allowed in [" << Utils::toString(state) << "] state";
			return false;
	}
	return true;
}

int CallSessionPrivate::restartInvite() {
	L_Q();
	createOp();
	return q->initiateOutgoing(subject);
}

/*
 * Called internally when reaching the Released state, to perform cleanups to break circular references.
 **/
void CallSessionPrivate::setReleased() {
	L_Q();
	if (op) {
		/* Transfer the last error so that it can be obtained even in Released state */
		if (!nonOpError) linphone_error_info_from_sal_op(ei, op);
		/* So that we cannot have anymore upcalls for SAL concerning this call */
		op->release();
		op = nullptr;
	}
	referer = nullptr;
	transferTarget = nullptr;
	while (pendingActions.empty() == false) {
		pendingActions.pop();
	}

	q->notifyCallSessionSetReleased();
}

/* This method is called internally to get rid of a call that was notified to the application,
 * because it reached the end or error state. It performs the following tasks:
 * - remove the call from the internal list of calls
 * - update the call logs accordingly
 */
void CallSessionPrivate::setTerminated() {
	L_Q();
	completeLog();
	q->notifyCallSessionSetTerminated();
}

LinphoneStatus CallSessionPrivate::startAcceptUpdate(CallSession::State nextState, const string &stateInfo) {
	op->accept();
	setState(nextState, stateInfo);
	return 0;
}

LinphoneStatus CallSessionPrivate::startUpdate(const CallSession::UpdateMethod method, const string &subject) {
	L_Q();
	string newSubject(subject);

	if (newSubject.empty()) {
		const auto conference = q->getCore()->findConference(q->getSharedFromThis(), false);
		if (!conference) {
			if (isInConference())
				newSubject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::Conference);
			else if (q->getParams()->getPrivate()->getInternalCallUpdate())
				newSubject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::InternalUpdate);
			else if (q->getParams()->getPrivate()->getNoUserConsent())
				newSubject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::Refresh);
			else newSubject = CallSession::predefinedSubject.at(CallSession::PredefinedSubjectType::MediaChange);
		}
	}

	/* Give a chance to update the contact address if connectivity has changed */
	refreshContactAddress();
	// Update custom headers
	op->setSentCustomHeaders(params->getPrivate()->getCustomHeaders());
	q->updateContactAddressInOp();

	bool noUserConsent = q->getParams()->getPrivate()->getNoUserConsent();
	if (method != CallSession::UpdateMethod::Default) {
		noUserConsent = method == CallSession::UpdateMethod::Update;
	}

	return op->update(newSubject, noUserConsent);
}

void CallSessionPrivate::terminate() {
	switch (state) {
		case CallSession::State::IncomingReceived:
		case CallSession::State::IncomingEarlyMedia: {
			LinphoneReason reason = linphone_error_info_get_reason(ei);
			if (reason == LinphoneReasonNone) {
				linphone_error_info_set_reason(ei, LinphoneReasonDeclined);
				nonOpError = true;
			} else if (reason != LinphoneReasonNotAnswered) {
				nonOpError = true;
			}
		} break;
		default:
			break;
	}

	setState(CallSession::State::End, "Call terminated");

	// The op pointer might be NULL if the core is still gathering the ICE candidates by the time the call session is
	// terminated
	if (!op || (op && !op->hasDialog())) {
		setState(CallSession::State::Released, "Call released");
	}
}

void CallSessionPrivate::updateCurrentParams() const {
}

void CallSessionPrivate::setDestAccount(const shared_ptr<Account> &destAccount) {
	account = destAccount;
	if (params) {
		params->setAccount(account);
	}
	if (currentParams) {
		currentParams->setAccount(account);
	}
}

// -----------------------------------------------------------------------------

void CallSessionPrivate::setBroken() {
	switch (state) {
		// For all the early states, we prefer to drop the call
		case CallSession::State::OutgoingInit:
		case CallSession::State::OutgoingProgress:
		case CallSession::State::OutgoingRinging:
		case CallSession::State::OutgoingEarlyMedia:
		case CallSession::State::IncomingReceived:
		case CallSession::State::IncomingEarlyMedia:
			// During the early states, the SAL layer reports the failure from the dialog or transaction layer,
			// hence, there is nothing special to do
		case CallSession::State::StreamsRunning:
		case CallSession::State::Updating:
		case CallSession::State::Pausing:
		case CallSession::State::Resuming:
		case CallSession::State::Paused:
		case CallSession::State::PausedByRemote:
		case CallSession::State::UpdatedByRemote:
			// During these states, the dialog is established. A failure of a transaction is not expected to close it.
			// Instead we have to repair the dialog by sending a reINVITE
			broken = true;
			needLocalIpRefresh = true;
			break;
		default:
			lError() << "CallSessionPrivate::setBroken(): unimplemented case";
			break;
	}
}

void CallSessionPrivate::setContactOp() {
	L_Q();
	auto contactAddress = getFixedContact();
	// Do not try to set contact address if it is not valid
	if (contactAddress && contactAddress->isValid()) {
		q->updateContactAddress(*contactAddress);
		if (isInConference()) {
			// Make the best effort to find the conference linked to the call session on the server side.
			// Try with both the To or From headers and the guessed contact address
			auto core = q->getCore();
			const auto conferenceIdParams = core->createConferenceIdParams();
			std::shared_ptr<Conference> conference =
			    core->findConference(ConferenceId(contactAddress, contactAddress, conferenceIdParams), false);
			// Find server conference based on From or To header as the tchat conference server may give an address to
			// the conference that doesn't match the identity address or contact address of any of the accounts held by
			// the core. For example, flexisip tchat servers based on SDK 5.3, will create chatroom with username
			// chatroom-XXXXX which doesn't match any of the account held by the core so the To or From header have the
			// chatroom address
			auto guessedConferenceAddress =
			    Address::create((direction == LinphoneCallIncoming) ? op->getTo() : op->getFrom());
			std::shared_ptr<Conference> guessedConference = core->findConference(
			    ConferenceId(guessedConferenceAddress, guessedConferenceAddress, conferenceIdParams), false);
			std::shared_ptr<Address> conferenceAddress;
			std::shared_ptr<Conference> conferenceFound;
			if (conference) {
				// The URI returned by getFixedAddress matches a conference
				conferenceAddress = conference->getConferenceAddress()->clone()->toSharedPtr();
				conferenceFound = conference;
			} else if (guessedConference) {
				// The conference is actually a chatroom in which its conference ID doesn't match the guessed account.
				// For example, if the chatroom peer address is sip:chatroom-xyz@sip.example.org and the account is
				// sip:focus@sip.example.org
				conferenceAddress = guessedConference->getConferenceAddress()->clone()->toSharedPtr();
				conferenceFound = guessedConference;
				lInfo() << "The guessed contact address " << *contactAddress
				        << " doesn't match the actual chatroom conference address " << *conferenceAddress;
			} else {
#ifdef HAVE_DB_STORAGE
				auto &mainDb = q->getCore()->getPrivate()->mainDb;
				const auto &confInfo = mainDb->getConferenceInfoFromURI(guessedConferenceAddress);
				if (confInfo) {
					// The conference may have already been terminated when setting the contact address.
					// This happens when an admin cancel a conference by sending an INVITE with an empty resource
					// list Add parameters stored in the conference information URI to the contact address
					conferenceAddress = confInfo->getUri()->clone()->toSharedPtr();
				}
#endif // HAVE_DB_STORAGE
			}
			if (conferenceAddress && conferenceAddress->isValid()) {
				if (contactAddress && contactAddress->isValid()) {
					// Copy all parameters of the guessed contact address into the conference address. Here, it is
					// interesting to pass the GRUU parameter on
					lInfo() << "Copying all parameters of the guessed contact address " << *contactAddress
					        << " to found conference address " << *conferenceAddress;
					conferenceAddress->merge(*contactAddress);
				}
				contactAddress = conferenceAddress;
			}
		}

		lInfo() << "Setting contact address for " << *q << " to " << *contactAddress;
		op->setContactAddress(contactAddress->getImpl());
	} else {
		lWarning() << "Unable to set contact address for " << *q << " to "
		           << ((contactAddress) ? contactAddress->toString() : std::string("sip:")) << " as it is not valid";
	}
}

// -----------------------------------------------------------------------------
void CallSessionPrivate::onNetworkReachable(bool sipNetworkReachable, BCTBX_UNUSED(bool mediaNetworkReachable)) {
	if (sipNetworkReachable) {
		repairIfBroken();
	} else {
		setBroken();
	}
}

void CallSessionPrivate::onRegistrationStateChanged(LinphoneProxyConfig *cfg,
                                                    LinphoneRegistrationState cstate,
                                                    BCTBX_UNUSED(const std::string &message)) {
	// might be better to add callbacks on Account, but due to the lake of internal listener, it is dangerous to expose
	// internal listeners to public object.
	const auto registeredChangedAccount = cfg ? Account::toCpp(cfg->account)->getSharedFromThis() : nullptr;
	if (registeredChangedAccount == getDestAccount() && cstate == LinphoneRegistrationOk) repairIfBroken();
	/*else
	    only repair call when the right account is in state connected*/
}

// -----------------------------------------------------------------------------
bool CallSessionPrivate::reportEvents() const {
	L_Q();
	const auto &contactAddress = q->getContactAddress();
	const auto localAddress = q->getLocalAddress();
	const auto serverConferenceGuessedAddress =
	    (contactAddress && contactAddress->hasUriParam("conf-id")) ? contactAddress : localAddress;
	const auto &remoteContactAddress = q->getRemoteContactAddress();
	const auto remoteAddress = q->getRemoteAddress();
	const auto clientConferenceGuessedAddress =
	    (remoteContactAddress && remoteContactAddress->hasUriParam("conf-id")) ? remoteContactAddress : remoteAddress;
	const auto &peerAddress = isInConference() ? serverConferenceGuessedAddress : clientConferenceGuessedAddress;
	const auto conference = q->getCore()->searchConference(nullptr, nullptr, peerAddress, {});
	ConferenceInterface::State conferenceState =
	    (conference) ? conference->getState() : ConferenceInterface::State::Instantiated;
	bool conferenceCreated = !((conferenceState == ConferenceInterface::State::Instantiated) ||
	                           (conferenceState == ConferenceInterface::State::CreationPending));
	// Add logs to the databse if it is a regular call.
	// The core should store call session linked to the creation or update of chat room and conferences
	return !q->getParams()->getPrivate()->isConferenceCreation() &&
	       (!conference || (conferenceCreated && conference->supportsMedia()));
}

void CallSessionPrivate::completeLog() {
	L_Q();
	int duration = log->getConnectedTime() == 0 ? 0 : computeDuration(); /* Store duration since connected */
	log->setDuration(duration);
	log->setErrorInfo(ei);
	if (log->getStatus() == LinphoneCallMissed) {
		auto account = getDestAccount();
		if (account) {
			account->setMissedCallsCount(account->getMissedCallsCount() + 1);
		} else {
			q->getCore()->getCCore()->missed_calls++;
		}
	}
	if (reportEvents()) {
		q->getCore()->reportConferenceCallEvent(EventLog::Type::ConferenceCallEnded, log, nullptr);
	}
}

void CallSessionPrivate::createOpTo(const std::shared_ptr<Address> &to) {
	L_Q();
	if (op) op->release();

	const auto &core = q->getCore()->getCCore();

	op = new SalCallOp(core->sal.get(), q->isCapabilityNegotiationEnabled());
	op->setUserPointer(q);
	if (params->getPrivate()->getReferer()) op->setReferrer(params->getPrivate()->getReferer()->getPrivate()->getOp());
	linphone_configure_op_with_account(core, op, to->toC(), q->getParams()->getPrivate()->getCustomHeaders(), false,
	                                   toC(getDestAccount()));
	if (q->getParams()->getPrivacy() != LinphonePrivacyDefault)
		op->setPrivacy((SalPrivacyMask)q->getParams()->getPrivacy());
	/* else privacy might be set by account */
}

// -----------------------------------------------------------------------------

std::shared_ptr<Address> CallSessionPrivate::getFixedContact() const {
	L_Q();
	std::shared_ptr<Address> result = nullptr;
	const auto &account = getDestAccount();
	if (op && op->getContactAddress()) {
		/* If already choosed, don't change it */
		result = Address::create();
		result->setImpl(op->getContactAddress());
		return result;
	} else if (pingOp && pingOp->getContactAddress()) {
		/* If the ping OPTIONS request succeeded use the contact guessed from the received, rport */
		lInfo() << "Contact has been fixed using OPTIONS";
		result = Address::create();
		result->setImpl(pingOp->getContactAddress());
		return result;
	} else if (account) {
		shared_ptr<Address> addr = nullptr;
		const auto &accountContactAddress = account->getContactAddress();
		if (accountContactAddress) {
			addr = accountContactAddress;
		} else {
			lError() << "Unable to retrieve contact address from account for call session " << q << " (local address "
			         << *q->getLocalAddress() << " remote address "
			         << (q->getRemoteAddress() ? q->getRemoteAddress()->toString() : "sip:") << ").";
		}
		if (addr && (account->getOp() || (account->getDependency() != nullptr) ||
		             linphone_core_conference_server_enabled(q->getCore()->getCCore()))) {
			/* If using a account, use the contact address as guessed with the REGISTERs */
			lInfo() << "Contact " << *addr << " has been fixed using account " << *account;
			result = addr->clone()->toSharedPtr();
			return result;
		}
	}
	result = Address::toCpp(linphone_core_get_primary_contact_parsed(q->getCore()->getCCore()))->toSharedPtr();
	if (result) {
		/* Otherwise use supplied localip */
		result->setDomain(std::string() /* localip */);
		result->setPort(-1 /* linphone_core_get_sip_port(core) */);
		lInfo() << "Contact has not been fixed, stack will do";
	}
	return result;
}

// -----------------------------------------------------------------------------

void CallSessionPrivate::reinviteToRecoverFromConnectionLoss() {
	L_Q();
	lInfo() << *q << " is going to be updated (reINVITE) in order to recover from lost connectivity";
	if (op) {
		// Reset retry function as we need to recover from a network loss
		op->resetRetryFunction();
	}
	q->update(params, CallSession::UpdateMethod::Invite);
}

void CallSessionPrivate::repairByNewInvite(bool withReplaces) {
	L_Q();
	lInfo() << *q << " is going to have a new INVITE one in order to recover from lost connectivity; "
	        << (withReplaces ? "with" : "without") << " Replaces header";

	// Reset retry function as we need to repair the INVITE session
	op->resetRetryFunction();

	// FIXME: startInvite shall() accept a list of bodies.
	// Since it is not the case, we can only re-use the first one.
	const list<Content> &contents = op->getLocalBodies();
	shared_ptr<Content> contentToRestore;

	if (!contents.empty()) {
		contentToRestore = (new Content(contents.front()))->toSharedPtr();
	}

	string callId = op->getCallId();
	string fromTag = op->getLocalTag();
	string toTag = op->getRemoteTag();

	op->killDialog();
	createOp();
	if (withReplaces) {
		op->setReplaces(callId.c_str(), fromTag, toTag.empty() ? "0" : toTag);
	} // empty tag is set to 0 as defined by rfc3891
	// We have to re-create the local media description because the media local IP may have change, so go throug
	// initiateOutgoing();
	bool defer = q->initiateOutgoing(subject, nullptr);
	if (!defer) q->startInvite(nullptr, subject, contentToRestore);
}

void CallSessionPrivate::refreshContactAddress() {
	L_Q();
	if (!op) return;
	Address contactAddress;
	const auto &account = getDestAccount();
	if (account) {
		const auto &accountOp = account->getOp();
		const auto &accountContactAddress = account->getContactAddress();
		if (accountOp) {
			/* Give a chance to update the contact address if connectivity has changed */
			contactAddress.setImpl(accountOp->getContactAddress());
		} else if (linphone_core_conference_server_enabled(q->getCore()->getCCore()) && accountContactAddress) {
			contactAddress = Address(*accountContactAddress);
		}

		if (contactAddress.isValid()) {
			q->updateContactAddress(contactAddress);
		}
	}

	if (contactAddress.isValid()) {
		op->setContactAddress(contactAddress.getImpl());
	} else {
		op->setContactAddress(nullptr);
	}
}

void CallSessionPrivate::repairIfBroken() {
	L_Q();

	try {
		LinphoneCore *lc = q->getCore()->getCCore();
		if (!lc->media_network_state.global_state || !broken ||
		    !linphone_config_get_int(linphone_core_get_config(lc), "sip", "repair_broken_calls", 1))
			return;
	} catch (const bad_weak_ptr &) {
		return; // Cannot repair if core is destroyed.
	}

	// If we are registered and this session has been broken due to a past network disconnection,
	// attempt to repair it
	// Make sure that the account from which we received this call, or to which we routed this call is registered first
	const auto &account = getDestAccount();
	if (account) {
		// In all other cases, ie no account, or a account for which no registration was requested,
		// we can start the call session repair immediately.
		const auto accountParams = account->getAccountParams();
		if (accountParams->getRegisterEnabled() && (account->getState() != LinphoneRegistrationOk)) return;
	}

	/* Give a chance to update the contact address if connectivity has changed */
	refreshContactAddress();

	SalErrorInfo sei;
	memset(&sei, 0, sizeof(sei));
	switch (state) {
		case CallSession::State::Updating:
		case CallSession::State::Pausing:
			if (op->dialogRequestPending()) {
				// Need to cancel first re-INVITE as described in section 5.5 of RFC 6141
				if (op->cancelInvite() == 0) {
					reinviteOnCancelResponseRequested = true;
					broken = false;
				}
			}
			break;
		case CallSession::State::StreamsRunning:
		case CallSession::State::Paused:
		case CallSession::State::PausedByRemote:
			if (!op->dialogRequestPending()) {
				reinviteToRecoverFromConnectionLoss();
				broken = false;
			}
			break;
		case CallSession::State::UpdatedByRemote:
			if (op->dialogRequestPending()) {
				sal_error_info_set(&sei, SalReasonServiceUnavailable, "SIP", 0, nullptr, nullptr);
				op->replyWithErrorInfo(&sei, nullptr);
			}
			reinviteToRecoverFromConnectionLoss();
			broken = false;
			break;
		case CallSession::State::OutgoingInit:
		case CallSession::State::OutgoingProgress:
			/* Don't attempt to send new INVITE with Replaces header: we don't know if the callee received
			 * the INVITE. Instead, cancel the call and make a new one.
			 */
			if (op->cancelInvite() == 0) {
				reinviteOnCancelResponseRequested = true;
				broken = false;
			} else {
				lError() << "Do something here.";
			}
			break;
		case CallSession::State::OutgoingEarlyMedia:
		case CallSession::State::OutgoingRinging:
			if (op->getRemoteTag() != nullptr && strlen(op->getRemoteTag()) > 0) {
				repairByNewInvite(true);
				broken = false;
			} else {
				lWarning() << "No remote tag in last provisional response, no early dialog, so trying to cancel lost "
				              "INVITE and will retry later.";
				if (op->cancelInvite() == 0) {
					reinviteOnCancelResponseRequested = true;
					broken = false;
				}
			}
			break;
		case CallSession::State::IncomingEarlyMedia:
		case CallSession::State::IncomingReceived:
		case CallSession::State::PushIncomingReceived:
			// Keep the call broken until a forked INVITE is received from the server
			break;
		default:
			lWarning() << "CallSessionPrivate::repairIfBroken: don't know what to do in state ["
			           << Utils::toString(state);
			broken = false;
			break;
	}
	sal_error_info_reset(&sei);
}

// =============================================================================

CallSession::CallSession(const shared_ptr<Core> &core, const CallSessionParams *params)
    : Object(*new CallSessionPrivate), CoreAccessor(core) {
	L_D();
	getCore()->getPrivate()->registerListener(d);
	if (params) d->setParams(new CallSessionParams(*params));
	d->init();
	lInfo() << "New CallSession [" << this << "] initialized (LinphoneCore version: " << linphone_core_get_version()
	        << ")";
}

CallSession::CallSession(CallSessionPrivate &p, const shared_ptr<Core> &core) : Object(p), CoreAccessor(core) {
	L_D();
	getCore()->getPrivate()->registerListener(d);
	d->init();
}

CallSession::~CallSession() {
	L_D();
	try { // getCore may no longuer be available when deleting, specially in case of managed enviroment like java
		getCore()->getPrivate()->unregisterListener(d);
	} catch (const bad_weak_ptr &) {
	}
	if (d->currentParams) delete d->currentParams;
	if (d->params) delete d->params;
	if (d->remoteParams) delete d->remoteParams;
	if (d->ei) linphone_error_info_unref(d->ei);
	if (d->op) d->op->release();
}

// -----------------------------------------------------------------------------

void CallSession::addListener(std::shared_ptr<CallSessionListener> listener) {
	L_D();
	if (listener) {
		// Avoid adding twice the same listener
		// This should never happen but just in case....
		const auto it = std::find_if(d->listeners.cbegin(), d->listeners.cend(),
		                             [&listener](const auto &l) { return l.lock() == listener; });
		if (it == d->listeners.cend()) {
			d->listeners.push_back(listener);
		}
	}
}

void CallSession::removeListener(const std::shared_ptr<CallSessionListener> &listener) {
	L_D();
	if (listener) {
		const auto it = std::find_if(d->listeners.cbegin(), d->listeners.cend(),
		                             [&listener](const auto &l) { return l.lock() == listener; });
		if (it != d->listeners.cend()) {
			d->listeners.erase(it);
		}
	}
}

void CallSession::clearListeners() {
	L_D();
	d->listeners.clear();
}

void CallSession::setStateToEnded() {
	L_D();
	d->setState(CallSession::State::End, "Call ended");
}

void CallSession::acceptDefault() {
	accept();
}

LinphoneStatus CallSession::accept(const CallSessionParams *csp) {
	L_D();
	LinphoneStatus result = d->checkForAcceptation();
	if (result < 0) return result;
	d->accept(csp);
	return 0;
}

void CallSession::accepting() {
	mIsAccepting = true;
}

LinphoneStatus CallSession::acceptUpdate(const CallSessionParams *csp) {
	L_D();
	if (d->state != CallSession::State::UpdatedByRemote) {
		lError() << "CallSession::acceptUpdate(): invalid state " << Utils::toString(d->state)
		         << " to call this method";
		return -1;
	}
	return d->acceptUpdate(csp, d->prevState, Utils::toString(d->prevState));
}

void CallSession::configure(LinphoneCallDir direction,
                            const std::shared_ptr<Account> &account,
                            SalCallOp *op,
                            const std::shared_ptr<const Address> &from,
                            const std::shared_ptr<const Address> &to) {
	L_D();
	d->direction = direction;
	d->log = CallLog::create(getCore(), direction, from, to);

	const auto &core = getCore()->getCCore();
	if (op) {
		/* We already have an op for incoming calls */
		d->op = op;
		d->op->setUserPointer(this);
		op->enableCapabilityNegotiation(isCapabilityNegotiationEnabled());
		op->enableCnxIpTo0000IfSendOnly(!!linphone_config_get_default_int(linphone_core_get_config(core), "sip",
		                                                                  "cnx_ip_to_0000_if_sendonly_enabled", 0));
		d->log->setCallId(op->getCallId()); /* Must be known at that time */
	}

	if (direction == LinphoneCallOutgoing) {
		if (d->params->getPrivate()->getReferer()) d->referer = d->params->getPrivate()->getReferer();
		d->startPing();
	} else if (!getParams() && (direction == LinphoneCallIncoming)) {
		d->setParams(new CallSessionParams());
		d->params->initDefault(getCore(), LinphoneCallIncoming);
	}

	assignAccount(account);
	if ((direction == LinphoneCallIncoming) && sdpFoundInRemoteBody() && d->reportEvents()) {
		getCore()->reportConferenceCallEvent(EventLog::Type::ConferenceCallStarted, d->log, nullptr);
	}
}

void CallSession::configure(LinphoneCallDir direction, const string &callid) {
	L_D();
	d->direction = direction;

	// Keeping a valid address while following https://www.ietf.org/rfc/rfc3323.txt guidelines.
	const auto anonymous = Address::create("Anonymous <sip:anonymous@anonymous.invalid>");
	d->log = CallLog::create(getCore(), direction, anonymous, anonymous);
	d->log->setCallId(callid);

	if (!getParams()) {
		d->setParams(new CallSessionParams());
		d->params->initDefault(getCore(), LinphoneCallIncoming);
	}
}

void CallSession::assignAccount(const std::shared_ptr<Account> &account) {
	L_D();
	d->setDestAccount(account);
	if (!d->getDestAccount()) {
		/* Try to define the destination account if it has not already been done to have a correct contact field in the
		 * SIP messages */
		const LinphoneAddress *toAddr = d->log->getToAddress()->toC();
		const LinphoneAddress *fromAddr = d->log->getFromAddress()->toC();
		const auto &core = getCore()->getCCore();
		const auto &direction = d->log->getDirection();
		LinphoneAccount *cAccount = nullptr;

		if (direction == LinphoneCallIncoming) {
			if (linphone_core_conference_server_enabled(core)) {
				// In the case of a server, clients may call the conference factory in order to create a conference
				cAccount = linphone_core_lookup_account_by_conference_factory_strict(core, toAddr);
			}
			if (!cAccount) {
				auto account = getCore()->findAccountByIdentityAddress(d->log->getToAddress());
				if (!account) {
					account = getCore()->guessLocalAccountFromMalformedMessage(d->log->getToAddress(),
					                                                           d->log->getFromAddress());
					if (account) {
						// We have a match for the from domain and the to username.
						// We may face an IPBPX that sets the To domain to our IP address, which is
						// a terribly stupid idea.
						lWarning() << "Applying workaround to have this call assigned to a known account.";
						// We must "hack" the call-log so it is correctly reported for this Account.
						d->log->setToAddress(account->getAccountParams()->getIdentityAddress());
					} else {
						account = getCore()->guessLocalAccountFromMalformedMessage(getRequestAddress(),
						                                                           d->log->getFromAddress());
						if (account) {
							// We failed to find matching local account using From & To headers but we did using the
							// request URI.
							lWarning() << "Applying workaround to have this call assigned to a known account using the "
							              "request URI";
							// We must "hack" the call-log so it is correctly reported for this Account.
							d->log->setToAddress(account->getAccountParams()->getIdentityAddress());
						}
					}
				}
				if (account) {
					cAccount = account->toC();
				}
			}
		} else {
			cAccount = linphone_core_lookup_account_by_identity(core, fromAddr);
		}
		std::shared_ptr<Account> account;
		if (cAccount) {
			account = Account::toCpp(cAccount)->getSharedFromThis();
			d->setDestAccount(account);
		}
	}
}

bool CallSession::isOpConfigured() {
	L_D();
	return d->op ? true : false;
}

LinphoneStatus CallSession::decline(LinphoneReason reason) {
	LinphoneErrorInfo *ei = linphone_error_info_new();
	linphone_error_info_set(ei, "SIP", reason, linphone_reason_to_error_code(reason), nullptr, nullptr);
	LinphoneStatus status = decline(ei);
	linphone_error_info_unref(ei);
	return status;
}

LinphoneStatus CallSession::decline(const LinphoneErrorInfo *ei) {
	L_D();
	if (d->state == CallSession::State::PushIncomingReceived && !d->op) {
		lInfo() << "[pushkit] Terminate CallSession [" << this << "]";
		linphone_error_info_set(d->ei, nullptr, LinphoneReasonDeclined, 3, "Declined", nullptr);
		d->terminate();
		d->setState(LinphonePrivate::CallSession::State::Released, "Call released");
		return 0;
	}

	SalErrorInfo sei;
	SalErrorInfo sub_sei;
	memset(&sei, 0, sizeof(sei));
	memset(&sub_sei, 0, sizeof(sub_sei));
	sei.sub_sei = &sub_sei;
	if ((d->state != CallSession::State::IncomingReceived) && (d->state != CallSession::State::IncomingEarlyMedia) &&
	    (d->state != CallSession::State::PushIncomingReceived)) {
		lError() << "Cannot decline a CallSession that is in state " << Utils::toString(d->state);
		return -1;
	}
	if (ei) {
		linphone_error_info_set(d->ei, nullptr, linphone_error_info_get_reason(ei),
		                        linphone_error_info_get_protocol_code(ei), linphone_error_info_get_phrase(ei), nullptr);
		linphone_error_info_to_sal(ei, &sei);
		d->op->replyWithErrorInfo(&sei, nullptr);
	} else d->op->decline(SalReasonDeclined);
	sal_error_info_reset(&sei);
	sal_error_info_reset(&sub_sei);
	d->terminate();
	return 0;
}

LinphoneStatus CallSession::declineNotAnswered(LinphoneReason reason) {
	L_D();
	d->log->setStatus(LinphoneCallMissed);
	d->nonOpError = true;
	linphone_error_info_set(d->ei, nullptr, reason, linphone_reason_to_error_code(reason), "Not answered", nullptr);
	return decline(reason);
}

LinphoneStatus CallSession::deferUpdate() {
	L_D();
	if (d->state != CallSession::State::UpdatedByRemote) {
		lError() << "CallSession::deferUpdate() not done in state CallSession::State::UpdatedByRemote";
		return -1;
	}
	d->deferUpdate = true;
	return 0;
}

const std::list<LinphoneMediaEncryption> CallSession::getSupportedEncryptions() const {
	L_D();
	if ((d->direction == LinphoneCallIncoming) && (d->state == CallSession::State::Idle)) {
		// If we are in the IncomingReceived state, we support all encryptions the core had enabled at compile time
		// In fact, the policy is to preliminary accept (i.e. send 180 Ringing) the wider possible range of offers.
		// Note that special treatment is dedicated to ZRTP as, for testing purposes, a core can have its member
		// zrtp_not_available_simulation set to TRUE which prevent the core to accept calls with ZRTP encryptions The
		// application can then decline a call based on the call parameter the call was accepted with
		const auto core = getCore()->getCCore();
		const auto encList = linphone_core_get_supported_media_encryptions_at_compile_time();
		std::list<LinphoneMediaEncryption> encEnumList;
		for (bctbx_list_t *enc = encList; enc != NULL; enc = enc->next) {
			const auto encEnum = static_cast<LinphoneMediaEncryption>(LINPHONE_PTR_TO_INT(bctbx_list_get_data(enc)));
			// Do not add ZRTP if it is not supported by core even though the core was compile with it on
			if ((encEnum != LinphoneMediaEncryptionZRTP) ||
			    ((encEnum == LinphoneMediaEncryptionZRTP) && !core->zrtp_not_available_simulation)) {
				encEnumList.push_back(encEnum);
			}
		}

		if (encList) {
			bctbx_list_free(encList);
		}

		return encEnumList;
	} else if (getParams()) {
		return getParams()->getPrivate()->getSupportedEncryptions();
	}
	return getCore()->getSupportedMediaEncryptions();
}

bool CallSession::sdpFoundInRemoteBody() const {
	L_D();
	if (!d->op) return false;
	const list<Content> &contents = d->op->getRemoteBodies();
	for (auto &content : contents) {
		if (content.getContentType() == ContentType::Sdp) {
			return true;
		}
	}
	return false;
}

bool CallSession::sdpFoundInLocalBody() const {
	L_D();
	if (!d->op) return false;
	const list<Content> &contents = d->op->getLocalBodies();
	for (auto &content : contents) {
		if (content.getContentType() == ContentType::Sdp) {
			return true;
		}
	}
	return false;
}

bool CallSession::isCapabilityNegotiationEnabled() const {
	if (getParams()) {
		return getParams()->getPrivate()->capabilityNegotiationEnabled();
	}
	return !!linphone_core_capability_negociation_enabled(getCore()->getCCore());
}

bool CallSession::hasTransferPending() {
	L_D();
	return d->referPending;
}

void CallSession::initiateIncoming() {
}

bool CallSession::initiateOutgoing(BCTBX_UNUSED(const string &subject),
                                   BCTBX_UNUSED(const std::shared_ptr<const Content> content)) {
	L_D();
	bool defer = false;
	d->setState(CallSession::State::OutgoingInit, "Starting outgoing call");
	if (!d->getDestAccount()) defer = d->startPing();
	return defer;
}

void CallSession::iterate(time_t currentRealTime, bool oneSecondElapsed) {
	L_D();
	int elapsed = (int)(currentRealTime - d->log->getStartTime());
	if ((d->state == CallSession::State::OutgoingInit) && (elapsed > getCore()->getCCore()->sip_conf.delayed_timeout) &&
	    (d->pingOp != nullptr)) {
		/* Start the call even if the OPTIONS reply did not arrive */
		startInvite(nullptr, "");
	}
	if ((d->state == CallSession::State::IncomingReceived) || (d->state == CallSession::State::IncomingEarlyMedia)) {
		notifyIncomingCallSessionTimeoutCheck(elapsed, oneSecondElapsed);
	}

	if (d->direction == LinphoneCallIncoming && !isOpConfigured()) {
		notifyPushCallSessionTimeoutCheck(elapsed);
	}

	const auto callTimeout = getCore()->getCCore()->sip_conf.in_call_timeout;
	const auto &connectedTime = d->log->getConnectedTime();
	if ((callTimeout > 0) && (connectedTime != 0) && ((currentRealTime - connectedTime) > callTimeout)) {
		lInfo() << "Terminating call session " << this << " (local address " << *getLocalAddress() << " remote address "
		        << (getRemoteAddress() ? getRemoteAddress()->toString() : "sip:") << ") because the call timeout ("
		        << callTimeout << "s) has been reached";
		terminate();
	}
}

LinphoneStatus CallSession::redirect(const string &redirectUri) {
	auto address = getCore()->interpretUrl(redirectUri, true);
	if (!address || !address->isValid()) {
		/* Bad url */
		lError() << "Bad redirect URI: " << redirectUri;
		return -1;
	}
	return redirect(*address);
}

LinphoneStatus CallSession::redirect(const Address &redirectAddr) {
	L_D();
	if (d->state != CallSession::State::IncomingReceived && d->state != CallSession::State::PushIncomingReceived) {
		lError() << "Unable to redirect call when in state " << d->state;
		return -1;
	}
	if (!redirectAddr.isValid()) {
		lError() << "Call session " << this << " (local address " << *getLocalAddress() << " remote address "
		         << (getRemoteAddress() ? getRemoteAddress()->toString() : "sip:")
		         << ") is being redirected to an invalid address - aborting the operation";
		return -1;
	}
	lInfo() << "Call session " << this << " (local address " << *getLocalAddress() << " remote address "
	        << (getRemoteAddress() ? getRemoteAddress()->toString() : "sip:") << ") is being redirected to address "
	        << redirectAddr;
	SalErrorInfo sei;
	memset(&sei, 0, sizeof(sei));
	sal_error_info_set(&sei, SalReasonRedirect, "SIP", 0, nullptr, nullptr);
	d->op->replyWithErrorInfo(
	    &sei, redirectAddr.getImpl(),
	    ((getParams()->getPrivate()->getEndTime() < 0) ? 0 : getParams()->getPrivate()->getEndTime()));
	linphone_error_info_set(d->ei, nullptr, LinphoneReasonMovedPermanently, 302, "Call redirected", nullptr);
	d->nonOpError = true;
	d->terminate();
	sal_error_info_reset(&sei);
	return 0;
}

void CallSession::startIncomingNotification(bool notifyRinging) {
	L_D();
	if (d->state != CallSession::State::PushIncomingReceived) {
		startBasicIncomingNotification(notifyRinging);
	}
	if (d->deferIncomingNotification) {
		lInfo() << "Defer incoming notification";
		return;
	}
	d->startIncomingNotification();
}

void CallSession::startBasicIncomingNotification(bool notifyRinging) {
	L_D();
	d->notifyRinging = notifyRinging;
	notifyIncomingCallSessionNotified();
	notifyBackgroundTaskToBeStarted();
}

void CallSession::startPushIncomingNotification() {
	L_D();
	notifyIncomingCallSessionStarted();
	notifyStartRingtone();

	d->setState(CallSession::State::PushIncomingReceived, "Push notification received");
}

int CallSession::startInvite(const std::shared_ptr<Address> &destination,
                             const string &subject,
                             const std::shared_ptr<const Content> content) {
	L_D();
	d->subject = subject;
	/* Try to be best-effort in giving real local or routable contact address */
	d->setContactOp();
	const SalAddress *destinationAddress;
	if (destination) destinationAddress = destination->getImpl();
	else destinationAddress = d->log->getToAddress()->getImpl();
	/* Take a ref because sal_call() may destroy the CallSession if no SIP transport is available */
	shared_ptr<CallSession> ref = getSharedFromThis();
	d->op->setLocalBodies({});
	if (content) {
		d->op->addLocalBody(*content);
	}

	// If a custom Content has been set in the call params, create a multipart body for the INVITE
	for (auto &c : d->params->getCustomContents()) {
		d->op->addLocalBody(*c);
	}

	int result = d->op->call(d->log->getFromAddress()->getImpl(), destinationAddress, subject);
	if (result < 0) {
		if ((d->state != CallSession::State::Error) && (d->state != CallSession::State::Released)) {
			// sal_call() may invoke call_failure() and call_released() SAL callbacks synchronously,
			// in which case there is no need to perform a state change here.
			d->setState(CallSession::State::Error, "Call failed");
		}
	} else {
		d->log->setCallId(d->op->getCallId()); /* Must be known at that time */
		d->setState(CallSession::State::OutgoingProgress, "Outgoing call in progress");
		if (sdpFoundInLocalBody() && d->reportEvents()) {
			getCore()->reportConferenceCallEvent(EventLog::Type::ConferenceCallStarted, d->log, nullptr);
		}
	}
	return result;
}

LinphoneStatus CallSession::terminate(const LinphoneErrorInfo *ei) {
	L_D();
	lInfo() << "Terminate CallSession [" << this << "] which is currently in state [" << Utils::toString(d->state)
	        << "]";
	SalErrorInfo sei;
	memset(&sei, 0, sizeof(sei));
	switch (d->state) {
		case CallSession::State::Released:
		case CallSession::State::End:
		case CallSession::State::Error:
			lWarning() << "No need to terminate CallSession [" << this << "] in state [" << Utils::toString(d->state)
			           << "]";
			return -1;
		case CallSession::State::IncomingReceived:
		case CallSession::State::PushIncomingReceived:
		case CallSession::State::IncomingEarlyMedia:
			return decline(ei);
		case CallSession::State::OutgoingInit:
			/* In state OutgoingInit, op has to be destroyed */
			d->op->release();
			d->op = nullptr;
			break;
		case CallSession::State::Idle:
			// Do nothing if trying to terminate call in idle state
			break;
		default:
			if (ei) {
				linphone_error_info_to_sal(ei, &sei);
				d->op->terminate(&sei);
				sal_error_info_reset(&sei);
			} else d->op->terminate();
			break;
	}

	d->terminate();
	return 0;
}

LinphoneStatus CallSession::transfer(const shared_ptr<CallSession> &dest) {
	L_D();
	int result = d->op->referWithReplaces(dest->getPrivate()->op);
	d->setTransferState(CallSession::State::OutgoingInit);
	return result;
}

LinphoneStatus CallSession::transfer(const Address &address) {
	L_D();
	if (!address.isValid()) {
		lError() << "Received invalid address " << address << " to transfer the call to";
		return -1;
	}
	d->op->refer(address.toString().c_str());
	d->setTransferState(CallSession::State::OutgoingInit);
	return 0;
}

LinphoneStatus CallSession::update(const CallSessionParams *csp,
                                   const UpdateMethod method,
                                   const string &subject,
                                   const std::shared_ptr<Content> content) {
	L_D();
	CallSession::State nextState;
	CallSession::State initialState = d->state;
	if (!d->isUpdateAllowed(nextState)) return -1;
	d->setState(nextState, "Updating call");
	if (d->currentParams == csp)
		lWarning() << "CallSession::update() is given the current params, this is probably not what you intend to do!";
	if (csp) d->setParams(new CallSessionParams(*csp));

	list<Content> contentList;
	if (content) contentList.push_back(*content);
	for (auto &c : d->params->getCustomContents()) {
		contentList.push_back(*c);
	}

	d->op->setLocalBodies(contentList);
	LinphoneStatus result = d->startUpdate(method, subject);
	if (result && (d->state != initialState)) {
		/* Restore initial state */
		d->setState(initialState, "Restore initial state");
	}
	return result;
}

// -----------------------------------------------------------------------------

LinphoneCallDir CallSession::getDirection() const {
	L_D();
	return d->direction;
}

Address CallSession::getDiversionAddress() const {
	L_D();

	Address diversionAddress;
	if (d->op && d->op->getDiversionAddress()) {
		diversionAddress.setImpl(d->op->getDiversionAddress());
	}
	return diversionAddress;
}

int CallSession::getDuration() const {
	L_D();
	switch (d->state) {
		case CallSession::State::End:
		case CallSession::State::Error:
		case CallSession::State::Released:
			return d->log->getDuration();
		default:
			return d->computeDuration();
	}
}

const LinphoneErrorInfo *CallSession::getErrorInfo() const {
	L_D();
	if (!d->nonOpError) linphone_error_info_from_sal_op(d->ei, d->op);
	return d->ei;
}

const std::shared_ptr<Address> CallSession::getLocalAddress() const {
	L_D();
	std::shared_ptr<Address> addr = nullptr;
	if (d->direction == LinphoneCallIncoming) {
		if (d->log && d->log->getToAddress()) {
			addr = d->log->getToAddress();
		}
	} else {
		if (d->log && d->log->getFromAddress()) {
			addr = d->log->getFromAddress();
		}
	}
	return addr;
}

shared_ptr<CallLog> CallSession::getLog() const {
	L_D();
	return d->log;
}

const std::shared_ptr<Address> CallSession::getContactAddress() const {
	L_D();
	const auto op = d->getOp();
	const auto &account = d->getDestAccount();
	const auto &accountContactAddress = (account) ? account->getContactAddress() : nullptr;
	std::shared_ptr<Address> contactAddress = nullptr;
	if (op && op->getContactAddress()) {
		contactAddress = Address::create();
		contactAddress->setImpl(op->getContactAddress());
	} else if (linphone_core_conference_server_enabled(getCore()->getCCore()) && account && accountContactAddress) {
		contactAddress = accountContactAddress->clone()->toSharedPtr();
	} else {
		lInfo() << "No contact address from op or account for at this time call session " << this << " (local address "
		        << *getLocalAddress() << " remote address "
		        << (getRemoteAddress() ? getRemoteAddress()->toString() : "sip:") << ").";
	}
	return contactAddress;
}

LinphoneReason CallSession::getReason() const {
	return linphone_error_info_get_reason(getErrorInfo());
}

shared_ptr<CallSession> CallSession::getReferer() const {
	L_D();
	return d->referer;
}

const string &CallSession::getReferTo() const {
	L_D();
	if (d->referToAddress) {
		d->referTo = d->referToAddress->toString();
		return d->referTo;
	}
	return Utils::getEmptyConstRefObject<string>();
}

const std::shared_ptr<Address> CallSession::getReferToAddress() const {
	L_D();
	return d->referToAddress;
}

std::shared_ptr<const Address> CallSession::getReferredBy() const {
	L_D();
	if (d->op) {
		auto addr = d->op->getReferredBy();
		if (addr) {
			d->mReferredBy = (new Address(addr))->toSharedPtr();
		}
	}
	return d->mReferredBy;
}

const std::shared_ptr<Address> CallSession::getRemoteAddress() const {
	L_D();
	return (d->log) ? (d->direction == LinphoneCallIncoming) ? d->log->getFromAddress() : d->log->getToAddress()
	                : nullptr;
}

const string &CallSession::getRemoteContact() const {
	L_D();
	if (d->op) {
		/* sal_op_get_remote_contact preserves header params */
		return d->op->getRemoteContact();
	}
	return Utils::getEmptyConstRefObject<string>();
}

const std::shared_ptr<Address> CallSession::getRemoteContactAddress() const {
	L_D();
	auto op = d->op;
	if (!op) {
		d->mRemoteContactAddress = nullptr;
		return d->mRemoteContactAddress;
	}
	auto salRemoteContactAddress = op->getRemoteContactAddress();
	if (!salRemoteContactAddress) {
		d->mRemoteContactAddress = nullptr;
		return d->mRemoteContactAddress;
	}
	std::shared_ptr<Address> remoteContactAddress = Address::create();
	remoteContactAddress->setImpl(salRemoteContactAddress);
	d->mRemoteContactAddress = remoteContactAddress;
	return d->mRemoteContactAddress;
}

const CallSessionParams *CallSession::getRemoteParams() {
	L_D();
	if (d->op) {
		const SalCustomHeader *ch = d->op->getRecvCustomHeaders();
		if (ch) {
			/* Instanciate a remote_params only if a SIP message was received before (custom headers indicates this) */
			if (!d->remoteParams) d->remoteParams = new CallSessionParams();
			d->remoteParams->getPrivate()->setCustomHeaders(ch);
		}

		const list<Content> &additionnalContents = d->op->getRemoteBodies();
		for (auto &content : additionnalContents) {
			if (content.getContentType() != ContentType::Sdp)
				d->remoteParams->addCustomContent(Content::create(content));
		}

		return d->remoteParams;
	}
	return nullptr;
}

CallSession::State CallSession::getState() const {
	L_D();
	return d->state;
}

CallSession::State CallSession::getPreviousState() const {
	L_D();
	return d->prevState;
}
CallSession::State CallSession::getLastStableState() const {
	L_D();
	return d->lastStableState;
}

const std::shared_ptr<Address> CallSession::getToAddress() const {
	L_D();
	return d->log->getToAddress();
}

const std::shared_ptr<Address> CallSession::getRequestAddress() const {
	L_D();
	if (d->op) {
		if (!d->requestAddress) {
			d->requestAddress = Address::create();
		}
		d->requestAddress->setImpl(d->op->getRequestAddress());
	} else {
		d->requestAddress = nullptr;
	}
	return d->requestAddress;
}

CallSession::State CallSession::getTransferState() const {
	L_D();
	return d->transferState;
}

shared_ptr<CallSession> CallSession::getTransferTarget() const {
	L_D();
	return d->transferTarget;
}

const char *CallSession::getToHeader(const string &name) const {
	L_D();
	if (d->op) {
		return sal_custom_header_find(d->op->getRecvCustomHeaders(), name.c_str());
	}
	return NULL;
}

// -----------------------------------------------------------------------------

const string CallSession::getToTag() const {
	L_D();
	if (d->op) {
		if (d->log->getDirection() == LinphoneCallIncoming) {
			return d->op->getLocalTag();
		} else {
			return d->op->getRemoteTag();
		}
	}
	return Utils::getEmptyConstRefObject<string>();
}

const string CallSession::getFromTag() const {
	L_D();
	if (d->op) {
		if (d->log->getDirection() == LinphoneCallIncoming) {
			return d->op->getRemoteTag();
		} else {
			return d->op->getLocalTag();
		}
	}
	return Utils::getEmptyConstRefObject<string>();
}

const string &CallSession::getRemoteUserAgent() const {
	L_D();
	if (d->op) return d->op->getRemoteUserAgent();
	return Utils::getEmptyConstRefObject<string>();
}

shared_ptr<CallSession> CallSession::getReplacedCallSession() const {
	L_D();
	if (!d->op) return nullptr;
	SalOp *replacedOp = d->op->getReplaces();
	if (!replacedOp) return nullptr;
	return static_cast<CallSession *>(replacedOp->getUserPointer())->getSharedFromThis();
}

CallSessionParams *CallSession::getCurrentParams() const {
	L_D();
	d->updateCurrentParams();
	return d->currentParams;
}

// -----------------------------------------------------------------------------

const CallSessionParams *CallSession::getParams() const {
	L_D();
	return d->params;
}

void CallSession::updateContactAddress(Address &contactAddress) const {
	auto contactUriParams = getParams()->getPrivate()->getCustomContactUriParameters();
	for (const auto &[name, value] : contactUriParams) {
		if (value.empty()) {
			contactAddress.setUriParam(name);
		} else {
			contactAddress.setUriParam(name, value);
		}
	}
	auto contactParams = getParams()->getPrivate()->getCustomContactParameters();
	for (const auto &[name, value] : contactParams) {
		if (value.empty()) {
			contactAddress.setParam(name);
		} else {
			contactAddress.setParam(name, value);
		}
	}
}

void CallSession::updateContactAddressInOp() {
	L_D();
	Address contactAddress;
	const auto &account = d->getDestAccount();
	if (account) {
		const auto &accountOp = account->getOp();
		const auto &accountContactAddress = account->getContactAddress();
		if (accountOp && accountOp->getContactAddress()) {
			/* Give a chance to update the contact address if connectivity has changed */
			contactAddress.setImpl(accountOp->getContactAddress());
		} else if (linphone_core_conference_server_enabled(getCore()->getCCore()) && accountContactAddress) {
			contactAddress = *accountContactAddress;
		}

	} else if (d->op && d->op->getContactAddress()) {
		contactAddress.setImpl(d->op->getContactAddress());
	} else {
		contactAddress = Address(L_C_TO_STRING(linphone_core_get_identity(getCore()->getCCore())));
	}

	updateContactAddress(contactAddress);
	lInfo() << "Updating contact address for session " << this << " to " << contactAddress;
	if (d->op) {
		d->op->setContactAddress(contactAddress.getImpl());
	}
}

void CallSession::addPendingAction(std::function<LinphoneStatus()> f) {
	L_D();
	d->pendingActions.push(f);
}

// -----------------------------------------------------------------------------

bool CallSession::isEarlyState(CallSession::State state) {
	switch (state) {
		case CallSession::State::Idle:
		case CallSession::State::OutgoingInit:
		case CallSession::State::OutgoingEarlyMedia:
		case CallSession::State::OutgoingRinging:
		case CallSession::State::OutgoingProgress:
		case CallSession::State::IncomingReceived:
		case CallSession::State::PushIncomingReceived:
		case CallSession::State::IncomingEarlyMedia:
		case CallSession::State::EarlyUpdatedByRemote:
		case CallSession::State::EarlyUpdating:
			return true;
		default:
			return false;
	}
}

bool CallSession::isPredefinedSubject(const std::string &subject) {
	return std::find_if(CallSession::predefinedSubject.cbegin(), CallSession::predefinedSubject.cend(),
	                    [&subject](const auto &element) { return (subject.compare(element.second) == 0); }) !=
	       CallSession::predefinedSubject.cend();
}

bool CallSession::areSoundResourcesAvailable() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	bool available = (listeners.size() > 0);
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			available &= listenerRef->areSoundResourcesAvailable(getSharedFromThis());
		}
	}
	return available;
}

bool CallSession::isPlayingRingbackTone() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	bool playing = (listeners.size() > 0);
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			playing &= listenerRef->isPlayingRingbackTone(getSharedFromThis());
		}
	}
	return playing;
}

void CallSession::notifyCameraNotWorking(const char *cameraName) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCameraNotWorking(getSharedFromThis(), cameraName);
		}
	}
}

void CallSession::notifyResetFirstVideoFrameDecoded() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onResetFirstVideoFrameDecoded(getSharedFromThis());
		}
	}
}

void CallSession::notifyFirstVideoFrameDecoded() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onFirstVideoFrameDecoded(getSharedFromThis());
		}
	}
}

void CallSession::notifySnapshotTaken(const char *filepath) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onSnapshotTaken(getSharedFromThis(), filepath);
		}
	}
}

void CallSession::notifyRealTimeTextCharacterReceived(RealtimeTextReceivedCharacter *data) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onRealTimeTextCharacterReceived(getSharedFromThis(), data);
		}
	}
}

#ifdef HAVE_BAUDOT
void CallSession::notifyBaudotCharacterReceived(char receivedCharacter) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onBaudotCharacterReceived(getSharedFromThis(), receivedCharacter);
		}
	}
}

void CallSession::notifyBaudotDetected(MSBaudotStandard standard) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onBaudotDetected(getSharedFromThis(), standard);
		}
	}
}
#endif /* HAVE_BAUDOT */

void CallSession::notifyLossOfMediaDetected() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onLossOfMediaDetected(getSharedFromThis());
		}
	}
}

void CallSession::notifySendMasterKeyChanged(const std::string key) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onSendMasterKeyChanged(getSharedFromThis(), key);
		}
	}
}

void CallSession::notifyReceiveMasterKeyChanged(const std::string key) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			listenerRef->onReceiveMasterKeyChanged(getSharedFromThis(), key);
		}
	}
}

void CallSession::notifyUpdateMediaInfoForReporting(const int type) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onUpdateMediaInfoForReporting(getSharedFromThis(), type);
		}
	}
}

void CallSession::notifyRtcpUpdateForReporting(SalStreamType type) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onRtcpUpdateForReporting(getSharedFromThis(), type);
		}
	}
}

void CallSession::notifyStatsUpdated(const shared_ptr<CallStats> &stats) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onStatsUpdated(getSharedFromThis(), stats);
		}
	}
}

void CallSession::notifyTmmbrReceived(const int index, const int max_bitrate) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onTmmbrReceived(getSharedFromThis(), index, max_bitrate);
		}
	}
}

void CallSession::notifyAlert(std::shared_ptr<Alert> &alert) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onAlertNotified(alert);
		}
	}
}

void CallSession::notifyCallSessionStateChanged(CallSession::State newState, const string &message) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCallSessionStateChanged(getSharedFromThis(), newState, message);
		}
	}
}

void CallSession::notifyCallSessionTransferStateChanged(CallSession::State newState) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCallSessionTransferStateChanged(getSharedFromThis(), newState);
		}
	}
}

void CallSession::notifyCallSessionStateChangedForReporting() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCallSessionStateChangedForReporting(getSharedFromThis());
		}
	}
}

void CallSession::notifyCallSessionEarlyFailed(LinphoneErrorInfo *ei) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCallSessionEarlyFailed(getSharedFromThis(), ei);
		}
	}
}

void CallSession::notifyStartRingtone() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onStartRingtone(getSharedFromThis());
		}
	}
}

void CallSession::notifyIncomingCallSessionTimeoutCheck(int elapsed, bool oneSecondElapsed) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onIncomingCallSessionTimeoutCheck(getSharedFromThis(), elapsed, oneSecondElapsed);
		}
	}
}

void CallSession::notifyPushCallSessionTimeoutCheck(int elapsed) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onPushCallSessionTimeoutCheck(getSharedFromThis(), elapsed);
		}
	}
}

void CallSession::notifyIncomingCallSessionNotified() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onIncomingCallSessionNotified(getSharedFromThis());
		}
	}
}

void CallSession::notifyIncomingCallSessionStarted() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onIncomingCallSessionStarted(getSharedFromThis());
		}
	}
}

void CallSession::notifyCallSessionAccepted() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCallSessionAccepted(getSharedFromThis());
		}
	}
}

void CallSession::notifyCallSessionAccepting() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCallSessionAccepting(getSharedFromThis());
		}
	}
}

void CallSession::notifyCallSessionSetTerminated() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCallSessionSetTerminated(getSharedFromThis());
		}
	}
}

void CallSession::notifyCallSessionSetReleased() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCallSessionSetReleased(getSharedFromThis());
		}
	}
}

void CallSession::notifyCheckForAcceptation() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onCheckForAcceptation(getSharedFromThis());
		}
	}
}

void CallSession::notifyGoClearAckSent() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onGoClearAckSent();
		}
	}
}

void CallSession::notifyAckBeingSent(LinphoneHeaders *headers) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onAckBeingSent(getSharedFromThis(), headers);
		}
	}
}

void CallSession::notifyAckReceived(LinphoneHeaders *headers) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onAckReceived(getSharedFromThis(), headers);
		}
	}
}

void CallSession::notifyBackgroundTaskToBeStarted() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onBackgroundTaskToBeStarted(getSharedFromThis());
		}
	}
}

void CallSession::notifyBackgroundTaskToBeStopped() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onBackgroundTaskToBeStopped(getSharedFromThis());
		}
	}
}

void CallSession::notifyInfoReceived(LinphoneInfoMessage *info) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onInfoReceived(getSharedFromThis(), info);
		}
	}
}

void CallSession::notifySetCurrentSession() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onSetCurrentSession(getSharedFromThis());
		}
	}
}

void CallSession::notifyResetCurrentSession() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onResetCurrentSession(getSharedFromThis());
		}
	}
}

void CallSession::notifyDtmfReceived(char dtmf) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onDtmfReceived(getSharedFromThis(), dtmf);
		}
	}
}

void CallSession::notifyRemoteRecording(bool isRecording) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onRemoteRecording(getSharedFromThis(), isRecording);
		}
	}
}

void CallSession::notifySecurityLevelDowngraded() {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onSecurityLevelDowngraded(getSharedFromThis());
		}
	}
}

void CallSession::notifyEncryptionChanged(bool activated, const std::string &authToken) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onEncryptionChanged(getSharedFromThis(), activated, authToken);
		}
	}
}

void CallSession::notifyAuthenticationTokenVerified(bool verified) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onAuthenticationTokenVerified(getSharedFromThis(), verified);
		}
	}
}

void CallSession::notifyVideoDisplayErrorOccurred(int errorCode) {
	L_D();
	// Copy list of listeners as the callback might delete one
	auto listeners = d->listeners;
	for (const auto &listener : listeners) {
		auto listenerRef = listener.lock();
		if (listenerRef) {
			auto logContext = listenerRef->getLogContextualizer();
			listenerRef->onVideoDisplayErrorOccurred(getSharedFromThis(), errorCode);
		}
	}
}

LINPHONE_END_NAMESPACE
