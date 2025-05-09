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
#include "linphone/api/c-account-cbs.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-content.h"
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

static void mwi_to_content() {
	const string str = "Messages-Waiting: yes\r\n"
	                   "Message-Account: sip:alice@vmail.example.com\r\n"
	                   "Voice-Message: 4/8 (1/2)\r\n";

	shared_ptr<const Mwi::MessageWaitingIndication> mwi = Mwi::Parser::getInstance()->parseMessageSummary(str);
	if (!BC_ASSERT_PTR_NOT_NULL(mwi)) return;

	shared_ptr<Mwi::MessageWaitingIndication> newMwi = mwi->clone()->toSharedPtr();
	shared_ptr<Address> newAddress = newMwi->getAccountAddress()->clone()->toSharedPtr();
	newAddress->setUsername("bob");
	newMwi->setAccountAddress(newAddress);
	shared_ptr<Content> content = newMwi->toContent();

	auto contentType = content->getContentType().asString();
	auto expectedContentType = ContentType::Mwi.asString();
	auto contentBody = content->getBodyAsUtf8String();
	const string expectedBody = "Messages-Waiting: yes\r\n"
	                            "Message-Account: sip:bob@vmail.example.com\r\n"
	                            "Voice-Message: 4/8 (1/2)\r\n";

	BC_ASSERT_STRING_EQUAL(contentType.c_str(), expectedContentType.c_str());
	BC_ASSERT_STRING_EQUAL(contentBody.c_str(), expectedBody.c_str());
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

static LinphoneMessageWaitingIndication *refd_mwi = nullptr;

static void mwi_changed_on_account_2(LinphoneAccount *account, const LinphoneMessageWaitingIndication *mwi) {
	LinphoneCore *lc = linphone_account_get_core(account);
	/* this external ref is absolutely not necessary for the test, it is just there to verify
	 * that reffing the passed LinphoneMessageWaitingIndication object (like wrappers do) and freeing it later
	 * does not create any bug.
	 */
	if (!refd_mwi) {
		refd_mwi = linphone_message_waiting_indication_ref((LinphoneMessageWaitingIndication *)mwi);
	}
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
	BC_ASSERT_PTR_NULL(linphone_message_waiting_indication_get_summary(mwi, LinphoneMessageWaitingIndicationPager));
}

static void mwi_subscription_state_changed(LinphoneCore *lc, LinphoneEvent *lev, LinphoneSubscriptionState state) {
	stats *counters = get_stats(lc);
	LinphoneCoreManager *mgr = get_manager(lc);

	const LinphoneAddress *from_addr = linphone_event_get_from(lev);
	char *from = linphone_address_as_string(from_addr);
	const LinphoneAddress *to_addr = linphone_event_get_to(lev);
	char *to = linphone_address_as_string(to_addr);
	ms_message("Subscription state [%s] from [%s] to [%s]", linphone_subscription_state_to_string(state), from, to);

	switch (state) {
		case LinphoneSubscriptionNone:
			break;
		case LinphoneSubscriptionIncomingReceived:
			counters->number_of_LinphoneSubscriptionIncomingReceived++;
			mgr->lev = lev;
			break;
		case LinphoneSubscriptionOutgoingProgress:
			counters->number_of_LinphoneSubscriptionOutgoingProgress++;
			break;
		case LinphoneSubscriptionPending:
			counters->number_of_LinphoneSubscriptionPending++;
			break;
		case LinphoneSubscriptionActive:
			counters->number_of_LinphoneSubscriptionActive++;
			if (mgr->subscribe_policy == AcceptSubscription) {
				if (linphone_event_get_subscription_dir(lev) == LinphoneSubscriptionIncoming) {
					mgr->lev = lev;
					if (strcmp(linphone_event_get_name(lev), "message-summary") == 0) {
						auto mwi_stream = std::ostringstream();
						mwi_stream << "Messages-Waiting: yes\r\n"
						           << "Message-Account: " << from << "\r\n"
						           << "Voice-Message: 4/8 (1/2)\r\n";
						string mwi = mwi_stream.str();

						LinphoneContent *content = linphone_core_create_content(lc);
						linphone_content_set_type(content, "application");
						linphone_content_set_subtype(content, "simple-message-summary");
						linphone_content_set_buffer(content, (const uint8_t *)mwi.c_str(), mwi.size());
						linphone_event_notify(lev, content);
						linphone_content_unref(content);
					}
				}
			}
			break;
		case LinphoneSubscriptionTerminated:
			counters->number_of_LinphoneSubscriptionTerminated++;
			if (lev == mgr->lev) {
				mgr->lev = NULL;
			}
			break;
		case LinphoneSubscriptionError:
			counters->number_of_LinphoneSubscriptionError++;
			if (lev == mgr->lev) {
				mgr->lev = NULL;
			}
			break;
		case LinphoneSubscriptionExpiring:
			counters->number_of_LinphoneSubscriptionExpiring++;
			if (lev == mgr->lev) {
				mgr->lev = NULL;
			}
			break;
	}

	ms_free(from);
	ms_free(to);
}

static void mwi_notified_on_account(void) {
	// This test subscribes on MWI from pauline to herself in order for the request uri and
	// the To header to be valid addresses that will be routed correctly and the subscribe be handled.
	// If we used an other account as the destination of the subscribe by setting the request
	// uri to this account, it would be routed correctly but not handled because the To header
	// would still be the pauline account.

	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneAccount *pauline_account = linphone_core_get_default_account(pauline->lc);
	linphone_core_cbs_set_subscription_state_changed(pauline->cbs, mwi_subscription_state_changed);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, nullptr, &pauline->stat.number_of_LinphoneRegistrationOk, 1, 5000));

	LinphoneAccountCbs *cbs = linphone_factory_create_account_cbs(linphone_factory_get());
	linphone_account_cbs_set_message_waiting_indication_changed(cbs, mwi_changed_on_account);
	linphone_account_add_callbacks(pauline_account, cbs);
	linphone_account_cbs_unref(cbs);

	const LinphoneAccountParams *initial_account_params = linphone_account_get_params(pauline_account);
	const LinphoneAddress *pauline_address = linphone_account_params_get_identity_address(initial_account_params);
	LinphoneAccountParams *new_account_params = linphone_account_params_clone(initial_account_params);
	LinphoneAddress *new_pauline_address = linphone_address_clone(pauline_address);
	linphone_account_params_set_mwi_server_address(new_account_params, new_pauline_address);
	linphone_account_set_params(pauline_account, new_account_params);
	linphone_account_params_unref(new_account_params);
	linphone_address_unref(new_pauline_address);

	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, nullptr, &pauline->stat.number_of_LinphoneSubscriptionIncomingReceived, 1, 5000));
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, nullptr, &pauline->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
	BC_ASSERT(wait_for(pauline->lc, pauline->lc, &pauline->stat.number_of_mwi, 1));
	BC_ASSERT_EQUAL(pauline->stat.number_of_new_LinphoneMessageWaitingIndicationVoice, 4, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_old_LinphoneMessageWaitingIndicationVoice, 8, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_new_urgent_LinphoneMessageWaitingIndicationVoice, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_old_urgent_LinphoneMessageWaitingIndicationVoice, 2, int, "%d");

	linphone_core_manager_destroy(pauline);
}

static void mwi_out_of_dialog() {
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAccount *pauline_account = linphone_core_get_default_account(pauline->lc);

	LinphoneAccountCbs *cbs = linphone_factory_create_account_cbs(linphone_factory_get());
	linphone_account_cbs_set_message_waiting_indication_changed(cbs, mwi_changed_on_account_2);
	linphone_account_add_callbacks(pauline_account, cbs);
	linphone_account_cbs_unref(cbs);

	LinphoneEvent *ev = linphone_core_create_notify(marie->lc, pauline->identity, "message-summary");
	LinphoneContent *ct = linphone_core_create_content(marie->lc);
	linphone_content_set_type(ct, "application");
	linphone_content_set_subtype(ct, "simple-message-summary");

	char *tmp = linphone_address_as_string_uri_only(pauline->identity);
	char *mwi_body = bctbx_strdup_printf("Messages-Waiting: yes\r\n"
	                                     "Message-Account: %s\r\n"
	                                     "Voice-Message: 2/8 (0/2)\r\n",
	                                     tmp);

	linphone_content_set_utf8_text(ct, mwi_body);

	linphone_event_notify(ev, ct);
	bctbx_free(tmp);
	bctbx_free(mwi_body);

	linphone_content_unref(ct);
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_mwi, 1));
	linphone_event_unref(ev);
	linphone_message_waiting_indication_unref(refd_mwi);
	refd_mwi = NULL;

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static test_t mwi_tests[] = {TEST_NO_TAG("Parse minimal MWI", parse_minimal_mwi),
                             TEST_NO_TAG("Parse normal MWI", parse_normal_mwi),
                             TEST_NO_TAG("Parse mwi without account address", parse_mwi_without_account_address),
                             TEST_NO_TAG("MWI to content", mwi_to_content),
                             TEST_NO_TAG("MWI notified on account", mwi_notified_on_account),
                             TEST_NO_TAG("MWI out of dialog", mwi_out_of_dialog)};

static int suite_begin() {
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
