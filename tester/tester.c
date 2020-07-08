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

#include <stdio.h>
#include <stdlib.h>
#include "linphone/core.h"
#include "linphone/logging.h"
#include "logging-private.h"
#include "liblinphone_tester.h"
#include <bctoolbox/tester.h>
#include "tester_utils.h"

#define SKIP_PULSEAUDIO 1

#if _WIN32
#define unlink _unlink
#endif

#ifdef __ANDROID__
extern jobject system_context;
#else
void *system_context=0;
#endif

static char *liblinphone_tester_empty_rc_path = NULL;
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
const char* pure_sha256_user="pure_sha256_user";
const char* test_password="secret";
const char* test_route="sip2.linphone.org";
const char *userhostsfile = "tester_hosts";
const char *file_transfer_url="https://transfer.example.org:9444/http-file-transfer-server/hft.php";
// These lime server authenticate user using Digest auth only
const char *lime_server_c25519_url="https://lime.wildcard1.linphone.org:8443/lime-server-c25519/lime-server.php";
const char *lime_server_c448_url="https://lime.wildcard1.linphone.org:8443/lime-server-c448/lime-server.php";
// These lime server authenticate user using TLS auth only
const char *lime_server_c25519_tlsauth_req_url="https://lime.wildcard1.linphone.org:8543/lime-server-c25519/lime-server.php";
const char *lime_server_c448_tlsauth_req_url="https://lime.wildcard1.linphone.org:8543/lime-server-c448/lime-server.php";
// These lime server authenticate user using optionnal TLS auth, falling back on digest auth if client did not provide a client certificate
const char *lime_server_c25519_tlsauth_opt_url="https://lime.wildcard1.linphone.org:8544/lime-server-c25519/lime-server.php";
const char *lime_server_c448_tlsauth_opt_url="https://lime.wildcard1.linphone.org:8544/lime-server-c448/lime-server.php";
bool_t liblinphonetester_ipv6 = TRUE;
bool_t liblinphonetester_show_account_manager_logs = FALSE;
bool_t liblinphonetester_no_account_creator = FALSE;
int liblinphonetester_transport_timeout = 9000; /*milliseconds. it is set to such low value to workaround a problem with our Freebox v6 when connecting to Ipv6 addresses.
			It was found that the freebox sometimes block SYN-ACK packets, which prevents connection to be succesful.
			Thanks to the timeout, it will fallback to IPv4*/
char* message_external_body_url=NULL;
static const char *notify_content="<somexml2>blabla</somexml2>";

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
	ms_message("Auth info requested (deprecated callback) for user id [%s] at realm [%s]\n", username, realm);
	counters = get_stats(lc);
	counters->number_of_auth_info_requested++;
}

void reset_counters( stats* counters) {
	if (counters->last_received_chat_message) linphone_chat_message_unref(counters->last_received_chat_message);
	if (counters->last_received_info_message) linphone_info_message_unref(counters->last_received_info_message);
	if (counters->dtmf_list_received) bctbx_free(counters->dtmf_list_received);

	memset(counters,0,sizeof(stats));
}

static void setup_dns(LinphoneCore *lc, const char *path){
	if (strcmp(userhostsfile, "none") != 0) {
		char *dnsuserhostspath = strchr(userhostsfile, '/') ? ms_strdup(userhostsfile) : ms_strdup_printf("%s/%s", path, userhostsfile);
		sal_set_dns_user_hosts_file(linphone_core_get_sal(lc), dnsuserhostspath);
		ms_free(dnsuserhostspath);
	} else {
		bctbx_message("no dns-hosts file used");
	}
}

void configure_lc(LinphoneCore *lc, const char *path, void *user_data) {
	linphone_core_enable_ipv6(lc, liblinphonetester_ipv6);
	linphone_core_set_sip_transport_timeout(lc, liblinphonetester_transport_timeout);
	linphone_core_set_user_data(lc, user_data);

	sal_enable_test_features(linphone_core_get_sal(lc), TRUE);

	setup_dns(lc, path);
}

LinphoneCore *configure_lc_from(LinphoneCoreCbs *cbs, const char *path, LinphoneConfig *config, void *user_data) {
	LinphoneCore *lc;
	char *ringpath         = NULL;
	char *ringbackpath     = NULL;
	char *rootcapath       = NULL;
	char *nowebcampath     = NULL;

	// setup dynamic-path assets
	ringpath         = ms_strdup_printf("%s/sounds/oldphone.wav",path);
	ringbackpath     = ms_strdup_printf("%s/sounds/ringback.wav", path);
	nowebcampath     = ms_strdup_printf("%s/images/nowebcamCIF.jpg", path);
	rootcapath       = ms_strdup_printf("%s/certificates/cn/cafile.pem", path);

	if (config) {
		linphone_config_set_string(config, "sound", "remote_ring", ringbackpath);
		linphone_config_set_string(config, "sound", "local_ring" , ringpath);
		linphone_config_set_string(config, "sip",   "root_ca"    , rootcapath);

		LinphoneCoreManager *mgr = (LinphoneCoreManager *)user_data;
		if (mgr && mgr->app_group_id) {
			lc = linphone_factory_create_shared_core_with_config(linphone_factory_get(), config, system_context, mgr->app_group_id, mgr->main_core);
		} else {
			lc = linphone_factory_create_core_with_config_3(linphone_factory_get(), config, system_context);
		}
	} else {
		lc = linphone_factory_create_core_3(linphone_factory_get(), NULL, liblinphone_tester_get_empty_rc(), system_context);
		linphone_core_set_ring(lc, ringpath);
		linphone_core_set_ringback(lc, ringbackpath);
		linphone_core_set_root_ca(lc, rootcapath);
	}
	if (cbs)
		linphone_core_add_callbacks(lc, cbs);
#ifdef VIDEO_ENABLED
	linphone_core_set_static_picture(lc,nowebcampath);
#endif
	configure_lc(lc, path, user_data);

	ms_free(ringpath);
	ms_free(ringbackpath);
	ms_free(nowebcampath);
	ms_free(rootcapath);
	return lc;
}

bool_t wait_for_until_interval(LinphoneCore* lc_1, LinphoneCore* lc_2,int* counter,int min,int max,int timout) {
	bctbx_list_t* lcs=NULL;
	bool_t result;
	if (lc_1)
		lcs=bctbx_list_append(lcs,lc_1);
	if (lc_2)
		lcs=bctbx_list_append(lcs,lc_2);
	result=wait_for_list_interval(lcs,counter,min,max,timout);
	bctbx_list_free(lcs);
	return result;
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

bool_t wait_for_list_interval(bctbx_list_t* lcs,int* counter,int min, int max,int timeout_ms) {
	bctbx_list_t* iterator;
	MSTimeSpec start;

	liblinphone_tester_clock_start(&start);
	while ((counter==NULL || *counter<min || *counter>max) && !liblinphone_tester_clock_elapsed(&start,timeout_ms)) {
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
	if(counter && (*counter<min || *counter>max)) return FALSE;
	else return TRUE;
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



#ifdef SKIP_PULSEAUDIO
static void avoid_pulseaudio_hack(LinphoneCoreManager *mgr){
	bctbx_list_t *cards = linphone_core_get_sound_devices_list(mgr->lc);
	bctbx_list_t *it;
	bool_t capture_set = FALSE, playback_set = FALSE;
	bool_t pulseaudio_found = FALSE;
	for (it = cards; it != NULL ; it = it->next){
		const char * card_id = (const char *)it->data;
		if (strstr(card_id, "PulseAudio") != NULL) {
			pulseaudio_found = TRUE;
			continue;
		}
		if (!capture_set && linphone_core_sound_device_can_capture(mgr->lc, card_id)){
			capture_set = TRUE;
			linphone_core_set_capture_device(mgr->lc, card_id);
		}
		if (!playback_set && linphone_core_sound_device_can_playback(mgr->lc, card_id)){
			playback_set = TRUE;
			linphone_core_set_playback_device(mgr->lc, card_id);
			linphone_core_set_ringer_device(mgr->lc, card_id);
		}
		if (playback_set && capture_set){
			if (pulseaudio_found) ms_warning("PulseAudio is not used in liblinphone_tester because of internal random crashes or hangs.");
			break;
		}
	}
	if (!playback_set || !capture_set){
		ms_error("Could not find soundcard other than pulseaudio to use during tests. Some tests may crash or hang.");
	}
	bctbx_list_free(cards);
}
#endif

void linphone_core_manager_setup_dns(LinphoneCoreManager *mgr){
	setup_dns(mgr->lc, bc_tester_get_resource_dir_prefix());
}

LinphoneCore *linphone_core_manager_configure_lc(LinphoneCoreManager *mgr) {
	LinphoneCore *lc;
	char *filepath = mgr->rc_path ? bctbx_strdup_printf("%s/%s", bc_tester_get_resource_dir_prefix(), mgr->rc_path) : NULL;
	if (filepath && bctbx_file_exist(filepath) != 0) {
		ms_fatal("Could not find file %s in path %s, did you configured resources directory correctly?", mgr->rc_path, bc_tester_get_resource_dir_prefix());
	}
	LinphoneConfig * config = linphone_factory_create_config_with_factory(linphone_factory_get(), NULL, filepath);
	linphone_config_set_string(config, "storage", "backend", "sqlite3");
	linphone_config_set_string(config, "storage", "uri", mgr->database_path);
	linphone_config_set_string(config, "lime", "x3dh_db_path", mgr->lime_database_path);
	lc = configure_lc_from(mgr->cbs, bc_tester_get_resource_dir_prefix(), config, mgr);
	linphone_config_unref(config);
	return lc;
}

void linphone_core_manager_configure(LinphoneCoreManager *mgr) {
	LinphoneImNotifPolicy *im_notif_policy;
	char *hellopath = bc_tester_res("sounds/hello8000.wav");
	char *filepath = mgr->rc_path ? bctbx_strdup_printf("%s/%s", bc_tester_get_resource_dir_prefix(), mgr->rc_path) : NULL;
	if (filepath && bctbx_file_exist(filepath) != 0) {
		ms_fatal("Could not find file %s in path %s, did you configured resources directory correctly?", mgr->rc_path, bc_tester_get_resource_dir_prefix());
	}

	mgr->lc = linphone_core_manager_configure_lc(mgr);

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
#elif defined(SKIP_PULSEAUDIO)
	{
		/* Special trick for linux. Pulseaudio has random hangs, deadlocks or abort while executing test suites.
		 * It never happens in the linphone app.
		 * So far we could not identify something bad in our pulseaudio usage. As a workaround, we disable pulseaudio for the tests.*/
		avoid_pulseaudio_hack(mgr);
	}
#endif

#ifdef VIDEO_ENABLED
	{
		MSWebCam *cam;

		cam = ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(linphone_core_get_ms_factory(mgr->lc)), "Mire: Mire (synthetic moving picture)");

		//Usefull especially for encoders not supporting qcif
		#ifdef __ANDROID__
			MSVideoSize vsize = MS_VIDEO_SIZE_CIF;
			linphone_core_set_preferred_video_size(mgr->lc, vsize);
		#endif


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

	// clean
	if (filepath) bctbx_free(filepath);
}

static void generate_random_database_path (LinphoneCoreManager *mgr) {
	char random_id[32];
	belle_sip_random_token(random_id, sizeof random_id);
	char *database_path_format = bctbx_strdup_printf("linphone_%s.db", random_id);
	mgr->database_path = bc_tester_file(database_path_format);
	bctbx_free(database_path_format);
	database_path_format = bctbx_strdup_printf("lime_%s.db", random_id);
	mgr->lime_database_path = bc_tester_file(database_path_format);
	bctbx_free(database_path_format);
}

static void conference_state_changed (LinphoneConference *conference, LinphoneConferenceState newState) {
	LinphoneCore *core = linphone_conference_get_core(conference);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	char * addr_str = linphone_conference_get_conference_address_as_string(conference);

	if (addr_str) {
		ms_message("Conference [%s] state changed: %d", addr_str, newState);
	}
	switch (newState) {
		case LinphoneConferenceStateNone:
			break;
		case LinphoneConferenceStateInstantiated:
			manager->stat.number_of_LinphoneConferenceStateInstantiated++;
			break;
		case LinphoneConferenceStateCreationPending:
			manager->stat.number_of_LinphoneConferenceStateCreationPending++;
			break;
		case LinphoneConferenceStateCreated:
			manager->stat.number_of_LinphoneConferenceStateCreated++;
			break;
		case LinphoneConferenceStateCreationFailed:
			manager->stat.number_of_LinphoneConferenceStateCreationFailed++;
			break;
		case LinphoneConferenceStateTerminationPending:
			manager->stat.number_of_LinphoneConferenceStateTerminationPending++;
			break;
		case LinphoneConferenceStateTerminated:
			manager->stat.number_of_LinphoneConferenceStateTerminated++;
			break;
		case LinphoneConferenceStateTerminationFailed:
			manager->stat.number_of_LinphoneConferenceStateTerminationFailed++;
			break;
		case LinphoneConferenceStateDeleted:
			manager->stat.number_of_LinphoneConferenceStateDeleted++;
			break;
		default:
			ms_error("Invalid Conference state for Conference [%s] EndOfEnum is used ONLY as a guard", addr_str);
			break;
	}

	bctbx_free(addr_str);
}

void core_conference_state_changed (LinphoneCore *core, LinphoneConference *conference, LinphoneConferenceState state) {
	if (state == LinphoneConferenceStateInstantiated) {
		LinphoneConferenceCbs * cbs = linphone_factory_create_conference_cbs(linphone_factory_get());
		linphone_conference_cbs_set_state_changed(cbs, conference_state_changed);

		linphone_conference_add_callbacks(conference, cbs);
		linphone_conference_cbs_unref(cbs);
	}
}

LinphoneStatus add_participant_to_conference_through_invite(bctbx_list_t *lcs, LinphoneCoreManager * conf_mgr, bctbx_list_t *participants, const LinphoneCallParams *params) {

	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);

	stats conf_initial_stats = conf_mgr->stat;
	stats * participants_initial_stats = NULL;
	bctbx_list_t * participant_addresses = NULL;
	int counter = 1;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		participant_addresses = bctbx_list_append(participant_addresses, m->identity);

		// Allocate memory
		participants_initial_stats = (stats*)realloc(participants_initial_stats, counter * sizeof(stats));
		// Append element
		participants_initial_stats[counter - 1] = m->stat;
		// Increment counter
		counter++;

	}

	LinphoneConference * conference = linphone_core_get_conference(conf_mgr->lc);
	LinphoneStatus status = linphone_conference_invite_participants(conference, participant_addresses, params);

	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallOutgoingProgress,conf_initial_stats.number_of_LinphoneCallOutgoingProgress + no_participants,2000));

	if (participants != NULL) {
		int idx = 0;
		for (bctbx_list_t *itm = participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(itm);
			if (linphone_core_is_network_reachable(m->lc)) {
				BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallIncomingReceived,(participants_initial_stats[idx].number_of_LinphoneCallIncomingReceived + 1),10000));
			}
			idx++;
		}
	}

	return status;

}

LinphoneStatus accept_call_in_conference(bctbx_list_t *lcs, LinphoneCoreManager * conf_mgr, LinphoneCoreManager * participant_mgr) {

	stats conf_initial_stats = conf_mgr->stat;
	stats participant_initial_stats = participant_mgr->stat;
	int init_subscription_count = *((int *)(conf_mgr->user_info));

	stats* other_participants_initial_stats = NULL;
	bctbx_list_t *other_participants = NULL;
	int counter = 1;
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
		if ((m != participant_mgr) && (m != conf_mgr)) {
			// Allocate memory
			other_participants_initial_stats = (stats*)realloc(other_participants_initial_stats, counter * sizeof(stats));
			// Append element
			other_participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
			other_participants = bctbx_list_append(other_participants, m);
		}
	}

	LinphoneCall * participant_call = linphone_core_get_current_call(participant_mgr->lc);
	BC_ASSERT_PTR_NOT_NULL(participant_call);
	LinphoneStatus status = linphone_call_accept(participant_call);

	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallConnected,conf_initial_stats.number_of_LinphoneCallConnected + 1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallStreamsRunning,conf_initial_stats.number_of_LinphoneCallStreamsRunning + 1,3000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneCallStreamsRunning,participant_initial_stats.number_of_LinphoneCallStreamsRunning + 1,3000));

	// Conference may have already been created of call was paused before hence initial stats can lead to false errors
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	// Check subscriptions
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,(participant_initial_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1),1000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneSubscriptionIncomingReceived,(conf_initial_stats.number_of_LinphoneSubscriptionIncomingReceived + 1),1000));

	int* subscription_count = ((int *)(conf_mgr->user_info));
	BC_ASSERT_TRUE(wait_for_list(lcs,subscription_count,(init_subscription_count + 1),5000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_NotifyReceived,(participant_initial_stats.number_of_NotifyReceived + 1),5000));

	LinphoneConference * conference = linphone_core_get_conference(conf_mgr->lc);

	if (other_participants != NULL) {
		int idx = 0;
		for (bctbx_list_t *itm = other_participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(itm);
			if (conference == linphone_core_get_conference(m->lc)) {
				BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_NotifyReceived,(other_participants_initial_stats[idx].number_of_NotifyReceived + 1),5000));
			}
			idx++;
		}
	}

	// Number of subscription errors should not change as they the participant should received a notification
	BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionError,conf_initial_stats.number_of_LinphoneSubscriptionError, int, "%0d");
	BC_ASSERT_EQUAL(participant_mgr->stat.number_of_LinphoneSubscriptionError,participant_initial_stats.number_of_LinphoneSubscriptionError, int, "%0d");

	// Number of subscription terminated should not change as they the participant should received a notification
	BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionTerminated, conf_initial_stats.number_of_LinphoneSubscriptionTerminated, int, "%d");
	BC_ASSERT_EQUAL(participant_mgr->stat.number_of_LinphoneSubscriptionTerminated, participant_initial_stats.number_of_LinphoneSubscriptionTerminated, int, "%d");

	return status;
}

static void check_participant_added_to_conference(bctbx_list_t *lcs, LinphoneCoreManager * conf_mgr, stats conf_initial_stats, bctbx_list_t *participants, stats* participant_initial_stats) {

	unsigned int no_participants = (unsigned int)bctbx_list_size(participants);
	int init_subscription_count = *((int *)(conf_mgr->user_info));

	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallStreamsRunning,conf_initial_stats.number_of_LinphoneCallStreamsRunning + no_participants,3000));

	int idx = 0;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallStreamsRunning,participant_initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1,3000));

		// Remote conference creation
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateCreationPending, participant_initial_stats[idx].number_of_LinphoneConferenceStateCreationPending + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateCreated, participant_initial_stats[idx].number_of_LinphoneConferenceStateCreationPending + 1, 5000));

		// Check subscriptions
		BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneSubscriptionOutgoingProgress,(participant_initial_stats[idx].number_of_LinphoneSubscriptionOutgoingProgress + 1),1000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneSubscriptionIncomingReceived,(conf_initial_stats.number_of_LinphoneSubscriptionIncomingReceived + idx),1000));

		int* subscription_count = ((int *)(conf_mgr->user_info));
		BC_ASSERT_TRUE(wait_for_list(lcs,subscription_count,(init_subscription_count + idx),5000));

		// Notify
		int idx2 = 0;
		for (bctbx_list_t *itm = participants; itm; itm = bctbx_list_next(itm)) {
			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_NotifyReceived,(participant_initial_stats[idx2].number_of_NotifyReceived + idx),5000));
			idx2++;
		}

		// Number of subscription errors should not change as they the participant should received a notification
		BC_ASSERT_EQUAL(m->stat.number_of_LinphoneSubscriptionError,participant_initial_stats[idx].number_of_LinphoneSubscriptionError, int, "%0d");

		// Number of subscription terminated should not change as they the participant should received a notification
		BC_ASSERT_EQUAL(m->stat.number_of_LinphoneSubscriptionTerminated, participant_initial_stats[idx].number_of_LinphoneSubscriptionTerminated, int, "%d");
	}

	BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionError,conf_initial_stats.number_of_LinphoneSubscriptionError, int, "%0d");
	BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionTerminated, conf_initial_stats.number_of_LinphoneSubscriptionTerminated, int, "%d");

}

LinphoneStatus add_calls_to_conference(bctbx_list_t *lcs, LinphoneCoreManager * conf_mgr, bctbx_list_t *calls) {

	stats conf_initial_stats = conf_mgr->stat;

	stats* participants_initial_stats = NULL;
	bctbx_list_t *participants = NULL;
	int counter = 1;
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
		if (m != conf_mgr) {
			// Allocate memory
			participants_initial_stats = (stats*)realloc(participants_initial_stats, counter * sizeof(stats));
			// Append element
			participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
			participants = bctbx_list_append(participants, m);
		}
	}

	for (bctbx_list_t *it = calls; it; it = bctbx_list_next(it)) {
		LinphoneCall * call = (LinphoneCall *)bctbx_list_get_data(it);
		linphone_core_add_to_conference(conf_mgr->lc,call);
	}

	check_participant_added_to_conference(lcs, conf_mgr, conf_initial_stats, participants, participants_initial_stats);

	return 0;
}

LinphoneStatus remove_participant_from_conference(bctbx_list_t *lcs, LinphoneCoreManager * conf_mgr, LinphoneCoreManager * participant_mgr) {

	stats conf_initial_stats = conf_mgr->stat;
	stats participant_initial_stats = conf_mgr->stat;

	LinphoneConference * conference = linphone_core_get_conference(conf_mgr->lc);
	int participant_size = linphone_conference_get_participant_count(conference);

	stats* participants_initial_stats = NULL;
	bctbx_list_t *participants = NULL;
	int counter = 1;
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
		if ((m != participant_mgr) && (m != conf_mgr) && (conference == linphone_core_get_conference(m->lc))) {
			// Allocate memory
			participants_initial_stats = (stats*)realloc(participants_initial_stats, counter * sizeof(stats));
			// Append element
			participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
			participants = bctbx_list_append(participants, m);
		}
	}

	LinphoneCall * participant_call = linphone_core_get_current_call(participant_mgr->lc);
//	const LinphoneAddress *c_participant_address = linphone_call_get_to_address(participant_call);
//	const char * participant_uri = L_GET_CPP_PTR_FROM_C_OBJECT(c_participant_address)->asStringUriOnly().c_str();
	const LinphoneAddress *participant_uri = linphone_call_get_to_address(participant_call);
	LinphoneCall * conf_call = linphone_core_get_call_by_remote_address(conf_mgr->lc, linphone_address_as_string(participant_uri));

	linphone_core_remove_from_conference(conf_mgr->lc, conf_call);

	// Calls are paused when removing a participant 
	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallPausing,(conf_initial_stats.number_of_LinphoneCallPausing + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallPaused,(conf_initial_stats.number_of_LinphoneCallPaused + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneCallPausedByRemote,(participant_initial_stats.number_of_LinphoneCallPausedByRemote + 1),5000));

	// Wait for conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneConferenceStateTerminationPending,(participant_initial_stats.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneConferenceStateTerminated,(participant_initial_stats.number_of_LinphoneConferenceStateTerminated + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneConferenceStateDeleted,(participant_initial_stats.number_of_LinphoneConferenceStateDeleted + 1),5000));

	int expected_no_participants = 0;
	if (participant_size == 2) {
		expected_no_participants = 0;
	} else {
		expected_no_participants = (participant_size - 1);
	}

	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), expected_no_participants, int, "%d");
	BC_ASSERT_EQUAL(linphone_conference_get_participant_count(conference), expected_no_participants, int, "%d");

	// Other participants call should not have their state modified
	if (participants != NULL) {
		int idx = 0;
		for (bctbx_list_t *itm = participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(itm);
			// If removing last participant, then its call is kicked out of conference
			// - Remote conference is deleted
			// - parameters are updated
			if (participant_size == 2) {
				BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallUpdatedByRemote,(participants_initial_stats[idx].number_of_LinphoneCallUpdatedByRemote + 1),5000));
				BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallStreamsRunning,(participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1), 5000));

				BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneConferenceStateTerminationPending,(participants_initial_stats[idx].number_of_LinphoneConferenceStateTerminationPending + 1),5000));
				BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneConferenceStateTerminated,(participants_initial_stats[idx].number_of_LinphoneConferenceStateTerminated + 1),5000));
				BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneConferenceStateDeleted,(participants_initial_stats[idx].number_of_LinphoneConferenceStateDeleted + 1),5000));
			} else {
				BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallStreamsRunning,participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning, int, "%0d");
			}
			BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallEnd,participants_initial_stats[idx].number_of_LinphoneCallEnd, int, "%0d");
			BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallReleased,participants_initial_stats[idx].number_of_LinphoneCallReleased, int, "%0d");
			idx++;
		}
	}

	return 0;
}

LinphoneStatus terminate_conference(bctbx_list_t *lcs, LinphoneCoreManager * conf_mgr) {

	stats* participants_initial_stats = NULL;
	int counter = 1;
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);
		if (m != conf_mgr) {
			// Allocate memory
			participants_initial_stats = (stats*)realloc(participants_initial_stats, counter * sizeof(stats));
			// Append element
			participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
		}
	}

	linphone_core_terminate_conference(conf_mgr->lc);

	int idx = 0;
	for (bctbx_list_t *it = lcs; it; it = bctbx_list_next(it)) {
		LinphoneCore * c = (LinphoneCore *)bctbx_list_get_data(it);
		LinphoneCoreManager * m = get_manager(c);

		int no_calls = (int)bctbx_list_size(linphone_core_get_calls(m->lc));
		linphone_core_terminate_all_calls(m->lc);

		// Wait for all calls to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd, participants_initial_stats[idx].number_of_LinphoneCallEnd + no_calls, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased, participants_initial_stats[idx].number_of_LinphoneCallEnd + no_calls, 5000));

		// Wait for all conferences to be terminated
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending, m->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated, m->stat.number_of_LinphoneConferenceStateCreated, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted, m->stat.number_of_LinphoneConferenceStateCreated, 5000));

		idx++;
	}

	return 0;
}

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
void linphone_core_manager_init2(LinphoneCoreManager *mgr, const char* rc_file, const char* phone_alias) {
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
	linphone_core_cbs_set_global_state_changed(mgr->cbs, global_state_changed);
	linphone_core_cbs_set_message_sent(mgr->cbs, liblinphone_tester_chat_room_msg_sent);
	linphone_core_cbs_set_first_call_started(mgr->cbs, first_call_started);
	linphone_core_cbs_set_last_call_ended(mgr->cbs, last_call_ended);
	linphone_core_cbs_set_audio_device_changed(mgr->cbs, audio_device_changed);
	linphone_core_cbs_set_audio_devices_list_updated(mgr->cbs, audio_devices_list_updated);
	linphone_core_cbs_set_imee_user_registration(mgr->cbs, liblinphone_tester_x3dh_user_created);
	linphone_core_cbs_set_conference_state_changed(mgr->cbs, core_conference_state_changed);

	mgr->phone_alias = phone_alias ? ms_strdup(phone_alias) : NULL;

	reset_counters(&mgr->stat);
	manager_count++;
}

void linphone_core_manager_init(LinphoneCoreManager *mgr, const char* rc_file, const char* phone_alias) {
	linphone_core_manager_init2(mgr, rc_file, phone_alias);
	if (rc_file) mgr->rc_path = ms_strdup_printf("rcfiles/%s", rc_file);
	generate_random_database_path(mgr);
	linphone_core_manager_configure(mgr);
}

void linphone_core_manager_init_shared(LinphoneCoreManager *mgr, const char* rc_file, const char* phone_alias, LinphoneCoreManager *mgr_to_copy) {
	linphone_core_manager_init2(mgr, rc_file, phone_alias);

	if (mgr_to_copy == NULL) {
		if (rc_file) mgr->rc_path = ms_strdup_printf("rcfiles/%s", rc_file);
		generate_random_database_path(mgr);
	} else {
		mgr->rc_path = ms_strdup(mgr_to_copy->rc_path);
		mgr->database_path = ms_strdup(mgr_to_copy->database_path);
		mgr->lime_database_path = ms_strdup(mgr_to_copy->lime_database_path);
	}
	linphone_core_manager_configure(mgr);
}
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif

void linphone_core_manager_start(LinphoneCoreManager *mgr, bool_t check_for_proxies) {
	LinphoneProxyConfig* proxy;
	int proxy_count;

	if (linphone_core_start(mgr->lc) == -1) {
		ms_error("Core [%p] failed to start", mgr->lc);
	}

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

LinphoneCoreManager *linphone_core_manager_new(const char *rc_file) {
	return linphone_core_manager_new2(rc_file, TRUE);
}

void linphone_core_start_process_remote_notification (LinphoneCoreManager *mgr, const char *callid) {
	LinphoneCall *incomingCall = linphone_core_get_call_by_callid(mgr->lc, callid);
	if (!incomingCall) {
		incomingCall = linphone_call_new_incoming_with_callid(mgr->lc, callid);
		linphone_call_start_basic_incoming_notification(incomingCall);
		linphone_call_start_push_incoming_notification(incomingCall);
		linphone_core_ensure_registered(mgr->lc);
	}
}


/**
 * Create a LinphoneCoreManager that holds a shared Core.
 * mgr_to_copy is used to create a second LinphoneCoreManager with the same identity.
 * If mgr_to_copy has a value, rc_file parameter is ignored.
 */
LinphoneCoreManager* linphone_core_manager_create_shared(const char *rc_file, const char *app_group_id, bool_t main_core, LinphoneCoreManager *mgr_to_copy) {
	LinphoneCoreManager *manager = ms_new0(LinphoneCoreManager, 1);
	manager->app_group_id = ms_strdup(app_group_id);
	manager->main_core = main_core;
	linphone_core_manager_init_shared(manager, rc_file, NULL, mgr_to_copy);
	return manager;
}

void linphone_core_manager_stop(LinphoneCoreManager *mgr) {
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
		linphone_core_stop(mgr->lc);
		linphone_core_unref(mgr->lc);
		mgr->lc = NULL;
	}
}

void linphone_core_manager_uninit_after_stop_async(LinphoneCoreManager *mgr) {
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
		if (linphone_config_get_string(linphone_core_get_config(mgr->lc), "misc", "uuid", NULL))
			uuid = bctbx_strdup(linphone_config_get_string(linphone_core_get_config(mgr->lc), "misc", "uuid", NULL));
		linphone_core_set_network_reachable(mgr->lc, FALSE); // to avoid unregister
		linphone_core_unref(mgr->lc);
	}
	linphone_core_manager_configure(mgr);
	reset_counters(&mgr->stat);
	// Make sure gruu is preserved
	linphone_config_set_string(linphone_core_get_config(mgr->lc), "misc", "uuid", uuid);
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
		bc_free(mgr->database_path);
	}
	if (mgr->lime_database_path) {
		unlink(mgr->lime_database_path);
		bc_free(mgr->lime_database_path);
	}
	if (mgr->app_group_id)
		bctbx_free(mgr->app_group_id);

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
	linphone_core_manager_uninit2(mgr);
	ms_free(mgr);
}
void linphone_core_manager_uninit2(LinphoneCoreManager* mgr) {
	if (mgr->lc && linphone_core_get_global_state(mgr->lc) != LinphoneGlobalOff && !linphone_core_is_network_reachable(mgr->lc)) {
		int previousNbRegistrationOk = mgr->stat.number_of_LinphoneRegistrationOk;
		linphone_core_set_network_reachable(mgr->lc, TRUE);
		wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_LinphoneRegistrationOk, previousNbRegistrationOk + 1, 2000);
	}
	linphone_core_manager_stop(mgr);
	linphone_core_manager_uninit(mgr);
}

void linphone_core_manager_destroy_after_stop_async(LinphoneCoreManager* mgr) {
	if (mgr->lc && !linphone_core_is_network_reachable(mgr->lc)) {
		int previousNbRegistrationOk = mgr->stat.number_of_LinphoneRegistrationOk;
		linphone_core_set_network_reachable(mgr->lc, TRUE);
		wait_for_until(mgr->lc, NULL, &mgr->stat.number_of_LinphoneRegistrationOk, previousNbRegistrationOk + 1, 2000);
	}
	linphone_core_manager_uninit_after_stop_async(mgr);
	linphone_core_manager_uninit(mgr);
	ms_free(mgr);
}

void linphone_core_manager_delete_chat_room (LinphoneCoreManager *mgr, LinphoneChatRoom *cr, bctbx_list_t *coresList) {
	stats mgrStats = mgr->stat;
	if (cr) {
		linphone_core_delete_chat_room(mgr->lc, cr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, mgrStats.number_of_LinphoneConferenceStateDeleted + 1, 10000));
	}
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

void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message){
	stats* counters;
	ms_message("New registration state %s for user id [%s] at proxy [%s]\n"
		   ,linphone_registration_state_to_string(cstate)
		   ,linphone_proxy_config_get_identity(cfg)
		   ,linphone_proxy_config_get_addr(cfg));
	counters = get_stats(lc);
	switch (cstate) {
	case LinphoneRegistrationNone:counters->number_of_LinphoneRegistrationNone++;break;
	case LinphoneRegistrationProgress:counters->number_of_LinphoneRegistrationProgress++;break;
	case LinphoneRegistrationOk:counters->number_of_LinphoneRegistrationOk++;break;
	case LinphoneRegistrationCleared:counters->number_of_LinphoneRegistrationCleared++;break;
	case LinphoneRegistrationFailed:counters->number_of_LinphoneRegistrationFailed++;break;
	default:
		BC_FAIL("unexpected event");break;
	}
}

void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg){
	stats* counters = get_stats(lc);

	if (linphone_call_is_op_configured(call)) {
		LinphoneCallLog *calllog = linphone_call_get_call_log(call);
		char* to=linphone_address_as_string(linphone_call_log_get_to_address(calllog));
		char* from=linphone_address_as_string(linphone_call_log_get_from_address(calllog));

		const LinphoneAddress *to_addr = linphone_call_get_to_address(call);
		const LinphoneAddress *remote_addr = linphone_call_get_remote_address(call);
		//const LinphoneAddress *from_addr = linphone_call_get_from_address(call);
		BC_ASSERT_PTR_NOT_NULL(to_addr);
		//BC_ASSERT_PTR_NOT_NULL(from_addr);
		BC_ASSERT_PTR_NOT_NULL(remote_addr);

		ms_message(" %s call from [%s] to [%s], new state is [%s]"	,linphone_call_log_get_dir(calllog)==LinphoneCallIncoming?"Incoming":"Outgoing"
																		,from
																		,to
																		,linphone_call_state_to_string(cstate));
		ms_free(to);
		ms_free(from);
	}

	switch (cstate) {
	case LinphoneCallIncomingReceived:counters->number_of_LinphoneCallIncomingReceived++;break;
	case LinphoneCallPushIncomingReceived:counters->number_of_LinphoneCallPushIncomingReceived++;break;
	case LinphoneCallOutgoingInit :counters->number_of_LinphoneCallOutgoingInit++;break;
	case LinphoneCallOutgoingProgress :counters->number_of_LinphoneCallOutgoingProgress++;break;
	case LinphoneCallOutgoingRinging :counters->number_of_LinphoneCallOutgoingRinging++;break;
	case LinphoneCallOutgoingEarlyMedia :counters->number_of_LinphoneCallOutgoingEarlyMedia++;break;
	case LinphoneCallConnected :counters->number_of_LinphoneCallConnected++;break;
	case LinphoneCallStreamsRunning :counters->number_of_LinphoneCallStreamsRunning++;break;
	case LinphoneCallPausing :counters->number_of_LinphoneCallPausing++;break;
	case LinphoneCallPaused :counters->number_of_LinphoneCallPaused++;break;
	case LinphoneCallResuming :counters->number_of_LinphoneCallResuming++;break;
	case LinphoneCallRefered :counters->number_of_LinphoneCallRefered++;break;
	case LinphoneCallError :counters->number_of_LinphoneCallError++;break;
	case LinphoneCallEnd :counters->number_of_LinphoneCallEnd++;break;
	case LinphoneCallPausedByRemote :counters->number_of_LinphoneCallPausedByRemote++;break;
	case LinphoneCallUpdatedByRemote :counters->number_of_LinphoneCallUpdatedByRemote++;break;
	case LinphoneCallIncomingEarlyMedia :counters->number_of_LinphoneCallIncomingEarlyMedia++;break;
	case LinphoneCallUpdating :counters->number_of_LinphoneCallUpdating++;break;
	case LinphoneCallReleased :counters->number_of_LinphoneCallReleased++;break;
	case LinphoneCallEarlyUpdating: counters->number_of_LinphoneCallEarlyUpdating++;break;
	case LinphoneCallEarlyUpdatedByRemote: counters->number_of_LinphoneCallEarlyUpdatedByRemote++;break;
	default:
		BC_FAIL("unexpected event");break;
	}
}

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

void is_composing_received(LinphoneCore *lc, LinphoneChatRoom *room) {
	stats *counters = get_stats(lc);
	if (linphone_chat_room_is_remote_composing(room)) {
		counters->number_of_LinphoneIsComposingActiveReceived++;
	} else {
		counters->number_of_LinphoneIsComposingIdleReceived++;
	}
}

void new_subscription_requested(LinphoneCore *lc, LinphoneFriend *lf, const char *url){
	stats* counters;
	const LinphoneAddress *addr = linphone_friend_get_address(lf);
	struct addrinfo *ai;
	const char *domain;
	char *ipaddr;

	if (addr != NULL) {
		char* from=linphone_address_as_string(addr);
		ms_message("New subscription request from [%s] url [%s]",from,url);
		ms_free(from);
	}
	counters = get_stats(lc);
	counters->number_of_NewSubscriptionRequest++;

	domain = linphone_address_get_domain(addr);
	if (domain[0] == '['){
		ipaddr = ms_strdup(domain+1);
		ipaddr[strlen(ipaddr)] = '\0';
	}else ipaddr = ms_strdup(domain);
	ai = bctbx_ip_address_to_addrinfo(strchr(domain, ':') != NULL ? AF_INET6 : AF_INET, SOCK_DGRAM, ipaddr, 4444);
	ms_free(ipaddr);
	if (ai){/* this SUBSCRIBE comes from friend without registered SIP account, don't attempt to subscribe, it will fail*/
		ms_message("Disabling subscription because friend has numeric host.");
		linphone_friend_enable_subscribes(lf, FALSE);
		bctbx_freeaddrinfo(ai);
	}


	linphone_core_add_friend(lc,lf); /*accept subscription*/
}

void notify_presence_received(LinphoneCore *lc, LinphoneFriend * lf) {
	stats* counters;
	unsigned int i;
	const LinphoneAddress *addr = linphone_friend_get_address(lf);
	if (addr != NULL) {
		char* from=linphone_address_as_string(addr);
		ms_message("New Notify request from [%s] ",from);
		ms_free(from);
	}
	counters = get_stats(lc);
	counters->number_of_NotifyPresenceReceived++;
	counters->last_received_presence = linphone_friend_get_presence_model(lf);
	if (linphone_presence_model_get_basic_status(counters->last_received_presence) == LinphonePresenceBasicStatusOpen) {
		counters->number_of_LinphonePresenceBasicStatusOpen++;
	} else if (linphone_presence_model_get_basic_status(counters->last_received_presence) == LinphonePresenceBasicStatusClosed) {
		counters->number_of_LinphonePresenceBasicStatusClosed++;
	} else {
		ms_error("Unexpected basic status [%i]",linphone_presence_model_get_basic_status(counters->last_received_presence));
	}
	if (counters->last_received_presence && linphone_presence_model_get_nb_activities(counters->last_received_presence) > 0) {
		for (i=0;counters->last_received_presence&&i<linphone_presence_model_get_nb_activities(counters->last_received_presence); i++) {
			LinphonePresenceActivity *activity = linphone_presence_model_get_nth_activity(counters->last_received_presence, i);
			switch (linphone_presence_activity_get_type(activity)) {
				case LinphonePresenceActivityAppointment:
					counters->number_of_LinphonePresenceActivityAppointment++; break;
				case LinphonePresenceActivityAway:
					counters->number_of_LinphonePresenceActivityAway++; break;
				case LinphonePresenceActivityBreakfast:
					counters->number_of_LinphonePresenceActivityBreakfast++; break;
				case LinphonePresenceActivityBusy:
					counters->number_of_LinphonePresenceActivityBusy++; break;
				case LinphonePresenceActivityDinner:
					counters->number_of_LinphonePresenceActivityDinner++; break;
				case LinphonePresenceActivityHoliday:
					counters->number_of_LinphonePresenceActivityHoliday++; break;
				case LinphonePresenceActivityInTransit:
					counters->number_of_LinphonePresenceActivityInTransit++; break;
				case LinphonePresenceActivityLookingForWork:
					counters->number_of_LinphonePresenceActivityLookingForWork++; break;
				case LinphonePresenceActivityLunch:
					counters->number_of_LinphonePresenceActivityLunch++; break;
				case LinphonePresenceActivityMeal:
					counters->number_of_LinphonePresenceActivityMeal++; break;
				case LinphonePresenceActivityMeeting:
					counters->number_of_LinphonePresenceActivityMeeting++; break;
				case LinphonePresenceActivityOnThePhone:
					counters->number_of_LinphonePresenceActivityOnThePhone++; break;
				case LinphonePresenceActivityOther:
					counters->number_of_LinphonePresenceActivityOther++; break;
				case LinphonePresenceActivityPerformance:
					counters->number_of_LinphonePresenceActivityPerformance++; break;
				case LinphonePresenceActivityPermanentAbsence:
					counters->number_of_LinphonePresenceActivityPermanentAbsence++; break;
				case LinphonePresenceActivityPlaying:
					counters->number_of_LinphonePresenceActivityPlaying++; break;
				case LinphonePresenceActivityPresentation:
					counters->number_of_LinphonePresenceActivityPresentation++; break;
				case LinphonePresenceActivityShopping:
					counters->number_of_LinphonePresenceActivityShopping++; break;
				case LinphonePresenceActivitySleeping:
					counters->number_of_LinphonePresenceActivitySleeping++; break;
				case LinphonePresenceActivitySpectator:
					counters->number_of_LinphonePresenceActivitySpectator++; break;
				case LinphonePresenceActivitySteering:
					counters->number_of_LinphonePresenceActivitySteering++; break;
				case LinphonePresenceActivityTravel:
					counters->number_of_LinphonePresenceActivityTravel++; break;
				case LinphonePresenceActivityTV:
					counters->number_of_LinphonePresenceActivityTV++; break;
				case LinphonePresenceActivityUnknown:
					counters->number_of_LinphonePresenceActivityUnknown++; break;
				case LinphonePresenceActivityVacation:
					counters->number_of_LinphonePresenceActivityVacation++; break;
				case LinphonePresenceActivityWorking:
					counters->number_of_LinphonePresenceActivityWorking++; break;
				case LinphonePresenceActivityWorship:
					counters->number_of_LinphonePresenceActivityWorship++; break;
			}
		}
	} else {
		if (linphone_presence_model_get_basic_status(counters->last_received_presence) == LinphonePresenceBasicStatusOpen)
			counters->number_of_LinphonePresenceActivityOnline++;
		else
			counters->number_of_LinphonePresenceActivityOffline++;
	}
}

void notify_presence_received_for_uri_or_tel(LinphoneCore *lc, LinphoneFriend *lf, const char *uri_or_tel, const LinphonePresenceModel *presence) {
	stats *counters = get_stats(lc);
	ms_message("Presence notification for URI or phone number [%s]", uri_or_tel);
	counters->number_of_NotifyPresenceReceivedForUriOrTel++;
}

void _check_friend_result_list(LinphoneCore *lc, const bctbx_list_t *resultList, const unsigned int index, const char* uri, const char* phone) {
	if (index >= bctbx_list_size(resultList)) {
		ms_error("Attempt to access result to an outbound index");
		return;
	}
	const LinphoneSearchResult *sr = bctbx_list_nth_data(resultList, index);
	const LinphoneFriend *lf = linphone_search_result_get_friend(sr);
	if (lf || linphone_search_result_get_address(sr)) {
		const LinphoneAddress *la = (linphone_search_result_get_address(sr)) ?
			linphone_search_result_get_address(sr) : linphone_friend_get_address(lf);
		if (la) {
			char* fa = linphone_address_as_string(la);
			BC_ASSERT_STRING_EQUAL(fa , uri);
			free(fa);
			return;
		} else if (phone) {
			const LinphonePresenceModel *presence = linphone_friend_get_presence_model_for_uri_or_tel(lf, phone);
			if (BC_ASSERT_PTR_NOT_NULL(presence)) {
				char *contact = linphone_presence_model_get_contact(presence);
				BC_ASSERT_STRING_EQUAL(contact, uri);
				free(contact);
				return;
			}
		}
	} else {
		const bctbx_list_t *callLog = linphone_core_get_call_logs(lc);
		const bctbx_list_t *f;
		for (f = callLog ; f != NULL ; f = bctbx_list_next(f)) {
			LinphoneCallLog *log = (LinphoneCallLog*)(f->data);
			const LinphoneAddress *addr = (linphone_call_log_get_dir(log) == LinphoneCallIncoming) ?
			linphone_call_log_get_from_address(log) : linphone_call_log_get_to_address(log);
			if (addr) {
				char *addrUri = linphone_address_as_string_uri_only(addr);
				if (addrUri && strcmp(addrUri, uri) == 0) {
					bctbx_free(addrUri);
					return;
				}
				if (addrUri) bctbx_free(addrUri);
			}
		}
	}
	BC_ASSERT(FALSE);
	ms_error("Address NULL and Presence NULL");
}

void linphone_transfer_state_changed(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) {
	LinphoneCallLog *clog = linphone_call_get_call_log(transfered);
	char* to=linphone_address_as_string(linphone_call_log_get_to_address(clog));
	char* from=linphone_address_as_string(linphone_call_log_get_to_address(clog));
	stats* counters;
	ms_message("Transferred call from [%s] to [%s], new state is [%s]",from,to,linphone_call_state_to_string(new_call_state));
	ms_free(to);
	ms_free(from);

	counters = get_stats(lc);
	switch (new_call_state) {
	case LinphoneCallOutgoingInit :counters->number_of_LinphoneTransferCallOutgoingInit++;break;
	case LinphoneCallOutgoingProgress :counters->number_of_LinphoneTransferCallOutgoingProgress++;break;
	case LinphoneCallOutgoingRinging :counters->number_of_LinphoneTransferCallOutgoingRinging++;break;
	case LinphoneCallOutgoingEarlyMedia :counters->number_of_LinphoneTransferCallOutgoingEarlyMedia++;break;
	case LinphoneCallConnected :counters->number_of_LinphoneTransferCallConnected++;break;
	case LinphoneCallStreamsRunning :counters->number_of_LinphoneTransferCallStreamsRunning++;break;
	case LinphoneCallError :counters->number_of_LinphoneTransferCallError++;break;
	default:
		BC_FAIL("unexpected event");break;
	}
}

void info_message_received(LinphoneCore *lc, LinphoneCall* call, const LinphoneInfoMessage *msg){
	stats* counters = get_stats(lc);

	if (counters->last_received_info_message) {
		linphone_info_message_unref(counters->last_received_info_message);
	}
	counters->last_received_info_message=linphone_info_message_copy(msg);
	counters->number_of_inforeceived++;
}

void linphone_subscription_state_change(LinphoneCore *lc, LinphoneEvent *lev, LinphoneSubscriptionState state) {
	stats* counters = get_stats(lc);
	LinphoneCoreManager *mgr=get_manager(lc);
	LinphoneContent* content;
	const LinphoneAddress* from_addr = linphone_event_get_from(lev);
	char* from = linphone_address_as_string(from_addr);
	content = linphone_core_create_content(lc);
	linphone_content_set_type(content,"application");
	linphone_content_set_subtype(content,"somexml2");
	linphone_content_set_buffer(content,(const uint8_t *)notify_content,strlen(notify_content));

	ms_message("Subscription state [%s] from [%s]",linphone_subscription_state_to_string(state),from);
	ms_free(from);

	switch(state){
		case LinphoneSubscriptionNone:
		break;
		case LinphoneSubscriptionIncomingReceived:
			counters->number_of_LinphoneSubscriptionIncomingReceived++;
			mgr->lev=lev;
		break;
		case LinphoneSubscriptionOutgoingProgress:
			counters->number_of_LinphoneSubscriptionOutgoingProgress++;
		break;
		case LinphoneSubscriptionPending:
			counters->number_of_LinphoneSubscriptionPending++;
		break;
		case LinphoneSubscriptionActive:
			counters->number_of_LinphoneSubscriptionActive++;
			if (linphone_event_get_subscription_dir(lev)==LinphoneSubscriptionIncoming){
				mgr->lev=lev;
				if(strcmp(linphone_event_get_name(lev), "conference") == 0) {
					// TODO : Get LocalConfEventHandler and call handler->subscribeReceived(lev)
				} else {
					linphone_event_notify(lev,content);
				}
			}
		break;
		case LinphoneSubscriptionTerminated:
			counters->number_of_LinphoneSubscriptionTerminated++;
			mgr->lev=NULL;
		break;
		case LinphoneSubscriptionError:
			counters->number_of_LinphoneSubscriptionError++;
			mgr->lev=NULL;
		break;
		case LinphoneSubscriptionExpiring:
			counters->number_of_LinphoneSubscriptionExpiring++;
			mgr->lev=NULL;
		break;
	}
	linphone_content_unref(content);
}

void linphone_notify_received(LinphoneCore *lc, LinphoneEvent *lev, const char *eventname, const LinphoneContent *content){
	LinphoneCoreManager *mgr;
	const char * ua = linphone_event_get_custom_header(lev, "User-Agent");
	if (!BC_ASSERT_PTR_NOT_NULL(content)) return;
	if (!linphone_content_is_multipart(content) && (!ua ||  !strstr(ua, "flexisip"))) { /*disable check for full presence server support*/
		/*hack to disable content checking for list notify */
		BC_ASSERT_STRING_EQUAL(linphone_content_get_string_buffer(content), notify_content);
	}
	mgr = get_manager(lc);
	mgr->stat.number_of_NotifyReceived++;
}

void linphone_subscribe_received(LinphoneCore *lc, LinphoneEvent *lev, const char *eventname, const LinphoneContent *content) {
	LinphoneCoreManager *mgr = get_manager(lc);
	if (!mgr->decline_subscribe)
		linphone_event_accept_subscription(lev);
	else
		linphone_event_deny_subscription(lev, LinphoneReasonDeclined);
}

void linphone_publish_state_changed(LinphoneCore *lc, LinphoneEvent *ev, LinphonePublishState state) {
	stats* counters = get_stats(lc);
	const LinphoneAddress* from_addr = linphone_event_get_from(ev);
	char* from = linphone_address_as_string(from_addr);
	ms_message("Publish state [%s] from [%s]",linphone_publish_state_to_string(state),from);
	ms_free(from);
	switch(state){
	case LinphonePublishProgress: counters->number_of_LinphonePublishProgress++; break;
	case LinphonePublishOk:
		/*make sure custom header access API is working*/
		BC_ASSERT_PTR_NOT_NULL(linphone_event_get_custom_header(ev,"From"));
		counters->number_of_LinphonePublishOk++;
		break;
	case LinphonePublishError: counters->number_of_LinphonePublishError++; break;
	case LinphonePublishExpiring: counters->number_of_LinphonePublishExpiring++; break;
	case LinphonePublishCleared: counters->number_of_LinphonePublishCleared++;break;
	default:
		break;
	}

}

void linphone_configuration_status(LinphoneCore *lc, LinphoneConfiguringState status, const char *message) {
	stats* counters;
	ms_message("Configuring state = %i with message %s", status, message?message:"");

	counters = get_stats(lc);
	if (status == LinphoneConfiguringSkipped) {
		counters->number_of_LinphoneConfiguringSkipped++;
	} else if (status == LinphoneConfiguringFailed) {
		counters->number_of_LinphoneConfiguringFailed++;
	} else if (status == LinphoneConfiguringSuccessful) {
		counters->number_of_LinphoneConfiguringSuccessful++;
	}
}

void linphone_call_encryption_changed(LinphoneCore *lc, LinphoneCall *call, bool_t on, const char *authentication_token) {
	LinphoneCallLog *calllog = linphone_call_get_call_log(call);
	char* to=linphone_address_as_string(linphone_call_log_get_to_address(calllog));
	char* from=linphone_address_as_string(linphone_call_log_get_from_address(calllog));
	stats* counters;
	ms_message(" %s call from [%s] to [%s], is now [%s]",linphone_call_log_get_dir(calllog)==LinphoneCallIncoming?"Incoming":"Outgoing"
		   ,from
		   ,to
		   ,(on?"encrypted":"unencrypted"));
	ms_free(to);
	ms_free(from);
	counters = get_stats(lc);
	if (on)
		counters->number_of_LinphoneCallEncryptedOn++;
	else
		counters->number_of_LinphoneCallEncryptedOff++;
}

void dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf) {
	stats* counters = get_stats(lc);
	char** dst = &counters->dtmf_list_received;
	*dst = *dst ? ms_strcat_printf(*dst, "%c", dtmf) : ms_strdup_printf("%c", dtmf);
	counters->dtmf_count++;
}

bool_t rtcp_is_type(const mblk_t *m, rtcp_type_t type){
	const rtcp_common_header_t *ch=rtcp_get_common_header(m);
	return (ch!=NULL && rtcp_common_header_get_packet_type(ch)==type);
}

void rtcp_received(stats* counters, mblk_t *packet) {
	do{
		if (rtcp_is_type(packet, RTCP_RTPFB)){
			if (rtcp_RTPFB_get_type(packet) ==  RTCP_RTPFB_TMMBR) {
				counters->number_of_tmmbr_received++;
				counters->last_tmmbr_value_received = (int)rtcp_RTPFB_tmmbr_get_max_bitrate(packet);
			}
		}
	}while (rtcp_next_packet(packet));
	rtcp_rewind(packet);
}


void call_stats_updated(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallStats *lstats) {
	const int updated = _linphone_call_stats_get_updated(lstats);
	stats *counters = get_stats(lc);

	counters->number_of_LinphoneCallStatsUpdated++;
	if (updated & LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE) {
		counters->number_of_rtcp_received++;
		if (_linphone_call_stats_rtcp_received_via_mux(lstats)){
			counters->number_of_rtcp_received_via_mux++;
		}
		rtcp_received(counters, _linphone_call_stats_get_received_rtcp(lstats));
		BC_ASSERT_TRUE(_linphone_call_stats_has_received_rtcp(lstats));
	}
	if (updated & LINPHONE_CALL_STATS_SENT_RTCP_UPDATE ) {
		counters->number_of_rtcp_sent++;
		BC_ASSERT_TRUE(_linphone_call_stats_has_sent_rtcp(lstats));
	}
	if (updated & LINPHONE_CALL_STATS_PERIODICAL_UPDATE ) {
		const int tab_size = sizeof counters->audio_download_bandwidth / sizeof(int);

		LinphoneCallStats *call_stats;
		int index;

		int type = linphone_call_stats_get_type(lstats);
		if (type != LINPHONE_CALL_STATS_AUDIO && type != LINPHONE_CALL_STATS_VIDEO)
			return; // Avoid out of bounds if type is TEXT.

		index = (counters->current_bandwidth_index[type]++) % tab_size;
		if (type == LINPHONE_CALL_STATS_AUDIO) {
			call_stats = linphone_call_get_audio_stats(call);
			counters->audio_download_bandwidth[index] = (int)linphone_call_stats_get_download_bandwidth(call_stats);
			counters->audio_upload_bandwidth[index] = (int)linphone_call_stats_get_upload_bandwidth(call_stats);
		} else {
			call_stats = linphone_call_get_video_stats(call);
			counters->video_download_bandwidth[index] = (int)linphone_call_stats_get_download_bandwidth(call_stats);
			counters->video_upload_bandwidth[index] = (int)linphone_call_stats_get_upload_bandwidth(call_stats);
		}
		linphone_call_stats_unref(call_stats);
	}
}

void liblinphone_tester_chat_message_msg_state_changed(LinphoneChatMessage *msg, LinphoneChatMessageState state) {
	LinphoneCore *lc = linphone_chat_message_get_core(msg);
	stats *counters = get_stats(lc);
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
		case LinphoneChatMessageStateFileTransferInProgress:
			counters->number_of_LinphoneMessageFileTransferInProgress++;
			return;
	}
	ms_error("Unexpected state [%s] for msg [%p]",linphone_chat_message_state_to_string(state), msg);
}

void liblinphone_tester_chat_room_msg_sent(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *msg) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneMessageSent++;
}

void liblinphone_tester_x3dh_user_created(LinphoneCore *lc, const bool_t status, const char* userId, const char *info) {
	stats *counters = get_stats(lc);
	if (status == TRUE) {
		counters->number_of_X3dhUserCreationSuccess++;
	} else {
		counters->number_of_X3dhUserCreationFailure++;
	}
}

void liblinphone_tester_chat_message_ephemeral_timer_started (LinphoneChatMessage *msg) {
	LinphoneCore *lc = linphone_chat_message_get_core(msg);
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneMessageEphemeralTimerStarted++;
}

void liblinphone_tester_chat_message_ephemeral_deleted (LinphoneChatMessage *msg) {
	LinphoneCore *lc = linphone_chat_message_get_core(msg);
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneMessageEphemeralDeleted++;
}

/*
 * function called when the file transfer is initiated. file content should be feed into object LinphoneContent
 * */
LinphoneBuffer * tester_file_transfer_send(LinphoneChatMessage *msg, LinphoneContent* content, size_t offset, size_t size){
	LinphoneBuffer *lb;
	size_t file_size;
	size_t size_to_send;
	uint8_t *buf;
	FILE *file_to_send = linphone_content_get_user_data(content);

	// If a file path is set, we should NOT call the on_send callback !
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_file_transfer_filepath(msg));
	BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int, "%d");

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
		linphone_content_set_user_data(content, NULL);
	}
	lb = linphone_buffer_new_from_data(buf, size_to_send);
	ms_free(buf);
	return lb;
}

/**
 * function invoked to report file transfer progress.
 * */
void file_transfer_progress_indication(LinphoneChatMessage *msg, LinphoneContent* content, size_t offset, size_t total) {
	const LinphoneAddress *from_address = linphone_chat_message_get_from_address(msg);
	const LinphoneAddress *to_address = linphone_chat_message_get_to_address(msg);
	int progress = (int)((offset * 100)/total);
	LinphoneCore *lc = linphone_chat_message_get_core(msg);
	stats *counters = get_stats(lc);
	char *address = linphone_address_as_string(linphone_chat_message_is_outgoing(msg) ? to_address : from_address);

	BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int, "%d");
	bctbx_message(
		"File transfer  [%d%%] %s of type [%s/%s] %s [%s] \n",
		progress,
		linphone_chat_message_is_outgoing(msg) ? "sent" : "received",
		linphone_content_get_type(content),
		linphone_content_get_subtype(content),
		linphone_chat_message_is_outgoing(msg) ? "to" : "from",
		address
	);
	counters->progress_of_LinphoneFileTransfer = progress;
	if (progress == 100) {
		counters->number_of_LinphoneFileTransferDownloadSuccessful++;
	}
	free(address);
}

/**
 * function invoked when a file transfer is received.
 * */
void file_transfer_received(LinphoneChatMessage *msg, LinphoneContent* content, const LinphoneBuffer *buffer){
	FILE* file=NULL;
	char *receive_file = NULL;

	// If a file path is set, we should NOT call the on_recv callback !
	BC_ASSERT_PTR_NULL(linphone_chat_message_get_file_transfer_filepath(msg));
	BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), LinphoneChatMessageStateFileTransferInProgress, int, "%d");

	receive_file = bc_tester_file("receive_file.dump");
	if (!linphone_content_get_user_data(content)) {
		/*first chunk, creating file*/
		file = fopen(receive_file,"wb");
		linphone_content_set_user_data(content,(void*)file); /*store fd for next chunks*/
	}

	file = (FILE*)linphone_content_get_user_data(content);
	BC_ASSERT_PTR_NOT_NULL(file);
	if (linphone_buffer_is_empty(buffer)) { /* tranfer complete */
		struct stat st;

		linphone_content_set_user_data(content, NULL);
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

void global_state_changed(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message) {
	stats *counters = get_stats(lc);
	switch (gstate) {
		case LinphoneGlobalOn:
			counters->number_of_LinphoneGlobalOn++;
			break;
		case LinphoneGlobalReady:
			counters->number_of_LinphoneGlobalReady++;
			break;
		case LinphoneGlobalOff:
			counters->number_of_LinphoneGlobalOff++;
			break;
		case LinphoneGlobalStartup:
			counters->number_of_LinphoneGlobalStartup++;
			break;
		case LinphoneGlobalShutdown:
			counters->number_of_LinphoneGlobalShutdown++;
			break;
		case LinphoneGlobalConfiguring:
			counters->number_of_LinphoneGlobalConfiguring++;
			break;
	}
}

void first_call_started(LinphoneCore *lc) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCoreFirstCallStarted++;
}

void last_call_ended(LinphoneCore *lc) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCoreLastCallEnded++;
}

void audio_device_changed(LinphoneCore *lc, LinphoneAudioDevice *device) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCoreAudioDeviceChanged++;
}

void audio_devices_list_updated(LinphoneCore *lc) {
	stats *counters = get_stats(lc);
	counters->number_of_LinphoneCoreAudioDevicesListUpdated++;
}

void setup_sdp_handling(const LinphoneCallTestParams* params, LinphoneCoreManager* mgr ){
	if( params->sdp_removal ){
		sal_default_set_sdp_handling(linphone_core_get_sal(mgr->lc), SalOpSDPSimulateRemove);
	} else if( params->sdp_simulate_error ){
		sal_default_set_sdp_handling(linphone_core_get_sal(mgr->lc), SalOpSDPSimulateError);
	}
}

/*
 * CAUTION this function is error prone. you should not use it anymore in new tests.
 * Creating callee call params before the call is actually received is not the good way
 * to use the Liblinphone API. Indeed, call params used for receiving calls shall be created by linphone_core_create_call_params() by passing
 * the call object for which params are to be created.
 * This function should be used only in test case where the programmer exactly knows the caller params, and then can deduce how
 * callee params will be set by linphone_core_create_call_params().
 * This function was developped at a time where the use of the API about incoming params was not yet clarified.
 * Tests relying on this function are then not testing the correct way to use the api (through linphone_core_create_call_params()), and so
 * it is not a so good idea to build new tests based on this function.
**/
bool_t call_with_params2(LinphoneCoreManager* caller_mgr
						,LinphoneCoreManager* callee_mgr
						, const LinphoneCallTestParams *caller_test_params
						, const LinphoneCallTestParams *callee_test_params
						, bool_t build_callee_params) {
	int retry=0;
	stats initial_caller=caller_mgr->stat;
	stats initial_callee=callee_mgr->stat;
	bool_t result=FALSE;
	LinphoneCallParams *caller_params = caller_test_params->base;
	LinphoneCallParams *callee_params = callee_test_params->base;
	bool_t did_receive_call;
	LinphoneCall *callee_call=NULL;
	LinphoneCall *caller_call=NULL;

	/* TODO: This should be handled correctly inside the liblinphone library but meanwhile handle this here. */
	linphone_core_manager_wait_for_stun_resolution(caller_mgr);
	linphone_core_manager_wait_for_stun_resolution(callee_mgr);

	setup_sdp_handling(caller_test_params, caller_mgr);
	setup_sdp_handling(callee_test_params, callee_mgr);

	if (!caller_params){
		BC_ASSERT_PTR_NOT_NULL((caller_call=linphone_core_invite_address(caller_mgr->lc,callee_mgr->identity)));
	}else{
		BC_ASSERT_PTR_NOT_NULL((caller_call=linphone_core_invite_address_with_params(caller_mgr->lc,callee_mgr->identity,caller_params)));
	}

	BC_ASSERT_PTR_NULL(linphone_call_get_remote_params(caller_call)); /*assert that remote params are NULL when no response is received yet*/
	//test ios simulator needs more time, 3s plus for connectng the network
	did_receive_call = wait_for_until(callee_mgr->lc
				,caller_mgr->lc
				,&callee_mgr->stat.number_of_LinphoneCallIncomingReceived
				,initial_callee.number_of_LinphoneCallIncomingReceived+1, 12000);
	BC_ASSERT_EQUAL(did_receive_call, !callee_test_params->sdp_simulate_error, int, "%d");

	sal_default_set_sdp_handling(linphone_core_get_sal(caller_mgr->lc), SalOpSDPNormal);
	sal_default_set_sdp_handling(linphone_core_get_sal(caller_mgr->lc), SalOpSDPNormal);

	if (!did_receive_call) return 0;


	if (linphone_core_get_calls_nb(callee_mgr->lc)<=1)
		BC_ASSERT_TRUE(linphone_core_is_incoming_invite_pending(callee_mgr->lc));
	BC_ASSERT_EQUAL(caller_mgr->stat.number_of_LinphoneCallOutgoingProgress,initial_caller.number_of_LinphoneCallOutgoingProgress+1, int, "%d");

	while (caller_mgr->stat.number_of_LinphoneCallOutgoingRinging!=(initial_caller.number_of_LinphoneCallOutgoingRinging + 1)
			&& caller_mgr->stat.number_of_LinphoneCallOutgoingEarlyMedia!=(initial_caller.number_of_LinphoneCallOutgoingEarlyMedia +1)
			&& retry++ < 100) {
			linphone_core_iterate(caller_mgr->lc);
			linphone_core_iterate(callee_mgr->lc);
			ms_usleep(20000);
	}


	BC_ASSERT_TRUE((caller_mgr->stat.number_of_LinphoneCallOutgoingRinging==initial_caller.number_of_LinphoneCallOutgoingRinging+1)
							||(caller_mgr->stat.number_of_LinphoneCallOutgoingEarlyMedia==initial_caller.number_of_LinphoneCallOutgoingEarlyMedia+1));

	if (linphone_core_get_calls_nb(callee_mgr->lc) == 1)
		BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call_remote_address(callee_mgr->lc)); /*only relevant if one call, otherwise, not always set*/
	callee_call=linphone_core_get_call_by_remote_address2(callee_mgr->lc,caller_mgr->identity);

	if(!linphone_core_get_current_call(caller_mgr->lc) || (!callee_call && !linphone_core_get_current_call(callee_mgr->lc)) /*for privacy case*/) {
		return 0;
	} else if (caller_mgr->identity){
		LinphoneAddress* callee_from=linphone_address_clone(caller_mgr->identity);
		linphone_address_set_port(callee_from,0); /*remove port because port is never present in from header*/

		if (linphone_call_params_get_privacy(linphone_call_get_current_params(linphone_core_get_current_call(caller_mgr->lc))) == LinphonePrivacyNone) {
			/*don't check in case of p asserted id*/
			if (!linphone_config_get_int(linphone_core_get_config(callee_mgr->lc),"sip","call_logs_use_asserted_id_instead_of_from",0))
				BC_ASSERT_TRUE(linphone_address_weak_equal(callee_from,linphone_call_get_remote_address(callee_call)));
		} else {
			BC_ASSERT_FALSE(linphone_address_weak_equal(callee_from,linphone_call_get_remote_address(linphone_core_get_current_call(callee_mgr->lc))));
		}
		linphone_address_unref(callee_from);
	}

	LinphoneCoreToneManagerStats *callee_stats = linphone_core_get_tone_manager_stats(callee_mgr->lc);
	if (callee_stats->number_of_startRingbackTone == callee_stats->number_of_stopRingbackTone) {
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(callee_mgr->lc)->number_of_startRingtone, callee_mgr->stat.number_of_LinphoneCallIncomingReceived, int, "%d");
	} else {
		// in this case, the call is currently in RingbackTone so the Ringtone should not start
		BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(callee_mgr->lc)->number_of_startRingtone, callee_mgr->stat.number_of_LinphoneCallIncomingReceived-1, int, "%d");
	}
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(caller_mgr->lc)->number_of_startRingbackTone, caller_mgr->stat.number_of_LinphoneCallOutgoingRinging, int, "%d");


	if (callee_params){
		linphone_call_accept_with_params(callee_call,callee_params);
	}else if (build_callee_params){
		LinphoneCallParams *default_params=linphone_core_create_call_params(callee_mgr->lc,callee_call);
		ms_message("Created default call params with video=%i", linphone_call_params_video_enabled(default_params));
		linphone_call_accept_with_params(callee_call,default_params);
		linphone_call_params_unref(default_params);
	}else if (callee_call) {
		linphone_call_accept(callee_call);
	} else {
		linphone_call_accept(linphone_core_get_current_call(callee_mgr->lc));
	}

	BC_ASSERT_TRUE(wait_for(callee_mgr->lc,caller_mgr->lc,&callee_mgr->stat.number_of_LinphoneCallConnected,initial_callee.number_of_LinphoneCallConnected+1));
	BC_ASSERT_TRUE(wait_for(callee_mgr->lc,caller_mgr->lc,&caller_mgr->stat.number_of_LinphoneCallConnected,initial_caller.number_of_LinphoneCallConnected+1));

	result = wait_for_until(callee_mgr->lc,caller_mgr->lc,&caller_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_caller.number_of_LinphoneCallStreamsRunning+1, 2000)
			&&
			wait_for_until(callee_mgr->lc,caller_mgr->lc,&callee_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_callee.number_of_LinphoneCallStreamsRunning+1, 2000);

	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(callee_mgr->lc)->number_of_stopRingtone, callee_mgr->stat.number_of_LinphoneCallIncomingReceived, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_tone_manager_stats(caller_mgr->lc)->number_of_stopRingbackTone, caller_mgr->stat.number_of_LinphoneCallOutgoingRinging, int, "%d");

	if (linphone_core_get_media_encryption(caller_mgr->lc) != LinphoneMediaEncryptionNone
		|| linphone_core_get_media_encryption(callee_mgr->lc) != LinphoneMediaEncryptionNone) {
		/*wait for encryption to be on, in case of zrtp or dtls, it can take a few seconds*/
		if (	(linphone_core_get_media_encryption(caller_mgr->lc) == LinphoneMediaEncryptionZRTP)
				|| (linphone_core_get_media_encryption(callee_mgr->lc) == LinphoneMediaEncryptionZRTP) /* if callee is ZRTP, wait for it */
				|| (linphone_core_get_media_encryption(caller_mgr->lc) == LinphoneMediaEncryptionDTLS))
			wait_for(callee_mgr->lc,caller_mgr->lc,&caller_mgr->stat.number_of_LinphoneCallEncryptedOn,initial_caller.number_of_LinphoneCallEncryptedOn+1);
		if ((linphone_core_get_media_encryption(callee_mgr->lc) == LinphoneMediaEncryptionZRTP)
			|| (linphone_core_get_media_encryption(callee_mgr->lc) == LinphoneMediaEncryptionDTLS)
			|| (linphone_core_get_media_encryption(caller_mgr->lc) == LinphoneMediaEncryptionZRTP)
			|| (linphone_core_get_media_encryption(caller_mgr->lc) == LinphoneMediaEncryptionDTLS) /*also take care of caller policy*/ )
			wait_for(callee_mgr->lc,caller_mgr->lc,&callee_mgr->stat.number_of_LinphoneCallEncryptedOn,initial_callee.number_of_LinphoneCallEncryptedOn+1);

		/* when caller is encryptionNone but callee is ZRTP, we expect ZRTP to take place */
		if ((linphone_core_get_media_encryption(caller_mgr->lc) == LinphoneMediaEncryptionNone)
			&& (linphone_core_get_media_encryption(callee_mgr->lc) == LinphoneMediaEncryptionZRTP)
			&& linphone_core_media_encryption_supported(caller_mgr->lc, LinphoneMediaEncryptionZRTP)) {
			const LinphoneCallParams* call_param = linphone_call_get_current_params(callee_call);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param), LinphoneMediaEncryptionZRTP, int, "%d");
			call_param = linphone_call_get_current_params(linphone_core_get_current_call(caller_mgr->lc));
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param), LinphoneMediaEncryptionZRTP, int, "%d");
		}else { /* otherwise, final status shall stick to caller core parameter */
			const LinphoneCallParams* call_param = linphone_call_get_current_params(callee_call);
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(caller_mgr->lc), int, "%d");
			call_param = linphone_call_get_current_params(linphone_core_get_current_call(caller_mgr->lc));
			BC_ASSERT_EQUAL(linphone_call_params_get_media_encryption(call_param),linphone_core_get_media_encryption(caller_mgr->lc), int, "%d");

		}
	}
	/*wait ice re-invite*/
	if (linphone_core_get_firewall_policy(caller_mgr->lc) == LinphonePolicyUseIce
			&& linphone_core_get_firewall_policy(callee_mgr->lc) == LinphonePolicyUseIce
			&& linphone_config_get_int(linphone_core_get_config(callee_mgr->lc), "sip", "update_call_when_ice_completed", TRUE)
			&& linphone_config_get_int(linphone_core_get_config(callee_mgr->lc), "sip", "update_call_when_ice_completed", TRUE)
			&& linphone_core_get_media_encryption(caller_mgr->lc) != LinphoneMediaEncryptionDTLS /*no ice-reinvite with DTLS*/) {
		BC_ASSERT_TRUE(wait_for(callee_mgr->lc,caller_mgr->lc,&caller_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_caller.number_of_LinphoneCallStreamsRunning+2));
		BC_ASSERT_TRUE(wait_for(callee_mgr->lc,caller_mgr->lc,&callee_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_callee.number_of_LinphoneCallStreamsRunning+2));

	} else if (linphone_core_get_firewall_policy(caller_mgr->lc) == LinphonePolicyUseIce) {
		/* check no ice re-invite received*/
		BC_ASSERT_FALSE(wait_for_until(callee_mgr->lc,caller_mgr->lc,&caller_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_caller.number_of_LinphoneCallStreamsRunning+2,2000));
		BC_ASSERT_FALSE(wait_for_until(callee_mgr->lc,caller_mgr->lc,&callee_mgr->stat.number_of_LinphoneCallStreamsRunning,initial_callee.number_of_LinphoneCallStreamsRunning+2,2000));
	}
	if (linphone_core_get_media_encryption(caller_mgr->lc) == LinphoneMediaEncryptionDTLS ) {
		LinphoneCall *call = linphone_core_get_current_call(caller_mgr->lc);
		AudioStream *astream = (AudioStream *)linphone_call_get_stream(call, LinphoneStreamTypeAudio);
#ifdef VIDEO_ENABLED
		VideoStream *vstream = (VideoStream *)linphone_call_get_stream(call, LinphoneStreamTypeVideo);
#endif
		if (astream)
			BC_ASSERT_TRUE(ms_media_stream_sessions_get_encryption_mandatory(&astream->ms.sessions));
#ifdef VIDEO_ENABLED
		if (vstream && video_stream_started(vstream))
			BC_ASSERT_TRUE(ms_media_stream_sessions_get_encryption_mandatory(&vstream->ms.sessions));
#endif

	}
	return result;
}

/*
 * CAUTION this function is error prone. you should not use it anymore in new tests.
 * Creating callee call params before the call is actually received is not the good way
 * to use the Liblinphone API. Indeed, call params used for receiving calls shall be created by linphone_core_create_call_params() by passing
 * the call object for which params are to be created.
 * This function should be used only in test case where the programmer exactly knows the caller params, and then can deduce how
 * callee params will be set by linphone_core_create_call_params().
 * This function was developped at a time where the use of the API about incoming params was not yet clarified.
 * Tests relying on this function are then not testing the correct way to use the api (through linphone_core_create_call_params()), and so
 * it is not a so good idea to build new tests based on this function.
**/
bool_t call_with_params(LinphoneCoreManager* caller_mgr
						,LinphoneCoreManager* callee_mgr
						,const LinphoneCallParams *caller_params
						,const LinphoneCallParams *callee_params){
	LinphoneCallTestParams caller_test_params = {0}, callee_test_params =  {0};
	caller_test_params.base = (LinphoneCallParams*)caller_params;
	callee_test_params.base = (LinphoneCallParams*)callee_params;
	return call_with_params2(caller_mgr,callee_mgr,&caller_test_params,&callee_test_params,FALSE);
}

/*
 * CAUTION this function is error prone. you should not use it anymore in new tests.
 * Creating callee call params before the call is actually received is not the good way
 * to use the Liblinphone API. Indeed, call params used for receiving calls shall be created by linphone_core_create_call_params() by passing
 * the call object for which params are to be created.
 * This function should be used only in test case where the programmer exactly knows the caller params, and then can deduce how
 * callee params will be set by linphone_core_create_call_params().
 * This function was developped at a time where the use of the API about incoming params was not yet clarified.
 * Tests relying on this function are then not testing the correct way to use the api (through linphone_core_create_call_params()), and so
 * it is not a so good idea to build new tests based on this function.
**/
bool_t call_with_test_params(LinphoneCoreManager* caller_mgr
				,LinphoneCoreManager* callee_mgr
				,const LinphoneCallTestParams *caller_test_params
				,const LinphoneCallTestParams *callee_test_params){
	return call_with_params2(caller_mgr,callee_mgr,caller_test_params,callee_test_params,FALSE);
}

bool_t call_with_caller_params(LinphoneCoreManager* caller_mgr,LinphoneCoreManager* callee_mgr, const LinphoneCallParams *params) {
	return call_with_params(caller_mgr,callee_mgr,params,NULL);
}

bool_t call(LinphoneCoreManager* caller_mgr,LinphoneCoreManager* callee_mgr){
	return call_with_params(caller_mgr,callee_mgr,NULL,NULL);
}

void end_call(LinphoneCoreManager *m1, LinphoneCoreManager *m2){
	int previous_count_1 = m1->stat.number_of_LinphoneCallEnd;
	int previous_count_2 = m2->stat.number_of_LinphoneCallEnd;
	linphone_core_terminate_all_calls(m1->lc);
	BC_ASSERT_TRUE(wait_for(m1->lc,m2->lc,&m1->stat.number_of_LinphoneCallEnd,previous_count_1+1));
	BC_ASSERT_TRUE(wait_for(m1->lc,m2->lc,&m2->stat.number_of_LinphoneCallEnd,previous_count_2+1));
	BC_ASSERT_TRUE(wait_for(m1->lc,m2->lc,&m1->stat.number_of_LinphoneCallReleased,previous_count_1+1));
	BC_ASSERT_TRUE(wait_for(m1->lc,m2->lc,&m2->stat.number_of_LinphoneCallReleased,previous_count_2+1));
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

const char *liblinphone_tester_get_empty_rc(void){
	if (liblinphone_tester_empty_rc_path == NULL){
		liblinphone_tester_empty_rc_path = bc_tester_res("rcfiles/empty_rc");
	}
	return liblinphone_tester_empty_rc_path;
}

/*
 * Copy file "from" to file "to".
 * Destination file is truncated if existing.
 * Return 0 on success, positive value on error.
 */
int liblinphone_tester_copy_file(const char *from, const char *to)
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

/*
 * Read a file and set its content in a buffer
 * caller must then free the buffer
 * return size read
 */
size_t liblinphone_tester_load_text_file_in_buffer(const char *filePath, char **buffer) {
	FILE *fp = fopen(filePath, "r");
	if ( fp == NULL )
	{
		ms_error("Can't open %s for reading: %s\n",filePath,strerror(errno));
		return 0;
	}

	/* get the size to read */
	if (fseek(fp, 0L, SEEK_END) == 0) {
		long bufsize = ftell(fp);
		*buffer = bctbx_malloc(sizeof(char) * (bufsize + 1)); // +1 to add a '\0'
		/* rewind */
		fseek(fp, 0L, SEEK_SET);
		/* read */
		size_t readSize = fread(*buffer, sizeof(char), bufsize, fp);
		if ( ferror( fp ) != 0 ) {
			bctbx_free(*buffer);
			fclose(fp);
			return 0;
		} else {
			(*buffer)[readSize++] = '\0';
			fclose(fp);
			return readSize;
		}
	}
	fclose(fp);
	return 0;
}

static const int flowControlIntervalMs = 5000;
static const int flowControlThresholdMs = 40;

static int dummy_set_sample_rate(MSFilter *obj, void *data) {
	return 0;
}

static int dummy_get_sample_rate(MSFilter *obj, void *data) {
	int *n = (int*)data;
	*n = 44100;
	return 0;
}

static int dummy_set_nchannels(MSFilter *obj, void *data) {
	return 0;
}

static int dummy_get_nchannels(MSFilter *obj, void *data) {
	int *n = (int*)data;
	*n = 1;
	return 0;
}

static MSFilterMethod dummy_snd_card_methods[] = {
	{MS_FILTER_SET_SAMPLE_RATE, dummy_set_sample_rate},
	{MS_FILTER_GET_SAMPLE_RATE, dummy_get_sample_rate},
	{MS_FILTER_SET_NCHANNELS, dummy_set_nchannels},
	{MS_FILTER_GET_NCHANNELS, dummy_get_nchannels},
	{0,NULL}
};

struct _DummyOutputContext {
	MSFlowControlledBufferizer buffer;
	int samplerate;
	int nchannels;
	ms_mutex_t mutex;
};

typedef struct _DummyOutputContext DummyOutputContext;

static void dummy_snd_write_init(MSFilter *obj){
	DummyOutputContext *octx = (DummyOutputContext *)ms_new0(DummyOutputContext, 1);
	octx->samplerate = 44100;
	ms_flow_controlled_bufferizer_init(&octx->buffer, obj, octx->samplerate, 1);
	ms_mutex_init(&octx->mutex,NULL);
	
	octx->nchannels = 1;

	obj->data = octx;
}

static void dummy_snd_write_uninit(MSFilter *obj){
	DummyOutputContext *octx = (DummyOutputContext*)obj->data;
	ms_flow_controlled_bufferizer_uninit(&octx->buffer);
	ms_mutex_destroy(&octx->mutex);
	free(octx);
}

static void dummy_snd_write_process(MSFilter *obj) {

	DummyOutputContext *octx = (DummyOutputContext*) obj->data;

	ms_mutex_lock(&octx->mutex);
	// Retrieve data and put them in the filter bugffer ready to be played
	ms_flow_controlled_bufferizer_put_from_queue(&octx->buffer, obj->inputs[0]);
	ms_mutex_unlock(&octx->mutex);
}

MSFilterDesc dummy_filter_write_desc = {
	MS_FILTER_PLUGIN_ID,
	"DummyPlayer",
	"dummy player",
	MS_FILTER_OTHER,
	NULL,
	1,
	0,
	dummy_snd_write_init,
	NULL,
	dummy_snd_write_process,
	NULL,
	dummy_snd_write_uninit,
	dummy_snd_card_methods
};

static MSFilter *dummy_snd_card_create_writer(MSSndCard *card) {
	MSFilter *f = ms_factory_create_filter_from_desc(ms_snd_card_get_factory(card), &dummy_filter_write_desc);
	DummyOutputContext *octx = (DummyOutputContext*)(f->data);
	ms_flow_controlled_bufferizer_set_samplerate(&octx->buffer, octx->samplerate);
	ms_flow_controlled_bufferizer_set_nchannels(&octx->buffer, octx->nchannels);
	ms_flow_controlled_bufferizer_set_max_size_ms(&octx->buffer, flowControlThresholdMs);
	ms_flow_controlled_bufferizer_set_flow_control_interval_ms(&octx->buffer, flowControlIntervalMs);
	return f;
}

struct _DummyInputContext {
	queue_t q;
	MSFlowControlledBufferizer buffer;
	int samplerate;
	int nchannels;
	ms_mutex_t mutex;
};

typedef struct _DummyInputContext DummyInputContext;

static void dummy_snd_read_init(MSFilter *obj){
	DummyInputContext *ictx = (DummyInputContext *)ms_new0(DummyInputContext, 1);
	ictx->samplerate = 44100;
	ms_flow_controlled_bufferizer_init(&ictx->buffer, obj, ictx->samplerate, 1);
	ms_mutex_init(&ictx->mutex,NULL);
	qinit(&ictx->q);
	
	ictx->nchannels = 1;

	obj->data = ictx;
}

static void dummy_snd_read_uninit(MSFilter *obj){
	DummyInputContext *ictx = (DummyInputContext*)obj->data;

	flushq(&ictx->q,0);
	ms_flow_controlled_bufferizer_uninit(&ictx->buffer);
	ms_mutex_destroy(&ictx->mutex);

	free(ictx);
}

static void dummy_snd_read_process(MSFilter *obj) {

	DummyInputContext *ictx = (DummyInputContext*) obj->data;

	mblk_t *m;

	ms_mutex_lock(&ictx->mutex);
	// Retrieve data and put them in the filter output queue
	while ((m = getq(&ictx->q)) != NULL) {
		ms_queue_put(obj->outputs[0], m);
	}
	ms_mutex_unlock(&ictx->mutex);
}

MSFilterDesc dummy_filter_read_desc = {
	MS_FILTER_PLUGIN_ID,
	"DummyRecorder",
	"dummy recorder",
	MS_FILTER_OTHER,
	NULL,
	0,
	1,
	dummy_snd_read_init,
	NULL,
	dummy_snd_read_process,
	NULL,
	dummy_snd_read_uninit,
	dummy_snd_card_methods
};

static MSFilter *dummy_snd_card_create_reader(MSSndCard *card) {
	MSFilter *f = ms_factory_create_filter_from_desc(ms_snd_card_get_factory(card), &dummy_filter_read_desc);

	DummyInputContext *ictx = (DummyInputContext*)(f->data);
	ms_flow_controlled_bufferizer_set_samplerate(&ictx->buffer, ictx->samplerate);
	ms_flow_controlled_bufferizer_set_nchannels(&ictx->buffer, ictx->nchannels);
	ms_flow_controlled_bufferizer_set_max_size_ms(&ictx->buffer, flowControlThresholdMs);
	ms_flow_controlled_bufferizer_set_flow_control_interval_ms(&ictx->buffer, flowControlIntervalMs);

	return f;
}

static void dummy_test_snd_card_detect(MSSndCardManager *m);

MSSndCardDesc dummy_test_snd_card_desc = {
	"dummyTest",
	dummy_test_snd_card_detect,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	dummy_snd_card_create_reader,
	dummy_snd_card_create_writer,
	NULL
};

static MSSndCard* create_dummy_test_snd_card(void) {
	MSSndCard* sndcard;
	sndcard = ms_snd_card_new(&dummy_test_snd_card_desc);
	sndcard->data = NULL;
	sndcard->name = ms_strdup(DUMMY_TEST_SOUNDCARD);
	sndcard->capabilities = MS_SND_CARD_CAP_PLAYBACK | MS_SND_CARD_CAP_CAPTURE;
	sndcard->latency = 0;
	sndcard->device_type = MS_SND_CARD_DEVICE_TYPE_BLUETOOTH;
	return sndcard;
}

static void dummy_test_snd_card_detect(MSSndCardManager *m) {
	ms_snd_card_manager_prepend_card(m, create_dummy_test_snd_card());
}

static void dummy2_test_snd_card_detect(MSSndCardManager *m);

MSSndCardDesc dummy2_test_snd_card_desc = {
	"dummyTest2",
	dummy2_test_snd_card_detect,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	dummy_snd_card_create_reader,
	dummy_snd_card_create_writer,
	NULL
};

static MSSndCard* create_dummy2_test_snd_card(void) {
	MSSndCard* sndcard;
	sndcard = ms_snd_card_new(&dummy2_test_snd_card_desc);
	sndcard->data = NULL;
	sndcard->name = ms_strdup(DUMMY2_TEST_SOUNDCARD);
	sndcard->capabilities = MS_SND_CARD_CAP_PLAYBACK | MS_SND_CARD_CAP_CAPTURE;
	sndcard->latency = 0;
	sndcard->device_type = MS_SND_CARD_DEVICE_TYPE_BLUETOOTH;
	return sndcard;
}

static void dummy2_test_snd_card_detect(MSSndCardManager *m) {
	ms_snd_card_manager_prepend_card(m, create_dummy2_test_snd_card());
}

static void dummy3_test_snd_card_detect(MSSndCardManager *m);

MSSndCardDesc dummy3_test_snd_card_desc = {
	"dummyTest3",
	dummy3_test_snd_card_detect,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	dummy_snd_card_create_reader,
	dummy_snd_card_create_writer,
	NULL
};

static MSSndCard* create_dummy3_test_snd_card(void) {
	MSSndCard* sndcard;
	sndcard = ms_snd_card_new(&dummy3_test_snd_card_desc);
	sndcard->data = NULL;
	sndcard->name = ms_strdup(DUMMY3_TEST_SOUNDCARD);
	sndcard->capabilities = MS_SND_CARD_CAP_PLAYBACK | MS_SND_CARD_CAP_CAPTURE;
	sndcard->latency = 0;
	sndcard->device_type = MS_SND_CARD_DEVICE_TYPE_BLUETOOTH;
	return sndcard;
}

static void dummy3_test_snd_card_detect(MSSndCardManager *m) {
	ms_snd_card_manager_prepend_card(m, create_dummy3_test_snd_card());
}

void set_lime_curve_tls(const int curveId, LinphoneCoreManager *manager, bool_t tls_auth_server, bool_t req) {
	if (curveId == 448) {
		// Change the curve setting before the server URL
		linphone_config_set_string(linphone_core_get_config(manager->lc),"lime","curve","c448");
		// changing the url will restart the encryption engine allowing to also use the changed curve config
		if (tls_auth_server == TRUE) {
			if (req==TRUE) {
				linphone_core_set_lime_x3dh_server_url(manager->lc, lime_server_c448_tlsauth_req_url);
			} else {
				linphone_core_set_lime_x3dh_server_url(manager->lc, lime_server_c448_tlsauth_opt_url);
			}
		} else {
			linphone_core_set_lime_x3dh_server_url(manager->lc, lime_server_c448_url);
		}
	} else {
		// Change the curve setting before the server URL
		linphone_config_set_string(linphone_core_get_config(manager->lc),"lime","curve","c25519");
		// changing the url will restart the encryption engine allowing to also use the changed curve config
		if (tls_auth_server == TRUE) {
			if (req == TRUE) {
				linphone_core_set_lime_x3dh_server_url(manager->lc, lime_server_c25519_tlsauth_req_url);
			} else {
				linphone_core_set_lime_x3dh_server_url(manager->lc, lime_server_c25519_tlsauth_opt_url);
			}
		} else {
			linphone_core_set_lime_x3dh_server_url(manager->lc, lime_server_c25519_url);
		}
	}
}

void set_lime_curve(const int curveId, LinphoneCoreManager *manager) {
	set_lime_curve_tls(curveId, manager, FALSE, FALSE);
}

void set_lime_curve_list_tls(const int curveId, bctbx_list_t *managerList, bool_t tls_auth_server, bool_t req) {
	bctbx_list_t *item = managerList;
	for (item = managerList; item; item = bctbx_list_next(item)) {
		set_lime_curve_tls(curveId, (LinphoneCoreManager *)(bctbx_list_get_data(item)), tls_auth_server, req);
	}
}

void set_lime_curve_list(const int curveId, bctbx_list_t *managerList) {
	set_lime_curve_list_tls (curveId, managerList, FALSE, FALSE);
}
