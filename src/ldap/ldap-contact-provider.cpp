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
#include "ldap-contact-search.h"
#include "ldap-friend.h"
#include "contact_providers_priv.h"

#include <algorithm>

// TODO: From coreapi. Remove me later.
#include "private.h"

#define FILTER_MAX_SIZE      512

LINPHONE_BEGIN_NAMESPACE

//*******************************************	CREATION

LDAPContactProvider::LDAPContactProvider(const std::shared_ptr<Core> &core, const std::map<std::string,std::string> &config ){
	mServerUri = NULL;
	mSalContext = NULL;
	ortp_mutex_init(&mLock, NULL);
	mName = "LDAP";
	mCore = core;
	mConnected = FALSE;
	mAwaitingMessageId = 0;
	mLd = nullptr;
	// register our hook into iterate so that LDAP can do its magic asynchronously.
	linphone_core_add_iterate_hook(core->getCCore(), iterate, this);
	if( !validConfig(config) ) {
		ms_error( "LDAP : Invalid configuration for LDAP, aborting creation");
		mState = STATE_ERROR;
	} else {
		loadConfig(config);
		mState = STATE_OK;
	}
	mCurrentAction = ACTION_NONE;
}

LDAPContactProvider::~LDAPContactProvider(){
	linphone_core_remove_iterate_hook(mCore->getCCore(), iterate, this);
// Wait for bind thread to end
	ortp_mutex_lock(&mLock);
	if(mConnected==1)
		ldap_unbind_ext_s(mLd, NULL, NULL);
	if(mAwaitingMessageId > 0){
			ldap_abandon_ext(mLd, mAwaitingMessageId, NULL, NULL );
			mAwaitingMessageId = 0;
	}
	if( mServerUri){
		belle_sip_object_unref(mServerUri);
		mServerUri = NULL;
	}
	ortp_mutex_unlock(&mLock);
	
	ortp_mutex_destroy(&mLock);
}

std::vector<std::shared_ptr<LDAPContactProvider> > LDAPContactProvider::create(const std::shared_ptr<Core> &core){
	std::vector<std::shared_ptr<LDAPContactProvider> > providers;
	LpConfig * lConfig = linphone_core_get_config(core->getCCore());

	const bctbx_list_t * bcSections = linphone_config_get_sections_names_list(lConfig);
	for(auto itSections = bcSections; itSections; itSections=itSections->next) {
		std::string section = static_cast<char *>(itSections->data);
		std::string sectionName;
		size_t i = section.length()-1;
		while(i>0 && section[i] != '_')// Get the name strip number
			--i;
		if(i>0){
			sectionName = section.substr(0,i);
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
				providers.push_back(std::make_shared<LDAPContactProvider>(core, config));
		}
	}
	return providers;
}

void LDAPContactProvider::initializeLdap(){
	int proto_version = LDAP_VERSION3;
	int ret = ldap_initialize(&mLd, mServerUrl.c_str());
	if( ret != LDAP_SUCCESS ){
		ms_error( "LDAP : Problem initializing ldap on url '%s': %s", mConfig["server"].c_str(), ldap_err2string(ret));
		mState = STATE_ERROR;
	} else if( (ret = ldap_set_option(mLd, LDAP_OPT_PROTOCOL_VERSION, &proto_version)) != LDAP_SUCCESS ){
		ms_error( "LDAP : Problem setting protocol version %d: %s", proto_version, ldap_err2string(ret));
		mState = STATE_ERROR;
	} else {
		if(mConfig.count("use_tls")>0 && mConfig["use_tls"] == "1"){
			int reqcert = LDAP_OPT_X_TLS_ALLOW;
			ldap_set_option (NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &reqcert);
			ret = ldap_start_tls_s(mLd, NULL, NULL);
		}
		if( ret == LDAP_SUCCESS ) {
			ms_message("[LDAP] Initialization success");
			mConnected = 1;
		}else {
			int err;
			ldap_get_option(mLd, LDAP_OPT_RESULT_CODE, &err);
			ms_error("[LDAP] Cannot initialize address to %s : %x, err %x (%s)",mConfig["server"].c_str(), ret, err, ldap_err2string(err));
			mState = STATE_ERROR;
		}
	}
}

//*******************************************	CONFIGURATION

class ConfigKeys{
public:
	ConfigKeys(const std::string& pValue, const bool_t& pRequired=FALSE) : value(pValue), required(pRequired){}
	
	std::string value;
	bool_t required;
	static std::vector<std::string> split(const std::string& pValue){
		std::vector<std::string> tokens;
		std::istringstream iss(pValue);
		std::string s;    
		while (std::getline(iss, s, ',')) {
			tokens.push_back(s);
		}
		return tokens;
	}
};

static const std::map<std::string, ConfigKeys> gConfigKeys={
	{"timeout", ConfigKeys("10")},
	{"max_results", ConfigKeys("5")},
	{"auth_method", ConfigKeys("SIMPLE")},
	//{"username", ConfigKeys("")},
	{"password", ConfigKeys("")},
	{"bind_dn", ConfigKeys("", TRUE)},
	{"base_object", ConfigKeys("dc=example,dc=com", TRUE)},
	{"server", ConfigKeys("ldap:///", TRUE)},
	{"filter", ConfigKeys("(sn=*%s*)")},
	{"name_attribute", ConfigKeys("sn")},
	{"sip_attribute", ConfigKeys("mobile,telephoneNumber,homePhone")},
	{"sip_scheme", ConfigKeys("sip")},
	{"sip_domain", ConfigKeys("sip.linphone.org")},
	{"enable", ConfigKeys("1")},
	{"use_sal", ConfigKeys("0")},
	{"use_tls", ConfigKeys("1")}
};

bool_t LDAPContactProvider::validConfig(const std::map<std::string, std::string>& config)const {
	bool_t valid = TRUE;
	for(auto it = gConfigKeys.begin() ; it != gConfigKeys.end() ; ++it)
		if( it->second.required && config.count(it->first)<=0){
			ms_error("LDAP : Missing LDAP config value for '%s'", it->first.c_str());
			valid = FALSE;
		}
	return valid;
}


void LDAPContactProvider::loadConfig(const std::map<std::string, std::string>& config)
{
	mConfig.clear();
	for(auto it = gConfigKeys.begin() ; it != gConfigKeys.end() ; ++it)
		mConfig[it->first] = (config.count(it->first)>0 ? config.at(it->first) : it->second.value);
	mNameAttributes = ConfigKeys::split(mConfig["name_attribute"]);
	mSipAttributes = ConfigKeys::split(mConfig["sip_attribute"]);
	mAttributes = mNameAttributes;
	for(auto it = mSipAttributes.begin() ; it != mSipAttributes.end() ; ++it)
		if( std::find(mAttributes.begin(), mAttributes.end(), *it) == mAttributes.end())
			mAttributes.push_back(*it);
}

int LDAPContactProvider::getTimeout() const{
	return atoi(mConfig.at("timeout").c_str());
}

//*******************************************	SEARCH


void LDAPContactProvider::search(const std::string& predicate, ContactSearchCallback cb, void* cbData){

	std::shared_ptr<LDAPContactSearch> request = std::make_shared<LDAPContactSearch>(this, predicate, cb, cbData );
	/*
	if( mConnected ){
		int ret = search(request);
		ms_message ( "LDAP : Created search %zu for '%s', msgid %d, @%p", mRequests.size(), predicate.c_str(), request->mMsgId, request.get() );
		if( ret != LDAP_SUCCESS ){
			request = NULL;
		}
	} else {
		ms_message("LDAP : Delayed search, wait for connection");
	}
*/
	ortp_mutex_lock(&mLock);
	if( request != NULL ) {
		mRequests.push_back(request);
	}
	ortp_mutex_unlock(&mLock);
}

int LDAPContactProvider::search(std::shared_ptr<LDAPContactSearch> request){
	int ret = -1;
#ifdef _WIN32
	struct l_timeval timeout = { atoi(mConfig["timeout"].c_str()), 0 };
#else
	struct timeval timeout = { atoi(mConfig["timeout"].c_str()), 0 };
#endif

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
			ms_error("LDAP : Error ldap_search_ext returned %d (%s)", ret, ldap_err2string(ret));
		} else {
			ms_message("LDAP : LinphoneLDAPContactSearch created @%p : msgid %d", request.get(), request->mMsgId);
		}

	} else {
		ms_warning( "LDAP : Search already performed for %s, msgid %d", request->mFilter.c_str(), request->mMsgId);
	}
	return ret;
}

std::list<std::shared_ptr<LDAPContactSearch> >::iterator LDAPContactProvider::cancelSearch(LDAPContactSearch * req) {
	std::list<std::shared_ptr<LDAPContactSearch>>::iterator listEntry = std::find_if(mRequests.begin(), mRequests.end(),[req] (const std::shared_ptr<LDAPContactSearch>& a) 
		{ return (a->mMsgId == req->mMsgId) && (a.get() == req); }
	);
	
	if( listEntry != mRequests.end()) {
		ms_message("LDAP : Delete search %p", req);
		if( !(*listEntry)->complete){
			(*listEntry)->complete = TRUE;
			(*listEntry)->callCallback();
		}
		listEntry = mRequests.erase(listEntry);
	} else {
		ms_warning("LDAP : Couldn't find ldap request %p (id %d) in monitoring.", req, req->mMsgId);
	}
	return listEntry;
}

LDAPContactSearch* LDAPContactProvider::requestSearch( int msgid ) {

	auto listEntry = std::find_if(mRequests.begin(), mRequests.end(), [msgid](const std::shared_ptr<LDAPContactSearch>& a ){
		return a->mMsgId == msgid;
	});
	if( listEntry != mRequests.end() ) 
		return listEntry->get();
	else 
		return NULL;
}

int LDAPContactProvider::completeContact( LDAPFriend* lf, const char* attr_name, const char* attr_value) {
	for(size_t attributeIndex = 0 ; attributeIndex < mNameAttributes.size() && (lf->mName == "" || lf->mNameIndex > (int)attributeIndex) ; ++attributeIndex){
		if( attr_name == mNameAttributes[attributeIndex]){
			lf->mName = attr_value;
			lf->mNameIndex = (int)attributeIndex;
		}
	}
	for(size_t attributeIndex = 0 ; attributeIndex < mSipAttributes.size() && (lf->mSip == "" || lf->mSipIndex > (int)attributeIndex) ; ++attributeIndex){
		if( attr_name == mSipAttributes[attributeIndex]){
			std::string sip;
			if(mConfig.count("sip_scheme")>0 && mConfig.at("sip_scheme") != "")
				sip = mConfig.at("sip_scheme")+":";
			sip += attr_value;
			if(mConfig.count("sip_domain")>0 && mConfig.at("sip_domain") != "")
				sip += "@" + mConfig.at("sip_domain");
// Test if this sip is ok	
			LinphoneAddress* la = linphone_core_interpret_url(mCore->getCCore(), sip.c_str());
			if( !la){
			}else{
				lf->mSip = sip;
				lf->mSipIndex = (int)attributeIndex;
				linphone_address_unref(la);
			}
		}
	}
	// return 1 if the structure has enough data to create a linphone friend
	if( lf->mName !="" && lf->mSip !="" )
		return 1;
	else
		return 0;
}

//*******************************************	ASYNC PROCESSING
// ACTION_NONE => ACTION_INIT => (ACTION_WAIT_DNS) => ACTION_INITIALIZE => ACTION_BIND => ACTION_WAIT_BIND => ACTION_WAIT_REQUEST
bool_t LDAPContactProvider::iterate(void *data) {
#ifdef _WIN32
		LDAP_TIMEVAL pollTimeout = {0,0};
#else
		struct timeval pollTimeout = {0,0};
#endif
	LDAPMessage* results = NULL;
	LDAPContactProvider* provider = (LDAPContactProvider*)data;
	if(provider->mState == STATE_ERROR){
		provider->handleSearchResult(NULL );
	}else if(provider->mCurrentAction == ACTION_NONE){
		ms_message("[LDAP] ACTION_NONE");
		ortp_mutex_lock(&provider->mLock);
		if( provider->mRequests.size() > 0){
			if( provider->mState != STATE_ERROR){
				if( provider->mConnected != 1)
					provider->mCurrentAction = ACTION_INIT;
				else
					provider->mCurrentAction = ACTION_BIND;
			}
		}
		ortp_mutex_unlock(&provider->mLock);
	}else if(provider->mCurrentAction == ACTION_BIND){ // Careful : Binds are not thread-safe
		ms_message("[LDAP] ACTION_BIND");
		std::string auth_mechanism = provider->mConfig.at("auth_method");
		int ret;
		if( (auth_mechanism == "ANONYMOUS") || (auth_mechanism == "SIMPLE") ) {
			struct berval passwd = { provider->mConfig.at("password").length(), ms_strdup(provider->mConfig.at("password").c_str())};
			//struct berval *serverResponse = NULL;
	#ifdef _WIN32
			ret = ldap_bind_s(provider->mLd,  provider->mConfig.at("bind_dn"), provider->mConfig.at("password"), LDAP_AUTH_SIMPLE);
	#else//"cn=Marie Laroueverte,ou=people,dc=bc,dc=com",
			//ret = ldap_sasl_bind_s(provider->ld, provider->bind_dn, NULL, &passwd, NULL, NULL, &serverResponse);
			//ret = ldap_sasl_bind_s(provider->mLd, provider->mConfig.at("bind_dn").c_str(), NULL, &passwd, NULL, NULL, &serverResponse);
			ret = ldap_sasl_bind(provider->mLd, provider->mConfig.at("bind_dn").c_str(), NULL, &passwd, NULL, NULL, &provider->mAwaitingMessageId);
	#endif
			ms_free(passwd.bv_val);
			if( ret == LDAP_SUCCESS ) {
				ms_message("[LDAP] Waiting for bind");
				provider->mCurrentAction = ACTION_WAIT_BIND;
			} else {
				int err;
				ldap_get_option(provider->mLd, LDAP_OPT_RESULT_CODE, &err);
				ms_error("[LDAP] ldap_sasl_bind error returned %x, err %x (%s), auth_method: %s", ret, err, ldap_err2string(err), auth_mechanism.c_str() );
				provider->mCurrentAction = ACTION_NONE;
				provider->mState = STATE_ERROR;
				provider->mAwaitingMessageId = 0;
			}
		} else {
			ms_error("LDAP : special authentifications is not supported. You must use SIMPLE or ANONYMOUS");
			provider->mCurrentAction = ACTION_NONE;
			provider->mState = STATE_ERROR;
			provider->mAwaitingMessageId = 0;
		}
	}else if(provider->mCurrentAction == ACTION_INITIALIZE){
		ms_message("[LDAP] ACTION_INITIALIZE");
		if (provider->mSalContext) {
			belle_sip_object_unref(provider->mSalContext);
			provider->mSalContext = NULL;
		}
		provider->initializeLdap();
		if( provider->mServerUri){
			belle_sip_object_unref(provider->mServerUri);
			provider->mServerUri = NULL;
		}
		provider->mCurrentAction = ACTION_NONE;
	}else if(provider->mCurrentAction == ACTION_WAIT_DNS){
		ms_message("[LDAP] ACTION_WAIT_DNS");
		// Do nothing, wait for SAL to do its work
	}else if(provider->mCurrentAction == ACTION_INIT){
		ms_message("[LDAP] ACTION_INIT");
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
				
				provider->mSalContext = provider->mCore->getCCore()->sal->resolveA(domain.c_str(), port, AF_INET, stun_server_resolved, provider);
				if (provider->mSalContext)
					belle_sip_object_ref(provider->mSalContext);
				else
					ms_error("LDAP : cannot request DNS.");
			}
			provider->mCurrentAction = ACTION_WAIT_DNS;
		}
	}else if(provider->mCurrentAction == ACTION_WAIT_BIND){
		ms_message("[LDAP] ACTION_WAIT_BIND");
		int ret = (int)ldap_result(provider->mLd, provider->mAwaitingMessageId, LDAP_MSG_ONE, &pollTimeout, &results);
		if(ret == LDAP_RES_BIND){
			ret = ldap_parse_sasl_bind_result( provider->mLd, results,NULL, 1); // Auto ldap_msgfree(results)
			if( ret == LDAP_SUCCESS){
				ms_message("[LDAP] Bind successful");
				provider->mConnected = 1;
				provider->mAwaitingMessageId = 0;
				provider->mCurrentAction = ACTION_WAIT_REQUEST;
			}else{
				ms_error("[LDAP] Cannot bind to server : %x (%s)", ret, ldap_err2string(ret));
				provider->mCurrentAction = ACTION_NONE;
				provider->mState = STATE_ERROR;
			}
		}
	}else if(provider->mCurrentAction == ACTION_WAIT_REQUEST){
		ms_message("[LDAP] ACTION_WAIT_REQUEST");
		ortp_mutex_lock(&provider->mLock);
		size_t requestSize = provider->mRequests.size();
		ortp_mutex_unlock(&provider->mLock);
		if( provider->mLd && provider->mConnected && requestSize > 0 ){
			// never block
			int ret = (int)ldap_result(provider->mLd, LDAP_RES_ANY, LDAP_MSG_ONE, &pollTimeout, &results);
			switch( ret ){
			case -1:
			{
	#ifdef _WIN32
				int lastError = LdapGetLastError();
	#else
				int lastError = errno;
	#endif
				ms_warning("LDAP : Error in ldap_result : returned -1 (req_count %zu): %s", requestSize, ldap_err2string(lastError));
				break;
			}
			case 0: break; // nothing to do
	
			case LDAP_RES_BIND:{
					ms_error("LDAP : iterate: unexpected LDAP_RES_BIND");
				if( results )
					ldap_msgfree(results);
				break;
			}
			case LDAP_RES_EXTENDED:
			case LDAP_RES_SEARCH_ENTRY:
	#ifndef _WIN32
			case LDAP_RES_SEARCH_REFERENCE:
			case LDAP_RES_INTERMEDIATE:
	#endif
			case LDAP_RES_SEARCH_RESULT:
			{
				provider->handleSearchResult(results );
				break;
			}
			case LDAP_RES_MODIFY:
			case LDAP_RES_ADD:
			case LDAP_RES_DELETE:
	#ifdef WIN32
			case LDAP_RES_MODRDN:
	#else
			case LDAP_RES_MODDN:
	#endif
			case LDAP_RES_COMPARE:
			default:
				ms_message("LDAP : Unhandled LDAP result %x", ret);
				break;
			}
	
			if( results )
				ldap_msgfree(results);
		}
	
		if( provider->mLd && provider->mConnected ){
			// check for pending searches
			ortp_mutex_lock(&provider->mLock);
			for(auto it = provider->mRequests.begin() ; it != provider->mRequests.end() ; ){
				if(!(*it))
					it = provider->mRequests.erase(it);
				else if((*it)->mMsgId == 0){
					int ret;
					ms_message("LDAP : Found pending search %p (for %s), launching...", it->get(), (*it)->mFilter.c_str());
					ret = provider->search(*it);
					if( ret != LDAP_SUCCESS ){
						it = provider->cancelSearch(it->get());
					}else
						++it;
				}else
					++it;
			}
			ortp_mutex_unlock(&provider->mLock);
		}
	}
	return TRUE;
}



void LDAPContactProvider::stun_server_resolved(void *data, belle_sip_resolver_results_t *results) {
	LDAPContactProvider * provider = (LDAPContactProvider*)(data);
	const struct addrinfo * addr = belle_sip_resolver_results_get_addrinfos(results);
	if (addr) {
		ms_message("LDAP : Server resolution successful.");
		const struct addrinfo *ai = NULL;
		int err;
		char ipstring [INET6_ADDRSTRLEN];
		ai = belle_sip_resolver_results_get_addrinfos(results);
		err = bctbx_getnameinfo((struct sockaddr*)ai->ai_addr, ai->ai_addrlen, ipstring, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
		if (err != 0)
			ms_error("LDAP resolver: getnameinfo error %s", gai_strerror(err));
		if(provider->mServerUri){
			belle_generic_uri_set_host(provider->mServerUri, ipstring);
			char * uriString = belle_generic_uri_to_string(provider->mServerUri);
			provider->mServerUrl = uriString;
			belle_sip_free(uriString);
		}else{
			provider->mServerUrl = provider->mConfig["server"];
		}
		ortp_mutex_lock(&provider->mLock);
		provider->mCurrentAction = ACTION_INITIALIZE;
		ortp_mutex_unlock(&provider->mLock);
	} else {
		ms_error("LDAP : Server resolution failed.");
		ortp_mutex_lock(&provider->mLock);
		provider->mState = STATE_ERROR;
		provider->mCurrentAction = ACTION_NONE;
		ortp_mutex_unlock(&provider->mLock);
	}
}

void LDAPContactProvider::handleSearchResult( LDAPMessage* message ) {
	ortp_mutex_lock(&mLock);
	if(message){
		int msgtype = ldap_msgtype(message);
		LDAPContactSearch* req = requestSearch(ldap_msgid(message));
		switch(msgtype){
		case LDAP_RES_SEARCH_ENTRY:
		case LDAP_RES_EXTENDED:
		{
			LDAPMessage *entry = ldap_first_entry(mLd, message);
			LinphoneCore*   lc = mCore->getCCore();
			while( entry != NULL ){
				LDAPFriend ldapData;
				bool_t contact_complete = FALSE;
				BerElement*  ber = NULL;
				char* attr = ldap_first_attribute(mLd, entry, &ber);
	
				while( attr ){
					struct berval** values = ldap_get_values_len(mLd, entry, attr);
					struct berval**     it = values;
					while( values && *it && (*it)->bv_val && (*it)->bv_len )
					{
						contact_complete = (completeContact(&ldapData, attr, (*it)->bv_val) == 1);
						it++;
					}
					if( values ) ldap_value_free_len(values);
					ldap_memfree(attr);
					attr = ldap_next_attribute(mLd, entry, ber);
				}
				if( contact_complete ) {
					LinphoneAddress* la = linphone_core_interpret_url(lc, ldapData.mSip.c_str());
					if( la ){
						linphone_address_set_display_name(la, ldapData.mName.c_str());
						req->mFoundEntries = bctbx_list_append(req->mFoundEntries, la);
						++req->mFoundCount;
					}
				}
	
				if( ber ) ber_free(ber, 0);
				if(attr) ldap_memfree(attr);
	
				entry = ldap_next_entry(mLd, entry);
			}
		}
		break;
		case LDAP_RES_SEARCH_RESULT:
		{
			// this one is received when a request is finished
			cancelSearch(req);
		}
		break;
		default: ms_message("[LDAP] Unhandled message type %x", msgtype); break;
		}
	}else{
		for(auto it = mRequests.begin() ; it != mRequests.end() ; ++it){
			(*it)->complete = TRUE;
			(*it)->callCallback();
		}
		mRequests.clear();
	}
	ortp_mutex_unlock(&mLock);
}


LINPHONE_END_NAMESPACE
