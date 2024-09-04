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

#ifndef _PRIVATE_STRUCTS_H_
#define _PRIVATE_STRUCTS_H_

#include <bctoolbox/map.h>

#ifdef HAVE_XML2
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#endif

#ifdef HAVE_SQLITE
#include "sqlite3.h"
#else
typedef struct _sqlite3 sqlite3;
#endif

#include "event/event-publish.h"
#include "linphone/core.h"
#include "linphone/core_utils.h"
#include "linphone/sipsetup.h"
#include "sal/event-op.h"
#include "sal/register-op.h"

struct _CallCallbackObj {
	LinphoneCallCbFunc _func;
	void *_user_data;
};

struct StunCandidate {
	char addr[64];
	int port;
};

struct _PortConfig {
	char multicast_ip[LINPHONE_IPADDR_SIZE];
	char multicast_bind_ip[LINPHONE_IPADDR_SIZE];
	int rtp_port;
	int rtcp_port;
};

struct _LinphoneProxyConfig {
	belle_sip_object_t base;
	LinphoneAccount *account;
	LinphoneAccountParams *edit;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneProxyConfig);

struct sip_config {
	char *contact;
	char *guessed_contact;
	int inc_timeout;                /*timeout after an un-answered incoming call is rejected*/
	int push_incoming_call_timeout; /*timeout after push incoming received if stream not received*/
	int in_call_timeout;            /*timeout after a call is hangup */
	int delayed_timeout;            /*timeout after a delayed call is resumed */
	unsigned int keepalive_period;  /* interval in ms between keep alive messages sent to the proxy server*/
	LinphoneSipTransports transports;
	int refresh_window_min; /*lower bound of the refresh window */
	int refresh_window_max; /*upper bound of the refresh window */
	bool_t guess_hostname;
	bool_t loopback_only;
	bool_t ipv6_enabled;
	bool_t sdp_200_ack;
	bool_t register_only_when_network_is_up;
	bool_t register_only_when_upnp_is_ok;
	bool_t ping_with_options;
	bool_t auto_net_state_mon;
	bool_t tcp_tls_keepalive;
	bool_t vfu_with_info;  /*use to enable vfu request using sip info*/
	bool_t save_auth_info; // if true, auth infos will be write in the config file when they are added to the list
};

struct rtp_config {
	int audio_rtp_min_port;
	int audio_rtp_max_port;
	int video_rtp_min_port;
	int video_rtp_max_port;
	int audio_jitt_comp; /*jitter compensation*/
	int video_jitt_comp; /*jitter compensation*/
	int nortp_timeout;
	int nortp_onhold_timeout;
	int disable_upnp;
	MSCryptoSuite *srtp_suites;
	LinphoneAVPFMode avpf_mode;
	bool_t rtp_no_xmit_on_audio_mute;
	/* stop rtp xmit when audio muted */
	bool_t audio_adaptive_jitt_comp_enabled;
	bool_t video_adaptive_jitt_comp_enabled;
	bool_t pad;
	char *audio_multicast_addr;
	bool_t audio_multicast_enabled;
	int audio_multicast_ttl;
	char *video_multicast_addr;
	int video_multicast_ttl;
	bool_t video_multicast_enabled;
	int text_rtp_min_port;
	int text_rtp_max_port;
};

struct net_config {
	char *nat_address;    /* may be IP or host name */
	char *nat_address_ip; /* ip translated from nat_address */
	struct addrinfo *stun_addrinfo;
	int download_bw;
	int upload_bw;
	int mtu;
	OrtpNetworkSimulatorParams netsim_params;
	bool_t nat_sdp_only;
};

struct net_state {
	bool_t global_state;
	bool_t user_state;
};

struct sound_config {
	struct _MSSndCard *ring_sndcard;  /* the playback sndcard currently used */
	struct _MSSndCard *play_sndcard;  /* the playback sndcard currently used */
	struct _MSSndCard *capt_sndcard;  /* the capture sndcard currently used */
	struct _MSSndCard *media_sndcard; /* the media sndcard currently used */
	struct _MSSndCard *lsd_card;      /* dummy playback card for Linphone Sound Daemon extension */
	const char **cards;
	int latency;         /* latency in samples of the current used sound device */
	float soft_play_lev; /*playback gain in db.*/
	float soft_mic_lev;  /*mic gain in db.*/
	char rec_lev;
	char play_lev;
	char ring_lev;
	char media_lev;
	char source;
	char *local_ring;
	char *remote_ring;
	char *ringback_tone;
	bool_t ec;
	bool_t ea;
	bool_t agc;
	bool_t disable_record_on_mute;
	bool_t mic_enabled;
};

struct codecs_config {
	MSList *audio_codecs; /* list of audio codecs in order of preference*/
	MSList *video_codecs;
	MSList *text_codecs;
	int dyn_pt;
	int dont_check_audio_codec_support;
	int dont_check_video_codec_support;
};

struct video_config {
	struct _MSWebCam *device;
	const char **cams;
	MSVideoSize vsize;
	MSVideoSize preview_vsize; /*is 0,0 if no forced preview size is set, in which case vsize field above is used.*/
	LinphoneVideoDefinition *vdef;
	LinphoneVideoDefinition *preview_vdef;
	float fps;
	bool_t capture;
	bool_t show_local;
	bool_t qrcode_decoder;
	bool_t display;
	bool_t selfview; /*during calls*/
	bool_t reuse_preview_source;
	bool_t retransmission_on_nack_enabled;
};

struct text_config {
	bool_t enabled;
	unsigned int keepalive_interval;
};

struct ui_config {
	int is_daemon;
	int is_applet;
	unsigned int timer_id; /* the timer id for registration */
};

struct autoreplier_config {
	int enabled;
	int after_seconds;   /* accept the call after x seconds*/
	int max_users;       /* maximum number of user that can call simultaneously */
	int max_rec_time;    /* the max time of incoming voice recorded */
	int max_rec_msg;     /* maximum number of recorded messages */
	const char *message; /* the path of the file to be played */
};

struct _LinphoneToneDescription {
	LinphoneToneID toneid; /*A tone type to play when this error arrives. This is played using tone generator*/
	char *audiofile;       /*An override audio file to play instead, when this error arrives*/
	/*Note that some tones are not affected to any error, in which case it is affected LinphoneReasonNone*/
};

struct _LinphoneTaskList {
	MSList *hooks;
};

struct _LinphoneCoreCbs {
	belle_sip_object_t base;
	LinphoneCoreVTable *vtable;
	bool_t autorelease;
};

struct _LCCallbackObj {
	LinphoneCoreCbFunc _func;
	void *_user_data;
};

struct _EcCalibrator {
	MSFactory *factory;
	ms_thread_t thread;
	MSSndCard *play_card, *capt_card;
	MSFilter *sndread, *det, *rec;
	MSFilter *play, *gen, *sndwrite;
	MSFilter *read_resampler, *write_resampler;
	MSTicker *ticker;
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
	LinphoneEcCalibrationCallback cb;
	void *cb_data;
	LinphoneEcCalibrationAudioInit audio_init_cb;
	LinphoneEcCalibrationAudioUninit audio_uninit_cb;
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
	int64_t acc;
	int delay;
	unsigned int rate;
	LinphoneEcCalibratorStatus status;
	bool_t freq1, freq2, freq3;
	uint64_t tone_start_time[3];
	bool_t play_cool_tones;
};

struct _EchoTester {
	MSFactory *factory;
	MSFilter *in, *out;
	MSSndCard *capture_card;
	MSSndCard *playback_card;
	MSTicker *ticker;
	unsigned int rate;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneContent);

struct _LinphoneBuffer {
	belle_sip_object_t base;
	void *user_data;
	uint8_t *content; /**< A pointer to the buffer content */
	size_t size;      /**< The size of the buffer content */
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneBuffer);

struct _LinphoneImNotifPolicy {
	belle_sip_object_t base;
	void *user_data;
	LinphoneCore *lc;
	bool_t send_is_composing;
	bool_t recv_is_composing;
	bool_t send_imdn_delivered;
	bool_t recv_imdn_delivered;
	bool_t send_imdn_delivery_error;
	bool_t recv_imdn_delivery_error;
	bool_t send_imdn_displayed;
	bool_t recv_imdn_displayed;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneImNotifPolicy);

/*****************************************************************************
 * XML-RPC interface                                                         *
 ****************************************************************************/

struct _LinphoneXmlRpcArg {
	LinphoneXmlRpcArgType type;
	union {
		int i;
		char *s;
		// bctbx_map_t *m;
		bctbx_list_t *l;
	} data;
};

struct _LinphoneXmlRpcRequestCbs {
	belle_sip_object_t base;
	void *user_data;
	LinphoneXmlRpcRequestCbsResponseCb response;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneXmlRpcRequestCbs);

struct _LinphoneXmlRpcRequest {
	belle_sip_object_t base;
	void *user_data;
	LinphoneXmlRpcRequestCbs *callbacks; // Deprecated, use a list of Cbs instead
	bctbx_list_t *callbacks_list;
	LinphoneXmlRpcRequestCbs *currentCbs;
	belle_sip_list_t *arg_list;
	char *content; /**< The string representation of the XML-RPC request */
	char *method;
	LinphoneXmlRpcStatus status;
	struct _LinphoneXmlRpcArg response;
	LinphoneCore *core;
	char *raw_response;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneXmlRpcRequest);

struct _LinphoneXmlRpcSession {
	belle_sip_object_t base;
	void *user_data;
	LinphoneCore *core;
	char *url;
	bool_t released;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneXmlRpcSession);

/*****************************************************************************
 * OTHER UTILITY FUNCTIONS                                                     *
 ****************************************************************************/

struct _LinphoneImEncryptionEngineCbs {
	belle_sip_object_t base;
	void *user_data;
	LinphoneImEncryptionEngineCbsIncomingMessageCb process_incoming_message;
	LinphoneImEncryptionEngineCbsOutgoingMessageCb process_outgoing_message;
	LinphoneImEncryptionEngineCbsIsEncryptionEnabledForFileTransferCb is_encryption_enabled_for_file_transfer;
	LinphoneImEncryptionEngineCbsGenerateFileTransferKeyCb generate_file_transfer_key;
	LinphoneImEncryptionEngineCbsDownloadingFileCb process_downloading_file;
	LinphoneImEncryptionEngineCbsUploadingFileCb process_uploading_file;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneImEncryptionEngineCbs);

struct _LinphoneImEncryptionEngine {
	belle_sip_object_t base;
	void *user_data;
	LinphoneCore *lc;
	LinphoneImEncryptionEngineCbs *callbacks;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneImEncryptionEngine);

struct _LinphoneRange {
	belle_sip_object_t base;
	void *user_data;
	int min;
	int max;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneRange);

struct _LinphoneTransports {
	belle_sip_object_t base;
	void *user_data;
	int udp_port;  /**< SIP/UDP port */
	int tcp_port;  /**< SIP/TCP port */
	int dtls_port; /**< SIP/DTLS port */
	int tls_port;  /**< SIP/TLS port */
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneTransports);

struct _LinphoneVideoActivationPolicy {
	belle_sip_object_t base;
	void *user_data;
	LinphoneMediaDirection accept_media_direction;
	bool_t automatically_initiate; /**<Whether video shall be automatically proposed for outgoing calls.*/
	bool_t automatically_accept;   /**<Whether video shall be automatically accepted for incoming calls*/
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneVideoActivationPolicy);

struct _VTableReference {
	LinphoneCoreCbs *cbs;
	bool_t valid;
	bool_t autorelease;
	bool_t internal;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneTunnelConfig);

struct _LinphoneErrorInfo {
	belle_sip_object_t base;
	LinphoneReason reason;
	char *protocol;    /* */
	int protocol_code; /*from SIP response*/
	char *phrase;      /*from SIP response*/
	char *warnings;    /*from SIP response*/
	char *full_string; /*concatenation of status_string + warnings*/
	int retry_after;
	struct _LinphoneErrorInfo *sub_ei;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneErrorInfo);

struct _LinphoneVideoDefinition {
	belle_sip_object_t base;
	void *user_data;
	unsigned int width;  /**< The width of the video */
	unsigned int height; /**< The height of the video */
	char *name;          /** The name of the video definition */
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneVideoDefinition);

struct _LinphoneUpdateCheck {
	LinphoneCore *lc;
	char *current_version;
};

namespace LinphonePrivate {
class Core;
};

#define LINPHONE_CORE_STRUCT_BASE_FIELDS                                                                               \
	MSFactory *factory;                                                                                                \
	MSList *vtable_refs;                                                                                               \
	int vtable_notify_recursion;                                                                                       \
	std::shared_ptr<LinphonePrivate::Sal> sal;                                                                         \
	void *platform_helper;                                                                                             \
	LinphoneGlobalState state;                                                                                         \
	struct _LpConfig *config;                                                                                          \
	MSList *default_audio_codecs;                                                                                      \
	MSList *default_video_codecs;                                                                                      \
	MSList *default_text_codecs;                                                                                       \
	net_config_t net_conf;                                                                                             \
	sip_config_t sip_conf;                                                                                             \
	rtp_config_t rtp_conf;                                                                                             \
	sound_config_t sound_conf;                                                                                         \
	video_config_t video_conf;                                                                                         \
	text_config_t text_conf;                                                                                           \
	codecs_config_t codecs_conf;                                                                                       \
	ui_config_t ui_conf;                                                                                               \
	autoreplier_config_t autoreplier_conf;                                                                             \
	LinphoneAccount *default_account;                                                                                  \
	MSList *friends_lists;                                                                                             \
	MSList *auth_info;                                                                                                 \
	LCCallbackObj preview_finished_cb;                                                                                 \
	MSList *queued_calls;                                                                                              \
	MSList *call_logs;                                                                                                 \
	int max_call_logs;                                                                                                 \
	int missed_calls;                                                                                                  \
	VideoPreview *previewstream;                                                                                       \
	LinphoneVideoDefinition *preview_video_definition_cache;                                                           \
	struct _MSEventQueue *msevq;                                                                                       \
	LinphoneRtpTransportFactories *rtptf;                                                                              \
	MSList *bl_reqs;                                                                                                   \
	MSList *subscribers;                                                                                               \
	int minutes_away;                                                                                                  \
	LinphonePresenceModel *presence_model;                                                                             \
	void *data;                                                                                                        \
	char *play_file;                                                                                                   \
	char *rec_file;                                                                                                    \
	uint64_t prevtime_ms;                                                                                              \
	int audio_bw;                                                                                                      \
	void *video_window_id;                                                                                             \
	void *preview_window_id;                                                                                           \
	time_t netup_time;                                                                                                 \
	struct _EcCalibrator *ecc;                                                                                         \
	struct _EchoTester *ect;                                                                                           \
	LinphoneTaskList hooks;                                                                                            \
	LinphoneConference *conf_ctx;                                                                                      \
	bctbx_list_t *plugin_list;                                                                                         \
	char *zrtp_secrets_cache;                                                                                          \
	char *user_certificates_path;                                                                                      \
	LinphoneVideoActivationPolicy *video_policy;                                                                       \
	LinphoneNatPolicy *nat_policy;                                                                                     \
	LinphoneImNotifPolicy *im_notif_policy;                                                                            \
	bool_t use_files;                                                                                                  \
	bool_t keep_stream_direction_for_rejected_stream;                                                                  \
	bool_t apply_nat_settings;                                                                                         \
	bool_t initial_subscribes_sent;                                                                                    \
	bool_t bl_refresh;                                                                                                 \
	bool_t preview_finished;                                                                                           \
	bool_t auto_net_state_mon;                                                                                         \
	net_state_t sip_network_state;                                                                                     \
	net_state_t media_network_state;                                                                                   \
	bool_t network_reachable_to_be_notified;                                                                           \
	bool_t use_preview_window;                                                                                         \
	bool_t network_last_status;                                                                                        \
	bool_t vtables_running;                                                                                            \
	bool_t send_call_stats_periodical_updates;                                                                         \
	bool_t forced_ice_relay;                                                                                           \
	bool_t short_turn_refresh;                                                                                         \
	MSRect qrcode_rect;                                                                                                \
	char localip4[LINPHONE_IPADDR_SIZE];                                                                               \
	char localip6[LINPHONE_IPADDR_SIZE];                                                                               \
	int device_rotation;                                                                                               \
	int max_calls;                                                                                                     \
	LinphoneTunnel *tunnel;                                                                                            \
	char *device_id;                                                                                                   \
	char *friends_db_file;                                                                                             \
	belle_http_request_listener_t *provisioning_http_listener;                                                         \
	belle_http_request_listener_t *base_contacts_list_http_listener;                                                   \
	LinphoneFriendList *base_contacts_list_for_synchronization;                                                        \
	MSList *tones;                                                                                                     \
	LinphoneReason chat_deny_code;                                                                                     \
	char *file_transfer_server;                                                                                        \
	const char **supported_formats;                                                                                    \
	LinphoneContent *log_collection_upload_information;                                                                \
	LinphoneCoreCbs *current_cbs;                                                                                      \
	LinphoneRingtonePlayer *ringtoneplayer;                                                                            \
	LinphoneVcardContext *vcard_context;                                                                               \
	bool_t zrtp_not_available_simulation;                                                                              \
	char *tls_cert;                                                                                                    \
	char *tls_key;                                                                                                     \
	char *ephemeral_version;                                                                                           \
	char *groupchat_version;                                                                                           \
	char *conference_version;                                                                                          \
	LinphoneAddress *default_rls_addr;                                                                                 \
	LinphoneImEncryptionEngine *im_encryption_engine;                                                                  \
	struct _LinphoneAccountCreatorService *default_ac_service;                                                         \
	MSBandwidthController *bw_controller;                                                                              \
	bctbx_list_t *supported_encryptions;                                                                               \
	bctbx_list_t *callsCache;                                                                                          \
	bool_t dns_set_by_app;                                                                                             \
	int auto_download_incoming_files_max_size;                                                                         \
	bool_t sender_name_hidden_in_forward_message;                                                                      \
	bool_t is_main_core;                                                                                               \
	bool_t has_already_started_once;                                                                                   \
	bool_t send_imdn_if_unregistered;                                                                                  \
	LinphonePushNotificationConfig *push_config;                                                                       \
	bool_t auto_download_incoming_voice_recordings;                                                                    \
	bool_t auto_download_incoming_icalendars;                                                                          \
	unsigned long iterate_thread_id;                                                                                   \
	bool_t record_aware;                                                                                               \
	bool_t auto_send_ringing;                                                                                          \
	int number_of_duplicated_messages;

#define LINPHONE_CORE_STRUCT_FIELDS                                                                                    \
	LINPHONE_CORE_STRUCT_BASE_FIELDS                                                                                   \
	sqlite3 *zrtp_cache_db;                                                                                            \
	bctbx_mutex_t zrtp_cache_db_mutex;                                                                                 \
	bool_t debug_storage;                                                                                              \
	void *system_context;                                                                                              \
	bool_t is_unreffing;                                                                                               \
	bool_t push_notification_enabled;                                                                                  \
	bool_t auto_iterate_enabled;                                                                                       \
	bool_t native_ringing_enabled;                                                                                     \
	bool_t vibrate_on_incoming_call;

#endif /* _PRIVATE_STRUCTS_H_ */
