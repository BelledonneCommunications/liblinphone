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

#include "nat-policy.h"
#include "core/core.h"

#include <cstring>

#include "private.h"

LINPHONE_BEGIN_NAMESPACE

NatPolicy::AsyncHandle NatPolicy::sAsyncHandleGenerator = 0;

NatPolicy::NatPolicy(const std::shared_ptr<Core> &core, NatPolicy::ConstructionMethod method, const std::string &value)
    : CoreAccessor(core) {
	LpConfig *config = linphone_core_get_config(core->getCCore());
	char ref[17] = {0};
	belle_sip_random_token(ref, 16);
	mRef = ref;

	if (method == ConstructionMethod::Default) {

	} else if (method == ConstructionMethod::FromSectionName) {
		initFromSection(config, value.c_str());
	} else if (method == ConstructionMethod::FromRefName) {
		int index;
		for (index = 0;; index++) {
			std::ostringstream section;
			section << "nat_policy_" << index;
			if (linphone_config_has_section(config, section.str().c_str())) {
				const char *config_ref = linphone_config_get_string(config, section.str().c_str(), "ref", NULL);
				if ((config_ref != NULL) && (strcmp(config_ref, value.c_str()) == 0)) {
					initFromSection(config, section.str().c_str());
					break;
				}
			} else {
				lError() << "There is no NatPolicy with ref [" << value << "]";
				break;
			}
		}
	}
}

NatPolicy::NatPolicy(const NatPolicy &other)
    : HybridObject<LinphoneNatPolicy, NatPolicy>(other), CoreAccessor(other.getCore()) {
	mStunServer = other.mStunServer;
	mStunServerUsername = other.mStunServerUsername;
	mRef = other.mRef;
	if (mRef.empty()) {
		char ref[17] = {0};
		belle_sip_random_token(ref, 16);
		mRef = ref;
	}
	mStunEnabled = other.mStunEnabled;
	mTurnEnabled = other.mTurnEnabled;
	mIceEnabled = other.mIceEnabled;
	mUpnpEnabled = other.mUpnpEnabled;
	mTurnUdpEnabled = other.mTurnUdpEnabled;
	mTurnTcpEnabled = false;
	mTurnTlsEnabled = false;
}

NatPolicy::~NatPolicy() {
	clearResolverContexts();
}

NatPolicy *NatPolicy::clone() const {
	return new NatPolicy(*this);
}

void NatPolicy::clearResolverContexts() {
	if (mStunResolverContext) {
		mStunResolverContext.cancel();
		mStunResolverContext.reset();
	}
	if (mResolverResults) {
		belle_sip_object_unref(mResolverResults);
		mResolverResults = nullptr;
	}
}

/* Simply cancel pending DNS resoltion, as the core is going to shutdown.*/
void NatPolicy::release() {
	clearResolverContexts();
	mResolverResultsFunctions.clear();
}

bool NatPolicy::stunServerActivated() const {
	return !mStunServer.empty() && (mStunEnabled || mTurnEnabled);
}

void NatPolicy::saveToConfig(LinphoneConfig *config, int index) const {
	char *section;
	bctbx_list_t *l = NULL;

	section = belle_sip_strdup_printf("nat_policy_%i", index);
	linphone_config_set_string(config, section, "ref", mRef.c_str());
	linphone_config_set_string(config, section, "stun_server", mStunServer.c_str());
	linphone_config_set_string(config, section, "stun_server_username", mStunServerUsername.c_str());
	if (mUpnpEnabled) {
		l = bctbx_list_append(l, (void *)"upnp");
	} else {
		if (mStunEnabled) l = bctbx_list_append(l, (void *)"stun");
		if (mTurnEnabled) l = bctbx_list_append(l, (void *)"turn");
		if (mIceEnabled) l = bctbx_list_append(l, (void *)"ice");
	}
	linphone_config_set_string_list(config, section, "protocols", l);
	belle_sip_free(section);
	bctbx_list_free(l);
}

void NatPolicy::clearConfigFromIndex(LinphoneConfig *config, int index) {
	int purged = 0;
	while (true) {
		std::ostringstream ostr;
		ostr << "nat_policy_" << index;
		if (linphone_config_has_section(config, ostr.str().c_str())) {
			linphone_config_clean_section(config, ostr.str().c_str());
			++purged;
		} else break;
		++index;
	}
	/* Warn about suspicious number of orphan NatPolicy sections.
	 * A bug occured December 2022 caused a massive leak of nat_policy_X sections in configuration file.
	 */
	if (purged > 5) {
		lWarning() << "Purged [" << purged << "] unused NatPolicy sections from config file.";
	}
}

void NatPolicy::clear() {
	clearResolverContexts();
	mStunServer.clear();
	mStunServerUsername.clear();
	mStunEnabled = false;
	mTurnEnabled = false;
	mIceEnabled = false;
	mUpnpEnabled = false;
	mTurnUdpEnabled = false;
	mTurnTcpEnabled = false;
	mTurnTlsEnabled = false;
}

void NatPolicy::setStunServer(const std::string &stunServer) {
	mStunServer = stunServer;
	clearResolverContexts();
}

const std::string &NatPolicy::getStunServer() const {
	return mStunServer;
}

void NatPolicy::setStunServerUsername(const std::string &stunServerUsername) {
	mStunServerUsername = stunServerUsername;
}
const std::string &NatPolicy::getStunServerUsername() const {
	return mStunServerUsername;
}

void NatPolicy::setNatV4Address(const std::string &natV4Address) {
	mNatV4Address = natV4Address;
}

const std::string &NatPolicy::getNatV4Address() const {
	return mNatV4Address;
}

void NatPolicy::setNatV6Address(const std::string &natV6Address) {
	mNatV6Address = natV6Address;
}
const std::string &NatPolicy::getNatV6Address() const {
	return mNatV6Address;
}

void NatPolicy::stunServerResolved(belle_sip_resolver_results_t *results) {
	if (mResolverResults) {
		belle_sip_object_unref(mResolverResults);
		mResolverResults = nullptr;
	}

	if (belle_sip_resolver_results_get_addrinfos(results)) {
		ms_message("Stun server resolution successful.");
		belle_sip_object_ref(results);
		mResolverResults = results;
	} else {
		ms_warning("Stun server resolution failed.");
	}
	if (mStunResolverContext) {
		mStunResolverContext.reset();
	}
	const struct addrinfo *ais =
	    mResolverResults ? belle_sip_resolver_results_get_addrinfos(mResolverResults) : nullptr;
	for (auto &p : mResolverResultsFunctions)
		p.second(ais);
	mResolverResultsFunctions.clear();
}

bool NatPolicy::resolveStunServer() {
	LinphoneCore *lc = getCore()->getCCore();
	const char *service = NULL;

	if (stunServerActivated() && (lc->sal != NULL) && !mStunResolverContext) {
		char host[NI_MAXHOST];
		int port = 0;
		linphone_parse_host_port(mStunServer.c_str(), host, sizeof(host), &port);
		if (mTurnEnabled) service = "turn";
		else if (mStunEnabled) service = "stun";
		if (service != NULL) {
			int family = AF_INET;
			if (linphone_core_ipv6_enabled(lc) == TRUE) family = AF_INET6;
			ms_message("Starting stun server resolution [%s]", host);
			auto lambda = [this](belle_sip_resolver_results_t *results) { this->stunServerResolved(results); };
			if (port == 0) {
				port = 3478;
				mStunResolverContext = lc->sal->resolve(service, "udp", host, port, family, lambda);
			} else {
				mStunResolverContext = lc->sal->resolveA(host, port, family, lambda);
			}
			if (mStunResolverContext) {
				return true;
			}
		}
	}
	return false;
}

const struct addrinfo *NatPolicy::getStunServerAddrinfo() {
	/*
	 * It is critical not to block for a long time if it can't be resolved, otherwise this stucks the main thread when
	 * making a call. On the contrary, a fully asynchronous call initiation is complex to develop. The compromise is
	 * then:
	 *  - have a cache of the stun server addrinfo
	 *  - this cached value is returned when it is non-null
	 *  - an asynchronous resolution is asked each time this function is called to ensure frequent refreshes of the
	 * cached value.
	 *  - if no cached value exists, block for a short time; this case must be unprobable because the resolution will be
	 * asked each time the stun server value is changed.
	 */
	if (stunServerActivated() && (mResolverResults == nullptr)) {
		int wait_ms = 0;
		int wait_limit = 1000;
		resolveStunServer();
		while ((mResolverResults == nullptr) && (mStunResolverContext) && (wait_ms < wait_limit)) {
			getCore()->getCCore()->sal->iterate();
			ms_usleep(10000);
			wait_ms += 10;
		}
	}
	return mResolverResults ? belle_sip_resolver_results_get_addrinfos(mResolverResults) : nullptr;
}

NatPolicy::AsyncHandle
NatPolicy::getStunServerAddrinfoAsync(const std::function<void(const struct addrinfo *)> &onResults) {
	if (stunServerActivated() && (mResolverResults == nullptr)) {
		/* need to request resolution */
		mResolverResultsFunctions[++sAsyncHandleGenerator] = onResults;
		if (resolveStunServer()) return sAsyncHandleGenerator;
	} else {
		/* results already available, invoke the lambda immediately.*/
		onResults(mResolverResults ? belle_sip_resolver_results_get_addrinfos(mResolverResults) : nullptr);
	}
	return 0;
}

void NatPolicy::cancelAsync(AsyncHandle h) {
	auto it = mResolverResultsFunctions.find(h);
	if (it != mResolverResultsFunctions.end()) {
		mResolverResultsFunctions.erase(it);
	} else {
		lError() << "NatPolicy: no AsyncHandle with id " << h;
	}
}

void NatPolicy::initFromSection(const LinphoneConfig *config, const char *section) {
	const char *ref = linphone_config_get_string(config, section, "ref", nullptr);
	if (ref != nullptr) mRef = ref;
	mStunServer = linphone_config_get_string(config, section, "stun_server", "");
	mStunServerUsername = linphone_config_get_string(config, section, "stun_server_username", "");
	mTurnUdpEnabled = !!linphone_config_get_bool(config, section, "turn_enable_udp", TRUE);
	mTurnTcpEnabled = !!linphone_config_get_bool(config, section, "turn_enable_tcp", FALSE);
	mTurnTlsEnabled = !!linphone_config_get_bool(config, section, "turn_enable_tls", FALSE);
	bctbx_list_t *l = linphone_config_get_string_list(config, section, "protocols", NULL);

	if (l != NULL) {
		bctbx_list_t *elem;
		for (elem = l; elem != NULL; elem = elem->next) {
			const char *value = (const char *)elem->data;
			if (strcmp(value, "stun") == 0) enableStun(true);
			else if (strcmp(value, "turn") == 0) enableTurn(true);
			else if (strcmp(value, "ice") == 0) enableIce(true);
			else if (strcmp(value, "upnp") == 0) mUpnpEnabled = true;
		}
		bctbx_list_free_with_data(l, (bctbx_list_free_func)ms_free);
	}
}

LINPHONE_END_NAMESPACE
