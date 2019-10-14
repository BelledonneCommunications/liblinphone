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

#ifndef _L_SAL_EVENT_OP_H_
#define _L_SAL_EVENT_OP_H_

#include "sal/op.h"

LINPHONE_BEGIN_NAMESPACE

class SalEventOp : public SalOp {
public:
	SalEventOp (Sal *sal) : SalOp(sal) {}
};

class SalSubscribeOp: public SalEventOp {
public:
	SalSubscribeOp (Sal *sal);

	int subscribe (const std::string &eventName, int expires, const SalBodyHandler *bodyHandler);
	int unsubscribe () { return SalOp::unsubscribe(); }
	int accept ();
	int decline (SalReason reason);
	int notifyPendingState ();
	int notify (const SalBodyHandler *bodyHandler);
	int closeNotify ();

private:
	void fillCallbacks () override;
	void handleNotify (belle_sip_request_t *request, const char *eventName, SalBodyHandler *bodyHandler);
	void handleSubscribeResponse (unsigned int statusCode, const char *reasonPhrase, int willRetry);

	static void subscribeProcessIoErrorCb (void *userCtx, const belle_sip_io_error_event_t *event);
	static void subscribeResponseEventCb (void *userCtx, const belle_sip_response_event_t *event);
	static void subscribeProcessTimeoutCb (void *userCtx, const belle_sip_timeout_event_t *event);
	static void subscribeProcessTransactionTerminatedCb (void *userCtx, const belle_sip_transaction_terminated_event_t *event) {}
	static void subscribeProcessRequestEventCb (void *userCtx, const belle_sip_request_event_t *event);
	static void subscribeProcessDialogTerminatedCb (void *userCtx, const belle_sip_dialog_terminated_event_t *event);
	static void releaseCb (SalOp *op);
	static void subscribeRefresherListenerCb (belle_sip_refresher_t *refresher, void *userCtx, unsigned int statusCode, const char *reasonPhrase, int willRetry);
};

class SalPublishOp : public SalEventOp {
public:
	SalPublishOp (Sal *sal);

	int publish (const std::string &eventName, int expires, const SalBodyHandler *bodyHandler);
	int unpublish ();

private:
	void fillCallbacks () override;

	static void publishResponseEventCb (void *userCtx, const belle_sip_response_event_t *event);
	static void publishRefresherListenerCb (belle_sip_refresher_t *refresher, void *userCtx, unsigned int statusCode, const char *reasonPhrase, int willRetry);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SAL_EVENT_OP_H_
