/*
 * Copyright (c) 2021 Belledonne Communications SARL.
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
#include <mutex>

#include <ldap.h>

LINPHONE_BEGIN_NAMESPACE

class LdapContactSearch;
class LdapContactFields;

class LINPHONE_PUBLIC LdapContactProvider {
public:
	
	/**
	 * Current action of the provider following this flow:
	 * ACTION_NONE => ACTION_INIT => (ACTION_WAIT_DNS) => ACTION_INITIALIZE => ACTION_BIND => ACTION_WAIT_BIND => ACTION_WAIT_REQUEST =>]
	 */	
	enum{
		ACTION_ERROR = -1,	// Error State
		ACTION_NONE = 0,	// Do nothing
		ACTION_INIT,		// First step of initialization. Check domain and use Sal if requested.
		ACTION_WAIT_DNS,	// Sal is processing : wait till a result
		ACTION_INITIALIZE,	// Initialize the connection to the server
		ACTION_BIND,		// Bind to the server
		ACTION_WAIT_BIND,	// Wait for binding
		ACTION_WAIT_REQUEST	// Wait for processing search requests
	};
//	CREATION
	/**
	 * @brief LdapContactProvider Instance to make searches on a LDAP server. Use create() to get all usable #LdapContactProvider.
	 * Only LDAP/STARTTLS is supported.
	 * @param core The Linphone core for Configurations, thread processing and callbacks
	 * @param config The configuration to used. It can contains only required fields, all others will be filled by default values. Check #LdapConfigKeys to know what fields are requiered
	 */
	
	LdapContactProvider(const std::shared_ptr<Core> &core, const std::map<std::string,std::string> &config );
	virtual ~LdapContactProvider();
	
	/**
	 * @brief create Create a list of #LdapContactProvider from the core's configuration : The section name is 'ldap' and have an additionnal index to separate servers (eg. 'ldap_4'). 
	 * A section must be enabled with 'enable' to 1 to be taken account.
	 * @param core The Linphone core
	 * @return A list of #LdapContactProvider
	 */
	static std::vector<std::shared_ptr<LdapContactProvider> > create(const std::shared_ptr<Core> &core);
	
	/**
	 * @brief initializeLdap  Call ldap_initialize, set options and start TLS if needed.
	 */
	void initializeLdap();
	
//	CONFIGURATION
	/**
	 * @brief getTimeout it's a convertor from configuration 'timeout' to integer
	 * @return The timeout
	 */
	int getTimeout() const;

	/**
	 * @brief getFilter Get filter key from Configuration
	 * @return The filter as astring
	 */
	std::string getFilter()const;
	
	/**
	 * @brief getCurrentAction Get the current action of Iteration.
	 * @return Return ACTION_ERROR in case of error.
	 */
	int getCurrentAction()const;

//	SEARCH
	/**
	 * @brief search Make a search to the provider. Results are retrieved from the callback #ContactSearchCallback.
	 * This function is thread-safe.
	 * @param mPredicate A value to be used to replace a format specifier in the format string : 'filter' configuration key.
	 * This key is a string that contains a format string that follows the same specifications as format in printf.
	 * @param mCb The callback where to get results in the form of 'static void resultsCb( LinphoneContactSearch* id, bctbx_list_t* friends, void* data );'
	 * @param mCbData The data to pass to the callback
	 */
	void search(const std::string& mPredicate, ContactSearchCallback mCb, void* mCbData);
	
	/**
	 * @brief search Start the search to LDAP
	 * @param request Request instance that contains data to make a search.
	 * @return the result of ldap_search_ext (LDAP_SUCCESS on success)
	 */
	int search(std::shared_ptr<LdapContactSearch> request);
	
	/**
	 * @brief cancelSearch Remove the search from the list and call the callback
	 * @param request the request
	 * @return The new list iterator after the deletion of the request
	 */
	std::list<std::shared_ptr<LdapContactSearch> >::iterator cancelSearch(LdapContactSearch* request);
	
	/**
	 * @brief requestSearch Get the #LdapContactSearch linked to the ID
	 * @param msgid The ID of the search request
	 * @return  The #LdapContactSearch linked to the ID. NULL if no request has been found.
	 */
	LdapContactSearch* requestSearch( int msgid );
	
	/**
	 * @brief completeContact Fill LdapContactFields with the attribute. This function has to be call for each attributes.
	 * The function use 'sip_domain' to complete the SIP attribute (attr_value@domain). These options can be empty to avoid this behaviour.
	 * @param lf the contact to fill
	 * @param attr_name The attribute name from LDAP.
	 * @param attr_value The value from LDAP.
	 * @return 1 if all contact's fields are filled. 0 if some fields are missing.
	 */
	int completeContact( LdapContactFields* lf, const char* attr_name, const char* attr_value);

//	ASYNC PROCESSING

	/**
	 * @brief iterate This function is called from main loop to do actions that need to be serialized.
	 * @param data A pointer to #LdapContactProvider
	 * @return 
	 */
	static bool iterate(void *data);
	/**
	 * @brief stun_server_resolved Callback for DNS resolution with Sal. It retrieve the Sal's result and replace the server domain to the IP.
	 * @param data #LdapContactProvider
	 * @param results The address from Sal
	 */
	static void ldapServerResolved(void *data, belle_sip_resolver_results_t *results);
private:
	/**
	 * @brief handleSearchResult Parse the LDAPMessage to get contacts and fill Search entries.
	 * @param message LDAPMessage to parse
	 */
	void handleSearchResult( LDAPMessage* message );

	std::shared_ptr<Core> mCore;
	std::map<std::string,std::string>  mConfig;
	std::vector<std::string> mAttributes;	// Request optimization to limit attributes
	std::vector<std::string> mNameAttributes;// Optimization to avoid split each times
	std::vector<std::string> mSipAttributes;// Optimization to avoid split each times
	LDAP *mLd;
	std::mutex mLock;
	std::list<std::shared_ptr<LdapContactSearch> > mRequests;

	int mAwaitingMessageId; // Waiting Message for ldap_abandon_ext on bind
	bool_t mConnected;	// If we are connected to server (bind)
	int mCurrentAction; // Iteration action
	belle_sip_source_t * mIteration;	// Iteration loop
	belle_sip_resolver_context_t * mSalContext;	// Sal Context for DNS
	belle_generic_uri_t *mServerUri;//Used to optimized query on SAL
	std::string mServerUrl;	// URL to use for connection. It can be different from configuration
};

LINPHONE_END_NAMESPACE

#endif /* LINPHONE_LDAP_CONTACT_PROVIDER_H_ */
