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

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <list>
#include <sstream>

#include <bctoolbox/charconv.h>
#include <bctoolbox/port.h>

#include "c-wrapper/internal/c-tools.h"
#include "conference/conference-info.h"
#include "conference/participant-info.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"
#include "private.h"
#ifdef HAVE_ADVANCED_IM
#include "xml/resource-lists.h"
#endif

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

bool Utils::iequals(const string &a, const string &b) {
	size_t size = a.size();
	if (b.size() != size) return false;

	for (size_t i = 0; i < size; ++i) {
		if (tolower(a[i]) != tolower(b[i])) return false;
	}

	return true;
}

// -----------------------------------------------------------------------------

#ifndef __ANDROID__
#define TO_STRING_IMPL(TYPE)                                                                                           \
	string Utils::toString(TYPE val) {                                                                                 \
		return to_string(val);                                                                                         \
	}
#else
#define TO_STRING_IMPL(TYPE)                                                                                           \
	string Utils::toString(TYPE val) {                                                                                 \
		ostringstream os;                                                                                              \
		os << val;                                                                                                     \
		return os.str();                                                                                               \
	}
#endif // ifndef __ANDROID__

TO_STRING_IMPL(int)
TO_STRING_IMPL(long)
TO_STRING_IMPL(long long)
TO_STRING_IMPL(unsigned)
TO_STRING_IMPL(unsigned long)
TO_STRING_IMPL(unsigned long long)
TO_STRING_IMPL(float)
TO_STRING_IMPL(double)
TO_STRING_IMPL(long double)

#undef TO_STRING_IMPL

string Utils::toString(const void *val) {
	ostringstream ss;
	ss << val;
	return ss.str();
}

// -----------------------------------------------------------------------------

#define STRING_TO_NUMBER_IMPL(TYPE, SUFFIX)                                                                            \
	TYPE Utils::sto##SUFFIX(const string &str, size_t *idx, int base) {                                                \
		return sto##SUFFIX(str.c_str(), idx, base);                                                                    \
	}                                                                                                                  \
	TYPE Utils::sto##SUFFIX(const char *str, size_t *idx, int base) {                                                  \
		char *p;                                                                                                       \
		TYPE v = strto##SUFFIX(str, &p, base);                                                                         \
		if (idx) *idx = static_cast<size_t>(p - str);                                                                  \
		return v;                                                                                                      \
	}

#define STRING_TO_NUMBER_IMPL_BASE_LESS(TYPE, SUFFIX)                                                                  \
	TYPE Utils::sto##SUFFIX(const string &str, size_t *idx) {                                                          \
		return sto##SUFFIX(str.c_str(), idx);                                                                          \
	}                                                                                                                  \
	TYPE Utils::sto##SUFFIX(const char *str, size_t *idx) {                                                            \
		char *p;                                                                                                       \
		TYPE v = strto##SUFFIX(str, &p);                                                                               \
		if (idx) *idx = static_cast<size_t>(p - str);                                                                  \
		return v;                                                                                                      \
	}

#define strtoi(STR, IDX, BASE) static_cast<int>(strtol(STR, IDX, BASE))
STRING_TO_NUMBER_IMPL(int, i)
#undef strtoi

STRING_TO_NUMBER_IMPL(long long, ll)
STRING_TO_NUMBER_IMPL(unsigned long long, ull)

STRING_TO_NUMBER_IMPL_BASE_LESS(double, d)
STRING_TO_NUMBER_IMPL_BASE_LESS(float, f)

#undef STRING_TO_NUMBER_IMPL
#undef STRING_TO_NUMBER_IMPL_BASE_LESS

const string Utils::btos(bool val) {
	return (val) ? "true" : "false";
}

bool Utils::stob(const string &str) {
	const string lowerStr = stringToLower(str);
	return !lowerStr.empty() && (lowerStr == "true" || lowerStr == "1");
}

// -----------------------------------------------------------------------------

string Utils::replaceAll(const string &source, const string &pattern, const string &replaceBy) {
	string copy = source;
	size_t pos = copy.find(pattern); 
	while (pos != string::npos) { 
		copy.replace(pos, pattern.size(), replaceBy);
		pos = copy.find(pattern, pos + replaceBy.size()); 
	}
	return copy;
}

string Utils::stringToLower(const string &str) {
	string result(str.size(), ' ');
	transform(str.cbegin(), str.cend(), result.begin(), ::tolower);
	return result;
}

std::vector<string> Utils::stringToLower(const std::vector<string> &strs) {
	std::vector<std::string> results;
	for (const auto &str : strs) {
		string result(str.size(), ' ');
		transform(str.cbegin(), str.cend(), result.begin(), ::tolower);
		results.push_back(result);
	}
	return results;
}

// -----------------------------------------------------------------------------

string Utils::unicodeToUtf8(uint32_t ic) {
	string result;

	result.resize(5);
	size_t size = 0;
	if (ic < 0x80) {
		result[0] = static_cast<char>(ic);
		size = 1;
	} else if (ic < 0x800) {
		result[1] = static_cast<char>(0x80 + ((ic & 0x3F)));
		result[0] = static_cast<char>(0xC0 + ((ic >> 6) & 0x1F));
		size = 2;
	} else if (ic < 0x10000) {
		result[2] = static_cast<char>(0x80 + (ic & 0x3F));
		result[1] = static_cast<char>(0x80 + ((ic >> 6) & 0x3F));
		result[0] = static_cast<char>(0xE0 + ((ic >> 12) & 0xF));
		size = 3;
	} else if (ic < 0x110000) {
		result[3] = static_cast<char>(0x80 + (ic & 0x3F));
		result[2] = static_cast<char>(0x80 + ((ic >> 6) & 0x3F));
		result[1] = static_cast<char>(0x80 + ((ic >> 12) & 0x3F));
		result[0] = static_cast<char>(0xF0 + ((ic >> 18) & 0x7));
		size = 4;
	}
	result.resize(size);
	return result;
}

/*
 * TODO: not optmized at all. Good enough for small vectors.
 */
std::string Utils::unicodeToUtf8(const std::vector<uint32_t> &chars) {
	std::ostringstream ss;
	for (auto character : chars) {
		ss << Utils::unicodeToUtf8(character);
	}
	return ss.str();
}

string Utils::trim(const string &str) {
	auto itFront = find_if_not(str.begin(), str.end(), [](unsigned char c) { return isspace(c); });
	auto itBack = find_if_not(str.rbegin(), str.rend(), [](unsigned char c) { return isspace(c); }).base();
	return (itBack <= itFront ? string() : string(itFront, itBack));
}

std::string Utils::normalizeFilename(const std::string &str) {
	std::string result(str);
#ifdef _WIN32
	const std::string illegalCharacters = "\\/:*\"<>|";
#elif defined(__APPLE__)
	const std::string illegalCharacters = ":/";
#else
	const std::string illegalCharacters = "/";
#endif
	// Invisible and illegal characters should not be part of a filename
	result.erase(std::remove_if(result.begin(), result.end(),
	                            [illegalCharacters](const unsigned char &c) {
		                            return c < ' ' || illegalCharacters.find((char)c) != std::string::npos;
	                            }),
	             result.end());
	return result;
}

// -----------------------------------------------------------------------------

tm Utils::getTimeTAsTm(time_t t) {
#ifdef _WIN32
	return *gmtime(&t);
#else
	tm result;
	return *gmtime_r(&t, &result);
#endif
}

time_t Utils::getTmAsTimeT(const tm &t) {
	tm tCopy = t;
	time_t result;
	time_t offset = 0;
#ifdef _WIN32
	result = _mkgmtime(&tCopy);
#elif defined(TARGET_IPHONE_SIMULATOR) || defined(__linux__)
	result = timegm(&tCopy);
#else
	// mktime uses local time => It's necessary to adjust the timezone to get an UTC time.
	result = mktime(&tCopy);
	offset = time_t(timezone);
#endif

	if (result == time_t(-1)) {
		if (tCopy.tm_hour == 0 && tCopy.tm_min == 0 && tCopy.tm_sec == 0 && tCopy.tm_year == 70 && tCopy.tm_mon == 0 &&
		    tCopy.tm_mday == 1)
			return time_t(
			    0); // Not really an error as we try to getTmAsTimeT from initial day (Error comes from timezones)
		lError() << "timegm/mktime failed: " << strerror(errno);
		return time_t(-1);
	}

	return result - offset;
}

std::string Utils::getTimeAsString(const std::string &format, time_t t) {
	tm dateTime = getTimeTAsTm(t);

	std::ostringstream os;
	os << std::put_time(&dateTime, format.c_str());
	return os.str();
}

time_t Utils::getStringToTime(const std::string &format, const std::string &s) {
#ifndef _WIN32
	tm dateTime;

	if (strptime(s.c_str(), format.c_str(), &dateTime)) {
		return getTmAsTimeT(dateTime);
	}
#endif
	return 0;
}

// -----------------------------------------------------------------------------

// TODO: Improve perf!!! Avoid c <--> cpp string conversions.
string Utils::localeToUtf8(const string &str) {
	if (str.empty()) return std::string();
	char *cStr = bctbx_locale_to_utf8(str.c_str());
	string utf8Str = L_C_TO_STRING(cStr);
	bctbx_free(cStr);
	return utf8Str;
}

string Utils::utf8ToLocale(const string &str) {
	if (str.empty()) return std::string();
	char *cStr = bctbx_utf8_to_locale(str.c_str());
	string localeStr = L_C_TO_STRING(cStr);
	bctbx_free(cStr);
	return localeStr;
}

string Utils::convertAnyToUtf8(const string &str, const string &encoding) {
	char *cStr = bctbx_convert_any_to_utf8(str.c_str(), encoding.empty() ? NULL : encoding.c_str());
	string convertedStr = L_C_TO_STRING(cStr);
	bctbx_free(cStr);
	return convertedStr;
}

string Utils::convertUtf8ToAny(const string &str, const string &encoding) {
	char *cStr = bctbx_convert_utf8_to_any(str.c_str(), encoding.empty() ? NULL : encoding.c_str());
	string convertedStr = L_C_TO_STRING(cStr);
	bctbx_free(cStr);
	return convertedStr;
}

string Utils::convert(const string &str, const string &fromEncoding, const string &toEncoding) {
	char *cStr = bctbx_convert_string(str.c_str(), fromEncoding.empty() ? NULL : fromEncoding.c_str(),
	                                  toEncoding.empty() ? NULL : toEncoding.c_str());
	string convertedStr = L_C_TO_STRING(cStr);
	bctbx_free(cStr);
	return convertedStr;
}

string Utils::quoteStringIfNotAlready(const string &str) {
	if (str.empty() || str[0] == '"') return str;
	return string("\"") + str + string("\"");
}

map<string, Utils::Version> Utils::parseCapabilityDescriptor(const string &descriptor) {
	map<string, Utils::Version> result;
	istringstream istr(descriptor);
	string cap;
	string version;
	while (std::getline(istr, cap, ',')) {
		istringstream capversion(cap);
		if (std::getline(capversion, cap, '/') && std::getline(capversion, version, '/')) {
			result[cap] = Utils::Version(version);
		} else result[cap] = Utils::Version(1, 0);
	}
	return result;
}

std::string Utils::getSipFragAddress(const Content &content) {
	if (content.getContentType() != ContentType::SipFrag) {
		lError() << "Content type is not SipFrag hence " << __func__ << " is unable to extract the address";
		return std::string();
	}
	// Extract Contact header from sipfrag content
	auto id = content.getBodyAsUtf8String();
	std::string toErase = "From: ";
	size_t contactPosition = id.find(toErase);
	if (contactPosition != std::string::npos) id.erase(contactPosition, toErase.length());
	auto tmpIdentityAddress = Address::create(id);
	return tmpIdentityAddress->toString();
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
std::string Utils::getResourceLists(const std::list<Address> &addresses) {
#ifdef HAVE_ADVANCED_IM
	Xsd::ResourceLists::ResourceLists rl = Xsd::ResourceLists::ResourceLists();
	Xsd::ResourceLists::ListType l = Xsd::ResourceLists::ListType();
	for (const auto &addr : addresses) {
		Xsd::ResourceLists::EntryType entry = Xsd::ResourceLists::EntryType(addr.asStringUriOnly());
		const auto &displayName = addr.getDisplayName();
		if (!displayName.empty()) {
			entry.setDisplayName(displayName);
		}
		l.getEntry().push_back(entry);
	}
	rl.getList().push_back(l);

	Xsd::XmlSchema::NamespaceInfomap map;
	std::stringstream xmlBody;
	serializeResourceLists(xmlBody, rl, map);
	return xmlBody.str();
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return "";
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

std::shared_ptr<Address> Utils::getSipAddress(const std::string &str, const std::string &scheme) {
	std::shared_ptr<Address> address = nullptr;
	const auto schemeBeginIt = str.find(scheme + ":");
	if (schemeBeginIt == std::string::npos) {
		lError() << "Unable to create CCMP XCON ID because the core is unable to find the scheme [" << scheme
		         << "] of the identity address [" << *address << "]";
	} else {
		auto addressStr = str;
		addressStr.replace(schemeBeginIt, scheme.size(), "sip");
		address = Address::create(addressStr);
	}
	return address;
}

std::string Utils::getXconId(const std::shared_ptr<Address> &address) {
	// CCMP user ID can only be made up by only 40 different characters
	// (https://www.rfc-editor.org/rfc/rfc6501#section-4.6.5) We are therefore taking the identity and transform invalid
	// characters into valid ones in a way that the resulting string is unique for any given mIdentity string
	auto id = address->asStringUriOnly();
	const auto scheme = address->getScheme();
	const auto schemeBeginIt = id.find(scheme + ":");
	if (schemeBeginIt == std::string::npos) {
		lError() << "Unable to create CCMP XCON ID because the core is unable to find the scheme [" << scheme
		         << "] of the identity address [" << *address << "]";
		id.clear();
	} else {
		id.replace(schemeBeginIt, scheme.size(), "xcon-userid");
	}
	return id;
}

// -----------------------------------------------------------------------------

ConferenceInfo::participant_list_t
Utils::parseResourceLists(std::optional<std::reference_wrapper<const Content>> content) {
	if (content) return Utils::parseResourceLists(content.value().get());
	return ConferenceInfo::participant_list_t();
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
ConferenceInfo::participant_list_t Utils::parseResourceLists(const Content &content) {
	ConferenceInfo::participant_list_t resources;
#ifdef HAVE_ADVANCED_IM
	if ((content.getContentType() == ContentType::ResourceLists) &&
	    ((content.getContentDisposition().weakEqual(ContentDisposition::RecipientList)) ||
	     (content.getContentDisposition().weakEqual(ContentDisposition::RecipientListHistory)))) {
		std::istringstream data(content.getBodyAsString());
		std::unique_ptr<Xsd::ResourceLists::ResourceLists> rl(
		    Xsd::ResourceLists::parseResourceLists(data, Xsd::XmlSchema::Flags::dont_validate));
		for (const auto &l : rl->getList()) {
			for (const auto &entry : l.getEntry()) {
				Address address(entry.getUri());
				auto basicAddress = Address::create(address.getUri());
				ParticipantInfo::participant_params_t params;
				for (const auto &[name, value] : address.getUriParams()) {
					params[name] = value;
					basicAddress->removeUriParam(name);
				}
				auto participantInfo = ParticipantInfo::create(basicAddress);
				participantInfo->setParameters(params);
				resources.push_back(participantInfo);
			}
		}
		return resources;
	}
	return resources;
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return resources;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

std::shared_ptr<ConferenceInfo> Utils::createConferenceInfoFromOp(SalCallOp *op, bool remote) {
	std::shared_ptr<ConferenceInfo> info = ConferenceInfo::create();
	if (!op) return info;
	const auto sipfrag = op->getContentInRemote(ContentType::SipFrag);
	const auto resourceList = op->getContentInRemote(ContentType::ResourceLists);

	if (sipfrag) {
		auto organizerStr = Utils::getSipFragAddress(sipfrag.value());
		auto organizer = Address::create(organizerStr);
		auto organizerInfo = ParticipantInfo::create(Address::create(organizer->getUri()));
		for (const auto &[name, value] : organizer->getParams()) {
			organizerInfo->addParameter(name, value);
		}
		info->setOrganizer(organizerInfo);
	}

	if (resourceList) {
		auto invitees = Utils::parseResourceLists(resourceList);
		for (const auto &invitee : invitees) {
			info->addParticipant(invitee);
		}
	}

	const std::shared_ptr<Address> conferenceAddress = Address::create();
	conferenceAddress->setImpl(remote ? op->getRemoteContactAddress() : op->getContactAddress());
	if (conferenceAddress && conferenceAddress->isValid()) {
		info->setUri(conferenceAddress);
	}

	auto &md = remote ? op->getRemoteMediaDescription() : op->getLocalMediaDescription();
	if (md && md->times.size() > 0) {
		const auto [startTime, endTime] = md->times.front();
		if (startTime >= 0) {
			info->setDateTime(startTime);
		} else {
			info->setDateTime(ms_time(NULL));
		}
		if ((startTime >= 0) && (endTime >= 0) && (endTime > startTime)) {
			unsigned int duration = (static_cast<unsigned int>(endTime - startTime)) / 60;
			info->setDuration(duration);
		}
	}

	info->setUtf8Subject(op->getSubject());

	return info;
}

std::string Utils::computeHa1ForAlgorithm(const std::string &userId,
                                          const std::string &password,
                                          const std::string &realm,
                                          const std::string &algorithm) {
	if (algorithm.empty() || algorithm == "MD5") {
		char ha1[33];
		if (sal_auth_compute_ha1(userId.c_str(), realm.c_str(), password.c_str(), ha1) == 0) {
			return ha1;
		}
	} else if (algorithm == "SHA-256") {
		char ha1[65];
		if (sal_auth_compute_ha1_for_algorithm(userId.c_str(), realm.c_str(), password.c_str(), ha1, 65,
		                                       algorithm.c_str()) == 0) {
			return ha1;
		}
	}
	return "";
}

std::ostream &operator<<(std::ostream &ostr, LinphoneGlobalState state) {
	ostr << linphone_global_state_to_string(state);
	return ostr;
}

LINPHONE_END_NAMESPACE
