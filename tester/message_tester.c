/*
	liblinphone_tester - liblinphone test suite
	Copyright (C) 2013  Belledonne Communications SARL

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "linphone/core.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"
#include "lime.h"
#include "bctoolbox/crypto.h"
#include <belle-sip/object.h>
#include "linphone/core_utils.h"
#include <bctoolbox/vfs.h>

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif


static char* message_external_body_url=NULL;

/* sql cache creation string, contains 3 string to be inserted : selfuri/selfuri/peeruri */
static const char *marie_zid_sqlcache = "BEGIN TRANSACTION; CREATE TABLE IF NOT EXISTS ziduri (zuid          INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,zid		BLOB NOT NULL DEFAULT '000000000000',selfuri	 TEXT NOT NULL DEFAULT 'unset',peeruri	 TEXT NOT NULL DEFAULT 'unset'); INSERT INTO `ziduri` (zuid,zid,selfuri,peeruri) VALUES (1,X'4ddc8042bee500ad0366bf93','%s','self'), (2,X'bcb4028bf55e1b7ac4c4edee','%s','%s'); CREATE TABLE IF NOT EXISTS zrtp (zuid		INTEGER NOT NULL DEFAULT 0 UNIQUE,rs1		BLOB DEFAULT NULL,rs2		BLOB DEFAULT NULL,aux		BLOB DEFAULT NULL,pbx		BLOB DEFAULT NULL,pvs		BLOB DEFAULT NULL,FOREIGN KEY(zuid) REFERENCES ziduri(zuid) ON UPDATE CASCADE ON DELETE CASCADE); INSERT INTO `zrtp` (zuid,rs1,rs2,aux,pbx,pvs) VALUES (2,X'f0e0ad4d3d4217ba4048d1553e5ab26fae0b386cdac603f29a66d5f4258e14ef',NULL,NULL,NULL,X'01'); CREATE TABLE IF NOT EXISTS lime (zuid		INTEGER NOT NULL DEFAULT 0 UNIQUE,sndKey		BLOB DEFAULT NULL,rcvKey		BLOB DEFAULT NULL,sndSId		BLOB DEFAULT NULL,rcvSId		BLOB DEFAULT NULL,sndIndex	BLOB DEFAULT NULL,rcvIndex	BLOB DEFAULT NULL,valid		BLOB DEFAULT NULL,FOREIGN KEY(zuid) REFERENCES ziduri(zuid) ON UPDATE CASCADE ON DELETE CASCADE); INSERT INTO `lime` (zuid,sndKey,rcvKey,sndSId,rcvSId,sndIndex,rcvIndex,valid) VALUES (2,X'97c75a5a92a041b415296beec268efc3373ef4aa8b3d5f301ac7522a7fb4e332',x'3b74b709b961e5ebccb1db6b850ea8c1f490546d6adee2f66b5def7093cead3d',X'e2ebca22ad33071bc37631393bf25fc0a9badeea7bf6dcbcb5d480be7ff8c5ea',X'a2086d195344ec2997bf3de7441d261041cda5d90ed0a0411ab2032e5860ea48',X'33376935',X'7ce32d86',X'0000000000000000'); COMMIT;";

static const char *pauline_zid_sqlcache = "BEGIN TRANSACTION; CREATE TABLE IF NOT EXISTS ziduri (zuid          INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,zid		BLOB NOT NULL DEFAULT '000000000000',selfuri	 TEXT NOT NULL DEFAULT 'unset',peeruri	 TEXT NOT NULL DEFAULT 'unset'); INSERT INTO `ziduri` (zuid,zid,selfuri,peeruri) VALUES (1,X'bcb4028bf55e1b7ac4c4edee','%s','self'), (2,X'4ddc8042bee500ad0366bf93','%s','%s'); CREATE TABLE IF NOT EXISTS zrtp (zuid		INTEGER NOT NULL DEFAULT 0 UNIQUE,rs1		BLOB DEFAULT NULL,rs2		BLOB DEFAULT NULL,aux		BLOB DEFAULT NULL,pbx		BLOB DEFAULT NULL,pvs		BLOB DEFAULT NULL,FOREIGN KEY(zuid) REFERENCES ziduri(zuid) ON UPDATE CASCADE ON DELETE CASCADE); INSERT INTO `zrtp` (zuid,rs1,rs2,aux,pbx,pvs) VALUES (2,X'f0e0ad4d3d4217ba4048d1553e5ab26fae0b386cdac603f29a66d5f4258e14ef',NULL,NULL,NULL,X'01'); CREATE TABLE IF NOT EXISTS lime (zuid		INTEGER NOT NULL DEFAULT 0 UNIQUE,sndKey		BLOB DEFAULT NULL,rcvKey		BLOB DEFAULT NULL,sndSId		BLOB DEFAULT NULL,rcvSId		BLOB DEFAULT NULL,sndIndex	BLOB DEFAULT NULL,rcvIndex	BLOB DEFAULT NULL,valid		BLOB DEFAULT NULL,FOREIGN KEY(zuid) REFERENCES ziduri(zuid) ON UPDATE CASCADE ON DELETE CASCADE); INSERT INTO `lime` (zuid,rcvKey,sndKey,rcvSId,sndSId,rcvIndex,sndIndex,valid) VALUES (2,X'97c75a5a92a041b415296beec268efc3373ef4aa8b3d5f301ac7522a7fb4e332',x'3b74b709b961e5ebccb1db6b850ea8c1f490546d6adee2f66b5def7093cead3d',X'e2ebca22ad33071bc37631393bf25fc0a9badeea7bf6dcbcb5d480be7ff8c5ea',X'a2086d195344ec2997bf3de7441d261041cda5d90ed0a0411ab2032e5860ea48',X'33376935',X'7ce32d86',X'0000000000000000'); COMMIT;";

static const char *xmlCacheMigration = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<cache><selfZID>00112233445566778899aabb</selfZID><peer><ZID>99887766554433221100ffee</ZID><rs1>c4274f13a2b6fa05c15ec93158f930e7264b0a893393376dbc80c6eb1cccdc5a</rs1><uri>sip:bob@sip.linphone.org</uri><sndKey>219d9e445d10d4ed64083c7ccbb83a23bc17a97df0af5de4261f3fe026b05b0b</sndKey><rcvKey>747e72a5cc996413cb9fa6e3d18d8b370436e274cd6ba4efc1a4580340af57ca</rcvKey><sndSId>df2bf38e719fa89e17332cf8d5e774ee70d347baa74d16dee01f306c54789869</sndSId><rcvSId>928ce78b0bfc30427a02b1b668b2b3b0496d5664d7e89b75ed292ee97e3fc850</rcvSId><sndIndex>496bcc89</sndIndex><rcvIndex>59337abe</rcvIndex><rs2>5dda11f388384b349d210612f30824268a3753a7afa52ef6df5866dca76315c4</rs2><uri>sip:bob2@sip.linphone.org</uri></peer><peer><ZID>ffeeddccbbaa987654321012</ZID><rs1>858b495dfad483af3c088f26d68c4beebc638bd44feae45aea726a771727235e</rs1><uri>sip:bob@sip.linphone.org</uri><sndKey>b6aac945057bc4466bfe9a23771c6a1b3b8d72ec3e7d8f30ed63cbc5a9479a25</sndKey><rcvKey>bea5ac3225edd0545b816f061a8190370e3ee5160e75404846a34d1580e0c263</rcvKey><sndSId>17ce70fdf12e500294bcb5f2ffef53096761bb1c912b21e972ae03a5a9f05c47</sndSId><rcvSId>7e13a20e15a517700f0be0921f74b96d4b4a0c539d5e14d5cdd8706441874ac0</rcvSId><sndIndex>75e18caa</sndIndex><rcvIndex>2cfbbf06</rcvIndex><rs2>1533dee20c8116dc2c282cae9adfea689b87bc4c6a4e18a846f12e3e7fea3959</rs2></peer><peer><ZID>0987654321fedcba5a5a5a5a</ZID><rs1>cb6ecc87d1dd87b23f225eec53a26fc541384917623e0c46abab8c0350c6929e</rs1><sndKey>92bb03988e8f0ccfefa37a55fd7c5893bea3bfbb27312f49dd9b10d0e3c15fc7</sndKey><rcvKey>2315705a5830b98f68458fcd49623144cb34a667512c4d44686aee125bb8b622</rcvKey><sndSId>94c56eea0dd829379263b6da3f6ac0a95388090f168a3568736ca0bd9f8d595f</sndSId><rcvSId>c319ae0d41183fec90afc412d42253c5b456580f7a463c111c7293623b8631f4</rcvSId><uri>sip:bob@sip.linphone.org</uri><sndIndex>2c46ddcc</sndIndex><rcvIndex>15f5779e</rcvIndex><valid>0000000058f095bf</valid><pvs>01</pvs></peer></cache>";

void message_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage* msg) {
	char* from=linphone_address_as_string(linphone_chat_message_get_from_address(msg));
	stats* counters;
	const char *text=linphone_chat_message_get_text(msg);
	const char *external_body_url=linphone_chat_message_get_external_body_url(msg);
	ms_message("Message from [%s]  is [%s] , external URL [%s]",from?from:""
																,text?text:""
																,external_body_url?external_body_url:"");
	ms_free(from);
	counters = get_stats(lc);
	counters->number_of_LinphoneMessageReceived++;
	if (counters->last_received_chat_message) {
		linphone_chat_message_unref(counters->last_received_chat_message);
	}
	counters->last_received_chat_message=linphone_chat_message_ref(msg);
	LinphoneContent * content = linphone_chat_message_get_file_transfer_information(msg);
	if (content)
		counters->number_of_LinphoneMessageReceivedWithFile++;
	else if (linphone_chat_message_get_external_body_url(msg)) {
		counters->number_of_LinphoneMessageExtBodyReceived++;
		if (message_external_body_url) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_external_body_url(msg),message_external_body_url);
			message_external_body_url=NULL;
		}
	}
}

/**
 * function invoked when a file transfer is received.
 * */
void file_transfer_received(LinphoneChatMessage *msg, const LinphoneContent* content, const LinphoneBuffer *buffer){
	FILE* file=NULL;
	char *receive_file = NULL;

	// If a file path is set, we should NOT call the on_recv callback !
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_file_transfer_filepath(msg));

	receive_file = bc_tester_file("receive_file.dump");
	if (!linphone_chat_message_get_user_data(msg)) {
		/*first chunk, creating file*/
		file = fopen(receive_file,"wb");
		linphone_chat_message_set_user_data(msg,(void*)file); /*store fd for next chunks*/
	}
	
	file = (FILE*)linphone_chat_message_get_user_data(msg);
	BC_ASSERT_PTR_NOT_NULL(file);
	if (linphone_buffer_is_empty(buffer)) { /* tranfer complete */
		struct stat st;
		
		linphone_chat_message_set_user_data(msg, NULL);
		fclose(file);
		BC_ASSERT_TRUE(stat(receive_file, &st)==0);
		BC_ASSERT_EQUAL((int)linphone_content_get_file_size(content), (int)st.st_size, int, "%i");
	} else { /* store content on a file*/
		if (fwrite(linphone_buffer_get_content(buffer),linphone_buffer_get_size(buffer),1,file)==0){
			ms_error("file_transfer_received(): write() failed: %s",strerror(errno));
		}
	}
	bc_free(receive_file);
}

/*
 * function called when the file transfer is initiated. file content should be feed into object LinphoneContent
 * */
LinphoneBuffer * tester_file_transfer_send(LinphoneChatMessage *msg, const LinphoneContent* content, size_t offset, size_t size){
	LinphoneBuffer *lb;
	size_t file_size;
	size_t size_to_send;
	uint8_t *buf;
	FILE *file_to_send = linphone_chat_message_get_user_data(msg);

	// If a file path is set, we should NOT call the on_send callback !
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_file_transfer_filepath(msg));

	BC_ASSERT_PTR_NOT_NULL(file_to_send);
	if (file_to_send == NULL){
		return NULL;
	}
	fseek(file_to_send, 0, SEEK_END);
	file_size = ftell(file_to_send);
	fseek(file_to_send, (long)offset, SEEK_SET);
	size_to_send = MIN(size, file_size - offset);
	buf = ms_malloc(size_to_send);
	if (fread(buf, sizeof(uint8_t), size_to_send, file_to_send) != size_to_send){
		// reaching end of file, close it
		fclose(file_to_send);
		linphone_chat_message_set_user_data(msg, NULL);
	}
	lb = linphone_buffer_new_from_data(buf, size_to_send);
	ms_free(buf);
	return lb;
}

/**
 * function invoked to report file transfer progress.
 * */
void file_transfer_progress_indication(LinphoneChatMessage *msg, const LinphoneContent* content, size_t offset, size_t total) {
	LinphoneChatRoom *cr = linphone_chat_message_get_chat_room(msg);
	LinphoneCore *lc = linphone_chat_room_get_core(cr);
	const LinphoneAddress* from_address = linphone_chat_message_get_from_address(msg);
	const LinphoneAddress* to_address = linphone_chat_message_get_to_address(msg);
	char *address = linphone_chat_message_is_outgoing(msg)?linphone_address_as_string(to_address):linphone_address_as_string(from_address);
	stats* counters = get_stats(lc);
	int progress = (int)((offset * 100)/total);
	ms_message(" File transfer  [%d%%] %s of type [%s/%s] %s [%s] \n", progress
																	,(linphone_chat_message_is_outgoing(msg)?"sent":"received")
																	, linphone_content_get_type(content)
																	, linphone_content_get_subtype(content)
																	,(linphone_chat_message_is_outgoing(msg)?"to":"from")
																	, address);
	counters->progress_of_LinphoneFileTransfer = progress;
	if (progress == 100) {
		counters->number_of_LinphoneFileTransferDownloadSuccessful++;
	}
	free(address);
}

void is_composing_received(LinphoneCore *lc, LinphoneChatRoom *room) {
	stats *counters = get_stats(lc);
	if (linphone_chat_room_is_remote_composing(room)) {
		counters->number_of_LinphoneIsComposingActiveReceived++;
	} else {
		counters->number_of_LinphoneIsComposingIdleReceived++;
	}
}

void liblinphone_tester_chat_message_state_change(LinphoneChatMessage* msg,LinphoneChatMessageState state,void* ud) {
	liblinphone_tester_chat_message_msg_state_changed(msg, state);
}

void liblinphone_tester_chat_message_msg_state_changed(LinphoneChatMessage *msg, LinphoneChatMessageState state) {
	LinphoneChatRoom *cr = linphone_chat_message_get_chat_room(msg);
	LinphoneCore *lc = linphone_chat_room_get_core(cr);
	stats* counters = get_stats(lc);
	switch (state) {
		case LinphoneChatMessageStateIdle:
			return;
		case LinphoneChatMessageStateDelivered:
			counters->number_of_LinphoneMessageDelivered++;
			return;
		case LinphoneChatMessageStateNotDelivered:
			counters->number_of_LinphoneMessageNotDelivered++;
			return;
		case LinphoneChatMessageStateInProgress:
			counters->number_of_LinphoneMessageInProgress++;
			return;
		case LinphoneChatMessageStateFileTransferError:
			counters->number_of_LinphoneMessageNotDelivered++;
			counters->number_of_LinphoneMessageFileTransferError++;
			return;
		case LinphoneChatMessageStateFileTransferDone:
			counters->number_of_LinphoneMessageFileTransferDone++;
			return;
		case LinphoneChatMessageStateDeliveredToUser:
			counters->number_of_LinphoneMessageDeliveredToUser++;
			return;
		case LinphoneChatMessageStateDisplayed:
			counters->number_of_LinphoneMessageDisplayed++;
			return;
	}
	ms_error("Unexpected state [%s] for msg [%p]",linphone_chat_message_state_to_string(state), msg);
}

void compare_files(const char *path1, const char *path2) {
	size_t size1;
	size_t size2;
	uint8_t *buf1;
	uint8_t *buf2;

	buf1 = (uint8_t*)ms_load_path_content(path1, &size1);
	buf2 = (uint8_t*)ms_load_path_content(path2, &size2);
	BC_ASSERT_PTR_NOT_NULL(buf1);
	BC_ASSERT_PTR_NOT_NULL(buf2);
	if (buf1 && buf2){
		BC_ASSERT_EQUAL(memcmp(buf1, buf2, size1), 0, int, "%d");
	}
	BC_ASSERT_EQUAL((uint8_t)size2, (uint8_t)size1, uint8_t, "%u");

	if (buf1) ms_free(buf1);
	if (buf2) ms_free(buf2);
}

LinphoneChatMessage* create_message_from_sintel_trailer(LinphoneChatRoom *chat_room) {
	FILE *file_to_send = NULL;
	LinphoneChatMessageCbs *cbs;
	LinphoneContent* content;
	LinphoneChatMessage* msg;
	char *send_filepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	size_t file_size;
	file_to_send = fopen(send_filepath, "rb");
	fseek(file_to_send, 0, SEEK_END);
	file_size = ftell(file_to_send);
	fseek(file_to_send, 0, SEEK_SET);

	content = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
	belle_sip_object_set_name(BELLE_SIP_OBJECT(content), "sintel trailer content");
	linphone_content_set_type(content,"video");
	linphone_content_set_subtype(content,"mkv");
	linphone_content_set_size(content,file_size); /*total size to be transfered*/
	linphone_content_set_name(content,"sintel_trailer_opus_h264.mkv");

	msg = linphone_chat_room_create_file_transfer_message(chat_room, content);
	cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_file_transfer_send(cbs, tester_file_transfer_send);
	linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
	linphone_chat_message_set_user_data(msg, file_to_send);
	BC_ASSERT_PTR_NOT_NULL(linphone_chat_message_get_user_data(msg));

	linphone_content_unref(content);
	bc_free(send_filepath);
	return msg;
}

LinphoneChatMessage* create_file_transfer_message_from_sintel_trailer(LinphoneChatRoom *chat_room) {
	LinphoneChatMessageCbs *cbs;
	LinphoneContent* content;
	LinphoneChatMessage* msg;
	char *send_filepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");

	content = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
	belle_sip_object_set_name(BELLE_SIP_OBJECT(content), "sintel trailer content");
	linphone_content_set_type(content,"video");
	linphone_content_set_subtype(content,"mkv");
	linphone_content_set_name(content,"sintel_trailer_opus_h264.mkv");

	msg = linphone_chat_room_create_file_transfer_message(chat_room, content);
	linphone_chat_message_set_file_transfer_filepath(msg, send_filepath);
	cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_file_transfer_send(cbs, tester_file_transfer_send);
	linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);

	linphone_content_unref(content);
	bc_free(send_filepath);
	return msg;
}

void text_message_base(LinphoneCoreManager* marie, LinphoneCoreManager* pauline) {
	LinphoneChatRoom *room = linphone_core_get_chat_room(pauline->lc,marie->identity);
	LinphoneChatMessage* msg = linphone_chat_room_create_message(room,"Bli bli bli \n blu");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageDelivered,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
	if (marie->stat.last_received_chat_message != NULL) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(marie->stat.last_received_chat_message), "text/plain");
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc,pauline->identity));
	linphone_chat_message_unref(msg);
}

/****************************** Tests starting below ******************************/

static void text_message(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_within_call_dialog(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	lp_config_set_int(linphone_core_get_config(pauline->lc),"sip","chat_use_call_dialogs",1);

	if (BC_ASSERT_TRUE(call(marie,pauline))){
		linphone_chat_room_send_message(linphone_core_get_chat_room(pauline->lc, marie->identity),"Bla bla bla bla");

		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
		// when using call dialogs, we will never receive delivered status
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered,0,int,"%d");

		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static LinphoneAuthInfo* text_message_with_credential_from_auth_cb_auth_info;
static void text_message_with_credential_from_auth_cb_auth_info_requested(LinphoneCore *lc, const char *realm, const char *username, const char *domain) {
	ms_message("text_message_with_credential_from_auth_callback:Auth info requested  for user id [%s] at realm [%s]\n"
						,username
						,realm);
	linphone_core_add_auth_info(lc,text_message_with_credential_from_auth_cb_auth_info); /*add stored authentication info to LinphoneCore*/
}
static void text_message_with_credential_from_auth_callback(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	/*to force cb to be called*/
	text_message_with_credential_from_auth_cb_auth_info=linphone_auth_info_clone((LinphoneAuthInfo*)(linphone_core_get_auth_info_list(pauline->lc)->data));
	linphone_core_clear_all_auth_info(pauline->lc);
	linphone_core_cbs_set_auth_info_requested(cbs, text_message_with_credential_from_auth_cb_auth_info_requested);
	linphone_core_add_callbacks(pauline->lc, cbs);
	linphone_core_cbs_unref(cbs);

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_auth_info_unref(text_message_with_credential_from_auth_cb_auth_info);
	text_message_with_credential_from_auth_cb_auth_info = NULL;
}

static void text_message_with_privacy(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	linphone_proxy_config_set_privacy(linphone_core_get_default_proxy_config(pauline->lc),LinphonePrivacyId);

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_compatibility_mode(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneProxyConfig* proxy = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneAddress* proxy_address=linphone_address_new(linphone_proxy_config_get_server_addr(proxy));
	char route[256];
	char*tmp;
	/*only keep tcp*/
	LCSipTransports transport = {0,-1,0,0};
	linphone_address_clean(proxy_address);
	tmp=linphone_address_as_string_uri_only(proxy_address);
	linphone_proxy_config_set_server_addr(proxy,tmp);
	sprintf(route,"sip:%s",test_route);
	linphone_proxy_config_set_route(proxy,route);
	ms_free(tmp);
	linphone_address_unref(proxy_address);
	linphone_core_set_sip_transports(marie->lc,&transport);
	marie->stat.number_of_LinphoneRegistrationOk=0;
	BC_ASSERT_TRUE (wait_for(marie->lc,marie->lc,&marie->stat.number_of_LinphoneRegistrationOk,1));

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_with_ack(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_with_send_error(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneChatRoom* chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage* msg = linphone_chat_room_create_message(chat_room,"Bli bli bli \n blu");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);

	/*simulate a network error*/
	sal_set_send_error(linphone_core_get_sal(marie->lc), -1);
	linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	/* check transient msg list: the msg should be in it, and should be the only one */
	BC_ASSERT_EQUAL(_linphone_chat_room_get_transient_message_count(chat_room), 1, int, "%d");
	BC_ASSERT_PTR_EQUAL(_linphone_chat_room_get_first_transient_message(chat_room), msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageNotDelivered,1));
	/*BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageInProgress,1, int, "%d");*/
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageReceived,0, int, "%d");

	/* the msg should have been discarded from transient list after an error */
	BC_ASSERT_EQUAL(_linphone_chat_room_get_transient_message_count(chat_room), 0, int, "%d");

	sal_set_send_error(linphone_core_get_sal(marie->lc), 0);

	/*give a chance to register again to allow linphone_core_manager_destroy to properly unregister*/
	linphone_core_refresh_registers(marie->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneRegistrationOk,marie->stat.number_of_LinphoneRegistrationOk + 1));

	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void transfer_message_base2(LinphoneCoreManager* marie, LinphoneCoreManager* pauline, bool_t upload_error, bool_t download_error,
							bool_t use_file_body_handler_in_upload, bool_t use_file_body_handler_in_download, bool_t download_from_history) {
	char *send_filepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *receive_filepath = bc_tester_file("receive_file.dump");
	LinphoneChatRoom* chat_room;
	LinphoneChatMessage* msg;
	LinphoneChatMessageCbs *cbs;
	bctbx_list_t *msg_list = NULL;

	/* Remove any previously downloaded file */
	remove(receive_filepath);

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc,"https://www.linphone.org:444/lft.php");

	/* create a chatroom on pauline's side */
	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	/* create a file transfer msg */
	if (use_file_body_handler_in_upload) {
		msg = create_file_transfer_message_from_sintel_trailer(chat_room);
	} else {
		msg = create_message_from_sintel_trailer(chat_room);
	}

	linphone_chat_message_send(msg);

	if (upload_error) {
		int chat_room_size = 0;
		bctbx_list_t *history;

		/*wait for file to be 25% uploaded and simulate a network error*/
		BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.progress_of_LinphoneFileTransfer,25, 60000));
		/* Check that the message is already in the chat room history during file upload */
		chat_room_size = linphone_chat_room_get_history_size(chat_room);
		BC_ASSERT_EQUAL(chat_room_size, 1, int, "%d");
		if (chat_room_size == 1) {
			history = linphone_chat_room_get_history(chat_room, 0);
			LinphoneChatMessage *sent_msg = (LinphoneChatMessage *)bctbx_list_get_data(history);
			BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(sent_msg), (int)LinphoneChatMessageStateInProgress, int, "%d");
			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
		}
		sal_set_send_error(linphone_core_get_sal(pauline->lc), -1);

		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageNotDelivered,1));

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageNotDelivered,1, int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful,0, int, "%d");

		sal_set_send_error(linphone_core_get_sal(pauline->lc), 0);

		linphone_core_refresh_registers(pauline->lc); /*to make sure registration is back in registered and so it can be later unregistered*/
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneRegistrationOk,pauline->stat.number_of_LinphoneRegistrationOk+1));

		/* Check that the message is in the chat room history even if the file upload failed */
		chat_room_size = linphone_chat_room_get_history_size(chat_room);
		BC_ASSERT_EQUAL(chat_room_size, 1, int, "%d");
		if (chat_room_size == 1) {
			history = linphone_chat_room_get_history(chat_room, 0);
			LinphoneChatMessage *sent_msg = (LinphoneChatMessage *)bctbx_list_get_data(history);
			BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(sent_msg), (int)LinphoneChatMessageStateNotDelivered, int, "%d");
			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
		}
	} else {
		BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceivedWithFile,1, 60000));
		if (marie->stat.last_received_chat_message) {
			LinphoneChatRoom *marie_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
			linphone_chat_room_mark_as_read(marie_room);
			// We shoudln't get displayed IMDN until file has been downloaded
			BC_ASSERT_FALSE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageDisplayed,1, 5000));

			LinphoneChatMessage *recv_msg;
			if (download_from_history) {
				msg_list = linphone_chat_room_get_history(marie_room,1);
				BC_ASSERT_PTR_NOT_NULL(msg_list);
				if (!msg_list)  goto end;
				recv_msg = (LinphoneChatMessage *)msg_list->data;
			} else {
				recv_msg = marie->stat.last_received_chat_message;
			}
			cbs = linphone_chat_message_get_callbacks(recv_msg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
			linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
			if (use_file_body_handler_in_download) {
				linphone_chat_message_set_file_transfer_filepath(recv_msg, receive_filepath);
			}
			linphone_chat_message_download_file(recv_msg);

			if (download_error) {
				/* wait for file to be 50% downloaded */
				BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.progress_of_LinphoneFileTransfer, 50));
				/* and simulate network error */
				belle_http_provider_set_recv_error(linphone_core_get_http_provider(marie->lc), -1);
				BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneMessageNotDelivered,1, 10000));
				belle_http_provider_set_recv_error(linphone_core_get_http_provider(marie->lc), 0);
				BC_ASSERT_FALSE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageDisplayed,1, 5000));
			} else {
				/* wait for a long time in case the DNS SRV resolution takes times - it should be immediate though */
				if (BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneFileTransferDownloadSuccessful,1,55000))) {
					compare_files(send_filepath, receive_filepath);
				}
				BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageDisplayed,1, 5000));
			}
		}
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress,2, int, "%d"); //sent twice because of file transfer
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered,1, int, "%d");
	}
end:
	linphone_chat_message_unref(msg);
	bctbx_list_free_with_data(msg_list, (bctbx_list_free_func)linphone_chat_message_unref);
	remove(receive_filepath);
	bc_free(send_filepath);
	bc_free(receive_filepath);
}

void transfer_message_base(
	bool_t upload_error, bool_t download_error, bool_t use_file_body_handler_in_upload,
	bool_t use_file_body_handler_in_download, bool_t download_from_history, bool_t enable_imdn
) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
		LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

		if (enable_imdn) {
			lp_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
			lp_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
			linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
			linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
		}
		transfer_message_base2(marie,pauline,upload_error,download_error, use_file_body_handler_in_upload, use_file_body_handler_in_download, download_from_history);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}
static void transfer_message(void) {
	transfer_message_base(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE);
}

static void transfer_message_2(void) {
	transfer_message_base(FALSE, FALSE, TRUE, FALSE, FALSE, TRUE);
}

static void transfer_message_3(void) {
	transfer_message_base(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE);
}

static void transfer_message_4(void) {
	transfer_message_base(FALSE, FALSE, TRUE, TRUE, FALSE, TRUE);
}

static void transfer_message_from_history(void) {
	transfer_message_base(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE);
}

static void transfer_message_with_upload_io_error(void) {
	transfer_message_base(TRUE, FALSE, FALSE, FALSE, FALSE, TRUE);
}

static void transfer_message_with_download_io_error(void) {
	transfer_message_base(FALSE, TRUE, FALSE, FALSE, FALSE, TRUE);
}

static void transfer_message_upload_cancelled(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
		LinphoneChatRoom* chat_room;
		LinphoneChatMessage* msg;
		LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

		/* Globally configure an http file transfer server. */
		linphone_core_set_file_transfer_server(pauline->lc,"https://www.linphone.org:444/lft.php");

		/* create a chatroom on pauline's side */
		chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

		msg = create_message_from_sintel_trailer(chat_room);
		linphone_chat_message_send(msg);

		/*wait for file to be 25% uploaded and cancel the transfer */
		BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.progress_of_LinphoneFileTransfer, 25, 60000));
		linphone_chat_message_cancel_file_transfer(msg);

		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageNotDelivered,1));

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageNotDelivered,1, int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful,0, int, "%d");

		linphone_chat_message_unref(msg);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

static void transfer_message_download_cancelled(void) {
	LinphoneChatRoom* chat_room;
	LinphoneChatMessage* msg;
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc,"https://www.linphone.org:444/lft.php");

	/* create a chatroom on pauline's side */
	chat_room = linphone_core_get_chat_room(pauline->lc,marie->identity);
	msg = create_message_from_sintel_trailer(chat_room);
	linphone_chat_message_send(msg);

	/* wait for marie to receive pauline's msg */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceivedWithFile,1, 60000));

	if (marie->stat.last_received_chat_message ) { /* get last msg and use it to download file */
		LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(marie->stat.last_received_chat_message);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
		linphone_chat_message_start_file_download(marie->stat.last_received_chat_message, liblinphone_tester_chat_message_state_change, marie->lc);
		/* wait for file to be 50% downloaded */
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.progress_of_LinphoneFileTransfer, 50));
		/* and cancel the transfer */
		linphone_chat_message_cancel_file_transfer(marie->stat.last_received_chat_message);
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress,2, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered,1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful,0, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageNotDelivered,1, int, "%d");

	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void file_transfer_2_messages_simultaneously(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
		LinphoneChatRoom* pauline_room;
		LinphoneChatMessage* msg;
		LinphoneChatMessage* msg2;
		LinphoneChatMessageCbs *cbs;
		char *send_filepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
		char *receive_filepath = bc_tester_file("receive_file.dump");
		LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

		/* Remove any previously downloaded file */
		remove(receive_filepath);

		/* Globally configure an http file transfer server. */
		linphone_core_set_file_transfer_server(pauline->lc,"https://www.linphone.org:444/lft.php");

		/* create a chatroom on pauline's side */
		pauline_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
		msg = create_message_from_sintel_trailer(pauline_room);
		msg2 = create_message_from_sintel_trailer(pauline_room);

		cbs = linphone_chat_message_get_callbacks(msg2);
		linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);

		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_chat_rooms(marie->lc)), 0, unsigned int, "%u");
		if (bctbx_list_size(linphone_core_get_chat_rooms(marie->lc)) == 0) {
			linphone_chat_message_send(msg);
			linphone_chat_message_send(msg2);
			if (BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceivedWithFile,1, 60000))) {
				LinphoneChatMessage *recvMsg = linphone_chat_message_ref(marie->stat.last_received_chat_message);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceivedWithFile,2, 60000));
				LinphoneChatMessage *recvMsg2 = marie->stat.last_received_chat_message;
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_core_get_chat_rooms(marie->lc)), 1, unsigned int, "%u");
				if (bctbx_list_size(linphone_core_get_chat_rooms(marie->lc)) != 1) {
					char * buf = ms_strdup_printf("Found %d rooms instead of 1: ", bctbx_list_size(linphone_core_get_chat_rooms(marie->lc)));
					const bctbx_list_t *it = linphone_core_get_chat_rooms(marie->lc);
					while (it) {
						const LinphoneAddress * peer = linphone_chat_room_get_peer_address(it->data);
						buf = ms_strcat_printf("%s, ", linphone_address_get_username(peer));
						it = it->next;
					}
					ms_error("%s", buf);
				}

				cbs = linphone_chat_message_get_callbacks(recvMsg);
				linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
				linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
				linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
				linphone_chat_message_download_file(recvMsg);

				cbs = linphone_chat_message_get_callbacks(recvMsg2);
				linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
				linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
				linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
				linphone_chat_message_download_file(recvMsg2);

				BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneFileTransferDownloadSuccessful,2,50000));

				BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress,4, int, "%d");
				BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered,2, int, "%d");
				compare_files(send_filepath, receive_filepath);

				linphone_chat_message_unref(recvMsg);
			}
		}
		linphone_chat_message_unref(msg);
		linphone_chat_message_unref(msg2);
		linphone_core_manager_destroy(pauline);
		remove(receive_filepath);
		bc_free(send_filepath);
		bc_free(receive_filepath);
		linphone_core_manager_destroy(marie);
	}
}

static void file_transfer_external_body_url(bool_t use_file_body_handler_in_download) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneChatRoom* chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage* msg = linphone_chat_room_create_message(chat_room, NULL);
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	char *receive_filepath = bc_tester_file("receive_file.dump");

	linphone_chat_message_set_external_body_url(msg, "https://www.linphone.org/img/linphone-open-source-voip-projectX2.png");
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceivedWithFile, 1, 60000));

	if (pauline->stat.last_received_chat_message) {
		LinphoneChatMessage *recv_msg = pauline->stat.last_received_chat_message;
		cbs = linphone_chat_message_get_callbacks(recv_msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
		if (use_file_body_handler_in_download) {
			/* Remove any previously downloaded file */
			remove(receive_filepath);
			linphone_chat_message_set_file_transfer_filepath(recv_msg, receive_filepath);
		}
		linphone_chat_message_download_file(recv_msg);

		/* wait for a long time in case the DNS SRV resolution takes times - it should be immediate though */
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneFileTransferDownloadSuccessful, 1, 55000));
	}

	remove(receive_filepath);
	bc_free(receive_filepath);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void file_transfer_using_external_body_url(void) {
	file_transfer_external_body_url(FALSE);
}

static void file_transfer_using_external_body_url_2(void) {
	file_transfer_external_body_url(TRUE);
}

static void text_message_denied(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneChatRoom* chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage* msg = linphone_chat_room_create_message(chat_room,"Bli bli bli \n blu");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);

	/*pauline doesn't want to be disturbed*/
	linphone_core_disable_chat(pauline->lc,LinphoneReasonDoNotDisturb);
	linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageNotDelivered,1));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageReceived,0, int, "%d");
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static const char *info_content="<somexml>blabla</somexml>";

void info_message_received(LinphoneCore *lc, LinphoneCall* call, const LinphoneInfoMessage *msg){
	stats* counters = get_stats(lc);

	if (counters->last_received_info_message) {
		linphone_info_message_unref(counters->last_received_info_message);
	}
	counters->last_received_info_message=linphone_info_message_copy(msg);
	counters->number_of_inforeceived++;
}

void info_message_base(bool_t with_content) {
	LinphoneInfoMessage *info;
	const LinphoneContent *content;
	const char *hvalue;

	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	if (BC_ASSERT_TRUE(call(pauline,marie))){

		info=linphone_core_create_info_message(marie->lc);
		linphone_info_message_add_header(info,"Weather","still bad");
		if (with_content) {
			LinphoneContent* content = linphone_core_create_content(marie->lc);
			linphone_content_set_type(content, "application");
			linphone_content_set_subtype(content, "somexml");
			linphone_content_set_buffer(content, (const uint8_t *)info_content, strlen(info_content));
			linphone_info_message_set_content(info, content);
			linphone_content_unref(content);
		}
		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc),info);
		linphone_info_message_unref(info);

		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_inforeceived,1));

		BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_info_message);
		hvalue=linphone_info_message_get_header(pauline->stat.last_received_info_message, "Weather");
		content=linphone_info_message_get_content(pauline->stat.last_received_info_message);

		BC_ASSERT_PTR_NOT_NULL(hvalue);
		if (hvalue)
			BC_ASSERT_STRING_EQUAL(hvalue, "still bad");

		if (with_content){
			BC_ASSERT_PTR_NOT_NULL(content);
			if (content) {
				BC_ASSERT_PTR_NOT_NULL(linphone_content_get_buffer(content));
				BC_ASSERT_PTR_NOT_NULL(linphone_content_get_type(content));
				BC_ASSERT_PTR_NOT_NULL(linphone_content_get_subtype(content));
				if (linphone_content_get_type(content)) BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content),"application");
				if (linphone_content_get_subtype(content)) BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content),"somexml");
				if (linphone_content_get_buffer(content))BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content),info_content);
				BC_ASSERT_EQUAL((int)linphone_content_get_size(content),(int)strlen(info_content), int, "%d");
			}
		}
		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void info_message(void){
	info_message_base(FALSE);
}

static void info_message_with_body(void){
	info_message_base(TRUE);
}

static int enable_lime_for_message_test(LinphoneCoreManager *marie, LinphoneCoreManager *pauline) {
	char* filepath = NULL;
	char* stmt = NULL;
	char* errmsg=NULL;
	int ret = 0;
	char* paulineUri = NULL;
	char* marieUri = NULL;
	char *tmp;

	if (!linphone_core_lime_available(marie->lc) || !linphone_core_lime_available(pauline->lc)) {
		ms_warning("Lime not available, skiping");
		return -1;
	}
	/* make sure lime is enabled */
	linphone_core_enable_lime(marie->lc, LinphoneLimeMandatory);
	linphone_core_enable_lime(pauline->lc, LinphoneLimeMandatory);

	/* make sure to not trigger the cache migration function */
	lp_config_set_int(linphone_core_get_config(marie->lc), "sip", "zrtp_cache_migration_done", TRUE);
	lp_config_set_int(linphone_core_get_config(pauline->lc), "sip", "zrtp_cache_migration_done", TRUE);

	/* create temporary cache files: setting the database_path will create and initialise the files */
	tmp = bc_tester_file("tmpZIDCacheMarie.sqlite");
	remove(tmp);
	bc_free(tmp);
	tmp = bc_tester_file("tmpZIDCachePauline.sqlite");
	remove(tmp);
	bc_free(tmp);
	filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
	linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
	bc_free(filepath);
	filepath = bc_tester_file("tmpZIDCachePauline.sqlite");
	linphone_core_set_zrtp_secrets_file(pauline->lc, filepath);
	bc_free(filepath);

	/* caches are empty, populate them */
	paulineUri =  linphone_address_as_string_uri_only(pauline->identity);
	marieUri = linphone_address_as_string_uri_only(marie->identity);

	stmt = sqlite3_mprintf(marie_zid_sqlcache, marieUri, marieUri, paulineUri);
	ret = sqlite3_exec(linphone_core_get_zrtp_cache_db(marie->lc),stmt,NULL,NULL,&errmsg);
	sqlite3_free(stmt);
	if (ret != SQLITE_OK) {
		ms_warning("Lime can't populate marie's sqlite cache: %s", errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	stmt = sqlite3_mprintf(pauline_zid_sqlcache, paulineUri, paulineUri, marieUri);
	ret = sqlite3_exec(linphone_core_get_zrtp_cache_db(pauline->lc),stmt,NULL,NULL,&errmsg);
	sqlite3_free(stmt);
	if (ret != SQLITE_OK) {
		ms_warning("Lime can't populate pauline's sqlite cache: %s", errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	ms_free(paulineUri);
	ms_free(marieUri);

	return 0;
}

static void _is_composing_notification(bool_t lime_enabled) {
	LinphoneChatRoom* pauline_chat_room;
	LinphoneChatRoom* marie_chat_room;
	int dummy = 0;
	const bctbx_list_t *composing_addresses = NULL;

	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	lp_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);

	if (lime_enabled) {
		if (enable_lime_for_message_test(marie, pauline) < 0) goto end;
	}

	pauline_chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	marie_chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	linphone_core_get_chat_room(marie->lc, pauline->identity); /*make marie create the chatroom with pauline, which is necessary for receiving the is-composing*/
	linphone_chat_room_compose(pauline_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 1));
	composing_addresses = linphone_chat_room_get_composing_addresses(marie_chat_room);
	BC_ASSERT_GREATER(bctbx_list_size(composing_addresses), 0, int, "%i");
	if (bctbx_list_size(composing_addresses) > 0) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		char *address_string = linphone_address_as_string(addr);
		char *pauline_address = linphone_address_as_string(pauline->identity);
		BC_ASSERT_STRING_EQUAL(address_string, pauline_address);
		bctbx_free(address_string);
		bctbx_free(pauline_address);
	}
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	LinphoneChatMessage *is_composing_msg = marie->stat.last_received_chat_message;
	if (BC_ASSERT_PTR_NOT_NULL(is_composing_msg)) {
		const char *expires = linphone_chat_message_get_custom_header(is_composing_msg, "Expires");
		if (BC_ASSERT_PTR_NOT_NULL(expires))
			BC_ASSERT_STRING_EQUAL(expires, "0");

		const char *priority = linphone_chat_message_get_custom_header(is_composing_msg, "Priority");
		if (BC_ASSERT_PTR_NOT_NULL(priority))
			BC_ASSERT_STRING_EQUAL(priority, "non-urgent");
	}
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /*just to sleep while iterating*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 2));
	is_composing_msg = marie->stat.last_received_chat_message;
	if (BC_ASSERT_PTR_NOT_NULL(is_composing_msg)) {
		const char *expires = linphone_chat_message_get_custom_header(is_composing_msg, "Expires");
		if (BC_ASSERT_PTR_NOT_NULL(expires))
			BC_ASSERT_STRING_EQUAL(expires, "0");

		const char *priority = linphone_chat_message_get_custom_header(is_composing_msg, "Priority");
		if (BC_ASSERT_PTR_NOT_NULL(priority))
			BC_ASSERT_STRING_EQUAL(priority, "non-urgent");
	}
	linphone_chat_room_send_message(pauline_chat_room, "Composing a msg");
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 2));
	composing_addresses = linphone_chat_room_get_composing_addresses(marie_chat_room);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 0, int, "%i");

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	remove("tmpZIDCacheMarie.sqlite");
	remove("tmpZIDCachePauline.sqlite");
}

static void is_composing_notification(void) {
	_is_composing_notification(FALSE);
}

static void is_composing_notification_with_lime(void) {
	_is_composing_notification(TRUE);
}

static void _imdn_notifications(bool_t with_lime) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	lp_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
	LinphoneChatRoom *pauline_chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatRoom *marie_chat_room;
	LinphoneChatMessage *sent_cm;
	LinphoneChatMessage *received_cm;
	LinphoneChatMessageCbs *cbs;
	bctbx_list_t *history;

	if (with_lime) {
		if (enable_lime_for_message_test(marie, pauline) < 0) goto end;
	}

	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	sent_cm = linphone_chat_room_create_message(pauline_chat_room, "Tell me if you get my message");
	cbs = linphone_chat_message_get_callbacks(sent_cm);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(sent_cm);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	marie_chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	history = linphone_chat_room_get_history(marie_chat_room, 1);
	BC_ASSERT_EQUAL((int)bctbx_list_size(history), 1, int, "%d");
	if (bctbx_list_size(history) > 0) {
		received_cm = (LinphoneChatMessage *)bctbx_list_nth_data(history, 0);
		BC_ASSERT_PTR_NOT_NULL(received_cm);
		if (received_cm != NULL) {
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDeliveredToUser, 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
			LinphoneChatMessage *imdn_message = pauline->stat.last_received_chat_message;
			if (BC_ASSERT_PTR_NOT_NULL(imdn_message)) {
				const char *priority = linphone_chat_message_get_custom_header(imdn_message, "Priority");
				if (BC_ASSERT_PTR_NOT_NULL(priority))
					BC_ASSERT_STRING_EQUAL(priority, "non-urgent");
			}
			linphone_chat_room_mark_as_read(marie_chat_room); /* This sends the display notification */
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDisplayed, 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 2));
			imdn_message = pauline->stat.last_received_chat_message;
			if (BC_ASSERT_PTR_NOT_NULL(imdn_message)) {
				const char *priority = linphone_chat_message_get_custom_header(imdn_message, "Priority");
				if (BC_ASSERT_PTR_NOT_NULL(priority))
					BC_ASSERT_STRING_EQUAL(priority, "non-urgent");
			}
			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
		}
	}
	linphone_chat_message_unref(sent_cm);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	remove("tmpZIDCacheMarie.sqlite");
	remove("tmpZIDCachePauline.sqlite");
}

static void _im_notification_policy(bool_t with_lime) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneImNotifPolicy *marie_policy = linphone_core_get_im_notif_policy(marie->lc);
	LinphoneImNotifPolicy *pauline_policy = linphone_core_get_im_notif_policy(pauline->lc);
	LinphoneChatRoom *pauline_chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatRoom *marie_chat_room;
	LinphoneChatMessage *msg1;
	LinphoneChatMessage *msg2;
	LinphoneChatMessage *msg3;
	LinphoneChatMessage *msg4;
	LinphoneChatMessageCbs *cbs;
	int dummy = 0;

	if (with_lime) {
		if (enable_lime_for_message_test(marie, pauline) < 0) goto end;
	}

	linphone_im_notif_policy_enable_all(marie_policy);
	linphone_im_notif_policy_clear(pauline_policy);
	marie_chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity); /* Make marie create the chatroom with pauline, which is necessary for receiving the is-composing */

	/* Test is_composing sending */
	linphone_chat_room_compose(pauline_chat_room);
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneIsComposingActiveReceived, 0, int, "%d");
	linphone_im_notif_policy_set_send_is_composing(pauline_policy, TRUE);
	linphone_chat_room_compose(pauline_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 1));

	/* Test is_composing receiving */
	linphone_chat_room_compose(marie_chat_room);
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneIsComposingActiveReceived, 0, int, "%d");
	linphone_im_notif_policy_set_recv_is_composing(pauline_policy, TRUE);
	linphone_chat_room_compose(marie_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, 1));

	/* Test imdn delivered */
	msg1 = linphone_chat_room_create_message(pauline_chat_room, "Happy new year!");
	cbs = linphone_chat_message_get_callbacks(msg1);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg1);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDeliveredToUser, 0, int, "%d");
	linphone_im_notif_policy_set_recv_imdn_delivered(pauline_policy, TRUE);
	msg2 = linphone_chat_room_create_message(pauline_chat_room, "I said: Happy new year!");
	cbs = linphone_chat_message_get_callbacks(msg2);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg2);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDeliveredToUser, 1));
	msg3 = linphone_chat_room_create_message(marie_chat_room, "Thank you! Happy easter to you!");
	cbs = linphone_chat_message_get_callbacks(msg3);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg3);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageDeliveredToUser, 0, int, "%d");
	linphone_im_notif_policy_set_send_imdn_delivered(pauline_policy, TRUE);
	msg4 = linphone_chat_room_create_message(marie_chat_room, "Yeah, yeah, I heard that...");
	cbs = linphone_chat_message_get_callbacks(msg4);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg4);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDeliveredToUser, 1));

	/* Test imdn displayed */
	linphone_im_notif_policy_set_send_imdn_displayed(pauline_policy, TRUE);
	linphone_chat_room_mark_as_read(pauline_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDisplayed, 2));
	linphone_im_notif_policy_set_recv_imdn_displayed(pauline_policy, TRUE);
	linphone_chat_room_mark_as_read(marie_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDisplayed, 2));

	linphone_chat_message_unref(msg1);
	linphone_chat_message_unref(msg2);
	linphone_chat_message_unref(msg3);
	linphone_chat_message_unref(msg4);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	remove("tmpZIDCacheMarie.sqlite");
	remove("tmpZIDCachePauline.sqlite");
}

static void imdn_notifications(void) {
	_imdn_notifications(FALSE);
}

static void im_notification_policy(void) {
	_im_notification_policy(FALSE);
}

static void imdn_notifications_with_lime(void) {
	_imdn_notifications(TRUE);
}

static void im_notification_policy_with_lime(void) {
	_im_notification_policy(TRUE);
}

static void _im_error_delivery_notification(bool_t online) {
	LinphoneChatRoom *chat_room;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneChatMessage *msg;
	LinphoneChatMessageCbs *cbs;
	int dummy = 0;
	void *zrtp_cache_db_holder=NULL;

	if (!linphone_core_lime_available(marie->lc)) {
		ms_warning("Lime not available, skiping");
		goto end;
	}

	/* Make sure lime is enabled */
	if (enable_lime_for_message_test(marie, pauline) < 0) goto end;

	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	linphone_chat_room_send_message(chat_room, "Bla bla bla bla");
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
	if (marie->stat.last_received_chat_message) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), "Bla bla bla bla");
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc, pauline->identity));

	/* Temporary disabling receiver cache and enable all IM notifications */
	zrtp_cache_db_holder = linphone_core_get_zrtp_cache_db(marie->lc);
	linphone_core_set_zrtp_cache_db(marie->lc, NULL);
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	msg = linphone_chat_room_create_message(chat_room, "Happy new year!");
	cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);
	if (!online) {
		linphone_core_set_network_reachable(marie->lc, FALSE);
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1, 60000));
		linphone_core_set_network_reachable(marie->lc, TRUE);
		BC_ASSERT_TRUE (wait_for(marie->lc, marie->lc, &marie->stat.number_of_LinphoneRegistrationOk, 2));
		wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	}
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageReceived, 1, int, "%d"); /* Check the new message is not considered as received */
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageNotDelivered, 1));

	/* Restore the ZID cache of the receiver and resend the chat message */
	linphone_core_set_zrtp_cache_db(marie->lc, zrtp_cache_db_holder);
	linphone_chat_message_send(msg);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 2)); /* Check the new message is now received */
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDeliveredToUser, 1));
	linphone_chat_message_unref(msg);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	remove("tmpZIDCacheMarie.sqlite");
	remove("tmpZIDCachePauline.sqlite");
}

static void im_error_delivery_notification_online(void) {
	_im_error_delivery_notification(TRUE);
}

static void im_error_delivery_notification_offline(void) {
	_im_error_delivery_notification(FALSE);
}

static void lime_text_message(void) {
	LinphoneChatRoom* chat_room;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	if (!linphone_core_lime_available(marie->lc)) {
		ms_warning("Lime not available, skiping");
		goto end;
	}

	if (enable_lime_for_message_test(marie, pauline) < 0) goto end;

	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	linphone_chat_room_send_message(chat_room,"Bla bla bla bla");
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
	if (marie->stat.last_received_chat_message) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), "Bla bla bla bla");
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc,pauline->identity));
end:
	remove("tmpZIDCacheMarie.sqlite");
	remove("tmpZIDCachePauline.sqlite");
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void lime_text_message_to_non_lime(bool_t sender_policy_mandatory, bool_t lime_key_available) {
	LinphoneChatRoom* chat_room;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	if (!linphone_core_lime_available(marie->lc)) {
		ms_warning("Lime not available, skiping");
		goto end;
	}
	/* make sure lime is enabled */
	linphone_core_enable_lime(marie->lc, LinphoneLimeDisabled);
	linphone_core_enable_lime(pauline->lc, sender_policy_mandatory ? LinphoneLimeMandatory : LinphoneLimePreferred);

	if (lime_key_available) {
		/* enable lime for both parts */
		if (enable_lime_for_message_test(marie, pauline) < 0) goto end;
		/* but then disable marie */
		sqlite3_close(linphone_core_get_zrtp_cache_db(marie->lc));
		linphone_core_set_zrtp_cache_db(marie->lc, NULL);
	}

	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	linphone_chat_room_send_message(chat_room,"Bla bla bla bla");
	//since we cannot decrypt message, we should not receive any message
	if (sender_policy_mandatory || lime_key_available) {
		int chat_room_size = 0;
		BC_ASSERT_FALSE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
		BC_ASSERT_FALSE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageNotDelivered,1));
		chat_room_size = linphone_chat_room_get_history_size(chat_room);
		BC_ASSERT_EQUAL(chat_room_size, 1, int, "%d");
		if (chat_room_size == 1) {
			bctbx_list_t *history = linphone_chat_room_get_history(chat_room, 0);
			LinphoneChatMessage *sent_msg = (LinphoneChatMessage *)bctbx_list_get_data(history);
			if (lime_key_available) {
				BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(sent_msg), (int)LinphoneChatMessageStateDelivered, int, "%d");
			} else {
				BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(sent_msg), (int)LinphoneChatMessageStateNotDelivered, int, "%d");
			}
			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
		}
	} else {
		BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc,pauline->identity));
end:
	remove("tmpZIDCachePauline.sqlite");
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void lime_text_message_to_non_lime_mandatory_policy(void) {
	lime_text_message_to_non_lime(TRUE, TRUE);
}

static void lime_text_message_to_non_lime_preferred_policy(void) {
	lime_text_message_to_non_lime(FALSE, TRUE);
}

static void lime_text_message_to_non_lime_preferred_policy_2(void) {
	lime_text_message_to_non_lime(FALSE, FALSE);
}

static void lime_text_message_without_cache(void) {
	lime_text_message_to_non_lime(TRUE, FALSE);
}

static void lime_multiple_messages_while_network_unreachable(void) {
	LinphoneChatRoom* chat_room;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	if (!linphone_core_lime_available(marie->lc)) {
		ms_warning("Lime not available, skiping");
		goto end;
	}

	if (enable_lime_for_message_test(marie, pauline) < 0) goto end;

	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	linphone_chat_room_send_message(chat_room,"Bla bla 1");
	linphone_chat_room_send_message(chat_room,"Bla bla 2");
	linphone_core_set_network_reachable(pauline->lc, TRUE);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,2));
	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
	if (marie->stat.last_received_chat_message) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), "Bla bla 2");
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc,pauline->identity));
end:
	remove("tmpZIDCacheMarie.sqlite");
	remove("tmpZIDCachePauline.sqlite");
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void lime_transfer_message_base(bool_t encrypt_file,bool_t download_file_from_stored_msg, bool_t use_file_body_handler_in_upload, bool_t use_file_body_handler_in_download) {
	LinphoneCoreManager *marie, *pauline;
	LinphoneChatMessage *msg;
	LinphoneChatMessageCbs *cbs;
	char *send_filepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *receive_filepath = bc_tester_file("receive_file.dump");
	MSList * msg_list = NULL;

	/* Remove any previously downloaded file */
	remove(receive_filepath);

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new( "pauline_tcp_rc");

	if (!linphone_core_lime_available(marie->lc)) {
		ms_warning("Lime not available, skiping");
		goto end;
	}
	/* make sure lime is enabled */
	enable_lime_for_message_test(marie, pauline);

	if (!encrypt_file) {
		LpConfig *pauline_lp = linphone_core_get_config(pauline->lc);
		lp_config_set_int(pauline_lp, "sip", "lime_for_file_sharing", 0);
	}
	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc,"https://www.linphone.org:444/lft.php");

	/* create a file transfer msg */
	if (use_file_body_handler_in_upload) {
		msg = create_file_transfer_message_from_sintel_trailer(linphone_core_get_chat_room(pauline->lc, marie->identity));
	} else {
		msg = create_message_from_sintel_trailer(linphone_core_get_chat_room(pauline->lc, marie->identity));
	}

	linphone_chat_message_send(msg);
	BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceivedWithFile,1, 60000));
	if (marie->stat.last_received_chat_message ) {
		LinphoneChatMessage *recv_msg;
		LinphoneContent* content;
		if (download_file_from_stored_msg) {
			LinphoneChatRoom *marie_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
			msg_list = linphone_chat_room_get_history(marie_room,1);
			BC_ASSERT_PTR_NOT_NULL(msg_list);
			if (!msg_list)  goto end;
			recv_msg = (LinphoneChatMessage *)msg_list->data;
		} else {
			recv_msg = marie->stat.last_received_chat_message;
		}
		cbs = linphone_chat_message_get_callbacks(recv_msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
		content = linphone_chat_message_get_file_transfer_information(recv_msg);
		if (!content) goto end;
		if (encrypt_file)
			BC_ASSERT_PTR_NOT_NULL(linphone_content_get_key(content));
		else
			BC_ASSERT_PTR_NULL(linphone_content_get_key(content));

		if (use_file_body_handler_in_download) {
			linphone_chat_message_set_file_transfer_filepath(recv_msg, receive_filepath);
		}
		linphone_chat_message_download_file(recv_msg);

		if (BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneFileTransferDownloadSuccessful,1,55000))) {
			compare_files(send_filepath, receive_filepath);
		}
		bctbx_list_free_with_data(msg_list, (bctbx_list_free_func)linphone_chat_message_unref);
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress,2, int, "%d"); // file transfer
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered,1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful,1, int, "%d");
	linphone_chat_message_unref(msg);

end:
	remove("tmpZIDCacheMarie.sqlite");
	remove("tmpZIDCachePauline.sqlite");
	remove(receive_filepath);
	bc_free(send_filepath);
	bc_free(receive_filepath);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static  void lime_transfer_message(void) {
	lime_transfer_message_base(TRUE, FALSE, FALSE, FALSE);
}

static  void lime_transfer_message_2(void) {
	lime_transfer_message_base(TRUE, FALSE, TRUE, FALSE);
}

static  void lime_transfer_message_3(void) {
	lime_transfer_message_base(TRUE, FALSE, FALSE, TRUE);
}

static  void lime_transfer_message_4(void) {
	lime_transfer_message_base(TRUE, FALSE, TRUE, TRUE);
}

static  void lime_transfer_message_from_history(void) {
	lime_transfer_message_base(TRUE, TRUE, FALSE, FALSE);
}

static  void lime_transfer_message_without_encryption(void) {
	lime_transfer_message_base(FALSE, FALSE, FALSE, FALSE);
}

static  void lime_transfer_message_without_encryption_2(void) {
	lime_transfer_message_base(FALSE, FALSE, TRUE, FALSE);
}

static void lime_cache_migration(void) {
	if (lime_is_available()) {
		char *xmlCache_filepath = bc_tester_file("tmp_zidCacheMigration");
		FILE *xmlCacheFD = NULL;
		/* create the temporary cache xml file, it will be turned to sqlite */
		if ((xmlCacheFD = fopen(xmlCache_filepath, "w") ) == NULL) {
			BC_ASSERT_PTR_NOT_NULL(xmlCacheFD);
			ms_error("Unable to create temporary XML ZID cache file to test cache migration");
			goto end2;
		}
		fprintf(xmlCacheFD, "%s", xmlCacheMigration);
		fclose(xmlCacheFD);

		LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
		LinphoneProxyConfig *cfg = linphone_core_get_default_proxy_config(marie->lc);
		LinphoneAddress *new_identity = linphone_address_clone(linphone_proxy_config_get_identity_address(cfg));
		linphone_proxy_config_edit(cfg);
		linphone_address_set_display_name(new_identity,"what about if we have a display name ?");
		linphone_proxy_config_set_identity_address(cfg, new_identity);

		linphone_proxy_config_done(cfg);

		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2, 5000));

		if (!linphone_core_lime_available(marie->lc)) {
			ms_warning("Lime not available, skiping");
			goto end1;
		}

		/* make sure lime is enabled */
		linphone_core_enable_lime(marie->lc, LinphoneLimeMandatory);

		/* make sure to trigger the cache migration function */
		lp_config_set_int(linphone_core_get_config(marie->lc), "sip", "zrtp_cache_migration_done", FALSE);

		/* set the cache path, it will trigger the migration function */
		linphone_core_set_zrtp_secrets_file(marie->lc, xmlCache_filepath);
		/*short check*/
		limeKey_t associatedKey={0};

		char * selfURI = linphone_address_as_string_uri_only(new_identity);
		linphone_address_unref(new_identity);
		bctbx_str_to_uint8(associatedKey.peerZID, (const uint8_t *)"0987654321fedcba5a5a5a5a", (uint16_t)strlen("0987654321fedcba5a5a5a5a"));
		/* 0987654321fedcba5a5a5a5a is the only one with pvs=1*/
		BC_ASSERT_FALSE(lime_getCachedRcvKeyByZid(linphone_core_get_zrtp_cache_db(marie->lc), &associatedKey, selfURI, "sip:bob@sip.linphone.org"));
		ms_free(selfURI);
		/* perform checks on the new cache, simple check is ok as deeper ones are performed in the bzrtp migration tester */
		/* TODO */

		/* free memory */

	end1:
		linphone_core_manager_destroy(marie);
	end2:
		remove(xmlCache_filepath);
		bc_free(xmlCache_filepath);
	}
}

static void lime_unit(void) {
	if (lime_is_available()) {
		LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
		LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

		BC_ASSERT_EQUAL(enable_lime_for_message_test(marie, pauline), 0, int, "%d");

		linphone_core_manager_destroy(marie);
		linphone_core_manager_destroy(pauline);
		/* TODO: perform more elaborate testing */
	}
}

/*
 * Copy file "from" to file "to".
 * Destination file is truncated if existing.
 * Return 0 on success, positive value on error.
 */
static int message_tester_copy_file(const char *from, const char *to)
{
	FILE *in, *out;
	char buf[256];
	size_t n;

	/* Open "from" file for reading */
	in=fopen(from, "rb");
	if ( in == NULL )
	{
		ms_error("Can't open %s for reading: %s\n",from,strerror(errno));
		return 1;
	}

	/* Open "to" file for writing (will truncate existing files) */
	out=fopen(to, "wb");
	if ( out == NULL )
	{
		ms_error("Can't open %s for writing: %s\n",to,strerror(errno));
		fclose(in);
		return 2;
	}

	/* Copy data from "in" to "out" */
	while ( (n=fread(buf, sizeof(char), sizeof(buf), in)) > 0 )
	{
		if ( ! fwrite(buf, 1, n, out) )
		{
			ms_error("Could not write in %s: %s\n",to,strerror(errno));
			fclose(in);
			fclose(out);
			return 3;
		}
	}

	fclose(in);
	fclose(out);

	return 0;
}

int check_no_strange_time(void* data,int argc, char** argv,char** cNames) {
	BC_ASSERT_EQUAL(argc, 1, int, "%d");
	BC_ASSERT_STRING_EQUAL(cNames[0], "COUNT(*)"); // count of non updated messages should be 0
	BC_ASSERT_STRING_EQUAL(argv[0], "0"); // count of non updated messages should be 0
	return 0;
}

void history_message_count_helper(LinphoneChatRoom* chatroom, int x, int y, unsigned int expected ){
	bctbx_list_t* messages = linphone_chat_room_get_history_range(chatroom, x, y);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(messages), expected, unsigned int, "%u");
	bctbx_list_free_with_data(messages, (void (*)(void *))linphone_chat_message_unref);
}

void crash_during_file_transfer(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneChatRoom *chat_room;
	LinphoneChatMessage *msg;
	int chat_room_size = 0;
	bctbx_list_t *msg_list = NULL;

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc, "https://www.linphone.org:444/lft.php");

	/* Create a chatroom and a file transfer message on pauline's side */
	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	msg = create_file_transfer_message_from_sintel_trailer(chat_room);
	linphone_chat_message_send(msg);

	/* Wait for 25% of the file to be uploaded and crash by stopping the iteration, saving the chat database and destroying the core */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.progress_of_LinphoneFileTransfer, 25, 60000));
	linphone_chat_message_unref(msg);
	linphone_core_manager_stop(pauline);

	/* Create a new core and check that the message stored in the saved database is in the not delivered state */
	linphone_core_manager_restart(pauline, TRUE);
	linphone_core_set_file_transfer_server(pauline->lc, "https://www.linphone.org:444/lft.php");

	//BC_ASSERT_TRUE(wait_for(pauline->lc, pauline->lc, &pauline->stat.number_of_LinphoneRegistrationOk, 1));

	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	chat_room_size = linphone_chat_room_get_history_size(chat_room);
	BC_ASSERT_EQUAL(chat_room_size, 1, int, "%d");
	if (chat_room_size == 1) {
		msg_list = linphone_chat_room_get_history(chat_room, 0);
		LinphoneChatMessage *sent_msg = (LinphoneChatMessage *)bctbx_list_get_data(msg_list);
		BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(sent_msg), (int)LinphoneChatMessageStateNotDelivered, int, "%d");
		//resend
		LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(sent_msg);
		linphone_chat_message_cbs_set_file_transfer_send(cbs, tester_file_transfer_send);
		linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
		linphone_chat_message_send(sent_msg);
		if (BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceivedWithFile,1, 60000))) {
			linphone_core_manager_stop(marie);
			/* Create a new core and check that the message stored in the saved database is in the not delivered state */
			linphone_core_manager_restart(marie, TRUE);
			LinphoneChatRoom *marie_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
			bctbx_list_t *msg_list_2 = linphone_chat_room_get_history(marie_room,1);
			BC_ASSERT_PTR_NOT_NULL(msg_list_2);
			LinphoneChatMessage *recv_msg = (LinphoneChatMessage *)msg_list_2->data;
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(recv_msg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
			linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
			linphone_chat_message_download_file(recv_msg);
			/* wait for a long time in case the DNS SRV resolution takes times - it should be immediate though */
			if (BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneFileTransferDownloadSuccessful,1,55000))) {
				BC_ASSERT_PTR_NULL(linphone_chat_message_get_external_body_url(recv_msg));
			}
			bctbx_list_free_with_data(msg_list_2, (bctbx_list_free_func)linphone_chat_message_unref);
		}
	}


	bctbx_list_free_with_data(msg_list, (bctbx_list_free_func)linphone_chat_message_unref);


	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void text_status_after_destroying_chat_room(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneChatRoom *chatroom = linphone_core_get_chat_room_from_uri(marie->lc, "<sip:Jehan@sip.linphone.org>");
	LinphoneChatMessage *msg = linphone_chat_room_create_message(chatroom, "hello");
	linphone_chat_message_send(msg);
	linphone_core_delete_chat_room(marie->lc, chatroom);
	//since message is orphan, we do not expect to be notified of state change
	BC_ASSERT_FALSE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageNotDelivered, 1, 1000));
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
}


static void file_transfer_not_sent_if_invalid_url(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneChatRoom *chatroom = linphone_core_get_chat_room_from_uri(marie->lc, "<sip:Jehan@sip.linphone.org>");
	LinphoneChatMessage *msg = create_message_from_sintel_trailer(chatroom);
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);
	linphone_core_set_file_transfer_server(marie->lc, "INVALID URL");
	linphone_chat_message_send(msg);
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageNotDelivered, 1, 1000));
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
}

void file_transfer_io_error_base(char *server_url, bool_t destroy_room) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneChatRoom *chatroom = linphone_core_get_chat_room_from_uri(marie->lc, "<sip:Jehan@sip.linphone.org>");
	LinphoneChatMessage *msg = create_message_from_sintel_trailer(chatroom);
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);
	linphone_core_set_file_transfer_server(marie->lc, server_url);
	linphone_chat_message_send(msg);
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageInProgress, 1, 1000));
	if (destroy_room) {
		linphone_core_delete_chat_room(marie->lc, chatroom);
		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageNotDelivered, 1, 1000));
	} else {
		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageNotDelivered, 1, 3000));
	}
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
}

static void file_transfer_not_sent_if_host_not_found(void) {
	file_transfer_io_error_base("https://not-existing-url.com", FALSE);
}

static void file_transfer_not_sent_if_url_moved_permanently(void) {
	file_transfer_io_error_base("http://linphone.org/toto.php", FALSE);
}

static void file_transfer_io_error_after_destroying_chatroom(void) {
	file_transfer_io_error_base("https://www.linphone.org:444/lft.php", TRUE);
}

static void real_time_text(
	bool_t audio_stream_enabled, bool_t srtp_enabled, bool_t mess_with_marie_payload_number,
	bool_t mess_with_pauline_payload_number, bool_t ice_enabled, bool_t sql_storage,
	bool_t do_not_store_rtt_messages_in_sql_storage
) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL;
	LinphoneCall *pauline_call, *marie_call;
	char *marie_db  = bc_tester_file("marie.db");
	char *pauline_db  = bc_tester_file("pauline.db");

	if (sql_storage) {
		linphone_core_set_chat_database_path(marie->lc, marie_db);
		linphone_core_set_chat_database_path(pauline->lc, pauline_db);

		if (do_not_store_rtt_messages_in_sql_storage) {
			lp_config_set_int(linphone_core_get_config(marie->lc), "misc", "store_rtt_messages", 0);
			lp_config_set_int(linphone_core_get_config(pauline->lc), "misc", "store_rtt_messages", 0);
		}
	}

	if (mess_with_marie_payload_number) {
		const bctbx_list_t *elem;
		for (elem = linphone_core_get_text_payload_types(marie->lc); elem != NULL; elem = elem->next) {
			PayloadType *pt = (PayloadType*)elem->data;
			if (strcasecmp(pt->mime_type, payload_type_t140.mime_type) == 0) {
				payload_type_set_number(pt, 99);
				break;
			}
		}
	} else if (mess_with_pauline_payload_number) {
		const bctbx_list_t *elem;
		for (elem = linphone_core_get_text_payload_types(pauline->lc); elem != NULL; elem = elem->next) {
			PayloadType *pt = (PayloadType*)elem->data;
			if (strcasecmp(pt->mime_type, payload_type_t140.mime_type) == 0) {
				payload_type_set_number(pt, 99);
				break;
			}
		}
	}

	if (ice_enabled) {
		linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	}

	if (srtp_enabled) {
		if (!ms_srtp_supported()) {
			ms_warning("test skipped, missing srtp support");
			goto srtp_end;
		}
		BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionSRTP));
		linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
		linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);
		linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);
	}

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params,TRUE);
	if (!audio_stream_enabled) {
		linphone_call_params_enable_audio(marie_params,FALSE);
		linphone_core_set_nortp_timeout(marie->lc, 5);
		linphone_core_set_nortp_timeout(pauline->lc, 5);
	}

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	if (pauline_call) {
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));
		if (audio_stream_enabled) {
			BC_ASSERT_TRUE(linphone_call_params_audio_enabled(linphone_call_get_current_params(pauline_call)));
		}

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		if (pauline_chat_room) {
			const char* message = "Be l3l";
			size_t i;
			LinphoneChatMessage* rtt_message = linphone_chat_room_create_message(pauline_chat_room,NULL);
			LinphoneChatRoom *marie_chat_room = linphone_call_get_chat_room(marie_call);

			for (i = 0; i < strlen(message); i++) {
				BC_ASSERT_FALSE(linphone_chat_message_put_char(rtt_message, message[i]));
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i+1, 1000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i], char, "%c");
			}
			linphone_chat_message_send(rtt_message);
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			linphone_chat_message_unref(rtt_message);

			if (sql_storage) {
				bctbx_list_t *marie_messages = linphone_chat_room_get_history(marie_chat_room, 0);
				bctbx_list_t *pauline_messages = linphone_chat_room_get_history(pauline_chat_room, 0);
				LinphoneChatMessage *marie_msg = NULL;
				LinphoneChatMessage *pauline_msg = NULL;
				if (do_not_store_rtt_messages_in_sql_storage) {
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(marie_messages), 0, unsigned int , "%u");
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(pauline_messages), 0, unsigned int , "%u");
				} else {
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(marie_messages), 1, unsigned int , "%u");
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(pauline_messages), 1, unsigned int , "%u");
					if (!marie_messages || !pauline_messages) {
						goto end;
					}
					marie_msg = (LinphoneChatMessage *)marie_messages->data;
					pauline_msg = (LinphoneChatMessage *)pauline_messages->data;
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie_msg), message);
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline_msg), message);
					bctbx_list_free_with_data(marie_messages, (void (*)(void *))linphone_chat_message_unref);
					bctbx_list_free_with_data(pauline_messages, (void (*)(void *))linphone_chat_message_unref);
				}
			}
		}

		if (!audio_stream_enabled) {
			int dummy = 0;
			wait_for_until(pauline->lc, marie->lc, &dummy, 1, 7000); /* Wait to see if call is dropped after the nortp_timeout */
			BC_ASSERT_FALSE(marie->stat.number_of_LinphoneCallEnd > 0);
			BC_ASSERT_FALSE(pauline->stat.number_of_LinphoneCallEnd > 0);
		}

		if (ice_enabled) {
			BC_ASSERT_TRUE(check_ice(pauline,marie,LinphoneIceStateHostConnection));
		}

end:
		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
srtp_end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	remove(marie_db);
	bc_free(marie_db);
	remove(pauline_db);
	bc_free(pauline_db);
}

static void real_time_text_message(void) {
	real_time_text(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
}

static void real_time_text_sql_storage(void) {
	real_time_text(TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE);
}

static void real_time_text_sql_storage_rtt_disabled(void) {
	real_time_text(TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE);
}

static void real_time_text_conversation(void) {
	LinphoneChatRoom *pauline_chat_room, *marie_chat_room;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCall *pauline_call, *marie_call;
	linphone_call_params_enable_realtime_text(marie_params,TRUE);

	if (BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params))){
		pauline_call=linphone_core_get_current_call(pauline->lc);
		marie_call=linphone_core_get_current_call(marie->lc);
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		marie_chat_room = linphone_call_get_chat_room(marie_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		if (pauline_chat_room && marie_chat_room) {
			const char* message1_1 = "Lorem";
			const char* message1_2 = "Ipsum";
			const char* message2_1 = "Be lle Com";
			const char* message2_2 = "eB ell moC";
			size_t i;
			LinphoneChatMessage* pauline_rtt_message = linphone_chat_room_create_message(pauline_chat_room,NULL);
			LinphoneChatMessage* marie_rtt_message = linphone_chat_room_create_message(marie_chat_room,NULL);

			for (i = 0; i < strlen(message1_1); i++) {
				linphone_chat_message_put_char(pauline_rtt_message, message1_1[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i+1, 1000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message1_1[i], char, "%c");

				linphone_chat_message_put_char(marie_rtt_message, message1_2[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, (int)i+1, 1000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(pauline_chat_room), message1_2[i], char, "%c");
			}

			/*Commit the message, triggers a NEW LINE in T.140 */
			linphone_chat_message_send(pauline_rtt_message);
			linphone_chat_message_send(marie_rtt_message);

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			{
				LinphoneChatMessage * msg = marie->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), message1_1);
				}
			}
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
			{
				LinphoneChatMessage * msg = pauline->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), message1_2);
				}
			}

			linphone_chat_message_unref(pauline_rtt_message);
			linphone_chat_message_unref(marie_rtt_message);
			reset_counters(&pauline->stat);
			reset_counters(&marie->stat);
			pauline_rtt_message = linphone_chat_room_create_message(pauline_chat_room,NULL);
			marie_rtt_message = linphone_chat_room_create_message(marie_chat_room,NULL);

			for (i = 0; i < strlen(message2_1); i++) {
				linphone_chat_message_put_char(pauline_rtt_message, message2_1[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i+1, 1000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message2_1[i], char, "%c");

				linphone_chat_message_put_char(marie_rtt_message, message2_2[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, (int)i+1, 1000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(pauline_chat_room), message2_2[i], char, "%c");
			}

			/*Commit the message, triggers a NEW LINE in T.140 */
			linphone_chat_message_send(pauline_rtt_message);
			linphone_chat_message_send(marie_rtt_message);

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			{
				LinphoneChatMessage * msg = marie->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), message2_1);
				}
			}
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
			{
				LinphoneChatMessage * msg = pauline->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), message2_2);
				}
			}
			linphone_chat_message_unref(pauline_rtt_message);
			linphone_chat_message_unref(marie_rtt_message);
		}
		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void real_time_text_without_audio(void) {
	real_time_text(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
}

static void real_time_text_srtp(void) {
	real_time_text(TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
}

static void real_time_text_ice(void) {
	real_time_text(TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE);
}

static void real_time_text_message_compat(bool_t end_with_crlf, bool_t end_with_lf) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL;
	LinphoneCall *pauline_call, *marie_call;

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params,TRUE);

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	pauline_call=linphone_core_get_current_call(pauline->lc);
	marie_call=linphone_core_get_current_call(marie->lc);
	if (pauline_call) {
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		if (pauline_chat_room) {
			const char* message = "Be l3l";
			size_t i;
			LinphoneChatMessage* rtt_message = linphone_chat_room_create_message(pauline_chat_room,NULL);
			LinphoneChatRoom *marie_chat_room = linphone_call_get_chat_room(marie_call);
			uint32_t crlf = 0x0D0A;
			uint32_t lf = 0x0A;

			for (i = 0; i < strlen(message); i++) {
				linphone_chat_message_put_char(rtt_message, message[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i+1, 1000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i], char, "%c");
			}

			if (end_with_crlf) {
				linphone_chat_message_put_char(rtt_message, crlf);
			} else if (end_with_lf) {
				linphone_chat_message_put_char(rtt_message, lf);
			}
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)strlen(message), 1000));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			linphone_chat_message_unref(rtt_message);
		}
		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void real_time_text_message_compat_crlf(void) {
	real_time_text_message_compat(TRUE, FALSE);
}

static void real_time_text_message_compat_lf(void) {
	real_time_text_message_compat(FALSE, TRUE);
}

static void real_time_text_message_accented_chars(void) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL;
	LinphoneCall *pauline_call, *marie_call;

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params,TRUE);

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	pauline_call=linphone_core_get_current_call(pauline->lc);
	marie_call=linphone_core_get_current_call(marie->lc);
	if (pauline_call) {
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		if (pauline_chat_room) {
			LinphoneChatMessage* rtt_message = linphone_chat_room_create_message(pauline_chat_room,NULL);
			LinphoneChatRoom *marie_chat_room = linphone_call_get_chat_room(marie_call);
			int i;
			uint32_t message[8];
			int message_len = 8;

			message[0] = 0xE3; // ã
			message[1] = 0xE6; // æ
			message[2] = 0xE7; // ç
			message[3] = 0xE9; // é
			message[4] = 0xEE; // î
			message[5] = 0xF8; // ø
			message[6] = 0xF9; // ù
			message[7] = 0xFF; // ÿ
			for (i = 0; i < message_len; i++) {
				linphone_chat_message_put_char(rtt_message, message[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, i+1, 1000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i], unsigned long, "%lu");
			}

			linphone_chat_message_send(rtt_message);
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
			if (marie->stat.last_received_chat_message) {
				const char *text = linphone_chat_message_get_text(marie->stat.last_received_chat_message);
				BC_ASSERT_PTR_NOT_NULL(text);
				if (text)
					BC_ASSERT_STRING_EQUAL(text, "ãæçéîøùÿ");
			}
			linphone_chat_message_unref(rtt_message);
		}
		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void real_time_text_message_different_text_codecs_payload_numbers_sender_side(void) {
	real_time_text(FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE);
}

static void real_time_text_message_different_text_codecs_payload_numbers_receiver_side(void) {
	real_time_text(FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE);
}

static void real_time_text_copy_paste(void) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL;
	LinphoneCall *pauline_call, *marie_call;

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params,TRUE);

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	if (pauline_call) {
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		if (pauline_chat_room) {
			const char* message = "Be l3l";
			size_t i;
			LinphoneChatMessage* rtt_message = linphone_chat_room_create_message(pauline_chat_room,NULL);
			LinphoneChatRoom *marie_chat_room = linphone_call_get_chat_room(marie_call);

			for (i = 1; i <= strlen(message); i++) {
				linphone_chat_message_put_char(rtt_message, message[i-1]);
				if (i % 4 == 0) {
					int j;
					BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i, 1000));
					for (j = 4; j > 0; j--) {
						BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i-j], char, "%c");
					}
				}
			}
			linphone_chat_message_send(rtt_message);
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			linphone_chat_message_unref(rtt_message);
		}

		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void file_transfer_with_http_proxy(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
		LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
		lp_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
		lp_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
		linphone_core_set_http_proxy_host(marie->lc, "sip.linphone.org");
		transfer_message_base2(marie,pauline,FALSE,FALSE,FALSE,FALSE,FALSE);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

void chat_message_custom_headers(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new( "marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneChatRoom* chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatMessage* msg = linphone_chat_room_create_message(chat_room, "Lorem Ipsum");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);

	linphone_chat_message_add_custom_header(msg, "Test1", "Value1");
	linphone_chat_message_add_custom_header(msg, "Test2", "Value2");
	linphone_chat_message_remove_custom_header(msg, "Test1");

	linphone_chat_message_cbs_set_msg_state_changed(cbs,liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageDelivered,1));

	if (marie->stat.last_received_chat_message) {
		const char *header = linphone_chat_message_get_custom_header(marie->stat.last_received_chat_message, "Test2");
		BC_ASSERT_STRING_EQUAL(header, "Value2");
		header = linphone_chat_message_get_custom_header(marie->stat.last_received_chat_message, "Test1");
		BC_ASSERT_PTR_NULL(header);
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), "Lorem Ipsum");
	}

	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void _text_message_with_custom_content_type(bool_t with_lime) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatMessage *msg;
	LinphoneChatMessageCbs *cbs;
	bctbx_vfs_t *vfs = bctbx_vfs_get_default();
	char *send_filepath;
	bctbx_vfs_file_t *file_to_send;
	size_t file_size;
	char *buf;

	linphone_core_add_content_type_support(marie->lc, "image/svg+xml");
	linphone_core_add_content_type_support(pauline->lc, "image/svg+xml");

	if (with_lime) {
		if (enable_lime_for_message_test(marie, pauline) < 0) goto end;
	}

	send_filepath = bc_tester_res("images/linphone.svg");
	file_to_send = bctbx_file_open(vfs, send_filepath, "r");
	file_size = (size_t)bctbx_file_size(file_to_send);
	buf = bctbx_malloc(file_size + 1);
	bctbx_file_read(file_to_send, buf, file_size, 0);
	buf[file_size] = '\0';
	bctbx_file_close(file_to_send);
	bc_free(send_filepath);
	msg = linphone_chat_room_create_message(chat_room, buf);
	linphone_chat_message_set_content_type(msg, "image/svg+xml");

	cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1));

	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(msg), "image/svg+xml");
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), buf);

	if (marie->stat.last_received_chat_message) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(marie->stat.last_received_chat_message), "image/svg+xml");
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), buf);
	}

	bctbx_free(buf);
	linphone_chat_message_unref(msg);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	remove("tmpZIDCacheMarie.sqlite");
	remove("tmpZIDCachePauline.sqlite");
}

void text_message_with_custom_content_type(void) {
	_text_message_with_custom_content_type(FALSE);
}

void text_message_with_custom_content_type_and_lime(void) {
	_text_message_with_custom_content_type(TRUE);
}


static int im_encryption_engine_process_incoming_message_cb(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room, LinphoneChatMessage *msg) {
	ms_debug("IM encryption process incoming message with content type %s", linphone_chat_message_get_content_type(msg));
	if (linphone_chat_message_get_content_type(msg)) {
		if (strcmp(linphone_chat_message_get_content_type(msg), "cipher/b64") == 0) {
			size_t b64Size = 0;
			unsigned char *output;
			const char *data = linphone_chat_message_get_text(msg);
			ms_debug("IM encryption process incoming message crypted message is %s", data);
			bctbx_base64_decode(NULL, &b64Size, (unsigned char *)data, strlen(data));
			output = (unsigned char *)ms_malloc(b64Size+1),
			bctbx_base64_decode(output, &b64Size, (unsigned char *)data, strlen(data));
			output[b64Size] = '\0';
			linphone_chat_message_set_text(msg, (char *)output);
			ms_debug("IM encryption process incoming message decrypted message is %s", output);
			ms_free(output);
			linphone_chat_message_set_content_type(msg, "text/plain");
			return 0;
		} else if (strcmp(linphone_chat_message_get_content_type(msg), "text/plain") == 0) {
			return -1; // Not encrypted, nothing to do
		} else {
			return 488; // Not acceptable
		}
	}
	return 500;
}

static bool_t im_encryption_engine_process_incoming_message_async_impl(LinphoneChatMessage** msg) {
	if (*msg) {
		im_encryption_engine_process_incoming_message_cb(NULL,NULL,*msg);
		linphone_chat_room_receive_chat_message(linphone_chat_message_get_chat_room(*msg), *msg);
		linphone_chat_message_unref(*msg);
		*msg=NULL;
	}
	return TRUE;
}

static int im_encryption_engine_process_outgoing_message_cb(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room, LinphoneChatMessage *msg) {
	if (strcmp(linphone_chat_message_get_content_type(msg),"text/plain") == 0) {
		size_t b64Size = 0;
		unsigned char *output;
		bctbx_base64_encode(NULL, &b64Size, (unsigned char *)linphone_chat_message_get_text(msg), strlen(linphone_chat_message_get_text(msg)));
		output = (unsigned char *)ms_malloc0(b64Size+1),
		bctbx_base64_encode(output, &b64Size, (unsigned char *)linphone_chat_message_get_text(msg), strlen(linphone_chat_message_get_text(msg)));
		output[b64Size] = '\0';
		linphone_chat_message_set_text(msg,(const char*)output);
		ms_free(output);
		linphone_chat_message_set_content_type(msg, "cipher/b64");
		return 0;
	}
	return -1;
}

static bool_t im_encryption_engine_process_outgoing_message_async_impl(LinphoneChatMessage** msg) {
	if (*msg) {
		im_encryption_engine_process_outgoing_message_cb(NULL,NULL,*msg);
		linphone_chat_message_send(*msg);
		linphone_chat_message_unref(*msg);
		*msg=NULL;
	}
	return TRUE;
}

static LinphoneChatMessage* pending_message=NULL; /*limited to one message at a time */
static LinphoneChatMessage* incoming_pending_message=NULL; /*limited to one message at a time */

static int im_encryption_engine_process_outgoing_message_async(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room, LinphoneChatMessage *msg) {
	pending_message=msg;
	linphone_chat_message_ref(pending_message);
	linphone_core_add_iterate_hook(linphone_chat_room_get_core(room), (LinphoneCoreIterateHook)im_encryption_engine_process_outgoing_message_async_impl,&pending_message);
	return 1;/*temporaly code to defer message sending*/
}

static int im_encryption_engine_process_incoming_message_async(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room, LinphoneChatMessage *msg) {
	incoming_pending_message=msg;
	linphone_chat_message_ref(incoming_pending_message);
	linphone_core_add_iterate_hook(linphone_chat_room_get_core(room), (LinphoneCoreIterateHook)im_encryption_engine_process_incoming_message_async_impl,&incoming_pending_message);
	return 1;/*temporaly code to defer message receiving*/
}

void im_encryption_engine_b64_base(bool_t async) {
	LinphoneChatMessage *chat_msg = NULL;
	LinphoneChatRoom* chat_room = NULL;
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneImEncryptionEngine *marie_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *marie_cbs = linphone_im_encryption_engine_get_callbacks(marie_imee);
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneImEncryptionEngine *pauline_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *pauline_cbs = linphone_im_encryption_engine_get_callbacks(pauline_imee);

	linphone_im_encryption_engine_cbs_set_process_outgoing_message(marie_cbs, im_encryption_engine_process_outgoing_message_cb);
	linphone_im_encryption_engine_cbs_set_process_incoming_message(pauline_cbs, im_encryption_engine_process_incoming_message_cb);
	if (async) {
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(pauline_cbs, im_encryption_engine_process_outgoing_message_async);
		linphone_im_encryption_engine_cbs_set_process_incoming_message(marie_cbs, im_encryption_engine_process_incoming_message_async);
	} else {
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(pauline_cbs, im_encryption_engine_process_outgoing_message_cb);
		linphone_im_encryption_engine_cbs_set_process_incoming_message(marie_cbs, im_encryption_engine_process_incoming_message_cb);
	}

	linphone_core_set_im_encryption_engine(marie->lc, marie_imee);
	linphone_core_set_im_encryption_engine(pauline->lc, pauline_imee);

	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	chat_msg = linphone_chat_room_create_message(chat_room, "Bla bla bla bla");
	linphone_chat_message_send(chat_msg);
	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageReceived,1));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(chat_msg), "Bla bla bla bla");
	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
	if (marie->stat.last_received_chat_message) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), "Bla bla bla bla");
	}
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc,pauline->identity));

	linphone_chat_message_unref(chat_msg);
	linphone_im_encryption_engine_unref(marie_imee);
	linphone_im_encryption_engine_unref(pauline_imee);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void im_encryption_engine_b64(void) {
	im_encryption_engine_b64_base(FALSE);
}

void im_encryption_engine_b64_async(void) {
	im_encryption_engine_b64_base(TRUE);
}

void unread_message_count(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");

	text_message_base(marie, pauline);

	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
	if (marie->stat.last_received_chat_message != NULL) {
		LinphoneChatRoom *marie_room = linphone_chat_message_get_chat_room(marie->stat.last_received_chat_message);
		BC_ASSERT_EQUAL(1, linphone_chat_room_get_unread_messages_count(marie_room), int, "%d");
		linphone_chat_room_mark_as_read(marie_room);
		BC_ASSERT_EQUAL(0, linphone_chat_room_get_unread_messages_count(marie_room), int, "%d");
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void message_received_callback(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage* msg) {
	BC_ASSERT_PTR_NOT_NULL(room);
	BC_ASSERT_EQUAL(1, linphone_chat_room_get_unread_messages_count(room), int, "%d");
	BC_ASSERT_PTR_NOT_NULL(msg);
	if (room != NULL) {
		linphone_chat_room_mark_as_read(room);
	}
	BC_ASSERT_EQUAL(0, linphone_chat_room_get_unread_messages_count(room), int, "%d");
}

void unread_message_count_callback(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	int dummy = 0;

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_message_received(cbs, message_received_callback);
	linphone_core_add_callbacks(marie->lc, cbs);

	text_message_base(marie, pauline);

	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 5000);

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void migration_from_messages_db (void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	char *src_db = bc_tester_res("db/messages.db");
	char *tmp_db  = bc_tester_file("tmp.db");

	BC_ASSERT_EQUAL(message_tester_copy_file(src_db, tmp_db), 0, int, "%d");

	// The messages.db has 10000 dummy messages with the very first DB scheme.
	// This will test the migration procedure
	linphone_core_set_chat_database_path(marie->lc, tmp_db);

	const bctbx_list_t *chatrooms = linphone_core_get_chat_rooms(marie->lc);
	BC_ASSERT(bctbx_list_size(chatrooms) > 0);

	linphone_core_manager_destroy(marie);
	remove(tmp_db);
	bctbx_free(src_db);
	bctbx_free(tmp_db);
}

test_t message_tests[] = {
	TEST_NO_TAG("Text message", text_message),
	TEST_NO_TAG("Text message with credentials from auth callback", text_message_with_credential_from_auth_callback),
	TEST_NO_TAG("Text message with privacy", text_message_with_privacy),
	TEST_NO_TAG("Text message compatibility mode", text_message_compatibility_mode),
	TEST_NO_TAG("Text message with ack", text_message_with_ack),
	TEST_NO_TAG("Text message with send error", text_message_with_send_error),
	TEST_NO_TAG("Transfer message", transfer_message),
	TEST_NO_TAG("Transfer message 2", transfer_message_2),
	TEST_NO_TAG("Transfer message 3", transfer_message_3),
	TEST_NO_TAG("Transfer message 4", transfer_message_4),
	TEST_NO_TAG("Transfer message from history", transfer_message_from_history),
	TEST_NO_TAG("Transfer message with http proxy", file_transfer_with_http_proxy),
	TEST_NO_TAG("Transfer message with upload io error", transfer_message_with_upload_io_error),
	TEST_NO_TAG("Transfer message with download io error", transfer_message_with_download_io_error),
	TEST_NO_TAG("Transfer message upload cancelled", transfer_message_upload_cancelled),
	TEST_NO_TAG("Transfer message download cancelled", transfer_message_download_cancelled),
	TEST_NO_TAG("Transfer 2 messages simultaneously", file_transfer_2_messages_simultaneously),
	TEST_NO_TAG("Transfer using external body URL", file_transfer_using_external_body_url),
	TEST_NO_TAG("Transfer using external body URL 2", file_transfer_using_external_body_url_2),
	TEST_NO_TAG("Text message denied", text_message_denied),
	TEST_NO_TAG("IsComposing notification", is_composing_notification),
	TEST_NO_TAG("IMDN notifications", imdn_notifications),
	TEST_NO_TAG("IM notification policy", im_notification_policy),
	TEST_NO_TAG("Unread message count", unread_message_count),
	TEST_NO_TAG("Unread message count in callback", unread_message_count_callback),
	TEST_ONE_TAG("IsComposing notification lime", is_composing_notification_with_lime, "LIME"),
	TEST_ONE_TAG("IMDN notifications with lime", imdn_notifications_with_lime, "LIME"),
	TEST_ONE_TAG("IM notification policy with lime", im_notification_policy_with_lime, "LIME"),
	TEST_ONE_TAG("IM error delivery notification online", im_error_delivery_notification_online, "LIME"),
	TEST_ONE_TAG("IM error delivery notification offline", im_error_delivery_notification_offline, "LIME"),
	TEST_ONE_TAG("Lime text message", lime_text_message, "LIME"),
	TEST_ONE_TAG("Lime text message to non lime", lime_text_message_to_non_lime_mandatory_policy, "LIME"),
	TEST_ONE_TAG("Lime text message to non lime with preferred policy", lime_text_message_to_non_lime_preferred_policy, "LIME"),
	TEST_ONE_TAG("Lime text message to non lime with preferred policy 2", lime_text_message_to_non_lime_preferred_policy_2, "LIME"),
	TEST_ONE_TAG("Lime text message without cache", lime_text_message_without_cache, "LIME"),
	TEST_ONE_TAG("Lime multiple messages while network wasn't reachable", lime_multiple_messages_while_network_unreachable, "LIME"),
	TEST_ONE_TAG("Lime transfer message", lime_transfer_message, "LIME"),
	TEST_ONE_TAG("Lime transfer message 2", lime_transfer_message_2, "LIME"),
	TEST_ONE_TAG("Lime transfer message 3", lime_transfer_message_3, "LIME"),
	TEST_ONE_TAG("Lime transfer message 4", lime_transfer_message_4, "LIME"),
	TEST_ONE_TAG("Lime transfer message from history", lime_transfer_message_from_history, "LIME"),
	TEST_ONE_TAG("Lime transfer message without encryption", lime_transfer_message_without_encryption, "LIME"),
	TEST_ONE_TAG("Lime transfer message without encryption 2", lime_transfer_message_without_encryption_2, "LIME"),
	TEST_ONE_TAG("Lime cache migration", lime_cache_migration, "LIME"),
	TEST_ONE_TAG("Lime unitary", lime_unit, "LIME"),
	TEST_NO_TAG("Transfer not sent if invalid url", file_transfer_not_sent_if_invalid_url),
	TEST_NO_TAG("Transfer not sent if host not found", file_transfer_not_sent_if_host_not_found),
	TEST_NO_TAG("Transfer not sent if url moved permanently", file_transfer_not_sent_if_url_moved_permanently),
	TEST_ONE_TAG("Real Time Text message", real_time_text_message, "RTT"),
	TEST_ONE_TAG("Real Time Text SQL storage", real_time_text_sql_storage, "RTT"),
	TEST_ONE_TAG("Real Time Text SQL storage with RTT messages not stored", real_time_text_sql_storage_rtt_disabled, "RTT"),
	TEST_ONE_TAG("Real Time Text conversation", real_time_text_conversation, "RTT"),
	TEST_ONE_TAG("Real Time Text without audio", real_time_text_without_audio, "RTT"),
	TEST_ONE_TAG("Real Time Text with srtp", real_time_text_srtp, "RTT"),
	TEST_ONE_TAG("Real Time Text with ice", real_time_text_ice, "RTT"),
	TEST_ONE_TAG("Real Time Text message compatibility crlf", real_time_text_message_compat_crlf, "RTT"),
	TEST_ONE_TAG("Real Time Text message compatibility lf", real_time_text_message_compat_lf, "RTT"),
	TEST_ONE_TAG("Real Time Text message with accented characters", real_time_text_message_accented_chars, "RTT"),
	TEST_ONE_TAG("Real Time Text offer answer with different payload numbers (sender side)", real_time_text_message_different_text_codecs_payload_numbers_sender_side, "RTT"),
	TEST_ONE_TAG("Real Time Text offer answer with different payload numbers (receiver side)", real_time_text_message_different_text_codecs_payload_numbers_receiver_side, "RTT"),
	TEST_ONE_TAG("Real Time Text copy paste", real_time_text_copy_paste, "RTT"),
	TEST_NO_TAG("IM Encryption Engine custom headers", chat_message_custom_headers),
	TEST_NO_TAG("Text message with custom content-type", text_message_with_custom_content_type),
	TEST_ONE_TAG("Text message with custom content-type and lime", text_message_with_custom_content_type_and_lime, "LIME"),
	TEST_NO_TAG("IM Encryption Engine b64", im_encryption_engine_b64),
	TEST_NO_TAG("IM Encryption Engine b64 async", im_encryption_engine_b64_async),
	TEST_NO_TAG("Text message within call dialog", text_message_within_call_dialog),
	TEST_NO_TAG("Info message", info_message),
	TEST_NO_TAG("Info message with body", info_message_with_body),
	TEST_NO_TAG("Crash during file transfer", crash_during_file_transfer),
	TEST_NO_TAG("Text status after destroying chat room", text_status_after_destroying_chat_room),
	TEST_NO_TAG("Transfer io error after destroying chatroom", file_transfer_io_error_after_destroying_chatroom),
	TEST_NO_TAG("Migration from messages db", migration_from_messages_db)
};

static int message_tester_before_suite(void) {
	//liblinphone_tester_keep_uuid = TRUE;
	return 0;
}

static int message_tester_after_suite(void) {
	//liblinphone_tester_keep_uuid = FALSE;
	return 0;
}

test_suite_t message_test_suite = {
	"Message",
	message_tester_before_suite,
	message_tester_after_suite,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(message_tests) / sizeof(message_tests[0]), message_tests
};
