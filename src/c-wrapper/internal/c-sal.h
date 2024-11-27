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

/**
 This header files defines the Signaling Abstraction Layer.
 The purpose of this layer is too allow experiment different call signaling
 protocols and implementations under linphone, for example SIP, JINGLE...
**/

#ifndef _L_C_SAL_H_
#define _L_C_SAL_H_

#ifdef __cplusplus
#include <map>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bctoolbox/crypto.h"
#include "belle-sip/belle-sip.h"
#include "linphone/types.h"
#include "mediastreamer2/mediastream.h"
#include "ortp/rtpsession.h"

#ifndef LINPHONE_PUBLIC
#if defined(_MSC_VER)
#ifdef LINPHONE_EXPORTS
#define LINPHONE_PUBLIC __declspec(dllexport)
#else
#define LINPHONE_PUBLIC __declspec(dllimport)
#endif
#else
#define LINPHONE_PUBLIC
#endif
#endif

/*Dirty hack, keep in sync with mediastreamer2/include/mediastream.h */
#ifndef PAYLOAD_TYPE_FLAG_CAN_RECV
#define PAYLOAD_TYPE_FLAG_CAN_RECV PAYLOAD_TYPE_USER_FLAG_1
#define PAYLOAD_TYPE_FLAG_CAN_SEND PAYLOAD_TYPE_USER_FLAG_2
#endif

struct SalAddress;

typedef struct SalAddress SalAddress;

struct SalBodyHandler;

typedef struct SalBodyHandler SalBodyHandler;

struct SalCustomHeader;

typedef struct SalCustomHeader SalCustomHeader;

struct SalCustomSdpAttribute;

typedef struct SalCustomSdpAttribute SalCustomSdpAttribute;

struct addrinfo;

typedef enum {
	SalTransportUDP = 0,  /*UDP*/
	SalTransportTCP = 1,  /*TCP*/
	SalTransportTLS = 2,  /*TLS*/
	SalTransportDTLS = 3, /*DTLS*/
} SalTransport;

typedef enum {
	SalStreamSendRecv = 0,
	SalStreamSendOnly = 1,
	SalStreamRecvOnly = 2,
	SalStreamInactive = 3
} SalStreamDir;

typedef enum { SalAudio = 0, SalVideo = 1, SalText = 2, SalOther = 3 } SalStreamType;

typedef enum {
	SalProtoRtpAvp = 0,
	SalProtoRtpSavp = 1,
	SalProtoRtpAvpf = 2,
	SalProtoRtpSavpf = 3,
	SalProtoUdpTlsRtpSavp = 4,
	SalProtoUdpTlsRtpSavpf = 5,
	SalProtoOther = 6
} SalMediaProto;

typedef enum {
	SalDtlsRoleInvalid = 0,
	SalDtlsRoleIsServer = 1,
	SalDtlsRoleIsClient = 2,
	SalDtlsRoleUnset = 3
} SalDtlsRole;

typedef enum {
	SalMulticastInactive = 0,
	SalMulticastSender = 1,
	SalMulticastReceiver = 2,
	SalMulticastSenderReceiver = 3
} SalMulticastRole;

typedef enum {
	SalMediaRecordNone = 0,
	SalMediaRecordOff = 1,
	SalMediaRecordOn = 2,
	SalMediaRecordPaused = 3
} SalMediaRecord;

#define SAL_MEDIA_DESCRIPTION_UNCHANGED 0x00
#define SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED (1)
#define SAL_MEDIA_DESCRIPTION_CODEC_CHANGED (1 << 1)
#define SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED (1 << 2)
#define SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED (1 << 3)
#define SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED (1 << 4)
#define SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED                                                                  \
	(1 << 5) /* use to notify when switching from multicast to unicast*/
#define SAL_MEDIA_DESCRIPTION_FORCE_STREAM_RECONSTRUCTION (1 << 6) /* use force graph reconstruction*/
#define SAL_MEDIA_DESCRIPTION_ICE_RESTART_DETECTED (1 << 7)
#define SAL_MEDIA_DESCRIPTION_CONFIGURATION_CHANGED (1 << 8)
#define SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED (1 << 9)
#define SAL_MEDIA_DESCRIPTION_MIXER_TO_CLIENT_EXTENSION_CHANGED (1 << 10)
#define SAL_MEDIA_DESCRIPTION_CLIENT_TO_MIXER_EXTENSION_CHANGED (1 << 11)
#define SAL_MEDIA_DESCRIPTION_PTIME_CHANGED (1 << 12)
#define SAL_MEDIA_DESCRIPTION_BANDWIDTH_CHANGED (1 << 13)
#define SAL_MEDIA_DESCRIPTION_FRAME_MARKING_EXTENSION_CHANGED (1 << 14)
#define SAL_MEDIA_DESCRIPTION_CONTENT_CHANGED (1 << 15)
#define SAL_MEDIA_DESCRIPTION_DIRECTION_CHANGED (1 << 16)

#ifdef __cplusplus
extern "C" {
#endif

const char *sal_transport_to_string(SalTransport transport);
SalTransport sal_transport_parse(const char *);
/* Address manipulation API*/
SalAddress *sal_address_new_empty(void);
LINPHONE_PUBLIC SalAddress *sal_address_new(const char *uri);
LINPHONE_PUBLIC SalAddress *sal_address_clone(const SalAddress *addr);
SalAddress *sal_address_ref(SalAddress *addr);
LINPHONE_PUBLIC void sal_address_unref(SalAddress *addr);
const char *sal_address_get_scheme(const SalAddress *addr);
const char *sal_address_get_display_name(const SalAddress *addr);
const char *sal_address_get_display_name_unquoted(const SalAddress *addr);
const char *sal_address_get_username(const SalAddress *addr);
const char *sal_address_get_domain(const SalAddress *addr);
int sal_address_get_port(const SalAddress *addr);
bool_t sal_address_is_secure(const SalAddress *addr);
void sal_address_set_secure(SalAddress *addr, bool_t enabled);

SalTransport sal_address_get_transport(const SalAddress *addr);
const char *sal_address_get_transport_name(const SalAddress *addr);
const char *sal_address_get_method_param(const SalAddress *addr);

void sal_address_set_display_name(SalAddress *addr, const char *display_name);
void sal_address_set_username(SalAddress *addr, const char *username);
void sal_address_set_domain(SalAddress *addr, const char *host);
void sal_address_set_port(SalAddress *uri, int port);
void sal_address_clean(SalAddress *addr);
char *sal_address_as_string(const SalAddress *u);
char *sal_address_as_string_uri_only(const SalAddress *u);
SalAddress *sal_address_new_uri_only(const SalAddress *addr);
LINPHONE_PUBLIC void sal_address_set_param(SalAddress *u, const char *name, const char *value);
void sal_address_set_transport(SalAddress *addr, SalTransport transport);
void sal_address_set_transport_name(SalAddress *addr, const char *transport);
void sal_address_set_method_param(SalAddress *addr, const char *method);
void sal_address_set_params(SalAddress *addr, const char *params);
void sal_address_remove_param(const SalAddress *addr, const char *name);
bool_t sal_address_has_param(const SalAddress *addr, const char *name);
const char *sal_address_get_param(const SalAddress *addr, const char *name);
void sal_address_set_uri_param(SalAddress *addr, const char *name, const char *value);
void sal_address_set_uri_params(SalAddress *addr, const char *params);
bool_t sal_address_has_uri_param(const SalAddress *addr, const char *name);
const char *sal_address_get_uri_param(const SalAddress *addr, const char *name);
#ifdef __cplusplus
void sal_address_get_uri_params(const SalAddress *addr, std::map<std::string, std::string> &params);
void sal_address_get_params(const SalAddress *addr, std::map<std::string, std::string> &params);
#endif // __cplusplus
void sal_address_remove_uri_param(const SalAddress *addr, const char *name);
bool_t sal_address_is_ipv6(const SalAddress *addr);
bool_t sal_address_is_sip(const SalAddress *addr);
void sal_address_set_password(SalAddress *addr, const char *passwd);
const char *sal_address_get_password(const SalAddress *addr);
void sal_address_set_header(SalAddress *addr, const char *header_name, const char *header_value);
const char *sal_address_get_header(const SalAddress *addr, const char *name);

int sal_address_equals(const SalAddress *addr_a, const SalAddress *addr_b);
/* comparison of user, host, port only */
int sal_address_weak_equals(const SalAddress *addr_a, const SalAddress *addr_b);
/* comparison of uri part only */
int sal_address_uri_equals(const SalAddress *addr_a, const SalAddress *addr_b);

void sal_address_clean_params(const SalAddress *addr);

void sal_set_log_handler(BctbxLogFunc log_handler);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

const char *sal_stream_type_to_string(SalStreamType type);
const char *sal_media_proto_to_string(SalMediaProto type);
SalMediaProto sal_media_proto_from_string(const char *type);
SalMediaProto linphone_media_encryption_to_sal_media_proto(const LinphoneMediaEncryption media_enc, const bool_t avpf);
LinphoneMediaEncryption sal_media_proto_to_linphone_media_encryption(const SalMediaProto proto,
                                                                     const bool_t haveZrtpHash);
const char *sal_stream_dir_to_string(SalStreamDir type);
const char *sal_media_record_to_string(SalMediaRecord record);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
inline std::ostream &operator<<(std::ostream &ostr, SalStreamType type) {
	ostr << sal_stream_type_to_string(type);
	return ostr;
}

inline std::ostream &operator<<(std::ostream &ostr, SalMediaProto proto) {
	ostr << sal_media_proto_to_string(proto);
	return ostr;
}
#endif // __cplusplus

#define SAL_ENDPOINT_CANDIDATE_MAX 2

typedef enum {
	SalOpSDPNormal = 0,        /** No special handling for SDP */
	SalOpSDPSimulateError = 1, /** Will simulate an SDP parsing error */
	SalOpSDPSimulateRemove = 2 /** Will simulate no SDP in the op */
} SalOpSDPHandling;

#define SAL_STREAM_DESCRIPTION_PORT_TO_BE_DETERMINED 65536

#ifdef __cplusplus
extern "C" {
#endif

const char *sal_multicast_role_to_string(SalMulticastRole role);

#ifdef __cplusplus
}
#endif

typedef struct SalMessage {
	const char *from;
	const char *text;
	const char *url;
	const char *message_id;
	const char *content_type;
	time_t time;
} SalMessage;

typedef enum SalReason {
	SalReasonNone = 0, /*no error, please leave first so that it takes 0 value*/
	SalReasonDeclined = 1,
	SalReasonBusy = 2,
	SalReasonRedirect = 3,
	SalReasonTemporarilyUnavailable = 4,
	SalReasonRequestTimeout = 5,
	SalReasonNotFound = 6,
	SalReasonDoNotDisturb = 7,
	SalReasonUnsupportedContent = 8,
	SalReasonBadEvent = 9,
	SalReasonForbidden = 10,
	SalReasonUnknown = 11,
	SalReasonServiceUnavailable = 12,
	SalReasonRequestPending = 13,
	SalReasonUnauthorized = 14,
	SalReasonNotAcceptable = 15,
	SalReasonNoMatch = 16, /*equivalent to 481 Transaction/Call leg does not exist*/
	SalReasonMovedPermanently = 17,
	SalReasonGone = 18,
	SalReasonAddressIncomplete = 19,
	SalReasonNotImplemented = 20,
	SalReasonBadGateway = 21,
	SalReasonServerTimeout = 22,
	SalReasonSessionIntervalTooSmall = 23,
	SalReasonIOError = 24,
	SalReasonInternalError = 25,
	SalReasonConditionalRequestFailed = 26,
	SalReasonSasCheckRequired = 27
} SalReason;

const char *sal_reason_to_string(const SalReason reason);

typedef struct SalErrorInfo {
	SalReason reason;
	char *status_string;
	int protocol_code;
	char *warnings;
	char *protocol;
	char *full_string; /*concatenation of status_string + warnings*/
	int retry_after;
	struct SalErrorInfo *sub_sei;
} SalErrorInfo;

typedef enum SalPresenceStatus {
	SalPresenceOffline = 0,
	SalPresenceOnline = 1,
	SalPresenceBusy = 2,
	SalPresenceBerightback = 3,
	SalPresenceAway = 4,
	SalPresenceOnthephone = 5,
	SalPresenceOuttolunch = 6,
	SalPresenceDonotdisturb = 7,
	SalPresenceMoved = 8,
	SalPresenceAltService = 9,
	SalPresenceOnVacation = 10
} SalPresenceStatus;

struct _SalPresenceModel;
typedef struct _SalPresenceModel SalPresenceModel;

#ifdef __cplusplus
extern "C" {
#endif

const char *sal_presence_status_to_string(const SalPresenceStatus status);

#ifdef __cplusplus
}
#endif

typedef enum SalReferStatus { SalReferTrying = 0, SalReferSuccess = 1, SalReferFailed = 2 } SalReferStatus;

typedef enum SalSubscribeStatus {
	SalSubscribeNone = 0,
	SalSubscribePending = 1,
	SalSubscribeActive = 2,
	SalSubscribeTerminated = 3
} SalSubscribeStatus;

typedef enum SalMessageDeliveryStatus {
	SalMessageDeliveryInProgress = 0,
	SalMessageDeliveryDone = 1,
	SalMessageDeliveryFailed = 2
} SalMessageDeliveryStatus;

/**
 * auth event mode
 * */
typedef enum SalAuthMode { /*this enum must be same as belle_sip_auth_mode_t*/
	                       SalAuthModeHttpDigest =
	                           BELLE_SIP_AUTH_MODE_HTTP_DIGEST,      /** Digest authentication requested*/
	                       SalAuthModeTls = BELLE_SIP_AUTH_MODE_TLS, /** Client certificate requested*/
	                       SalAuthModeBearer = BELLE_SIP_AUTH_MODE_HTTP_BEARER
} SalAuthMode;

/**
 * Format of certificate buffer
 * */
typedef enum SalCertificateRawFormat { /*this enum must be same as belle_sip_certificate_raw_format_t*/
	                                   SAL_CERTIFICATE_RAW_FORMAT_PEM, /** PEM format*/
	                                   SAL_CERTIFICATE_RAW_FORMAT_DER  /** ASN.1 raw format*/
} SalCertificateRawFormat;

typedef struct SalAuthInfo {
	char *username;
	char *userid;
	char *password;
	char *realm;
	char *domain;
	char *ha1;
	char *algorithm;
	char *authz_server;
	SalAuthMode mode;
	belle_sip_signing_key_t *key;
	belle_sip_certificates_chain_t *certificates;
	belle_sip_bearer_token_t *bearer_token;
} SalAuthInfo;

#ifdef __cplusplus
extern "C" {
#endif

const char *sal_auth_mode_to_string(SalAuthMode mode);

SalAuthInfo *sal_auth_info_new(void);
// SalAuthInfo *sal_auth_info_clone(const SalAuthInfo *auth_info);
void sal_auth_info_delete(SalAuthInfo *auth_info);
LINPHONE_PUBLIC int sal_auth_compute_ha1(const char *userid, const char *realm, const char *password, char ha1[33]);
LINPHONE_PUBLIC int sal_auth_compute_ha1_for_algorithm(
    const char *userid, const char *realm, const char *password, char *ha1, size_t size, const char *algo);
SalAuthMode sal_auth_info_get_mode(const SalAuthInfo *auth_info);
belle_sip_signing_key_t *sal_auth_info_get_signing_key(const SalAuthInfo *auth_info);
belle_sip_certificates_chain_t *sal_auth_info_get_certificates_chain(const SalAuthInfo *auth_info);
void sal_auth_info_set_mode(SalAuthInfo *auth_info, SalAuthMode mode);

/** Parse a file containing either a certificate chain order in PEM format or a single DER cert
 * @param auth_info structure where to store the result of parsing
 * @param path path to certificate chain file
 * @param format either PEM or DER
 */
void sal_certificates_chain_parse_file(SalAuthInfo *auth_info, const char *path, SalCertificateRawFormat format);

/**
 * Parse a file containing either a private or public rsa key
 * @param auth_info structure where to store the result of parsing
 * @param passwd password (optionnal)
 */
void sal_signing_key_parse_file(SalAuthInfo *auth_info, const char *path, const char *passwd);

/** Parse a buffer containing either a certificate chain order in PEM format or a single DER cert
 * @param auth_info structure where to store the result of parsing
 * @param buffer the buffer to parse
 * @param format either PEM or DER
 */
void sal_certificates_chain_parse(SalAuthInfo *auth_info, const char *buffer, SalCertificateRawFormat format);

/**
 * Parse a buffer containing either a private or public rsa key
 * @param auth_info structure where to store the result of parsing
 * @param passwd password (optionnal)
 */
void sal_signing_key_parse(SalAuthInfo *auth_info, const char *buffer, const char *passwd);

/**
 * Parse a directory for files containing certificate with the given subject CNAME
 * @param[out]	certificate_pem				the address of a string to store the certificate in PEM format. To be freed
 * by caller
 * @param[out]	key_pem						the address of a string to store the key in PEM format. To be freed by
 * caller
 * @param[in]	path						directory to parse
 * @param[in]	subject						subject CNAME
 * @param[in]	format 						either PEM or DER
 * @param[in]	generate_certificate		if true, if matching certificate and key can't be found, generate it and
 * store it into the given dir, filename will be subject.pem
 * @param[in]	generate_dtls_fingerprint	if true and we have a certificate, generate the dtls fingerprint as
 * described in rfc4572
 */
void sal_certificates_chain_parse_directory(char **certificate_pem,
                                            char **key_pem,
                                            char **fingerprint,
                                            const char *path,
                                            const char *subject,
                                            SalCertificateRawFormat format,
                                            bool_t generate_certificate,
                                            bool_t generate_dtls_fingerprint);

void sal_certificates_chain_delete(belle_sip_certificates_chain_t *chain);
void sal_signing_key_delete(belle_sip_signing_key_t *key);

const SalErrorInfo *sal_error_info_none(void);
void sal_error_info_reset(SalErrorInfo *ei);
void sal_error_info_init_to_null(SalErrorInfo *sei);
void sal_error_info_set(
    SalErrorInfo *ei, SalReason reason, const char *protocol, int code, const char *status_string, const char *warning);

/*entity tag used for publish (see RFC 3903)*/

#ifdef __cplusplus
}
#endif

/*Call API*/
#ifdef __cplusplus
extern "C" {
#endif

/*presence publish */
// int sal_publish_presence(SalOp *op, const char *from, const char *to, int expires, SalPresenceModel *presence);
SalBodyHandler *sal_presence_model_create_body_handler(SalPresenceModel *presence);

#ifdef __cplusplus
}
#endif

/*privacy, must be in sync with LinphonePrivacyMask*/
typedef enum _SalPrivacy {
	SalPrivacyNone = 0x0,
	SalPrivacyUser = 0x1,
	SalPrivacyHeader = 0x2,
	SalPrivacySession = 0x4,
	SalPrivacyId = 0x8,
	SalPrivacyCritical = 0x10,
	SalPrivacyDefault = 0x8000
} SalPrivacy;
typedef unsigned int SalPrivacyMask;

#ifdef __cplusplus
extern "C" {
#endif

const char *sal_privacy_to_string(SalPrivacy privacy);

#ifdef __cplusplus
}
#endif

#define payload_type_set_number(pt, n) (pt)->user_data = (void *)((intptr_t)n);
#define payload_type_get_number(pt) ((int)(intptr_t)(pt)->user_data)

#ifdef __cplusplus
extern "C" {
#endif

SalCustomHeader *sal_custom_header_ref(SalCustomHeader *ch);
LINPHONE_PUBLIC void sal_custom_header_unref(SalCustomHeader *ch);
LINPHONE_PUBLIC SalCustomHeader *sal_custom_header_append(SalCustomHeader *ch, const char *name, const char *value);
const char *sal_custom_header_find(const SalCustomHeader *ch, const char *name);
bool_t sal_custom_sdp_attribute_is_present(const SalCustomSdpAttribute *csa, const char *name);
SalCustomHeader *sal_custom_header_remove(SalCustomHeader *ch, const char *name);
void sal_custom_header_free(SalCustomHeader *ch);
SalCustomHeader *sal_custom_header_clone(const SalCustomHeader *ch);

SalCustomSdpAttribute *sal_custom_sdp_attribute_append(SalCustomSdpAttribute *csa, const char *name, const char *value);
const char *sal_custom_sdp_attribute_find(const SalCustomSdpAttribute *csa, const char *name);
bool_t sal_custom_sdp_attribute_is_present(const SalCustomSdpAttribute *csa, const char *name);
void sal_custom_sdp_attribute_free(SalCustomSdpAttribute *csa);
SalCustomSdpAttribute *sal_custom_sdp_attribute_clone(const SalCustomSdpAttribute *csa);

/** deprecated. use sal_set_log_level instead **/
void sal_enable_log(void);
/** deprecated. use sal_set_log_level instead **/
void sal_disable_log(void);
void sal_set_log_level(OrtpLogLevel level);

unsigned int sal_get_random(void);
LINPHONE_PUBLIC char *sal_get_random_token(int size);
LINPHONE_PUBLIC char *sal_get_random_token_lowercase(int size);
unsigned char *sal_get_random_bytes(unsigned char *ret, size_t size);
#if 0
belle_sip_source_t * sal_create_timer(Sal *sal, belle_sip_source_func_t func, void *data, unsigned int timeout_value_ms, const char* timer_name);
void sal_cancel_timer(Sal *sal, belle_sip_source_t *timer);
#endif

// SalBodyHandler * sal_body_handler_new(const char *type, const char *subtype, void *data, size_t size, const char
// *encoding);
SalBodyHandler *sal_body_handler_new(void);
SalBodyHandler *sal_body_handler_new_from_buffer(const void *data, size_t size);
SalBodyHandler *sal_body_handler_ref(SalBodyHandler *body_handler);
void sal_body_handler_unref(SalBodyHandler *body_handler);
const char *sal_body_handler_get_type(const SalBodyHandler *body_handler);
void sal_body_handler_set_type(SalBodyHandler *body_handler, const char *type);
const char *sal_body_handler_get_subtype(const SalBodyHandler *body_handler);
void sal_body_handler_set_subtype(SalBodyHandler *body_handler, const char *subtype);
const belle_sip_list_t *sal_body_handler_get_content_type_parameters_names(const SalBodyHandler *body_handler);
const char *sal_body_handler_get_content_type_parameter(const SalBodyHandler *body_handler, const char *name);
void sal_body_handler_set_content_type_parameter(SalBodyHandler *body_handler,
                                                 const char *paramName,
                                                 const char *paramValue);
const char *sal_body_handler_get_encoding(const SalBodyHandler *body_handler);
void sal_body_handler_set_encoding(SalBodyHandler *body_handler, const char *encoding);
const char *sal_body_handler_get_content_disposition(const SalBodyHandler *body_handler);
void sal_body_handler_set_content_disposition(SalBodyHandler *body_handler, const char *disposition);
void sal_body_handler_add_header(SalBodyHandler *body_handler, const char *header_name, const char *header_value);
void *sal_body_handler_get_data(const SalBodyHandler *body_handler);
void sal_body_handler_set_data(SalBodyHandler *body_handler, void *data);
size_t sal_body_handler_get_size(const SalBodyHandler *body_handler);
void sal_body_handler_set_size(SalBodyHandler *body_handler, size_t size);
bool_t sal_body_handler_is_multipart(const SalBodyHandler *body_handler);
SalBodyHandler *sal_body_handler_get_part(const SalBodyHandler *body_handler, int idx);
const belle_sip_list_t *sal_body_handler_get_parts(const SalBodyHandler *body_handler);
SalBodyHandler *sal_body_handler_find_part_by_header(const SalBodyHandler *body_handler,
                                                     const char *header_name,
                                                     const char *header_value);
const char *sal_body_handler_get_header(const SalBodyHandler *body_handler, const char *header_name);
const belle_sip_list_t *sal_body_handler_get_headers(const SalBodyHandler *body_handler);

/*this function parses a document with key=value pairs separated by new lines, and extracts the value for a given key*/
int sal_lines_get_value(const char *data, const char *key, char *value, size_t value_size);

unsigned long sal_begin_background_task(const char *name, void (*max_time_reached)(void *), void *data);
void sal_end_background_task(unsigned long id);

char *sal_generate_uuid(void);

/*Some old equipment may not only rely on attribute sendonly/recvonly/sendrecv/inative*/

#ifdef __cplusplus
}
#endif

#endif
