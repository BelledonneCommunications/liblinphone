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

#include "sal/event-op.h"
#include "c-wrapper/internal/c-tools.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void SalSubscribeOp::subscribeProcessIoErrorCb (void *userCtx, const belle_sip_io_error_event_t *event) {
	auto op = static_cast<SalSubscribeOp *>(userCtx);
	auto src = belle_sip_io_error_event_get_source(event);
	if (!BELLE_SIP_OBJECT_IS_INSTANCE_OF(src, belle_sip_client_transaction_t))
		return;

	auto clientTransaction = BELLE_SIP_CLIENT_TRANSACTION(src);
	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(clientTransaction));
	const string method(belle_sip_request_get_method(request));
	if (method == "NOTIFY") {
		sal_error_info_set(&op->mErrorInfo, SalReasonIOError, "SIP", 0, nullptr, nullptr);
		op->mRoot->mCallbacks.on_notify_response(op);
	}
}

void SalSubscribeOp::subscribeResponseEventCb (void *userCtx, const belle_sip_response_event_t *event) {
	auto op = static_cast<SalSubscribeOp *>(userCtx);
	auto statusCode = belle_sip_response_get_status_code(belle_sip_response_event_get_response(event));
	auto clientTransaction = belle_sip_response_event_get_client_transaction(event);
	if (!clientTransaction)
		return;

	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(clientTransaction));
	
	op->setOrUpdateDialog(belle_sip_response_event_get_dialog(event));
	
	const string method(belle_sip_request_get_method(request));
	if (op->mDialog) {
		switch (belle_sip_dialog_get_state(op->mDialog)) {
			case BELLE_SIP_DIALOG_NULL:
			case BELLE_SIP_DIALOG_EARLY:
				lInfo() << "SalSubscribeOp [" << op << "] received an unexpected answer [" << statusCode << "]";
				break;
			case BELLE_SIP_DIALOG_CONFIRMED:
				if (strcmp("SUBSCRIBE", belle_sip_request_get_method(request)) == 0) {
					auto expiresHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_expires_t);
					if (op->mRefresher) {
						belle_sip_refresher_stop(op->mRefresher);
						belle_sip_object_unref(op->mRefresher);
						op->mRefresher = nullptr;
					}
					if (expiresHeader && (belle_sip_header_expires_get_expires(expiresHeader) > 0)) {
						op->mRefresher = belle_sip_client_transaction_create_refresher(clientTransaction);
						belle_sip_refresher_set_listener(op->mRefresher, subscribeRefresherListenerCb, op);
						belle_sip_refresher_set_realm(op->mRefresher, L_STRING_TO_C(op->mRealm));
						belle_sip_refresher_enable_manual_mode(op->mRefresher, op->mManualRefresher);
					}
				}
				break;
			default:
				lInfo() << "SalSubscribeOp [" << op << "] received answer [" << statusCode << "]: not implemented";
				break;
		}
	}
	
	if (method == "NOTIFY") {
		op->setErrorInfoFromResponse(belle_sip_response_event_get_response(event));
		op->mRoot->mCallbacks.on_notify_response(op);
	} else if (method == "SUBSCRIBE") {
		auto response = belle_sip_transaction_get_response(BELLE_SIP_TRANSACTION(clientTransaction));
		op->handleSubscribeResponse((unsigned int)statusCode, belle_sip_response_get_reason_phrase(response), FALSE);
	}
}

void SalSubscribeOp::subscribeProcessTimeoutCb (void *userCtx, const belle_sip_timeout_event_t *event) {
	auto op = static_cast<SalSubscribeOp *>(userCtx);
	auto clientTransaction = belle_sip_timeout_event_get_client_transaction(event);
	if (!clientTransaction)
		return;

	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(clientTransaction));
	const string method(belle_sip_request_get_method(request));
	if (method == "NOTIFY") {
		sal_error_info_set(&op->mErrorInfo, SalReasonRequestTimeout, "SIP", 0, nullptr, nullptr);
		op->mRoot->mCallbacks.on_notify_response(op);
	}
}

void SalSubscribeOp::handleNotify (belle_sip_request_t *request, const char *eventName, SalBodyHandler *bodyHandler) {
	SalSubscribeStatus subscribeStatus = SalSubscribeActive;
	auto subscriptionStateHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_subscription_state_t);
	if (!subscriptionStateHeader
		|| (strcasecmp(BELLE_SIP_SUBSCRIPTION_STATE_TERMINATED, belle_sip_header_subscription_state_get_state(subscriptionStateHeader)) == 0)
	) {
		subscribeStatus = SalSubscribeTerminated;
		lInfo() << "Outgoing subscription terminated by remote [" << getTo() << "]";
	}

	ref();
	mRoot->mCallbacks.notify(this, subscribeStatus, eventName, bodyHandler);
	auto response = createResponseFromRequest(request, 200);
	belle_sip_server_transaction_send_response(mPendingServerTransaction, response);
	unref();
}

void SalSubscribeOp::subscribeProcessRequestEventCb (void *userCtx, const belle_sip_request_event_t *event) {
	auto op = static_cast<SalSubscribeOp *>(userCtx);
	auto serverTransaction = belle_sip_provider_create_server_transaction(op->mRoot->mProvider, belle_sip_request_event_get_request(event));

	belle_sip_object_ref(serverTransaction);
	if (op->mPendingServerTransaction)
		belle_sip_object_unref(op->mPendingServerTransaction);
	op->mPendingServerTransaction = serverTransaction;

	auto request = belle_sip_request_event_get_request(event);
	auto eventHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_event_t);
	if (!eventHeader) {
		lWarning() << "No event header in incoming SUBSCRIBE";
		auto response = op->createResponseFromRequest(request, 400);
		belle_sip_server_transaction_send_response(serverTransaction, response);
		if (!op->mDialog)
			op->release();
		return;
	}
	if (!op->mEvent) {
		op->mEvent = eventHeader;
		belle_sip_object_ref(op->mEvent);
	}

	const char *eventName = belle_sip_header_event_get_package_name(eventHeader);
	auto bodyHandler = BELLE_SIP_BODY_HANDLER(op->getBodyHandler(BELLE_SIP_MESSAGE(request)));
	const string method = belle_sip_request_get_method(request);
	if (!op->mDialog) {
		if (method == "SUBSCRIBE") {
			auto dialog = belle_sip_provider_create_dialog(op->mRoot->mProvider, BELLE_SIP_TRANSACTION(serverTransaction));
			if (!dialog) {
				auto response = op->createResponseFromRequest(request, 481);
				belle_sip_server_transaction_send_response(serverTransaction, response);
				op->release();
				return;
			}
			op->setOrUpdateDialog(dialog);
			lInfo() << "new incoming subscription from [" << op->getFrom() << "] to [" << op->getTo() << "]";
		} else {
			// This is a NOTIFY
			op->handleNotify(request, eventName, reinterpret_cast<SalBodyHandler *>(bodyHandler));
			return;
		}
	}

	const char *type = nullptr;
	auto contentTypeHeader = belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(request), belle_sip_header_content_type_t);
	auto expiresHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_expires_t);
	auto dialogState = belle_sip_dialog_get_state(op->mDialog);
	switch (dialogState) {
		case BELLE_SIP_DIALOG_NULL:
			if (contentTypeHeader)
				type = belle_sip_header_content_type_get_type(contentTypeHeader);
			op->mRoot->mCallbacks.subscribe_received(op, eventName, type ? reinterpret_cast<SalBodyHandler *>(bodyHandler) : nullptr);
			break;
		case BELLE_SIP_DIALOG_EARLY:
			lError() << "Unexpected method [" << method << "] for dialog [" << op->mDialog << "] in state BELLE_SIP_DIALOG_EARLY";
			break;
		case BELLE_SIP_DIALOG_CONFIRMED:
			if (method == "NOTIFY") {
				op->handleNotify(request, eventName, reinterpret_cast<SalBodyHandler *>(bodyHandler));
			} else if (method == "SUBSCRIBE") {
				// Either a refresh or an unsubscribe
				if (expiresHeader && belle_sip_header_expires_get_expires(expiresHeader) > 0) {
					auto response = op->createResponseFromRequest(request, 200);
					belle_sip_server_transaction_send_response(serverTransaction, response);
				} else if (expiresHeader) {
					lInfo() << "Unsubscribe received from [" << op->getFrom() << "]";
					auto response = op->createResponseFromRequest(request, 200);
					belle_sip_server_transaction_send_response(serverTransaction, response);
					op->mRoot->mCallbacks.incoming_subscribe_closed(op);
				}
			}
			break;
		default:
			lError() << "Unexpected dialog state [" << belle_sip_dialog_state_to_string(dialogState) << "]";
			break;
	}
}

void SalSubscribeOp::subscribeProcessDialogTerminatedCb (void *userCtx, const belle_sip_dialog_terminated_event_t *event) {
	auto op = static_cast<SalSubscribeOp *>(userCtx);
	if (!op->mDialog)
		return;
	if (op->mState == SalOp::State::Terminated) {
		lInfo() << "Op [" << op << "] is terminated, nothing to do with this dialog terminated";
	} else if (belle_sip_dialog_terminated_event_is_expired(event)) {
		auto dialog = belle_sip_dialog_terminated_event_get_dialog(event);
		if (belle_sip_dialog_is_server(dialog)) {
			op->mRoot->mCallbacks.incoming_subscribe_closed(op);
		} else {
			// Notify the app that our subscription is dead
			const char *eventName = nullptr;
			if (op->mEvent)
				eventName = belle_sip_header_event_get_package_name(op->mEvent);
			op->mRoot->mCallbacks.notify(op, SalSubscribeTerminated, eventName, nullptr);
		}
	}
	op->setOrUpdateDialog(nullptr);
}

void SalSubscribeOp::releaseCb (SalOp *op) {
	auto subscribeOp = static_cast<SalSubscribeOp *>(op);
	if(subscribeOp->mRefresher) {
		belle_sip_refresher_stop(subscribeOp->mRefresher);
		belle_sip_object_unref(subscribeOp->mRefresher);
		subscribeOp->mRefresher = nullptr;
		subscribeOp->setOrUpdateDialog(nullptr); // Only if we have refresher. else dialog terminated event will remove association
	}
}

SalSubscribeOp::SalSubscribeOp (Sal *sal) : SalEventOp(sal) {
	mType = Type::Subscribe;
	mReleaseCb = releaseCb;
}

void SalSubscribeOp::fillCallbacks () {
	static belle_sip_listener_callbacks_t opSubscribeCallbacks = { 0 };
	if (!opSubscribeCallbacks.process_io_error) {
		opSubscribeCallbacks.process_io_error = subscribeProcessIoErrorCb;
		opSubscribeCallbacks.process_response_event = subscribeResponseEventCb;
		opSubscribeCallbacks.process_timeout = subscribeProcessTimeoutCb;
		opSubscribeCallbacks.process_transaction_terminated = subscribeProcessTransactionTerminatedCb;
		opSubscribeCallbacks.process_request_event = subscribeProcessRequestEventCb;
		opSubscribeCallbacks.process_dialog_terminated = subscribeProcessDialogTerminatedCb;
	}
	mCallbacks = &opSubscribeCallbacks;
}

void SalSubscribeOp::subscribeRefresherListenerCb (belle_sip_refresher_t *refresher, void *userCtx, unsigned int statusCode, const char *reasonPhrase, int willRetry) {
	auto op = static_cast<SalSubscribeOp *>(userCtx);
	auto transaction = BELLE_SIP_TRANSACTION(belle_sip_refresher_get_transaction(refresher));
	op->setOrUpdateDialog(belle_sip_transaction_get_dialog(transaction));
	lInfo() << "Subscribe refresher [" << statusCode << "] reason [" << (reasonPhrase ? reasonPhrase : "none") << "]";
	op->handleSubscribeResponse(statusCode, reasonPhrase, willRetry);
}

void SalSubscribeOp::handleSubscribeResponse (unsigned int statusCode, const char *reasonPhrase, int willRetry) {
	SalSubscribeStatus sss = SalSubscribeTerminated;
	if ((statusCode >= 200) && (statusCode < 300)) {
		if (statusCode == 200)
			sss = SalSubscribeActive;
		else if (statusCode == 202)
			sss = SalSubscribePending;
		mRoot->mCallbacks.subscribe_response(this, sss, willRetry);
	} else if (statusCode >= 300) {
		SalReason reason = SalReasonUnknown;
		if (statusCode == 503) // Refresher returns 503 for IO error
			reason = SalReasonIOError;
		sal_error_info_set(&mErrorInfo, reason, "SIP", (int)(statusCode), reasonPhrase, nullptr);
		mRoot->mCallbacks.subscribe_response(this, sss, willRetry);
	} else if (statusCode == 0) {
		mRoot->mCallbacks.on_expire(this);
	}
}

int SalSubscribeOp::subscribe (const string &eventName, int expires, const SalBodyHandler *bodyHandler) {
	if (!mDialog) {
		fillCallbacks();
		auto request = buildRequest("SUBSCRIBE");
		if (!request)
			return -1;
		setEvent(eventName);
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(mEvent));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(belle_sip_header_expires_create(expires)));
		belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(request), BELLE_SIP_BODY_HANDLER(bodyHandler));
		//It is not possible to transfer control of the transaction to a refresher until dialog in state confirmed because in case of Notify received befor 200ok and subscribed challanged handled by the refresher, op set in the intial transaction app data is lost. 
		return sendRequest(request);
	} else if (mRefresher) {
		auto transaction = reinterpret_cast<const belle_sip_transaction_t *>(belle_sip_refresher_get_transaction(mRefresher));
		auto lastRequest = belle_sip_transaction_get_request(transaction);
		// Modify last request to update body
		belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(lastRequest), BELLE_SIP_BODY_HANDLER(bodyHandler));
		return belle_sip_refresher_refresh(mRefresher, expires);
	}

	lWarning() << "SalSubscribeOp::subscribe(): no dialog and no refresher?";
	return -1;
}

int SalSubscribeOp::accept () {
	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(mPendingServerTransaction));
	auto expiresHeader = belle_sip_message_get_header_by_type(request, belle_sip_header_expires_t);
	auto response = createResponseFromRequest(request, 200);
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(response), BELLE_SIP_HEADER(expiresHeader));
	belle_sip_server_transaction_send_response(mPendingServerTransaction, response);
	return 0;
}

int SalSubscribeOp::decline (SalReason reason) {
	auto response = belle_sip_response_create_from_request(
		belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(mPendingServerTransaction)),
		toSipCode(reason)
	);
	belle_sip_server_transaction_send_response(mPendingServerTransaction, response);
	return 0;
}

int SalSubscribeOp::notifyPendingState () {
	if (mDialog && mPendingServerTransaction) {
		ms_message("Sending NOTIFY with subscription state pending for op [%p]", this);
		auto request = belle_sip_dialog_create_request(mDialog, "NOTIFY");
		if (!request) {
			lError() << "Cannot create NOTIFY on op [" << this << "]";
			return -1;
		}
		if (mEvent)
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(mEvent));
		auto subscriptionStateHeader = belle_sip_header_subscription_state_new();
		belle_sip_header_subscription_state_set_state(subscriptionStateHeader, BELLE_SIP_SUBSCRIPTION_STATE_PENDING);
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(subscriptionStateHeader));
		return sendRequest(request);
	} else {
		lWarning() << "NOTIFY with subscription state pending for op [" << this
			<< "] not implemented in this case (either dialog pending trans does not exist";
	}

	return 0;
}

int SalSubscribeOp::notify (const SalBodyHandler *bodyHandler) {
	belle_sip_request_t *request = nullptr;
	if (mDialog) {
		request = belle_sip_dialog_create_queued_request(mDialog, "NOTIFY");
		if (!request)
			return -1;
	} else {
		fillCallbacks();
		request = buildRequest("NOTIFY");
	}

	if (mEvent)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(mEvent));

	belle_sip_message_add_header(
		BELLE_SIP_MESSAGE(request),
		mDialog
			? BELLE_SIP_HEADER(belle_sip_header_subscription_state_create(BELLE_SIP_SUBSCRIPTION_STATE_ACTIVE, 600))
			: BELLE_SIP_HEADER(belle_sip_header_subscription_state_create(BELLE_SIP_SUBSCRIPTION_STATE_TERMINATED, 0))
	);
	belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(request), BELLE_SIP_BODY_HANDLER(bodyHandler));
	return sendRequest(request);
}

int SalSubscribeOp::closeNotify () {
	if (!mDialog)
		return -1;

	auto request = belle_sip_dialog_create_queued_request(mDialog, "NOTIFY");
	if (!request)
		return -1;

	if (mEvent)
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(mEvent));
	belle_sip_message_add_header(
		BELLE_SIP_MESSAGE(request),
		BELLE_SIP_HEADER(belle_sip_header_subscription_state_create(BELLE_SIP_SUBSCRIPTION_STATE_TERMINATED, -1))
	);
	return sendRequest(request);
}



SalPublishOp::SalPublishOp (Sal *sal) : SalEventOp(sal) {
	mType = Type::Publish;
}

void SalPublishOp::publishResponseEventCb (void *userCtx, const belle_sip_response_event_t *event) {
	auto op = static_cast<SalPublishOp *>(userCtx);
	op->setErrorInfoFromResponse(belle_sip_response_event_get_response(event));
	if (op->mErrorInfo.protocol_code >= 200)
		op->mRoot->mCallbacks.on_publish_response(op);
}

void SalPublishOp::fillCallbacks () {
	static belle_sip_listener_callbacks_t opPublishCallbacks{};
	if (!opPublishCallbacks.process_response_event)
		opPublishCallbacks.process_response_event = publishResponseEventCb;

	mCallbacks = &opPublishCallbacks;
}

void SalPublishOp::publishRefresherListenerCb (belle_sip_refresher_t *refresher, void *userCtx, unsigned int statusCode, const char *reasonPhrase, int willRetry) {
	auto op = static_cast<SalPublishOp *>(userCtx);
	auto lastTransaction = belle_sip_refresher_get_transaction(op->mRefresher);
	auto response = belle_sip_transaction_get_response(BELLE_SIP_TRANSACTION(lastTransaction));
	lInfo() << "Publish refresher [" << statusCode << "] reason [" << (reasonPhrase ? reasonPhrase : "none") << "] for proxy [" << op->getProxy() << "]";
	if (statusCode == 0) {
		op->mRoot->mCallbacks.on_expire(op);
	} else if (statusCode >= 200) {
		string sipEtagStr;
		belle_sip_header_t *sipEtagHeader = nullptr;
		if (response) {
			sipEtagHeader = belle_sip_message_get_header(BELLE_SIP_MESSAGE(response), "SIP-ETag");
			if (sipEtagHeader)
				sipEtagStr = belle_sip_header_get_unparsed_value(sipEtagHeader);
		}
		op->setEntityTag(sipEtagStr);
		sal_error_info_set(&op->mErrorInfo, SalReasonUnknown, "SIP", static_cast<int>(statusCode), reasonPhrase, nullptr);
		op->assignRecvHeaders(BELLE_SIP_MESSAGE(response));
		op->mRoot->mCallbacks.on_publish_response(op);
	}
}

int SalPublishOp::publish (const string &eventName, int expires, const SalBodyHandler *bodyHandler) {
	if (!mRefresher || !belle_sip_refresher_get_transaction(mRefresher)) {
		fillCallbacks();
		auto request = buildRequest("PUBLISH");
		if (!request)
			return -1;

		if (!mEntityTag.empty())
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), belle_sip_header_create("SIP-If-Match", mEntityTag.c_str()));
		if (getContactAddress())
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(createContact()));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), belle_sip_header_create("Event", eventName.c_str()));
		belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(request), BELLE_SIP_BODY_HANDLER(bodyHandler));
		if (expires != -1)
			return sendRequestAndCreateRefresher(request, expires, publishRefresherListenerCb);
		else return sendRequest(request);
	} else {
		// Update status
		auto lastTransaction = belle_sip_refresher_get_transaction(mRefresher);
		auto lastRequest = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(lastTransaction));
		// update body
		if (expires == 0)
			belle_sip_message_set_body(BELLE_SIP_MESSAGE(lastRequest), nullptr, 0);
		else
			belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(lastRequest), BELLE_SIP_BODY_HANDLER(bodyHandler));
		return belle_sip_refresher_refresh(mRefresher, (expires == -1) ? BELLE_SIP_REFRESHER_REUSE_EXPIRES : expires);
	}
}

int SalPublishOp::unpublish () {
	if (!mRefresher)
		return -1;

	auto transaction = reinterpret_cast<const belle_sip_transaction_t *>(belle_sip_refresher_get_transaction(mRefresher));
	auto lastRequest = belle_sip_transaction_get_request(transaction);
	belle_sip_message_set_body(BELLE_SIP_MESSAGE(lastRequest), nullptr, 0);
	belle_sip_refresher_refresh(mRefresher, 0);
	return 0;
}

LINPHONE_END_NAMESPACE
