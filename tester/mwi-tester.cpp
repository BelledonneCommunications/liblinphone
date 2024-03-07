/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "bctoolbox/defs.h"

#include "account/mwi/message-waiting-indication-summary.h"
#include "account/mwi/message-waiting-indication.h"
#include "account/mwi/parser/mwi-parser.h"
#include "address/address.h"
#include "belr/grammarbuilder.h"
#include "core/core.h"
// TODO: Remove me later.
#include "private.h"

#include "liblinphone_tester.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

static void parse_minimal_mwi() {
	const string str = "Messages-Waiting: no\r\n";

	shared_ptr<const Mwi::MessageWaitingIndication> mwi = Mwi::Parser::getInstance()->parseMessageSummary(str);
	if (!BC_ASSERT_PTR_NOT_NULL(mwi)) return;

	BC_ASSERT_FALSE(mwi->hasMessageWaiting());
	BC_ASSERT_EQUAL(mwi->getSummaries().size(), 0, size_t, "%zu");
	auto account_address = mwi->getAccountAddress();
	BC_ASSERT_PTR_NULL(account_address);
}

static void parse_normal_mwi() {
	const string str = "Messages-Waiting: yes\r\n"
	                   "Message-Account: sip:alice@vmail.example.com\r\n"
	                   "Voice-Message: 4/8 (1/2)\r\n";

	shared_ptr<const Mwi::MessageWaitingIndication> mwi = Mwi::Parser::getInstance()->parseMessageSummary(str);
	if (!BC_ASSERT_PTR_NOT_NULL(mwi)) return;

	BC_ASSERT_TRUE(mwi->hasMessageWaiting());
	BC_ASSERT_EQUAL(mwi->getSummaries().size(), 1, size_t, "%zu");
	auto summary = mwi->getSummary(LinphoneMessageWaitingIndicationVoice);
	if (!BC_ASSERT_PTR_NOT_NULL(summary)) return;
	BC_ASSERT_EQUAL(summary->getNbNew(), 4, size_t, "%zu");
	BC_ASSERT_EQUAL(summary->getNbOld(), 8, size_t, "%zu");
	BC_ASSERT_EQUAL(summary->getNbNewUrgent(), 1, size_t, "%zu");
	BC_ASSERT_EQUAL(summary->getNbOldUrgent(), 2, size_t, "%zu");
	auto account_address = mwi->getAccountAddress();
	if (!BC_ASSERT_PTR_NOT_NULL(account_address)) return;
	Address addr = Address("sip:alice@vmail.example.com");
	BC_ASSERT_TRUE(*account_address.get() == addr);
}

static void parse_mwi_without_account_address() {
	const string str = "Messages-Waiting: yes\r\n"
	                   "Text-message: 2/3 (0/2)\r\n";

	shared_ptr<const Mwi::MessageWaitingIndication> mwi = Mwi::Parser::getInstance()->parseMessageSummary(str);
	if (!BC_ASSERT_PTR_NOT_NULL(mwi)) return;

	BC_ASSERT_TRUE(mwi->hasMessageWaiting());
	BC_ASSERT_EQUAL(mwi->getSummaries().size(), 1, size_t, "%zu");
	auto summary = mwi->getSummary(LinphoneMessageWaitingIndicationText);
	if (!BC_ASSERT_PTR_NOT_NULL(summary)) return;
	BC_ASSERT_EQUAL(summary->getNbNew(), 2, size_t, "%zu");
	BC_ASSERT_EQUAL(summary->getNbOld(), 3, size_t, "%zu");
	BC_ASSERT_EQUAL(summary->getNbNewUrgent(), 0, size_t, "%zu");
	BC_ASSERT_EQUAL(summary->getNbOldUrgent(), 2, size_t, "%zu");
	auto account_address = mwi->getAccountAddress();
	BC_ASSERT_PTR_NULL(account_address);
}

static void mwi_changed_on_account(LinphoneAccount *account, const LinphoneMessageWaitingIndication *mwi) {
	LinphoneCore *lc = linphone_account_get_core(account);
	lInfo() << "MWI changed for user id [" << linphone_account_params_get_identity(linphone_account_get_params(account))
	        << "] at account [" << linphone_account_params_get_server_addr(linphone_account_get_params(account)) << "]";
	stats *counters = get_stats(lc);
	if (linphone_message_waiting_indication_has_message_waiting(mwi)) {
		counters->number_of_mwi++;
	}
	const LinphoneMessageWaitingIndicationSummary *summary =
	    linphone_message_waiting_indication_get_summary(mwi, LinphoneMessageWaitingIndicationVoice);
	if (summary) {
		counters->number_of_new_LinphoneMessageWaitingIndicationVoice +=
		    linphone_message_waiting_indication_summary_get_nb_new(summary);
		counters->number_of_old_LinphoneMessageWaitingIndicationVoice +=
		    linphone_message_waiting_indication_summary_get_nb_old(summary);
		counters->number_of_new_urgent_LinphoneMessageWaitingIndicationVoice +=
		    linphone_message_waiting_indication_summary_get_nb_new_urgent(summary);
		counters->number_of_old_urgent_LinphoneMessageWaitingIndicationVoice +=
		    linphone_message_waiting_indication_summary_get_nb_old_urgent(summary);
	}
}

static void mwi_notified_on_account(void) {
	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");
	LinphoneAccount *laure_account = linphone_core_get_default_account(laure->lc);

	BC_ASSERT_TRUE(wait_for_until(laure->lc, NULL, &laure->stat.number_of_LinphoneRegistrationOk, 1, 5000));

	LinphoneAccountCbs *cbs = linphone_factory_create_account_cbs(linphone_factory_get());
	linphone_account_cbs_set_message_waiting_indication_changed(cbs, mwi_changed_on_account);
	linphone_account_add_callbacks(laure_account, cbs);
	linphone_account_cbs_unref(cbs);

	LinphoneAddress *laure_addr = linphone_account_get_contact_address(laure_account);
	linphone_address_clean(laure_addr);
	char *laure_uri = linphone_address_as_string_uri_only(laure_addr);

	auto mwi_notify_stream = std::ostringstream();
	mwi_notify_stream
	    << "NOTIFY " << laure_uri << " SIP/2.0\r\n"
	    << "Via: SIP/2.0/UDP [2a01:cb19:83ad:ed00:6066:af3f:16b3:d980]:37486;branch=z9hG4bK.N5OIiMSAe;rport\r\n"
	    << "To: <" << laure_uri << ">\r\n"
	    << "From: <" << laure_uri << ">;tag=4442\r\n"
	    << "Date: Mon, 10 Jul 2000 15:55:07 GMT\r\n"
	    << "Call-Id: 1349882@sip.example.com\r\n"
	    << "CSeq: 1 NOTIFY\r\n"
	    << "Contact: <" << laure_uri << ">\r\n"
	    << "Event: message-summary\r\n"
	    << "Subscription-State: active\r\n"
	    << "Content-Type: application/simple-message-summary\r\n"
	    << "Content-Length: " << (68 + strlen(laure_uri)) << "\r\n"
	    << "\r\n"
	    << "Messages-Waiting: yes\r\n"
	    << "Message-Account: " << laure_uri << "\r\n"
	    << "Voice-Message: 4/8 (1/2)\r\n";
	string mwi_notify = mwi_notify_stream.str();
	bctbx_free(laure_uri);

	LinphoneTransports *tp = linphone_core_get_transports_used(laure->lc);
	BC_ASSERT_TRUE(liblinphone_tester_send_data(mwi_notify.c_str(), mwi_notify.length(), "127.0.0.1",
	                                            linphone_transports_get_udp_port(tp), SOCK_DGRAM) > 0);
	linphone_transports_unref(tp);

	stats *counters = &laure->stat;
	BC_ASSERT(wait_for(laure->lc, laure->lc, &counters->number_of_mwi, 1));
	BC_ASSERT_EQUAL(counters->number_of_new_LinphoneMessageWaitingIndicationVoice, 4, int, "%d");
	BC_ASSERT_EQUAL(counters->number_of_old_LinphoneMessageWaitingIndicationVoice, 8, int, "%d");
	BC_ASSERT_EQUAL(counters->number_of_new_urgent_LinphoneMessageWaitingIndicationVoice, 1, int, "%d");
	BC_ASSERT_EQUAL(counters->number_of_old_urgent_LinphoneMessageWaitingIndicationVoice, 2, int, "%d");

	linphone_core_manager_destroy(laure);
}

static test_t mwi_tests[] = {
    TEST_NO_TAG("Parse minimal MWI", parse_minimal_mwi),
    TEST_NO_TAG("Parse normal MWI", parse_normal_mwi),
    TEST_NO_TAG("Parse mwi without account address", parse_mwi_without_account_address),
    TEST_NO_TAG("MWI notified on account", mwi_notified_on_account),
};

static int suite_begin(void) {
	// Supposed to be done by platform helper, but in this case, we don't have it"
	belr::GrammarLoader::get().addPath(std::string(bc_tester_get_resource_dir_prefix()).append("/share/belr/grammars"));
	return 0;
}
test_suite_t mwi_test_suite = {"Mwi",
                               suite_begin,
                               NULL,
                               liblinphone_tester_before_each,
                               liblinphone_tester_after_each,
                               sizeof(mwi_tests) / sizeof(mwi_tests[0]),
                               mwi_tests,
                               0};
