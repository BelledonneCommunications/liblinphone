/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#include "address.h"
#include "address/identity-address.h"
#include "c-wrapper/c-wrapper.h"
#include "containers/lru-cache.h"
#include "logger/logger.h"

// TODO: delete after Addres is not derived anymore from ClonableObject
#include "object/clonable-object-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

namespace {
	class SalAddressWrap {
	public:
		explicit SalAddressWrap (SalAddress *salAddress = nullptr) : mSalAddress(salAddress) {}

		SalAddressWrap (const SalAddressWrap &other) : mSalAddress(other.mSalAddress) {
			if (mSalAddress)
				sal_address_ref(mSalAddress);
		}

		SalAddressWrap (SalAddressWrap &&other) : mSalAddress(other.mSalAddress) {
			other.mSalAddress = nullptr;
		}

		~SalAddressWrap () {
			if (mSalAddress)
				sal_address_unref(mSalAddress);
		}

		const SalAddress *get () {
			return mSalAddress;
		}

	private:
		SalAddress *mSalAddress;
	};
	LruCache<string, SalAddressWrap> addressesCache;
}

static SalAddress *getSalAddressFromCache (const string &uri) {
	SalAddressWrap *wrap = addressesCache[uri];
	if (wrap)
		return sal_address_clone(wrap->get());

	SalAddress *address = sal_address_new(L_STRING_TO_C(uri));
	if (address) {
		addressesCache.insert(uri, SalAddressWrap(address));
		return sal_address_clone(address);
	}

	return nullptr;
}

// -----------------------------------------------------------------------------

Address::Address (const string &address) : ClonableObject(*new ClonableObjectPrivate) {
	if (!(internalAddress = getSalAddressFromCache(address))) {
		lWarning() << "Cannot create Address, bad uri [" << address << "]";
	}
}

Address::Address (const IdentityAddress &identityAddress) : ClonableObject(*new ClonableObjectPrivate) {
	const string &username = identityAddress.getUsername();
	if (username.empty())
		return;
	const string &domain = identityAddress.getDomain();
	if (domain.empty())
		return;

	string uri = identityAddress.asString();
	internalAddress = getSalAddressFromCache(uri);
}

Address::Address (const ConferenceAddress &conferenceAddress) : Address(IdentityAddress(conferenceAddress)) {
	setUriParam ("conf-id", conferenceAddress.getConfId());
}

Address::Address (const Address &other) : ClonableObject(*new ClonableObjectPrivate) {
	SalAddress *salAddress = other.internalAddress;
	if (salAddress)
		internalAddress = sal_address_clone(salAddress);
}

Address::~Address () {
	if (internalAddress)
		sal_address_unref(internalAddress);
}

Address &Address::operator= (const Address &other) {
	if (this != &other) {
		if (internalAddress)
			sal_address_unref(internalAddress);
		SalAddress *salAddress = other.internalAddress;
		internalAddress = salAddress ? sal_address_clone(salAddress) : nullptr;
	}

	return *this;
}

bool Address::operator== (const Address &other) const {
	return asString() == other.asString();
}

bool Address::operator!= (const Address &other) const {
	return !(*this == other);
}

bool Address::operator< (const Address &other) const {
	return asString() < other.asString();
}

// -----------------------------------------------------------------------------

void Address::setInternalAddress (const SalAddress *addr) {
	if (internalAddress)
		sal_address_unref(internalAddress);
	internalAddress = sal_address_clone(addr);
}

void Address::clearSipAddressesCache () {
	addressesCache.clear();
}

bool Address::isValid () const {
	return !!internalAddress;
}

const string &Address::getScheme () const {
	if (!internalAddress)
		return Utils::getEmptyConstRefObject<string>();

	string scheme(L_C_TO_STRING(sal_address_get_scheme(internalAddress)));
	if (scheme != cache.scheme)
		cache.scheme = scheme;
	return cache.scheme;
}

const string &Address::getDisplayName () const {
	if (!internalAddress)
		return Utils::getEmptyConstRefObject<string>();

	string displayName(L_C_TO_STRING(sal_address_get_display_name(internalAddress)));
	if (displayName != cache.displayName)
		cache.displayName = displayName;
	return cache.displayName;
}

bool Address::setDisplayName (const string &displayName) {
	if (!internalAddress)
		return false;

	sal_address_set_display_name(internalAddress, L_STRING_TO_C(displayName));
	return true;
}

const string &Address::getUsername () const {
	if (!internalAddress)
		return Utils::getEmptyConstRefObject<string>();

	string username(L_C_TO_STRING(sal_address_get_username(internalAddress)));
	if (username != cache.username)
		cache.username = username;
	return cache.username;
}

bool Address::setUsername (const string &username) {
	if (!internalAddress)
		return false;

	sal_address_set_username(internalAddress, L_STRING_TO_C(username));
	return true;
}

const string &Address::getDomain () const {
	if (!internalAddress)
		return Utils::getEmptyConstRefObject<string>();

	string domain(L_C_TO_STRING(sal_address_get_domain(internalAddress)));
	if (domain != cache.domain)
		cache.domain = domain;
	return cache.domain;
}

bool Address::setDomain (const string &domain) {
	if (!internalAddress)
		return false;

	sal_address_set_domain(internalAddress, L_STRING_TO_C(domain));
	return true;
}

int Address::getPort () const {
	return internalAddress ? sal_address_get_port(internalAddress) : 0;
}

bool Address::setPort (int port) {
	if (!internalAddress)
		return false;

	sal_address_set_port(internalAddress, port);
	return true;
}

Transport Address::getTransport () const {
	return internalAddress ? static_cast<Transport>(sal_address_get_transport(internalAddress)) : Transport::Udp;
}

bool Address::setTransport (Transport transport) {
	if (!internalAddress)
		return false;

	sal_address_set_transport(internalAddress, static_cast<SalTransport>(transport));
	return true;
}

bool Address::getSecure () const {
	return internalAddress && sal_address_is_secure(internalAddress);
}

bool Address::setSecure (bool enabled) {
	if (!internalAddress)
		return false;

	sal_address_set_secure(internalAddress, enabled);
	return true;
}

bool Address::isSip () const {
	return internalAddress && sal_address_is_sip(internalAddress);
}

const string &Address::getMethodParam () const {
	if (!internalAddress)
		return Utils::getEmptyConstRefObject<string>();

	string methodParam(L_C_TO_STRING(sal_address_get_method_param(internalAddress)));
	if (methodParam != cache.methodParam)
		cache.methodParam = methodParam;
	return cache.methodParam;
}

bool Address::setMethodParam (const string &methodParam) {
	if (!internalAddress)
		return false;

	sal_address_set_method_param(internalAddress, L_STRING_TO_C(methodParam));
	return true;
}

const string &Address::getPassword () const {
	if (!internalAddress)
		return Utils::getEmptyConstRefObject<string>();

	string password(L_C_TO_STRING(sal_address_get_password(internalAddress)));
	if (password != cache.password)
		cache.password = password;
	return cache.password;
}

bool Address::setPassword (const string &password) {
	if (!internalAddress)
		return false;

	sal_address_set_password(internalAddress, L_STRING_TO_C(password));
	return true;
}

bool Address::clean () {
	if (!internalAddress)
		return false;

	sal_address_clean(internalAddress);
	return true;
}

string Address::asString () const {
	if (!internalAddress)
		return "";

	char *buf = sal_address_as_string(internalAddress);
	string out = buf;
	ms_free(buf);
	return out;
}

string Address::asStringUriOnly () const {
	if (!internalAddress)
		return "";

	char *buf = sal_address_as_string_uri_only(internalAddress);
	string out = buf;
	ms_free(buf);
	return out;
}

bool Address::weakEqual (const Address &address) const {
	return getUsername() == address.getUsername() &&
				 getDomain() == address.getDomain() &&
				 getPort() == address.getPort();
}

const string &Address::getHeaderValue (const string &headerName) const {
	if (internalAddress) {
		const char *value = sal_address_get_header(internalAddress, L_STRING_TO_C(headerName));
		if (value) {
			cache.headers[headerName] = value;
			return cache.headers[headerName];
		}
	}

	return Utils::getEmptyConstRefObject<string>();
}

bool Address::setHeader (const string &headerName, const string &headerValue) {
	if (!internalAddress)
		return false;

	sal_address_set_header(internalAddress, L_STRING_TO_C(headerName), L_STRING_TO_C(headerValue));
	return true;
}

bool Address::hasParam (const string &paramName) const {
	return internalAddress && !!sal_address_has_param(internalAddress, L_STRING_TO_C(paramName));
}

const string &Address::getParamValue (const string &paramName) const {
	if (internalAddress) {
		const char *value = sal_address_get_param(internalAddress, L_STRING_TO_C(paramName));
		if (value) {
			cache.params[paramName] = value;
			return cache.params[paramName];
		}
	}

	return Utils::getEmptyConstRefObject<string>();
}

bool Address::setParam (const string &paramName, const string &paramValue) {
	if (!internalAddress)
		return false;

	sal_address_set_param(internalAddress, L_STRING_TO_C(paramName), L_STRING_TO_C(paramValue));
	return true;
}

bool Address::setParams (const string &params) {
	if (!internalAddress)
		return false;

	sal_address_set_params(internalAddress, L_STRING_TO_C(params));
	return true;
}

bool Address::hasUriParam (const string &uriParamName) const {
	return internalAddress && !!sal_address_has_uri_param(internalAddress, L_STRING_TO_C(uriParamName));
}

const string &Address::getUriParamValue (const string &uriParamName) const {
	if (internalAddress) {
		const char *value = sal_address_get_uri_param(internalAddress, L_STRING_TO_C(uriParamName));
		if (value) {
			cache.uriParams[uriParamName] = value;
			return cache.uriParams[uriParamName];
		}
	}

	return Utils::getEmptyConstRefObject<string>();
}

bool Address::setUriParam (const string &uriParamName, const string &uriParamValue) {
	if (!internalAddress)
		return false;

	sal_address_set_uri_param(internalAddress, L_STRING_TO_C(uriParamName), L_STRING_TO_C(uriParamValue));
	return true;
}

bool Address::setUriParams (const string &uriParams) {
	if (!internalAddress)
		return false;

	sal_address_set_uri_params(internalAddress, L_STRING_TO_C(uriParams));
	return true;
}

bool Address::removeUriParam (const string &uriParamName) {
	if (!internalAddress)
		return false;

	sal_address_remove_uri_param(internalAddress, L_STRING_TO_C(uriParamName));
	return true;
}

LINPHONE_END_NAMESPACE
