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

#include "sal/refer-op.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void SalReferOp::processError () {
	mState = State::Terminated;
}

void SalReferOp::processIoErrorCb (void *userCtx, const belle_sip_io_error_event_t *event) {
	auto op = static_cast<SalReferOp *>(userCtx);
	sal_error_info_set(&op->mErrorInfo, SalReasonIOError, "SIP", 503, "IO Error", nullptr);
	op->processError();
}

void SalReferOp::processResponseEventCb (void *userCtx, const belle_sip_response_event_t *event) {
	auto op = static_cast<SalReferOp *>(userCtx);
	op->setErrorInfoFromResponse(belle_sip_response_event_get_response(event));
	// The response is not notified to the app, to be done when necessary
}

void SalReferOp::processTimeoutCb (void *userCtx, const belle_sip_timeout_event_t *event) {
	auto op = static_cast<SalReferOp *>(userCtx);
	sal_error_info_set(&op->mErrorInfo, SalReasonRequestTimeout, "SIP", 408, "Request timeout", nullptr);
	op->processError();
}

void SalReferOp::processRequestEventCb (void *userCtx, const belle_sip_request_event_t *event) {
	auto op = static_cast<SalReferOp *>(userCtx);
	auto request = belle_sip_request_event_get_request(event);
	auto referToHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_refer_to_t);
	auto transaction = belle_sip_provider_create_server_transaction(op->mRoot->mProvider, belle_sip_request_event_get_request(event));
	belle_sip_object_ref(transaction);
	belle_sip_transaction_set_application_data(BELLE_SIP_TRANSACTION(transaction), op->ref());
	op->mPendingServerTransaction = transaction;

	if (!referToHeader) {
		lWarning() << "Cannot do anything with the REFER without destination";
		op->reply(SalReasonUnknown); // Is mapped on bad request
		op->unref();
		return;
	}

	auto referToAddr = sal_address_new(belle_sip_header_get_unparsed_value(BELLE_SIP_HEADER(referToHeader)));
	op->mRoot->mCallbacks.refer_received(op, referToAddr);
	// The app is expected to reply in the callback
	sal_address_unref(referToAddr);
	op->unref();
}

void SalReferOp::fillCallbacks () {
	static belle_sip_listener_callbacks_t opReferCallbacks = { 0 };
	if (!opReferCallbacks.process_io_error) {
		opReferCallbacks.process_io_error = processIoErrorCb;
		opReferCallbacks.process_response_event = processResponseEventCb;
		opReferCallbacks.process_timeout = processTimeoutCb;
		opReferCallbacks.process_request_event = processRequestEventCb;
	}
	mCallbacks = &opReferCallbacks;
}

SalReferOp::SalReferOp (Sal *sal) : SalOp(sal) {
	mType = Type::Refer;
	fillCallbacks();
}

int SalReferOp::sendRefer (const SalAddress *referToAddr) {
	mDir = Dir::Outgoing;
	auto request = buildRequest("REFER");
	if (!request)
		return -1;
	if (getContactAddress())
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(createContact()));
	auto addressHeader = BELLE_SIP_HEADER_ADDRESS(referToAddr);
	auto uri = belle_sip_header_address_get_uri(addressHeader);
	if (!belle_sip_uri_get_host(uri))
		belle_sip_header_address_set_automatic(addressHeader, true);
	auto referToHeader = belle_sip_header_refer_to_create(addressHeader);
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(referToHeader));
	return sendRequest(request);
}

int SalReferOp::reply (SalReason reason) {
	if (!mPendingServerTransaction) {
		lError() << "SalReferOp::reply: no server transaction";
		return -1;
	}

	int code = toSipCode(reason);
	auto response = belle_sip_response_create_from_request(
		belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(mPendingServerTransaction)),
		code
	);
	belle_sip_server_transaction_send_response(mPendingServerTransaction, response);
	return 0;
}

LINPHONE_END_NAMESPACE
