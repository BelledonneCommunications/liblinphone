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

#include <bctoolbox/defs.h>

#include "address-parser.h"
#include "belle-sip/sip-uri.h"

#include "address.h"
#include "c-wrapper/c-wrapper.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

std::unordered_map<std::string, std::unique_ptr<SalAddress, Address::SalAddressDeleter>> Address::sAddressCache;

SalAddress *Address::getSalAddressFromCache(const string &address, bool assumeGrUri) {
	auto &ptr = sAddressCache[address];
	if (ptr) return sal_address_clone(ptr.get());

	// lInfo() << "Creating SalAddress for " << address;
	/* To optimize, use the fast uri parser from AddressParser when we can assume that it is a simple URI with
	 * gr param.
	 */
	SalAddress *parsedAddress = nullptr;
	if (assumeGrUri) {
		parsedAddress = AddressParser::get().parseAddress(address);
	}
	if (!parsedAddress) parsedAddress = sal_address_new(L_STRING_TO_C(address));
	if (parsedAddress) {
		removeFromLeakDetector(parsedAddress);
		ptr = (unique_ptr<SalAddress, SalAddressDeleter>(parsedAddress, SalAddressDeleter()));
		return sal_address_clone(parsedAddress);
	}
	return nullptr;
}

// -----------------------------------------------------------------------------

Address::Address(const string &address, bool assumeGrUri, bool logError) {
	if (address.empty()) {
		mImpl = sal_address_new_empty();
	} else if (!(mImpl = getSalAddressFromCache(address, assumeGrUri))) {
		if (logError) {
			lWarning() << "Cannot create Address, bad uri [" << address << "]";
		}
	}
}

Address::Address(const Address &other) : HybridObject(other) {
	SalAddress *salAddress = other.mImpl;
	if (salAddress) mImpl = sal_address_clone(salAddress);
	else mImpl = sal_address_new_empty();
}

Address::Address() {
	mImpl = sal_address_new_empty();
}

Address::Address(Address &&other) : bellesip::HybridObject<LinphoneAddress, Address>(std::move(other)) {
	mImpl = other.mImpl;
	other.mImpl = nullptr;
}

Address::Address(SalAddress *addr, bool acquire) {
	if (acquire) {
		mImpl = addr;
	} else {
		mImpl = sal_address_clone(addr);
	}
}

Address::Address(const SalAddress *addr) {
	mImpl = sal_address_clone(addr);
}

Address::~Address() {
	if (mImpl) sal_address_unref(mImpl);
}

Address *Address::clone() const {
	return new Address(*this);
}

Address &Address::operator=(const Address &other) {
	if (this != &other) {
		if (mImpl) sal_address_unref(mImpl);
		SalAddress *salAddress = other.mImpl;
		mImpl = salAddress ? sal_address_clone(salAddress) : nullptr;
	}

	return *this;
}

bool Address::operator==(const Address &other) const {
	// If either internal addresses is NULL, then the two addresses are not the same
	if (!mImpl || !other.mImpl) return false;
	return (sal_address_equals(mImpl, other.mImpl) == 0);
}

bool Address::operator!=(const Address &other) const {
	return !(*this == other);
}

bool Address::operator<(const Address &other) const {
	return toStringUriOnlyOrdered() < other.toStringUriOnlyOrdered();
}

// -----------------------------------------------------------------------------

Address Address::getUri() const {
	if (mImpl) {
		return Address(sal_address_new_uri_only(mImpl), true);
	}
	return Address();
}

Address Address::getUriWithoutGruu() const {
	auto uri = getUri();
	uri.removeUriParam("gr");
	return uri;
}

void Address::setImpl(const SalAddress *addr) {
	setImpl(sal_address_clone(addr));
}

void Address::setImpl(SalAddress *addr) {
	if (mImpl) sal_address_unref(mImpl);
	mImpl = addr;
}

void Address::clearSipAddressesCache() {
	sAddressCache.clear();
}

bool Address::isValid() const {
	return mImpl && sal_address_get_domain(mImpl);
}

bool Address::setScheme(const std::string &scheme) {
	if (!mImpl) return false;
	if (scheme == "sip") setSecure(false);
	else if (scheme == "sips") setSecure(true);
	else {
		lError() << "Address::setScheme() can't be set to " << scheme;
		return false;
	}
	return true;
}

const char *Address::getSchemeCstr() const {
	return mImpl ? sal_address_get_scheme(mImpl) : nullptr;
}

std::string Address::getScheme() const {
	return L_C_TO_STRING(getSchemeCstr());
}

const char *Address::getDisplayNameCstr() const {
	return mImpl ? sal_address_get_display_name(mImpl) : nullptr;
}

std::string Address::getDisplayName() const {
	return L_C_TO_STRING(getDisplayNameCstr());
}

bool Address::setDisplayName(const string &displayName) {
	if (!mImpl) return false;

	sal_address_set_display_name(mImpl, L_STRING_TO_C(displayName));
	return true;
}

const char *Address::getUsernameCstr() const {
	return mImpl ? sal_address_get_username(mImpl) : nullptr;
}

const std::string Address::getUsername() const {
	return L_C_TO_STRING(getUsernameCstr());
}

bool Address::setUsername(const string &username) {
	if (!mImpl) return false;

	sal_address_set_username(mImpl, L_STRING_TO_C(username));
	return true;
}

const char *Address::getDomainCstr() const {
	return mImpl ? sal_address_get_domain(mImpl) : nullptr;
}

std::string Address::getDomain() const {
	return L_C_TO_STRING(getDomainCstr());
}

bool Address::setDomain(const string &domain) {
	if (!mImpl) return false;

	sal_address_set_domain(mImpl, L_STRING_TO_C(domain));
	return true;
}

int Address::getPort() const {
	return mImpl ? sal_address_get_port(mImpl) : 0;
}

bool Address::setPort(int port) {
	if (!mImpl) return false;

	sal_address_set_port(mImpl, port);
	return true;
}

Transport Address::getTransport() const {
	return mImpl ? static_cast<Transport>(sal_address_get_transport(mImpl)) : Transport::Udp;
}

bool Address::setTransport(Transport transport) {
	if (!mImpl) return false;

	sal_address_set_transport(mImpl, static_cast<SalTransport>(transport));
	return true;
}

bool Address::getSecure() const {
	return mImpl && sal_address_is_secure(mImpl);
}

bool Address::setSecure(bool enabled) {
	if (!mImpl) return false;

	sal_address_set_secure(mImpl, enabled);
	return true;
}

bool Address::isSip() const {
	return mImpl && sal_address_is_sip(mImpl);
}

bool Address::setMethodParam(const std::string &value) {
	if (!mImpl) return false;
	sal_address_set_method_param(mImpl, value.c_str());
	return true;
}

const char *Address::getMethodParamCstr() const {
	return mImpl ? sal_address_get_method_param(mImpl) : nullptr;
}

std::string Address::getMethodParam() const {
	return L_C_TO_STRING(getMethodParamCstr());
}

const char *Address::getPasswordCstr() const {
	return mImpl ? sal_address_get_password(mImpl) : nullptr;
}

std::string Address::getPassword() const {
	return L_C_TO_STRING(getPasswordCstr());
}

bool Address::setPassword(const string &password) {
	if (!mImpl) return false;

	sal_address_set_password(mImpl, L_STRING_TO_C(password));
	return true;
}

bool Address::clean() {
	if (!mImpl) return false;

	sal_address_clean(mImpl);
	return true;
}

char *Address::toStringCstr() const {
	return isValid() ? sal_address_as_string(mImpl) : ms_strdup("");
}

std::string Address::toString() const {
	char *tmp = toStringCstr();
	std::string ret(L_C_TO_STRING(tmp));
	bctbx_free(tmp);
	return ret;
}

string Address::toStringUriOnlyOrdered(bool lowercaseParams) const {
	ostringstream res;
	res << getScheme() << ":";
	if (!getUsername().empty()) {
		char *tmp = belle_sip_uri_to_escaped_username(getUsername().c_str());
		res << tmp << "@";
		ms_free(tmp);
	}

	if (getDomain().find(":") != string::npos) {
		res << "[" << getDomain() << "]";
	} else {
		res << getDomain();
	}

	const auto uriParams = getUriParams();
	for (const auto &param : uriParams) {
		auto name = param.first;
		if (lowercaseParams) {
			std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return tolower(c); });
		}
		res << ";" << name;
		auto value = param.second;
		if (!value.empty()) {
			if (lowercaseParams) {
				std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return tolower(c); });
			}
			res << "=" << value;
		}
	}
	return res.str();
}

char *Address::toStringUriOnlyOrderedCstr(bool lowercaseParams) const {
	auto ordered = toStringUriOnlyOrdered(lowercaseParams);
	return ms_strdup(ordered.c_str());
}

char *Address::asStringUriOnlyCstr() const {
	return isValid() ? sal_address_as_string_uri_only(mImpl) : ms_strdup("");
}

std::string Address::asStringUriOnly() const {
	char *buf = asStringUriOnlyCstr();
	std::string tmp(L_C_TO_STRING(buf));
	bctbx_free(buf);
	return tmp;
}

bool Address::weakEqual(const Address &address) const {
	return !!sal_address_weak_equals(mImpl, address.mImpl);
}

bool Address::weakEqual(const shared_ptr<const Address> address) const {
	return address && weakEqual(*address);
}

bool Address::uriEqual(const Address &other) const {
	return !!sal_address_uri_equals(mImpl, other.mImpl);
}

const char *Address::getHeaderValueCstr(const string &headerName) const {
	return mImpl ? sal_address_get_header(mImpl, headerName.c_str()) : nullptr;
}

std::string Address::getHeaderValue(const std::string &headerName) const {
	return L_C_TO_STRING(getHeaderValueCstr(headerName));
}

bool Address::setHeader(const string &headerName, const string &headerValue) {
	if (!mImpl) return false;

	sal_address_set_header(mImpl, L_STRING_TO_C(headerName), L_STRING_TO_C(headerValue));
	return true;
}

bool Address::hasParam(const string &paramName) const {
	return mImpl && !!sal_address_has_param(mImpl, L_STRING_TO_C(paramName));
}

const char *Address::getParamValueCstr(const string &paramName) const {
	return mImpl ? sal_address_get_param(mImpl, paramName.c_str()) : nullptr;
}

std::string Address::getParamValue(const std::string &paramName) const {
	return L_C_TO_STRING(getParamValueCstr(paramName));
}

bool Address::setParam(const string &paramName, const string &paramValue) {
	if (!mImpl) return false;

	sal_address_set_param(mImpl, L_STRING_TO_C(paramName), L_STRING_TO_C(paramValue));
	return true;
}

bool Address::setParams(const string &params) {
	if (!mImpl) return false;

	sal_address_set_params(mImpl, L_STRING_TO_C(params));
	return true;
}

bool Address::removeParam(const string &uriParamName) {
	if (!mImpl) return false;

	sal_address_remove_param(mImpl, L_STRING_TO_C(uriParamName));
	return true;
}

bool Address::hasUriParam(const string &uriParamName) const {
	return mImpl && !!sal_address_has_uri_param(mImpl, L_STRING_TO_C(uriParamName));
}

const char *Address::getUriParamValueCstr(const string &uriParamName) const {
	return mImpl ? sal_address_get_uri_param(mImpl, uriParamName.c_str()) : nullptr;
}

std::string Address::getUriParamValue(const std::string &uriParamName) const {
	return L_C_TO_STRING(getUriParamValueCstr(uriParamName));
}

bool Address::setUriParam(const string &uriParamName, const string &uriParamValue) {
	if (!mImpl) return false;

	sal_address_set_uri_param(mImpl, L_STRING_TO_C(uriParamName), L_STRING_TO_C(uriParamValue));
	return true;
}

bool Address::setUriParams(const string &uriParams) {
	if (!mImpl) return false;

	sal_address_set_uri_params(mImpl, L_STRING_TO_C(uriParams));
	return true;
}

bool Address::removeUriParam(const string &uriParamName) {
	if (!mImpl) return false;

	sal_address_remove_uri_param(mImpl, L_STRING_TO_C(uriParamName));
	return true;
}

void Address::copyUriParams(const Address &other) {
	const auto otherUriParams = other.getUriParams();
	for (const auto &[name, value] : otherUriParams) {
		setUriParam(name, value);
	}
}

void Address::copyParams(const Address &other) {
	const auto otherParams = other.getParams();
	for (const auto &[name, value] : otherParams) {
		setParam(name, value);
	}
}

void Address::merge(const Address &other) {
	copyParams(other);
	copyUriParams(other);
}

void Address::removeFromLeakDetector(SalAddress *addr) {
	belle_sip_header_address_t *header_addr = BELLE_SIP_HEADER_ADDRESS(addr);
	belle_sip_uri_t *sip_uri = belle_sip_header_address_get_uri(header_addr);
	if (sip_uri) {
		belle_sip_object_remove_from_leak_detector(
		    BELLE_SIP_OBJECT(const_cast<belle_sip_parameters_t *>(belle_sip_uri_get_headers(sip_uri))));
		belle_sip_object_remove_from_leak_detector(BELLE_SIP_OBJECT(sip_uri));
	}
	belle_sip_object_remove_from_leak_detector(BELLE_SIP_OBJECT(header_addr));
}

LINPHONE_END_NAMESPACE
