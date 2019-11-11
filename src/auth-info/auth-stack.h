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
#ifndef AUTH_STACK_H
#define AUTH_STACK_H


#include "linphone/api/c-types.h"
#include "auth-info.h"

typedef struct belle_sip_source belle_sip_source_t;

LINPHONE_BEGIN_NAMESPACE

class CorePrivate;
/*
 * Help object to request authentication information to the app.
 * It solves the following problem:
 * Belle-sip requests authentication information sequentially by iterating on challenges
 * However, there might be for a same proxy, a challenge with SHA256 and another one with MD5, for the same user account.
 * Now let's imagine the core has been provisionned with a SHA256 password but MD5 is proposed first bby the server.
 * LinphoneCore will requests the app (in auth_info_requested() callback) the app for password (for MD5), while
 * it has a SHA256 authentication information.
 * This is annoying. This class solves the problem by deferring the auth_info_Requested() calls outside of the
 * belle-sip iteration on challenges.
 */
class AuthStack{
public:
	AuthStack(CorePrivate &core);
	AuthStack(const AuthStack &ref) = delete;
	void pushAuthRequested(const std::shared_ptr<AuthInfo> &ai);
	void authFound(const std::shared_ptr<AuthInfo> &ai);
	bool empty()const{
		return mAuthQueue.empty();
	}
	~AuthStack();
private:
	void processAuthRequested();
	CorePrivate &mCore;
	belle_sip_source_t *mTimer = nullptr;
	std::list<std::shared_ptr<AuthInfo>> mAuthQueue;
	bool mAuthBeingRequested = false;
	static int onTimeout(void *data, unsigned int events);
};

LINPHONE_END_NAMESPACE

#endif
