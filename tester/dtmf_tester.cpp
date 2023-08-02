/* Copyright (c) 2022 Belledonne Communications SARL.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <chrono>
#include <stddef.h>

#include "bctoolbox/tester.h"
#include "call/call.h"
#include "liblinphone_tester++.h"

using namespace std::chrono_literals;
using namespace Linphone::Tester;

namespace {

class SimpleAsserter {
	// Forcing Rep to int to make MSVC happy (with Rep = int64_t the result of .count() could be truncated)
	using Timeout = std::chrono::duration<int, std::milli>;

public:
	SimpleAsserter(CoreManager &core1, CoreManager &core2) : mCore1(core1.getCCore()), mCore2(core2.getCCore()) {
	}

	bool waitFor(const int &counter, int expected, Timeout timeout) {
		return wait_for_until(&mCore1, &mCore2, &counter, expected, timeout.count());
	}

private:
	LinphoneCore &mCore1;
	LinphoneCore &mCore2;
};

struct CallContext {
	std::shared_ptr<LinphonePrivate::Call> marie = nullptr;
	std::shared_ptr<LinphonePrivate::Call> pauline = nullptr;

	CallContext(CoreManager &marie, CoreManager &pauline) {
		BC_ASSERT_TRUE(marie.call(pauline));
		this->marie = marie.getCurrentCall();
		this->pauline = pauline.getCurrentCall();
		BC_ASSERT_PTR_NOT_NULL(this->marie);
		BC_ASSERT_PTR_NOT_NULL(this->pauline);
	}

	~CallContext() {
		marie->terminate();
	}
};

/* Test that DTMF does not get through when clients are configured in incompatible ways */
void linphone_call_send_dtmf__char__not_received() {
	CoreManager marie("marie_rc");
	CoreManager pauline("pauline_rc");
	SimpleAsserter asserter(marie, pauline);
	auto timeout = 200ms;

	/* Sanity check
	 *         | RFC 2833 | SIP INFO
	 * Marie   |   YES    |    NO
	 * Pauline |   YES    |    NO
	 */
	marie.setUseRfc2833ForDtmf(true);
	marie.setUseInfoForDtmf(false);
	pauline.setUseRfc2833ForDtmf(true);
	pauline.setUseInfoForDtmf(false);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('B'), 0, int, "%i");

		// DTMF received
		BC_ASSERT_TRUE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
		pauline.getStats().dtmf_count = 0;
	}

	/*         | RFC 2833 | SIP INFO
	 * Marie   |    NO    |    NO
	 * Pauline |    NO    |    NO
	 */
	marie.setUseRfc2833ForDtmf(false);
	pauline.setUseRfc2833ForDtmf(false);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('D'), 0, int, "%i");

		// DTMF NOT received
		BC_ASSERT_FALSE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
	}

	/*         | RFC 2833 | SIP INFO
	 * Marie   |   YES    |    NO
	 * Pauline |    NO    |    NO
	 */
	marie.setUseRfc2833ForDtmf(true);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('3'), 0, int, "%i");

		// DTMF NOT received
		BC_ASSERT_FALSE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
	}

	/*         | RFC 2833 | SIP INFO
	 * Marie   |    NO    |    NO
	 * Pauline |   YES    |    NO
	 */
	marie.setUseRfc2833ForDtmf(false);
	pauline.setUseRfc2833ForDtmf(true);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('2'), 0, int, "%i");

		// DTMF NOT received
		BC_ASSERT_FALSE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
	}

	/*         | RFC 2833 | SIP INFO
	 * Marie   |    NO    |    NO
	 * Pauline |   YES    |   YES
	 */
	pauline.setUseInfoForDtmf(true);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('9'), 0, int, "%i");

		// DTMF NOT received
		BC_ASSERT_FALSE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
	}

	/*         | RFC 2833 | SIP INFO
	 * Marie   |    NO    |    NO
	 * Pauline |    NO    |   YES
	 */
	pauline.setUseRfc2833ForDtmf(false);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('1'), 0, int, "%i");

		// DTMF NOT received
		BC_ASSERT_FALSE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
	}

	/*         | RFC 2833 | SIP INFO
	 * Marie   |   YES    |    NO
	 * Pauline |    NO    |   YES
	 */
	marie.setUseRfc2833ForDtmf(true);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('4'), 0, int, "%i");

		// DTMF NOT received
		BC_ASSERT_FALSE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
	}
}

void linphone_call_send_dtmf__char__SIP_INFO_does_not_depend_on_callee_config() {
	CoreManager marie("marie_rc");
	CoreManager pauline("pauline_rc");
	SimpleAsserter asserter(marie, pauline);
	auto timeout = 200ms;

	/*         | RFC 2833 | SIP INFO
	 * Marie   |   YES    |   YES
	 * Pauline |    NO    |    NO
	 */
	marie.setUseRfc2833ForDtmf(true);
	marie.setUseInfoForDtmf(true);
	pauline.setUseRfc2833ForDtmf(false);
	pauline.setUseInfoForDtmf(false);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('6'), 0, int, "%i");

		// DTMF received
		BC_ASSERT_TRUE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
		pauline.getStats().dtmf_count = 0;
	}

	/*         | RFC 2833 | SIP INFO
	 * Marie   |    NO    |   YES
	 * Pauline |    NO    |    NO
	 */
	marie.setUseRfc2833ForDtmf(false);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('8'), 0, int, "%i");

		// DTMF received
		BC_ASSERT_TRUE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
		pauline.getStats().dtmf_count = 0;
	}

	/*         | RFC 2833 | SIP INFO
	 * Marie   |    NO    |   YES
	 * Pauline |   YES    |    NO
	 */
	pauline.setUseRfc2833ForDtmf(true);
	{
		CallContext call(marie, pauline);

		BC_ASSERT_EQUAL(call.marie->sendDtmf('4'), 0, int, "%i");

		// DTMF received
		BC_ASSERT_TRUE(asserter.waitFor(pauline.getStats().dtmf_count, 1, timeout));
	}
}

constexpr test_t new_tests[] = {
    TEST_NO_TAG_AUTO_NAMED(linphone_call_send_dtmf__char__not_received),
    TEST_NO_TAG_AUTO_NAMED(linphone_call_send_dtmf__char__SIP_INFO_does_not_depend_on_callee_config),
};

test_t merged[sizeof(dtmf_tests) / sizeof(dtmf_tests[0]) + sizeof(new_tests) / sizeof(new_tests[0])];

constexpr test_suite_t test_suite() {
	int i = 0;
	for (auto &test : dtmf_tests) {
		merged[i] = test;
		i++;
	}
	for (auto &test : new_tests) {
		merged[i] = test;
		i++;
	}

	return {
	    "DTMF",                         // Suite name
	    nullptr,                        // Before suite
	    nullptr,                        // After suite
	    liblinphone_tester_before_each, // Before each test
	    liblinphone_tester_after_each,  // After each test
	    i,                              // test array length
	    merged,                          // test array
	    26                              // Average execution time of the suite
	};
}

} // namespace

test_suite_t dtmf_test_suite = [] {
	auto suite = test_suite();
	bc_tester_add_suite(&suite);
	return suite;
}();
