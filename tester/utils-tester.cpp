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

#include "bctoolbox/utils.hh"

#include "address/address.h"
#include "conference/conference-id.h"
#include "liblinphone_tester.h"
#include "linphone/utils/utils.h"
#include "tester_utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;
using namespace LinphonePrivate::Utils;

static void split() {
	string emptyString;
	vector<string> result = bctoolbox::Utils::split(emptyString, ",");
	BC_ASSERT_EQUAL((int)result.size(), 1, int, "%d");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), "");
	string contentDisposition("positive-delivery, negative-delivery, display");
	result = bctoolbox::Utils::split(contentDisposition, ", ");
	BC_ASSERT_EQUAL((int)result.size(), 3, int, "%d");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), "positive-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(1).c_str(), "negative-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(2).c_str(), "display");
	result = bctoolbox::Utils::split(contentDisposition, ",");
	BC_ASSERT_EQUAL((int)result.size(), 3, int, "%d");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), "positive-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(1).c_str(), " negative-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(2).c_str(), " display");
	result = bctoolbox::Utils::split(contentDisposition, ',');
	BC_ASSERT_EQUAL((int)result.size(), 3, int, "%d");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), "positive-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(1).c_str(), " negative-delivery");
	BC_ASSERT_STRING_EQUAL(result.at(2).c_str(), " display");
	result = bctoolbox::Utils::split(contentDisposition, "|");
	BC_ASSERT_EQUAL((int)result.size(), 1, int, "%d");
	BC_ASSERT_STRING_EQUAL(result.at(0).c_str(), contentDisposition.c_str());
}

static void trim() {
	string emptyString;
	string result = Utils::trim(emptyString);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "");
	string stringWithLeadingSpace(" hello");
	result = Utils::trim(stringWithLeadingSpace);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "hello");
	string stringWithTailingSpace("hello ");
	result = Utils::trim(stringWithTailingSpace);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "hello");
	string stringWithSpaces("   hello  ");
	result = Utils::trim(stringWithSpaces);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "hello");
	string stringContainingSpaces(" hello world!    ");
	result = Utils::trim(stringContainingSpaces);
	BC_ASSERT_STRING_EQUAL(result.c_str(), "hello world!");
}

static void version_comparisons(void) {
	BC_ASSERT_TRUE(Version(1, 0) == Version(1, 0));
	BC_ASSERT_TRUE(Version(2, 0) > Version(1, 0));
	BC_ASSERT_TRUE(Version(1, 1) > Version(1, 0));
	BC_ASSERT_TRUE(Version(1, 1) >= Version(1, 1));
	BC_ASSERT_TRUE(Version("1.2") == Version(1, 2));
	BC_ASSERT_TRUE(Version("1.2.4") == Version(1, 2, 4));
	BC_ASSERT_TRUE(Version(1, 1) < Version(1, 4));
	BC_ASSERT_TRUE(Version("1.0.0-alpha") < Version("1.0.0-alpha.1"));
	BC_ASSERT_TRUE(Version("1.0.0-alpha.12") < Version("1.0.0-alpha.13"));
	BC_ASSERT_TRUE(Version("1.0.0-alpha.17+aaaaaaa") < Version("1.0.0-alpha.17+baaaaaa"));
	BC_ASSERT_TRUE(Version("1.0.0-alpha.34") < Version("1.0.0-beta"));
	BC_ASSERT_TRUE(Version("1.0.0-beta.26") < Version("1.0.0"));
	BC_ASSERT_TRUE(Version("1.0.0") < Version("1.0.1-pre.1"));
}

static void timestamp_pruning(void) {
	// UTC
	// No fractional part
	std::string utcTimestamp = "2024-10-23T14:02:13,000000Z";
	long long expectedUtcTimestamp = 1729692133;
	auto convertedUtcTimestamp = Utils::iso8601ToTime(utcTimestamp);
	BC_ASSERT_EQUAL((long long)convertedUtcTimestamp, expectedUtcTimestamp, long long, "%lld");
	auto backConvertedUtcTimestamp = Utils::timeToIso8601(convertedUtcTimestamp);
	std::string utcTimestampBackConvertion =
#ifdef __APPLE__
	    "2024-10-23T14:02:13Z";
#else
	    "2024-10-23T14:02:13+0000";
#endif // __APPLE__
	BC_ASSERT_STRING_EQUAL(backConvertedUtcTimestamp.c_str(), utcTimestampBackConvertion.c_str());

	// With fractional part
	std::string utcCommaTimestamp = "2024-10-23T14:02:13,000000Z";
	auto convertedUtcCommaTimestamp = Utils::iso8601ToTime(utcCommaTimestamp);
	BC_ASSERT_EQUAL((long long)convertedUtcCommaTimestamp, expectedUtcTimestamp, long long, "%lld");
	auto backConvertedFractionalUtcTimestamp = Utils::timeToIso8601(convertedUtcCommaTimestamp);
	BC_ASSERT_STRING_EQUAL(backConvertedFractionalUtcTimestamp.c_str(), utcTimestampBackConvertion.c_str());

	std::string utcDotTimestamp = "2024-10-23T14:02:13.000000Z";
	auto convertedUtcDotTimestamp = Utils::iso8601ToTime(utcDotTimestamp);
	BC_ASSERT_EQUAL((long long)convertedUtcDotTimestamp, expectedUtcTimestamp, long long, "%lld");

	// Positive offset
	std::string offsetPlusCommaTimestamp = "2024-10-23T14:02:13,000000+0100";
	long long expectedOffsetPlusTimestamp = 1729688533;
	auto convertedOffsetPlusCommaTimestamp = Utils::iso8601ToTime(offsetPlusCommaTimestamp);
	BC_ASSERT_EQUAL((long long)convertedOffsetPlusCommaTimestamp, expectedOffsetPlusTimestamp, long long, "%lld");
	auto backConvertedOffsetPlusTimestamp = Utils::timeToIso8601(convertedOffsetPlusCommaTimestamp);
	std::string offsetPlusTimestampBackConvertion =
#ifdef __APPLE__
	    "2024-10-23T13:02:13Z";
#else
	    "2024-10-23T13:02:13+0000";
#endif // __APPLE__
	BC_ASSERT_STRING_EQUAL(backConvertedOffsetPlusTimestamp.c_str(), offsetPlusTimestampBackConvertion.c_str());

	std::string offsetPlusDotTimestamp = "2024-10-23T14:02:13.000000+01";
	auto convertedOffsetPlusDotTimestamp = Utils::iso8601ToTime(offsetPlusDotTimestamp);
	BC_ASSERT_EQUAL((long long)convertedOffsetPlusDotTimestamp, expectedOffsetPlusTimestamp, long long, "%lld");

	// Negative offset
	std::string offsetMinusCommaTimestamp = "2024-10-23T14:02:13,000000-0100";
	long long expectedOffsetMinusTimestamp = 1729695733;
	auto convertedOffsetMinusCommaTimestamp = Utils::iso8601ToTime(offsetMinusCommaTimestamp);
	BC_ASSERT_EQUAL((long long)convertedOffsetMinusCommaTimestamp, expectedOffsetMinusTimestamp, long long, "%lld");
	auto backConvertedOffsetMinusTimestamp = Utils::timeToIso8601(convertedOffsetMinusCommaTimestamp);
	std::string offsetMinusTimestampBackConvertion =
#ifdef __APPLE__
	    "2024-10-23T15:02:13Z";
#else
	    "2024-10-23T15:02:13+0000";
#endif // __APPLE__
	BC_ASSERT_STRING_EQUAL(backConvertedOffsetMinusTimestamp.c_str(), offsetMinusTimestampBackConvertion.c_str());

	std::string offsetMinusDotTimestamp = "2024-10-23T14:02:13.000000-01";
	auto convertedOffsetMinusDotTimestamp = Utils::iso8601ToTime(offsetMinusDotTimestamp);
	BC_ASSERT_EQUAL((long long)convertedOffsetMinusDotTimestamp, expectedOffsetMinusTimestamp, long long, "%lld");
}

static void address_comparisons() {
	Address a1("sip:toto@sip.example.org;a=dada;b=dede;c=didi;d=dodo");
	BC_ASSERT_TRUE(a1.isValid());
	Address a2("sip:toto@sip.example.org;b=dede;a=dada;d=dodo;c=didi");
	BC_ASSERT_TRUE(a2.isValid());
	Address a3("sip:toto@sip.example.org;d=dodo;c=didi;b=dede");
	BC_ASSERT_TRUE(a3.isValid());
	Address a4("sip:hihi@sip.example.org;d=dodo;c=didi;b=dede");
	BC_ASSERT_TRUE(a4.isValid());
	BC_ASSERT_TRUE(a3 == a2);
	BC_ASSERT_TRUE(a1 == a2);
	BC_ASSERT_FALSE(a1 == a4);
	BC_ASSERT_TRUE(a1.toStringUriOnlyOrdered() == a2.toStringUriOnlyOrdered());
	BC_ASSERT_FALSE(a1.toStringUriOnlyOrdered() == a3.toStringUriOnlyOrdered());
	BC_ASSERT_FALSE(a3.toStringUriOnlyOrdered() == a4.toStringUriOnlyOrdered());
	BC_ASSERT_TRUE(a3.weakEqual(a2));
	BC_ASSERT_TRUE(a3.weakEqual(a1));
	BC_ASSERT_FALSE(a3.weakEqual(a4));
}

static void address_serialization() {
	Address a("sip:toto@sip.example.org;b=dede;a=dada;d=dodo;c=didi");
	Address b("<sip:toto@sip.example.org;b=dede;a=dada;d=dodo;c=didi>");
	Address c("<sip:toto@sip.example.org;b=dede;a=dada;d=dodo;c=didi>;f=dudu;e=caca");

	string result = a.toStringUriOnlyOrdered();
	BC_ASSERT_STRING_EQUAL(result.c_str(), "sip:toto@sip.example.org;a=dada;b=dede;c=didi;d=dodo");
	result = b.toStringUriOnlyOrdered();
	BC_ASSERT_STRING_EQUAL(result.c_str(), "sip:toto@sip.example.org;a=dada;b=dede;c=didi;d=dodo");
	result = c.toStringUriOnlyOrdered();
	BC_ASSERT_STRING_EQUAL(result.c_str(), "sip:toto@sip.example.org;a=dada;b=dede;c=didi;d=dodo");
}

static void conferenceId_comparisons() {
	std::shared_ptr<Address> a1 = Address::create("sip:toto@sip.example.org;a=dada;b=dede;c=didi;d=dodo");
	std::shared_ptr<Address> a2 = Address::create("sip:toto@sip.example.org;b=dede;a=dada;d=dodo;c=didi");
	std::shared_ptr<Address> a3 = Address::create("sip:toto@sip.example.org;d=dodo;c=didi;b=dede");
	std::shared_ptr<Address> a4 = Address::create("sip:hihi@sip.example.org;d=dodo;c=didi;b=dede");

	std::shared_ptr<Address> b1 = Address::create("sip:popo@sip.example.org;a=dada;b=dede;c=didi;d=dodo");
	std::shared_ptr<Address> b2 = Address::create("sip:popo@sip.example.org;b=dede;a=dada;d=dodo;c=didi");
	std::shared_ptr<Address> b3 = Address::create("sip:popo@sip.example.org;d=dodo;c=didi;b=dede");
	std::shared_ptr<Address> b4 = Address::create("sip:popo@sip.example.org;d=tutu;c=didi;b=dede");

	ConferenceIdParams conferenceIdParams;
	conferenceIdParams.enableExtractUri(false);
	conferenceIdParams.setKeepGruu(false);
	ConferenceId c1(a1, b1, conferenceIdParams);
	ConferenceId c2(a2, b2, conferenceIdParams);
	ConferenceId c3(a1, b3, conferenceIdParams);
	ConferenceId c4(a3, b2, conferenceIdParams);
	ConferenceId c5(a3, b3, conferenceIdParams);
	ConferenceId c6(a4, b3, conferenceIdParams);
	ConferenceId c7(a3, b4, conferenceIdParams);
	BC_ASSERT_TRUE(c1 == c2);
	BC_ASSERT_TRUE(c3 == c2);
	BC_ASSERT_TRUE(c1 == c4);
	BC_ASSERT_TRUE(c1 == c5);
	BC_ASSERT_FALSE(c6 == c5);
	BC_ASSERT_FALSE(c7 == c5);
}

static void parse_capabilities(void) {
	auto caps = Utils::parseCapabilityDescriptor("groupchat,lime,ephemeral");
	BC_ASSERT_TRUE(caps.find("groupchat") != caps.end());
	BC_ASSERT_TRUE(caps.find("lime") != caps.end());
	BC_ASSERT_TRUE(caps.find("ephemeral") != caps.end());

	caps = Utils::parseCapabilityDescriptor("groupchat/1.3,lime/1.1,ephemeral");
	BC_ASSERT_TRUE(caps["lime"] == Version(1, 1));
	BC_ASSERT_TRUE(caps["groupchat"] == Version(1, 3));
	BC_ASSERT_TRUE(caps["ephemeral"] == Version(1, 0));
}

// clang-format off
static test_t utils_tests[] = {
    TEST_NO_TAG("split", split),
    TEST_NO_TAG("trim", trim),
    TEST_NO_TAG("Timestamp pruning", timestamp_pruning),
    TEST_NO_TAG("Version comparisons", version_comparisons),
    TEST_NO_TAG("Address comparisons", address_comparisons),
    TEST_NO_TAG("Address serialization", address_serialization),
    TEST_NO_TAG("Conference ID comparisons", conferenceId_comparisons),
    TEST_NO_TAG("Parse capabilities", parse_capabilities)
};
// clang-format on

test_suite_t utils_test_suite = {"Utils",
                                 NULL,
                                 NULL,
                                 liblinphone_tester_before_each,
                                 liblinphone_tester_after_each,
                                 sizeof(utils_tests) / sizeof(utils_tests[0]),
                                 utils_tests,
                                 0};
