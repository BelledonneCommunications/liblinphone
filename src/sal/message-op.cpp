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

#include "sal/message-op.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void SalMessageOp::processError () {
	if (mDir == Dir::Outgoing)
		mRoot->mCallbacks.message_delivery_update(this, SalMessageDeliveryFailed);
	else
		lWarning() << "Unexpected error for incoming message on op [" << this << "]";
	mState = State::Terminated;
}

void SalMessageOp::processIoErrorCb (void *userCtx, const belle_sip_io_error_event_t *event) {
	auto op = static_cast<SalMessageOp *>(userCtx);
	sal_error_info_set(&op->mErrorInfo, SalReasonIOError, "SIP", 503, "IO Error", nullptr);
	op->processError();
}

void SalMessageOp::processResponseEventCb (void *userCtx, const belle_sip_response_event_t *event) {
	auto op = static_cast<SalMessageOp *>(userCtx);
	int statusCode = belle_sip_response_get_status_code(belle_sip_response_event_get_response(event));

	op->setErrorInfoFromResponse(belle_sip_response_event_get_response(event));

	SalMessageDeliveryStatus status = SalMessageDeliveryFailed;
	if ((statusCode >= 100) && (statusCode < 200))
		status = SalMessageDeliveryInProgress;
	else if ((statusCode >= 200) && (statusCode < 300))
		status = SalMessageDeliveryDone;

	op->mRoot->mCallbacks.message_delivery_update(op, status);
}

void SalMessageOp::processTimeoutCb (void *userCtx, const belle_sip_timeout_event_t *event) {
	auto op = static_cast<SalMessageOp *>(userCtx);
	sal_error_info_set(&op->mErrorInfo, SalReasonRequestTimeout, "SIP", 408, "Request timeout", nullptr);
	op->processError();
}

void SalMessageOp::processRequestEventCb (void *userCtx, const belle_sip_request_event_t *event) {
	auto op = static_cast<SalMessageOp *>(userCtx);
	op->processIncomingMessage(event);
}

SalMessageOp::SalMessageOp (Sal *sal) : SalOp(sal) {
	mType = Type::Message;
	fillCallbacks();
}

void SalMessageOp::fillCallbacks () {
	static belle_sip_listener_callbacks_t opMessageCallbacks = { 0 };
	if (!opMessageCallbacks.process_io_error) {
		opMessageCallbacks.process_io_error = processIoErrorCb;
		opMessageCallbacks.process_response_event = processResponseEventCb;
		opMessageCallbacks.process_timeout = processTimeoutCb;
		opMessageCallbacks.process_request_event = processRequestEventCb;
	}
	mCallbacks = &opMessageCallbacks;
}

int SalMessageOp::sendMessage (const Content &content) {
	mDir = Dir::Outgoing;

	auto request = buildRequest("MESSAGE");
	if (!request)
		return -1;

	prepareMessageRequest(request, content);
	return sendRequest(request);
}

LINPHONE_END_NAMESPACE
