/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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
#ifndef AUTH_INFO_H
#define AUTH_INFO_H

#include "bearer-token.h"
#include "belle-sip/object++.hh"
#include "linphone/api/c-types.h"

LINPHONE_BEGIN_NAMESPACE

class AuthInfo : public bellesip::HybridObject<LinphoneAuthInfo, AuthInfo> {
public:
	AuthInfo(const std::string &username = "",
	         const std::string &userid = "",
	         const std::string &passwd = "",
	         const std::string &ha1 = "",
	         const std::string &realm = "",
	         const std::string &domain = "");
	AuthInfo(const std::string &username,
	         const std::string &userid,
	         const std::string &passwd,
	         const std::string &ha1,
	         const std::string &realm,
	         const std::string &domain,
	         const std::string &algorithm);
	AuthInfo(const std::string &username,
	         const std::string &userid,
	         const std::string &passwd,
	         const std::string &ha1,
	         const std::string &realm,
	         const std::string &domain,
	         const std::string &algorithm,
	         const std::list<std::string> &availableAlgorithms);
	AuthInfo(const std::string &username, const std::string &realm, const std::string &domain);
	AuthInfo(const std::string &username, std::shared_ptr<BearerToken> accessToken, const std::string &realm);
	AuthInfo(const AuthInfo &other) = default;

	AuthInfo(LpConfig *config, std::string key);
	AuthInfo *clone() const override;
	void init(const std::string &username,
	          const std::string &userid,
	          const std::string &passwd,
	          const std::string &ha1,
	          const std::string &realm,
	          const std::string &domain,
	          const std::string &algorithm);
	void init(const std::string &username,
	          const std::string &userid,
	          const std::string &passwd,
	          const std::string &ha1,
	          const std::string &realm,
	          const std::string &domain,
	          const std::string &algorithm,
	          const std::list<std::string> &availableAlgorithms);

	void setPassword(const std::string &passwd);
	void setUsername(const std::string &username);
	void setAlgorithm(const std::string &algorithm);                       // Specify the algorithm to choose.
	void setAvailableAlgorithms(const std::list<std::string> &algorithms); // Set without testing unicity
	void addAvailableAlgorithm(
	    const std::string &algorithm); // Add a unique algorithm : Algorithms that already exist will not be added.
	void setUserid(const std::string &userid);
	void setRealm(const std::string &realm);
	void setDomain(const std::string &domain);
	void setHa1(const std::string &ha1);
	void setTlsCert(const std::string &tlsCert);
	void setTlsKey(const std::string &tlsKey);
	void setTlsCertPath(const std::string &tlsCertPath);
	void setTlsKeyPath(const std::string &tlsKeyPath);
	void setTlsKeyPassword(const std::string &tlsKeyPassword);
	void setAccessToken(const std::shared_ptr<BearerToken> &accessToken) {
		mAccessToken = accessToken;
	};
	void setRefreshToken(const std::shared_ptr<BearerToken> &refreshToken) {
		mRefreshToken = refreshToken;
	};
	void setAuthorizationServer(const std::string &uri) {
		mAuthServer = uri;
	}
	void setTokenEndpointUri(const std::string &uri) {
		mTokenEndpoint = uri;
	}
	void setClientId(const std::string &client_id) {
		mClientId = client_id;
	}
	void writeConfig(LpConfig *config, int pos);
	void clearAvailableAlgorithms(); // Remove all algorithms

	const std::string &getUsername() const;
	const std::string &getAlgorithm() const; // Get the selected algorithm
	const std::list<std::string> &getAvailableAlgorithms() const;
	const std::string &getPassword() const;
	const std::string &getUserid() const;
	const std::string &getRealm() const;
	const std::string &getDomain() const;
	const std::string &getHa1() const;
	const std::string &getTlsCert() const;
	const std::string &getTlsKey() const;
	const std::string &getTlsCertPath() const;
	const std::string &getTlsKeyPath() const;
	const std::string &getTlsKeyPassword() const;
	std::shared_ptr<const BearerToken> getAccessToken() const {
		return mAccessToken;
	}
	std::shared_ptr<const BearerToken> getRefreshToken() const {
		return mRefreshToken;
	}
	const std::string &getAuthorizationServer() const {
		return mAuthServer;
	}
	const std::string &getTokenEndpointUri() const {
		return mTokenEndpoint;
	}
	const std::string &getClientId() const {
		return mClientId;
	}

	time_t getExpires() const {
		return mExpires;
	}
	void setExpires(time_t expires) {
		mExpires = expires;
	}

	std::string toString() const override;
	// Check if Authinfos are the same without taking account algorithms
	bool isEqualButAlgorithms(const AuthInfo *authInfo) const;

private:
	std::string mUsername;
	std::string mUserid;
	std::string mPasswd;
	std::string mHa1;
	std::string mRealm;
	std::string mDomain;
	std::string mAlgorithm;                      // Selected algorithm
	std::list<std::string> mAvailableAlgorithms; // Pool of available algorithms for this AuthInfo
	std::string mTlsCert;
	std::string mTlsKey;
	std::string mTlsCertPath;
	std::string mTlsKeyPath;
	std::string mTlsKeyPassword;
	std::shared_ptr<BearerToken> mAccessToken;
	std::shared_ptr<BearerToken> mRefreshToken;
	std::string mAuthServer;
	std::string mTokenEndpoint;
	std::string mClientId;
	time_t mExpires = 0;
	bool mNeedToRenewHa1 = false;

	void setNeedToRenewHa1(bool needToRenewHa1);
	bool getNeedToRenewHa1() const;
};

LINPHONE_END_NAMESPACE
#endif
