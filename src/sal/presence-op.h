/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#ifndef _L_SAL_PRESENCE_OP_H_
#define _L_SAL_PRESENCE_OP_H_

#include "sal/event-op.h"

LINPHONE_BEGIN_NAMESPACE

class SalPresenceOp : public SalSubscribeOp {
public:
	SalPresenceOp(Sal *sal);

	int subscribe(int expires);
	int unsubscribe() {
		return SalOp::unsubscribe();
	}
	int notifyPresence(SalPresenceModel *presence);
	int notifyPresenceClose();

private:
	void fillCallbacks() override;
	void handleNotify(belle_sip_request_t *request, belle_sip_dialog_t *dialog);
	SalPresenceModel *processPresenceNotification(belle_sip_request_t *request);
	int checkDialogState();
	belle_sip_request_t *createPresenceNotify();
	void addPresenceInfo(belle_sip_message_t *notify, SalPresenceModel *presence);

	static SalSubscribeStatus getSubscriptionState(const belle_sip_message_t *message);

	static void presenceProcessIoErrorCb(void *userCtx, const belle_sip_io_error_event_t *event);
	static void presenceResponseEventCb(void *userCtx, const belle_sip_response_event_t *event);
	static void presenceRefresherListenerCb(belle_sip_refresher_t *refresher,
	                                        void *userCtx,
	                                        unsigned int statusCode,
	                                        const char *reasonPhrase,
	                                        int willRetry);
	static void presenceProcessTimeoutCb(void *userCtx, const belle_sip_timeout_event_t *event);
	static void presenceProcessTransactionTerminatedCb(void *userCtx,
	                                                   const belle_sip_transaction_terminated_event_t *event);
	static void presenceProcessRequestEventCb(void *userCtx, const belle_sip_request_event_t *event);
	static void presenceProcessDialogTerminatedCb(void *userCtx, const belle_sip_dialog_terminated_event_t *event);
	static void releaseCb(SalOp *op);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SAL_PRESENCE_OP_H_
