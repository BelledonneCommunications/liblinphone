/*
 * call-op.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "bellesip_sal/sal_impl.h"
#include "offeranswer.h"
#include "sal/call-op.h"

#include <bctoolbox/defs.h>
#include <belle-sip/provider.h>

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SalCallOp::SalCallOp (Sal *sal) : SalOp(sal) {
	mType = Type::Call;
	fillCallbacks();
}

SalCallOp::~SalCallOp () {
	if (mLocalMedia)
		sal_media_description_unref(mLocalMedia);
	if (mRemoteMedia)
		sal_media_description_unref(mRemoteMedia);
}

int SalCallOp::setLocalMediaDescription (SalMediaDescription *desc) {
	if (desc) {
		sal_media_description_ref(desc);
		belle_sip_error_code error;
		belle_sdp_session_description_t *sdp = media_description_to_sdp(desc);
		vector<char> buffer = marshalMediaDescription(sdp, error);
		belle_sip_object_unref(sdp);
		if (error != BELLE_SIP_OK)
			return -1;

		mLocalBody.setContentType(ContentType::Sdp);
		mLocalBody.setBody(move(buffer));
	} else {
		mLocalBody = Content();
	}

	if (mLocalMedia)
		sal_media_description_unref(mLocalMedia);
	mLocalMedia = desc;

	if (mRemoteMedia) {
		// Case of an incoming call where we modify the local capabilities between the time
		// the call is ringing and it is accepted (for example if you want to accept without video
		// reset the sdp answer so that it is computed again.
		if (mSdpAnswer) {
			belle_sip_object_unref(mSdpAnswer);
			mSdpAnswer = nullptr;
		}
	}
	return 0;
}

int SalCallOp::setLocalBody (const Content &body) {
	Content bodyCopy = body;
	return setLocalBody(move(bodyCopy));
}

int SalCallOp::setLocalBody (Content &&body) {
	if (!body.isValid())
		return -1;

	if (body.getContentType() == ContentType::Sdp) {
		SalMediaDescription *desc = nullptr;
		if (body.getSize() > 0) {
			belle_sdp_session_description_t *sdp = belle_sdp_session_description_parse(body.getBodyAsString().c_str());
			if (!sdp)
				return -1;
			desc = sal_media_description_new();
			if (sdp_to_media_description(sdp, desc) != 0) {
				sal_media_description_unref(desc);
				return -1;
			}
		}
		if (mLocalMedia)
			sal_media_description_unref(mLocalMedia);
		mLocalMedia = desc;
	}

	mLocalBody = move(body);
	return 0;
}

belle_sip_header_allow_t *SalCallOp::createAllow (bool enableUpdate) {
	ostringstream oss;
	oss << "INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO";
	if (enableUpdate)
		oss << ", UPDATE";
	return belle_sip_header_allow_create(oss.str().c_str());
}

std::vector<char> SalCallOp::marshalMediaDescription (belle_sdp_session_description_t *sessionDesc, belle_sip_error_code &error) {
	size_t length = 0;
	size_t bufferSize = 2048;
	vector<char> buffer(bufferSize);

	// Try to marshal the description. This could go higher than 2k so we iterate.
	error = BELLE_SIP_BUFFER_OVERFLOW;
	while ((error != BELLE_SIP_OK) && (bufferSize <= SIP_MESSAGE_BODY_LIMIT)) {
		error = belle_sip_object_marshal(BELLE_SIP_OBJECT(sessionDesc), buffer.data(), bufferSize, &length);
		if (error != BELLE_SIP_OK) {
			bufferSize *= 2;
			length = 0;
			buffer.resize(bufferSize);
		}
	}

	// Give up if hard limit is reached
	if (error != BELLE_SIP_OK) {
		lError() << "Buffer too small (" << bufferSize << ") or not enough memory, giving up SDP";
		return std::vector<char>(); // Return a new vector in order to free the buffer held by 'buffer' vector
	}

	buffer.resize(length);
	return buffer;
}

int SalCallOp::setSdp (belle_sip_message_t *msg, belle_sdp_session_description_t *sessionDesc) {
	if (!sessionDesc)
		return -1;

	belle_sip_error_code error;
	vector<char> buffer = marshalMediaDescription(sessionDesc, error);
	if (error != BELLE_SIP_OK)
		return -1;

	Content body;
	body.setContentType(ContentType::Sdp);
	body.setBody(move(buffer));
	setCustomBody(msg, body);
	return 0;
}

int SalCallOp::setSdpFromDesc (belle_sip_message_t *msg, const SalMediaDescription *desc) {
	auto sdp = media_description_to_sdp(desc);
	int err = setSdp(msg, sdp);
	belle_sip_object_unref(sdp);
	return err;
}

void SalCallOp::fillInvite (belle_sip_request_t *invite) {
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(invite), BELLE_SIP_HEADER(createAllow(mRoot->mEnableSipUpdate)));
	if (mRoot->mSessionExpires != 0) {
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(invite), belle_sip_header_create("Session-expires", "600;refresher=uas"));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(invite), belle_sip_header_create("Supported", "timer"));
	}
	mSdpOffering = (mLocalBody.getContentType() == ContentType::Sdp);
	setCustomBody(BELLE_SIP_MESSAGE(invite), mLocalBody);
}

void SalCallOp::setReleased () {
	if (!mCallReleased){
		mState = State::Terminated;
		mRoot->mCallbacks.call_released(this);
		mCallReleased = true;
		// Be aware that the following line may destroy the op
		setOrUpdateDialog(nullptr);
	}
}

void SalCallOp::processIoErrorCb (void *userCtx, const belle_sip_io_error_event_t *event) {
	auto op = static_cast<SalCallOp *>(userCtx);
	if (op->mState == State::Terminated)
		return;

	if (op->mPendingClientTransaction
		&& (belle_sip_transaction_get_state(BELLE_SIP_TRANSACTION(op->mPendingClientTransaction)) == BELLE_SIP_TRANSACTION_INIT)
	) {
		sal_error_info_set(&op->mErrorInfo, SalReasonIOError, "SIP", 503, "IO error", nullptr);
		op->mRoot->mCallbacks.call_failure(op);

		if (!op->mDialog || (belle_sip_dialog_get_state(op->mDialog) != BELLE_SIP_DIALOG_CONFIRMED)) {
			// Call terminated very very early, before INVITE is even sent, probably DNS resolution timeout
			op->mState = State::Terminating;
			op->setReleased();
		}
	} else {
		// Nothing to be done. If the error comes from a connectivity loss,
		// the call will be marked as broken, and an attempt to repair it will be done.
	}
}

void SalCallOp::cancellingInvite (const SalErrorInfo *info) {
	cancelInvite(info);
	mState = State::Terminating;
}

Content SalCallOp::extractBody (belle_sip_message_t *message) {
	Content body;

	auto contentTypeHeader = belle_sip_message_get_header_by_type(message, belle_sip_header_content_type_t);
	string typeStr = contentTypeHeader ? belle_sip_header_content_type_get_type(contentTypeHeader) : "";
	string subtypeStr = contentTypeHeader ? belle_sip_header_content_type_get_subtype(contentTypeHeader) : "";
	if (!typeStr.empty() && !subtypeStr.empty())
		body.setContentType(ContentType(typeStr, subtypeStr));

	auto contentDispositionHeader = belle_sip_message_get_header_by_type(message, belle_sip_header_content_disposition_t);
	if (contentDispositionHeader) {
		auto contentDisposition = ContentDisposition(belle_sip_header_content_disposition_get_content_disposition(contentDispositionHeader));
		if (belle_sip_parameters_has_parameter(BELLE_SIP_PARAMETERS(contentDispositionHeader), "handling")) {
			contentDisposition.setParameter("handling=" + string(belle_sip_parameters_get_parameter(BELLE_SIP_PARAMETERS(contentDispositionHeader), "handling")));
		}
		body.setContentDisposition(contentDisposition);
	}

	auto contentLengthHeader = belle_sip_message_get_header_by_type(message, belle_sip_header_content_length_t);
	size_t length = contentLengthHeader ? belle_sip_header_content_length_get_content_length(contentLengthHeader) : 0;
	const char *content = belle_sip_message_get_body(message);
	if ((length > 0) && content)
		body.setBody(content, length);
	return body;
}

int SalCallOp::parseSdpBody (const Content &body, belle_sdp_session_description_t **sessionDesc, SalReason *error) {
	*sessionDesc = nullptr;
	*error = SalReasonNone;

	if (mSdpHandling == SalOpSDPSimulateError) {
		lError() << "Simulating SDP parsing error for op " << this;
		*error = SalReasonNotAcceptable;
		return -1;
	}

	if (mSdpHandling == SalOpSDPSimulateRemove) {
		lError() << "Simulating no SDP for op " << this;
		return 0;
	}

	string strBody = body.getBodyAsString();
	if (strBody.empty())
		return 0;
	*sessionDesc = belle_sdp_session_description_parse(strBody.c_str());
	if (!*sessionDesc) {
		lError() << "Failed to parse SDP message";
		*error = SalReasonNotAcceptable;
		return -1;
	}

	return 0;
}

void SalCallOp::setAddrTo0000 (char value[], size_t sz) {
	if (ms_is_ipv6(value))
		strncpy(value, "::0", sz);
	else
		strncpy(value, "0.0.0.0", sz);
}

void SalCallOp::sdpProcess () {
	lInfo() << "Doing SDP offer/answer process of type " << (mSdpOffering ? "outgoing" : "incoming");
	if (mResult) {
		sal_media_description_unref(mResult);
		mResult = nullptr;
	}

	// If SDP was invalid
	if (!mRemoteMedia)
		return;

	mResult = sal_media_description_new();
	if (mSdpOffering) {
		offer_answer_initiate_outgoing(mRoot->mFactory, mLocalMedia, mRemoteMedia, mResult);
	} else {
		if (mSdpAnswer)
			belle_sip_object_unref(mSdpAnswer);
		offer_answer_initiate_incoming(mRoot->mFactory, mLocalMedia, mRemoteMedia, mResult, mRoot->mOneMatchingCodec);
		// For backward compatibility purpose
		if (mCnxIpTo0000IfSendOnlyEnabled && sal_media_description_has_dir(mResult,SalStreamSendOnly)) {
			setAddrTo0000(mResult->addr, sizeof(mResult->addr));
			for (int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
				if (mResult->streams[i].dir == SalStreamSendOnly) {
					setAddrTo0000(mResult->streams[i].rtp_addr, sizeof(mResult->streams[i].rtp_addr));
					setAddrTo0000(mResult->streams[i].rtcp_addr, sizeof(mResult->streams[i].rtcp_addr));
				}
			}
		}

		mSdpAnswer = reinterpret_cast<belle_sdp_session_description_t *>(belle_sip_object_ref(media_description_to_sdp(mResult)));
		// Once we have generated the SDP answer, we modify the result description for processing by the upper layer
		// It should contain media parameters constraints from the remote offer, not our response
		strcpy(mResult->addr, mRemoteMedia->addr);
		mResult->bandwidth = mRemoteMedia->bandwidth;

		for(int i = 0; i < SAL_MEDIA_DESCRIPTION_MAX_STREAMS; i++) {
			// Copy back parameters from remote description that we need in our result description
			if (mResult->streams[i].rtp_port != 0) { // If the stream was accepted
				strcpy(mResult->streams[i].rtp_addr, mRemoteMedia->streams[i].rtp_addr);
				mResult->streams[i].ptime = mRemoteMedia->streams[i].ptime;
				mResult->streams[i].bandwidth = mRemoteMedia->streams[i].bandwidth;
				mResult->streams[i].rtp_port = mRemoteMedia->streams[i].rtp_port;
				strcpy(mResult->streams[i].rtcp_addr, mRemoteMedia->streams[i].rtcp_addr);
				mResult->streams[i].rtcp_port = mRemoteMedia->streams[i].rtcp_port;
				if (sal_stream_description_has_srtp(&mResult->streams[i])) {
					int cryptoIdx = Sal::findCryptoIndexFromTag(	mRemoteMedia->streams[i].crypto, static_cast<unsigned char>(mResult->streams[i].crypto[0].tag));
					if (cryptoIdx >= 0)
						mResult->streams[i].crypto[0] = mRemoteMedia->streams[i].crypto[cryptoIdx];
					else
						lError() << "Failed to find crypto algo with tag: " << mResult->streams[i].crypto_local_tag << "from resulting description [" << mResult << "]";
				}
			}
		}
	}
}

void SalCallOp::handleBodyFromResponse (belle_sip_response_t *response) {
	Content body = extractBody(BELLE_SIP_MESSAGE(response));
	if (mRemoteMedia) {
		sal_media_description_unref(mRemoteMedia);
		mRemoteMedia = nullptr;
	}
	if (body.getContentType() == ContentType::Sdp) {
		belle_sdp_session_description_t *sdp = nullptr;
		SalReason reason;
		if (parseSdpBody(body, &sdp, &reason) == 0) {
			if (sdp) {
				mRemoteMedia = sal_media_description_new();
				sdp_to_media_description(sdp, mRemoteMedia);
				mRemoteBody = move(body);
			} // If no SDP in response, what can we do?
		}
		// Process sdp in any case to reset result media description
		if (mLocalMedia)
			sdpProcess();
	} else {
		mRemoteBody = move(body);
	}
}

void SalCallOp::setError (belle_sip_response_t *response, bool fatal) {
	setErrorInfoFromResponse(response);
	if (fatal)
		mState = State::Terminating;
	mRoot->mCallbacks.call_failure(this);
}

int SalCallOp::vfuRetryCb (void *userCtx, unsigned int events) {
	auto op = static_cast<SalCallOp *>(userCtx);
	op->sendVfuRequest();
	op->unref();
	return BELLE_SIP_STOP;
}

void SalCallOp::processResponseCb (void *userCtx, const belle_sip_response_event_t *event) {
	auto op = static_cast<SalCallOp *>(userCtx);
	auto response = belle_sip_response_event_get_response(event);
	int code = belle_sip_response_get_status_code(response);
	auto clientTransaction = belle_sip_response_event_get_client_transaction(event);
	if (!clientTransaction) {
		lWarning() << "Discarding stateless response [" << code << "] on op [" << op << "]";
		return;
	}

	auto dialog = belle_sip_response_event_get_dialog(event);
	op->setOrUpdateDialog(dialog);
	auto dialogState = dialog ? belle_sip_dialog_get_state(dialog) : BELLE_SIP_DIALOG_NULL;
	lInfo() << "Op [" << op << "] receiving call response [" << code << "], dialog is [" << dialog << "] in state ["
		<< belle_sip_dialog_state_to_string(dialogState) << "]";
	op->ref(); // To make sure no cb will destroy op

	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(clientTransaction));
	string method = belle_sip_request_get_method(request);

	switch (dialogState) {
		case BELLE_SIP_DIALOG_NULL:
		case BELLE_SIP_DIALOG_EARLY:
			if (method == "INVITE") {
				if (op->mState == State::Terminating) {
					// Check if CANCEL was sent before
					string pendingRequestMethod = belle_sip_request_get_method(belle_sip_transaction_get_request(
						BELLE_SIP_TRANSACTION(op->mPendingClientTransaction)
					));
					if (pendingRequestMethod == "CANCEL") {
						// It was sent already, so just expect the 487 or any error response to send the call_released() notification
						if ((code >= 300) && !op->mDialog)
							op->setReleased();
					} else {
						// It wasn't sent
						if (code < 200) {
							op->cancellingInvite(nullptr);
						} else {
							// No need to send the INVITE because the UAS rejected the INVITE
							if (!op->mDialog)
								op->setReleased();
						}
					}
				} else if ((code >= 180) && (code < 200)) {
					auto *previousResponse = dialog ? static_cast<belle_sip_response_t *>(
						belle_sip_object_data_get(BELLE_SIP_OBJECT(dialog), "early_response")
					) : nullptr;
					if (previousResponse == nullptr || (code > belle_sip_response_get_status_code(previousResponse))) {
						op->handleBodyFromResponse(response);
						op->mRoot->mCallbacks.call_ringing(op);
					}
					if (dialog) {
						belle_sip_object_data_set(
							BELLE_SIP_OBJECT(dialog),
							"early_response",
							belle_sip_object_ref(response),
							belle_sip_object_unref
						);
					}
				} else if (code >= 300) {
					op->setError(response, true);
					if (!op->mDialog)
						op->setReleased();
				}
			} else if ((code >= 200) && (code < 300)) {
				if (method == "UPDATE") {
					op->handleBodyFromResponse(response);
					op->mRoot->mCallbacks.call_accepted(op);
				} else if (method == "CANCEL") {
					op->mRoot->mCallbacks.call_cancel_done(op);
				}
			}
			break;
		case BELLE_SIP_DIALOG_CONFIRMED:
			switch (op->mState) {
				case State::Early: // Invite case
				case State::Active: // Re-invite, INFO, UPDATE case
					if (method == "INVITE") {
						if ((code >= 200) && (code < 300)) {
							op->handleBodyFromResponse(response);
							auto ack = belle_sip_dialog_create_ack(op->mDialog, belle_sip_dialog_get_local_seq_number(op->mDialog));
							if (!ack) {
								lError() << "This call has been already terminated";
								return;
							}
							// Ref the ack request so that it is not destroyed when the call_ack_being_sent callbacks is called
							belle_sip_object_ref(ack);
							if (op->mSdpAnswer) {
								setSdp(BELLE_SIP_MESSAGE(ack), op->mSdpAnswer);
								belle_sip_object_unref(op->mSdpAnswer);
								op->mSdpAnswer = nullptr;
							}
							belle_sip_message_add_header(BELLE_SIP_MESSAGE(ack), BELLE_SIP_HEADER(op->mRoot->mUserAgentHeader));
							op->mRoot->mCallbacks.call_accepted(op); // INVITE
							op->mRoot->mCallbacks.call_ack_being_sent(op, reinterpret_cast<SalCustomHeader *>(ack));
							belle_sip_dialog_send_ack(op->mDialog, ack);
							belle_sip_object_unref(ack);
							op->mState = State::Active;
						} else if (code >= 300) {
							op->setError(response, false);
						}
					} else if (method == "INFO") {
						auto contentTypeHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_content_type_t);
						if (contentTypeHeader) {
							string typeStr = belle_sip_header_content_type_get_type(contentTypeHeader);
							string subtypeStr = belle_sip_header_content_type_get_subtype(contentTypeHeader);
							if ((code == 491) && (typeStr == "application") && (subtypeStr == "media_control+xml")) {
								unsigned int retryIn = rand() % 1001; // [0;1000]
								belle_sip_source_t *s = op->mRoot->createTimer(vfuRetryCb, op->ref(), retryIn, "vfu request retry");
								lInfo() << "Rejected vfu request on op [" << op << "], just retry in [" << retryIn << "] ms";
								belle_sip_object_unref(s);
							}
						}
					} else if (method == "UPDATE") {
						op->mRoot->mCallbacks.call_accepted(op); // INVITE
					} else if (method == "CANCEL") {
						op->mRoot->mCallbacks.call_cancel_done(op);
					}
					break;
				case State::Terminating:
					op->sendRequest(belle_sip_dialog_create_request(op->mDialog, "BYE"));
					break;
				case State::Terminated:
				default:
					lError() << "Call op [" << op << "] receives unexpected answer [" << code << "] while in state [" << toString(op->mState) << "]";
					break;
			}
			break;
		case BELLE_SIP_DIALOG_TERMINATED:
			if ((code >= 300) && ((method == "INVITE") || (method == "BYE")))
				op->setError(response, true);
			break;
		default:
			lError() << "Call op [" << op << "] receive answer [" << code << "] not implemented";
			break;
	}
	op->unref();
}

void SalCallOp::processTimeoutCb (void *userCtx, const belle_sip_timeout_event_t *event) {
	auto op = static_cast<SalCallOp *>(userCtx);
	if (op->mState == State::Terminated)
		return;

	if (op->mDialog) {
		// Dialog will terminated shortly, nothing to do
	} else {
		// Call terminated very early
		sal_error_info_set(&op->mErrorInfo, SalReasonRequestTimeout, "SIP", 408, "Request timeout", nullptr);
		op->mRoot->mCallbacks.call_failure(op);
		op->mState = State::Terminating;
		op->setReleased();
	}
}

void SalCallOp::processTransactionTerminatedCb (void *userCtx, const belle_sip_transaction_terminated_event_t *event) {
	auto op = static_cast<SalCallOp *>(userCtx);
	auto clientTransaction = belle_sip_transaction_terminated_event_get_client_transaction(event);
	auto serverTransaction = belle_sip_transaction_terminated_event_get_server_transaction(event);
	auto request = (clientTransaction
		? belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(clientTransaction))
		: belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(serverTransaction))
	);
	auto response = (clientTransaction
		? belle_sip_transaction_get_response(BELLE_SIP_TRANSACTION(clientTransaction))
		: belle_sip_transaction_get_response(BELLE_SIP_TRANSACTION(serverTransaction))
	);

	bool releaseCall = false;
	int code = 0;
	if (response)
		code = belle_sip_response_get_status_code(response);

	string method = belle_sip_request_get_method(request);
	if ((op->mState == State::Terminating)
		&& (method == "BYE")
		&& (!response
			|| ((belle_sip_response_get_status_code(response) != 401)
				&& (belle_sip_response_get_status_code(response) != 407)
			))
		&& !op->mDialog
	) {
		releaseCall = true;
	} else if ((op->mState == State::Early) && (code < 200)) {
		// Call terminated early
		sal_error_info_set(&op->mErrorInfo, SalReasonIOError, "SIP", 503, "I/O error", nullptr);
		op->mState = State::Terminating;
		op->mRoot->mCallbacks.call_failure(op);
		releaseCall = true;
	}

	if (serverTransaction) {
		if (op->mPendingServerTransaction == serverTransaction) {
			belle_sip_object_unref(op->mPendingServerTransaction);
			op->mPendingServerTransaction = nullptr;
		}
		if (op->mPendingUpdateServerTransaction == serverTransaction) {
			belle_sip_object_unref(op->mPendingUpdateServerTransaction);
			op->mPendingUpdateServerTransaction = nullptr;
		}
	}

	if (releaseCall)
		op->setReleased();
}

bool SalCallOp::isMediaDescriptionAcceptable (SalMediaDescription *md) {
	if (md->nb_streams == 0) {
		lWarning() << "Media description does not define any stream";
		return false;
	}
	return true;
}

SalReason SalCallOp::processBodyForInvite (belle_sip_request_t *invite) {
	SalReason reason = SalReasonNone;
	Content body = extractBody(BELLE_SIP_MESSAGE(invite));
	if (!body.isValid())
		return SalReasonUnsupportedContent;

	if ((body.getContentType() == ContentType::Sdp) || (body.getContentType().isEmpty() && body.isEmpty())) {
		belle_sdp_session_description_t *sdp;
		if (parseSdpBody(body, &sdp, &reason) == 0) {
			if (sdp) {
				mSdpOffering = false;
				if (mRemoteMedia)
					sal_media_description_unref(mRemoteMedia);
				mRemoteMedia = sal_media_description_new();
				sdp_to_media_description(sdp, mRemoteMedia);
				// Make some sanity check about the received SDP
				if (!isMediaDescriptionAcceptable(mRemoteMedia))
					reason = SalReasonNotAcceptable;
				belle_sip_object_unref(sdp);
			} else {
				mSdpOffering = true; // INVITE without SDP
			}
		}
		if (reason != SalReasonNone) {
			SalErrorInfo sei;
			memset(&sei, 0, sizeof(sei));
			sal_error_info_set(&sei, reason, "SIP", 0, nullptr, nullptr);
			declineWithErrorInfo(&sei, nullptr);
			sal_error_info_reset(&sei);
		}
	}
	mRemoteBody = move(body);
	return reason;
}

SalReason SalCallOp::processBodyForAck (belle_sip_request_t *ack) {
	SalReason reason = SalReasonNone;
	Content body = extractBody(BELLE_SIP_MESSAGE(ack));
	if (!body.isValid())
		return SalReasonUnsupportedContent;
	if (body.getContentType() == ContentType::Sdp) {
		belle_sdp_session_description_t *sdp;
		if (parseSdpBody(body, &sdp, &reason) == 0) {
			if (sdp) {
				if (mRemoteMedia)
					sal_media_description_unref(mRemoteMedia);
				mRemoteMedia = sal_media_description_new();
				sdp_to_media_description(sdp, mRemoteMedia);
				sdpProcess();
				belle_sip_object_unref(sdp);
			} else {
				lWarning() << "SDP expected in ACK but not found";
			}
		}
	}
	mRemoteBody = move(body);
	return reason;
}

void SalCallOp::callTerminated (belle_sip_server_transaction_t *serverTransaction, int statusCode, belle_sip_request_t *cancelRequest) {
	auto serverRequest = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(serverTransaction));
	mState = State::Terminating;
	setReasonErrorInfo(BELLE_SIP_MESSAGE(cancelRequest ? cancelRequest : serverRequest));
	belle_sip_response_t *response = createResponseFromRequest(serverRequest, statusCode);
	belle_sip_server_transaction_send_response(serverTransaction, response);
	mRoot->mCallbacks.call_terminated(this, (mDir == Dir::Incoming) ? getFrom().c_str() : getTo().c_str());
}

void SalCallOp::resetDescriptions () {
	if (mRemoteMedia) {
		sal_media_description_unref(mRemoteMedia);
		mRemoteMedia = nullptr;
	}
	if (mResult) {
		sal_media_description_unref(mResult);
		mResult = nullptr;
	}
}

void SalCallOp::unsupportedMethod (belle_sip_server_transaction_t *serverTransaction, belle_sip_request_t *request) {
	auto response = belle_sip_response_create_from_request(request, 501);
	belle_sip_server_transaction_send_response(serverTransaction, response);
}

bool SalCallOp::isAPendingIncomingInviteTransaction (belle_sip_transaction_t *transaction) {
	return (BELLE_SIP_OBJECT_IS_INSTANCE_OF(transaction, belle_sip_ist_t)
		&& belle_sip_transaction_state_is_transient(belle_sip_transaction_get_state(transaction))
	);
}

void SalCallOp::processRequestEventCb (void *userCtx, const belle_sip_request_event_t *event) {
	auto op = static_cast<SalCallOp *>(userCtx);
	bool isUpdate = false;
	bool dropOp = false;
	belle_sip_response_t *response = nullptr;
	belle_sip_server_transaction_t *serverTransaction = nullptr;
	auto request = belle_sip_request_event_get_request(event);
	string method = belle_sip_request_get_method(request);
	if (method != "ACK") { // ACK doesn't create a server transaction
		serverTransaction = belle_sip_provider_create_server_transaction(op->mRoot->mProvider, belle_sip_request_event_get_request(event));
		belle_sip_object_ref(serverTransaction);
		belle_sip_transaction_set_application_data(BELLE_SIP_TRANSACTION(serverTransaction), op->ref());
	}

	if (method == "INVITE") {
		if (op->mPendingServerTransaction)
			belle_sip_object_unref(op->mPendingServerTransaction);
		// Updating pending invite transaction
		op->mPendingServerTransaction = serverTransaction;
		belle_sip_object_ref(op->mPendingServerTransaction);
	}

	if (method == "UPDATE") {
		if (op->mPendingUpdateServerTransaction)
			belle_sip_object_unref(op->mPendingUpdateServerTransaction);
		// Updating pending update transaction
		op->mPendingUpdateServerTransaction = serverTransaction;
		belle_sip_object_ref(op->mPendingUpdateServerTransaction);
	}

	if (!op->mDialog) {
		op->setOrUpdateDialog(belle_sip_provider_create_dialog(op->mRoot->mProvider, BELLE_SIP_TRANSACTION(op->mPendingServerTransaction)));
		lInfo() << "New incoming call from [" << op->getFrom() << "] to [" << op->getTo() << "]";
	}

	auto dialogState = belle_sip_dialog_get_state(op->mDialog);
	switch (dialogState) {
		case BELLE_SIP_DIALOG_NULL:
			if (method == "INVITE") {
				if (!op->mReplaces
					&& (op->mReplaces = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request),belle_sip_header_replaces_t))
				) {
					belle_sip_object_ref(op->mReplaces);
				} else if(op->mReplaces) {
					lWarning() << "Replace header already set";
				}

				SalReason reason = op->processBodyForInvite(request);
				if (reason == SalReasonNone) {
					auto callInfoHeader = belle_sip_message_get_header(BELLE_SIP_MESSAGE(request), "Call-Info");
					if (callInfoHeader) {
						if (strstr(belle_sip_header_get_unparsed_value(callInfoHeader), "answer-after=")) {
							op->mAutoAnswerAsked = true;
							lInfo() << "The caller asked to automatically answer the call (Emergency?)";
						}
					}
					op->mRoot->mCallbacks.call_received(op);
				} else {
					sal_error_info_set(&op->mErrorInfo, reason, "SIP", 0, nullptr, nullptr);
					op->mRoot->mCallbacks.call_rejected(op);
					// The INVITE was declined by process_sdp_for_invite(). As we are not inside an established dialog, we can drop the op immediately.
					dropOp = true;
				}
				break;
			}
			BCTBX_NO_BREAK; // Else same behavior as for EARLY state, thus NO BREAK
		case BELLE_SIP_DIALOG_EARLY:
			if (method == "CANCEL") {
				if (belle_sip_request_event_get_server_transaction(event)) {
					// First answer 200 ok to cancel
					belle_sip_server_transaction_send_response(serverTransaction, op->createResponseFromRequest(request, 200));
					// Terminate invite transaction
					op->callTerminated(op->mPendingServerTransaction, 487, request);
				} else {
					// Call leg does not exist
					belle_sip_server_transaction_send_response(serverTransaction, op->createResponseFromRequest(request, 481));
				}
			} else if (method == "PRACK") {
				response = op->createResponseFromRequest(request, 200);
				belle_sip_server_transaction_send_response(serverTransaction, response);
			} else if (method == "UPDATE") {
				op->resetDescriptions();
				if (op->processBodyForInvite(request) == SalReasonNone)
					op->mRoot->mCallbacks.call_updating(op, true);
			} else {
				lError() << "Unexpected method [" << method << "] for dialog state BELLE_SIP_DIALOG_EARLY";
				unsupportedMethod(serverTransaction, request);
			}
			break;
		case BELLE_SIP_DIALOG_CONFIRMED:
			if (method == "ACK") { // Great ACK received
				if (!op->mPendingClientTransaction ||
					!belle_sip_transaction_state_is_transient(belle_sip_transaction_get_state(BELLE_SIP_TRANSACTION(op->mPendingClientTransaction)))
				) {
					if (op->mSdpOffering)
						op->processBodyForAck(request);
					op->mRoot->mCallbacks.call_ack_received(op, reinterpret_cast<SalCustomHeader *>(request));
				} else {
					lInfo() << "Ignored received ack since a new client transaction has been started since";
				}
			} else if (method == "BYE") {
				op->callTerminated(serverTransaction, 200, request);
				// Call end not notified by dialog deletion because transaction can end before dialog
			} else if ((method == "INVITE") || (isUpdate = (method == "UPDATE"))) {
				if (isUpdate && !belle_sip_message_get_body(BELLE_SIP_MESSAGE(request))) {
					// Session timer case
					// Session expire should be handled. To be done when real session timer (rfc4028) will be implemented.
					response = op->createResponseFromRequest(request, 200);
					belle_sip_server_transaction_send_response(serverTransaction, response);
					belle_sip_object_unref(op->mPendingUpdateServerTransaction);
					op->mPendingUpdateServerTransaction = nullptr;
				} else {
					// Re-invite
					op->resetDescriptions();
					if (op->processBodyForInvite(request) == SalReasonNone)
						op->mRoot->mCallbacks.call_updating(op, isUpdate);
				}
			} else if (method == "INFO") {
				auto body = belle_sip_message_get_body(BELLE_SIP_MESSAGE(request));
				if (body && strstr(body, "picture_fast_update")) {
					// VFU request
					lInfo() << "Receiving VFU request on op [" << op << "]";
					if (op->mRoot->mCallbacks.vfu_request)
						op->mRoot->mCallbacks.vfu_request(op);
				} else {
					auto msg = BELLE_SIP_MESSAGE(request);
					auto bodyHandler = BELLE_SIP_BODY_HANDLER(op->getBodyHandler(msg));
					if (bodyHandler) {
						auto contentTypeHeader = belle_sip_message_get_header_by_type(msg, belle_sip_header_content_type_t);
						if (contentTypeHeader) {
							string typeStr = belle_sip_header_content_type_get_type(contentTypeHeader);
							string subtypeStr = belle_sip_header_content_type_get_subtype(contentTypeHeader);
							if ((typeStr == "application") && (subtypeStr == "dtmf-relay")) {
								char tmp[10];
								if (sal_lines_get_value(belle_sip_message_get_body(msg), "Signal", tmp, sizeof(tmp)))
									op->mRoot->mCallbacks.dtmf_received(op, tmp[0]);
							} else {
								op->mRoot->mCallbacks.info_received(op, reinterpret_cast<SalBodyHandler *>(bodyHandler));
							}
						}
					} else {
						op->mRoot->mCallbacks.info_received(op, nullptr);
					}
				}
				response = op->createResponseFromRequest(request, 200);
				belle_sip_server_transaction_send_response(serverTransaction, response);
			} else if (method == "REFER") {
				op->processRefer(event, serverTransaction);
			} else if (method == "NOTIFY") {
				op->processNotify(event, serverTransaction);
			} else if (method == "OPTIONS") {
				response = op->createResponseFromRequest(request, 200);
				belle_sip_server_transaction_send_response(serverTransaction, response);
			} else if (method == "CANCEL") {
				auto lastTransaction = belle_sip_dialog_get_last_transaction(op->mDialog);
				if (!lastTransaction || !isAPendingIncomingInviteTransaction(lastTransaction) ) {
					// Call leg does not exist because 200ok already sent
					belle_sip_server_transaction_send_response(serverTransaction, op->createResponseFromRequest(request, 481));
				} else {
					// CANCEL on re-INVITE for which a 200ok has not been sent yet
					belle_sip_server_transaction_send_response(serverTransaction, op->createResponseFromRequest(request, 200));
					belle_sip_server_transaction_send_response(
						BELLE_SIP_SERVER_TRANSACTION(lastTransaction),
						op->createResponseFromRequest(belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(lastTransaction)), 487)
					);
				}
			} else if (method == "MESSAGE") {
				op->processIncomingMessage(event);
			} else {
				lError() << "Unexpected method [" << method << "] for dialog [" << op->mDialog << "]";
				unsupportedMethod(serverTransaction, request);
			}
			break;
		default:
			lError() << "Unexpected dialog state [" << belle_sip_dialog_state_to_string(dialogState) << "]";
			break;
	}

	if (serverTransaction)
		belle_sip_object_unref(serverTransaction);
	if (dropOp)
		op->release();
}

void SalCallOp::setCallAsReleased (SalCallOp *op) {
	op->setReleased();
}

void SalCallOp::processDialogTerminatedCb (void *userCtx, const belle_sip_dialog_terminated_event_t *event) {
	auto op = static_cast<SalCallOp *>(userCtx);
	if (op->mDialog && (op->mDialog == belle_sip_dialog_terminated_event_get_dialog(event))) {
		lInfo() << "Dialog [" << belle_sip_dialog_terminated_event_get_dialog(event) << "] terminated for op [" << op << "]";

		switch(belle_sip_dialog_get_previous_state(op->mDialog)) {
			case BELLE_SIP_DIALOG_EARLY:
			case BELLE_SIP_DIALOG_NULL:
				if ((op->mState != State::Terminated) && (op->mState != State::Terminating)) {
					// This is an early termination due to incorrect response received
					op->mRoot->mCallbacks.call_failure(op);
					op->mState = State::Terminating;
				}
				break;
			case BELLE_SIP_DIALOG_CONFIRMED:
				if ((op->mState != State::Terminated) && (op->mState != State::Terminating)) {
					// This is probably a normal termination from a BYE
					op->mRoot->mCallbacks.call_terminated(op, (op->mDir == Dir::Incoming) ? op->getFrom().c_str() : op->getTo().c_str());
					op->mState = State::Terminating;
				}
				break;
			default:
				break;
		}
		belle_sip_main_loop_do_later(
			belle_sip_stack_get_main_loop(op->mRoot->mStack),
			(belle_sip_callback_t)setCallAsReleased,
			op
		);
	} else {
		lError() << "Dialog unknown for op";
	}
}

void SalCallOp::fillCallbacks () {
	static belle_sip_listener_callbacks_t callOpCallbacks = { 0 };
	if (!callOpCallbacks.process_response_event){
		callOpCallbacks.process_io_error = processIoErrorCb;
		callOpCallbacks.process_response_event = processResponseCb;
		callOpCallbacks.process_timeout = processTimeoutCb;
		callOpCallbacks.process_transaction_terminated = processTransactionTerminatedCb;
		callOpCallbacks.process_request_event = processRequestEventCb;
		callOpCallbacks.process_dialog_terminated = processDialogTerminatedCb;
	}
	mCallbacks = &callOpCallbacks;
}

int SalCallOp::call (const string &from, const string &to, const string &subject) {
	mDir = Dir::Outgoing;
	setFrom(from);
	setTo(to);

	lInfo() << "[" << from << "] calling [" << to << "] on op [" << this << "]";

	belle_sip_request_t *invite = buildRequest("INVITE");
	if (!invite) // Can happen if the op has an invalid address
		return -1;

	fillInvite(invite);
	if (!subject.empty())
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(invite), belle_sip_header_create("Subject", subject.c_str()));

	if (mReplaces)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(invite), BELLE_SIP_HEADER(mReplaces));
	if (mReferredBy)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(invite), BELLE_SIP_HEADER(mReferredBy));

	return sendRequest(invite);
}

int SalCallOp::notifyRinging (bool earlyMedia) {
	int statusCode = earlyMedia ? 183 : 180;
	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(mPendingServerTransaction));
	belle_sip_response_t *ringingResponse = createResponseFromRequest(request, statusCode);

	if (earlyMedia)
		handleOfferAnswerResponse(ringingResponse);

	const char *tags = nullptr;
	auto requireHeader = belle_sip_message_get_header(BELLE_SIP_MESSAGE(request), "Require");
	if (requireHeader)
		tags = belle_sip_header_get_unparsed_value(requireHeader);
	// If client requires 100rel, then add necessary stuff
	if (tags && (strstr(tags, "100rel") != 0)) {
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(ringingResponse), belle_sip_header_create("Require", "100rel"));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(ringingResponse), belle_sip_header_create("RSeq", "1"));
	}

#ifndef SAL_OP_CALL_FORCE_CONTACT_IN_RINGING
	if (tags && (strstr(tags, "100rel") != 0))
#endif
	{
		auto contact = reinterpret_cast<const belle_sip_header_address_t *>(getContactAddress());
		belle_sip_header_contact_t *contactHeader;
		if (contact && (contactHeader = belle_sip_header_contact_create(contact)))
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(ringingResponse), BELLE_SIP_HEADER(contactHeader));
	}
	belle_sip_server_transaction_send_response(mPendingServerTransaction, ringingResponse);
	return 0;
}

int SalCallOp::accept () {
	belle_sip_server_transaction_t *transaction = nullptr;

	// First check if an UPDATE transaction need to be accepted
	if (mPendingUpdateServerTransaction) {
		transaction = mPendingUpdateServerTransaction;
	} else if (mPendingServerTransaction) {
		// So it must be an invite/re-invite
		transaction = mPendingServerTransaction;
	} else {
		lError() << "No transaction to accept for op [" << this << "]";
		return -1;
	}

	lInfo() << "Accepting server transaction [" << transaction << "] on op [" << this << "]";

	// Send a 200 OK
	auto response = createResponseFromRequest(belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(transaction)), 200);
	if (!response) {
		lError() << "Failed to build answer for call";
		return -1;
	}
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), BELLE_SIP_HEADER(createAllow(mRoot->mEnableSipUpdate)));
	if (mRoot->mSessionExpires != 0) {
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), belle_sip_header_create("Supported", "timer"));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), belle_sip_header_create( "Session-expires", "600;refresher=uac"));
	}

	auto contactHeader = createContact();
	if (contactHeader)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), BELLE_SIP_HEADER(contactHeader));

	addCustomHeaders(BELLE_SIP_MESSAGE(response));
	handleOfferAnswerResponse(response);
	belle_sip_server_transaction_send_response(transaction, response);
	if (mPendingUpdateServerTransaction) {
		belle_sip_object_unref(mPendingUpdateServerTransaction);
		mPendingUpdateServerTransaction = nullptr;
	}
	if (mState == State::Early)
		mState = State::Active;
	return 0;
}

int SalCallOp::decline (SalReason reason, const string &redirectionUri) {
	belle_sip_header_contact_t *contactHeader = nullptr;
	int status = toSipCode(reason);

	if (reason == SalReasonRedirect) {
		if (!redirectionUri.empty()) {
			if (strstr(redirectionUri.c_str(), "sip:") != 0)
				status = 302;
			else
				status = 380;
			contactHeader = belle_sip_header_contact_new();
			belle_sip_header_address_set_uri(BELLE_SIP_HEADER_ADDRESS(contactHeader), belle_sip_uri_parse(redirectionUri.c_str()));
		} else {
			lError() << "Cannot redirect to null";
		}
	}
	auto transaction = BELLE_SIP_TRANSACTION(mPendingServerTransaction);
	if (!transaction)
		transaction = BELLE_SIP_TRANSACTION(mPendingUpdateServerTransaction);
	if (!transaction) {
		lError() << "SalCallOp::decline(): no pending transaction to decline";
		return -1;
	}
	auto response = createResponseFromRequest(belle_sip_transaction_get_request(transaction), status);
	if (contactHeader)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), BELLE_SIP_HEADER(contactHeader));
	belle_sip_server_transaction_send_response(BELLE_SIP_SERVER_TRANSACTION(transaction), response);
	return 0;
}

belle_sip_header_reason_t *SalCallOp::makeReasonHeader (const SalErrorInfo *info) {
	if (info && (info->reason != SalReasonNone)) {
		auto reasonHeader = BELLE_SIP_HEADER_REASON(belle_sip_header_reason_new());
		belle_sip_header_reason_set_text(reasonHeader, info->status_string);
		belle_sip_header_reason_set_protocol(reasonHeader, info->protocol);
		belle_sip_header_reason_set_cause(reasonHeader, info->protocol_code);
		return reasonHeader;
	}
	return nullptr;
}

int SalCallOp::declineWithErrorInfo (const SalErrorInfo *info, const SalAddress *redirectionAddr) {
	belle_sip_header_contact_t *contactHeader = nullptr;
	belle_sip_header_retry_after_t *retryAfterHeader = nullptr;
	int status = info->protocol_code;

	if (info->reason == SalReasonRedirect) {
		if (redirectionAddr) {
			status = 302;
			contactHeader = belle_sip_header_contact_create(BELLE_SIP_HEADER_ADDRESS(redirectionAddr));
		} else {
			lError() << "Cannot redirect to null";
		}
	}
	auto transaction = BELLE_SIP_TRANSACTION(mPendingServerTransaction);
	if (!transaction)
		transaction = BELLE_SIP_TRANSACTION(mPendingUpdateServerTransaction);
	if (!transaction) {
		lError() << "SalCallOp::declineWithErrorInfo(): no pending transaction to decline";
		return -1;
	}
	auto response = createResponseFromRequest(belle_sip_transaction_get_request(transaction), status);
	auto reasonHeader = makeReasonHeader(info->sub_sei);
	if (info->retry_after > 0)
		retryAfterHeader = belle_sip_header_retry_after_create(info->retry_after);

	if (reasonHeader)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), BELLE_SIP_HEADER(reasonHeader));

	if (contactHeader)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), BELLE_SIP_HEADER(contactHeader));

	if (retryAfterHeader)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), BELLE_SIP_HEADER(retryAfterHeader));
	belle_sip_server_transaction_send_response(BELLE_SIP_SERVER_TRANSACTION(transaction), response);
	return 0;
}

int SalCallOp::update (const string &subject, bool noUserConsent) {
	if (!mDialog) {
		// If the dialog does not exist, this is that we are trying to recover from a connection loss
		// during a very early state of outgoing call initiation (the dialog has not been created yet).
		return call(mFrom, mTo, subject);
	}

	auto state = belle_sip_dialog_get_state(mDialog);
	belle_sip_dialog_enable_pending_trans_checking(mDialog, mRoot->mPendingTransactionChecking);

	// Check for dialog state
	belle_sip_request_t *update = nullptr;
	if (state == BELLE_SIP_DIALOG_CONFIRMED) {
		if (noUserConsent)
			update = belle_sip_dialog_create_request(mDialog, "UPDATE");
		else
			update = belle_sip_dialog_create_request(mDialog, "INVITE");
	} else if (state == BELLE_SIP_DIALOG_EARLY) {
		update = belle_sip_dialog_create_request(mDialog, "UPDATE");
	} else {
		lError() << "Cannot update op [" << this << "] with dialog [" << mDialog << "] in state [" << belle_sip_dialog_state_to_string(state) << "]";
		return -1;
	}
	if (update) {
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(update), belle_sip_header_create("Subject", subject.c_str()));
		fillInvite(update);
		return sendRequest(update);
	}
	// Why did it fail?
	if (belle_sip_dialog_request_pending(mDialog))
		sal_error_info_set(&mErrorInfo, SalReasonRequestPending, "SIP", 491, nullptr, nullptr);
	else
		sal_error_info_set(&mErrorInfo, SalReasonUnknown, "SIP", 500, nullptr, nullptr);
	return -1;
}

int SalCallOp::cancelInvite (const SalErrorInfo *info) {
	lInfo() << "Cancelling INVITE request from [" << getFrom() << "] to [" << getTo() << "]";
	if (!mPendingClientTransaction) {
		lWarning() << "There is no transaction to cancel";
		return -1;
	}

	auto cancel = belle_sip_client_transaction_create_cancel(mPendingClientTransaction);
	if (cancel) {
		if (info && (info->reason != SalReasonNone)) {
			auto reasonHeader = makeReasonHeader(info);
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(cancel), BELLE_SIP_HEADER(reasonHeader));
		}
		sendRequest(cancel);
		return 0;
	} else if (mDialog) {
		belle_sip_dialog_state_t state = belle_sip_dialog_get_state(mDialog);
		// Case where the response received is invalid (could not establish a dialog),
		// but the transaction is not cancellable because already terminated.
		switch (state) {
			case BELLE_SIP_DIALOG_EARLY:
			case BELLE_SIP_DIALOG_NULL:
				// Force kill the dialog
				lWarning() << "op [" << this << "]: force kill of dialog [" << mDialog << "]";
				belle_sip_dialog_delete(mDialog);
				break;
			default:
				break;
		}
	}
	return -1;
}

SalMediaDescription *SalCallOp::getFinalMediaDescription () {
	if (mLocalMedia && mRemoteMedia && !mResult)
		sdpProcess();
	return mResult;
}

int SalCallOp::referTo (belle_sip_header_refer_to_t *referToHeader, belle_sip_header_referred_by_t *referredByHeader) {
	auto request = mDialog ? belle_sip_dialog_create_request(mDialog, "REFER") : buildRequest("REFER");
	if (!request) {
		char *tmp = belle_sip_uri_to_string(belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(referToHeader)));
		lError() << "Cannot refer to [" << tmp << "] for op [" << this << "]";
		belle_sip_free(tmp);
		return -1;
	}
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(referToHeader));
	if (referredByHeader)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(referredByHeader));
	return sendRequest(request);
}

int SalCallOp::refer (const string &referToUri) {
	belle_sip_header_address_t *referredByHeader;
	if (mDialog)
		referredByHeader = BELLE_SIP_HEADER_ADDRESS(belle_sip_object_clone(BELLE_SIP_OBJECT(belle_sip_dialog_get_local_party(mDialog))));
	else
		referredByHeader = BELLE_SIP_HEADER_ADDRESS(getFromAddress());
	auto referToHeader = belle_sip_header_refer_to_create(belle_sip_header_address_parse(referToUri.c_str()));
	return referTo(referToHeader, belle_sip_header_referred_by_create(referredByHeader));
}

int SalCallOp::referWithReplaces (SalCallOp *otherCallOp) {
	// First, build refer to
	auto otherCallDialogState = otherCallOp->mDialog ? belle_sip_dialog_get_state(otherCallOp->mDialog) : BELLE_SIP_DIALOG_NULL;
	if ((otherCallDialogState != BELLE_SIP_DIALOG_CONFIRMED) && (otherCallDialogState != BELLE_SIP_DIALOG_EARLY)) {
		lError() << "Wrong dialog state [" << belle_sip_dialog_state_to_string(otherCallDialogState) << "] for op ["
			<< otherCallOp << "], should be BELLE_SIP_DIALOG_CONFIRMED or BELE_SIP_DIALOG_EARLY";
		return -1;
	}
	auto dialogState = mDialog ? belle_sip_dialog_get_state(mDialog) : BELLE_SIP_DIALOG_NULL;
	if (dialogState != BELLE_SIP_DIALOG_CONFIRMED) {
		lError() << "Wrong dialog state [" << belle_sip_dialog_state_to_string(dialogState) << "] for op ["
			<< this << "], should be BELLE_SIP_DIALOG_CONFIRMED";
		return -1;
	}

	auto referToHeader = belle_sip_header_refer_to_create(belle_sip_dialog_get_remote_party(otherCallOp->mDialog));
	belle_sip_parameters_clean(BELLE_SIP_PARAMETERS(referToHeader));

	// rfc3891
	// ...
	// 4.  User Agent Client Behavior: Sending a Replaces Header
	//
	// A User Agent that wishes to replace a single existing early or
	// confirmed dialog with a new dialog of its own, MAY send the target
	// User Agent an INVITE request containing a Replaces header field.  The
	// User Agent Client (UAC) places the Call-ID, to-tag, and from-tag
	// information for the target dialog in a single Replaces header field
	// and sends the new INVITE to the target.
	const char *fromTag = belle_sip_dialog_get_local_tag(otherCallOp->mDialog);
	const char *toTag = belle_sip_dialog_get_remote_tag(otherCallOp->mDialog);
	auto replacesHeader = belle_sip_header_replaces_create(
		belle_sip_header_call_id_get_call_id(belle_sip_dialog_get_call_id(otherCallOp->mDialog)),
		fromTag,
		toTag
	);
	char *escapedReplaces = belle_sip_header_replaces_value_to_escaped_string(replacesHeader);
	belle_sip_uri_set_header(belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(referToHeader)), "Replaces", escapedReplaces);
	belle_sip_free(escapedReplaces);
	auto referredByHeader = belle_sip_header_referred_by_create(belle_sip_dialog_get_local_party(mDialog));
	belle_sip_parameters_clean(BELLE_SIP_PARAMETERS(referredByHeader));
	return referTo(referToHeader, referredByHeader);
}

int SalCallOp::setReferrer (SalCallOp *referredCall) {
	if (referredCall->mReplaces)
		SalOp::setReplaces(referredCall->mReplaces);
	if (referredCall->mReferredBy)
		setReferredBy(referredCall->mReferredBy);
	return 0;
}

SalCallOp *SalCallOp::getReplaces () const {
	if (!mReplaces)
		return nullptr;

	// rfc3891
	// 3.  User Agent Server Behavior: Receiving a Replaces Header
	//
	// The Replaces header contains information used to match an existing
	// SIP dialog (call-id, to-tag, and from-tag).  Upon receiving an INVITE
	// with a Replaces header, the User Agent (UA) attempts to match this
	// information with a confirmed or early dialog.  The User Agent Server
	// (UAS) matches the to-tag and from-tag parameters as if they were tags
	// present in an incoming request.  In other words, the to-tag parameter
	// is compared to the local tag, and the from-tag parameter is compared
	// to the remote tag.
	auto dialog = belle_sip_provider_find_dialog(
		mRoot->mProvider,
		belle_sip_header_replaces_get_call_id(mReplaces),
		belle_sip_header_replaces_get_to_tag(mReplaces),
		belle_sip_header_replaces_get_from_tag(mReplaces)
	);

	if (dialog)
		return reinterpret_cast<SalCallOp *>(belle_sip_dialog_get_application_data(dialog));
	return nullptr;
}

int SalCallOp::sendDtmf (char dtmf) {
	if (mDialog && ((belle_sip_dialog_get_state(mDialog) == BELLE_SIP_DIALOG_CONFIRMED) || (belle_sip_dialog_get_state(mDialog) == BELLE_SIP_DIALOG_EARLY))) {
		auto request = belle_sip_dialog_create_queued_request(mDialog, "INFO");
		if (request) {
			char body[128] = { 0 };
			snprintf(body, sizeof(body) - 1, "Signal=%c\r\nDuration=250\r\n", dtmf);
			size_t bodylen = strlen(body);
			belle_sip_message_set_body(BELLE_SIP_MESSAGE(request), body, bodylen);
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(belle_sip_header_content_length_create(bodylen)));
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(belle_sip_header_content_type_create("application", "dtmf-relay")));
			sendRequest(request);
		} else {
			lError() << "SalCallOp::sendDtmf(): could not build request";
		}
	} else {
		lError() << "SalCallOp::sendDtmf(): no dialog";
	}
	return 0;
}

int SalCallOp::terminate (const SalErrorInfo *info) {
	SalErrorInfo sei;
	const SalErrorInfo *pSei = nullptr;
	int ret = 0;
	auto dialogState = mDialog ? belle_sip_dialog_get_state(mDialog) : BELLE_SIP_DIALOG_NULL;

	memset(&sei, 0, sizeof(sei));
	if (!info && (dialogState != BELLE_SIP_DIALOG_CONFIRMED) && (mDir == Dir::Incoming)) {
		// The purpose of this line is to set a default SalErrorInfo for declining an incoming call (not yet established of course)
		sal_error_info_set(&sei, SalReasonDeclined, "SIP", 0, nullptr, nullptr);
		pSei = &sei;
	} else {
		pSei = info;
	}
	if ((mState == State::Terminating) || (mState == State::Terminated)) {
		lError() << "Cannot terminate op [" << this << "] in state [" << toString(mState) << "]";
		ret = -1;
		goto end;
	}
	switch (dialogState) {
		case BELLE_SIP_DIALOG_CONFIRMED: {
			auto request = belle_sip_dialog_create_request(mDialog, "BYE");
			if (info && (info->reason != SalReasonNone)) {
				auto reasonHeader = makeReasonHeader(info);
				belle_sip_message_add_header(BELLE_SIP_MESSAGE(request),BELLE_SIP_HEADER(reasonHeader));
			}
			sendRequest(request);
			mState = State::Terminating;
			break;
		}
		case BELLE_SIP_DIALOG_NULL:
			if (mDir == Dir::Incoming) {
				declineWithErrorInfo(pSei, nullptr);
				mState = State::Terminated;
			} else if (mPendingClientTransaction) {
				if (belle_sip_transaction_get_state(BELLE_SIP_TRANSACTION(mPendingClientTransaction)) == BELLE_SIP_TRANSACTION_PROCEEDING) {
					cancellingInvite(pSei);
				} else {
					// Case where the CANCEL cannot be sent because no provisional response was received so far.
					//The Op must be kept for the time of the transaction in case a response is received later.
					//The state is passed to Terminating to remember to terminate later.
					mState = State::Terminating;
					// However, even if the transaction is kept alive, we can stop sending retransmissions to avoid flowing the network with no longer
					//necessary messages and avoid confusion in logs.
					belle_sip_client_transaction_stop_retransmissions(mPendingClientTransaction);
				}
			}
			break;
		case BELLE_SIP_DIALOG_EARLY:
			if (mDir == Dir::Incoming) {
				declineWithErrorInfo(pSei, nullptr);
				mState = State::Terminated;
			} else {
				cancellingInvite(pSei);
			}
			break;
		default:
			lError() << "SalCallOp::terminate() not implemented yet for dialog state [" << belle_sip_dialog_state_to_string(dialogState) << "]";
			ret = -1;
			goto end;
	}

end:
	sal_error_info_reset(&sei);
	return ret;
}

void SalCallOp::sendVfuRequest () {
	string body(
		"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
		"<media_control>"
		"  <vc_primitive>"
		"    <to_encoder>"
		"      <picture_fast_update></picture_fast_update>"
		"    </to_encoder>"
		"  </vc_primitive>"
		"</media_control>"
	);
	auto dialogState = mDialog ? belle_sip_dialog_get_state(mDialog) : BELLE_SIP_DIALOG_NULL; // no dialog = dialog in NULL state
	if (dialogState == BELLE_SIP_DIALOG_CONFIRMED) {
		auto infoRequest = belle_sip_dialog_create_queued_request(mDialog, "INFO");
		int error = true;
		if (infoRequest) {
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(infoRequest), BELLE_SIP_HEADER(belle_sip_header_content_type_create("application", "media_control+xml")));
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(infoRequest), BELLE_SIP_HEADER(belle_sip_header_content_length_create(body.size())));
			belle_sip_message_set_body(BELLE_SIP_MESSAGE(infoRequest), body.c_str(), body.size());
			error = sendRequest(infoRequest);
		}
		if (error)
			lWarning() << "Cannot send vfu request to [" << getTo() << "]";
	} else {
		lWarning() << "Cannot send vfu request to [" << getTo() << "] because dialog [" << mDialog << "] in wrong state ["
			<< belle_sip_dialog_state_to_string(dialogState) << "]";
	}
}

int SalCallOp::sendNotifyForRefer (int code, const string &reason) {
	auto notifyRequest = belle_sip_dialog_create_queued_request(mDialog, "NOTIFY");
	char *sipfrag = belle_sip_strdup_printf("SIP/2.0 %i %s\r\n", code, reason.c_str());
	size_t contentLength = strlen(sipfrag);

	belle_sip_message_add_header(
		BELLE_SIP_MESSAGE(notifyRequest),
		BELLE_SIP_HEADER(belle_sip_header_subscription_state_create(BELLE_SIP_SUBSCRIPTION_STATE_ACTIVE, -1))
	);
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(notifyRequest), belle_sip_header_create("Event", "refer"));
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(notifyRequest), BELLE_SIP_HEADER(belle_sip_header_content_type_create("message", "sipfrag")));
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(notifyRequest), BELLE_SIP_HEADER(belle_sip_header_content_length_create(contentLength)));
	belle_sip_message_assign_body(BELLE_SIP_MESSAGE(notifyRequest), sipfrag, contentLength);
	return sendRequest(notifyRequest);
}

void SalCallOp::notifyLastResponse (SalCallOp *newCallOp) {
	auto clientTransaction = newCallOp->mPendingClientTransaction;
	belle_sip_response_t *response = nullptr;
	if (clientTransaction)
		response = belle_sip_transaction_get_response(BELLE_SIP_TRANSACTION(clientTransaction));
	if (response)
		sendNotifyForRefer(belle_sip_response_get_status_code(response), belle_sip_response_get_reason_phrase(response));
	else
		sendNotifyForRefer(100, "Trying");
}

int SalCallOp::notifyReferState (SalCallOp *newCallOp) {
	if (belle_sip_dialog_get_state(mDialog) == BELLE_SIP_DIALOG_TERMINATED)
		return 0;

	auto state = newCallOp->mDialog ? belle_sip_dialog_get_state(newCallOp->mDialog) : BELLE_SIP_DIALOG_NULL;
	switch (state) {
		case BELLE_SIP_DIALOG_EARLY:
			sendNotifyForRefer(100, "Trying");
			break;
		case BELLE_SIP_DIALOG_CONFIRMED:
			sendNotifyForRefer(200, "Ok");
			break;
		case BELLE_SIP_DIALOG_TERMINATED:
		case BELLE_SIP_DIALOG_NULL:
			notifyLastResponse(newCallOp);
			break;
	}
	return 0;
}

void SalCallOp::setReplaces (const string &callId, const string &fromTag, const string &toTag) {
	auto replacesHeader = belle_sip_header_replaces_create(callId.c_str(), fromTag.c_str(), toTag.c_str());
	SalOp::setReplaces(replacesHeader);
}

void SalCallOp::setSdpHandling (SalOpSDPHandling handling) {
	if (handling != SalOpSDPNormal)
		lInfo() << "Enabling special SDP handling for SalOp [" << this << "]!";
	mSdpHandling = handling;
}

void SalCallOp::processRefer (const belle_sip_request_event_t *event, belle_sip_server_transaction_t *serverTransaction) {
	auto request = belle_sip_request_event_get_request(event);

	lInfo() << "Receiving REFER request on op [" << this << "]";
	auto referToHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_refer_to_t);
	if (referToHeader) {
		auto referToUri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(referToHeader));
		const char *replaces = nullptr;
		if (referToUri)
			replaces = belle_sip_uri_get_header(referToUri," Replaces");
		if (replaces) {
			SalOp::setReplaces(belle_sip_header_replaces_create2(replaces));
			belle_sip_uri_remove_header(referToUri, "Replaces");
		}
		auto referredByHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_referred_by_t);
		if (referredByHeader)
			setReferredBy(referredByHeader);
		auto response = createResponseFromRequest(request, 202);
		belle_sip_server_transaction_send_response(serverTransaction, response);
		mRoot->mCallbacks.call_refer_received(this, reinterpret_cast<SalAddress *>(BELLE_SIP_HEADER_ADDRESS(referToHeader)));
	} else {
		lWarning() << "Cannot do anything with the refer without destination";
		auto response = createResponseFromRequest(request, 400);
		belle_sip_server_transaction_send_response(serverTransaction, response);
	}
}

void SalCallOp::processNotify (const belle_sip_request_event_t *event, belle_sip_server_transaction_t *serverTransaction) {
	lInfo() << "Receiving NOTIFY request on op [" << this << "]";

	auto request = belle_sip_request_event_get_request(event);
	auto eventHeader = belle_sip_message_get_header(BELLE_SIP_MESSAGE(request), "Event");
	auto contentTypeHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_content_type_t);
	const char* body = belle_sip_message_get_body(BELLE_SIP_MESSAGE(request));

	if (eventHeader
		&& (strncasecmp(belle_sip_header_get_unparsed_value(eventHeader), "refer", strlen("refer")) == 0)
		&& contentTypeHeader
		&& (strcmp(belle_sip_header_content_type_get_type(contentTypeHeader), "message") == 0)
		&& (strcmp(belle_sip_header_content_type_get_subtype(contentTypeHeader), "sipfrag") == 0)
		&& body
	) {
		auto sipfrag = BELLE_SIP_RESPONSE(belle_sip_message_parse(body));
		if (sipfrag) {
			int code = belle_sip_response_get_status_code(sipfrag);
			SalReferStatus status = SalReferFailed;
			if (code < 200)
				status = SalReferTrying;
			else if (code < 300)
				status = SalReferSuccess;
			else if (code >= 400)
				status = SalReferFailed;
			belle_sip_object_unref(sipfrag);
			auto response = createResponseFromRequest(request, 200);
			belle_sip_server_transaction_send_response(serverTransaction, response);
			mRoot->mCallbacks.notify_refer(this, status);
		}
	} else {
		lError() << "Notify without sipfrag or not for 'refer' event package, rejecting";
		auto response = createResponseFromRequest(request, 489);
		belle_sip_server_transaction_send_response(serverTransaction, response);
	}
}

int SalCallOp::sendMessage (const Content &content) {
	if (!mDialog)
		return -1;
	auto request = belle_sip_dialog_create_queued_request(mDialog, "MESSAGE");
	prepareMessageRequest(request, content);
	return sendRequest(request);
}

bool SalCallOp::compareOp (const SalCallOp *op2) const {
	return mCallId == op2->mCallId;
}

void SalCallOp::handleOfferAnswerResponse (belle_sip_response_t *response) {
	if (!mLocalMedia) {
		lError() << "You are accepting a call but not defined any media capabilities!";
		return;
	}

	// This is the case where we received an invite without SDP
	if (mSdpOffering) {
		setSdpFromDesc(BELLE_SIP_MESSAGE(response), mLocalMedia);
	} else {
		if (!mSdpAnswer) {
			if (mSdpHandling == SalOpSDPSimulateRemove)
				lWarning() << "Simulating SDP removal in answer for op " << this;
			else
				sdpProcess();
		}

		if (mSdpAnswer) {
			setSdp(BELLE_SIP_MESSAGE(response), mSdpAnswer);
			belle_sip_object_unref(mSdpAnswer);
			mSdpAnswer = nullptr;
		}
	}
}

LINPHONE_END_NAMESPACE
