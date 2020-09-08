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

#include "c-wrapper/internal/c-sal.h"
#include "sal/call-op.h"
#include "sal/message-op.h"
#include "sal/refer-op.h"

#include "linphone/api/c-content.h"
#include "linphone/core.h"
#include "linphone/utils/utils.h"

#include "private.h"
#include "mediastreamer2/mediastream.h"
#include "linphone/lpconfig.h"
#include <bctoolbox/defs.h>

// stat
#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room.h"
#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/client-group-chat-room-p.h"
#include "chat/chat-room/server-group-chat-room-p.h"
#endif
#include "conference/participant.h"
#include "conference/session/call-session-p.h"
#include "conference/session/call-session.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "core/core-p.h"

using namespace std;

using namespace LinphonePrivate;

static void register_failure(SalOp *op);

bool_t check_core_state(LinphoneCore *lc, SalOp *op) {
	if (linphone_core_get_global_state(lc) != LinphoneGlobalOn) {
		ms_warning("Linphone core global state is not on. Please check if the core is started.");
		op->replyMessage(SalReasonServiceUnavailable);
		op->release();
		return FALSE;
	}
	return TRUE;
}

static void call_received(SalCallOp *h) {
	LinphoneCore *lc = reinterpret_cast<LinphoneCore *>(h->getSal()->getUserPointer());

	if (!check_core_state(lc, h))
		return;

	/* Look if this INVITE is for a call that has already been notified but broken because of network failure */
	if (L_GET_PRIVATE_FROM_C_OBJECT(lc)->inviteReplacesABrokenCall(h))
		return;

	LinphoneAddress *fromAddr = nullptr;
	const char *pAssertedId = sal_custom_header_find(h->getRecvCustomHeaders(), "P-Asserted-Identity");
	/* In some situation, better to trust the network rather than the UAC */
	if (linphone_config_get_int(linphone_core_get_config(lc), "sip", "call_logs_use_asserted_id_instead_of_from", 0)) {
		if (pAssertedId) {
			LinphoneAddress *pAssertedIdAddr = linphone_address_new(pAssertedId);
			if (pAssertedIdAddr) {
				ms_message("Using P-Asserted-Identity [%s] instead of from [%s] for op [%p]", pAssertedId, h->getFrom().c_str(), h);
				fromAddr = pAssertedIdAddr;
			} else{
				ms_warning("Unsupported P-Asserted-Identity header for op [%p] ", h);
			}

		} else{
			ms_warning("No P-Asserted-Identity header found so cannot use it for op [%p] instead of from", h);
		}
	}

	if (!fromAddr)
		fromAddr = linphone_address_new(h->getFrom().c_str());
	LinphoneAddress *toAddr = linphone_address_new(h->getTo().c_str());

	if (_linphone_core_is_conference_creation(lc, toAddr)) {
#ifdef HAVE_ADVANCED_IM
		linphone_address_unref(toAddr);
		linphone_address_unref(fromAddr);
		if (sal_address_has_param(h->getRemoteContactAddress(), "text")) {
			string oneToOneChatRoom = L_C_TO_STRING(sal_custom_header_find(h->getRecvCustomHeaders(), "One-To-One-Chat-Room"));
			if (oneToOneChatRoom == "true") {
				bool_t oneToOneChatRoomEnabled = linphone_config_get_bool(linphone_core_get_config(lc), "misc", "enable_one_to_one_chat_room", TRUE);
				if (!oneToOneChatRoomEnabled) {
					h->decline(SalReasonNotAcceptable);
					h->release();
					return;
				}
				IdentityAddress from(h->getFrom());
				list<IdentityAddress> identAddresses = ServerGroupChatRoom::parseResourceLists(h->getRemoteBody());
				if (identAddresses.size() != 1) {
					h->decline(SalReasonNotAcceptable);
					h->release();
					return;
				}
				const char *endToEndEncryptedStr = sal_custom_header_find(h->getRecvCustomHeaders(), "End-To-End-Encrypted");
				bool encrypted = endToEndEncryptedStr && strcmp(endToEndEncryptedStr, "true") == 0;

				IdentityAddress confAddr = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb->findOneToOneConferenceChatRoomAddress(from, identAddresses.front(), encrypted);
				if (confAddr.isValid()) {
					shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(ConferenceId(confAddr, confAddr));
					L_GET_PRIVATE(static_pointer_cast<ServerGroupChatRoom>(chatRoom))->confirmRecreation(h);
					return;
				}
			}
			_linphone_core_create_server_group_chat_room(lc, h);
		}
		// TODO: handle media conference creation if the "text" feature tag is not present
		return;
#else
		ms_warning("Advanced IM such as group chat is disabled!");
		return;
#endif
	}

	if (sal_address_has_param(h->getRemoteContactAddress(), "text")) {
#ifdef HAVE_ADVANCED_IM
		linphone_address_unref(toAddr);
		linphone_address_unref(fromAddr);
		if (linphone_core_conference_server_enabled(lc)) {
			shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(
				ConferenceId(IdentityAddress(h->getTo()), IdentityAddress(h->getTo()))
			);
			if (chatRoom) {
				L_GET_PRIVATE(static_pointer_cast<ServerGroupChatRoom>(chatRoom))->confirmJoining(h);
			} else {
				//invite is for an unknown chatroom
				h->decline(SalReasonNotFound);
				h->release();
			}
		} else {
			shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(
				ConferenceId(IdentityAddress(h->getFrom()), IdentityAddress(h->getTo()))
			);
			if (chatRoom && chatRoom->getCapabilities() & ChatRoom::Capabilities::Basic) {
				lError() << "Invalid basic chat room found. It should have been a ClientGroupChatRoom... Recreating it...";
				chatRoom->deleteFromDb();
				chatRoom.reset();
			}
			if (!chatRoom) {
				string endToEndEncrypted = L_C_TO_STRING(sal_custom_header_find(h->getRecvCustomHeaders(), "End-To-End-Encrypted"));
				chatRoom = L_GET_PRIVATE_FROM_C_OBJECT(lc)->createClientGroupChatRoom(
					h->getSubject(),
					ConferenceId(IdentityAddress(h->getRemoteContact()), IdentityAddress(h->getTo())),
					h->getRemoteBody(),
					endToEndEncrypted == "true"
				);
			}

			const char *oneToOneChatRoomStr = sal_custom_header_find(h->getRecvCustomHeaders(), "One-To-One-Chat-Room");
			if (oneToOneChatRoomStr && (strcmp(oneToOneChatRoomStr, "true") == 0))
				L_GET_PRIVATE(static_pointer_cast<ClientGroupChatRoom>(chatRoom))->addOneToOneCapability();
			L_GET_PRIVATE(static_pointer_cast<ClientGroupChatRoom>(chatRoom))->confirmJoining(h);
		}
		return;
#else
		ms_warning("Advanced IM such as group chat is disabled!");
		return;
#endif
	} else {
		// TODO: handle media conference joining if the "text" feature tag is not present
	}

	/* First check if we can answer successfully to this invite */
	LinphonePresenceActivity *activity = nullptr;
	if ((linphone_presence_model_get_basic_status(lc->presence_model) == LinphonePresenceBasicStatusClosed)
		&& (activity = linphone_presence_model_get_activity(lc->presence_model))) {
		char *altContact = nullptr;
		switch (linphone_presence_activity_get_type(activity)) {
			case LinphonePresenceActivityPermanentAbsence:
				altContact = linphone_presence_model_get_contact(lc->presence_model);
				if (altContact) {
					SalErrorInfo sei;
					memset(&sei, 0, sizeof(sei));
					sal_error_info_set(&sei, SalReasonRedirect, "SIP", 0, nullptr, nullptr);
					SalAddress *altAddr = sal_address_new(altContact);
					h->declineWithErrorInfo(&sei, altAddr);
					ms_free(altContact);
					sal_address_unref(altAddr);
					LinphoneErrorInfo *ei = linphone_error_info_new();
					linphone_error_info_set(ei, nullptr, LinphoneReasonMovedPermanently, 302, "Moved permanently", nullptr);
					linphone_core_report_early_failed_call(lc, LinphoneCallIncoming, fromAddr, toAddr, ei, L_STRING_TO_C(h->getCallId()));
					h->release();
					sal_error_info_reset(&sei);
					return;
				}
				break;
			default:
				/* Nothing special to be done */
				break;
		}
	}

	if (!L_GET_PRIVATE_FROM_C_OBJECT(lc)->canWeAddCall()) { /* Busy */
		h->decline(SalReasonBusy);
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, nullptr, LinphoneReasonBusy, 486, "Busy - too many calls", nullptr);
		linphone_core_report_early_failed_call(lc, LinphoneCallIncoming, fromAddr, toAddr, ei, L_STRING_TO_C(h->getCallId()));
		h->release();
		return;
	}

	if (linphone_config_get_int(linphone_core_get_config(lc), "sip", "reject_duplicated_calls", 1)){
		/* Check if I'm the caller */
		LinphoneAddress *fromAddressToSearchIfMe = nullptr;
		if (h->getPrivacy() == SalPrivacyNone)
			fromAddressToSearchIfMe = linphone_address_clone(fromAddr);
		else if (pAssertedId)
			fromAddressToSearchIfMe = linphone_address_new(pAssertedId);
		else
			ms_warning("Hidden from identity, don't know if it's me");
		if (fromAddressToSearchIfMe && L_GET_PRIVATE_FROM_C_OBJECT(lc)->isAlreadyInCallWithAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(fromAddressToSearchIfMe))) {
			char *addr = linphone_address_as_string(fromAddr);
			ms_warning("Receiving a call while one with same address [%s] is initiated, refusing this one with busy message", addr);
			h->decline(SalReasonBusy);
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, nullptr, LinphoneReasonBusy, 486, "Busy - duplicated call", nullptr);
			linphone_core_report_early_failed_call(lc, LinphoneCallIncoming, fromAddr, toAddr, ei, L_STRING_TO_C(h->getCallId()));
			h->release();
			linphone_address_unref(fromAddressToSearchIfMe);
			ms_free(addr);
			return;
		}
		if (fromAddressToSearchIfMe)
			linphone_address_unref(fromAddressToSearchIfMe);
	}

	LinphoneCall *call = linphone_core_get_call_by_callid(lc, h->getCallId().c_str());
	if (call) {
		// There is already a call created when PushIncomingReceived
		linphone_call_configure(call, fromAddr, toAddr, h);
	} else {
		call = linphone_call_new_incoming(lc, fromAddr, toAddr, h);
	}
	linphone_address_unref(fromAddr);
	linphone_address_unref(toAddr);
	LinphonePrivate::Call::toCpp(call)->startIncomingNotification();
}

static void call_rejected(SalCallOp *h){
	LinphoneCore *lc = reinterpret_cast<LinphoneCore *>(h->getSal()->getUserPointer());
	LinphoneErrorInfo *ei = linphone_error_info_new();
	linphone_error_info_from_sal_op(ei, h);
	linphone_core_report_early_failed_call(lc, LinphoneCallIncoming, linphone_address_new(h->getFrom().c_str()), linphone_address_new(h->getTo().c_str()), ei, L_STRING_TO_C(h->getCallId()));
}

static void call_ringing(SalOp *h) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(h->getUserPointer());
	if (!session) return;
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->remoteRinging();
}

/*
 * could be reach :
 *  - when the call is accepted
 *  - when a request is accepted (pause, resume)
 */
static void call_accepted(SalOp *op) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_accepted: CallSession no longer exists");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->accepted();
}

static void call_refreshed(SalOp *op) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_updating: CallSession no longer exists");
		return;
	}
	auto sessionRef = session->getSharedFromThis();

	L_GET_PRIVATE(sessionRef)->setState(CallSession::State::UpdatedByRemote, "Session refresh");
	L_GET_PRIVATE(sessionRef)->setState(CallSession::State::StreamsRunning, "Session refresh");
}

/* Used to set the CallSession state to Updating */
static void call_refreshing(SalOp *op) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_updating: CallSession no longer exists");
		return;
	}
	auto sessionRef = session->getSharedFromThis();

	L_GET_PRIVATE(sessionRef)->setState(CallSession::State::Updating, "Session refreshing");
}

/* this callback is called when an incoming re-INVITE/ SIP UPDATE modifies the session*/
static void call_updating(SalOp *op, bool_t is_update) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_updating: CallSession no longer exists");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->updating(!!is_update);
}


static void call_ack_received(SalOp *op, SalCustomHeader *ack) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_ack_received(): no CallSession for which an ack is expected");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->ackReceived(reinterpret_cast<LinphoneHeaders *>(ack));
}


static void call_ack_being_sent(SalOp *op, SalCustomHeader *ack) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_ack_being_sent(): no CallSession for which an ack is supposed to be sent");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->ackBeingSent(reinterpret_cast<LinphoneHeaders *>(ack));
}

static void call_terminated(SalOp *op, const char *from) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session)
		return;
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->terminated();
}

static void call_failure(SalOp *op) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("Failure reported on already terminated CallSession");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->failure();
}

static void call_released(SalOp *op) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		/* We can get here when the core manages call at Sal level without creating a Call object. Typicially,
		 * when declining an incoming call with busy because maximum number of calls is reached. */
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->setState(LinphonePrivate::CallSession::State::Released, "Call released");
}

static void call_cancel_done(SalOp *op) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("Cancel done reported on already terminated CallSession");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->cancelDone();
}

static void auth_failure(SalOp *op, SalAuthInfo* info) {
	LinphoneCore *lc = reinterpret_cast<LinphoneCore *>(op->getSal()->getUserPointer());
	LinphoneAuthInfo *ai = NULL;

	if (info != NULL) {
		ai = (LinphoneAuthInfo*)_linphone_core_find_auth_info(lc, info->realm, info->username, info->domain, info->algorithm, TRUE);
		if (ai){
			LinphoneAuthMethod method = info->mode == SalAuthModeHttpDigest ? LinphoneAuthHttpDigest : LinphoneAuthTls;
			LinphoneAuthInfo *auth_info = linphone_core_create_auth_info(lc, info->username, NULL, NULL, NULL, info->realm, info->domain);
			ms_message("%s/%s/%s/%s authentication fails.", info->realm, info->username, info->domain, info->mode == SalAuthModeHttpDigest ? "HttpDigest" : "Tls");
			if (method == LinphoneAuthHttpDigest){
				/*ask again for password if auth info was already supplied but apparently not working*/
				L_GET_PRIVATE_FROM_C_OBJECT(lc)->getAuthStack().pushAuthRequested(AuthInfo::toCpp(ai)->getSharedFromThis());
			}else{
				linphone_core_notify_authentication_requested(lc, auth_info, method);
				// Deprecated
				linphone_core_notify_auth_info_requested(lc, info->realm, info->username, info->domain);
			}
			linphone_auth_info_unref(auth_info);
		}
	}
}

static void register_success(SalOp *op, bool_t registered){
	LinphoneProxyConfig *cfg=(LinphoneProxyConfig *)op->getUserPointer();
	if (!cfg){
		ms_message("Registration success for deleted proxy config, ignored");
		return;
	}
	linphone_proxy_config_set_state(cfg, registered ? LinphoneRegistrationOk : LinphoneRegistrationCleared ,
									registered ? "Registration successful" : "Unregistration done");
}

static void register_failure(SalOp *op){
	LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)op->getUserPointer();
	const SalErrorInfo *ei=op->getErrorInfo();
	const char *details=ei->full_string;

	if (cfg==NULL){
		ms_warning("Registration failed for unknown proxy config.");
		return ;
	}
	if (details==NULL)
		details="no response timeout";

	if ((ei->reason == SalReasonServiceUnavailable || ei->reason == SalReasonIOError)
			&& linphone_proxy_config_get_state(cfg) == LinphoneRegistrationOk) {
		linphone_proxy_config_set_state(cfg,LinphoneRegistrationProgress,"Service unavailable, retrying");
	} else if (ei->protocol_code == 401 || ei->protocol_code == 407){
		/* Do nothing. There will be an auth_requested() callback. If the callback doesn't provide an AuthInfo, then
		 * the proxy config will transition to the failed state.*/
	} else {
		linphone_proxy_config_set_state(cfg,LinphoneRegistrationFailed, details);
	}
	if (cfg->presence_publish_event){
		/*prevent publish to be sent now until registration gets successful*/
		linphone_event_terminate(cfg->presence_publish_event);
		cfg->presence_publish_event=NULL;
		cfg->send_publish=cfg->publish;
	}
}

static void vfu_request(SalOp *op) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session)
		return;
	auto sessionRef = session->getSharedFromThis();
	auto mediaSessionRef = dynamic_pointer_cast<LinphonePrivate::MediaSession>(sessionRef);
	if (!mediaSessionRef) {
		ms_warning("VFU request but no MediaSession!");
		return;
	}
	L_GET_PRIVATE(mediaSessionRef)->sendVfu();
}

static void dtmf_received(SalOp *op, char dtmf) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session)
		return;
	auto sessionRef = session->getSharedFromThis();
	auto mediaSessionRef = dynamic_pointer_cast<LinphonePrivate::MediaSession>(sessionRef);
	if (!mediaSessionRef) {
		ms_warning("DTMF received but no MediaSession!");
		return;
	}
	L_GET_PRIVATE(mediaSessionRef)->dtmfReceived(dtmf);
}

static void call_refer_received(SalOp *op, const SalAddress *referTo) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	char *addrStr = sal_address_as_string_uri_only(referTo);
	Address referToAddr(addrStr);
	string method;
	if (referToAddr.isValid())
		method = referToAddr.getMethodParam();
	if (session && (method.empty() || (method == "INVITE"))) {
		auto sessionRef = session->getSharedFromThis();
		L_GET_PRIVATE(sessionRef)->referred(referToAddr);
	} else {
		LinphoneCore *lc = reinterpret_cast<LinphoneCore *>(op->getSal()->getUserPointer());
		linphone_core_notify_refer_received(lc, addrStr);
	}
	bctbx_free(addrStr);
}

static void message_received(SalOp *op, const SalMessage *msg){
	LinphoneCore *lc=(LinphoneCore *)op->getSal()->getUserPointer();

	if (!check_core_state(lc, op))
		return;

	LinphoneCall *call=(LinphoneCall*)op->getUserPointer();
	LinphoneReason reason = lc->chat_deny_code;
	if (reason == LinphoneReasonNone) {
		reason = linphone_core_message_received(lc, op, msg);
	}
	auto messageOp = dynamic_cast<SalMessageOpInterface *>(op);
	messageOp->reply(linphone_reason_to_sal(reason));
	if (!call) op->release();
}

static void parse_presence_requested(SalOp *op, const char *content_type, const char *content_subtype, const char *body, SalPresenceModel **result) {
	linphone_notify_parse_presence(content_type, content_subtype, body, result);
}

static void convert_presence_to_xml_requested(SalOp *op, SalPresenceModel *presence, const char *contact, char **content) {
	/*for backward compatibility because still used by notify. No loguer used for publish*/

	if(linphone_presence_model_get_presentity((LinphonePresenceModel*)presence) == NULL) {
		LinphoneAddress * presentity = linphone_address_new(contact);
		linphone_presence_model_set_presentity((LinphonePresenceModel*)presence, presentity);
		linphone_address_unref(presentity);
	}
	*content = linphone_presence_model_to_xml((LinphonePresenceModel*)presence);
}

static void notify_presence(SalOp *op, SalSubscribeStatus ss, SalPresenceModel *model, const char *msg){
	LinphoneCore *lc=(LinphoneCore *)op->getSal()->getUserPointer();
	linphone_notify_recv(lc,op,ss,model);
}

static void subscribe_presence_received(SalPresenceOp *op, const char *from){
	LinphoneCore *lc=(LinphoneCore *)op->getSal()->getUserPointer();
	if (!check_core_state(lc, op))
		return;
	linphone_subscription_new(lc,op,from);
}

static void subscribe_presence_closed(SalPresenceOp *op, const char *from){
	LinphoneCore *lc=(LinphoneCore *)op->getSal()->getUserPointer();
	linphone_subscription_closed(lc,op);
}

static void ping_reply(SalOp *op) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("Ping reply without CallSession attached...");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->pingReply();
}

static bool_t fill_auth_info_with_client_certificate(LinphoneCore *lc, SalAuthInfo* sai) {
	const char *chain_file = linphone_core_get_tls_cert_path(lc);
	const char *key_file = linphone_core_get_tls_key_path(lc);

	if (key_file && chain_file) {
#ifndef _WIN32
		// optinal check for files
		struct stat st;
		if (stat(key_file, &st)) {
			ms_warning("No client certificate key found in %s", key_file);
			return FALSE;
		}
		if (stat(chain_file, &st)) {
			ms_warning("No client certificate chain found in %s", chain_file);
			return FALSE;
		}
#endif
		sal_certificates_chain_parse_file(sai, chain_file, SAL_CERTIFICATE_RAW_FORMAT_PEM);
		sal_signing_key_parse_file(sai, key_file, "");
	} else if (lc->tls_cert && lc->tls_key) {
		sal_certificates_chain_parse(sai, lc->tls_cert, SAL_CERTIFICATE_RAW_FORMAT_PEM);
		sal_signing_key_parse(sai, lc->tls_key, "");
	}
	return sai->certificates && sai->key;
}

static bool_t fill_auth_info(LinphoneCore *lc, SalAuthInfo* sai) {
	LinphoneAuthInfo *ai = NULL;
	if (sai->mode == SalAuthModeTls) {
		ai = (LinphoneAuthInfo*)_linphone_core_find_tls_auth_info(lc);
	} else {
		ai = (LinphoneAuthInfo*)_linphone_core_find_auth_info(lc,sai->realm,sai->username,sai->domain, sai->algorithm, FALSE);
	}
	if (ai) {
		if (sai->mode == SalAuthModeHttpDigest) {
			sai->userid = ms_strdup(linphone_auth_info_get_userid(ai) ? linphone_auth_info_get_userid(ai) : linphone_auth_info_get_username(ai));
			sai->password = linphone_auth_info_get_passwd(ai)?ms_strdup(linphone_auth_info_get_passwd(ai)) : NULL;
			sai->ha1 = linphone_auth_info_get_ha1(ai) ? ms_strdup(linphone_auth_info_get_ha1(ai)) : NULL;


			AuthStack & as = L_GET_PRIVATE_FROM_C_OBJECT(lc)->getAuthStack();
			/* We have to construct the auth info as it was originally requested in auth_requested() below,
			 * so that the matching is made correctly.
			 */
			as.authFound(AuthInfo::create(sai->username, "", "", "", sai->realm, sai->domain));

		} else if (sai->mode == SalAuthModeTls) {
			if (linphone_auth_info_get_tls_cert(ai) && linphone_auth_info_get_tls_key(ai)) {
				sal_certificates_chain_parse(sai, linphone_auth_info_get_tls_cert(ai), SAL_CERTIFICATE_RAW_FORMAT_PEM);
				sal_signing_key_parse(sai, linphone_auth_info_get_tls_key(ai), "");
			} else if (linphone_auth_info_get_tls_cert_path(ai) && linphone_auth_info_get_tls_key_path(ai)) {
				sal_certificates_chain_parse_file(sai, linphone_auth_info_get_tls_cert_path(ai), SAL_CERTIFICATE_RAW_FORMAT_PEM);
				sal_signing_key_parse_file(sai, linphone_auth_info_get_tls_key_path(ai), "");
			} else {
				fill_auth_info_with_client_certificate(lc, sai);
			}
		}

		if (sai->realm && (!linphone_auth_info_get_realm(ai) || !linphone_auth_info_get_algorithm(ai))) {
			/*if realm was not known, then set it so that ha1 may eventually be calculated and clear text password dropped*/
			linphone_auth_info_set_realm(ai, sai->realm);
			linphone_auth_info_set_algorithm(ai, sai->algorithm);
			linphone_core_write_auth_info(lc, ai);
		}
		return TRUE;
	} else {
		if (sai->mode == SalAuthModeTls) {
			return fill_auth_info_with_client_certificate(lc, sai);
		}
		return FALSE;
	}
}
static bool_t auth_requested(Sal* sal, SalAuthInfo* sai) {
	LinphoneCore *lc = (LinphoneCore *)sal->getUserPointer();
	if (fill_auth_info(lc,sai)) {
		return TRUE;
	} else {
		LinphoneAuthMethod method = sai->mode == SalAuthModeHttpDigest ? LinphoneAuthHttpDigest : LinphoneAuthTls;
		LinphoneAuthInfo *ai = linphone_core_create_auth_info(lc, sai->username, NULL, NULL, NULL, sai->realm, sai->domain);
		linphone_auth_info_set_algorithm(ai, sai->algorithm);

		if (method == LinphoneAuthHttpDigest){
			/* Request app for new authentication information, but later. */
			L_GET_PRIVATE_FROM_C_OBJECT(lc)->getAuthStack().pushAuthRequested(AuthInfo::toCpp(ai)->getSharedFromThis());
		}else{
			linphone_core_notify_authentication_requested(lc, ai, method);
			// Deprecated callback
			linphone_core_notify_auth_info_requested(lc, sai->realm, sai->username, sai->domain);
		}
		linphone_auth_info_unref(ai);

		if (fill_auth_info(lc, sai)) {
			return TRUE;
		}
		return FALSE;
	}
}

static void notify_refer(SalOp *op, SalReferStatus status) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("Receiving notify_refer for unknown CallSession");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	LinphonePrivate::CallSession::State cstate;
	switch (status) {
		case SalReferTrying:
			cstate = LinphonePrivate::CallSession::State::OutgoingProgress;
			break;
		case SalReferSuccess:
			cstate = LinphonePrivate::CallSession::State::Connected;
			break;
		case SalReferFailed:
		default:
			cstate = LinphonePrivate::CallSession::State::Error;
			break;
	}
	L_GET_PRIVATE(sessionRef)->setTransferState(cstate);
	if (cstate == LinphonePrivate::CallSession::State::Connected)
		sessionRef->terminate(); // Automatically terminate the call as the transfer is complete
}

static LinphoneChatMessageState chatStatusSal2Linphone(SalMessageDeliveryStatus status){
	switch(status){
		case SalMessageDeliveryInProgress:
			return LinphoneChatMessageStateInProgress;
		case SalMessageDeliveryDone:
			return LinphoneChatMessageStateDelivered;
		case SalMessageDeliveryFailed:
			return LinphoneChatMessageStateNotDelivered;
	}
	return LinphoneChatMessageStateIdle;
}

static void message_delivery_update(SalOp *op, SalMessageDeliveryStatus status) {
	auto lc = reinterpret_cast<LinphoneCore *>(op->getSal()->getUserPointer());
	if (linphone_core_get_global_state(lc) != LinphoneGlobalOn) {
		static_cast<SalMessageOp *>(op)->reply(SalReasonDeclined);
		return;
	}

	LinphonePrivate::ChatMessage *msg = reinterpret_cast<LinphonePrivate::ChatMessage *>(op->getUserPointer());
	if (!msg)
		return; // Do not handle delivery status for isComposing messages.

	// Check that the message does not belong to an already destroyed chat room - if so, do not invoke callbacks
	if (msg->getChatRoom())
		L_GET_PRIVATE(msg)->setState((LinphonePrivate::ChatMessage::State)chatStatusSal2Linphone(status));
}

static void info_received(SalOp *op, SalBodyHandler *body_handler) {
	LinphonePrivate::CallSession *session = reinterpret_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session)
		return;
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->infoReceived(body_handler);
}

static void subscribe_response(SalOp *op, SalSubscribeStatus status, int will_retry){
	LinphoneEvent *lev=(LinphoneEvent*)op->getUserPointer();
	LinphoneCore *lc=(LinphoneCore *)op->getSal()->getUserPointer();

	if (lev==NULL) return;

	if (status==SalSubscribeActive){
		linphone_event_set_state(lev,LinphoneSubscriptionActive);
	}else if (status==SalSubscribePending){
		linphone_event_set_state(lev,LinphoneSubscriptionPending);
	}else{
		if (will_retry && linphone_core_get_global_state(lc) != LinphoneGlobalShutdown ){
			linphone_event_set_state(lev,LinphoneSubscriptionOutgoingProgress);
		}
		else linphone_event_set_state(lev,LinphoneSubscriptionError);
	}
}

static void notify(SalSubscribeOp *op, SalSubscribeStatus st, const char *eventname, SalBodyHandler *body_handler){
	LinphoneEvent *lev=(LinphoneEvent*)op->getUserPointer();
	LinphoneCore *lc=(LinphoneCore *)op->getSal()->getUserPointer();
	bool_t out_of_dialog = (lev==NULL);
	if (out_of_dialog) {
		/*out of dialog notify */
		lev = linphone_event_new_with_out_of_dialog_op(lc,op,LinphoneSubscriptionOutgoing,eventname);
	}
	{
		LinphoneContent *ct = linphone_content_from_sal_body_handler(body_handler);
		if (ct) {
			linphone_core_notify_notify_received(lc,lev,eventname,ct);
			linphone_content_unref(ct);
		}
	}
	if (out_of_dialog){
		/*out of dialog NOTIFY do not create an implicit subscription*/
		linphone_event_set_state(lev, LinphoneSubscriptionTerminated);
	}else if (st!=SalSubscribeNone){
		linphone_event_set_state(lev,linphone_subscription_state_from_sal(st));
	}
}

static void subscribe_received(SalSubscribeOp *op, const char *eventname, const SalBodyHandler *body_handler){
	LinphoneEvent *lev=(LinphoneEvent*)op->getUserPointer();
	LinphoneCore *lc=(LinphoneCore *)op->getSal()->getUserPointer();

	if (!check_core_state(lc, op))
		return;

	if (lev==NULL) {
		lev=linphone_event_new_with_op(lc,op,LinphoneSubscriptionIncoming,eventname);
		linphone_event_set_state(lev,LinphoneSubscriptionIncomingReceived);
		LinphoneContent *ct = linphone_content_from_sal_body_handler(body_handler);
		linphone_core_notify_subscribe_received(lc,lev,eventname,ct);
		if (ct)
			linphone_content_unref(ct);
	} else {
		/*subscribe refresh, unhandled*/
	}

}

static void incoming_subscribe_closed(SalOp *op){
	LinphoneEvent *lev=(LinphoneEvent*)op->getUserPointer();

	linphone_event_set_state(lev,LinphoneSubscriptionTerminated);
}

static void on_publish_response(SalOp* op){
	LinphoneEvent *lev=(LinphoneEvent*)op->getUserPointer();
	const SalErrorInfo *ei=op->getErrorInfo();

	if (lev==NULL) return;
	if (ei->reason==SalReasonNone){
		if (!lev->terminating)
			linphone_event_set_publish_state(lev,LinphonePublishOk);
		else
			linphone_event_set_publish_state(lev,LinphonePublishCleared);
	}else{
		if (lev->publish_state==LinphonePublishOk){
			linphone_event_set_publish_state(lev,LinphonePublishProgress);
		}else{
			linphone_event_set_publish_state(lev,LinphonePublishError);
		}
	}
}


static void on_expire(SalOp *op){
	LinphoneEvent *lev=(LinphoneEvent*)op->getUserPointer();

	if (lev==NULL) return;

	if (linphone_event_get_publish_state(lev)==LinphonePublishOk){
		linphone_event_set_publish_state(lev,LinphonePublishExpiring);
	}else if (linphone_event_get_subscription_state(lev)==LinphoneSubscriptionActive){
		linphone_event_set_state(lev,LinphoneSubscriptionExpiring);
	}
}

static void on_notify_response(SalOp *op){
	LinphoneEvent *lev=(LinphoneEvent*)op->getUserPointer();
	if (!lev)
		return;

	if (lev->is_out_of_dialog_op) {
		switch (linphone_event_get_subscription_state(lev)) {
			case LinphoneSubscriptionIncomingReceived:
				if (op->getErrorInfo()->reason == SalReasonNone)
					linphone_event_set_state(lev, LinphoneSubscriptionTerminated);
				else
					linphone_event_set_state(lev, LinphoneSubscriptionError);
				break;
			default:
				ms_warning("Unhandled on_notify_response() case %s",
					linphone_subscription_state_to_string(linphone_event_get_subscription_state(lev)));
				break;
		}
	} else {
		ms_warning("on_notify_response in dialog");
		_linphone_event_notify_notify_response(lev);
	}
}

static void refer_received(SalOp *op, const SalAddress *refer_to){
	if (sal_address_has_param(refer_to, "text")) {
		char *refer_uri = sal_address_as_string(refer_to);
		LinphonePrivate::Address addr(refer_uri);
		bctbx_free(refer_uri);
		if (addr.isValid()) {
			LinphoneCore *lc = reinterpret_cast<LinphoneCore *>(op->getSal()->getUserPointer());

			if (linphone_core_get_global_state(lc) != LinphoneGlobalOn) {
				static_cast<SalReferOp *>(op)->reply(SalReasonDeclined);
				return;
			}

			if (addr.hasUriParam("method") && (addr.getUriParamValue("method") == "BYE")) {
				if (linphone_core_conference_server_enabled(lc)) {
					// Removal of a participant at the server side
					shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(
						ConferenceId(IdentityAddress(op->getTo()), IdentityAddress(op->getTo()))
					);
					if (chatRoom) {
						std::shared_ptr<Participant> participant = chatRoom->findParticipant(IdentityAddress(op->getFrom()));
						if (!participant || !participant->isAdmin()) {
							static_cast<SalReferOp *>(op)->reply(SalReasonDeclined);
							return;
						}
						participant = chatRoom->findParticipant(addr);
						if (participant)
							chatRoom->removeParticipant(participant);
						static_cast<SalReferOp *>(op)->reply(SalReasonNone);
						return;
					}
				} else {
					// The server asks a participant to leave a chat room
					LinphoneChatRoom *cr = L_GET_C_BACK_PTR(
						L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(ConferenceId(addr, IdentityAddress(op->getTo())))
					);
					if (cr) {
						L_GET_CPP_PTR_FROM_C_OBJECT(cr)->leave();
						static_cast<SalReferOp *>(op)->reply(SalReasonNone);
						return;
					}
					static_cast<SalReferOp *>(op)->reply(SalReasonDeclined);
				}
			} else {
				if (linphone_core_conference_server_enabled(lc)) {
#ifdef HAVE_ADVANCED_IM
					shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(
						ConferenceId(IdentityAddress(op->getTo()), IdentityAddress(op->getTo()))
					);
					LinphoneChatRoom *cr = L_GET_C_BACK_PTR(chatRoom);
					if (cr) {
						Address fromAddr(op->getFrom());
						shared_ptr<Participant> participant = chatRoom->findParticipant(fromAddr);
						if (!participant || !participant->isAdmin()) {
							static_cast<SalReferOp *>(op)->reply(SalReasonForbidden);
							return;
						}
						if (addr.hasParam("admin")) {
							participant = chatRoom->findParticipant(addr);
							if (participant) {
								bool value = Utils::stob(addr.getParamValue("admin"));
								chatRoom->setParticipantAdminStatus(participant, value);
								static_cast<SalReferOp *>(op)->reply(SalReasonNone);
								return;
							}
						} else {
							participant = L_GET_PRIVATE(static_pointer_cast<ServerGroupChatRoom>(chatRoom))->findAuthorizedParticipant(addr);
							if (!participant) {
								bool ret = static_pointer_cast<ServerGroupChatRoom>(chatRoom)->addParticipant(
									IdentityAddress(addr), nullptr, false);
								static_cast<SalReferOp *>(op)->reply(ret ? SalReasonNone : SalReasonNotAcceptable);
								return;
							}
						}
					}
#else
					ms_warning("Advanced IM such as group chat is disabled!");
#endif
				}
			}
		}
	}
	static_cast<SalReferOp *>(op)->reply(SalReasonDeclined);
}

Sal::Callbacks linphone_sal_callbacks={
	call_received,
	call_rejected,
	call_ringing,
	call_accepted,
	call_ack_received,
	call_ack_being_sent,
	call_updating,
	call_refreshed,
	call_refreshing,
	call_terminated,
	call_failure,
	call_released,
	call_cancel_done,
	call_refer_received,
	auth_failure,
	register_success,
	register_failure,
	vfu_request,
	dtmf_received,
	message_received,
	message_delivery_update,
	notify_refer,
	subscribe_received,
	incoming_subscribe_closed,
	subscribe_response,
	notify,
	subscribe_presence_received,
	subscribe_presence_closed,
	parse_presence_requested,
	convert_presence_to_xml_requested,
	notify_presence,
	ping_reply,
	auth_requested,
	info_received,
	on_publish_response,
	on_expire,
	on_notify_response,
	refer_received,
};
