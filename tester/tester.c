 /*
 tester - liblinphone test suite
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

#include <stdio.h>
#include <stdlib.h>
#include "linphone/core.h"
#include "linphone/logging.h"
#include "logging-private.h"
#include "liblinphone_tester.h"
#include <bctoolbox/tester.h>
#include "tester_utils.h"

#if _WIN32
#define unlink _unlink
#endif

#ifdef __ANDROID__
extern jobject system_context;
#else
void *system_context=0;
#endif

static int liblinphone_tester_keep_accounts_flag = 0;
static bool_t liblinphone_tester_keep_record_files = FALSE;
static bool_t liblinphone_tester_leak_detector_disabled = FALSE;
bool_t liblinphone_tester_keep_uuid = FALSE;
bool_t liblinphone_tester_tls_support_disabled = FALSE;
int manager_count = 0;
int leaked_objects_count = 0;
const MSAudioDiffParams audio_cmp_params = {10,2000};

const char* test_domain="sipopen.example.org";
const char* auth_domain="sip.example.org";
const char* test_username="liblinphone_tester";
const char* test_sha_username="liblinphone_sha_tester";
const char* test_password="secret";
const char* test_route="sip2.linphone.org";
const char *userhostsfile = "tester_hosts";
bool_t liblinphonetester_ipv6 = TRUE;
bool_t liblinphonetester_show_account_manager_logs = FALSE;
int liblinphonetester_transport_timeout = 9000; /*milliseconds. it is set to such low value to workaround a problem with our Freebox v6 when connecting to Ipv6 addresses.
			It was found that the freebox sometimes block SYN-ACK packets, which prevents connection to be succesful.
			Thanks to the timeout, it will fallback to IPv4*/

const char *liblinphone_tester_mire_id="Mire: Mire (synthetic moving picture)";
const char *liblinphone_tester_static_image_id="StaticImage: Static picture";

static void network_reachable(LinphoneCore *lc, bool_t reachable) {
	stats* counters;
	ms_message("Network reachable [%s]",reachable?"TRUE":"FALSE");
	counters = get_stats(lc);
	if (reachable)
		counters->number_of_NetworkReachableTrue++;
	else
		counters->number_of_NetworkReachableFalse++;
}
void liblinphone_tester_clock_start(MSTimeSpec *start){
	ms_get_cur_time(start);
}

bool_t liblinphone_tester_clock_elapsed(const MSTimeSpec *start, int value_ms){
	MSTimeSpec current;
	ms_get_cur_time(&current);
	if ((((current.tv_sec-start->tv_sec)*1000LL) + ((current.tv_nsec-start->tv_nsec)/1000000LL))>=value_ms)
		return TRUE;
	return FALSE;
}


LinphoneAddress * create_linphone_address(const char * domain) {
	return create_linphone_address_for_algo(domain,NULL);
}

LinphoneAddress * create_linphone_address_for_algo(const char * domain, const char* username) {
	LinphoneAddress *addr = linphone_address_new(NULL);
	if (!BC_ASSERT_PTR_NOT_NULL(addr)) return NULL;
	/* For clients who support different algorithms, their usernames must be differnet for having diffrent forms of password */
	if (username) linphone_address_set_username(addr,username);
	else linphone_address_set_username(addr,test_username);
	if (username) BC_ASSERT_STRING_EQUAL(username, linphone_address_get_username(addr));
	else BC_ASSERT_STRING_EQUAL(test_username, linphone_address_get_username(addr));
	if (!domain) domain = test_route;
	linphone_address_set_domain(addr,domain);
	BC_ASSERT_STRING_EQUAL(domain, linphone_address_get_domain(addr));
	linphone_address_set_display_name(addr, NULL);
	linphone_address_set_display_name(addr, "Mr Tester");
	BC_ASSERT_STRING_EQUAL("Mr Tester", linphone_address_get_display_name(addr));
	return addr;
}

static void auth_info_requested(LinphoneCore *lc, const char *realm, const char *username, const char *domain) {
	stats* counters;
	ms_message("Auth info requested  for user id [%s] at realm [%s]\n", username, realm);
	counters = get_stats(lc);
	counters->number_of_auth_info_requested++;
}

void reset_counters( stats* counters) {
	if (counters->last_received_chat_message) linphone_chat_message_unref(counters->last_received_chat_message);
	if (counters->last_received_info_message) linphone_info_message_unref(counters->last_received_info_message);
	if (counters->dtmf_list_received) bctbx_free(counters->dtmf_list_received);

	memset(counters,0,sizeof(stats));
}

LinphoneCore *configure_lc_from(LinphoneCoreCbs *cbs, const char *path, const char *file, void *user_data) {
	LinphoneCore *lc;
	LinphoneConfig *config = NULL;
	char *filepath         = NULL;
	char *ringpath         = NULL;
	char *ringbackpath     = NULL;
	char *rootcapath       = NULL;
	char *dnsuserhostspath = NULL;
	char *nowebcampath     = NULL;

	if (!path)
		path = ".";

	if (file){
		filepath = bctbx_strdup_printf("%s/%s", path, file);
		if (bctbx_file_exist(filepath) != 0) {
			ms_fatal("Could not find file %s in path %s, did you configured resources directory correctly?", file, path);
		}
		config = lp_config_new_with_factory(NULL, filepath);
	}

	// setup dynamic-path assets
	ringpath         = ms_strdup_printf("%s/sounds/oldphone.wav",path);
	ringbackpath     = ms_strdup_printf("%s/sounds/ringback.wav", path);
	nowebcampath     = ms_strdup_printf("%s/images/nowebcamCIF.jpg", path);
	rootcapath       = ms_strdup_printf("%s/certificates/cn/cafile.pem", path);
	dnsuserhostspath = userhostsfile[0]=='/' ? ms_strdup(userhostsfile) : ms_strdup_printf("%s/%s", path, userhostsfile);

	if (config) {
		lp_config_set_string(config, "sound", "remote_ring", ringbackpath);
		lp_config_set_string(config, "sound", "local_ring" , ringpath);
		lp_config_set_string(config, "sip",   "root_ca"    , rootcapath);
		lc = linphone_factory_create_core_with_config_3(linphone_factory_get(), config, system_context);
	} else {
		lc = linphone_factory_create_core_3(linphone_factory_get(), NULL, (filepath && (filepath[0] != '\0')) ? filepath : NULL, system_context);
		linphone_core_set_ring(lc, ringpath);
		linphone_core_set_ringback(lc, ringbackpath);
		linphone_core_set_root_ca(lc,rootcapath);
	}
	linphone_core_set_user_data(lc, user_data);
	if (cbs)
		linphone_core_add_callbacks(lc, cbs);

	linphone_core_enable_ipv6(lc, liblinphonetester_ipv6);
	linphone_core_set_sip_transport_timeout(lc, liblinphonetester_transport_timeout);

	sal_enable_test_features(linphone_core_get_sal(lc),TRUE);
	sal_set_dns_user_hosts_file(linphone_core_get_sal(lc), dnsuserhostspath);
#ifdef VIDEO_ENABLED
	linphone_core_set_static_picture(lc,nowebcampath);
#endif

	ms_free(ringpath);
	ms_free(ringbackpath);
	ms_free(nowebcampath);
	ms_free(rootcapath);
	ms_free(dnsuserhostspath);

	if (filepath)
		bctbx_free(filepath);

	if (config)
		linphone_config_unref(config);

	return lc;
}


bool_t wait_for_until(LinphoneCore* lc_1, LinphoneCore* lc_2,int* counter,int value,int timout) {
	bctbx_list_t* lcs=NULL;
	bool_t result;
	if (lc_1)
		lcs=bctbx_list_append(lcs,lc_1);
	if (lc_2)
		lcs=bctbx_list_append(lcs,lc_2);
	result=wait_for_list(lcs,counter,value,timout);
	bctbx_list_free(lcs);
	return result;
}

bool_t wait_for(LinphoneCore* lc_1, LinphoneCore* lc_2,int* counter,int value) {
	return wait_for_until(lc_1, lc_2,counter,value,10000);
}

bool_t wait_for_list(bctbx_list_t* lcs,int* counter,int value,int timeout_ms) {
	bctbx_list_t* iterator;
	MSTimeSpec start;

	liblinphone_tester_clock_start(&start);
	while ((counter==NULL || *counter<value) && !liblinphone_tester_clock_elapsed(&start,timeout_ms)) {
		for (iterator=lcs;iterator!=NULL;iterator=iterator->next) {
			linphone_core_iterate((LinphoneCore*)(iterator->data));
		}
#ifdef LINPHONE_WINDOWS_DESKTOP
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0,1)){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
#endif
		ms_usleep(20000);
	}
	if(counter && *counter<value) return FALSE;
	else return TRUE;
}

bool_t wait_for_stun_resolution(LinphoneCoreManager *m) {
	MSTimeSpec start;
	int timeout_ms = 10000;
	liblinphone_tester_clock_start(&start);
	while (linphone_core_get_stun_server_addrinfo(m->lc) == NULL && !liblinphone_tester_clock_elapsed(&start,timeout_ms)) {
		linphone_core_iterate(m->lc);
		ms_usleep(20000);
	}
	return linphone_core_get_stun_server_addrinfo(m->lc) != NULL;
}

static void set_codec_enable(LinphoneCore* lc,const char* type,int rate,bool_t enable) {
	bctbx_list_t* codecs=bctbx_list_copy(linphone_core_get_audio_codecs(lc));
	bctbx_list_t* codecs_it;
	PayloadType* pt;
	for (codecs_it=codecs;codecs_it!=NULL;codecs_it=codecs_it->next) {
		linphone_core_enable_payload_type(lc,(PayloadType*)codecs_it->data,0);
	}
	if ((pt = linphone_core_find_payload_type(lc,type,rate,1))) {
		linphone_core_enable_payload_type(lc,pt, enable);
	}
	bctbx_list_free(codecs);
}

static void enable_codec(LinphoneCore* lc,const char* type,int rate) {
	set_codec_enable(lc,type,rate,TRUE);
}
stats * get_stats(LinphoneCore *lc){
	LinphoneCoreManager *manager=(LinphoneCoreManager *)linphone_core_get_user_data(lc);
	return &manager->stat;
}

LinphoneCoreManager *get_manager(LinphoneCore *lc){
	LinphoneCoreManager *manager=(LinphoneCoreManager *)linphone_core_get_user_data(lc);
	return manager;
}

bool_t transport_supported(LinphoneTransportType transport) {
	if ((transport == LinphoneTransportDtls || transport == LinphoneTransportTls) && liblinphone_tester_tls_support_disabled == TRUE) {
		return FALSE;
	} else {
		Sal *sal = sal_init(NULL);
		bool_t supported = sal_transport_available(sal,(SalTransport)transport);
		if (!supported) ms_message("TLS transport not supported, falling back to TCP if possible otherwise skipping test.");
		sal_uninit(sal);
		return  supported;
	}
}

void linphone_core_manager_configure (LinphoneCoreManager *mgr) {
	LinphoneImNotifPolicy *im_notif_policy;
	char *hellopath = bc_tester_res("sounds/hello8000.wav");

	mgr->lc = configure_lc_from(mgr->cbs, bc_tester_get_resource_dir_prefix(), mgr->rc_path, mgr);
	linphone_core_manager_check_accounts(mgr);
	im_notif_policy = linphone_core_get_im_notif_policy(mgr->lc);
	if (im_notif_policy != NULL) {
		/* The IM notification policy can be NULL at this point in case of remote provisioning. */
		linphone_im_notif_policy_clear(im_notif_policy);
		linphone_im_notif_policy_set_send_is_composing(im_notif_policy, TRUE);
		linphone_im_notif_policy_set_recv_is_composing(im_notif_policy, TRUE);
	}

#if TARGET_OS_IPHONE
	linphone_core_set_ringer_device( mgr->lc, "AQ: Audio Queue Device");
	linphone_core_set_ringback(mgr->lc, NULL);
#elif __QNX__
	linphone_core_set_playback_device(mgr->lc, "QSA: voice");
#endif

#ifdef VIDEO_ENABLED
	{
		MSWebCam *cam;

		cam = ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(linphone_core_get_ms_factory(mgr->lc)), "Mire: Mire (synthetic moving picture)");

		if (cam == NULL) {
			MSWebCamDesc *desc = ms_mire_webcam_desc_get();
			if (desc){
				cam=ms_web_cam_new(desc);
				ms_web_cam_manager_add_cam(ms_factory_get_web_cam_manager(linphone_core_get_ms_factory(mgr->lc)), cam);
			}
		}
	}
#endif


	linphone_core_set_play_file(mgr->lc,hellopath); /*is also used when in pause*/
	ms_free(hellopath);

	if( manager_count >= 2){
		char *recordpath = ms_strdup_printf("%s/record_for_lc_%p.wav",bc_tester_get_writable_dir_prefix(),mgr->lc);
		ms_message("Manager for '%s' using files", mgr->rc_path ? mgr->rc_path : "--");
		linphone_core_set_use_files(mgr->lc, TRUE);
		linphone_core_set_record_file(mgr->lc,recordpath);
		ms_free(recordpath);
	}

	linphone_core_set_user_certificates_path(mgr->lc,bc_tester_get_writable_dir_prefix());
	/*for now, we need the periodical updates facility to compute bandwidth measurements correctly during tests*/
	linphone_core_enable_send_call_stats_periodical_updates(mgr->lc, TRUE);

	LinphoneConfig *config = linphone_core_get_config(mgr->lc);
	linphone_config_set_string(config, "storage", "backend", "sqlite3");
	linphone_config_set_string(config, "storage", "uri", mgr->database_path);
}

static void generate_random_database_path (LinphoneCoreManager *mgr) {
	char random_id[32];
	belle_sip_random_token(random_id, sizeof random_id);
	char *database_path_format = bctbx_strdup_printf("linphone_%s.db", random_id);
	mgr->database_path = bc_tester_file(database_path_format);
	bctbx_free(database_path_format);
}

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
void linphone_core_manager_init(LinphoneCoreManager *mgr, const char* rc_file, const char* phone_alias) {
	mgr->number_of_bcunit_error_at_creation =  bc_get_number_of_failures();
	mgr->cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_registration_state_changed(mgr->cbs, registration_state_changed);
	linphone_core_cbs_set_auth_info_requested(mgr->cbs, auth_info_requested);
	linphone_core_cbs_set_call_state_changed(mgr->cbs, call_state_changed);
	linphone_core_cbs_set_message_received(mgr->cbs, message_received);
	linphone_core_cbs_set_is_composing_received(mgr->cbs, is_composing_received);
	linphone_core_cbs_set_new_subscription_requested(mgr->cbs, new_subscription_requested);
	linphone_core_cbs_set_notify_presence_received(mgr->cbs, notify_presence_received);
	linphone_core_cbs_set_notify_presence_received_for_uri_or_tel(mgr->cbs, notify_presence_received_for_uri_or_tel);
	linphone_core_cbs_set_transfer_state_changed(mgr->cbs, linphone_transfer_state_changed);
	linphone_core_cbs_set_info_received(mgr->cbs, info_message_received);
	linphone_core_cbs_set_subscription_state_changed(mgr->cbs, linphone_subscription_state_change);
	linphone_core_cbs_set_notify_received(mgr->cbs, linphone_notify_received);
	linphone_core_cbs_set_subscribe_received(mgr->cbs, linphone_subscribe_received);
	linphone_core_cbs_set_publish_state_changed(mgr->cbs, linphone_publish_state_changed);
	linphone_core_cbs_set_configuring_status(mgr->cbs, linphone_configuration_status);
	linphone_core_cbs_set_call_encryption_changed(mgr->cbs, linphone_call_encryption_changed);
	linphone_core_cbs_set_network_reachable(mgr->cbs, network_reachable);
	linphone_core_cbs_set_dtmf_received(mgr->cbs, dtmf_received);
	linphone_core_cbs_set_call_stats_updated(mgr->cbs, call_stats_updated);

	mgr->phone_alias = phone_alias ? ms_strdup(phone_alias) : NULL;

	reset_counters(&mgr->stat);
	if (rc_file) mgr->rc_path = ms_strdup_printf("rcfiles/%s", rc_file);

	manager_count++;

	generate_random_database_path(mgr);
	linphone_core_manager_configure(mgr);
}
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif

void linphone_core_manager_start(LinphoneCoreManager *mgr, bool_t check_for_proxies) {
	LinphoneProxyConfig* proxy;
	int proxy_count;

	linphone_core_start(mgr->lc);

	/*BC_ASSERT_EQUAL(bctbx_list_size(linphone_core_get_proxy_config_list(lc)),proxy_count, int, "%d");*/
	if (check_for_proxies){ /**/
		proxy_count=(int)bctbx_list_size(linphone_core_get_proxy_config_list(mgr->lc));
	}else{
		proxy_count=0;
		/*this is to prevent registration to go on*/
		linphone_core_set_network_reachable(mgr->lc, FALSE);
	}

	if (proxy_count){
#define REGISTER_TIMEOUT 20 /* seconds */
		int success = wait_for_until(mgr->lc,NULL,&mgr->stat.number_of_LinphoneRegistrationOk,
									proxy_count,(REGISTER_TIMEOUT * 1000 * proxy_count));
		if( !success ){
			ms_error("Did not register after %d seconds for %d proxies", REGISTER_TIMEOUT, proxy_count);
		}
	}
	BC_ASSERT_EQUAL(mgr->stat.number_of_LinphoneRegistrationOk,proxy_count, int, "%d");
	enable_codec(mgr->lc,"PCMU",8000);

	proxy = linphone_core_get_default_proxy_config(mgr->lc);
	if (proxy) {
		if (mgr->identity){
			linphone_address_unref(mgr->identity);
		}
		mgr->identity = linphone_address_clone(linphone_proxy_config_get_identity_address(proxy));
		linphone_address_clean(mgr->identity);
	}

	linphone_core_manager_wait_for_stun_resolution(mgr);
	if (!check_for_proxies){
		/*now that stun server resolution is done, we can start registering*/
		linphone_core_set_network_reachable(mgr->lc, TRUE);
	}

}

LinphoneCoreManager* linphone_core_manager_create2(const char* rc_file, const char* phone_alias) {
	LinphoneCoreManager *manager = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(manager, rc_file, phone_alias);
	return manager;
}

LinphoneCoreManager* linphone_core_manager_create(const char* rc_file) {
	return linphone_core_manager_create2(rc_file, NULL);
}

LinphoneCoreManager* linphone_core_manager_new4(const char* rc_file, int check_for_proxies, const char* phone_alias, const char* contact_params, int expires) {
	/* This function is for testing purposes. */
	LinphoneCoreManager *manager = ms_new0(LinphoneCoreManager, 1);

	linphone_core_manager_init(manager, rc_file, phone_alias);
	linphone_proxy_config_set_contact_parameters(linphone_core_get_default_proxy_config(manager->lc), contact_params);
	linphone_proxy_config_set_expires(linphone_core_get_default_proxy_config(manager->lc), expires);
	linphone_core_manager_start(manager, check_for_proxies);
	return manager;
}

LinphoneCoreManager* linphone_core_manager_new3(const char* rc_file, bool_t check_for_proxies, const char* phone_alias) {
	LinphoneCoreManager *manager = linphone_core_manager_create2(rc_file, phone_alias);
	linphone_core_manager_start(manager, check_for_proxies);
	return manager;
}

LinphoneCoreManager* linphone_core_manager_new2(const char* rc_file, bool_t check_for_proxies) {
	return linphone_core_manager_new3(rc_file, check_for_proxies, NULL);
}

LinphoneCoreManager* linphone_core_manager_new( const char* rc_file) {
	return linphone_core_manager_new2(rc_file, TRUE);
}


void linphone_core_manager_stop(LinphoneCoreManager *mgr){
	if (mgr->lc) {
		const char *record_file = linphone_core_get_record_file(mgr->lc);
		if (!liblinphone_tester_keep_record_files && record_file && ortp_file_exist(record_file)==0) {
			if ((bc_get_number_of_failures() - mgr->number_of_bcunit_error_at_creation)>0) {
				ms_error("Test has failed, keeping recorded file [%s]", record_file);
			}
			else {
				unlink(record_file);
			}
		}
		linphone_core_unref(mgr->lc);
		mgr->lc = NULL;
	}
}

void linphone_core_manager_reinit(LinphoneCoreManager *mgr) {
	char *uuid = NULL;
	if (mgr->lc) {
		if (lp_config_get_string(linphone_core_get_config(mgr->lc), "misc", "uuid", NULL))
			uuid = bctbx_strdup(lp_config_get_string(linphone_core_get_config(mgr->lc), "misc", "uuid", NULL));
		linphone_core_unref(mgr->lc);
	}
	linphone_core_manager_configure(mgr);
	reset_counters(&mgr->stat);
	// Make sure gruu is preserved
	lp_config_set_string(linphone_core_get_config(mgr->lc), "misc", "uuid", uuid);
	if (uuid)
		bctbx_free(uuid);
}

void linphone_core_manager_restart(LinphoneCoreManager *mgr, bool_t check_for_proxies) {
	linphone_core_manager_reinit(mgr);
	linphone_core_manager_start(mgr, check_for_proxies);
}

void linphone_core_manager_uninit(LinphoneCoreManager *mgr) {
	int old_log_level = linphone_core_get_log_level_mask();
	linphone_core_set_log_level(ORTP_ERROR);
	if (mgr->phone_alias) {
		ms_free(mgr->phone_alias);
	}
	if (mgr->identity) {
		linphone_address_unref(mgr->identity);
	}
	if (mgr->rc_path)
		bctbx_free(mgr->rc_path);
	if (mgr->database_path) {
		unlink(mgr->database_path);
		bctbx_free(mgr->database_path);
	}

	if (mgr->cbs)
		linphone_core_cbs_unref(mgr->cbs);

	reset_counters(&mgr->stat);

	manager_count--;
	linphone_core_set_log_level_mask(old_log_level);
}

void linphone_core_manager_wait_for_stun_resolution(LinphoneCoreManager *mgr) {
	LinphoneNatPolicy *nat_policy = linphone_core_get_nat_policy(mgr->lc);
	if ((nat_policy != NULL) && (linphone_nat_policy_get_stun_server(nat_policy) != NULL) &&
		(linphone_nat_policy_stun_enabled(nat_policy) || linphone_nat_policy_turn_enabled(nat_policy)) &&
		(linphone_nat_policy_ice_enabled(nat_policy))) {
		/*before we go, ensure that the stun server is resolved, otherwise all ice related test will fail*/
		BC_ASSERT_TRUE(wait_for_stun_resolution(mgr));
	}
}

void linphone_core_manager_destroy(LinphoneCoreManager* mgr) {
	if (mgr->lc && !linphone_core_is_network_reachable(mgr->lc)) {
		int previousNbRegistrationOk = mgr->stat.number_of_LinphoneRegistrationOk;
		linphone_core_set_network_reachable(mgr->lc, TRUE);
		wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_LinphoneRegistrationOk, previousNbRegistrationOk + 1, 2000);
	}
	linphone_core_manager_stop(mgr);
	linphone_core_manager_uninit(mgr);
	ms_free(mgr);
}

void linphone_core_manager_delete_chat_room (LinphoneCoreManager *mgr, LinphoneChatRoom *cr, bctbx_list_t *coresList) {
	stats mgrStats = mgr->stat;
	linphone_core_delete_chat_room(mgr->lc, cr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateDeleted, mgrStats.number_of_LinphoneChatRoomStateDeleted + 1, 10000));
}

int liblinphone_tester_ipv6_available(void){
	if (liblinphonetester_ipv6) {
		struct addrinfo *ai=bctbx_ip_address_to_addrinfo(AF_INET6,SOCK_STREAM,"2a01:e00::2",53);
		if (ai){
			struct sockaddr_storage ss;
			struct addrinfo src;
			socklen_t slen=sizeof(ss);
			char localip[128];
			int port=0;
			belle_sip_get_src_addr_for(ai->ai_addr,(socklen_t)ai->ai_addrlen,(struct sockaddr*) &ss,&slen,4444);
			src.ai_addr=(struct sockaddr*) &ss;
			src.ai_addrlen=slen;
			bctbx_addrinfo_to_ip_address(&src,localip, sizeof(localip),&port);
			freeaddrinfo(ai);
			return strcmp(localip,"::1")!=0;
		}
	}
	return FALSE;
}

int liblinphone_tester_ipv4_available(void){
	struct addrinfo *ai=bctbx_ip_address_to_addrinfo(AF_INET,SOCK_STREAM,"212.27.40.240",53);
	if (ai){
		struct sockaddr_storage ss;
		struct addrinfo src;
		socklen_t slen=sizeof(ss);
		char localip[128];
		int port=0;
		belle_sip_get_src_addr_for(ai->ai_addr,(socklen_t)ai->ai_addrlen,(struct sockaddr*) &ss,&slen,4444);
		src.ai_addr=(struct sockaddr*) &ss;
		src.ai_addrlen=slen;
		bctbx_addrinfo_to_ip_address(&src,localip, sizeof(localip),&port);
		freeaddrinfo(ai);
		return strcmp(localip,"127.0.0.1")!=0;
	}
	return FALSE;
}


void liblinphone_tester_keep_accounts( int keep ){
	liblinphone_tester_keep_accounts_flag = keep;
}

void liblinphone_tester_keep_recorded_files(int keep){
	liblinphone_tester_keep_record_files = keep;
}

void liblinphone_tester_disable_leak_detector(int disabled){
	liblinphone_tester_leak_detector_disabled = disabled;
}

void liblinphone_tester_clear_accounts(void){
	account_manager_destroy();
}

void liblinphone_tester_add_suites() {
	bc_tester_add_suite(&setup_test_suite);
	bc_tester_add_suite(&register_test_suite);
	bc_tester_add_suite(&tunnel_test_suite);
	bc_tester_add_suite(&offeranswer_test_suite);
	bc_tester_add_suite(&call_test_suite);
	#ifdef VIDEO_ENABLED
		bc_tester_add_suite(&call_video_test_suite);
	#endif // ifdef VIDEO_ENABLED
	bc_tester_add_suite(&audio_bypass_suite);
	bc_tester_add_suite(&multi_call_test_suite);
	bc_tester_add_suite(&message_test_suite);
	bc_tester_add_suite(&presence_test_suite);
	bc_tester_add_suite(&presence_server_test_suite);
	bc_tester_add_suite(&account_creator_test_suite);
	bc_tester_add_suite(&stun_test_suite);
	bc_tester_add_suite(&event_test_suite);
	bc_tester_add_suite(&conference_event_test_suite);
	bc_tester_add_suite(&contents_test_suite);
	bc_tester_add_suite(&flexisip_test_suite);
	bc_tester_add_suite(&remote_provisioning_test_suite);
	bc_tester_add_suite(&quality_reporting_test_suite);
	bc_tester_add_suite(&log_collection_test_suite);
	bc_tester_add_suite(&player_test_suite);
	bc_tester_add_suite(&dtmf_test_suite);
	bc_tester_add_suite(&cpim_test_suite);
	bc_tester_add_suite(&multipart_test_suite);
	bc_tester_add_suite(&clonable_object_test_suite);
	bc_tester_add_suite(&main_db_test_suite);
	bc_tester_add_suite(&property_container_test_suite);
	#ifdef VIDEO_ENABLED
		bc_tester_add_suite(&video_test_suite);
	#endif // ifdef VIDEO_ENABLED
	bc_tester_add_suite(&multicast_call_test_suite);
	bc_tester_add_suite(&proxy_config_test_suite);
#if HAVE_SIPP
	bc_tester_add_suite(&complex_sip_call_test_suite);
#endif
#ifdef VCARD_ENABLED
	bc_tester_add_suite(&vcard_test_suite);
#endif
	bc_tester_add_suite(&group_chat_test_suite);
}

static int linphone_core_manager_get_max_audio_bw_base(const int array[],int array_size) {
	int i,result=0;
	for (i=0; i<array_size; i++) {
		result = MAX(result,array[i]);
	}
	return result;
}

static int linphone_core_manager_get_mean_audio_bw_base(const int array[],int array_size) {
	int i,result=0;
	for (i=0; i<array_size; i++) {
		result += array[i];
	}
	return result/array_size;
}

int linphone_core_manager_get_max_audio_down_bw(const LinphoneCoreManager *mgr) {
	return linphone_core_manager_get_max_audio_bw_base(mgr->stat.audio_download_bandwidth
			, sizeof(mgr->stat.audio_download_bandwidth)/sizeof(int));
}
int linphone_core_manager_get_max_audio_up_bw(const LinphoneCoreManager *mgr) {
	return linphone_core_manager_get_max_audio_bw_base(mgr->stat.audio_upload_bandwidth
			, sizeof(mgr->stat.audio_upload_bandwidth)/sizeof(int));
}

int linphone_core_manager_get_mean_audio_down_bw(const LinphoneCoreManager *mgr) {
	return linphone_core_manager_get_mean_audio_bw_base(mgr->stat.audio_download_bandwidth
			, sizeof(mgr->stat.audio_download_bandwidth)/sizeof(int));
}
int linphone_core_manager_get_mean_audio_up_bw(const LinphoneCoreManager *mgr) {
	return linphone_core_manager_get_mean_audio_bw_base(mgr->stat.audio_upload_bandwidth
			, sizeof(mgr->stat.audio_upload_bandwidth)/sizeof(int));
}

void liblinphone_tester_before_each(void) {
	if (!liblinphone_tester_leak_detector_disabled){
		belle_sip_object_enable_leak_detector(TRUE);
		leaked_objects_count = belle_sip_object_get_object_count();
	}
}

static char* all_leaks_buffer = NULL;

void liblinphone_tester_after_each(void) {
	linphone_factory_clean();
	if (!liblinphone_tester_leak_detector_disabled){
		int leaked_objects = belle_sip_object_get_object_count() - leaked_objects_count;
		if (leaked_objects > 0) {
			char* format = ms_strdup_printf("%d object%s leaked in suite [%s] test [%s], please fix that!",
											leaked_objects, leaked_objects>1?"s were":" was",
											bc_tester_current_suite_name(), bc_tester_current_test_name());
			belle_sip_object_dump_active_objects();
			belle_sip_object_flush_active_objects();
			bc_tester_printf(ORTP_MESSAGE, format);
			ms_error("%s", format);

			all_leaks_buffer = ms_strcat_printf(all_leaks_buffer, "\n%s", format);
			ms_free(format);
		}

		// prevent any future leaks
		{
			const char **tags = bc_tester_current_test_tags();
			int leaks_expected =
				(tags && ((tags[0] && !strcmp(tags[0], "LeaksMemory")) || (tags[1] && !strcmp(tags[1], "LeaksMemory"))));
			// if the test is NOT marked as leaking memory and it actually is, we should make it fail
			if (!leaks_expected && leaked_objects > 0) {
				BC_FAIL("This test is leaking memory!");
				// and reciprocally
			} else if (leaks_expected && leaked_objects == 0) {
				BC_FAIL("This test is not leaking anymore, please remove LeaksMemory tag!");
			}
		}
	}

	if (manager_count != 0) {
		ms_fatal("%d Linphone core managers are still alive!", manager_count);
	}
}

void liblinphone_tester_uninit(void) {
	// show all leaks that happened during the test
	if (all_leaks_buffer) {
		bc_tester_printf(ORTP_MESSAGE, all_leaks_buffer);
		ms_free(all_leaks_buffer);
		all_leaks_buffer = NULL;
	}
	bc_tester_uninit();
	bctbx_uninit_logger();
}

static void check_ice_from_rtp(LinphoneCall *c1, LinphoneCall *c2, LinphoneStreamType stream_type) {
	MediaStream *ms;
	LinphoneCallStats *stats;
	switch (stream_type) {
	case LinphoneStreamTypeAudio:
		ms=linphone_call_get_stream(c1, LinphoneStreamTypeAudio);
		break;
	case LinphoneStreamTypeVideo:
		ms=linphone_call_get_stream(c1, LinphoneStreamTypeVideo);
		break;
	case LinphoneStreamTypeText:
		ms=linphone_call_get_stream(c1, LinphoneStreamTypeText);
		break;
	default:
		ms_error("Unknown stream type [%s]",  linphone_stream_type_to_string(stream_type));
		BC_ASSERT_FALSE(stream_type >= LinphoneStreamTypeUnknown);
		return;
	}

	stats = linphone_call_get_audio_stats(c1);
	if (linphone_call_stats_get_ice_state(stats) == LinphoneIceStateHostConnection && media_stream_started(ms)) {
		struct sockaddr_storage remaddr;
		socklen_t remaddrlen = sizeof(remaddr);
		char ip[NI_MAXHOST] = { 0 };
		int port = 0;
		SalMediaDescription *result_desc;
		char *expected_addr = NULL;
		AudioStream *astream;

		const LinphoneCallParams *cp1 = linphone_call_get_current_params(c1);
		const LinphoneCallParams *cp2 = linphone_call_get_current_params(c2);
		if (linphone_call_params_get_update_call_when_ice_completed(cp1) && linphone_call_params_get_update_call_when_ice_completed(cp2)) {
			memset(&remaddr, 0, remaddrlen);
			result_desc = sal_call_get_final_media_description(linphone_call_get_op_as_sal_op(c2));
			expected_addr = result_desc->streams[0].rtp_addr;
			if (expected_addr[0] == '\0') expected_addr = result_desc->addr;
			astream = (AudioStream *)linphone_call_get_stream(c1, LinphoneStreamTypeAudio);
			if ((strchr(expected_addr, ':') == NULL) && (astream->ms.sessions.rtp_session->rtp.gs.rem_addr.ss_family == AF_INET6)) {
				bctbx_sockaddr_ipv6_to_ipv4((struct sockaddr *)&astream->ms.sessions.rtp_session->rtp.gs.rem_addr, (struct sockaddr *)&remaddr, &remaddrlen);
			} else {
				memcpy(&remaddr, &astream->ms.sessions.rtp_session->rtp.gs.rem_addr, astream->ms.sessions.rtp_session->rtp.gs.rem_addrlen);
			}
			bctbx_sockaddr_to_ip_address((struct sockaddr *)&remaddr, remaddrlen, ip, sizeof(ip), &port);

			BC_ASSERT_STRING_EQUAL(ip, expected_addr);
		}
	}
	linphone_call_stats_unref(stats);
}

bool_t check_ice(LinphoneCoreManager* caller, LinphoneCoreManager* callee, LinphoneIceState state) {
	LinphoneCall *c1,*c2;
	bool_t global_success = TRUE;
	bool_t audio_success=FALSE;
	bool_t video_success=FALSE;
	bool_t text_success=FALSE;
	bool_t audio_enabled, video_enabled, realtime_text_enabled;
	MSTimeSpec ts;

	c1=linphone_core_get_current_call(caller->lc);
	c2=linphone_core_get_current_call(callee->lc);

	BC_ASSERT_PTR_NOT_NULL(c1);
	BC_ASSERT_PTR_NOT_NULL(c2);
	if (!c1 || !c2) return FALSE;
	linphone_call_ref(c1);
	linphone_call_ref(c2);

	BC_ASSERT_EQUAL(linphone_call_params_video_enabled(linphone_call_get_current_params(c1)),linphone_call_params_video_enabled(linphone_call_get_current_params(c2)), int, "%d");
	BC_ASSERT_EQUAL(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c1)),linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c2)), int, "%d");
	audio_enabled=linphone_call_params_audio_enabled(linphone_call_get_current_params(c1));
	video_enabled=linphone_call_params_video_enabled(linphone_call_get_current_params(c1));
	realtime_text_enabled=linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c1));
	if (audio_enabled) {
		liblinphone_tester_clock_start(&ts);
		LinphoneCallStats *stats1 = NULL;
		LinphoneCallStats *stats2 = NULL;
		do {
			if ((c1 != NULL) && (c2 != NULL)) {
				stats1 = linphone_call_get_audio_stats(c1);
				stats2 = linphone_call_get_audio_stats(c2);
				if (linphone_call_stats_get_ice_state(stats1)==state &&
					linphone_call_stats_get_ice_state(stats2)==state){
					audio_success=TRUE;
					check_ice_from_rtp(c1,c2,LinphoneStreamTypeAudio);
					check_ice_from_rtp(c2,c1,LinphoneStreamTypeAudio);
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
				linphone_call_stats_unref(stats1);
				linphone_call_stats_unref(stats2);
				stats1 = stats2 = NULL;
			}
			ms_usleep(20000);
		} while (!liblinphone_tester_clock_elapsed(&ts,10000));
		if (stats1)
			linphone_call_stats_unref(stats1);
		if (stats2)
			linphone_call_stats_unref(stats2);
	}

	if (video_enabled){
		liblinphone_tester_clock_start(&ts);
		LinphoneCallStats *stats1 = NULL;
		LinphoneCallStats *stats2 = NULL;
		do {
			if ((c1 != NULL) && (c2 != NULL)) {
				stats1 = linphone_call_get_video_stats(c1);
				stats2 = linphone_call_get_video_stats(c2);
				if (linphone_call_stats_get_ice_state(stats1)==state &&
					linphone_call_stats_get_ice_state(stats2)==state){
					video_success=TRUE;
					check_ice_from_rtp(c1,c2,LinphoneStreamTypeVideo);
					check_ice_from_rtp(c2,c1,LinphoneStreamTypeVideo);
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
				linphone_call_stats_unref(stats1);
				linphone_call_stats_unref(stats2);
				stats1 = stats2 = NULL;
			}
			ms_usleep(20000);
		} while (!liblinphone_tester_clock_elapsed(&ts,10000));
		if (stats1)
			linphone_call_stats_unref(stats1);
		if (stats2)
			linphone_call_stats_unref(stats2);
	}

	if (realtime_text_enabled){
		liblinphone_tester_clock_start(&ts);
		LinphoneCallStats *stats1 = NULL;
		LinphoneCallStats *stats2 = NULL;
		do {
			if ((c1 != NULL) && (c2 != NULL)) {
				stats1 = linphone_call_get_text_stats(c1);
				stats2 = linphone_call_get_text_stats(c2);
				if (linphone_call_stats_get_ice_state(stats1)==state &&
					linphone_call_stats_get_ice_state(stats2)==state){
					text_success=TRUE;
					check_ice_from_rtp(c1,c2,LinphoneStreamTypeText);
					check_ice_from_rtp(c2,c1,LinphoneStreamTypeText);
					break;
				}
				linphone_core_iterate(caller->lc);
				linphone_core_iterate(callee->lc);
				linphone_call_stats_unref(stats1);
				linphone_call_stats_unref(stats2);
				stats1 = stats2 = NULL;
			}
			ms_usleep(20000);
		} while (!liblinphone_tester_clock_elapsed(&ts,10000));
		if (stats1)
			linphone_call_stats_unref(stats1);
		if (stats2)
			linphone_call_stats_unref(stats2);
	}

	/*make sure encryption mode are preserved*/
	if (c1) {
		const LinphoneCallParams* call_param = linphone_call_get_current_params(c1);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(caller->lc), int, "%d");
	}
	if (c2) {
		const LinphoneCallParams* call_param = linphone_call_get_current_params(c2);
		BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(callee->lc), int, "%d");
	}
	linphone_call_unref(c1);
	linphone_call_unref(c2);
	if (audio_enabled) global_success = global_success && audio_success;
	if (video_enabled) global_success = global_success && video_success;
	if (realtime_text_enabled) global_success = global_success && text_success;
	return global_success;
}

static void linphone_conference_server_call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg) {
	LinphoneCoreCbs *cbs = linphone_core_get_current_callbacks(lc);
	LinphoneConferenceServer *conf_srv = (LinphoneConferenceServer *)linphone_core_cbs_get_user_data(cbs);

	switch(cstate) {
		case LinphoneCallIncomingReceived:
			linphone_call_accept(call);
			break;

		case LinphoneCallStreamsRunning:
			if(linphone_call_get_conference(call) == NULL) {
				linphone_core_add_to_conference(lc, call);
				linphone_core_leave_conference(lc);
				if(conf_srv->first_call == NULL) conf_srv->first_call = call;
			}
			break;

		case LinphoneCallEnd:
			if(call == conf_srv->first_call) {
				if(linphone_core_get_conference(lc)) {
					linphone_core_terminate_conference(lc);
				}
				conf_srv->first_call = NULL;
			}
			break;

		default: break;
	}
}

static void linphone_conference_server_refer_received(LinphoneCore *core, const char *refer_to) {
	char method[20];
	LinphoneAddress *refer_to_addr = linphone_address_new(refer_to);
	char *uri;
	LinphoneCall *call;

	if(refer_to_addr == NULL) return;
	strncpy(method, linphone_address_get_method_param(refer_to_addr), sizeof(method));
	method[sizeof(method) - 1] = '\0';
	if(strcmp(method, "BYE") == 0) {
		linphone_address_clean(refer_to_addr);
		uri = linphone_address_as_string_uri_only(refer_to_addr);
		call = linphone_core_find_call_from_uri(core, uri);
		if(call) linphone_call_terminate(call);
		ms_free(uri);
	}
	linphone_address_unref(refer_to_addr);
}

static void linphone_conference_server_registration_state_changed(
	LinphoneCore *core,
	LinphoneProxyConfig *cfg,
	LinphoneRegistrationState cstate,
	const char *message
) {
	LinphoneCoreCbs *cbs = linphone_core_get_current_callbacks(core);
	LinphoneConferenceServer *m = (LinphoneConferenceServer *)linphone_core_cbs_get_user_data(cbs);
	if(cfg == linphone_core_get_default_proxy_config(core)) {
		m->reg_state = cstate;
	}
}

LinphoneConferenceServer* linphone_conference_server_new(const char *rc_file, bool_t do_registration) {
	LinphoneConferenceServer *conf_srv = (LinphoneConferenceServer *)ms_new0(LinphoneConferenceServer, 1);
	LinphoneCoreManager *lm = (LinphoneCoreManager *)conf_srv;
	LinphoneProxyConfig *proxy;
	conf_srv->cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_call_state_changed(conf_srv->cbs, linphone_conference_server_call_state_changed);
	linphone_core_cbs_set_refer_received(conf_srv->cbs, linphone_conference_server_refer_received);
	linphone_core_cbs_set_registration_state_changed(conf_srv->cbs, linphone_conference_server_registration_state_changed);
	linphone_core_cbs_set_user_data(conf_srv->cbs, conf_srv);
	conf_srv->reg_state = LinphoneRegistrationNone;
	linphone_core_manager_init(lm, rc_file,NULL);
	if (!do_registration) {
		proxy = linphone_core_get_default_proxy_config(lm->lc);
		linphone_proxy_config_edit(proxy);
		linphone_proxy_config_enable_register(proxy,FALSE);
		linphone_proxy_config_done(proxy);
	}
	linphone_core_add_callbacks(lm->lc, conf_srv->cbs);
	linphone_core_manager_start(lm, do_registration);
	return conf_srv;
}

void linphone_conference_server_destroy(LinphoneConferenceServer *conf_srv) {
	linphone_core_cbs_unref(conf_srv->cbs);
	linphone_core_manager_destroy((LinphoneCoreManager *)conf_srv);
}
