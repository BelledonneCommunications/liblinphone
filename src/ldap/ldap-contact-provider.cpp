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

#include "ldap-contact-provider.h"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>

#include "bctoolbox/crypto.hh"
#include "contact_providers_priv.h"
#include "friend/friend.h"
#include "ldap-config-keys.h"
#include "ldap-contact-fields.h"
#include "ldap-contact-search.h"
#include "ldap-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-types.h"
#include "search/search-result.h"
#include "vcard/vcard.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

#define FILTER_MAX_SIZE 512

LINPHONE_BEGIN_NAMESPACE

#ifdef _WIN32
// someone does include the evil windef.h so we must undef the min and max macros to be able to use std::min and
// std::max
#undef min
#undef max
#endif
// Custom redefinitions

//*******************************************	CREATION

LdapContactProvider::LdapContactProvider(const std::shared_ptr<Core> &core,
                                         std::shared_ptr<LdapParams> ldap,
                                         int maxResults) {
	mAwaitingMessageId = 0;
	mConnected = false;
	mCore = core;
	mMaxResults = maxResults;
	mLd = nullptr;
	mLdapServer = ldap;
	mSalContext = NULL;
	const std::map<std::string, std::vector<std::string>> &config = ldap->getConfig();
	// register our hook into iterate so that LDAP can do its magic asynchronously.
	mIteration = mCore->createTimer(std::bind(&LdapContactProvider::iterate, this), 50, "LdapContactProvider");
	if (!LdapConfigKeys::validConfig(config)) {
		ms_error("[LDAP] Invalid configuration for LDAP, aborting creation");
		mCurrentAction = ACTION_ERROR;
	} else {
		mConfig = LdapConfigKeys::loadConfig(config);
		// tolower on configuration to avoid changing case on each result.
		if (mConfig.count("name_attribute") > 0)
			mConfig["name_attribute"] = Utils::stringToLower(mConfig["name_attribute"]);
		if (mConfig.count("sip_attribute") > 0)
			mConfig["sip_attribute"] = Utils::stringToLower(mConfig["sip_attribute"]);
		mCurrentAction = ACTION_NONE;
	}
}

LdapContactProvider::~LdapContactProvider() {
	if (mIteration) {
		mCore->destroyTimer(mIteration);
		mIteration = nullptr;
	}
	cleanLdap();
}

void LdapContactProvider::cleanLdap() {
	if (mSalContext) {
		belle_sip_resolver_context_cancel(mSalContext);
		belle_sip_object_unref(mSalContext);
		mSalContext = NULL;
	}
	if (mAwaitingMessageId > 0) { // There is currently a request that has not been processed. Abandon it.
		ldap_abandon_ext(mLd, mAwaitingMessageId, NULL, NULL);
		mAwaitingMessageId = 0;
	}
	if (mLd) // We have been bind. Clean the exit
		ldap_unbind_ext_s(mLd, NULL, NULL);
	mConnected = false;
	mLd = nullptr;
}

void LdapContactProvider::fallbackToNextServerUrl() {
	lDebug() << "[LDAP] fallbackToNextServerUrl (" << mServerUrlIndex << "," << mServerUrl.size() << ")";
	if (++mServerUrlIndex >=
	    mServerUrl.size()) { // ServerUrl comes from SRV or config. Check if there are more for next url in config.
		mServerUrl.clear();
		mServerUrlIndex = 0;
		if (++mConfigServerIndex >= mConfig["server"].size()) // No more urls
			mCurrentAction = ACTION_ERROR;
		else {
			lDebug() << "[LDAP] fallback to next config : " << mConfig["server"][mConfigServerIndex];
			mCurrentAction = ACTION_INIT; // Restart with sal for new config url
		}
	} else {
		lDebug() << "[LDAP] fallback to next url : " << mServerUrl[mServerUrlIndex];
		mCurrentAction = ACTION_INITIALIZE; // No need to check with sal.
	}
	cleanLdap();
}

void LdapContactProvider::ldapTlsConnection() {
	std::string srText;
	void *value = LDAP_OPT_ON;
	int ldapReturnStatus;
	int resultStatus;
	int timeout = configValueToInt("timeout_tls_ms");
	// 1) Start TLS
	if (mTlsConnectionId < 0) {                              // Start TLS
		ldap_set_option(mLd, LDAP_OPT_CONNECT_ASYNC, value); // If not Async, ldap_start_tls can block on connect.
		ldapReturnStatus = ldap_start_tls(mLd, NULL, NULL, &mTlsConnectionId); // Try to open a socket.
		if (ldapReturnStatus != LDAP_SUCCESS) {
			lError() << "[LDAP] Cannot start TLS connection (" << ldap_err2string(ldapReturnStatus) << ") for "
			         << mServerUrl[mServerUrlIndex];
			fallbackToNextServerUrl();
			mTlsConnectionId = -1;
		} else // mTlsConnectionId is not -1 only on success.
			lInfo() << "[LDAP] ldap_start_tls success";
	} // Not 'else' : we try to get a result without having to wait an iteration
	  // 2) Wait for connection
	if (mTlsConnectionId >= 0) {
		LDAPMessage *resultMessage = NULL;
		struct timeval tv = {0, 0}; // Do not block
		ldapReturnStatus = ldap_result(mLd, mTlsConnectionId, LDAP_MSG_ALL, &tv, &resultMessage);
		switch (ldapReturnStatus) {
			case -1:
				lError() << "[LDAP] Cannot start TLS connection : Remote server is down at "
				         << mServerUrl[mServerUrlIndex];
				fallbackToNextServerUrl();
				break;
			case 0: { // Retry on the next iteration.
				if (1000 * difftime(time(NULL), mTlsConnectionTimeout) > timeout) {
					lError() << "[LDAP] Tls was starting with success but the remote server doesn't respond to "
					            "ldap_result. TLS timeout has been reached ["
					         << timeout << "] at " << mServerUrl[mServerUrlIndex];
					fallbackToNextServerUrl();
				}
				return;
			}
			case LDAP_RES_EXTENDED:
				ldapReturnStatus = ldap_parse_extended_result(mLd, resultMessage, NULL, NULL, 0);
				lDebug() << "[LDAP] ldap_parse_extended_result: " << ldapReturnStatus;
				if (ldapReturnStatus == LDAP_SUCCESS) {
					ldapReturnStatus = ldap_parse_result(mLd, resultMessage, &resultStatus, NULL, NULL, NULL, NULL, 1);
					lDebug() << "[LDAP] ldap_parse_result: " << ldapReturnStatus;
					resultMessage = NULL; // Freed by ldap_parse_result

					if (ldapReturnStatus == LDAP_SUCCESS) {
						ldapReturnStatus = resultStatus;
					}
					if (ldapReturnStatus == LDAP_SUCCESS) {
						ldapReturnStatus = ldap_install_tls(mLd);
						lDebug() << "[LDAP] ldap_install_tls: " << ldapReturnStatus;
						if (ldapReturnStatus == LDAP_SUCCESS || ldapReturnStatus == LDAP_LOCAL_ERROR) {
							mCurrentAction = ACTION_BIND;
						} else {
							ldap_get_option(mLd, LDAP_OPT_RESULT_CODE, &resultStatus);
							lError() << "[LDAP] Cannot install the TLS handler (" << ldap_err2string(ldapReturnStatus)
							         << "), resultStatus " << resultStatus << " (" << ldap_err2string(resultStatus)
							         << ")";
							fallbackToNextServerUrl();
						}

					} else if (ldapReturnStatus == LDAP_REFERRAL) {
						lError() << "[LDAP] Unwilling to chase referral returned by Start TLS exop";
						fallbackToNextServerUrl();
					}
				}
				break;
			default:
				lWarning() << "[LDAP] Unknown response to StartTLS request : ExtendedResponse is expected";
				break;
		}
		if (resultMessage != NULL) {
			ldap_msgfree(resultMessage);
		}
	}
}

static void onLdapLog(const char *msg) {
	if (msg) {
		std::string cppMsg(msg);
		// Remove trailing \n from openldap logs.
		if (cppMsg.size() != 0 && cppMsg[cppMsg.size() - 1] == '\n') cppMsg.resize(cppMsg.size() - 1);
		lInfo() << "libldap: " << cppMsg;
	}
}

int LdapContactProvider::randomProvider(void *buffer, int bytes) {
	bctoolbox::RNG::cRandomize((uint8_t *)buffer, (size_t)bytes);
	return 0;
}

void LdapContactProvider::initializeLdap() {
	int proto_version = LDAP_VERSION3;
	int ret = ldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &proto_version);
	int debLevel = 0;
	struct timeval timeout = {configValueToInt("timeout"), 0};
	mCurrentAction = ACTION_NONE;

	ber_set_option(NULL, LBER_OPT_LOG_PRINT_FN, (const void *)onLdapLog);

	if (ret != LDAP_SUCCESS)
		lError() << "[LDAP] Problem initializing default Protocol version to 3 : " << ret << ", ("
		         << ldap_err2string(ret) << ")";
	ret = ldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeout);
	if (ret != LDAP_SUCCESS)
		lError() << "[LDAP] Problem initializing default timeout to " << timeout.tv_sec << " : " << ret << " ("
		         << ldap_err2string(ret) << ")";
	// Setting global options for the next initialization. These options cannot be done with the LDAP instance directly.
	// if (mConfig.count("debug") > 0 &&
	//    LinphoneLdapDebugLevelVerbose == static_cast<LinphoneLdapDebugLevel>(atoi(mConfig["debug"][0].c_str())))
	// Note that openldap library does not report any error log, only debug logs.
	if (bctbx_log_level_enabled(BCTBX_LOG_DOMAIN, BCTBX_LOG_MESSAGE)) {
		debLevel = 0xffff; // There is debug level definition in openLDAP.
	} else {
		debLevel = 0;
	}
	ret = ldap_set_option(NULL, LDAP_OPT_DEBUG_LEVEL, &debLevel);
	if (ret != LDAP_SUCCESS)
		lError() << "[LDAP] Problem initializing debug options LDAP_OPT_DEBUG_LEVEL : " << ret << " ("
		         << ldap_err2string(ret) << ")";
	if (mConfig.count("use_tls") > 0 && mConfig["use_tls"][0] == "1") {
		std::string caFile = linphone_core_get_root_ca(mCore->getCCore());
		bool enableVerification = true;
		if (mConfig.count("verify_server_certificates") > 0) {
			auto mode = mConfig["verify_server_certificates"][0];
			if (mode == "-1") enableVerification = linphone_core_is_verify_server_certificates(mCore->getCCore());
			else if (mode == "0") enableVerification = false;
		}
		int reqcert = (enableVerification ? LDAP_OPT_X_TLS_DEMAND : LDAP_OPT_X_TLS_ALLOW);
		int reqsan = LDAP_OPT_X_TLS_ALLOW;
		ret = ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &reqcert);
		if (ret != LDAP_SUCCESS)
			lError() << "[LDAP] Problem initializing TLS on setting require certification '"
			         << mServerUrl[mServerUrlIndex] << "': " << ret << " (" << ldap_err2string(ret) << ")";
		ret = ldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTFILE, caFile.c_str());
		if (ret != LDAP_SUCCESS)
			lError() << "[LDAP] Problem initializing TLS on setting CA Certification file '"
			         << mServerUrl[mServerUrlIndex] << "': " << ret << " (" << ldap_err2string(ret) << ")";
		ret = ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_SAN, &reqsan);
		if (ret != LDAP_SUCCESS)
			lError() << "[LDAP] Problem initializing TLS on setting require SAN '" << mServerUrl[mServerUrlIndex]
			         << "': " << ret << " (" << ldap_err2string(ret) << ")";

		ret = ldap_set_option(NULL, LDAP_OPT_X_TLS_RANDOM_FUNC, (void *)randomProvider);
		if (ret != LDAP_SUCCESS) {
			lError() << "[LDAP] Problem initializing TLS random generator function.";
		}
	}
	ret = ldap_initialize(&mLd, mServerUrl[mServerUrlIndex].c_str()); // Trying to connect even on error on options

	if (ret != LDAP_SUCCESS) {
		lError() << "[LDAP] Problem initializing ldap on url '" << mServerUrl[mServerUrlIndex] << "': " << ret << " ("
		         << ldap_err2string(ret) << ")";
		fallbackToNextServerUrl();
	} else if ((ret = ldap_set_option(mLd, LDAP_OPT_PROTOCOL_VERSION, &proto_version)) != LDAP_SUCCESS) {
		lError() << "[LDAP] Problem setting protocol version " << proto_version << ": " << ret << " ("
		         << ldap_err2string(ret) << ")";
		fallbackToNextServerUrl();
	} else if (ret != LDAP_SUCCESS) {
		int err;
		ldap_get_option(mLd, LDAP_OPT_RESULT_CODE, &err);
		lError() << "[LDAP] Cannot initialize address to " << mServerUrl[mServerUrlIndex] << " : " << ret << " ("
		         << ldap_err2string(ret) << "), err " << err << " (" << ldap_err2string(err) << ")";
		fallbackToNextServerUrl();
	} else if (mConfig.count("use_tls") > 0 && mConfig["use_tls"][0] == "1") {
		if (mConfig.count("use_sal") > 0 &&
		    mConfig["use_sal"][0] ==
		        "1") { // Using Sal give an IP for a domain. So check the domain rather than the IP.
			belle_generic_uri_t *serverUri = belle_generic_uri_parse(
			    mConfig["server"][mConfigServerIndex].c_str()); // mServer are results of sal. Use the root urls.
			const char *cHost = belle_generic_uri_get_host(serverUri);
			std::string hostname = cHost ? cHost : "";
			ldap_set_option(mLd, LDAP_OPT_X_TLS_PEER_CN, &hostname[0]);
			if (serverUri) belle_sip_object_unref(serverUri);
		}
		mTlsConnectionId = -1;
		mCurrentAction = ACTION_WAIT_TLS_CONNECT;
		mTlsConnectionTimeout = time(NULL);
	} else {
		lDebug() << "[LDAP] Initialization success";
		mConnected = true;
	}
}

int LdapContactProvider::configValueToInt(const std::string &key) const {
	return atoi((mConfig.count(key) > 0 ? mConfig.at(key)[0]
	                                    : LdapConfigKeys::split(key, LdapConfigKeys::getConfigKeys(key).mValue)[0])
	                .c_str());
}

std::string LdapContactProvider::configValueToStr(const std::string &key) const {
	return mConfig.count(key) > 0 ? mConfig.at(key)[0]
	                              : LdapConfigKeys::split(key, LdapConfigKeys::getConfigKeys(key).mValue)[0];
}

int LdapContactProvider::getCurrentAction() const {
	return mCurrentAction;
}

std::shared_ptr<LdapParams> LdapContactProvider::getLdapServer() {
	return mLdapServer;
}

void LdapContactProvider::computeLastRequestTime(const std::list<SearchRequest> &requestHistory) {
	// Find the first LDAP search.
	std::list<SearchRequest>::const_iterator itRequest = requestHistory.begin();
	mLastRequestTime = 0; // No search in history: do not delay

	do {
		itRequest = std::find_if(itRequest, requestHistory.end(), [](const SearchRequest &r) {
			return (r.getSourceFlags() & LinphoneMagicSearchSourceLdapServers) == LinphoneMagicSearchSourceLdapServers;
		});
		if (itRequest != requestHistory.end()) {
			if (mLastRequestTime == 0)
				mLastRequestTime =
				    itRequest->getStartTime(); // There was one search. Store the time and loop on next LDAP searches.
			else {
				uint64_t t = itRequest->getStartTime();
				if (t - mLastRequestTime >
				    (uint64_t)configValueToInt(
				        "delay")) // This last search should be start as it's superior from timeout. Take it account.
					mLastRequestTime = t;
			}
			// Continue with the next LDAP search
			++itRequest;
		}
	} while (itRequest != requestHistory.end());
}
//*******************************************	SEARCH

// Create a search object and store the request to be used when the provider is ready
bool LdapContactProvider::search(const std::string &predicate,
                                 MagicSearchCallback cb,
                                 void *cbData,
                                 const std::list<SearchRequest> &requestHistory) {
	if (configValueToInt("min_chars") <= (int)predicate.length()) {
		std::shared_ptr<LdapContactSearch> request = std::make_shared<LdapContactSearch>(this, predicate, cb, cbData);
		if (request != NULL) {
			mRequests.push_back(request);
		}
		computeLastRequestTime(requestHistory);
		return true;
	} else return false;
}

// Start the search
int LdapContactProvider::search(std::shared_ptr<LdapContactSearch> request) {
	int ret = -1;
	struct timeval timeout = {configValueToInt("timeout"), 0};
	int maxResults = 0;
	if (mMaxResults == -1) {
		// If magic search is configured to return unlimited results, only use LDAP configuration max results value
		maxResults = configValueToInt("max_results");
	} else {
		// Otherwise take min value between both configs
		maxResults = std::min(configValueToInt("max_results"), mMaxResults);
	}
	if (maxResults > 0) ++maxResults; // +1 to know if there is more than limit

	if (request->mMsgId == 0) {
		ret = ldap_search_ext(mLd,
		                      configValueToStr("base_object").c_str(), // base from which to start
		                      LDAP_SCOPE_SUBTREE,
		                      request->mFilter.c_str(), // search predicate
		                      NULL,                     //(char**)attributes, // which attributes to get
		                      0,                        // 0 = get attrs AND value, 1 = get attrs only
		                      NULL, NULL,
		                      &timeout, // server timeout for the search
		                      maxResults, &request->mMsgId);
		if (ret != LDAP_SUCCESS) {
			lError() << "[LDAP] Error ldap_search_ext returned " << ret << " (" << ldap_err2string(ret) << ")";
		} else {
			lDebug() << "[LDAP] LinphoneLdapContactSearch created @" << request.get() << " : msgid " << request->mMsgId;
		}

	} else {
		lWarning() << "[LDAP] Search already performed for " << request->mFilter << ", msgid " << request->mMsgId;
	}
	return ret;
}

// Call callback, mark it as completed and remove the request
std::list<std::shared_ptr<LdapContactSearch>>::iterator LdapContactProvider::cancelSearch(LdapContactSearch *request) {
	std::list<std::shared_ptr<LdapContactSearch>>::iterator listEntry =
	    std::find_if(mRequests.begin(), mRequests.end(), [request](const std::shared_ptr<LdapContactSearch> &a) {
		    return (a->mMsgId == request->mMsgId) && (a.get() == request);
	    });

	if (listEntry != mRequests.end()) {
		lDebug() << "[LDAP] Delete search " << request;
		if (!(*listEntry)->complete) {
			(*listEntry)->complete = true;
			(*listEntry)->callCallback();
		}
		listEntry = mRequests.erase(listEntry);
	} else {
		lWarning() << "[LDAP] Couldn't find ldap request " << request << " (id " << request->mMsgId
		           << ") in monitoring.";
	}
	return listEntry;
}

LdapContactSearch *LdapContactProvider::requestSearch(int msgid) {
	auto listEntry = std::find_if(mRequests.begin(), mRequests.end(),
	                              [msgid](const std::shared_ptr<LdapContactSearch> &a) { return a->mMsgId == msgid; });
	if (listEntry != mRequests.end()) return listEntry->get();
	else return NULL;
}

int LdapContactProvider::buildContact(LdapContactFields *contact,
                                      const std::vector<std::pair<std::string, std::string>> &attributes) {
	for (size_t configIndex = 0; contact->mName.second < 0 && configIndex < mConfig["name_attribute"].size();
	     ++configIndex) {
		std::istringstream parser(mConfig["name_attribute"][configIndex]);
		std::string requestedAttribute;
		auto attributesIt =
		    attributes.begin(); // If no attributes, we don't do anything. Each loop must have an attribute or the
		                        // contact cannot be build because of missing attributes.
		while (attributesIt != attributes.end() && std::getline(parser, requestedAttribute, '+')) {
			attributesIt =
			    std::find_if(attributes.begin(), attributes.end(),
			                 // Attributes have been lowered so it doesn't need to to insensitive case comparaison
			                 [requestedAttribute](const std::pair<std::string, std::string> &p) {
				                 return p.first == requestedAttribute;
			                 });
			if (attributesIt != attributes.end()) {
				if (contact->mName.first != "") contact->mName.first += " ";
				contact->mName.first += attributesIt->second;
			}
		}
		if (attributesIt != attributes.end()) contact->mName.second = (int)configIndex;
		else contact->mName.first = ""; // Reset result
	}
	for (size_t attributeIndex = 0; attributeIndex < attributes.size(); ++attributeIndex) {
		for (size_t configIndex = 0; configIndex < mConfig["sip_attribute"].size(); ++configIndex) {
			if (attributes[attributeIndex].first ==
			    mConfig["sip_attribute"][configIndex]) { // Complete SIP with custom data (scheme and domain)
				std::string sip;
				sip += attributes[attributeIndex].second;
				// Test if this sip is ok
				LinphoneAddress *la = linphone_core_interpret_url(mCore->getCCore(), sip.c_str());
				if (!la) {
				} else {
					if (mConfig.count("sip_domain") > 0 && mConfig.at("sip_domain")[0] != "")
						linphone_address_set_domain(la, mConfig.at("sip_domain")[0].c_str());
					char *newSip = linphone_address_as_string(la);
					const char *username = linphone_address_get_username(la);
					char *phoneNumber = username ? linphone_account_normalize_phone_number(
					                                   linphone_core_get_default_account(mCore->getCCore()), username)
					                             : nullptr;
					if (contact->mSip.count(newSip) == 0 || contact->mSip[newSip] == "")
						contact->mSip[newSip] = (phoneNumber ? phoneNumber : "");
					if (phoneNumber) ms_free(phoneNumber);
					ms_free(newSip);
					linphone_address_unref(la);
				}
			}
		}
	}
	// return 1 if the structure has enough data to create a linphone friend
	if (contact->mName.second >= 0 && contact->mSip.size() > 0) return 1;
	else return 0;
}

//*******************************************	ASYNC PROCESSING
// ACTION_NONE => ACTION_INIT => (ACTION_WAIT_DNS) => ACTION_INITIALIZE => (ACTION_WAIT_TLS_CONNECT) => ACTION_BIND
// => ACTION_WAIT_BIND => ACTION_WAIT_REQUEST
bool LdapContactProvider::iterate(void *data) {
	struct timeval pollTimeout = {0, 0};
	LDAPMessage *results = NULL;
	LdapContactProvider *provider = (LdapContactProvider *)data;

	if (provider->mCurrentAction == ACTION_ERROR) {
		lDebug() << "[LDAP] ACTION_ERROR";
		provider->cleanLdap();
		provider->handleSearchResult(NULL);
	} else {
		// not using switch is wanted : we can do severals steps in one iteration if wanted.
		if (provider->mCurrentAction == ACTION_NONE) {
			lDebug() << "[LDAP] ACTION_NONE";
			if (provider->mRequests.size() > 0 && provider->isReadyForStart()) {
				if (provider->mCurrentAction != ACTION_ERROR) {
					if (!provider->mConnected) provider->mCurrentAction = ACTION_INIT;
					else provider->mCurrentAction = ACTION_BIND;
				}
			}
		}

		if (provider->mCurrentAction == ACTION_INIT) {
			lDebug() << "[LDAP] ACTION_INIT";
			if (provider->mConfig["use_sal"][0] == "0") {
				provider->mServerUrl = provider->mConfig["server"];
				provider->mConfigServerIndex =
				    provider->mConfig.size() - 1; // we don't use sal, so mConfigServerIndex doesn't need to correspond
				                                  // to the current mConfig (for TLS resolution)
				provider->mCurrentAction = ACTION_INITIALIZE;
			} else {
				belle_generic_uri_t *uri =
				    belle_generic_uri_parse(provider->mConfig["server"][provider->mConfigServerIndex].c_str());
				if (uri) {
					belle_sip_object_ref(uri);
					const char *cHost = belle_generic_uri_get_host(uri);
					std::string domain = cHost ? cHost : "localhost";
					int port = belle_generic_uri_get_port(uri);
					if (port <= 0) {
						const char *cScheme = belle_generic_uri_get_scheme(uri);
						std::string scheme = cScheme ? Utils::stringToLower(cScheme) : "ldap";
						if (scheme == "ldap") port = 389;
						else port = 636;
					}
					belle_sip_object_unref(uri);
					lDebug() << "[LDAP] Check with Sal : " << domain << ":" << port;
					if (provider->mSalContext) {
						belle_sip_object_unref(provider->mSalContext);
						provider->mSalContext = NULL;
					}
					provider->mSalContext = provider->mCore->getCCore()->sal->resolve(
					    "ldap", "tcp", domain.c_str(), port,
					    (linphone_core_ipv6_enabled(provider->mCore->getCCore()) ? AF_INET6 : AF_INET),
					    ldapServerResolved, provider);
					if (provider->mSalContext) {
						belle_sip_object_ref(provider->mSalContext);
						provider->mCurrentAction = ACTION_WAIT_DNS;
					} else if (provider->mCurrentAction ==
					           ACTION_INIT) // //Sal is NULL : we cannot use it or it could be done synchonously. In the
					                        // last case, mCurrentAction will be changed directly from callback. Try
					                        // another iteration to use it by not setting any new action.
						lError() << "[LDAP] Cannot request DNS : no context for Sal. Retry on next iteration.";
				} else {
					lError() << "[LDAP] Cannot parse the server to URI : "
					         << provider->mConfig["server"][provider->mConfigServerIndex];
					provider->fallbackToNextServerUrl();
				}
			}
		}

		if (provider->mCurrentAction == ACTION_WAIT_DNS) {
			// Do nothing, wait for SAL to do its work
			lDebug() << "[LDAP] ACTION_WAIT_DNS";
		}

		if (provider->mCurrentAction == ACTION_INITIALIZE) {
			lDebug() << "[LDAP] ACTION_INITIALIZE";
			provider->cleanLdap();
			provider->initializeLdap();
		}

		if (provider->mCurrentAction == ACTION_WAIT_TLS_CONNECT) {
			lDebug() << "[LDAP] ACTION_WAIT_TLS_CONNECT";
			provider->ldapTlsConnection();
		}

		if (provider->mCurrentAction == ACTION_BIND) { // Careful : Binds are not thread-safe
			lDebug() << "[LDAP] ACTION_BIND";
			LinphoneLdapAuthMethod auth_mechanism =
			    static_cast<LinphoneLdapAuthMethod>(provider->configValueToInt("auth_method"));
			int ret = 0;
			if ((auth_mechanism == LinphoneLdapAuthMethodAnonymous) ||
			    (auth_mechanism == LinphoneLdapAuthMethodSimple)) {
				std::string bindDn, password;
				if (auth_mechanism == LinphoneLdapAuthMethodSimple && provider->mConfig.count("bind_dn") > 0) {
					bindDn = provider->mConfig.at("bind_dn")[0];
					password = provider->mConfig.at("password")[0];
				} // else : anonymous connection
				struct berval passwd = {(ber_len_t)password.length(), ms_strdup(password.c_str())};
				ret = ldap_sasl_bind(provider->mLd, bindDn.c_str(), NULL, &passwd, NULL, NULL,
				                     &provider->mAwaitingMessageId);
				ms_free(passwd.bv_val);
				if (ret == LDAP_SUCCESS) {
					provider->mCurrentAction = ACTION_WAIT_BIND;
				} else {
					int err = 0;
					ldap_get_option(provider->mLd, LDAP_OPT_RESULT_CODE, &err);
					lError() << "[LDAP] ldap_sasl_bind error returned " << ret << ", err " << err << " ("
					         << ldap_err2string(err) << "), auth_method: " << auth_mechanism;
					provider->mCurrentAction = ACTION_ERROR;
					provider->mAwaitingMessageId = 0;
				}
			} else {
				lError() << "[LDAP] Special authentifications is not supported. You must use SIMPLE or ANONYMOUS";
				provider->mCurrentAction = ACTION_ERROR;
				provider->mAwaitingMessageId = 0;
			}
		}

		if (provider->mCurrentAction == ACTION_WAIT_BIND) {
			lDebug() << "[LDAP] ACTION_WAIT_BIND";
			int ret =
			    (int)ldap_result(provider->mLd, provider->mAwaitingMessageId, LDAP_MSG_ONE, &pollTimeout, &results);
			if (ret == LDAP_RES_BIND) {
				ret = ldap_parse_sasl_bind_result(provider->mLd, results, NULL, 1); // Auto ldap_msgfree(results)
				if (ret == LDAP_SUCCESS) {
					lDebug() << "[LDAP] Bind successful";
					provider->mConnected = true;
					provider->mAwaitingMessageId = 0;
					provider->mCurrentAction = ACTION_WAIT_REQUEST;
				} else {
					lError() << "[LDAP] Cannot bind to server : " << ret << " (" << ldap_err2string(ret) << ")";
					provider->mCurrentAction = ACTION_ERROR;
				}
			}
		}

		if (provider->mCurrentAction == ACTION_WAIT_REQUEST) {
			lDebug() << "[LDAP] ACTION_WAIT_REQUEST";
			size_t requestSize = 0;
			if (provider->mLd && provider->mConnected) {
				// check for pending searches
				for (auto it = provider->mRequests.begin(); it != provider->mRequests.end();) {
					if (!(*it)) it = provider->mRequests.erase(it);
					else if ((*it)->mMsgId == 0) {
						int ret;
						lInfo() << "[LDAP] Found pending search " << it->get() << " (for " << (*it)->mFilter
						        << "), launching...";
						ret = provider->search(*it);
						if (ret != LDAP_SUCCESS) {
							it = provider->cancelSearch(it->get());
						} else ++it;
					} else ++it;
				}
				requestSize = provider->mRequests.size();
			}
			if (requestSize > 0) { // No need to check connectivity as it is checked before
				// never block
				int ret = (int)ldap_result(provider->mLd, LDAP_RES_ANY, LDAP_MSG_ONE, &pollTimeout, &results);
				switch (ret) {
					case -1: {
						int lastError = errno;
						lWarning() << "[LDAP] : Error in ldap_result : returned -1 (req_count " << requestSize
						           << "): " << ldap_err2string(lastError);
						break;
					}
					case 0:
						break; // nothing to do

					case LDAP_RES_BIND: {
						lWarning() << "[LDAP] iterate: unexpected LDAP_RES_BIND";
						if (results) ldap_msgfree(results);
						break;
					}
					case LDAP_RES_EXTENDED:
					case LDAP_RES_SEARCH_ENTRY:
					case LDAP_RES_SEARCH_REFERENCE:
					case LDAP_RES_INTERMEDIATE:
					case LDAP_RES_SEARCH_RESULT: {
						provider->handleSearchResult(results);
						break;
					}
					case LDAP_RES_MODIFY:
					case LDAP_RES_ADD:
					case LDAP_RES_DELETE:
					case LDAP_RES_MODDN:
					case LDAP_RES_COMPARE:
					default:
						lWarning() << "[LDAP] Unhandled LDAP result " << ret;
						break;
				}
				if (results) ldap_msgfree(results);
			}
		}
	}
	return true;
}

void LdapContactProvider::ldapServerResolved(void *data, belle_sip_resolver_results_t *results) {
	LdapContactProvider *provider = (LdapContactProvider *)(data);
	provider->mServerUrl.clear();
	provider->mServerUrlIndex = 0;
	auto addr = belle_sip_resolver_results_get_addrinfos(results);
	if (!addr) {
		lError() << "[LDAP] Server resolution failed, no address can be found.";
	} else lDebug() << "[LDAP] Server resolution successful.";
	while (addr) {
		const struct addrinfo *ai = addr;
		int err;
		char ipstring[64];
		err = bctbx_addrinfo_to_printable_ip_address(ai, ipstring, sizeof(ipstring));
		if (err != 0)
			lError() << "[LDAP] DNS resolver: bctbx_addrinfo_to_printable_ip_address error " << gai_strerror(err);
		else lDebug() << "[LDAP] find : " << ipstring;
		provider->mServerUrl.push_back("ldap://" + std::string(ipstring));
		addr = addr->ai_next;
	}
	if (provider->mServerUrl.size() == 0) provider->fallbackToNextServerUrl();
	else provider->mCurrentAction = ACTION_INITIALIZE;
}

void LdapContactProvider::handleSearchResult(LDAPMessage *message) {
	if (message) {
		int msgtype = ldap_msgtype(message);
		LdapContactSearch *req = requestSearch(ldap_msgid(message));
		switch (msgtype) {
			case LDAP_RES_SEARCH_ENTRY:
			case LDAP_RES_EXTENDED: {
				LDAPMessage *entry = ldap_first_entry(mLd, message);
				// Message can be a list. Loop on entries
				while (entry != NULL) {
					LdapContactFields ldapData;
					bool contact_complete = false;
					BerElement *ber = NULL;
					char *attr = ldap_first_attribute(mLd, entry, &ber);
					// Each entry is about a contact. Loop on all attributes and fill contact. We do not stop when
					// contact is completed to know if there are better attributes
					std::vector<std::pair<std::string, std::string>> attributes;
					while (attr) {
						struct berval **values = ldap_get_values_len(mLd, entry, attr);
						struct berval **it = values;
						while (values && *it && (*it)->bv_val && (*it)->bv_len) {
							attributes.push_back({Utils::stringToLower(attr), Utils::utf8ToLocale((*it)->bv_val)});
							it++;
						}
						if (values) ldap_value_free_len(values);
						ldap_memfree(attr);
						attr = ldap_next_attribute(mLd, entry, ber);
					}
					if (attributes.size() > 0 && mConfig.count("debug") > 0 &&
					    LinphoneLdapDebugLevelVerbose ==
					        static_cast<LinphoneLdapDebugLevel>(atoi(mConfig["debug"][0].c_str()))) {
						std::string attributesDebug = attributes[0].first + "=" + attributes[0].second;
						for (size_t i = 1; i < attributes.size(); ++i)
							attributesDebug += ", " + attributes[i].first + "=" + attributes[i].second;
						lInfo() << "[LDAP] Got attributes from Server : " << attributesDebug;
					}
					contact_complete = buildContact(&ldapData, attributes);
					if (contact_complete) {
						std::shared_ptr<Friend> lFriend = Friend::create(mCore, ldapData.mName.first);
						for (auto sipAddress : ldapData.mSip) {
							std::shared_ptr<Address> addr = mCore->interpretUrl(sipAddress.first, false);
							if (addr) {
								addr->setDisplayName(ldapData.mName.first);
								lFriend->addAddress(addr);
								if (!sipAddress.second.empty()) {
									lFriend->addPhoneNumber(sipAddress.second);
								}

								auto vCard = lFriend->getVcard();
								if (vCard && vCard->getUid().empty()) {
									vCard->generateUniqueId();
									lFriend->setRefKey(vCard->getUid());
								}
							}
						}
						int maxResults = atoi(mConfig["max_results"][0].c_str());
						if (maxResults == 0 || req->mFoundCount < (unsigned int)maxResults) {
							req->mFoundEntries.push_back(lFriend);
							++req->mFoundCount;
						} else { // Have more result (requested max_results+1). Do not store this result to
							     // avoid missunderstanding from user.
							req->mHaveMoreResults = true;
						}
					}
					if (ber) ber_free(ber, 0);
					if (attr) ldap_memfree(attr);
					entry = ldap_next_entry(mLd, entry);
				}
			} break;
			case LDAP_RES_SEARCH_RESULT: {
				// this one is received when a request is finished
				cancelSearch(req);
			} break;
			default:
				lWarning() << "[LDAP] Unhandled message type " << msgtype;
				break;
		}
	} else {
		for (auto it = mRequests.begin(); it != mRequests.end(); ++it) {
			(*it)->complete = true;
			(*it)->callCallback();
		}
		mRequests.clear();
	}
}

bool LdapContactProvider::isReadyForStart() {
	return mLastRequestTime + (uint64_t)configValueToInt("delay") < bctbx_get_cur_time_ms();
}

LINPHONE_END_NAMESPACE
