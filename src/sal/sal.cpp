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

#include <algorithm>

#include "sal/sal.h"
#include "sal/call-op.h"
#include "sal/presence-op.h"
#include "sal/refer-op.h"
#include "sal/event-op.h"
#include "sal/message-op.h"
#include "bellesip_sal/sal_impl.h"
#include "tester_utils.h"
#include "private.h"

#include "c-wrapper/internal/c-tools.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void Sal::processDialogTerminatedCb (void *sal, const belle_sip_dialog_terminated_event_t *event) {
	auto dialog = belle_sip_dialog_terminated_event_get_dialog(event);
	auto op = static_cast<SalOp *>(belle_sip_dialog_get_application_data(dialog));
	if (op && op->mCallbacks && op->mCallbacks->process_dialog_terminated)
		op->mCallbacks->process_dialog_terminated(op, event);
	else
		lError() << "Sal::processDialogTerminatedCb(): no op found for this dialog [" << dialog << "], ignoring";
}

void Sal::processIoErrorCb (void *userCtx, const belle_sip_io_error_event_t *event) {
	if (BELLE_SIP_OBJECT_IS_INSTANCE_OF(belle_sip_io_error_event_get_source(event), belle_sip_client_transaction_t)) {
		auto client_transaction = BELLE_SIP_CLIENT_TRANSACTION(belle_sip_io_error_event_get_source(event));
		auto op = static_cast<SalOp *>(belle_sip_transaction_get_application_data(BELLE_SIP_TRANSACTION(client_transaction)));
		// Also reset auth count on IO error
		op->mAuthRequests = 0;
		if (op->mCallbacks && op->mCallbacks->process_io_error)
			op->mCallbacks->process_io_error(op, event);
	} else {
		// Nop, because already handled at transaction layer
	}
}

void Sal::processRequestEventCb (void *userCtx, const belle_sip_request_event_t *event) {
	auto sal = static_cast<Sal *>(userCtx);
	SalOp *op = nullptr;
	belle_sip_header_t *evh = nullptr;
	auto request = belle_sip_request_event_get_request(event);
	string method = belle_sip_request_get_method(request);

	auto dialog = belle_sip_request_event_get_dialog(event);
	if (dialog) {
		op = static_cast<SalOp *>(belle_sip_dialog_get_application_data(dialog));
		if (!op && (method == "NOTIFY")) {
			// Special case for dialog created by notify matching subscribe
			auto subscribeTransaction = belle_sip_dialog_get_last_transaction(dialog);
			op = static_cast<SalOp *>(belle_sip_transaction_get_application_data(subscribeTransaction));
		}
		if (!op || (op->mState == SalOp::State::Terminated)) {
			lWarning() << "Receiving request for null or terminated op [" << op << "], ignored";
			return;
		}
	} else {
		// Handle the case where we are receiving a request with to tag but it is not belonging to any dialog
		auto toHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_to_t);
		if (((method == "INVITE") || (method == "NOTIFY")) && belle_sip_header_to_get_tag(toHeader)) {
			lWarning() << "Receiving " << method << " with to-tag but no know dialog here, rejecting";
			auto response = belle_sip_response_create_from_request(request, 481);
			belle_sip_provider_send_response(sal->mProvider, response);
			return;
		// By default (eg. when a to-tag is present), out of dialog ACK are automatically
		// handled in lower layers (belle-sip) but in case it misses, it will be forwarded to us
		} else if ((method == "ACK") && !belle_sip_header_to_get_tag(toHeader)) {
			lWarning() << "Receiving ACK without to-tag but no know dialog here, ignoring";
			return;
		}

		if (method == "INVITE") {
			op = new SalCallOp(sal);
			op->fillCallbacks();
		} else if (((method == "SUBSCRIBE") || (method == "NOTIFY")) && (evh = belle_sip_message_get_header(BELLE_SIP_MESSAGE(request), "Event")) != nullptr) {
			string eventHeaderValue = belle_sip_header_get_unparsed_value(evh);
			if (eventHeaderValue == "presence")
				op = new SalPresenceOp(sal);
			else
				op = new SalSubscribeOp(sal);
			op->fillCallbacks();
		} else if (method == "MESSAGE") {
			op = new SalMessageOp(sal);
			op->fillCallbacks();
		} else if (method == "REFER") {
			op = new SalReferOp(sal);
		} else if (method == "OPTIONS") {
			auto response = belle_sip_response_create_from_request(request, 200);
			belle_sip_provider_send_response(sal->mProvider, response);
			return;
		} else if (method == "INFO") { // INFO out of call dialogs are not allowed
			auto response = belle_sip_response_create_from_request(request, 481);
			belle_sip_provider_send_response(sal->mProvider, response);
			return;
		} else if (method == "BYE") { // Out of dialog BYE
			auto response = belle_sip_response_create_from_request(request, 481);
			belle_sip_provider_send_response(sal->mProvider, response);
			return;
		} else if (method == "CANCEL") { // Out of dialog CANCEL
			auto response = belle_sip_response_create_from_request(request, 481);
			belle_sip_provider_send_response(sal->mProvider, response);
			return;
		} else if (sal->mEnableTestFeatures && (method == "PUBLISH")) { // Out of dialog PUBLISH
			auto response = belle_sip_response_create_from_request(request, 200);
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), belle_sip_header_create("SIP-Etag", "4441929FFFZQOA"));
			belle_sip_provider_send_response(sal->mProvider, response);
			return;
		} else {
			lError() << "Sal::processRequestEventCb(): not implemented yet for method [" << method << "]";
			auto response = belle_sip_response_create_from_request(request, 405);
			belle_sip_message_add_header(
				BELLE_SIP_MESSAGE(response),
				BELLE_SIP_HEADER(belle_sip_header_allow_create("INVITE, CANCEL, ACK, BYE, SUBSCRIBE, NOTIFY, MESSAGE, OPTIONS, INFO"))
			);
			belle_sip_provider_send_response(sal->mProvider, response);
			return;
		}
		op->mDir = SalOp::Dir::Incoming;
	}

	if (!op->mFromAddress) {
		auto fromHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_from_t);
		belle_sip_header_address_t *addressHeader = nullptr;
		auto uri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(fromHeader));
		auto absoluteUri = belle_sip_header_address_get_absolute_uri(BELLE_SIP_HEADER_ADDRESS(fromHeader));
		if (uri) {
			addressHeader = belle_sip_header_address_create(
				belle_sip_header_address_get_displayname(BELLE_SIP_HEADER_ADDRESS(fromHeader)),
				uri
			);
		} else if (absoluteUri) {
			addressHeader = belle_sip_header_address_create2(
				belle_sip_header_address_get_displayname(BELLE_SIP_HEADER_ADDRESS(fromHeader)),
				absoluteUri
			);
		} else {
			lError() << "Cannot find from uri from request [" << request << "]";
		}
		if (addressHeader) {
			op->setFromAddress(reinterpret_cast<SalAddress *>(addressHeader));
			belle_sip_object_unref(addressHeader);
		}
	}

	auto remoteContactHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_contact_t);
	if (remoteContactHeader)
		op->setRemoteContact(belle_sip_header_get_unparsed_value(BELLE_SIP_HEADER(remoteContactHeader)));

	if (!op->mToAddress) {
		auto toHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_to_t);
		belle_sip_header_address_t *addressHeader = nullptr;
		auto uri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(toHeader));
		auto absoluteUri = belle_sip_header_address_get_absolute_uri(BELLE_SIP_HEADER_ADDRESS(toHeader));
		if (uri) {
			addressHeader = belle_sip_header_address_create(
				belle_sip_header_address_get_displayname(BELLE_SIP_HEADER_ADDRESS(toHeader)),
				uri
			);
		} else if (absoluteUri) {
			addressHeader = belle_sip_header_address_create2(
				belle_sip_header_address_get_displayname(BELLE_SIP_HEADER_ADDRESS(toHeader)),
				absoluteUri
			);
		} else {
			lError() << "Cannot find to uri from request [" << request << "]";
		}
		if (addressHeader) {
			op->setToAddress(reinterpret_cast<SalAddress *>(addressHeader));
			belle_sip_object_unref(addressHeader);
		}
	}

	auto subjectHeader = belle_sip_message_get_header(BELLE_SIP_MESSAGE(request), "Subject");
	if (subjectHeader)
		op->setSubject(belle_sip_header_get_unparsed_value(subjectHeader));

	if(!op->mDiversionAddress) {
		auto diversionHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_diversion_t);
		if (diversionHeader) {
			belle_sip_header_address_t *addressHeader = nullptr;
			auto uri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(diversionHeader));
			auto absoluteUri = belle_sip_header_address_get_absolute_uri(BELLE_SIP_HEADER_ADDRESS(diversionHeader));
			if (uri) {
				addressHeader = belle_sip_header_address_create(
					belle_sip_header_address_get_displayname(BELLE_SIP_HEADER_ADDRESS(diversionHeader)),
					uri
				);
			} else if (absoluteUri) {
				addressHeader = belle_sip_header_address_create2(
					belle_sip_header_address_get_displayname(BELLE_SIP_HEADER_ADDRESS(diversionHeader)),
					absoluteUri
				);
			} else {
				lWarning() << "Cannot not find diversion header from request [" << request << "]";
			}
			if (addressHeader) {
				op->setDiversionAddress(reinterpret_cast<SalAddress *>(addressHeader));
				belle_sip_object_unref(addressHeader);
			}
		}
	}

	if (op->mOrigin.empty()) {
		// Set origin uri
		auto originAddressHeader = belle_sip_header_address_create(nullptr, belle_sip_request_extract_origin(request));
		op->setNetworkOriginAddress(reinterpret_cast<SalAddress *>(originAddressHeader));
		belle_sip_object_unref(originAddressHeader);
	}
	if (op->mRemoteUserAgent.empty())
		op->setRemoteUserAgent(BELLE_SIP_MESSAGE(request));

	if (op->mCallId.empty()) {
		op->mCallId = belle_sip_header_call_id_get_call_id(
			BELLE_SIP_HEADER_CALL_ID(belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_call_id_t))
		);
	}

	// It is worth noting that proxies can (and will) remove this header field
	op->setPrivacyFromMessage(BELLE_SIP_MESSAGE(request));

	if (method != "ACK") // The ACK custom header is processed specifically later on
		op->assignRecvHeaders(BELLE_SIP_MESSAGE(request));

	if (op->mCallbacks && op->mCallbacks->process_request_event)
		op->mCallbacks->process_request_event(op, event);
	else
		lError() << "Sal::processRequestEventCb(): not implemented yet";
}

void Sal::processResponseEventCb (void *userCtx, const belle_sip_response_event_t *event) {
	auto response = belle_sip_response_event_get_response(event);
	int responseCode = belle_sip_response_get_status_code(response);

	auto clientTransaction = belle_sip_response_event_get_client_transaction(event);
	if (!clientTransaction) {
		lWarning() << "Discarding stateless response [" << responseCode << "]";
		return;
	}

	auto op = static_cast<SalOp *>(belle_sip_transaction_get_application_data(BELLE_SIP_TRANSACTION(clientTransaction)));
	if (op->mState == SalOp::State::Terminated) {
		lInfo() << "Op [" << op << "] is terminated, nothing to do with this [" << responseCode << "]";
		return;
	}

	// Do it all the time, since we can receive provisional responses from a different instance than the final one
	op->setRemoteUserAgent(BELLE_SIP_MESSAGE(response));

	auto remoteContactHeader = belle_sip_message_get_header_by_type(response, belle_sip_header_contact_t);
	if (remoteContactHeader)
		op->setRemoteContact(belle_sip_header_get_unparsed_value(BELLE_SIP_HEADER(remoteContactHeader)));

	if (op->mCallId.empty()) {
		op->mCallId = belle_sip_header_call_id_get_call_id(
			BELLE_SIP_HEADER_CALL_ID(belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(response), belle_sip_header_call_id_t))
		);
	}

	op->assignRecvHeaders(BELLE_SIP_MESSAGE(response));

	if (op->mCallbacks && op->mCallbacks->process_response_event) {
		// Handle authorization
		auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(clientTransaction));
		string method = belle_sip_request_get_method(request);
		switch (responseCode) {
			case 200:
				break;
			case 401:
			case 407:
				if ((op->mState == SalOp::State::Terminating) && (method != "BYE")) {
					// Only bye are completed
					lInfo() << "Op is in state terminating, nothing else to do";
				} else {
					if (op->mPendingAuthTransaction) {
						belle_sip_object_unref(op->mPendingAuthTransaction);
						op->mPendingAuthTransaction = nullptr;
					}
					if (++op->mAuthRequests > 2) {
						lWarning() << "Auth info cannot be found for op [" << op->getFrom() << "/" << op->getTo() << "] after 2 attempts, giving up";
						op->mRoot->mCallbacks.auth_failure(op, op->mAuthInfo);
						op->mRoot->removePendingAuth(op);
					} else {
						op->mPendingAuthTransaction = BELLE_SIP_CLIENT_TRANSACTION(belle_sip_object_ref(clientTransaction));
						op->processAuthentication();
						return;
					}
				}
				break;
			case 403:
				if (op->mAuthInfo)
					op->mRoot->mCallbacks.auth_failure(op, op->mAuthInfo);
				break;
			case 302:
			case 301:
				op->mAuthRequests = 0;
				if (op->processRedirect() == 0)
					return;
				break;
		}
		if ((responseCode >= 180) && (responseCode != 401) && (responseCode != 407) && (responseCode != 403)) // Not an auth request
			op->mAuthRequests = 0;
		op->mCallbacks->process_response_event(op, event);
	} else {
		lError() << "Unhandled event response [" << event << "]";
	}
}

void Sal::processTimeoutCb (void *userCtx, const belle_sip_timeout_event_t *event) {
	auto clientTransaction = belle_sip_timeout_event_get_client_transaction(event);
	auto op = static_cast<SalOp *>(belle_sip_transaction_get_application_data(BELLE_SIP_TRANSACTION(clientTransaction)));
	if (op && op->mCallbacks && op->mCallbacks->process_timeout)
		op->mCallbacks->process_timeout(op, event);
	else
		lError() << "Unhandled event timeout [" << event << "]";
}

void Sal::processTransactionTerminatedCb (void *userCtx, const belle_sip_transaction_terminated_event_t *event) {
	auto clientTransaction = belle_sip_transaction_terminated_event_get_client_transaction(event);
	auto serverTransaction = belle_sip_transaction_terminated_event_get_server_transaction(event);
	auto transaction = clientTransaction ? BELLE_SIP_TRANSACTION(clientTransaction) : BELLE_SIP_TRANSACTION(serverTransaction);
	auto op = static_cast<SalOp *>(belle_sip_transaction_get_application_data(transaction));

	if (op && op->mCallbacks && op->mCallbacks->process_transaction_terminated)
		op->mCallbacks->process_transaction_terminated(op, event);
	else
		lInfo() << "Unhandled transaction terminated [" << transaction << "]";

	if (op) {
		op->unref(); // Because every transaction ref op
		belle_sip_transaction_set_application_data(transaction, nullptr); // No longer reference something we do not ref to avoid future access of a released op
	}
}

void Sal::processAuthRequestedCb (void *userCtx, belle_sip_auth_event_t *event) {
	auto sal = static_cast<Sal *>(userCtx);
	SalAuthInfo *authInfo = sal_auth_info_create(event);
	sal->mCallbacks.auth_requested(sal, authInfo);
	belle_sip_auth_event_set_passwd(event, (const char *)authInfo->password);
	belle_sip_auth_event_set_ha1(event, (const char *)authInfo->ha1);
	belle_sip_auth_event_set_userid(event, (const char *)authInfo->userid);
	belle_sip_auth_event_set_signing_key(event, (belle_sip_signing_key_t *)authInfo->key);
	belle_sip_auth_event_set_client_certificates_chain(event, (belle_sip_certificates_chain_t *)authInfo->certificates);
	sal_auth_info_delete(authInfo);
}

Sal::Sal (MSFactory *factory) : mFactory(factory) {
	// First create the stack, which initializes the belle-sip object's pool for this thread
	mStack = belle_sip_stack_new(nullptr);

	mUserAgentHeader = belle_sip_header_user_agent_new();
#if defined(PACKAGE_NAME) && defined(LIBLINPHONE_VERSION)
	belle_sip_header_user_agent_add_product(mUserAgentHeader, PACKAGE_NAME "/" LIBLINPHONE_VERSION);
#else
	belle_sip_header_user_agent_add_product(mUserAgentHeader, "Unknown");
#endif
	appendStackStringToUserAgent();
	belle_sip_object_ref(mUserAgentHeader);

	mProvider = belle_sip_stack_create_provider(mStack, nullptr);
	enableNatHelper(true);

	belle_sip_listener_callbacks_t listenerCallbacks = { 0 };
	listenerCallbacks.process_dialog_terminated = processDialogTerminatedCb;
	listenerCallbacks.process_io_error = processIoErrorCb;
	listenerCallbacks.process_request_event = processRequestEventCb;
	listenerCallbacks.process_response_event = processResponseEventCb;
	listenerCallbacks.process_timeout = processTimeoutCb;
	listenerCallbacks.process_transaction_terminated = processTransactionTerminatedCb;
	listenerCallbacks.process_auth_requested = processAuthRequestedCb;
	mListener = belle_sip_listener_create_from_callbacks(&listenerCallbacks, this);
	belle_sip_provider_add_sip_listener(mProvider, mListener);
}

Sal::~Sal () {
	belle_sip_object_unref(mUserAgentHeader);
	belle_sip_object_unref(mProvider);
	belle_sip_object_unref(mStack);
	belle_sip_object_unref(mListener);
	if (mSupportedHeader)
		belle_sip_object_unref(mSupportedHeader);
}

void Sal::setCallbacks (const Callbacks *cbs) {
	memcpy(&mCallbacks, cbs, sizeof(*cbs));
	if (!mCallbacks.call_received)
		mCallbacks.call_received = (OnCallReceivedCb)unimplementedStub;
	if (!mCallbacks.call_ringing)
		mCallbacks.call_ringing = (OnCallRingingCb)unimplementedStub;
	if (!mCallbacks.call_accepted)
		mCallbacks.call_accepted = (OnCallAcceptedCb)unimplementedStub;
	if (!mCallbacks.call_failure)
		mCallbacks.call_failure = (OnCallFailureCb)unimplementedStub;
	if (!mCallbacks.call_terminated)
		mCallbacks.call_terminated = (OnCallTerminatedCb)unimplementedStub;
	if (!mCallbacks.call_released)
		mCallbacks.call_released = (OnCallReleasedCb)unimplementedStub;
	if (!mCallbacks.call_updating)
		mCallbacks.call_updating = (OnCallUpdatingCb)unimplementedStub;
	if (!mCallbacks.auth_failure)
		mCallbacks.auth_failure = (OnAuthFailureCb)unimplementedStub;
	if (!mCallbacks.register_success)
		mCallbacks.register_success = (OnRegisterSuccessCb)unimplementedStub;
	if (!mCallbacks.register_failure)
		mCallbacks.register_failure = (OnRegisterFailureCb)unimplementedStub;
	if (!mCallbacks.dtmf_received)
		mCallbacks.dtmf_received = (OnDtmfReceivedCb)unimplementedStub;
	if (!mCallbacks.notify)
		mCallbacks.notify = (OnNotifyCb)unimplementedStub;
	if (!mCallbacks.subscribe_received)
		mCallbacks.subscribe_received = (OnSubscribeReceivedCb)unimplementedStub;
	if (!mCallbacks.incoming_subscribe_closed)
		mCallbacks.incoming_subscribe_closed = (OnIncomingSubscribeClosedCb)unimplementedStub;
	if (!mCallbacks.parse_presence_requested)
		mCallbacks.parse_presence_requested = (OnParsePresenceRequestedCb)unimplementedStub;
	if (!mCallbacks.convert_presence_to_xml_requested)
		mCallbacks.convert_presence_to_xml_requested = (OnConvertPresenceToXMLRequestedCb)unimplementedStub;
	if (!mCallbacks.notify_presence)
		mCallbacks.notify_presence = (OnNotifyPresenceCb)unimplementedStub;
	if (!mCallbacks.subscribe_presence_received)
		mCallbacks.subscribe_presence_received = (OnSubscribePresenceReceivedCb)unimplementedStub;
	if (!mCallbacks.message_received)
		mCallbacks.message_received = (OnMessageReceivedCb)unimplementedStub;
	if (!mCallbacks.ping_reply)
		mCallbacks.ping_reply = (OnPingReplyCb)unimplementedStub;
	if (!mCallbacks.auth_requested)
		mCallbacks.auth_requested = (OnAuthRequestedCb)unimplementedStub;
	if (!mCallbacks.info_received)
		mCallbacks.info_received = (OnInfoReceivedCb)unimplementedStub;
	if (!mCallbacks.on_publish_response)
		mCallbacks.on_publish_response = (OnPublishResponseCb)unimplementedStub;
	if (!mCallbacks.on_expire)
		mCallbacks.on_expire = (OnExpireCb)unimplementedStub;
}

void Sal::setTlsProperties () {
	auto listeningPoint = belle_sip_provider_get_listening_point(mProvider, "TLS");
	if (!listeningPoint)
		return;

	auto cryptoConfig = belle_tls_crypto_config_new();
	int verifyExceptions = BELLE_TLS_VERIFY_NONE;
	if (!mTlsVerify)
		verifyExceptions = BELLE_TLS_VERIFY_ANY_REASON;
	else if (!mTlsVerifyCn)
		verifyExceptions = BELLE_TLS_VERIFY_CN_MISMATCH;
	belle_tls_crypto_config_set_verify_exceptions(cryptoConfig, verifyExceptions);

	if (!mRootCa.empty())
		belle_tls_crypto_config_set_root_ca(cryptoConfig, mRootCa.c_str());
	if (!mRootCaData.empty())
		belle_tls_crypto_config_set_root_ca_data(cryptoConfig, mRootCaData.c_str());
	if (mSslConfig)
		belle_tls_crypto_config_set_ssl_config(cryptoConfig, mSslConfig);
	if (mTlsPostcheckCb)
		belle_tls_crypto_config_set_postcheck_callback(cryptoConfig, mTlsPostcheckCb, mTlsPostcheckCbData);
	auto tlsListeningPoint = BELLE_SIP_TLS_LISTENING_POINT(listeningPoint);
	belle_sip_tls_listening_point_set_crypto_config(tlsListeningPoint, cryptoConfig);
	belle_sip_object_unref(cryptoConfig);
}

int Sal::addListenPort (SalAddress *addr, bool isTunneled) {
	belle_sip_listening_point_t *listeningPoint;

	if (isTunneled) {
#ifdef TUNNEL_ENABLED
		if (sal_address_get_transport(addr) != SalTransportUDP) {
			lError() << "Tunneled mode is only available for UDP kind of transports";
			return -1;
		}
		listeningPoint = belle_sip_tunnel_listening_point_new(mStack, mTunnelClient);
		if (!listeningPoint) {
			lError() << "Could not create tunnel listening point";
			return -1;
		}
#else
		lError() << "No tunnel support in library";
		return -1;
#endif
	} else {
		listeningPoint = belle_sip_stack_create_listening_point(
			mStack,
			sal_address_get_domain(addr),
			sal_address_get_port(addr),
			sal_transport_to_string(sal_address_get_transport(addr))
		);
	}

	if (!listeningPoint)
		return -1;

	belle_sip_listening_point_set_keep_alive(listeningPoint, (int)mKeepAlive);
	int result = belle_sip_provider_add_listening_point(mProvider, listeningPoint);
	if (sal_address_get_transport(addr) == SalTransportTLS)
		setTlsProperties();
	return result;
}

int Sal::setListenPort (const string &addr, int port, SalTransport tr, bool isTunneled) {
	SalAddress *salAddr = sal_address_new(nullptr);
	sal_address_set_domain(salAddr, L_STRING_TO_C(addr));
	sal_address_set_port(salAddr, port);
	sal_address_set_transport(salAddr, tr);
	int result = addListenPort(salAddr, isTunneled);
	sal_address_unref(salAddr);
	return result;
}

int Sal::getListeningPort (SalTransport tr) {
	const char *tpn = sal_transport_to_string(tr);
	auto listeningPoint = belle_sip_provider_get_listening_point(mProvider, tpn);
	if (listeningPoint)
		return belle_sip_listening_point_get_port(listeningPoint);
	return 0;
}

void Sal::unlistenPorts () {
	auto listeningPoints = belle_sip_provider_get_listening_points(mProvider);
	auto listeningPointsCopy = belle_sip_list_copy(listeningPoints);
	belle_sip_list_for_each2(listeningPointsCopy, (bctbx_list_iterate2_func)removeListeningPoint, mProvider);
	belle_sip_list_free(listeningPointsCopy);
	lInfo() << "Sal::unlistenPorts(): done";
}

bool Sal::isTransportAvailable (SalTransport tr) {
	switch (tr) {
		case SalTransportUDP:
		case SalTransportTCP:
			return true;
		case SalTransportTLS:
			return !!belle_sip_stack_tls_available(mStack);
		case SalTransportDTLS:
		default:
			return false;
	}
}

void Sal::makeSupportedHeader () {
	if (mSupportedHeader) {
		belle_sip_object_unref(mSupportedHeader);
		mSupportedHeader = nullptr;
	}
	string tags = Utils::join(mSupportedTags, ", ");
	if (tags.empty())
		return;
	mSupportedHeader = belle_sip_header_create("Supported", tags.c_str());
	if (mSupportedHeader)
		belle_sip_object_ref(mSupportedHeader);
}

void Sal::setSupportedTags (const string &tags) {
	vector<string> splittedTags = Utils::split(tags, ",");
	mSupportedTags.clear();
	for (const auto &tag : splittedTags)
		mSupportedTags.push_back(Utils::trim(tag));
	makeSupportedHeader();
}

const string &Sal::getSupportedTags () const {
	if (mSupportedHeader)
		mSupported = belle_sip_header_get_unparsed_value(mSupportedHeader);
	else
		mSupported.clear();
	return mSupported;
}

void Sal::addSupportedTag (const string &tag) {
	auto it = find(mSupportedTags.cbegin(), mSupportedTags.cend(), tag);
	if (it == mSupportedTags.cend()) {
		mSupportedTags.push_back(tag);
		makeSupportedHeader();
	}
}

void Sal::removeSupportedTag (const string &tag) {
	auto it = find(mSupportedTags.begin(), mSupportedTags.end(), tag);
	if (it != mSupportedTags.end()) {
		mSupportedTags.erase(it);
		makeSupportedHeader();
	}
}

void Sal::setWellKnownPort (int value) {
	belle_sip_stack_set_well_known_port(value);
}

void Sal::setTLSWellKnownPort (int value) {
	belle_sip_stack_set_well_known_port_tls(value);
}

void Sal::resetTransports () {
	lInfo() << "Reseting transports";
	belle_sip_provider_clean_channels(mProvider);
}

void Sal::setUserAgent (const string &value) {
	belle_sip_header_user_agent_set_products(mUserAgentHeader, nullptr);
	belle_sip_header_user_agent_add_product(mUserAgentHeader, L_STRING_TO_C(value));
}

const string &Sal::getUserAgent () const {
	char userAgent[256];
	belle_sip_header_user_agent_get_products_as_string(mUserAgentHeader, userAgent, (unsigned int)sizeof(userAgent) - 1);
	mUserAgent = userAgent;
	return mUserAgent;
}

void Sal::appendStackStringToUserAgent () {
	stringstream ss;
	ss << "(belle-sip/" << belle_sip_version_to_string() << ")";
	string stackStr = ss.str();
	belle_sip_header_user_agent_add_product(mUserAgentHeader, stackStr.c_str());
}

void Sal::setKeepAlivePeriod (unsigned int value) {
	mKeepAlive = value;
	for (auto it = belle_sip_provider_get_listening_points(mProvider); it; it = bctbx_list_next(it)) {
		auto listeningPoint = static_cast<belle_sip_listening_point_t *>(bctbx_list_get_data(it));
		if (mUseTcpTlsKeepAlive
			|| (strcasecmp(belle_sip_listening_point_get_transport(listeningPoint), "udp") == 0)
		) {
			belle_sip_listening_point_set_keep_alive(listeningPoint, (int)mKeepAlive);
		}
	}
}

void Sal::sendKeepAlive () {
	for (auto it = belle_sip_provider_get_listening_points(mProvider); it; it = bctbx_list_next(it)) {
		auto listeningPoint = static_cast<belle_sip_listening_point_t *>(bctbx_list_get_data(it));
		belle_sip_listening_point_send_keep_alive(listeningPoint);
	}
}

void Sal::cleanUnreliableConnections(){
	belle_sip_provider_clean_unreliable_channels(mProvider);
}

int Sal::setTunnel (void *tunnelclient) {
#ifdef TUNNEL_ENABLED
	mTunnelClient = tunnelclient;
	return 0;
#else
	return -1;
#endif
}

void Sal::setHttpProxyHost (const string &value) {
	belle_sip_stack_set_http_proxy_host(mStack, L_STRING_TO_C(value));
}

const string &Sal::getHttpProxyHost () const {
	mHttpProxyHost = belle_sip_stack_get_http_proxy_host(mStack);
	return mHttpProxyHost;
}

bool Sal::isContentEncodingAvailable (const string &contentEncoding) const {
	return !!belle_sip_stack_content_encoding_available(mStack, L_STRING_TO_C(contentEncoding));
}

bool Sal::isContentTypeSupported (const string &contentType) const {
	auto it = find_if(mSupportedContentTypes.cbegin(), mSupportedContentTypes.cend(),
		[contentType](string supportedContentType) {
			return contentType == supportedContentType;
		}
	);
	return it != mSupportedContentTypes.cend();
}

void Sal::addContentTypeSupport (const string &contentType) {
	if (!contentType.empty() && !isContentTypeSupported(contentType))
		mSupportedContentTypes.push_back(contentType);
}

void Sal::removeContentTypeSupport (const string &contentType) {
	auto it = find(mSupportedContentTypes.begin(), mSupportedContentTypes.end(), contentType);
	if (it != mSupportedContentTypes.end())
		mSupportedContentTypes.erase(it);
}

void Sal::useRport (bool useRports) {
	belle_sip_provider_enable_rport(mProvider, useRports);
	lInfo() << "Sal use rports [" << (useRports ? "enabled" : "disabled") << "]";
}

void Sal::setRootCa (const string &value) {
	mRootCa = value;
	setTlsProperties();
}

void Sal::setRootCaData (const string &value) {
	mRootCaData = value;
	setTlsProperties();
}

void Sal::verifyServerCertificates (bool verify) {
	mTlsVerify = verify;
	setTlsProperties();
}

void Sal::verifyServerCn (bool verify) {
	mTlsVerifyCn = verify;
	setTlsProperties();
}

void Sal::setSslConfig (void *sslConfig) {
	mSslConfig = sslConfig;
	setTlsProperties();
}

void Sal::setTlsPostcheckCallback (int (*cb)(void *, const bctbx_x509_certificate_t *), void *data) {
	mTlsPostcheckCb = cb;
	mTlsPostcheckCbData = data;
}

string Sal::createUuid () {
	setUuid(generateUuid());
	return mUuid;
}

string Sal::generateUuid () {
	// Create an UUID as described in RFC4122, 4.4
	SalUuid uuidStruct;
	belle_sip_random_bytes((unsigned char *)&uuidStruct, sizeof(uuidStruct));
	uuidStruct.clockSeqHiAndReserved &= (unsigned char)~(1 << 6);
	uuidStruct.clockSeqHiAndReserved |= (unsigned char)(1 << 7);
	uuidStruct.timeHiAndVersion &= (unsigned char)~(0xf << 12);
	uuidStruct.timeHiAndVersion |= (unsigned char)(4 << 12);

	char generatedUuid[128] = { 0 };
	int written = snprintf(
		generatedUuid,
		sizeof(generatedUuid) - 1,
		"%8.8x-%4.4x-%4.4x-%2.2x%2.2x-",
		uuidStruct.timeLow,
		uuidStruct.timeMid,
		uuidStruct.timeHiAndVersion,
		uuidStruct.clockSeqHiAndReserved,
		uuidStruct.clockSeqLow
	);
	for (int i = 0; i < 6; i++)
		written += snprintf(generatedUuid + written, sizeof(generatedUuid) - (unsigned long)written, "%2.2x", uuidStruct.node[i]);
	return generatedUuid;
}

void Sal::addPendingAuth (SalOp *op) {
	auto it = find(mPendingAuths.cbegin(), mPendingAuths.cend(), op);
	if (it == mPendingAuths.cend()) {
		mPendingAuths.push_back(op);
		op->mHasAuthPending = true;
		lInfo() << "Op " << op << " added as pending authentication";
	}
}

void Sal::removePendingAuth (SalOp *op) {
	if (op->mHasAuthPending) {
		op->mHasAuthPending = false;
		mPendingAuths.remove(op);
		lInfo() << "Op " << op << " removed as pending authentication";
	}
}

void Sal::setDefaultSdpHandling (SalOpSDPHandling sdpHandlingMethod) {
	if (sdpHandlingMethod != SalOpSDPNormal)
		lInfo() << "Enabling special SDP handling for all new SalOp in Sal[" << this << "]!";
	mDefaultSdpHandling = sdpHandlingMethod;
}

void Sal::enableNatHelper (bool enable) {
	mNatHelperEnabled = enable;
	belle_sip_provider_enable_nat_helper(mProvider, enable);
	lInfo() << "Sal nat helper [" << (enable ? "enabled" : "disabled") << "]";
}

void Sal::setDnsServers (const bctbx_list_t *servers) {
	belle_sip_stack_set_dns_servers(mStack, servers);
}

void Sal::setDnsUserHostsFile (const string &value) {
	belle_sip_stack_set_dns_user_hosts_file(mStack, value.c_str());
}

const string &Sal::getDnsUserHostsFile () const {
	mDnsUserHostsFile = belle_sip_stack_get_dns_user_hosts_file(mStack);
	return mDnsUserHostsFile;
}

belle_sip_resolver_context_t *Sal::resolveA (const string &name, int port, int family, belle_sip_resolver_callback_t cb, void *data) {
	return belle_sip_stack_resolve_a(mStack, L_STRING_TO_C(name), port, family, cb, data);
}

belle_sip_resolver_context_t *Sal::resolve (const string &service, const string &transport, const string &name, int port, int family, belle_sip_resolver_callback_t cb, void *data) {
	return belle_sip_stack_resolve(mStack, L_STRING_TO_C(service), L_STRING_TO_C(transport), L_STRING_TO_C(name), port, family, cb, data);
}

belle_sip_source_t *Sal::createTimer (const std::function<bool ()> &something, unsigned int milliseconds, const string &name) {
	return belle_sip_main_loop_create_cpp_timeout_2(belle_sip_stack_get_main_loop(mStack), something, (unsigned)milliseconds, name.c_str());
}

belle_sip_source_t *Sal::createTimer (belle_sip_source_func_t func, void *data, unsigned int timeoutValueMs, const string &timerName) {
	belle_sip_main_loop_t *ml = belle_sip_stack_get_main_loop(mStack);
	return belle_sip_main_loop_create_timeout(ml, func, data, timeoutValueMs, L_STRING_TO_C(timerName));
}

void Sal::cancelTimer(belle_sip_source_t *timer) {
	belle_sip_main_loop_t *ml = belle_sip_stack_get_main_loop(mStack);
	belle_sip_main_loop_remove_source(ml, timer);
}

belle_sip_response_t *Sal::createResponseFromRequest (belle_sip_request_t *request, int code) {
	auto response = belle_sip_response_create_from_request(request, code);
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), BELLE_SIP_HEADER(mUserAgentHeader));
	if (mSupportedHeader) {
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), mSupportedHeader);
	}
	return response;
}

int Sal::findCryptoIndexFromTag (const SalSrtpCryptoAlgo crypto[], unsigned char tag) {
	for (int i = 0; i < SAL_CRYPTO_ALGO_MAX; i++) {
		if (crypto[i].tag == tag)
			return i;
	}
	return -1;
}


//***********************************
// Global functions implementation
//***********************************

int toSipCode (SalReason reason) {
	switch (reason) {
		case SalReasonNone:
			return 200;
		case SalReasonIOError:
			return 503;
		case SalReasonUnknown:
			return 400;
		case SalReasonBusy:
			return 486;
		case SalReasonDeclined:
			return 603;
		case SalReasonDoNotDisturb:
			return 600;
		case SalReasonForbidden:
			return 403;
		case SalReasonUnsupportedContent:
			return 415;
		case SalReasonNotFound:
			return 404;
		case SalReasonRedirect:
			return 302;
		case SalReasonTemporarilyUnavailable:
			return 480;
		case SalReasonServiceUnavailable:
			return 503;
		case SalReasonRequestPending:
			return 491;
		case SalReasonUnauthorized:
			return 401;
		case SalReasonNotAcceptable:
			return 488; // Or maybe 606 Not Acceptable?
		case SalReasonNoMatch:
			return 481;
		case SalReasonRequestTimeout:
			return 408;
		case SalReasonMovedPermanently:
			return 301;
		case SalReasonGone:
			return 410;
		case SalReasonAddressIncomplete:
			return 484;
		case SalReasonNotImplemented:
			return 501;
		case SalReasonServerTimeout:
			return 504;
		case SalReasonBadGateway:
			return 502;
		case SalReasonInternalError:
		default:
			return 500;
	}
}

LINPHONE_END_NAMESPACE

//*******************************
// C++ to C wrapping functions
//*******************************

using namespace LinphonePrivate;

// NOTE: This is ugly but it's not possible to export simply this set of functions in tester_utils...
// Because tester_utils and private files are ill-thought.
// A workaround is to use LINPHONE_PUBLIC here.

extern "C" {

LINPHONE_PUBLIC Sal *linphone_core_get_sal (const LinphoneCore *lc) {
	return lc->sal;
}

LINPHONE_PUBLIC SalOp *linphone_proxy_config_get_sal_op (const LinphoneProxyConfig *cfg) {
	return cfg->op;
}

LINPHONE_PUBLIC SalOp *linphone_call_get_op_as_sal_op (const LinphoneCall *call) {
	return linphone_call_get_op(call);
}

LINPHONE_PUBLIC Sal *sal_init (MSFactory *factory) {
	return new Sal(factory);
}

LINPHONE_PUBLIC void sal_uninit (Sal* sal) {
	delete sal;
}

LINPHONE_PUBLIC int sal_create_uuid (Sal *ctx, char *uuid, size_t len) {
	string uuidStr = ctx->createUuid();
	if (len < uuidStr.size())
		return -1;
	snprintf(uuid, len, "%s", uuidStr.c_str());
	return 0;
}

LINPHONE_PUBLIC void sal_set_uuid (Sal *ctx, const char *uuid) {
	ctx->setUuid(L_C_TO_STRING(uuid));
}

LINPHONE_PUBLIC void sal_default_set_sdp_handling (Sal* h, SalOpSDPHandling sdpHandlingMethod) {
	h->setDefaultSdpHandling(sdpHandlingMethod);
}

LINPHONE_PUBLIC void sal_set_send_error (Sal *sal,int value) {
	sal->setSendError(value);
}

LINPHONE_PUBLIC void sal_set_recv_error (Sal *sal,int value) {
	sal->setRecvError(value);
}

LINPHONE_PUBLIC void sal_enable_pending_trans_checking (Sal *sal, bool_t value) {
	sal->enablePendingTransactionChecking(!!value);
}

LINPHONE_PUBLIC void sal_enable_unconditional_answer (Sal *sal, bool_t value) {
	sal->enableUnconditionalAnswer(value);
}
LINPHONE_PUBLIC void sal_set_unconditional_answer (Sal *sal, unsigned short value) {
	sal->setUnconditionalAnswer(value);
}

LINPHONE_PUBLIC void sal_set_client_bind_port(Sal *sal, int port){
	sal->setClientBindPort(port);
}

LINPHONE_PUBLIC void sal_set_dns_timeout (Sal* sal, int timeout) {
	sal->setDnsTimeout(timeout);
}

LINPHONE_PUBLIC void sal_set_dns_user_hosts_file (Sal *sal, const char *hostsFile) {
	sal->setDnsUserHostsFile(hostsFile);
}

LINPHONE_PUBLIC const char* sal_get_dns_user_hosts_file (const Sal *sal) {
	return sal->getDnsUserHostsFile().c_str();
}

LINPHONE_PUBLIC void *sal_get_stack_impl (Sal *sal) {
	return sal->getStackImpl();
}

LINPHONE_PUBLIC void sal_set_refresher_retry_after (Sal *sal, int value) {
	sal->setRefresherRetryAfter(value);
}

LINPHONE_PUBLIC int sal_get_refresher_retry_after (const Sal *sal) {
	return sal->getRefresherRetryAfter();
}

LINPHONE_PUBLIC void sal_set_transport_timeout (Sal* sal, int timeout) {
	sal->setTransportTimeout(timeout);
}

LINPHONE_PUBLIC void sal_enable_test_features (Sal*ctx, bool_t value) {
	ctx->enableTestFeatures(!!value);
}

LINPHONE_PUBLIC bool_t sal_transport_available (Sal *ctx, SalTransport tr) {
	return (bool_t)ctx->isTransportAvailable(tr);
}

LINPHONE_PUBLIC const SalErrorInfo *sal_op_get_error_info (const SalOp *op) {
	return op->getErrorInfo();
}

LINPHONE_PUBLIC bool_t sal_call_dialog_request_pending (const SalOp *op) {
	auto callOp = dynamic_cast<const SalCallOp *>(op);
	if (!callOp)
		return FALSE;
	return callOp->dialogRequestPending();
}

LINPHONE_PUBLIC void sal_call_set_sdp_handling (SalOp *op, SalOpSDPHandling handling) {
	auto callOp = dynamic_cast<SalCallOp *>(op);
	if (callOp)
		callOp->setSdpHandling(handling);
}

LINPHONE_PUBLIC SalMediaDescription *sal_call_get_final_media_description (SalOp *op) {
	auto callOp = dynamic_cast<SalCallOp *>(op);
	if (!callOp)
		return nullptr;
	return callOp->getFinalMediaDescription();
}

LINPHONE_PUBLIC const char *sal_call_get_local_tag (SalOp *op) {
	auto callOp = dynamic_cast<SalCallOp *>(op);
	if (!callOp)
		return nullptr;
	return callOp->getLocalTag();
}

LINPHONE_PUBLIC const char *sal_call_get_remote_tag (SalOp *op) {
	auto callOp = dynamic_cast<SalCallOp *>(op);
	if (!callOp)
		return nullptr;
	return callOp->getRemoteTag();
}

LINPHONE_PUBLIC void sal_call_set_replaces (SalOp *op, const char *callId, const char *fromTag, const char *toTag) {
	auto callOp = dynamic_cast<SalCallOp *>(op);
	if (!callOp)
		return;
	callOp->setReplaces(callId, fromTag, toTag);
}

LINPHONE_PUBLIC belle_sip_resolver_context_t *sal_resolve_a (Sal *sal, const char *name, int port, int family, belle_sip_resolver_callback_t cb, void *data) {
	return sal->resolveA(name, port, family, cb, data);
}

LINPHONE_PUBLIC Sal *sal_op_get_sal (SalOp *op) {
	return op->getSal();
}

LINPHONE_PUBLIC SalOp *sal_create_refer_op (Sal *sal) {
	return new SalReferOp(sal);
}

LINPHONE_PUBLIC void sal_release_op (SalOp *op) {
	op->release();
}

LINPHONE_PUBLIC void sal_op_set_from (SalOp *salReferOp, const char *from) {
	auto referOp = dynamic_cast<SalReferOp *>(salReferOp);
	if (referOp)
		referOp->setFrom(from);
}

LINPHONE_PUBLIC void sal_op_set_to (SalOp *salReferOp, const char *to) {
	auto referOp = dynamic_cast<SalReferOp *>(salReferOp);
	if (referOp)
		referOp->setTo(to);
}

LINPHONE_PUBLIC void sal_op_send_refer (SalOp *salReferOp, SalAddress *referTo) {
	auto referOp = dynamic_cast<SalReferOp *>(salReferOp);
	if (referOp)
		referOp->sendRefer(referTo);
}

LINPHONE_PUBLIC void sal_set_user_pointer (Sal *sal, void *userPointer) {
	sal->setUserPointer(userPointer);
}

LINPHONE_PUBLIC void *sal_get_user_pointer (Sal *sal) {
	return sal->getUserPointer();
}

LINPHONE_PUBLIC void sal_set_call_refer_callback (Sal *sal, void (*OnReferCb)(SalOp *op, const SalAddress *referTo)) {
	struct Sal::Callbacks cbs = { nullptr };
	cbs.refer_received = OnReferCb;
	sal->setCallbacks(&cbs);
}

char *sal_generate_uuid (void) {
	string uuid = Sal::generateUuid();
	return bctbx_strdup(uuid.c_str());
}

}
