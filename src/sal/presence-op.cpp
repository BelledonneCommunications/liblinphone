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

#include "c-wrapper/internal/c-tools.h"
#include "sal/presence-op.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void SalPresenceOp::presenceProcessIoErrorCb (void *userCtx, const belle_sip_io_error_event_t *event) {
	auto op = static_cast<SalPresenceOp *>(userCtx);
	belle_sip_client_transaction_t *clientTransaction = nullptr;
	if (BELLE_SIP_OBJECT_IS_INSTANCE_OF(belle_sip_io_error_event_get_source(event), belle_sip_client_transaction_t))
		clientTransaction = BELLE_SIP_CLIENT_TRANSACTION(belle_sip_io_error_event_get_source(event));
	if (!clientTransaction)
		return;

	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(clientTransaction));
	if (strcmp("SUBSCRIBE", belle_sip_request_get_method(request)) == 0) {
		if (op->mRefresher) {
			lWarning() << "IO error in SalPresenceOp: refresher is present, should not happen";
			return;
		}
		lInfo() << "Subscription to [" << op->getTo() << "] io error";
		if (!op->mOpReleased)
			op->mRoot->mCallbacks.notify_presence(op, SalSubscribeTerminated, nullptr, nullptr); // Offline
	}
}

void SalPresenceOp::presenceRefresherListenerCb (belle_sip_refresher_t *refresher, void *userCtx, unsigned int statusCode, const char *reasonPhrase, int willRetry) {
	auto op = static_cast<SalPresenceOp *>(userCtx);
	if (statusCode >= 300) {
		lInfo() << "The SUBSCRIBE dialog no longer works, let's start a new one";
		belle_sip_refresher_stop(op->mRefresher);
		if (op->mDialog) { /*delete previous dialog if any*/
			op->setOrUpdateDialog(NULL);
		}

		if (op->getContactAddress()) {
			// Contact is also probably not good
			auto contact = sal_address_clone(op->getContactAddress());
			sal_address_set_port(contact, -1);
			sal_address_set_domain(contact, nullptr);
			op->setContactAddress(contact);
			sal_address_unref(contact);
		}
		// Send a new SUBSCRIBE, that will attempt to establish a new dialog
		op->subscribe(-1);
	} else if ((statusCode == 0) || (statusCode == 503)) {
		// Timeout or IO error: the remote doesn't seem reachable
		if (!op->mOpReleased)
			op->mRoot->mCallbacks.notify_presence(op, SalSubscribeActive, nullptr, nullptr); // Offline
	}
}

void SalPresenceOp::presenceResponseEventCb (void *userCtx, const belle_sip_response_event_t *event) {
	auto op = static_cast<SalPresenceOp *>(userCtx);
	auto clientTransaction = belle_sip_response_event_get_client_transaction(event);
	auto response = belle_sip_response_event_get_response(event);
	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(clientTransaction));
	int statusCode = belle_sip_response_get_status_code(response);

	op->setErrorInfoFromResponse(response);

	if (statusCode >= 300) {
		if (strcmp("SUBSCRIBE", belle_sip_request_get_method(request)) == 0) {
			lInfo() << "Subscription to [" << op->getTo() << "] rejected";
			if (!op->mOpReleased)
				op->mRoot->mCallbacks.notify_presence(op, SalSubscribeTerminated, nullptr, nullptr); // Offline
			return;
		}
	}
	op->setOrUpdateDialog(belle_sip_response_event_get_dialog(event));
	if (!op->mDialog) {
		lInfo() << "SalPresenceOp [" << op << "] received out of dialog answer [" << statusCode << "]";
		return;
	}

	switch (belle_sip_dialog_get_state(op->mDialog)) {
		case BELLE_SIP_DIALOG_NULL:
		case BELLE_SIP_DIALOG_EARLY:
			lInfo() << "SalPresenceOp [" << op << "] received an unexpected answer [" << statusCode << "]";
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
					belle_sip_refresher_set_listener(op->mRefresher, presenceRefresherListenerCb, op);
					belle_sip_refresher_set_realm(op->mRefresher, L_STRING_TO_C(op->mRealm));
				}
			}
			break;
		default:
			lInfo() << "SalPresenceOp [" << op << "] received answer [" << statusCode << "]: not implemented";
			break;
	}
}

void SalPresenceOp::presenceProcessTimeoutCb (void *userCtx, const belle_sip_timeout_event_t *event) {
	auto op = static_cast<SalPresenceOp *>(userCtx);
	auto clientTransaction = belle_sip_timeout_event_get_client_transaction(event);
	if (!clientTransaction)
		return;

	auto request = belle_sip_transaction_get_request(BELLE_SIP_TRANSACTION(clientTransaction));
	if (strcmp("SUBSCRIBE", belle_sip_request_get_method(request)) == 0) {
		lInfo() << "Subscription to [" << op->getTo() << "] timed out";
		if (!op->mOpReleased)
			op->mRoot->mCallbacks.notify_presence(op, SalSubscribeTerminated, nullptr, nullptr); // Offline
	}
}

void SalPresenceOp::presenceProcessTransactionTerminatedCb (void *userCtx, const belle_sip_transaction_terminated_event_t *event) {
	lInfo() << "SalPresenceOp::presenceProcessTransactionTerminatedCb not implemented yet";
}

SalPresenceModel *SalPresenceOp::processPresenceNotification (belle_sip_request_t *request) {
	auto contentTypeHeader = belle_sip_message_get_header_by_type(
		BELLE_SIP_MESSAGE(request),
		belle_sip_header_content_type_t
	);
	auto contentLengthHeader = belle_sip_message_get_header_by_type(
		BELLE_SIP_MESSAGE(request),
		belle_sip_header_content_length_t
	);
	if (!contentTypeHeader || !contentLengthHeader || (belle_sip_header_content_length_get_content_length(contentLengthHeader) == 0))
		return nullptr;

	const char *body = belle_sip_message_get_body(BELLE_SIP_MESSAGE(request));
	if (!body)
		return nullptr;

	SalPresenceModel *result = nullptr;
	if (!mOpReleased) {
		mRoot->mCallbacks.parse_presence_requested(
			this,
			belle_sip_header_content_type_get_type(contentTypeHeader),
			belle_sip_header_content_type_get_subtype(contentTypeHeader),
			body,
			&result
		);
	}

	return result;
}

void SalPresenceOp::handleNotify (belle_sip_request_t *request, belle_sip_dialog_t *dialog) {
	belle_sip_response_t *response = nullptr;
	
	if (strcmp("NOTIFY", belle_sip_request_get_method(request)) != 0)
		return;

	if (mDialog && (dialog != mDialog)) {
		lWarning() << "Receiving a NOTIFY from a dialog we haven't stored (op->dialog=" << mDialog << " dialog=" << dialog;
	}
	if (dialog == nullptr){
		lError() << "Out of dialog presence notify are not allowed.";
		response = createResponseFromRequest(request, 481);
		belle_sip_server_transaction_send_response(mPendingServerTransaction, response);
		return;
	}

	SalSubscribeStatus subscriptionState;
	auto subscriptionStateHeader = belle_sip_message_get_header_by_type(
		request,
		belle_sip_header_subscription_state_t
	);
	if (!subscriptionStateHeader
		|| (strcasecmp(BELLE_SIP_SUBSCRIPTION_STATE_TERMINATED, belle_sip_header_subscription_state_get_state(subscriptionStateHeader)) == 0)
	) {
		subscriptionState = SalSubscribeTerminated;
		lInfo() << "Outgoing subscription terminated by remote [" << getTo() << "]";
	} else {
		subscriptionState = getSubscriptionState(BELLE_SIP_MESSAGE(request));
	}

	ref(); // Take a ref because the notify_presence callback may release the op
	
	const char *body = belle_sip_message_get_body(BELLE_SIP_MESSAGE(request));
	auto presenceModel = processPresenceNotification(request);
	if (presenceModel || !body) {
		// Presence notification body parsed successfully
		response = createResponseFromRequest(request, 200); // Create first because the op may be destroyed by notify_presence
		if (!mOpReleased)
			mRoot->mCallbacks.notify_presence(this, subscriptionState, presenceModel, nullptr);
	} else if (body) {
		// Formatting error in presence notification body
		lWarning() << "Wrongly formatted presence document";
		response = createResponseFromRequest(request, 488);
	}
	if (response)
		belle_sip_server_transaction_send_response(mPendingServerTransaction, response);
	unref();
}

void SalPresenceOp::presenceProcessRequestEventCb (void *userCtx, const belle_sip_request_event_t *event) {
	auto op = static_cast<SalPresenceOp *>(userCtx);
	auto serverTransaction = belle_sip_provider_create_server_transaction(
		op->mRoot->mProvider,
		belle_sip_request_event_get_request(event)
	);
	auto request = belle_sip_request_event_get_request(event);
	const char *method = belle_sip_request_get_method(request);

	belle_sip_object_ref(serverTransaction);
	if (op->mPendingServerTransaction)
		belle_sip_object_unref(op->mPendingServerTransaction);
	op->mPendingServerTransaction = serverTransaction;

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

	if (!op->mDialog) {
		if (strcmp(method, "SUBSCRIBE") == 0) {
			auto dialog = belle_sip_provider_create_dialog(op->mRoot->mProvider, BELLE_SIP_TRANSACTION(serverTransaction));
			if (!dialog) {
				auto response = op->createResponseFromRequest(request, 481);
				belle_sip_server_transaction_send_response(serverTransaction, response);
				op->release();
				return;
			}
			op->setOrUpdateDialog(dialog);
			lInfo() << "New incoming subscription from [" << op->getFrom() << "] to [" << op->getTo() << "]";
		}else if ((strcmp(method, "NOTIFY") == 0) && belle_sip_request_event_get_dialog(event)) {
			// Special case of dialog created by notify matching subscribe
			op->setOrUpdateDialog(belle_sip_request_event_get_dialog(event));
		} else {
			// This is a NOTIFY
			lInfo() << "Receiving out of dialog notify";
			op->handleNotify(request, belle_sip_request_event_get_dialog(event));
			return;
		}
	}

	auto dialogState = belle_sip_dialog_get_state(op->mDialog);
	switch (dialogState) {
		case BELLE_SIP_DIALOG_NULL:
			if (strcmp("NOTIFY", method) == 0)
				op->handleNotify(request, belle_sip_request_event_get_dialog(event));
			else if (strcmp("SUBSCRIBE", method) == 0)
				op->mRoot->mCallbacks.subscribe_presence_received(op, op->getFrom().c_str());
			break;
		case BELLE_SIP_DIALOG_EARLY:
			lError() << "Unexpected method [" << method << "] for dialog [" << op->mDialog << "] in state BELLE_SIP_DIALOG_EARLY";
			break;
		case BELLE_SIP_DIALOG_CONFIRMED:
			if (strcmp("NOTIFY", method) == 0) {
				op->handleNotify(request, belle_sip_request_event_get_dialog(event));
			} else if (strcmp("SUBSCRIBE", method) == 0) {
				// Either a refresh or an unsubscribe.
				// If it is a refresh there is nothing to notify to the app. If it is an unSUBSCRIBE, then the dialog
				// will be terminated shortly, and this will be notified to the app through the dialog_terminated callback.
				auto response = op->createResponseFromRequest(request, 200);
				belle_sip_server_transaction_send_response(serverTransaction, response);
			}
			break;
		default:
			lError() << "Unexpected dialog state [" << belle_sip_dialog_state_to_string(dialogState) << "]";
			break;
	}
}

void SalPresenceOp::presenceProcessDialogTerminatedCb (void *userCtx, const belle_sip_dialog_terminated_event_t *event) {
	auto op = static_cast<SalPresenceOp *>(userCtx);
	if (!op->mDialog || !belle_sip_dialog_is_server(op->mDialog))
		return; // Client dialog is managed by refresher

	lInfo() << "Incoming subscribtion from [" << op->getFrom() << "] terminated";
	if (!op->mOpReleased)
		op->mRoot->mCallbacks.subscribe_presence_closed(op, op->getFrom().c_str());
	op->setOrUpdateDialog(nullptr);
}

void SalPresenceOp::releaseCb (SalOp *userCtx) {
	auto op = static_cast<SalPresenceOp *>(userCtx);
	if (op->mRefresher) {
		belle_sip_refresher_stop(op->mRefresher);
		belle_sip_object_unref(op->mRefresher);
		op->mRefresher = nullptr;
		op->setOrUpdateDialog(nullptr); // Only if we have refresher else dialog terminated event will remove association
	}
}

void SalPresenceOp::fillCallbacks () {
	static belle_sip_listener_callbacks_t opPresenceCallbacks = { 0 };
	if (!opPresenceCallbacks.process_request_event) {
		opPresenceCallbacks.process_io_error = presenceProcessIoErrorCb;
		opPresenceCallbacks.process_response_event = presenceResponseEventCb;
		opPresenceCallbacks.process_timeout = presenceProcessTimeoutCb;
		opPresenceCallbacks.process_transaction_terminated = presenceProcessTransactionTerminatedCb;
		opPresenceCallbacks.process_request_event = presenceProcessRequestEventCb;
		opPresenceCallbacks.process_dialog_terminated = presenceProcessDialogTerminatedCb;
	}
	mCallbacks = &opPresenceCallbacks;
}

SalPresenceOp::SalPresenceOp (Sal *sal) : SalSubscribeOp(sal) {
	mType = Type::Presence;
	mReleaseCb = releaseCb;
	fillCallbacks();
}

int SalPresenceOp::subscribe (int expires) {
	if (expires == -1) {
		if (mRefresher) {
			expires = belle_sip_refresher_get_expires(mRefresher);
			belle_sip_object_unref(mRefresher);
			mRefresher = nullptr;
		} else {
			lError() << "SalPresenceOp::subscribe(): cannot guess expires from previous refresher";
			return -1;
		}
	}

	if (!mEvent) {
		mEvent = belle_sip_header_event_create("presence");
		belle_sip_object_ref(mEvent);
	}
	belle_sip_parameters_remove_parameter(BELLE_SIP_PARAMETERS(mFromAddress), "tag");
	belle_sip_parameters_remove_parameter(BELLE_SIP_PARAMETERS(mToAddress), "tag");
	auto request = buildRequest("SUBSCRIBE");
	if (request) {
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(mEvent));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), BELLE_SIP_HEADER(belle_sip_header_expires_create(expires)));
	}

	return sendRequest(request);
}

int SalPresenceOp::checkDialogState () {
	auto dialogState = mDialog ? belle_sip_dialog_get_state(mDialog) : BELLE_SIP_DIALOG_NULL;
	if (dialogState == BELLE_SIP_DIALOG_CONFIRMED)
		return 0;

	lWarning() << "Cannot notify presence for op [" << this << "] because dialog is in state [" << belle_sip_dialog_state_to_string(dialogState) << "]";
	return -1;
}

belle_sip_request_t *SalPresenceOp::createPresenceNotify () {
	auto request = belle_sip_dialog_create_queued_request(mDialog, "NOTIFY");
	if (!request)
		return nullptr;

	belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), belle_sip_header_create("Event", "presence"));
	return request;
}

void SalPresenceOp::addPresenceInfo (belle_sip_message_t *notify, SalPresenceModel *presence) {
	char *content = nullptr;

	if (presence) {
		auto fromHeader = belle_sip_message_get_header_by_type(notify, belle_sip_header_from_t);
		char *contactInfo = belle_sip_uri_to_string(belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(fromHeader)));
		mRoot->mCallbacks.convert_presence_to_xml_requested(this, presence, contactInfo, &content);
		belle_sip_free(contactInfo);
		if (!content)
			return;
	}

	belle_sip_message_remove_header(BELLE_SIP_MESSAGE(notify), BELLE_SIP_CONTENT_TYPE);
	belle_sip_message_remove_header(BELLE_SIP_MESSAGE(notify), BELLE_SIP_CONTENT_LENGTH);
	belle_sip_message_set_body(BELLE_SIP_MESSAGE(notify), nullptr, 0);

	if (content) {
		size_t contentLength = strlen(content);
		belle_sip_message_add_header(
			BELLE_SIP_MESSAGE(notify),
			BELLE_SIP_HEADER(belle_sip_header_content_type_create("application", "pidf+xml"))
		);
		belle_sip_message_add_header(
			BELLE_SIP_MESSAGE(notify),
			BELLE_SIP_HEADER(belle_sip_header_content_length_create(contentLength))
		);
		belle_sip_message_set_body(BELLE_SIP_MESSAGE(notify), content, contentLength);
		ms_free(content);
	}
}

int SalPresenceOp::notifyPresence (SalPresenceModel *presence) {
	if (checkDialogState())
		return -1;

	auto request = createPresenceNotify();
	if (!request)
		return-1;

	addPresenceInfo(BELLE_SIP_MESSAGE(request), presence); // FIXME, what about expires??
	belle_sip_message_add_header(
		BELLE_SIP_MESSAGE(request),
		BELLE_SIP_HEADER(belle_sip_header_subscription_state_create(BELLE_SIP_SUBSCRIPTION_STATE_ACTIVE, 600))
	);
	return sendRequest(request);
}

int SalPresenceOp::notifyPresenceClose () {
	if (checkDialogState())
		return -1;

	auto request = createPresenceNotify();
	if (!request)
		return-1;

	addPresenceInfo(BELLE_SIP_MESSAGE(request), nullptr); // FIXME, what about expires??
	belle_sip_message_add_header(
		BELLE_SIP_MESSAGE(request),
		BELLE_SIP_HEADER(belle_sip_header_subscription_state_create(BELLE_SIP_SUBSCRIPTION_STATE_TERMINATED, -1))
	);

	int status = sendRequest(request);
	setOrUpdateDialog(nullptr); // Because we may be chalanged for the notify, so we must release dialog right now
	return status;
}

SalSubscribeStatus SalPresenceOp::getSubscriptionState (const belle_sip_message_t *message) {
	auto subscriptionStateHeader = belle_sip_message_get_header_by_type(message, belle_sip_header_subscription_state_t);
	if (!subscriptionStateHeader)
		return SalSubscribeNone;

	if (strcmp(belle_sip_header_subscription_state_get_state(subscriptionStateHeader), BELLE_SIP_SUBSCRIPTION_STATE_TERMINATED) == 0)
		return SalSubscribeTerminated;
	else if (strcmp(belle_sip_header_subscription_state_get_state(subscriptionStateHeader), BELLE_SIP_SUBSCRIPTION_STATE_PENDING) == 0)
		return SalSubscribePending;
	else if (strcmp(belle_sip_header_subscription_state_get_state(subscriptionStateHeader), BELLE_SIP_SUBSCRIPTION_STATE_ACTIVE) == 0)
		return SalSubscribeActive;

	return SalSubscribeNone;
}

LINPHONE_END_NAMESPACE
