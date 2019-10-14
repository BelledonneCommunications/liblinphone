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

#ifndef _L_SAL_REFER_OP_H_
#define _L_SAL_REFER_OP_H_

#include "sal/op.h"

LINPHONE_BEGIN_NAMESPACE

class SalReferOp : public SalOp {
public:
	SalReferOp (Sal *sal);

	int sendRefer (const SalAddress *referTo);
	int reply (SalReason reason);

private:
	void fillCallbacks () override;
	void processError ();

	static void processIoErrorCb (void *userCtx, const belle_sip_io_error_event_t *event);
	static void processResponseEventCb (void *userCtx, const belle_sip_response_event_t *event);
	static void processTimeoutCb (void *userCtx, const belle_sip_timeout_event_t *event);
	static void processRequestEventCb (void *userCtx, const belle_sip_request_event_t *event);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SAL_REFER_OP_H_
