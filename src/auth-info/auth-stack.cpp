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

#include "auth-stack.h"
#include "core/core-p.h"

#include "private_functions.h"

using namespace::std;

LINPHONE_BEGIN_NAMESPACE

AuthStack::AuthStack(CorePrivate & core) : mCore(core){
}

AuthStack::~AuthStack(){
	if (mTimer){
		mCore.getSal()->cancelTimer(mTimer);
		belle_sip_object_unref(mTimer);
		mTimer = nullptr;
	}
}

void AuthStack::pushAuthRequested(const std::shared_ptr<AuthInfo> &ai){
	if (mAuthBeingRequested) return;
	lInfo() << "AuthRequested pushed";
	mAuthQueue.push_back(ai);
	if (!mTimer){
		mTimer = mCore.getSal()->createTimer(&onTimeout, this, 0, "authentication requests");
	}
}

void AuthStack::authFound(const std::shared_ptr<AuthInfo> &ai){
	if (mAuthBeingRequested) return;
	lInfo() << "AuthStack::authFound() for " << ai->toString();
	for(auto it = mAuthQueue.begin(); it != mAuthQueue.end(); ){
		const shared_ptr<AuthInfo> &authInfo = (*it);
		if (authInfo->getRealm() == ai->getRealm() &&
			authInfo->getUsername() == ai->getUsername() &&
			authInfo->getDomain() == ai->getDomain()){
			lInfo() << "Authentication request removed.";
			it = mAuthQueue.erase(it);
		}else ++it;
	}
	if (mAuthQueue.empty()){
		lInfo() << "No need to request authentication information from application.";
		if (mTimer){
			mCore.getSal()->cancelTimer(mTimer);
			belle_sip_object_unref(mTimer);
			mTimer = nullptr;
		}
	}
}


void AuthStack::processAuthRequested(){
	/* The auth_info_requested() callback may cause the application to directly call linphone_core_add_auth_info(), which
	 * will re-invoke the auth_requsted callback of the SAL, which may call authFound() here.
	 * The mAuthBeingRequested flag is to inhinit this behavior.
	 */
	mAuthBeingRequested = true;
	for(const auto &authInfo : mAuthQueue){
		linphone_core_notify_authentication_requested(mCore.getCCore(), authInfo->toC(), LinphoneAuthHttpDigest);
		// Deprecated callback:
		linphone_core_notify_auth_info_requested(mCore.getCCore(), authInfo->getRealm().c_str(), authInfo->getUsername().c_str(), authInfo->getDomain().c_str());
	}
	mAuthQueue.clear();
	belle_sip_object_unref(mTimer);
	mTimer = nullptr;
	mAuthBeingRequested = false;
}

int AuthStack::onTimeout(void *data, unsigned int events){
	AuthStack *zis = static_cast<AuthStack*>(data);
	zis->processAuthRequested();
	return BELLE_SIP_STOP;
}


LINPHONE_END_NAMESPACE
