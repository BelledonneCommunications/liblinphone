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

#ifndef _L_ADDRESS_H_
#define _L_ADDRESS_H_

#include <ostream>
#include <unordered_map>

#include "belle-sip/object++.hh"
#include "c-wrapper/internal/c-sal.h"

#include "enums.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

/**
 * Base class for SIP addresses (not just URIs).
 * It simply wraps a SalAddress structure (actually a belle_sip_header_address_t).
 */
class LINPHONE_PUBLIC Address : public bellesip::HybridObject<LinphoneAddress, Address> {
public:
	explicit Address(const std::string &address, bool assumeGrUri = false, bool logError = true);
	Address();
	Address(Address &&other);
	Address(const Address &other);
	// Instanciate an address by copying from a SalAddress.
	Address(const SalAddress *addr);
	// Instanciate an address by acquiring a SalAddress if acquire is true. If acquire is false, does the same as
	// Address(const Address &other);
	Address(SalAddress *addr, bool acquire);
	virtual ~Address();
	virtual Address *clone() const override;
	virtual std::string toString() const override;

	Address getUri() const;
	Address getUriWithoutGruu() const;

	virtual char *toStringCstr() const; // This one can be overriden.
	char *asStringUriOnlyCstr() const;
	Address &operator=(const Address &other);

	bool operator==(const Address &other) const;
	bool operator!=(const Address &other) const;

	bool operator<(const Address &other) const;

	bool isValid() const;

	std::string getScheme() const;
	const char *getSchemeCstr() const;
	bool setScheme(const std::string &scheme);

	std::string getDisplayName() const;
	const char *getDisplayNameCstr() const;
	bool setDisplayName(const std::string &displayName);

	const std::string getUsername() const;
	const char *getUsernameCstr() const;
	bool setUsername(const std::string &username);

	std::string getDomain() const;
	const char *getDomainCstr() const;
	bool setDomain(const std::string &domain);

	const char *getPasswordCstr() const;
	std::string getPassword() const;
	bool setPassword(const std::string &password);

	int getPort() const;
	bool setPort(int port);

	Transport getTransport() const;
	bool setTransport(Transport transport);

	bool getSecure() const;
	bool setSecure(bool enabled);

	bool isSip() const;

	bool setMethodParam(const std::string &value);
	std::string getMethodParam() const;
	const char *getMethodParamCstr() const;

	std::string getHeaderValue(const std::string &headerName) const;
	const char *getHeaderValueCstr(const std::string &headerName) const;
	bool setHeader(const std::string &headerName, const std::string &headerValue);

	bool hasParam(const std::string &paramName) const;
	std::string getParamValue(const std::string &paramName) const;
	const char *getParamValueCstr(const std::string &paramName) const;
	bool setParam(const std::string &paramName, const std::string &paramValue = "");
	bool setParams(const std::string &params);
	bool removeParam(const std::string &paramName);
	inline std::map<std::string, std::string> getParams() const {
		std::map<std::string, std::string> params;
		if (mImpl) sal_address_get_params(mImpl, params);
		return params;
	}

	bool hasUriParam(const std::string &uriParamName) const;
	std::string getUriParamValue(const std::string &uriParamName) const;
	const char *getUriParamValueCstr(const std::string &uriParamName) const;
	inline std::map<std::string, std::string> getUriParams() const {
		std::map<std::string, std::string> params;
		if (mImpl) sal_address_get_uri_params(mImpl, params);
		return params;
	}
	bool setUriParam(const std::string &uriParamName, const std::string &uriParamValue = "");
	bool setUriParams(const std::string &uriParams);
	bool removeUriParam(const std::string &uriParamName);

	// This function copies the parameters of the argument address other to this
	void merge(const Address &other);
	void copyParams(const Address &other);
	void copyUriParams(const Address &other);

	inline std::string asString() const {
		return toString();
	}
	char *toStringUriOnlyOrderedCstr(bool lowercaseParams = false) const;
	std::string toStringUriOnlyOrdered(bool lowercaseParams = false) const;

	std::string asStringUriOnly() const;

	bool clean();
	bool weakEqual(const Address &other) const;
	bool weakEqual(const std::shared_ptr<const Address> address) const;
	bool uriEqual(const Address &other) const;

	inline const SalAddress *getImpl() const {
		return mImpl;
	}
	void setImpl(SalAddress *value);
	void setImpl(const SalAddress *value);
	static void clearSipAddressesCache();
	struct WeakLess {
		bool operator()(const Address &address1, const Address &address2) const {
			return address1 < address2;
		}
	};
	struct WeakEqual {
		bool operator()(const Address &address1, const Address &address2) const {
			return address1.weakEqual(address2);
		}
	};

protected:
	static SalAddress *getSalAddressFromCache(const std::string &address, bool assumeGrUri);

private:
	SalAddress *mImpl = nullptr;
	struct SalAddressDeleter {
		void operator()(SalAddress *addr) {
			sal_address_unref(addr);
		}
	};
	static void removeFromLeakDetector(SalAddress *addr);

	static std::unordered_map<std::string, std::unique_ptr<SalAddress, SalAddressDeleter>> sAddressCache;
};

inline std::ostream &operator<<(std::ostream &os, const Address &address) {
	os << "Address(" << address.asString() << ")";
	return os;
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ADDRESS_H_
