/*
 * cpim-tester.cpp
 * Copyright (C) 2017  Belledonne Communications SARL
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "address/address.h"
#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/basic-chat-room.h"
#include "content/content-type.h"
#include "content/content.h"
#include "content/file-content.h"
#include "core/core.h"

// TODO: Remove me later.
#include "private.h"

#include "liblinphone_tester.h"
#include "tester_utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

static void check_contents(const bctbx_list_t *contents, bool first_file_transfer, bool second_file_transfer, bool third_content) {
	BC_ASSERT_PTR_NOT_NULL(contents);
	if (third_content)
		BC_ASSERT_EQUAL(bctbx_list_size(contents), 3, int, "%d");
	else
		BC_ASSERT_EQUAL(bctbx_list_size(contents), 2, int, "%d");

	int textContentCount = 0;
	int fileTransferContentCount = 0;
	int unexpectedContentCount = 0;
	const bctbx_list_t *it;

	for (it = contents; it != NULL; it = bctbx_list_next(it)) {
		const LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(it);
		BC_ASSERT_PTR_NOT_NULL(content);

		if (linphone_content_is_file_transfer(content)) {
			fileTransferContentCount += 1;
			if (first_file_transfer && second_file_transfer) {
				// Order should be maintained
				if (fileTransferContentCount == 1) {
					BC_ASSERT_EQUAL(linphone_content_get_file_size(content), 1095946, int, "%d");
					BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), "sintel_trailer_opus_h264.mkv");
				} else {
					BC_ASSERT_EQUAL(linphone_content_get_file_size(content), 425, int, "%d");
					BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), "vcards.vcf");
				}
			} else if (first_file_transfer || second_file_transfer) {
				if (first_file_transfer) {
					BC_ASSERT_EQUAL(linphone_content_get_file_size(content), 1095946, int, "%d");
					BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), "sintel_trailer_opus_h264.mkv");
				} else {
					BC_ASSERT_EQUAL(linphone_content_get_file_size(content), 425, int, "%d");
					BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), "vcards.vcf");
				}
			}
		} else if (linphone_content_is_text(content)) {
			textContentCount += 1;
			if (first_file_transfer && second_file_transfer) {
				if (third_content) {
					BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 3");
				}
			} else if (first_file_transfer || second_file_transfer) {
				if (third_content) {
					// Order should be maintained
					if (first_file_transfer) {
						if (textContentCount == 1) {
							BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 2");
						} else {
							BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 3");
						}
					} else {
						if (textContentCount == 1) {
							BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 1");
						} else {
							BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 3");
						}
					}
				} else {
					if (first_file_transfer) {
						BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 2");
					} else {
						BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 1");
					}
				}
			} else {
				// Order should be maintained
				if (textContentCount == 1) {
					BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 1");
				} else if (textContentCount == 2) {
					BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 2");
				} else {
					BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), "Hello part 3");
				}
			}
		} else {
			unexpectedContentCount += 1;
			lError() << "Content type is " << linphone_content_get_type(content) << "/" << linphone_content_get_subtype(content);
		}
	}

	if (first_file_transfer && second_file_transfer) {
		BC_ASSERT_EQUAL(textContentCount, third_content ? 1 : 0, int, "%d");
		BC_ASSERT_EQUAL(fileTransferContentCount, 2, int, "%d");
		BC_ASSERT_EQUAL(unexpectedContentCount, 0, int, "%d");
	} else if (first_file_transfer || second_file_transfer) {
		BC_ASSERT_EQUAL(textContentCount, third_content ? 2 : 1, int, "%d");
		BC_ASSERT_EQUAL(fileTransferContentCount, 1, int, "%d");
		BC_ASSERT_EQUAL(unexpectedContentCount, 0, int, "%d");
	} else {
		BC_ASSERT_EQUAL(textContentCount, third_content ? 3 : 2, int, "%d");
		BC_ASSERT_EQUAL(fileTransferContentCount, 0, int, "%d");
		BC_ASSERT_EQUAL(unexpectedContentCount, 0, int, "%d");
	}
}

static void chat_message_multipart_modifier_base(bool first_file_transfer, bool second_file_transfer, bool third_content, bool use_cpim) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");

	char *paulineUriStr = linphone_address_as_string_uri_only(pauline->identity);
	IdentityAddress paulineAddress(paulineUriStr);
	bctbx_free(paulineUriStr);
	shared_ptr<AbstractChatRoom> marieRoom = marie->lc->cppPtr->getOrCreateBasicChatRoom(paulineAddress);
	marieRoom->allowMultipart(true);

	shared_ptr<ChatMessage> marieMessage = marieRoom->createChatMessage();
	if (first_file_transfer) {
		char *send_filepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
		FileContent *content = new FileContent();
		content->setContentType(ContentType("video/mkv"));
		content->setFilePath(send_filepath);
		content->setFileName("sintel_trailer_opus_h264.mkv");
		marieMessage->addContent(content);
		bc_free(send_filepath);
	} else {
		Content *content = new Content();
		content->setContentType(ContentType::PlainText);
		content->setBody("Hello part 1");
		marieMessage->addContent(content);
	}

	if (second_file_transfer) {
		char *send_filepath = bc_tester_res("vcards/vcards.vcf");
		FileContent *content = new FileContent();
		content->setContentType(ContentType("file/vcf"));
		content->setFilePath(send_filepath);
		content->setFileName("vcards.vcf");
		marieMessage->addContent(content);
		bc_free(send_filepath);
	} else {
		Content *content = new Content();
		content->setContentType(ContentType::PlainText);
		content->setBody("Hello part 2");
		marieMessage->addContent(content);
	}

	if (third_content) {
		Content *content = new Content();
		content->setContentType(ContentType::PlainText);
		content->setBody("Hello part 3");
		marieMessage->addContent(content);
	}

	linphone_core_set_file_transfer_server(marie->lc,"https://www.linphone.org:444/lft.php");
	marieMessage->send();

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageReceived,1));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);

	const bctbx_list_t *contents = linphone_chat_message_get_contents(pauline->stat.last_received_chat_message);
	check_contents(contents, first_file_transfer, second_file_transfer, third_content);

	marieRoom.reset(); // Avoid bad weak ptr when the core is destroyed below this line.

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void multipart_two_text_content(void) {
	chat_message_multipart_modifier_base(false, false, false, false);
}

static void multipart_two_text_content_with_cpim(void) {
	chat_message_multipart_modifier_base(false, false, false, true);
}

static void multipart_one_text_and_one_file_content(void) {
	chat_message_multipart_modifier_base(true, false, false, false);
}

static void multipart_one_text_and_one_file_content_with_cpim(void) {
	chat_message_multipart_modifier_base(true, false, false, true);
}

static void multipart_two_file_content(void) {
	chat_message_multipart_modifier_base(true, true, false, false);
}

static void multipart_two_file_content_with_cpim(void) {
	chat_message_multipart_modifier_base(true, true, false, true);
}

static void multipart_two_file_content_and_one_text(void) {
	chat_message_multipart_modifier_base(true, true, true, false);
}

static void multipart_two_file_content_and_one_text_with_cpim(void) {
	chat_message_multipart_modifier_base(true, true, true, true);
}

test_t multipart_tests[] = {
	TEST_NO_TAG("Chat message multipart 2 text content", multipart_two_text_content),
	TEST_NO_TAG("Chat message multipart 2 text content with CPIM", multipart_two_text_content_with_cpim),
	TEST_NO_TAG("Chat message multipart 1 file content and 1 text content", multipart_one_text_and_one_file_content),
	TEST_NO_TAG("Chat message multipart 1 file content and 1 text content with CPIM", multipart_one_text_and_one_file_content_with_cpim),
	TEST_NO_TAG("Chat message multipart 2 file content", multipart_two_file_content),
	TEST_NO_TAG("Chat message multipart 2 file content with CPIM", multipart_two_file_content_with_cpim),
	TEST_NO_TAG("Chat message multipart 2 file content and 1 text", multipart_two_file_content_and_one_text),
	TEST_NO_TAG("Chat message multipart 2 file content and 1 text with CPIM", multipart_two_file_content_and_one_text_with_cpim),
};

test_suite_t multipart_test_suite = {
	"Multipart", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(multipart_tests) / sizeof(multipart_tests[0]), multipart_tests
};
