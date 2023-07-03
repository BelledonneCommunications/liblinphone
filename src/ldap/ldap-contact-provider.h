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

#ifndef LINPHONE_LDAP_CONTACT_PROVIDER_H_
#define LINPHONE_LDAP_CONTACT_PROVIDER_H_

#include "linphone/core.h"
#include "linphone/types.h"
#include "linphone/contactprovider.h"
#include "belle-sip/object++.hh"
#include "bctoolbox/port.h"
#include "core/core.h"
#include "core/core-accessor.h"
#include <map>
#include <vector>
#include <string>
#include <list>
#include <mutex>

#include <ldap.h>	// OpenLDAP
#include "../search/search-request.h"
#include "../search/magic-search.h"
#include "ldap.h"	// Linphone

LINPHONE_BEGIN_NAMESPACE

class LdapContactSearch;
class LdapContactFields;

class LINPHONE_PUBLIC LdapContactProvider {
public:
	
	/**
	 * Current action of the provider following this flow:
	 * ACTION_NONE => ACTION_INIT => (ACTION_WAIT_DNS) => ACTION_INITIALIZE => (ACTION_WAIT_TLS_CONNECT) => ACTION_BIND => ACTION_WAIT_BIND => ACTION_WAIT_REQUEST =>]
	 */	
	enum{
		ACTION_ERROR = -1,			// Error State
		ACTION_NONE = 0,			// Do nothing
		ACTION_INIT,				// First step of initialization. Check domain and use Sal if requested.
		ACTION_WAIT_DNS,			// Sal is processing : wait till a result
		ACTION_INITIALIZE,			// Initialize the connection to the server
		ACTION_WAIT_TLS_CONNECT,	// Wait TLS connection
		ACTION_BIND,				// Bind to the server
		ACTION_WAIT_BIND,			// Wait for binding
		ACTION_WAIT_REQUEST			// Wait for processing search requests
	};
//	CREATION
	/**
	 * @brief LdapContactProvider Instance to make searches on a LDAP server. Use create() to get all usable #LdapContactProvider.
	 * Only LDAP/STARTTLS is supported.
	 * @param core The Linphone core for Configurations, thread processing and callbacks
	 * @param ldap The LDAP server to use from linphone_core_get_ldap_list()
	 */
	
	LdapContactProvider(const std::shared_ptr<Core> &core, std::shared_ptr<Ldap> ldap, int maxResults);
	
	
	virtual ~LdapContactProvider();
	
	/**
	 * @brief create Create a list of #LdapContactProvider from the core's configuration : The section name is 'ldap' and have an additionnal index to separate servers (eg. 'ldap_4'). 
	 * A section must be enabled with 'enable' to 1 to be taken account.
	 * @param core The Linphone Core
	 * @param maxResults The max results to return
	 * @return A list of #LdapContactProvider
	 */
	static std::vector<std::shared_ptr<LdapContactProvider> > create(const std::shared_ptr<Core> &core, int maxResults);
	
	/**
	 * @brief initializeLdap  Call ldap_initialize, set options and start TLS if needed.
	 */
	void initializeLdap();
	
//	CONFIGURATION
	/**
	 * @brief getMinChars it's a convertor from configuration 'min_chars' to integer
	 * @return The minimum characters for one search.
	 */
	int getMinChars() const;
	
	/**
	 * @brief getTimeout it's a convertor from configuration 'timeout' to integer
	 * @return The timeout in seconds
	 */
	int getTimeout() const;
	
	/**
	 * @brief getDelay it's a convertor from configuration 'delay' to integer. Default is 500.
	 * @return The timeout in ms
	 */
	int getDelay() const;

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
	
	/**
	 * @brief getLdapServer Get the LDAP server that is coming from core
	 * @return The LDAP server. Return nullptr if not set.
	 */
	std::shared_ptr<Ldap> getLdapServer();

//	SEARCH
	/**
	 * @brief search Make a search to the provider. Results are retrieved from the callback #ContactSearchCallback.
	 * This function is thread-safe.
	 * @param predicate A value to be used to replace a format specifier in the format string : 'filter' configuration key.
	 * This key is a string that contains a format string that follows the same specifications as format in printf.
	 * @param cb The callback where to get results in the form of 'static void resultsCb( LinphoneContactSearch* id, bctbx_list_t* searchResults, void* data );'
	 * @param cbData The data to pass to the callback
	 * @param requestHistory The list of search that have been requested. It is used to make a delay between the same kind of searchs.
	 * @return true if the request can be processed.
	 */
	bool search(const std::string& predicate, ContactSearchCallback cb, void* cbData, const std::list<SearchRequest>& requestHistory = std::list<SearchRequest>());
	
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
	
	/**
	 * @brief ldapTlsConnection Procedure to Start a TLS connection using mTlsConnectionTimeout.
	 */
	void ldapTlsConnection();
	
	/**
	 * @brief isReadyForStart check if the search can be started from mLastRequestTime.
	 */
	bool isReadyForStart();
	
	/**
	 * @brief computeLastRequestTime Compute the last request time on LDAP servers, from a list of request.
	 */
	void computeLastRequestTime(const std::list<SearchRequest>& requestHistory);

	std::shared_ptr<Core> mCore;
	std::shared_ptr<Ldap> mLdapServer;		// The LDAP server coming from core if set. Useful to know what server is using.
	std::map<std::string,std::string>  mConfig;
	std::vector<std::string> mAttributes;	// Request optimization to limit attributes
	std::vector<std::string> mNameAttributes;// Optimization to avoid split each times
	std::vector<std::string> mSipAttributes;// Optimization to avoid split each times
	LDAP *mLd;
	std::list<std::shared_ptr<LdapContactSearch> > mRequests;
	int mMaxResults;

	int mAwaitingMessageId; // Waiting Message for ldap_abandon_ext on bind
	bool_t mConnected;	// If we are connected to server (bind)
	int mCurrentAction; // Iteration action
	belle_sip_source_t * mIteration;	// Iteration loop
	belle_sip_resolver_context_t * mSalContext;	// Sal Context for DNS
	belle_generic_uri_t *mServerUri;//Used to optimized query on SAL
	std::string mServerUrl;	// URL to use for connection. It can be different from configuration

// TLS connection
	int mTlsConnectionId = -1;	// Used for getting async results from a start_tls
	time_t mTlsConnectionTimeout;
	
	uint64_t mLastRequestTime; // Store bctbx_get_cur_time_ms and use it as reference to make a delay between LDAP requests. 
};

LINPHONE_END_NAMESPACE

#endif /* LINPHONE_LDAP_CONTACT_PROVIDER_H_ */
