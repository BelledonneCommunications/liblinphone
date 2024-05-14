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

#include "auth-info.h"
#include "bellesip_sal/sal_impl.h"
#include "linphone/lpconfig.h"
#include "logger/logger.h"

#include <algorithm>

using namespace std;

LINPHONE_BEGIN_NAMESPACE

AuthInfo::AuthInfo(const std::string &username, const std::string &realm, const std::string &domain) {
	AuthInfo::init(username, "", "", "", realm, domain, "");
}

AuthInfo::AuthInfo(const string &username,
                   const string &userid,
                   const string &passwd,
                   const string &ha1,
                   const string &realm,
                   const string &domain) {
	AuthInfo::init(username, userid, passwd, ha1, realm, domain, "");
}

AuthInfo::AuthInfo(const string &username,
                   const string &userid,
                   const string &passwd,
                   const string &ha1,
                   const string &realm,
                   const string &domain,
                   const string &algorithm) {
	AuthInfo::init(username, userid, passwd, ha1, realm, domain, algorithm);
}
AuthInfo::AuthInfo(const string &username,
                   const string &userid,
                   const string &passwd,
                   const string &ha1,
                   const string &realm,
                   const string &domain,
                   const string &algorithm,
                   const list<string> &availableAlgorithms) {
	AuthInfo::init(username, userid, passwd, ha1, realm, domain, algorithm, availableAlgorithms);
}

AuthInfo::AuthInfo(const std::string &username, std::shared_ptr<BearerToken> token, const std::string &realm) {
	mUsername = username;
	mAccessToken = token;
	mRealm = realm;
}

void AuthInfo::init(const string &username,
                    const string &userid,
                    const string &passwd,
                    const string &ha1,
                    const string &realm,
                    const string &domain,
                    const string &algorithm) {
	mUsername = username;
	mUserid = userid;
	mPasswd = passwd;
	mHa1 = ha1;
	mRealm = realm;
	mDomain = domain;
	if (!ha1.empty() && algorithm.empty()) {
		/* When ha1 is specified, algorithm must be specified too. For backward compatibility when this is not the case,
		 * we assume it is md5.*/
		setAlgorithm("MD5");
	} else setAlgorithm(algorithm);
	mNeedToRenewHa1 = false;
}
void AuthInfo::init(const string &username,
                    const string &userid,
                    const string &passwd,
                    const string &ha1,
                    const string &realm,
                    const string &domain,
                    const string &algorithm,
                    const list<string> &availableAlgorithms) {
	init(username, userid, passwd, ha1, realm, domain, algorithm);
	setAvailableAlgorithms(availableAlgorithms);
}
AuthInfo::AuthInfo(LpConfig *config, string key) {
	const char *username, *userid, *passwd, *ha1, *realm, *domain, *tls_cert_path, *tls_key_path, *tls_key_password,
	    *algo;
	bctbx_list_t *algos;
	const char *tokenString;

	username = linphone_config_get_string(config, key.c_str(), "username", "");
	userid = linphone_config_get_string(config, key.c_str(), "userid", "");
	passwd = linphone_config_get_string(config, key.c_str(), "passwd", "");
	ha1 = linphone_config_get_string(config, key.c_str(), "ha1", "");
	realm = linphone_config_get_string(config, key.c_str(), "realm", "");
	domain = linphone_config_get_string(config, key.c_str(), "domain", "");
	tls_cert_path = linphone_config_get_string(config, key.c_str(), "client_cert_chain", "");
	tls_key_path = linphone_config_get_string(config, key.c_str(), "client_cert_key", "");
	tls_key_password = linphone_config_get_string(config, key.c_str(), "client_cert_key_passsword", "");
	algo = linphone_config_get_string(config, key.c_str(), "algorithm", "");
	algos = linphone_config_get_string_list(config, key.c_str(), "available_algorithms", nullptr);

	tokenString = linphone_config_get_string(config, key.c_str(), "access_token", nullptr);
	if (tokenString) mAccessToken = BearerToken::createFromConfig(tokenString)->toSharedPtr();
	tokenString = linphone_config_get_string(config, key.c_str(), "refresh_token", nullptr);
	if (tokenString) mRefreshToken = BearerToken::createFromConfig(tokenString)->toSharedPtr();
	mAuthServer = linphone_config_get_string(config, key.c_str(), "authorization_server", "");
	mTokenEndpoint = linphone_config_get_string(config, key.c_str(), "token_endpoint", "");
	mClientId = linphone_config_get_string(config, key.c_str(), "client_id", "");

	setTlsCertPath(tls_cert_path);
	setTlsKeyPath(tls_key_path);
	setTlsKeyPassword(tls_key_password);

	init(username, userid, passwd, ha1, realm, domain, algo);

	if (algos) {
		bctbx_list_t *elem;
		for (elem = algos; elem != NULL; elem = elem->next)
			addAvailableAlgorithm((const char *)elem->data);
		bctbx_list_free_with_data(algos, (bctbx_list_free_func)bctbx_free);
	}
}

AuthInfo *AuthInfo::clone() const {
	AuthInfo *ai = new AuthInfo(*this);
	return ai;
}

const string &AuthInfo::getUsername() const {
	return mUsername;
}

const string &AuthInfo::getAlgorithm() const {
	return mAlgorithm;
}
const list<string> &AuthInfo::getAvailableAlgorithms() const {
	return mAvailableAlgorithms;
}
const string &AuthInfo::getPassword() const {
	return mPasswd;
}

const string &AuthInfo::getUserid() const {
	return mUserid;
}

const string &AuthInfo::getRealm() const {
	return mRealm;
}

const string &AuthInfo::getDomain() const {
	return mDomain;
}

const string &AuthInfo::getHa1() const {
	return mHa1;
}

bool AuthInfo::getNeedToRenewHa1() const {
	return mNeedToRenewHa1;
}

const string &AuthInfo::getTlsCert() const {
	return mTlsCert;
}

const string &AuthInfo::getTlsKey() const {
	return mTlsKey;
}

const string &AuthInfo::getTlsCertPath() const {
	return mTlsCertPath;
}

const string &AuthInfo::getTlsKeyPath() const {
	return mTlsKeyPath;
}

const string &AuthInfo::getTlsKeyPassword() const {
	return mTlsKeyPassword;
}

void AuthInfo::setPassword(const string &passwd) {
	if (!passwd.empty() && mPasswd != passwd && !mHa1.empty()) {
		setNeedToRenewHa1(true);
	}
	mPasswd = passwd;
}

void AuthInfo::setUsername(const string &username) {
	if (!username.empty() && mUsername != username && !mHa1.empty()) {
		setNeedToRenewHa1(true);
	}
	mUsername = username;
}

void AuthInfo::setAlgorithm(const string &algorithm) { // Select algorithm
	if (!algorithm.empty() && algorithm != "MD5" && algorithm != "SHA-256") {
		lError() << "Given algorithm is not correct. Set algorithm failed";
	}
	if (!algorithm.empty() && mAlgorithm != algorithm && !mHa1.empty()) {
		setNeedToRenewHa1(true);
	}
	mAlgorithm = algorithm;
	if (!algorithm.empty()) addAvailableAlgorithm(algorithm);
}

void AuthInfo::setAvailableAlgorithms(const list<string> &algorithms) {
	mAvailableAlgorithms = algorithms;
}

void AuthInfo::addAvailableAlgorithm(const string &algorithm) {
	if (!algorithm.empty() && algorithm != "MD5" && algorithm != "SHA-256") {
		lError() << "Given algorithm is not correct. Add algorithm failed";
	} else if (find(mAvailableAlgorithms.begin(), mAvailableAlgorithms.end(), algorithm) == mAvailableAlgorithms.end())
		mAvailableAlgorithms.push_back(algorithm);
}

void AuthInfo::clearAvailableAlgorithms() {
	mAvailableAlgorithms.clear();
}

void AuthInfo::setUserid(const string &userid) {
	if (!userid.empty() && mUserid != userid && !mHa1.empty()) {
		setNeedToRenewHa1(true);
	}
	mUserid = userid;
}

void AuthInfo::setRealm(const string &realm) {
	if (!realm.empty() && mRealm != realm && !mHa1.empty()) {
		setNeedToRenewHa1(true);
	}
	mRealm = realm;
}

void AuthInfo::setDomain(const string &domain) {
	mDomain = domain;
}

void AuthInfo::setHa1(const string &ha1) {
	if (!ha1.empty()) setNeedToRenewHa1(false);
	mHa1 = ha1;
}

void AuthInfo::setNeedToRenewHa1(bool needToRenewHa1) {
	mNeedToRenewHa1 = needToRenewHa1;
}

void AuthInfo::setTlsCert(const string &tlsCert) {
	mTlsCert = tlsCert;
}

void AuthInfo::setTlsKey(const string &tlsKey) {
	mTlsKey = tlsKey;
}

void AuthInfo::setTlsCertPath(const string &tlsCertPath) {
	mTlsCertPath = tlsCertPath;
}

void AuthInfo::setTlsKeyPath(const string &tlsKeyPath) {
	mTlsKeyPath = tlsKeyPath;
}

void AuthInfo::setTlsKeyPassword(const string &tlsKeyPassword) {
	mTlsKeyPassword = tlsKeyPassword;
}

void AuthInfo::writeConfig(LpConfig *config, int pos) {
	char key[50];
	char *myHa1;
	bool_t store_ha1_passwd = !!linphone_config_get_int(config, "sip", "store_ha1_passwd", 1);
	bctbx_list_t *algos = NULL;

	sprintf(key, "auth_info_%i", pos);
	linphone_config_clean_section(config, key);

	if (linphone_config_get_int(config, "sip", "store_auth_info", 1) == 0) {
		return;
	}
	if ((getNeedToRenewHa1() || getHa1().empty()) && !getRealm().empty() && !getPassword().empty() &&
	    (!getUsername().empty() || !getUserid().empty()) && store_ha1_passwd) {
		/* Default algorithm is MD5 if it's NULL */
		if (getAlgorithm().empty() || getAlgorithm().compare("MD5") == 0) {
			myHa1 = reinterpret_cast<char *>(ms_malloc(33));
			sal_auth_compute_ha1(getUserid().empty() ? getUsername().c_str() : getUserid().c_str(), getRealm().c_str(),
			                     getPassword().c_str(), myHa1);
			setHa1(myHa1);
			ms_free(myHa1);
		}

		/* If algorithm is SHA-256, calcul ha1 by sha256*/
		else if (getAlgorithm().compare("SHA-256") == 0) {
			myHa1 = reinterpret_cast<char *>(ms_malloc(65));
			sal_auth_compute_ha1_for_algorithm(getUserid().empty() ? getUsername().c_str() : getUserid().c_str(),
			                                   getRealm().c_str(), getPassword().c_str(), myHa1, 65,
			                                   getAlgorithm().c_str());
			setHa1(myHa1);
			ms_free(myHa1);
		}
	}
	linphone_config_set_string(config, key, "username", getUsername().c_str());
	linphone_config_set_string(config, key, "userid", getUserid().c_str());
	linphone_config_set_string(config, key, "ha1", getHa1().c_str());

	if (store_ha1_passwd && !getHa1().empty()) {
		/*if we have our ha1 and store_ha1_passwd set to TRUE, then drop the clear text password for security*/
		setPassword("");
	}
	linphone_config_set_string(config, key, "passwd", getPassword().c_str());
	linphone_config_set_string(config, key, "realm", getRealm().c_str());
	linphone_config_set_string(config, key, "domain", getDomain().c_str());
	linphone_config_set_string(config, key, "client_cert_chain", getTlsCertPath().c_str());
	linphone_config_set_string(config, key, "client_cert_key", getTlsKeyPath().c_str());
	linphone_config_set_string(config, key, "client_cert_key_password", getTlsKeyPassword().c_str());
	linphone_config_set_string(config, key, "algorithm", getAlgorithm().c_str());

	if (mAvailableAlgorithms.size() > 0) {
		for (auto i = mAvailableAlgorithms.begin(); i != mAvailableAlgorithms.end(); ++i)
			algos = bctbx_list_append(algos, (void *)i->c_str());
		linphone_config_set_string_list(config, key, "available_algorithms", algos);
		bctbx_list_free(algos);
	}
	if (mAccessToken) {
		linphone_config_set_string(config, key, "access_token", mAccessToken->toConfigString().c_str());
	}
	if (mRefreshToken) {
		linphone_config_set_string(config, key, "refresh_token", mRefreshToken->toConfigString().c_str());
	}
	if (!mAuthServer.empty()) {
		linphone_config_set_string(config, key, "authorization_server", mAuthServer.c_str());
	}
	if (!mTokenEndpoint.empty()) {
		linphone_config_set_string(config, key, "token_endpoint", mTokenEndpoint.c_str());
	}
	if (!mClientId.empty()) {
		linphone_config_set_string(config, key, "client_id", mClientId.c_str());
	}
}

std::string AuthInfo::toString() const {
	std::ostringstream ss;

	ss << "Username[" << mUsername << "];";
	if (!mUserid.empty()) ss << "Userid[" << mUserid << "];";
	ss << "Realm[" << mRealm << "];";
	ss << "Domain[" << mDomain << "];";
	if (!mAlgorithm.empty()) ss << "Algorithm[" << mAlgorithm << "];";
	ss << "AvailableAlgorithms[";
	if (mAvailableAlgorithms.size() > 0) {
		auto index = mAvailableAlgorithms.begin();
		ss << *(index++);
		while (index != mAvailableAlgorithms.end())
			ss << "," << *(index++);
	}
	ss << "];";
	if (!mAuthServer.empty()) ss << "AuthServer[" << mAuthServer << "];";
	if (!mTokenEndpoint.empty()) ss << "TokenEndpoint[" << mTokenEndpoint << "];";
	if (!mClientId.empty()) ss << "ClientID[" << mClientId << "];";
	if (mAccessToken) ss << "AccessToken[" << mAccessToken->getToken().substr(0, 4) << "...];";
	if (mRefreshToken) ss << "RefreshToken[" << mRefreshToken->getToken().substr(0, 4) << "...];";
	return ss.str();
}

// Check if Authinfos are the same without taking account algorithms
bool AuthInfo::isEqualButAlgorithms(const AuthInfo *authInfo) const {
	return authInfo && getUsername() == authInfo->getUsername() && getUserid() == authInfo->getUserid() &&
	       getRealm() == authInfo->getRealm() && getDomain() == authInfo->getDomain();
}
LINPHONE_END_NAMESPACE
