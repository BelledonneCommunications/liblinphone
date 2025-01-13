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

#include "bctoolbox/utils.hh"

#include "liblinphone_tester.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

// =============================================================================

using namespace LinphonePrivate;

static void out_of_dialog_refer_received_cb(LinphoneCore *core,
                                            const LinphoneAddress *referToAddr,
                                            const LinphoneHeaders *customHeaders,
                                            const LinphoneContent *content) {
	if (!referToAddr) return;

	stats *counters = get_stats(core);
	counters->number_of_out_of_dialog_refer_received++;

	const char *headerValue = linphone_headers_get_value(customHeaders, "X-WD-Source-System");
	BC_ASSERT_STRING_EQUAL(headerValue, "SANFRAN");
	headerValue = linphone_headers_get_value(customHeaders, "Refer-To");
	BC_ASSERT_STRING_EQUAL(headerValue, "cid:6a650fba");

	BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "multipart");
	BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "mixed");
	BC_ASSERT_TRUE(linphone_content_is_multipart(content));

	LinphoneContent *firstPart = linphone_content_get_part(content, 0);
	BC_ASSERT_PTR_NOT_NULL(firstPart);
	BC_ASSERT_STRING_EQUAL(linphone_content_get_type(firstPart), "application");
	BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(firstPart), "x-cisco-remotecc-request+xml");
	BC_ASSERT_STRING_EQUAL(linphone_content_get_utf8_text(firstPart), "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	                                                                  "<x-cisco-remotecc-request>\n"
	                                                                  "<statuslineupdatereq>\n"
	                                                                  "<action>notify_display</action>\n"
	                                                                  "<statustext>CP 401 -> CP 302</statustext>\n"
	                                                                  "<displaytimeout>6</displaytimeout>\n"
	                                                                  "<linenumber>0</linenumber>\n"
	                                                                  "<priority>1</priority>\n"
	                                                                  "</statuslineupdatereq>\n"
	                                                                  "</x-cisco-remotecc-request>\n");
	linphone_content_unref(firstPart);

	LinphoneContent *secondPart = linphone_content_get_part(content, 1);
	BC_ASSERT_PTR_NOT_NULL(secondPart);
	BC_ASSERT_STRING_EQUAL(linphone_content_get_type(secondPart), "application");
	BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(secondPart), "x-cisco-remotecc-request+xml");
	BC_ASSERT_STRING_EQUAL(linphone_content_get_utf8_text(secondPart), "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	                                                                   "<x-cisco-remotecc-request>\n"
	                                                                   "<playtonereq>\n"
	                                                                   "<tonetype>DtZipZip</tonetype>\n"
	                                                                   "<direction>all</direction>\n"
	                                                                   "</playtonereq>\n"
	                                                                   "</x-cisco-remotecc-request>\n");
	linphone_content_unref(secondPart);
}

static void test_out_of_dialog_refer(void) {
	const char *out_of_dialog_refer =
	    "REFER sip:301.SEPAC7E8AB76184@10.189.1.19:5060;transport=udp SIP/2.0\r\n"
	    "Record-Route: <sip:172.25.132.116;lr;ftag=as0797a5c8>\r\n"
	    "Via: SIP/2.0/UDP 172.25.132.116;branch=z9hG4bKe68a.c4e34f47e3e4789051e40946056b1bc3.0;rport\r\n"
	    "Via: SIP/2.0/UDP 10.42.100.29:5060;rport=5060;branch=z9hG4bK6c155988\r\n"
	    "Max-Forwards: 69\r\n"
	    "From: <sip:asterisk@10.42.100.29>;tag=as0797a5c8\r\n"
	    "To: <sip:301.SEPAC7E8AB76184@10.189.1.19:5060;transport=udp>\r\n"
	    "Contact: <sip:asterisk@10.42.100.29:5060>\r\n"
	    "Call-ID: 3129ef634a95e061529537cc52d4237f@10.42.100.29:5060\r\n"
	    "CSeq: 101 REFER\r\n"
	    "Require: norefersub\r\n"
	    "Refer-To: cid:6a650fba\r\n"
	    "Content-Id: 6a650fba\r\n"
	    "Content-Type: multipart/mixed; boundary=uniqueBoundary\r\n"
	    "Expires: 0\r\n"
	    "Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY, INFO, PUBLISH, MESSAGE\r\n"
	    "Supported: replaces,timer,X-cisco-sis-10.0.0\r\n"
	    "Content-Length: 654\r\n"
	    "X-WD-Source-System: SANFRAN\r\n"
	    "\r\n"
	    "--uniqueBoundary\r\n"
	    "Content-Type: application/x-cisco-remotecc-request+xml\r\n"
	    "\r\n"
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	    "<x-cisco-remotecc-request>\n"
	    "<statuslineupdatereq>\n"
	    "<action>notify_display</action>\n"
	    "<statustext>CP 401 -> CP 302</statustext>\n"
	    "<displaytimeout>6</displaytimeout>\n"
	    "<linenumber>0</linenumber>\n"
	    "<priority>1</priority>\n"
	    "</statuslineupdatereq>\n"
	    "</x-cisco-remotecc-request>\n"
	    "\r\n"
	    "--uniqueBoundary\r\n"
	    "Content-Type: application/x-cisco-remotecc-request+xml\r\n"
	    "\r\n"
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	    "<x-cisco-remotecc-request>\n"
	    "<playtonereq>\n"
	    "<tonetype>DtZipZip</tonetype>\n"
	    "<direction>all</direction>\n"
	    "</playtonereq>\n"
	    "</x-cisco-remotecc-request>\n"
	    "\r\n"
	    "--uniqueBoundary--\r\n";

	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_refer_received(cbs, out_of_dialog_refer_received_cb);
	linphone_core_add_callbacks(laure->lc, cbs);

	LinphoneTransports *tp = linphone_core_get_transports_used(laure->lc);
	BC_ASSERT_TRUE(liblinphone_tester_send_data(out_of_dialog_refer, strlen(out_of_dialog_refer), "127.0.0.1",
	                                            linphone_transports_get_udp_port(tp), SOCK_DGRAM) > 0);
	linphone_transports_unref(tp);

	BC_ASSERT_TRUE(wait_for_until(laure->lc, nullptr, &laure->stat.number_of_out_of_dialog_refer_received, 1, 1000));

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(laure);
}

// clang-format off
static test_t refer_tests[] = {
    TEST_NO_TAG("Out-of-dialog REFER", test_out_of_dialog_refer),
};
// clang-format on

test_suite_t refer_test_suite = {"Refer",
                                 nullptr,
                                 nullptr,
                                 liblinphone_tester_before_each,
                                 liblinphone_tester_after_each,
                                 sizeof(refer_tests) / sizeof(refer_tests[0]),
                                 refer_tests,
                                 1,
                                 0};
