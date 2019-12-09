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

#ifndef _L_SAL_REGISTER_OP_H_
#define _L_SAL_REGISTER_OP_H_

#include "sal/op.h"

LINPHONE_BEGIN_NAMESPACE

class SalRegisterOp : public SalOp {
public:
	SalRegisterOp(Sal *sal) : SalOp(sal) { mType = Type::Register; }

	int sendRegister (const std::string &proxy, const std::string &from, int expires, const SalAddress *oldContact);
	int refreshRegister (int expires) {
		return mRefresher ? belle_sip_refresher_refresh(mRefresher, expires) : -1;
	}
	int unregister() { return refreshRegister(0); }

	void authenticate (const SalAuthInfo *info) override {
		mRoot->removePendingAuth(this);
		refreshRegister(-1);
	}

private:
	void fillCallbacks () override {};
	static void registerRefresherListener (belle_sip_refresher_t *refresher, void *userCtx, unsigned int statusCode, const char *reasonPhrase, int willRetry);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SAL_REGISTER_OP_H_
