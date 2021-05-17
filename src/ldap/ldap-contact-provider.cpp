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

#include "ldap-contact-provider.h"

#include "ldap-config-keys.h"
#include "ldap-contact-search.h"
#include "ldap-contact-fields.h"
#include "contact_providers_priv.h"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>

// TODO: From coreapi. Remove me later.
#include "private.h"

#define FILTER_MAX_SIZE      512

LINPHONE_BEGIN_NAMESPACE

// Custom redefinitions

//*******************************************	CREATION

LdapContactProvider::LdapContactProvider(const std::shared_ptr<Core> &core, const std::map<std::string,std::string> &config ){
	mAwaitingMessageId = 0;
	mConnected = FALSE;
	mCore = core;
	mLd = nullptr;
	mSalContext = NULL;
	mServerUri = NULL;

	// register our hook into iterate so that LDAP can do its magic asynchronously.
	mIteration = mCore->createTimer(std::bind(&LdapContactProvider::iterate, this), 50, "LdapContactProvider");
	if( !LdapConfigKeys::validConfig(config) ) {
		ms_error( "[LDAP] Invalid configuration for LDAP, aborting creation");
		mCurrentAction = ACTION_ERROR;
	} else {
		mConfig = LdapConfigKeys::loadConfig(config, &mNameAttributes, &mSipAttributes, &mAttributes);
		mCurrentAction = ACTION_NONE;
	}
}

LdapContactProvider::~LdapContactProvider(){
	if(mIteration){
		mCore->destroyTimer(mIteration);
		mIteration = nullptr;
	}
// Wait for bind thread to end
	mLock.lock();
	if (mSalContext) {
		belle_sip_resolver_context_cancel(mSalContext);
		belle_sip_object_unref(mSalContext);
		mSalContext = NULL;
	}
	if(mConnected==1)// We have been bind. Clean the exit
		ldap_unbind_ext_s(mLd, NULL, NULL);
	if(mAwaitingMessageId > 0){//There is currently a request that has not been processed. Abandon it.
			ldap_abandon_ext(mLd, mAwaitingMessageId, NULL, NULL );
			mAwaitingMessageId = 0;
	}
	if( mServerUri){
		belle_sip_object_unref(mServerUri);
		mServerUri = NULL;
	}
	mLock.unlock();
}

std::vector<std::shared_ptr<LdapContactProvider> > LdapContactProvider::create(const std::shared_ptr<Core> &core){
	std::vector<std::shared_ptr<LdapContactProvider> > providers;
	LpConfig * lConfig = linphone_core_get_config(core->getCCore());
// Read configuration
	const bctbx_list_t * bcSections = linphone_config_get_sections_names_list(lConfig);
	for(auto itSections = bcSections; itSections; itSections=itSections->next) {
		std::string section = static_cast<char *>(itSections->data);
		std::string sectionName;
		size_t sectionNameIndex = section.length()-1;
		while(sectionNameIndex > 0 && section[sectionNameIndex] != '_')// Get the name strip number
			--sectionNameIndex;
		if( sectionNameIndex > 0 ){
			sectionName = section.substr(0,sectionNameIndex);
		}else
			sectionName = section;
		if(sectionName == "ldap"){
			std::map<std::string, std::string> config;
			const bctbx_list_t * keys = linphone_config_get_keys_names_list(lConfig, section.c_str());
			for(auto itKeys = keys ; itKeys ; itKeys=itKeys->next){
				std::string key = static_cast<char *>(itKeys->data);
				config[key] = linphone_config_get_string(lConfig, section.c_str(), key.c_str(), "");
			}
			if(config["enable"] == "1")
				providers.push_back(std::make_shared<LdapContactProvider>(core, config));
		}
	}
	return providers;
}

void LdapContactProvider::initializeLdap(){
	int proto_version = LDAP_VERSION3;
	int ret = ldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &proto_version);
	mCurrentAction = ACTION_NONE;
	if( ret != LDAP_SUCCESS )
		ms_error( "[LDAP] Problem initializing default Protocol version to 3 : %x (%s)", ret, ldap_err2string(ret));
// Setting global options for the next initialization. These options cannot be done with the LDAP instance directly.
	if(mConfig.count("debug")>0 && mConfig["debug"] == "1"){
		int debLevel = 7;
		ret = ldap_set_option(NULL, LDAP_OPT_DEBUG_LEVEL, &debLevel);
		if( ret != LDAP_SUCCESS )
			ms_error( "[LDAP] Problem initializing debug options to mode 7 : %x (%s)", ret, ldap_err2string(ret));	
	}
	if(mConfig.count("use_tls")>0 && mConfig["use_tls"] == "1"){
		std::string caFile = linphone_core_get_root_ca(mCore->getCCore());
		bool enableVerification = true;
		if( mConfig.count("verify_server_certificates")>0){
			if( mConfig["verify_server_certificates"] == "-1")
				enableVerification = linphone_core_is_verify_server_certificates(mCore->getCCore());
			else if( mConfig["verify_server_certificates"] == "0")
				enableVerification = false;
		}
		int reqcert = (enableVerification?LDAP_OPT_X_TLS_DEMAND:LDAP_OPT_X_TLS_ALLOW);
		int reqsan = LDAP_OPT_X_TLS_ALLOW;
		ret = ldap_set_option (NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &reqcert);
		if( ret != LDAP_SUCCESS )
			ms_error( "[LDAP] Problem initializing TLS on setting require certification '%s': %x (%s)", mConfig["server"].c_str(),ret, ldap_err2string(ret));
		ret = ldap_set_option (NULL, LDAP_OPT_X_TLS_CACERTFILE,caFile.c_str());
		if( ret != LDAP_SUCCESS )
			ms_error( "[LDAP] Problem initializing TLS on setting CA Certification file '%s': %x (%s)", mConfig["server"].c_str(),ret, ldap_err2string(ret));
		ret = ldap_set_option (NULL, LDAP_OPT_X_TLS_REQUIRE_SAN,&reqsan);
		if( ret != LDAP_SUCCESS )
			ms_error( "[LDAP] Problem initializing TLS on setting require SAN '%s': %x (%s)", mConfig["server"].c_str(),ret, ldap_err2string(ret));
	}
	ret = ldap_initialize(&mLd, mServerUrl.c_str());// Trying to connect even on error on options
	
	if( ret != LDAP_SUCCESS ){
		ms_error( "[LDAP] Problem initializing ldap on url '%s': %x (%s)", mConfig["server"].c_str(),ret, ldap_err2string(ret));
		mCurrentAction = ACTION_ERROR;
	} else if( (ret = ldap_set_option(mLd, LDAP_OPT_PROTOCOL_VERSION, &proto_version)) != LDAP_SUCCESS ){
		ms_error( "[LDAP] Problem setting protocol version %d: %x (%s)", proto_version,ret, ldap_err2string(ret));
		mCurrentAction = ACTION_ERROR;
	} else {
		
		if(mConfig.count("use_tls")>0 && mConfig["use_tls"] == "1"){
			if(mConfig.count("use_sal")>0 && mConfig["use_sal"] == "1"){// Using Sal give an IP for a domain. So check the domain rather than the IP.
				belle_generic_uri_t *serverUri = belle_generic_uri_parse(mConfig["server"].c_str());
				std::string hostname = belle_generic_uri_get_host(serverUri);
				std::vector<char> cHostname(hostname.c_str(), hostname.c_str() + hostname.size() + 1);
				ldap_set_option(mLd, LDAP_OPT_X_TLS_PEER_CN, &cHostname[0]);
			}
			ret = ldap_start_tls_s(mLd, NULL, NULL);
		}
		if( ret == LDAP_SUCCESS ) {
			ms_debug("[LDAP] Initialization success");
			mConnected = 1;
		}else {
			int err;
			ldap_get_option(mLd, LDAP_OPT_RESULT_CODE, &err);
			ms_error("[LDAP] Cannot initialize address to %s : %x, err %x (%s)",mConfig["server"].c_str(), ret, err, ldap_err2string(err));
			mCurrentAction = ACTION_ERROR;
		}
	}
}

int LdapContactProvider::getTimeout() const{
	return atoi(mConfig.at("timeout").c_str());
}

std::string LdapContactProvider::getFilter()const{
	return mConfig.at("filter");
}

int LdapContactProvider::getCurrentAction()const{
	return mCurrentAction;
}
//*******************************************	SEARCH

// Create a search object and store the request to be used when the provider is ready
void LdapContactProvider::search(const std::string& predicate, ContactSearchCallback cb, void* cbData){
	std::shared_ptr<LdapContactSearch> request = std::make_shared<LdapContactSearch>(this, predicate, cb, cbData );
	mLock.lock();
	if( request != NULL ) {
		mRequests.push_back(request);
	}
	mLock.unlock();
}

// Start the search
int LdapContactProvider::search(std::shared_ptr<LdapContactSearch> request){
	int ret = -1;
	struct timeval timeout = { atoi(mConfig["timeout"].c_str()), 0 };

	if( request->mMsgId == 0 ){
		ret = ldap_search_ext(mLd,
						mConfig["base_object"].c_str(),// base from which to start
						LDAP_SCOPE_SUBTREE,
						request->mFilter.c_str(),     // search predicate
						NULL,//(char**)attributes, // which attributes to get
						0,               // 0 = get attrs AND value, 1 = get attrs only
						NULL,
						NULL,
						&timeout,        // server timeout for the search
						atoi(mConfig["max_results"].c_str()),// max result number
						&request->mMsgId );
		if( ret != LDAP_SUCCESS ){
			ms_error("[LDAP] Error ldap_search_ext returned %d (%s)", ret, ldap_err2string(ret));
		} else {
			ms_debug("[LDAP] LinphoneLdapContactSearch created @%p : msgid %d", request.get(), request->mMsgId);
		}

	} else {
		ms_warning( "[LDAP] Search already performed for %s, msgid %d", request->mFilter.c_str(), request->mMsgId);
	}
	return ret;
}

// Call callback, mark it as completed and remove the request
std::list<std::shared_ptr<LdapContactSearch> >::iterator LdapContactProvider::cancelSearch(LdapContactSearch * request) {
	std::list<std::shared_ptr<LdapContactSearch>>::iterator listEntry = std::find_if(mRequests.begin(), mRequests.end(),[request] (const std::shared_ptr<LdapContactSearch>& a) 
		{ return (a->mMsgId == request->mMsgId) && (a.get() == request); }
	);
	
	if( listEntry != mRequests.end()) {
		ms_debug("[LDAP] Delete search %p", request);
		if( !(*listEntry)->complete){
			(*listEntry)->complete = TRUE;
			(*listEntry)->callCallback();
		}
		listEntry = mRequests.erase(listEntry);
	} else {
		ms_warning("[LDAP] Couldn't find ldap request %p (id %d) in monitoring.", request, request->mMsgId);
	}
	return listEntry;
}

LdapContactSearch* LdapContactProvider::requestSearch( int msgid ) {
	auto listEntry = std::find_if(mRequests.begin(), mRequests.end(), [msgid](const std::shared_ptr<LdapContactSearch>& a ){
		return a->mMsgId == msgid;
	});
	if( listEntry != mRequests.end() ) 
		return listEntry->get();
	else 
		return NULL;
}

int LdapContactProvider::completeContact( LdapContactFields* contact, const char* attr_name, const char* attr_value) {
	// These loops follow the priority rule on position in attributes array. The first item is better than the last.
	for(size_t attributeIndex = 0 ; attributeIndex < mNameAttributes.size() && (contact->mName.second < 0 || (std::string(attr_value) != "" && contact->mName.second > (int)attributeIndex)) ; ++attributeIndex){
		if( attr_name == mNameAttributes[attributeIndex]){
			contact->mName.first = attr_value;
			contact->mName.second = (int)attributeIndex;
		}
	}
	for(size_t attributeIndex = 0 ; attributeIndex < mSipAttributes.size() && (contact->mSip.second < 0 || (std::string(attr_value) != "" && contact->mSip.second >= (int)attributeIndex)) ; ++attributeIndex){
		if( attr_name == mSipAttributes[attributeIndex]){// Complete SIP with custom data (scheme and domain)
			std::string sip;
			sip += attr_value;
// Test if this sip is ok	
			LinphoneAddress* la = linphone_core_interpret_url(mCore->getCCore(), sip.c_str());
			if( !la){
			}else{
				if(mConfig.count("sip_domain")>0 && mConfig.at("sip_domain") != "")
					linphone_address_set_domain(la, mConfig.at("sip_domain").c_str());
				char *newSip = linphone_address_as_string(la);
				if( contact->mSip.second != (int)attributeIndex){
					contact->mSip.first.clear();
					contact->mSip.second = (int)attributeIndex;
				}
				contact->mSip.first.push_back(newSip);
				contact->mSip.second = (int)attributeIndex;
				ms_free(newSip);
				linphone_address_unref(la);
			}
		}
	}
	// return 1 if the structure has enough data to create a linphone friend
	if( contact->mName.second >= 0 && contact->mSip.second >= 0 )
		return 1;
	else
		return 0;
}

//*******************************************	ASYNC PROCESSING
// ACTION_NONE => ACTION_INIT => (ACTION_WAIT_DNS) => ACTION_INITIALIZE => ACTION_BIND => ACTION_WAIT_BIND => ACTION_WAIT_REQUEST
bool LdapContactProvider::iterate(void *data) {
	struct timeval pollTimeout = {0,0};
	LDAPMessage* results = NULL;
	LdapContactProvider* provider = (LdapContactProvider*)data;

	if(provider->mCurrentAction == ACTION_ERROR){
		provider->handleSearchResult(NULL );
	}else{
		// not using switch is wanted : we can do severals steps in one iteration if wanted.
		if(provider->mCurrentAction == ACTION_NONE){
			ms_debug("[LDAP] ACTION_NONE");
			provider->mLock.lock();
			if( provider->mRequests.size() > 0){
				if( provider->mCurrentAction != ACTION_ERROR){
					if( provider->mConnected != 1)
						provider->mCurrentAction = ACTION_INIT;
					else
						provider->mCurrentAction = ACTION_BIND;
				}
			}
			provider->mLock.unlock();
		}

		if(provider->mCurrentAction == ACTION_INIT){
			ms_debug("[LDAP] ACTION_INIT");
			if(provider->mConfig["use_sal"] == "0"){
				provider->mServerUrl = provider->mConfig["server"];
				provider->mCurrentAction = ACTION_INITIALIZE;
			}else{
				provider->mServerUri = belle_generic_uri_parse(provider->mConfig["server"].c_str());
				if(provider->mServerUri){
					belle_sip_object_ref(provider->mServerUri);
					std::string domain = belle_generic_uri_get_host(provider->mServerUri);
					int port = belle_generic_uri_get_port(provider->mServerUri);
					if(port  <= 0){
						if( belle_generic_uri_get_scheme(provider->mServerUri) == std::string("ldap"))
							port = 389;
						else
							port = 636;
						belle_generic_uri_set_port(provider->mServerUri, port);
					}
					provider->mSalContext = provider->mCore->getCCore()->sal->resolveA(domain.c_str(), port, AF_INET, ldapServerResolved, provider);
					if (provider->mSalContext){
						belle_sip_object_ref(provider->mSalContext);
						provider->mCurrentAction = ACTION_WAIT_DNS;
					}else// We cannot use Sal. Try another iteration to use it by not setting any new action
						ms_error("[LDAP] Cannot request DNS : no context for Sal.");
				}else{
					ms_error("[LDAP] Cannot parse the server to URI : %s", provider->mConfig["server"].c_str());
					provider->mCurrentAction = ACTION_ERROR;
				}
			}
		}

		if(provider->mCurrentAction == ACTION_WAIT_DNS){
			// Do nothing, wait for SAL to do its work
			ms_debug("[LDAP] ACTION_WAIT_DNS");
		}

		if(provider->mCurrentAction == ACTION_INITIALIZE){
			ms_debug("[LDAP] ACTION_INITIALIZE");
			if (provider->mSalContext) {
				belle_sip_object_unref(provider->mSalContext);
				provider->mSalContext = NULL;
			}
			provider->initializeLdap();
			if( provider->mServerUri){
				belle_sip_object_unref(provider->mServerUri);
				provider->mServerUri = NULL;
			}
		}

		if(provider->mCurrentAction == ACTION_BIND){ // Careful : Binds are not thread-safe
			ms_debug("[LDAP] ACTION_BIND");
			std::string auth_mechanism = provider->mConfig.at("auth_method");
			int ret=0;
			if( (auth_mechanism == "ANONYMOUS") || (auth_mechanism == "SIMPLE") ) {
				std::string bindDn, password;
				if( auth_mechanism == "SIMPLE" && provider->mConfig.count("bind_dn") > 0) {
					bindDn = provider->mConfig.at("bind_dn");
					password = provider->mConfig.at("password");
				}//else : anonymous connection
				struct berval passwd = { (ber_len_t)password.length(), ms_strdup(password.c_str())};
				ret = ldap_sasl_bind(provider->mLd, bindDn.c_str(), NULL, &passwd, NULL, NULL, &provider->mAwaitingMessageId);
				ms_free(passwd.bv_val);
				if( ret == LDAP_SUCCESS ) {
					provider->mCurrentAction = ACTION_WAIT_BIND;
				} else {
					int err=0;
					ldap_get_option(provider->mLd, LDAP_OPT_RESULT_CODE, &err);
					ms_error("[LDAP] ldap_sasl_bind error returned %x, err %x (%s), auth_method: %s", ret, err, ldap_err2string(err), auth_mechanism.c_str() );
					provider->mCurrentAction = ACTION_ERROR;
					provider->mAwaitingMessageId = 0;
				}
			} else {
				ms_error("[LDAP] Special authentifications is not supported. You must use SIMPLE or ANONYMOUS");
				provider->mCurrentAction = ACTION_ERROR;
				provider->mAwaitingMessageId = 0;
			}
		}

		if(provider->mCurrentAction == ACTION_WAIT_BIND){
			ms_debug("[LDAP] ACTION_WAIT_BIND");
			int ret = (int)ldap_result(provider->mLd, provider->mAwaitingMessageId, LDAP_MSG_ONE, &pollTimeout, &results);
			if(ret == LDAP_RES_BIND){
				ret = ldap_parse_sasl_bind_result( provider->mLd, results,NULL, 1); // Auto ldap_msgfree(results)
				if( ret == LDAP_SUCCESS){
					ms_debug("[LDAP] Bind successful");
					provider->mConnected = 1;
					provider->mAwaitingMessageId = 0;
					provider->mCurrentAction = ACTION_WAIT_REQUEST;
				}else{
					ms_error("[LDAP] Cannot bind to server : %x (%s)", ret, ldap_err2string(ret));
					provider->mCurrentAction = ACTION_ERROR;
				}
			}
		}

		if(provider->mCurrentAction == ACTION_WAIT_REQUEST){
			ms_debug("[LDAP] ACTION_WAIT_REQUEST");
			size_t requestSize = 0;
			if( provider->mLd && provider->mConnected ){
				// check for pending searches
				provider->mLock.lock();
				for(auto it = provider->mRequests.begin() ; it != provider->mRequests.end() ; ){
					if(!(*it))
						it = provider->mRequests.erase(it);
					else if((*it)->mMsgId == 0){
						int ret;
						ms_message("[LDAP] Found pending search %p (for %s), launching...", it->get(), (*it)->mFilter.c_str());
						ret = provider->search(*it);
						if( ret != LDAP_SUCCESS ){
							it = provider->cancelSearch(it->get());
						}else
							++it;
					}else
						++it;
				}
				requestSize = provider->mRequests.size();
				provider->mLock.unlock();
			}
			if( requestSize > 0 ){// No need to check connectivity as it is checked before
				// never block
				int ret = (int)ldap_result(provider->mLd, LDAP_RES_ANY, LDAP_MSG_ONE, &pollTimeout, &results);
				switch( ret ){
				case -1: {
					int lastError = errno;
					ms_warning("LDAP : Error in ldap_result : returned -1 (req_count %zu): %s", requestSize, ldap_err2string(lastError));
					break;
				}
				case 0: break; // nothing to do
		
				case LDAP_RES_BIND:{
						ms_warning("[LDAP] iterate: unexpected LDAP_RES_BIND");
						if( results )
							ldap_msgfree(results);
					break;
				}
				case LDAP_RES_EXTENDED:
				case LDAP_RES_SEARCH_ENTRY:
				case LDAP_RES_SEARCH_REFERENCE:
				case LDAP_RES_INTERMEDIATE:
				case LDAP_RES_SEARCH_RESULT:
				{
					provider->handleSearchResult(results );
					break;
				}
				case LDAP_RES_MODIFY:
				case LDAP_RES_ADD:
				case LDAP_RES_DELETE:
				case LDAP_RES_MODDN:
				case LDAP_RES_COMPARE:
				default:
					ms_warning("[LDAP] Unhandled LDAP result %x", ret);
					break;
				}
				if( results )
					ldap_msgfree(results);
			}
		}
	}
	return true;
}


void LdapContactProvider::ldapServerResolved(void *data, belle_sip_resolver_results_t *results) {
	LdapContactProvider * provider = (LdapContactProvider*)(data);
	const struct addrinfo * addr = belle_sip_resolver_results_get_addrinfos(results);
	if (addr) {
		ms_debug("[LDAP] Server resolution successful.");
		const struct addrinfo *ai = NULL;
		int err;
		char ipstring [INET6_ADDRSTRLEN];
		ai = belle_sip_resolver_results_get_addrinfos(results);
		err = bctbx_getnameinfo((struct sockaddr*)ai->ai_addr, (socklen_t)ai->ai_addrlen, ipstring, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
		if (err != 0)
			ms_error("[LDAP] DNS resolver: getnameinfo error %s", gai_strerror(err));
		if(provider->mServerUri){
			belle_generic_uri_set_host(provider->mServerUri, ipstring);
			char * uriString = belle_generic_uri_to_string(provider->mServerUri);
			provider->mServerUrl = uriString;
			belle_sip_free(uriString);
		}else{
			provider->mServerUrl = provider->mConfig["server"];
		}
		provider->mLock.lock();
		provider->mCurrentAction = ACTION_INITIALIZE;
		provider->mLock.unlock();
	} else {
		ms_error("[LDAP] Server resolution failed, no address can be found.");
		provider->mLock.lock();
		provider->mCurrentAction = ACTION_ERROR;
		provider->mLock.unlock();
	}
}

void LdapContactProvider::handleSearchResult( LDAPMessage* message ) {
	mLock.lock();
	if(message){
		int msgtype = ldap_msgtype(message);
		LdapContactSearch* req = requestSearch(ldap_msgid(message));
		switch(msgtype){
		case LDAP_RES_SEARCH_ENTRY:
		case LDAP_RES_EXTENDED: {
			LDAPMessage *entry = ldap_first_entry(mLd, message);
			LinphoneCore*   lc = mCore->getCCore();
// Message can be a list. Loop on entries
			while( entry != NULL ){
				LdapContactFields ldapData;
				bool_t contact_complete = FALSE;
				BerElement*  ber = NULL;
				char* attr = ldap_first_attribute(mLd, entry, &ber);
// Each entry is about a contact. Loop on all attributes and fill contact. We do not stop when contact is completed to know if there are better attributes
				while( attr ) {
					struct berval** values = ldap_get_values_len(mLd, entry, attr);
					struct berval**     it = values;
					while( values && *it && (*it)->bv_val && (*it)->bv_len ) {
						contact_complete = (completeContact(&ldapData, attr, (*it)->bv_val) == 1);
						it++;
					}
					if( values ) ldap_value_free_len(values);
					ldap_memfree(attr);
					attr = ldap_next_attribute(mLd, entry, ber);
				}
				if( contact_complete ) {
					for(size_t i = 0 ; i < ldapData.mSip.first.size() ; ++i){
						LinphoneAddress* la = linphone_core_interpret_url(lc, ldapData.mSip.first[i].c_str());
						if( la ){
							linphone_address_set_display_name(la, ldapData.mName.first.c_str());
							req->mFoundEntries = bctbx_list_append(req->mFoundEntries, la);
							++req->mFoundCount;
						}
					}
				}
				if( ber ) ber_free(ber, 0);
				if(attr) ldap_memfree(attr);
				entry = ldap_next_entry(mLd, entry);
			}
		}
		break;
		case LDAP_RES_SEARCH_RESULT: {
			// this one is received when a request is finished
			cancelSearch(req);
		}
		break;
		default: ms_warning("[LDAP] Unhandled message type %x", msgtype); break;
		}
	}else{
		for(auto it = mRequests.begin() ; it != mRequests.end() ; ++it){
			(*it)->complete = TRUE;
			(*it)->callCallback();
		}
		mRequests.clear();
	}
	mLock.unlock();
}


LINPHONE_END_NAMESPACE
