/*
 * op.cpp
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

#include <cstring>

#include "c-wrapper/internal/c-tools.h"
#include "bellesip_sal/sal_impl.h"
#include "sal/op.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SalOp::SalOp (Sal *sal) {
	mRoot = sal;
	mSdpHandling = sal->mDefaultSdpHandling;
	memset(&mErrorInfo, 0, sizeof(mErrorInfo));
	memset(&mReasonErrorInfo, 0, sizeof(mReasonErrorInfo));
	ref();
}

SalOp::~SalOp () {
	lInfo() << "Destroying op [" << this << "] of type [" << toString(mType) << "]";

	if (mPendingAuthTransaction)
		belle_sip_object_unref(mPendingAuthTransaction);
	mRoot->removePendingAuth(this);
	if (mAuthInfo)
		sal_auth_info_delete(mAuthInfo);
	if (mSdpAnswer)
		belle_sip_object_unref(mSdpAnswer);
	if (mRefresher)
		belle_sip_object_unref(mRefresher);
	if (mResult)
		sal_media_description_unref(mResult);
	if(mReplaces)
		belle_sip_object_unref(mReplaces);
	if(mReferredBy)
		belle_sip_object_unref(mReferredBy);

	if (mPendingClientTransaction)
		belle_sip_object_unref(mPendingClientTransaction);
	if (mPendingServerTransaction)
		belle_sip_object_unref(mPendingServerTransaction);
	if (mPendingUpdateServerTransaction) 
		belle_sip_object_unref(mPendingUpdateServerTransaction);
	if (mEvent)
		belle_sip_object_unref(mEvent);

	sal_error_info_reset(&mErrorInfo);

	if (mFromAddress)
		sal_address_unref(mFromAddress);
	if (mToAddress)
		sal_address_unref(mToAddress);
	if (mServiceRoute)
		sal_address_unref(mServiceRoute);
	if (mOriginAddress)
		sal_address_unref(mOriginAddress);
	if (mContactAddress)
		sal_address_unref(mContactAddress);
	if (mRemoteContactAddress)
		sal_address_unref(mRemoteContactAddress);
	if (mServiceRoute)
		sal_address_unref(mServiceRoute);
	for (auto &addr : mRouteAddresses)
		sal_address_unref(addr);

	if (mRecvCustomHeaders)
		sal_custom_header_free(mRecvCustomHeaders);
	if (mSentCustomHeaders)
		sal_custom_header_free(mSentCustomHeaders);
}

SalOp *SalOp::ref () {
	mRef++;
	return this;
}

void *SalOp::unref () {
	mRef--;
	if (mRef == 0)
		delete this;
	else if (mRef < 0)
		lFatal() << "SalOp [" << this << "]: too many unrefs!";
	return nullptr;
}

void SalOp::setContactAddress (const SalAddress *value) {
	if (mContactAddress)
		sal_address_unref(mContactAddress);
	mContactAddress = value ? sal_address_clone(value) : nullptr;
}

void SalOp::assignAddress (SalAddress **address, const string &value) {
	if (*address) {
		sal_address_unref(*address);
		*address = nullptr;
	}
	if (!value.empty())
		*address = sal_address_new(value.c_str());
}

void SalOp::setRoute (const string &value) {
	for (auto &address : mRouteAddresses)
		sal_address_unref(address);
	mRouteAddresses.clear();
	if (value.empty()) {
		mRoute.clear();
	} else {
		auto address = sal_address_new(value.c_str());
		mRouteAddresses.push_back(address);
		char *routeStr = sal_address_as_string(address);
		mRoute = routeStr;
		ms_free(routeStr);
	}
}

void SalOp::setRouteAddress (const SalAddress *value) {
	// Can probably be optimized
	char *addressStr = sal_address_as_string(value);
	setRoute(addressStr);
	ms_free(addressStr);
}

void SalOp::addRouteAddress (const SalAddress *address) {
	if (mRouteAddresses.empty())
		setRouteAddress(address);
	else
		mRouteAddresses.push_back(sal_address_clone(address));
}

void SalOp::setFrom (const string &value) {
	assignAddress(&mFromAddress, value);
	if (mFromAddress) {
		char *valueStr = sal_address_as_string(mFromAddress);
		mFrom = valueStr;
		ms_free(valueStr);
	} else {
		mFrom.clear();
	}
}

void SalOp::setFromAddress (const SalAddress *value) {
	// Can probably be optimized
	char *addressStr = sal_address_as_string(value);
	setFrom(addressStr);
	ms_free(addressStr);
}

void SalOp::setTo (const string &value) {
	assignAddress(&mToAddress, value);
	if (mToAddress) {
		char *valueStr = sal_address_as_string(mToAddress);
		mTo = valueStr;
		ms_free(valueStr);
	} else {
		mTo.clear();
	}
}

void SalOp::setToAddress (const SalAddress *value) {
	// Can probably be optimized
	char *addressStr = sal_address_as_string(value);
	setTo(addressStr);
	ms_free(addressStr);
}

void SalOp::setDiversionAddress (const SalAddress *diversion) {
	if (mDiversionAddress)
		sal_address_unref(mDiversionAddress);
	mDiversionAddress = diversion ? sal_address_clone(diversion) : nullptr;
}

int SalOp::refresh () {
	if (mRefresher) {
		belle_sip_refresher_refresh(mRefresher, belle_sip_refresher_get_expires(mRefresher));
		return 0;
	}
	lWarning() << "No refresher on op [" << this << "] of type [" << toString(mType) << "]";
	return -1;
}

void SalOp::killDialog () {
	lInfo() << "op [" << this << "]: force kill of dialog [" << mDialog << "]";
	belle_sip_dialog_delete(mDialog);
}

void SalOp::release () {
	if (mOpReleased){
		lError() << "op [" << this << "]: double release detected and ignored.";
		return;
	}
	// If in terminating state, keep this state because it means we are waiting for a response to be able to terminate the operation
	if (mState != State::Terminating)
		mState = State::Terminated;
	setUserPointer(nullptr); // Mandatory because releasing op doesn't not mean freeing op, so make sure back pointer will not be used later
	if (mReleaseCb)
		mReleaseCb(this);
	if (mRefresher)
		belle_sip_refresher_stop(mRefresher);
	mOpReleased = true;
	unref();
}

int SalOp::sendRequestWithContact (belle_sip_request_t *request, bool addContact) {
	belle_sip_uri_t *nextHopUri = nullptr;

	if (addContact && !belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_contact_t)) {
		auto contactHeader = createContact();
		belle_sip_message_set_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(contactHeader));
	}

	addCustomHeaders(BELLE_SIP_MESSAGE(request));

	if (!mDialog || (belle_sip_dialog_get_state(mDialog) == BELLE_SIP_DIALOG_NULL)) {
		// Don't put route header if dialog is in confirmed state
		auto routeAddresses = getRouteAddresses();
		if (routeAddresses.empty())
			nextHopUri = BELLE_SIP_URI(belle_sip_object_clone(BELLE_SIP_OBJECT(belle_sip_request_get_uri(request))));
		else
			nextHopUri = belle_sip_header_address_get_uri(reinterpret_cast<belle_sip_header_address_t *>(routeAddresses.front()));

		auto udpListeningPoint = belle_sip_provider_get_listening_point(mRoot->mProvider, "UDP");
		const char *transport = belle_sip_uri_get_transport_param(nextHopUri);
		if (transport) {
#ifdef TUNNEL_ENABLED
			if (udpListeningPoint && BELLE_SIP_OBJECT_IS_INSTANCE_OF(udpListeningPoint, belle_sip_tunnel_listening_point_t)) {
				// Our tunnel mode only supports UDP, force transport to be set to UDP
				belle_sip_uri_set_transport_param(nextHopUri, "udp");
			}
#endif
		} else {
			// Compatibility mode: by default it should be udp as not explicitely set and if no udp listening point is
			// available, then use the first available transport
			if (!belle_sip_uri_is_secure(nextHopUri)) {
				if (!udpListeningPoint) {
					if (belle_sip_provider_get_listening_point(mRoot->mProvider, "TCP"))
						transport = "tcp";
					else if (belle_sip_provider_get_listening_point(mRoot->mProvider, "TLS"))
						transport = "tls";
				}
				if (transport) {
					lInfo() << "Transport is not specified, using " << transport << " because UDP is not available.";
					belle_sip_uri_set_transport_param(nextHopUri, transport);
				}
			}
		}
		// Because in case of tunnel, transport can be changed
		transport = belle_sip_uri_get_transport_param(nextHopUri);

		string method = belle_sip_request_get_method(request);
		if (((method == "REGISTER") || (method == "SUBSCRIBE"))
			&& transport
			&& ((strcasecmp(transport, "TCP") == 0) || (strcasecmp(transport, "TLS") == 0))
		) {
			// RFC5923: add 'alias' parameter to tell the server that we want it to keep the connection for future requests
			auto viaHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_via_t);
			belle_sip_parameters_set_parameter(BELLE_SIP_PARAMETERS(viaHeader), "alias", nullptr);
		}
	}

	auto clientTransaction = belle_sip_provider_create_client_transaction(mRoot->mProvider, request);
	belle_sip_transaction_set_application_data(BELLE_SIP_TRANSACTION(clientTransaction), ref());
	if (mPendingClientTransaction)
		belle_sip_object_unref(mPendingClientTransaction);
	mPendingClientTransaction = clientTransaction; // Update pending inv for being able to cancel
	belle_sip_object_ref(mPendingClientTransaction);

	if (!belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_user_agent_t))
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(mRoot->mUserAgentHeader));

	if (!belle_sip_message_get_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_AUTHORIZATION)
		&& !belle_sip_message_get_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_PROXY_AUTHORIZATION)
	) {
		// Hmm just in case we already have authentication param in cache
		belle_sip_provider_add_authorization(mRoot->mProvider, request, nullptr, nullptr, nullptr, L_STRING_TO_C(mRealm));
	}

	int result = belle_sip_client_transaction_send_request_to(clientTransaction, nextHopUri);

	// Update call id if not set yet for this op
	if ((result == 0) && mCallId.empty()) {
		mCallId = belle_sip_header_call_id_get_call_id(
			BELLE_SIP_HEADER_CALL_ID(belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_call_id_t))
		);
	}

	return result;
}

int SalOp::sendRequest (belle_sip_request_t *request) {
	if (!request)
		return -1; // Sanity check

	bool needContact = false;

	//  Header field          where   proxy ACK BYE CAN INV OPT REG
	//  ___________________________________________________________
	//  Contact                 R            o   -   -   m   o   o
	//
	// Despite contact seems not mandatory, call flow example show a Contact in REFER requests

	string method = belle_sip_request_get_method(request);
	if ((method == "INVITE")
		|| (method == "REGISTER")
		|| (method == "SUBSCRIBE")
		|| (method == "OPTIONS")
		|| (method == "REFER")
	) {
		needContact = true;
	}

	return sendRequestWithContact(request, needContact);
}

void SalOp::resendRequest (belle_sip_request_t *request) {
	auto cseqHeader = BELLE_SIP_HEADER_CSEQ(belle_sip_message_get_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_CSEQ));
	belle_sip_header_cseq_set_seq_number(cseqHeader, belle_sip_header_cseq_get_seq_number(cseqHeader) + 1);
	sendRequest(request);
}

int SalOp::processRedirect () {
	auto response = belle_sip_transaction_get_response(BELLE_SIP_TRANSACTION(mPendingClientTransaction));
	auto redirectContactHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(response), belle_sip_header_contact_t);

	if (!redirectContactHeader) {
		lWarning() << "Redirect not handled, there is no redirect contact header in response";
		return -1;
	}

	auto redirectUri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(redirectContactHeader));
	if (!redirectUri) {
		lWarning() << "Redirect not handled, there is no usable uri in contact";
		return -1;
	}

	if (mDialog && (belle_sip_dialog_get_state(mDialog) == BELLE_SIP_DIALOG_CONFIRMED)) {
		lWarning() << "Redirect not handled within established dialogs. Does it make sense?";
		return -1;
	}

	setOrUpdateDialog(nullptr);

	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(mPendingClientTransaction));
	auto callIdHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_call_id_t);
	belle_sip_message_remove_header_from_ptr(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(callIdHeader));
	callIdHeader = belle_sip_provider_create_call_id(getSal()->mProvider);
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(callIdHeader));
	mCallId.clear(); // Reset the call-id of op, it will be set when new request will be sent
	belle_sip_request_set_uri(request, redirectUri);
	redirectUri = BELLE_SIP_URI(belle_sip_object_clone(BELLE_SIP_OBJECT(redirectUri)));
	belle_sip_uri_set_port(redirectUri, 0);
	belle_sip_parameters_remove_parameter(BELLE_SIP_PARAMETERS(redirectUri), "transport");
	belle_sip_header_to_t *toHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_to_t);
	belle_sip_header_address_set_uri(BELLE_SIP_HEADER_ADDRESS(toHeader), redirectUri);
	sendRequest(request);
	return 0;
}

void SalOp::processAuthentication () {
	auto initialRequest = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(mPendingAuthTransaction));
	auto fromHeader = belle_sip_message_get_header_by_type(initialRequest, belle_sip_header_from_t);
	auto fromUri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(fromHeader));

	if (strcasecmp(belle_sip_uri_get_host(fromUri), "anonymous.invalid") == 0) {
		// Prefer using the from from the SalOp
		fromUri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(getFromAddress()));
	}

	bool isWithinDialog = false;
	belle_sip_request_t *newRequest = nullptr;
	if (mDialog && (belle_sip_dialog_get_state(mDialog) == BELLE_SIP_DIALOG_CONFIRMED)) {
		newRequest = belle_sip_dialog_create_request_from(mDialog, initialRequest);
		if (!newRequest)
			newRequest = belle_sip_dialog_create_queued_request_from(mDialog, initialRequest);
		isWithinDialog = true;
	} else {
		newRequest = initialRequest;
		belle_sip_message_remove_header(BELLE_SIP_MESSAGE(newRequest), BELLE_SIP_AUTHORIZATION);
		belle_sip_message_remove_header(BELLE_SIP_MESSAGE(newRequest), BELLE_SIP_PROXY_AUTHORIZATION);
	}
	if (!newRequest) {
		lError() << "SalOp::processAuthentication() op [" << this << "] cannot obtain new request from dialog";
		return;
	}

	belle_sip_list_t *authList = nullptr;
	auto response = belle_sip_transaction_get_response(BELLE_SIP_TRANSACTION(mPendingAuthTransaction));
	if (belle_sip_provider_add_authorization(mRoot->mProvider, newRequest, response, fromUri, &authList, L_STRING_TO_C(mRealm))) {
		if (isWithinDialog)
			sendRequest(newRequest);
		else
			resendRequest(newRequest);
		mRoot->removePendingAuth(this);
	} else {
		auto fromHeader = belle_sip_message_get_header_by_type(response, belle_sip_header_from_t);
		char *tmp = belle_sip_object_to_string(belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(fromHeader)));
		lInfo() << "No auth info found for [" << tmp << "]";
		belle_sip_free(tmp);
		mRoot->addPendingAuth(this);
		if (isWithinDialog)
			belle_sip_object_unref(newRequest);
	}
	// Always store auth info, for case of wrong credential
	if (mAuthInfo) {
		sal_auth_info_delete(mAuthInfo);
		mAuthInfo = nullptr;
	}
	if (authList) {
		auto authEvent = reinterpret_cast<belle_sip_auth_event_t *>(authList->data);
		mAuthInfo = sal_auth_info_create(authEvent);
		belle_sip_list_free_with_data(authList, (void (*)(void*))belle_sip_auth_event_destroy);
	}
}

string SalOp::getDialogId () const {
	if (!mDialog)
		return string();
	stringstream ss;
	const char *to_tag = belle_sip_dialog_is_server(mDialog) ? belle_sip_dialog_get_local_tag(mDialog) : belle_sip_dialog_get_remote_tag(mDialog);
	const char *from_tag = belle_sip_dialog_is_server(mDialog) ? belle_sip_dialog_get_remote_tag(mDialog) : belle_sip_dialog_get_local_tag(mDialog);
	if (!to_tag) to_tag = "";
	if (!from_tag) from_tag = ""; /* Not having from-tag should never happen.*/
	ss << mCallId << ";to-tag=" << to_tag << ";from-tag=" << from_tag;
	return ss.str();
}

int SalOp::getAddressFamily () const {
	belle_sip_transaction_t *transaction = nullptr;
	if (mRefresher)
		transaction = BELLE_SIP_TRANSACTION(belle_sip_refresher_get_transaction(mRefresher));
	if (!transaction)
		transaction = BELLE_SIP_TRANSACTION(mPendingClientTransaction);
	if (!transaction)
		transaction = BELLE_SIP_TRANSACTION(mPendingServerTransaction);
	if (!transaction) {
		lError() << "Unable to determine IP version from signaling operation";
		return AF_UNSPEC;
	}

	if (mRefresher) {
		auto response = belle_sip_transaction_get_response(transaction);
		auto viaHeader = response ? belle_sip_message_get_header_by_type(response, belle_sip_header_via_t) : nullptr;
		if (!viaHeader) {
			lError() << "Unable to determine IP version from signaling operation, no via header found";
			return AF_UNSPEC;
		}
		const char *host = belle_sip_header_via_get_host(viaHeader);
		if (!host) {
			lError() << "Unable to determine IP version from signaling operation, no via header is not yet completed";
			return AF_UNSPEC;
		}
		return strchr(host, ':') ? AF_INET6 : AF_INET;
	}

	belle_sip_request_t *request = belle_sip_transaction_get_request(transaction);
	auto contactHeader = reinterpret_cast<belle_sip_header_address_t *>(
		belle_sip_message_get_header_by_type(request, belle_sip_header_contact_t)
	);
	if (!contactHeader)
		lError() << "Unable to determine IP version from signaling operation, no contact header found";
	return sal_address_is_ipv6(reinterpret_cast<SalAddress *>(contactHeader)) ? AF_INET6 : AF_INET;
}

bool SalOp::isIdle () const {
	if (mDialog)
		return !belle_sip_dialog_request_pending(mDialog);
	return true;
}

void SalOp::setEvent (const string &eventName) {
	belle_sip_header_event_t *header = nullptr;
	if (mEvent)
		belle_sip_object_unref(mEvent);
	if (!eventName.empty()) {
		header = belle_sip_header_event_create(eventName.c_str());
		belle_sip_object_ref(header);
	}
	mEvent = header;
}

void SalOp::addInitialRouteSet (belle_sip_request_t *request, const list<SalAddress *> &routeAddresses) {
	bool uniqueRoute = (routeAddresses.size() == 1);
	for (const auto &address : routeAddresses) {
		// Optimization: if the initial route set only contains one URI which is the same as the request URI, ommit it
		if (uniqueRoute) {
			auto requestUri = belle_sip_request_get_uri(request);
			// Skip the first route it is the same as the request uri
			if (strcmp(sal_address_get_domain(address), belle_sip_uri_get_host(requestUri)) == 0) {
				lInfo() << "Skipping top route of initial route-set because same as request-uri";
				continue;
			}
		}

		auto routeHeader = belle_sip_header_route_create(reinterpret_cast<belle_sip_header_address_t *>(address));
		auto uri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(routeHeader));
		belle_sip_uri_set_lr_param(uri, 1);
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(routeHeader));
	}
}

belle_sip_request_t *SalOp::buildRequest (const string &method) {
	// Check that the op has a correct to address
	auto toAddress = getToAddress();
	if (!toAddress) {
		lError() << "No To: address, cannot build request";
		return nullptr;
	}

	auto toUri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(toAddress));
	if (!toUri) {
		lError() << "To: address is invalid, cannot build request";
		return nullptr;
	}

	char token[10];
	belle_sip_header_from_t *fromHeader = nullptr;
	if ((method == "REGISTER") || (mPrivacy == SalPrivacyNone)) {
		fromHeader = belle_sip_header_from_create(
			BELLE_SIP_HEADER_ADDRESS(getFromAddress()),
			belle_sip_random_token(token, sizeof(token))
		);
	} else {
		fromHeader = belle_sip_header_from_create2(
			"Anonymous <sip:anonymous@anonymous.invalid>",
			belle_sip_random_token(token, sizeof(token))
		);
	}

	// Make sure to preserve components like headers or port
	auto requestUri = BELLE_SIP_URI(belle_sip_object_clone(BELLE_SIP_OBJECT(toUri)));
	belle_sip_uri_set_secure(requestUri, isSecure());

	auto toHeader = belle_sip_header_to_create(BELLE_SIP_HEADER_ADDRESS(toAddress), nullptr);
	auto callIdHeader = belle_sip_provider_create_call_id(mRoot->mProvider);
	if (!mCallId.empty())
		belle_sip_header_call_id_set_call_id(callIdHeader, mCallId.c_str());

	auto request = belle_sip_request_create(
		requestUri,
		method.c_str(),
		callIdHeader,
		belle_sip_header_cseq_create(20, method.c_str()),
		fromHeader,
		toHeader,
		belle_sip_header_via_new(),
		70
	);

	if (mPrivacy & SalPrivacyId) {
		auto pPreferredIdentityHeader = belle_sip_header_p_preferred_identity_create(
			BELLE_SIP_HEADER_ADDRESS(getFromAddress())
		);
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(pPreferredIdentityHeader));
	}

	auto routeAddresses = getRouteAddresses();
	if (!routeAddresses.empty() && (method != "REGISTER") && !mRoot->mNoInitialRoute)
		addInitialRouteSet(request, routeAddresses);

	if ((method != "REGISTER") && (mPrivacy != SalPrivacyNone)) {
		auto privacyHeader = belle_sip_header_privacy_new();
		if (mPrivacy & SalPrivacyCritical)
			belle_sip_header_privacy_add_privacy(privacyHeader, sal_privacy_to_string(SalPrivacyCritical));
		if (mPrivacy & SalPrivacyHeader)
			belle_sip_header_privacy_add_privacy(privacyHeader, sal_privacy_to_string(SalPrivacyHeader));
		if (mPrivacy & SalPrivacyId)
			belle_sip_header_privacy_add_privacy(privacyHeader, sal_privacy_to_string(SalPrivacyId));
		if (mPrivacy & SalPrivacyNone)
			belle_sip_header_privacy_add_privacy(privacyHeader, sal_privacy_to_string(SalPrivacyNone));
		if (mPrivacy & SalPrivacySession)
			belle_sip_header_privacy_add_privacy(privacyHeader, sal_privacy_to_string(SalPrivacySession));
		if (mPrivacy & SalPrivacyUser)
			belle_sip_header_privacy_add_privacy(privacyHeader, sal_privacy_to_string(SalPrivacyUser));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(privacyHeader));
	}
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), mRoot->mSupportedHeader);
	return request;
}

void SalOp::setErrorInfoFromResponse (belle_sip_response_t *response) {
	int statusCode = belle_sip_response_get_status_code(response);
	const char *reasonPhrase = belle_sip_response_get_reason_phrase(response);
	auto warningHeader = belle_sip_message_get_header(BELLE_SIP_MESSAGE(response), "Warning");
	auto warningStr = warningHeader ? belle_sip_header_get_unparsed_value(warningHeader) : nullptr;
	sal_error_info_set(&mErrorInfo, SalReasonUnknown, "SIP", statusCode, reasonPhrase, warningStr);
	setReasonErrorInfo(BELLE_SIP_MESSAGE(response));
}

string SalOp::toString (const State value) {
	switch (value) {
		case State::Early:
			return "SalOpStateEarly";
		case State::Active:
			return "SalOpStateActive";
		case State::Terminating:
			return "SalOpStateTerminating";
		case State::Terminated:
			return "SalOpStateTerminated";
		default:
			return "Unknown";
	}
}

void SalOp::setReasonErrorInfo (belle_sip_message_t *message) {
	auto reasonHeader = belle_sip_message_get_header_by_type(message, belle_sip_header_reason_t);
	if (!reasonHeader)
		return;

	sal_error_info_set(
		&mReasonErrorInfo,
		SalReasonUnknown,
		belle_sip_header_reason_get_protocol(reasonHeader),
		belle_sip_header_reason_get_cause(reasonHeader),
		belle_sip_header_reason_get_text(reasonHeader),
		nullptr
	);
}

void SalOp::setReferredBy (belle_sip_header_referred_by_t *referredByHeader) {
	if (mReferredBy)
		belle_sip_object_unref(mReferredBy);
	mReferredBy = referredByHeader;
	belle_sip_object_ref(mReferredBy);
}

void SalOp::setReplaces (belle_sip_header_replaces_t *replacesHeader) {
	if (mReplaces)
		belle_sip_object_unref(mReplaces);
	mReplaces = replacesHeader;
	belle_sip_object_ref(mReplaces);
}

int SalOp::sendRequestWithExpires (belle_sip_request_t *request, int expires) {
	auto expiresHeader = BELLE_SIP_HEADER_EXPIRES(belle_sip_message_get_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_EXPIRES));

	if (!expiresHeader && (expires >= 0)) {
		expiresHeader = belle_sip_header_expires_new();
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(expiresHeader));
	}
	if (expiresHeader)
		belle_sip_header_expires_set_expires(expiresHeader, expires);
	return sendRequest(request);
}

int SalOp::sendRequestAndCreateRefresher (belle_sip_request_t *request, int expires, belle_sip_refresher_listener_t listener) {
	if (sendRequestWithExpires(request, expires) != 0)
		return -1;

	if (mRefresher) {
		belle_sip_refresher_stop(mRefresher);
		belle_sip_object_unref(mRefresher);
	}

	//As stated a few lines below, "we should remove our context from the transaction", including op
	belle_sip_transaction_set_application_data(BELLE_SIP_TRANSACTION(mPendingClientTransaction), nullptr);
	mRefresher = belle_sip_client_transaction_create_refresher(mPendingClientTransaction);
	if (!mRefresher)
		return -1;

	// Since the refresher acquires the transaction we should remove our context from the transaction,
	// because we won't be notified that it is terminated anymore.
	unref(); // Loose the reference that was given to the transaction when creating it
	// Note that the refresher will replace our data with belle_sip_transaction_set_application_data().
	// Something in the design is not very good here, it makes things complicated to the belle-sip user.
	// Possible ideas to improve things: refresher shall not use belle_sip_transaction_set_application_data() internally,
	// refresher should let the first transaction notify the user as a normal transaction.
	belle_sip_refresher_set_listener(mRefresher, listener, this);
	belle_sip_refresher_set_retry_after(mRefresher, mRoot->mRefresherRetryAfter);
	belle_sip_refresher_set_realm(mRefresher, L_STRING_TO_C(mRealm));
	belle_sip_refresher_enable_manual_mode(mRefresher, mManualRefresher);
	return 0;
}

belle_sip_header_contact_t *SalOp::createContact () {
	belle_sip_header_contact_t *contactHeader = nullptr;
	if (getContactAddress()) {
		contactHeader = belle_sip_header_contact_create(BELLE_SIP_HEADER_ADDRESS(getContactAddress()));
	} else {
		contactHeader= belle_sip_header_contact_new();
	}

	auto contactUri = belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(contactHeader));
	if (!contactUri) {
		// No uri, just creating a new one
		contactUri = belle_sip_uri_new();
		belle_sip_header_address_set_uri(BELLE_SIP_HEADER_ADDRESS(contactHeader), contactUri);
	}
	belle_sip_uri_set_user_password(contactUri, nullptr);
	belle_sip_uri_set_secure(contactUri, isSecure());
	if (mPrivacy != SalPrivacyNone)
		belle_sip_uri_set_user(contactUri, nullptr);

	// Don't touch contact in case of gruu
	if (!belle_sip_parameters_has_parameter(
		BELLE_SIP_PARAMETERS(belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(contactHeader))),
		"gr"
		)
	) {
		belle_sip_header_contact_set_automatic(contactHeader, mRoot->mAutoContacts);
		if (!mRoot->mUuid.empty()
			&& !belle_sip_parameters_has_parameter(BELLE_SIP_PARAMETERS(contactHeader), "+sip.instance")
		) {
			stringstream ss;
			ss << "\"<urn:uuid:" << mRoot->mUuid << ">\"";
			string instanceId = ss.str();
			belle_sip_parameters_set_parameter(BELLE_SIP_PARAMETERS(contactHeader), "+sip.instance", instanceId.c_str());
		}
	}
	if (!mRoot->mLinphoneSpecs.empty()
	    && !belle_sip_parameters_has_parameter(BELLE_SIP_PARAMETERS(contactHeader), "+org.linphone.specs")
	) {
		stringstream ss;
		ss << "\"" << mRoot->mLinphoneSpecs << "\"";
		string specs = ss.str();
		belle_sip_parameters_set_parameter(BELLE_SIP_PARAMETERS(contactHeader), "+org.linphone.specs", specs.c_str());
	}
	return contactHeader;
}

void SalOp::unlinkOpFromDialog (belle_sip_dialog_t *dialog) {
	belle_sip_dialog_set_application_data(dialog, nullptr);
	unref();
	belle_sip_object_unref(dialog);
}

belle_sip_dialog_t *SalOp::linkOpWithDialog (belle_sip_dialog_t *dialog) {
	belle_sip_dialog_set_application_data(dialog, ref());
	belle_sip_object_ref(dialog);
	return dialog;
}

void SalOp::setOrUpdateDialog (belle_sip_dialog_t *dialog) {
	lInfo() << "op [" << this << "] : set_or_update_dialog() current=[" << mDialog << "] new=[" << dialog << "]";
	ref();
	if (mDialog != dialog) {
		if (mDialog){
			// FIXME: shouldn't we delete unconfirmed dialogs?
			unlinkOpFromDialog(mDialog);
			mDialog = nullptr;
		}
		if (dialog) {
			mDialog = linkOpWithDialog(dialog);
			belle_sip_dialog_enable_pending_trans_checking(dialog, mRoot->mPendingTransactionChecking);
		}
	}
	unref();
}

int SalOp::ping (const string &from, const string &to) {
	setFrom(from);
	setTo(to);
	return sendRequest(buildRequest("OPTIONS"));
}

int SalOp::sendInfo (const SalBodyHandler *bodyHandler) {
	if (mDialog && (belle_sip_dialog_get_state(mDialog) == BELLE_SIP_DIALOG_CONFIRMED)) {
		belle_sip_dialog_enable_pending_trans_checking(mDialog, mRoot->mPendingTransactionChecking);
		auto request = belle_sip_dialog_create_queued_request(mDialog, "INFO");
		belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(request), BELLE_SIP_BODY_HANDLER(bodyHandler));
		return sendRequest(request);
	} else {
		lError() << "Cannot send INFO message on op [" << this << "] because dialog is not in confirmed state yet";
	}
	return -1;
}

SalBodyHandler *SalOp::getBodyHandler (belle_sip_message_t *message) {
	auto bodyHandler = belle_sip_message_get_body_handler(message);
	if (bodyHandler) {
		auto contentTypeHeader = belle_sip_message_get_header_by_type(message, belle_sip_header_content_type_t);
		if (contentTypeHeader)
			belle_sip_body_handler_add_header(bodyHandler, BELLE_SIP_HEADER(contentTypeHeader));
		auto contentLengthHeader = belle_sip_message_get_header_by_type(message, belle_sip_header_content_length_t);
		if (contentLengthHeader)
			belle_sip_body_handler_add_header(bodyHandler, BELLE_SIP_HEADER(contentLengthHeader));
		auto contentEncodingHeader = belle_sip_message_get_header(message, "Content-Encoding");
		if (contentEncodingHeader)
			belle_sip_body_handler_add_header(bodyHandler, contentEncodingHeader);
	}
	return reinterpret_cast<SalBodyHandler *>(bodyHandler);
}

void SalOp::assignRecvHeaders (belle_sip_message_t *message) {
	if (message)
		belle_sip_object_ref(message);
	if (mRecvCustomHeaders) {
		belle_sip_object_unref(mRecvCustomHeaders);
		mRecvCustomHeaders = nullptr;
	}
	if (message)
		mRecvCustomHeaders = reinterpret_cast<SalCustomHeader *>(message);
}

void SalOp::setRemoteContact (const string &value) {
	assignAddress(&mRemoteContactAddress, value);
	mRemoteContact = value; // To preserve header params
}

void SalOp::setNetworkOrigin (const string &value) {
	assignAddress(&mOriginAddress, value);
	if (mOriginAddress) {
		char *valueStr = sal_address_as_string(mOriginAddress);
		mOrigin = valueStr;
		ms_free(valueStr);
	} else {
		mOrigin.clear();
	}
}

void SalOp::setNetworkOriginAddress (SalAddress *value) {
	// Can probably be optimized
	char *valueStr = sal_address_as_string(value);
	setNetworkOrigin(valueStr);
	ms_free(valueStr);
}

void SalOp::setPrivacyFromMessage (belle_sip_message_t *message) {
	// RFC3323 4.2 Expressing Privacy Preferences
	// When a Privacy header is constructed, it MUST consist of either the
	// value 'none', or one or more of the values 'user', 'header' and
	// 'session' (each of which MUST appear at most once) which MAY in turn
	// be followed by the 'critical' indicator.

	auto privacyHeader = belle_sip_message_get_header_by_type(message, belle_sip_header_privacy_t);
	if (!privacyHeader) {
		setPrivacy(SalPrivacyNone);
		return;
	}

	auto privacyList = belle_sip_header_privacy_get_privacy(privacyHeader);
	setPrivacy(0);
	for (;privacyList; privacyList = privacyList->next) {
		string privacyValue = static_cast<char *>(privacyList->data);
		if (privacyValue == sal_privacy_to_string(SalPrivacyCritical))
			setPrivacy(getPrivacy() | SalPrivacyCritical);
		if (privacyValue == sal_privacy_to_string(SalPrivacyHeader))
			setPrivacy(getPrivacy() | SalPrivacyHeader);
		if (privacyValue == sal_privacy_to_string(SalPrivacyId))
			setPrivacy(getPrivacy() | SalPrivacyId);
		if (privacyValue == sal_privacy_to_string(SalPrivacyNone)) {
			setPrivacy(SalPrivacyNone);
			break;
		}
		if (privacyValue == sal_privacy_to_string(SalPrivacySession))
			setPrivacy(getPrivacy() | SalPrivacySession);
		if (privacyValue == sal_privacy_to_string(SalPrivacyUser))
			setPrivacy(getPrivacy() | SalPrivacyUser);
	}
}

void SalOp::setRemoteUserAgent (belle_sip_message_t *message) {
	auto userAgentHeader = belle_sip_message_get_header_by_type(message, belle_sip_header_user_agent_t);
	char userAgentStr[256];
	if (userAgentHeader
		&& belle_sip_header_user_agent_get_products_as_string(userAgentHeader, userAgentStr, sizeof(userAgentStr)) > 0
	) {
		mRemoteUserAgent = userAgentStr;
	}
}

string SalOp::toString (const Type type) {
	switch (type) {
		case Type::Register:
			return "SalOpRegister";
		case Type::Call:
			return "SalOpCall";
		case Type::Message:
			return "SalOpMessage";
		case Type::Presence:
			return "SalOpPresence";
		default:
			return "SalOpUnknown";
	}
}

bool SalOp::isSecure() const {
	auto from = getFromAddress();
	auto to = getToAddress();
	return from
		&& to
		&& (strcasecmp("sips", sal_address_get_scheme(from)) == 0)
		&& (strcasecmp("sips", sal_address_get_scheme(to)) == 0);
}

// Warning: this function takes owneship of the custom headers
void SalOp::setSentCustomHeaders (SalCustomHeader *ch) {
	if (mSentCustomHeaders) {
		sal_custom_header_free(mSentCustomHeaders);
		mSentCustomHeaders = nullptr;
	}
	if (ch)
		belle_sip_object_ref(reinterpret_cast<belle_sip_message_t *>(ch));
	mSentCustomHeaders = ch;
}

void SalOp::addHeaders (belle_sip_header_t *h, belle_sip_message_t *message) {
	if (BELLE_SIP_OBJECT_IS_INSTANCE_OF(h, belle_sip_header_contact_t)){
		// Special case for contact, we want to keep everything from the custom contact but
		// set automatic mode and add our own parameters as well
		setContactAddress(reinterpret_cast<SalAddress *>(BELLE_SIP_HEADER_ADDRESS(h)));
		auto newContactHeader = createContact();
		belle_sip_message_set_header(BELLE_SIP_MESSAGE(message), BELLE_SIP_HEADER(newContactHeader));
		return;
	}
	// If a header already exists in the message, replace it
	belle_sip_message_set_header(message, h);
}

void SalOp::addCustomHeaders (belle_sip_message_t *message) {
	if (!mSentCustomHeaders)
		return;

	auto ch = reinterpret_cast<belle_sip_message_t *>(mSentCustomHeaders);
	auto l = belle_sip_message_get_all_headers(ch);
	for (auto elem = l; elem; elem = elem->next)
		addHeaders(reinterpret_cast<belle_sip_header_t *>(elem->data), message);
	belle_sip_list_free(l);
}

int SalOp::unsubscribe () {
	if (!mRefresher)
		return -1;

	auto transaction = reinterpret_cast<const belle_sip_transaction_t *>(belle_sip_refresher_get_transaction(mRefresher));
	auto lastRequest = belle_sip_transaction_get_request(transaction);
	belle_sip_message_set_body(BELLE_SIP_MESSAGE(lastRequest), nullptr, 0);
	belle_sip_refresher_refresh(mRefresher, 0);
	return 0;
}

int SalOp::setCustomBody(belle_sip_message_t *msg, const Content &body) {
	ContentType contentType = body.getContentType();
	auto contentDisposition = body.getContentDisposition();
	string contentEncoding = body.getContentEncoding();
	size_t bodySize = body.getBody().size();

	if (bodySize > SIP_MESSAGE_BODY_LIMIT) {
		bctbx_error("trying to add a body greater than %lukB to message [%p]", (unsigned long)SIP_MESSAGE_BODY_LIMIT/1024, msg);
		return -1;
	}

	if (contentType.isValid()) {
		belle_sip_header_content_type_t *content_type = belle_sip_header_content_type_create(contentType.getType().c_str(), contentType.getSubType().c_str());
		belle_sip_message_add_header(msg, BELLE_SIP_HEADER(content_type));
	}
	if (contentDisposition.isValid()) {
		belle_sip_header_content_disposition_t *contentDispositionHeader = belle_sip_header_content_disposition_create(
			contentDisposition.asString().c_str()
		);
		belle_sip_message_add_header(msg, BELLE_SIP_HEADER(contentDispositionHeader));
	}
	if (!contentEncoding.empty())
		belle_sip_message_add_header(msg, belle_sip_header_create("Content-Encoding", contentEncoding.c_str()));
	belle_sip_header_content_length_t *content_length = belle_sip_header_content_length_create(bodySize);
	belle_sip_message_add_header(msg, BELLE_SIP_HEADER(content_length));

	if (bodySize > 0) {
		char *buffer = bctbx_new(char, bodySize + 1);
		memcpy(buffer, body.getBody().data(), bodySize);
		buffer[bodySize] = '\0';
		belle_sip_message_assign_body(msg, buffer, bodySize);
	}

	return 0;
}

void SalOp::processIncomingMessage (const belle_sip_request_event_t *event) {
	auto request = belle_sip_request_event_get_request(event);
	auto serverTransaction = belle_sip_provider_create_server_transaction(mRoot->mProvider, request);
	auto contentTypeHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_content_type_t);
	if (contentTypeHeader) {
		if (mPendingServerTransaction)
			belle_sip_object_unref(mPendingServerTransaction);
		mPendingServerTransaction = serverTransaction;
		belle_sip_object_ref(mPendingServerTransaction);

		bool externalBody = isExternalBody(contentTypeHeader);
		auto fromHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_from_t);
		auto addressHeader = belle_sip_header_address_create(
			belle_sip_header_address_get_displayname(BELLE_SIP_HEADER_ADDRESS(fromHeader)),
			belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(fromHeader))
		);

		SalMessage salMessage;
		char messageId[256]{};
		char *from = belle_sip_object_to_string(BELLE_SIP_OBJECT(addressHeader));
		auto callIdHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_call_id_t);
		auto cseqHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_cseq_t);
		snprintf(
			messageId,
			sizeof(messageId) - 1,
			"%s%i",
			belle_sip_header_call_id_get_call_id(callIdHeader),
			belle_sip_header_cseq_get_seq_number(cseqHeader)
		);
		salMessage.from = from;
		// If we just deciphered a message, use the deciphered part (which can be a rcs xml body
		// pointing to the file to retreive from server)
		salMessage.text = (externalBody) ? nullptr : belle_sip_message_get_body(BELLE_SIP_MESSAGE(request));
		salMessage.url = nullptr;

		char buffer[1024];
		size_t offset = 0;
		belle_sip_parameters_marshal(BELLE_SIP_PARAMETERS(contentTypeHeader), buffer, sizeof(buffer), &offset);
		buffer[offset] = '\0';
		salMessage.content_type = ms_strdup_printf(
			"%s/%s%s",
			belle_sip_header_content_type_get_type(contentTypeHeader),
			belle_sip_header_content_type_get_subtype(contentTypeHeader),
			buffer
		);
		if (externalBody && belle_sip_parameters_get_parameter(BELLE_SIP_PARAMETERS(contentTypeHeader), "URL")) {
			size_t urlLength = strlen(belle_sip_parameters_get_parameter(BELLE_SIP_PARAMETERS(contentTypeHeader), "URL"));
			salMessage.url = ms_strdup(belle_sip_parameters_get_parameter(BELLE_SIP_PARAMETERS(contentTypeHeader), "URL") + 1); // Skip first "
			((char *)salMessage.url)[urlLength - 2] = '\0'; // Remove trailing "
		}

		salMessage.message_id = messageId;
		auto dateHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_date_t);
		salMessage.time = dateHeader ? belle_sip_header_date_get_time(dateHeader) : time(nullptr);
		mRoot->mCallbacks.message_received(this, &salMessage);

		belle_sip_object_unref(addressHeader);
		belle_sip_free(from);
		if (salMessage.url)
			ms_free((char *)(salMessage.url));
		ms_free((char *)(salMessage.content_type));
	} else {
		lError() << "Unsupported MESSAGE (no Content-Type)";
		auto response = belle_sip_response_create_from_request(request, 500);
		addMessageAccept(BELLE_SIP_MESSAGE(response));
		belle_sip_server_transaction_send_response(serverTransaction, response);
		release();
	}
}

bool SalOp::isExternalBody (belle_sip_header_content_type_t *contentTypeHeader) {
	return (strcmp("message", belle_sip_header_content_type_get_type(contentTypeHeader)) == 0)
		&& (strcmp("external-body", belle_sip_header_content_type_get_subtype(contentTypeHeader)) == 0);
}

int SalOp::replyMessage (SalReason reason) {
	if (!mPendingServerTransaction) {
		lError() << "SalOp::replyMessage(): no server transaction";
		return -1;
	}

	auto response = belle_sip_response_create_from_request(
		belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(mPendingServerTransaction)),
		toSipCode(reason)
	);
	belle_sip_server_transaction_send_response(mPendingServerTransaction, response);
	return 0;
}

void SalOp::addMessageAccept (belle_sip_message_t *message) {
	stringstream ss;
	ss << "xml/cipher, application/cipher.vnd.gsma.rcs-ft-http+xml";
	for (const auto &supportedContentType : mRoot->mSupportedContentTypes)
		ss << ", " << supportedContentType;
	string headerValue = ss.str();
	belle_sip_message_add_header(message, belle_sip_header_create("Accept", headerValue.c_str()));
}

void SalOp::setServiceRoute(const SalAddress *value) {
	if (mServiceRoute)
		sal_address_unref(mServiceRoute);
	mServiceRoute = value ? sal_address_clone(value) : nullptr;
}

LINPHONE_END_NAMESPACE
