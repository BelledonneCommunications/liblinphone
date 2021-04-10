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

#ifndef LINPHONE_LDAP_CONTACT_PROVIDER_H_
#define LINPHONE_LDAP_CONTACT_PROVIDER_H_

#include "linphone/core.h"
#include "linphone/types.h"
#include "linphone/contactprovider.h"
#include "belle-sip/object++.hh"
#include "core/core.h"
#include "core/core-accessor.h"
#include <map>
#include <vector>
#include <string>
#include <list>

#include <ldap.h>

LINPHONE_BEGIN_NAMESPACE

class LDAPContactSearch;
class LDAPFriend;

class LINPHONE_PUBLIC LDAPContactProvider {
public:
	enum{
		STATE_ERROR = -1,
		STATE_OK = 0,
		STATE_CONNECTED
	};
	enum{
		ACTION_NONE = 0,
		ACTION_INIT,
		ACTION_WAIT_DNS,
		ACTION_INITIALIZE,
		ACTION_BIND,
		ACTION_WAIT_BIND,
		ACTION_WAIT_REQUEST
	};
//	CREATION
	LDAPContactProvider(const std::shared_ptr<Core> &core, const std::map<std::string,std::string> &config );
	virtual ~LDAPContactProvider();
	static std::vector<std::shared_ptr<LDAPContactProvider> > create(const std::shared_ptr<Core> &core);// Read configuration and create all providers from them
	void initializeLdap();	// call ldap_initialize and set options. Return mState
	
//	CONFIGURATION
	bool_t validConfig(const std::map<std::string, std::string>& dict)const;
	void loadConfig(const std::map<std::string, std::string>& config);
	int getTimeout() const;

//	SEARCH
	void search(const std::string& mPredicate, ContactSearchCallback mCb, void* mCbData);
	int search(std::shared_ptr<LDAPContactSearch> request);
	std::list<std::shared_ptr<LDAPContactSearch> >::iterator cancelSearch(LDAPContactSearch* req);
	LDAPContactSearch* requestSearch( int msgid );
	int completeContact( LDAPFriend* lf, const char* attr_name, const char* attr_value);

//	ASYNC PROCESSING
	
	static bool_t iterate(void *data);
	//static void* bind( void*arg);
	//int bindAsync();// Start a thread to bind
	void handleSearchResult( LDAPMessage* message );
	static void stun_server_resolved(void *data, belle_sip_resolver_results_t *results);

	std::shared_ptr<Core> mCore;
	std::map<std::string,std::string>  mConfig;
	std::vector<std::string> mAttributes;
	std::vector<std::string> mNameAttributes;// Optimization to avoid split each times
	std::vector<std::string> mSipAttributes;// Optimization to avoid split each times
	LDAP*   mLd;
	ortp_mutex_t mLock;
	std::list<std::shared_ptr<LDAPContactSearch> > mRequests;

	// bind transaction
	bool_t mConnected;
	std::string mName;
	belle_generic_uri_t *mServerUri;//Used to optimized query on SAL
	belle_sip_resolver_context_t * mSalContext;
	std::string mServerUrl;
	int mState;
	int mCurrentAction;
	int mAwaitingMessageId;
	
};

LINPHONE_END_NAMESPACE

#endif /* LINPHONE_LDAP_CONTACT_PROVIDER_H_ */
