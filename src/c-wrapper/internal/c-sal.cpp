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
This file contains SAL API functions that do not depend on the underlying implementation (like belle-sip).
**/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "c-wrapper/internal/c-sal.h"

#include <ctype.h>

const char *sal_multicast_role_to_string(SalMulticastRole role) {
	switch (role) {
		case SalMulticastInactive:
			return "inactive";
		case SalMulticastReceiver:
			return "receiver";
		case SalMulticastSender:
			return "sender";
		case SalMulticastSenderReceiver:
			return "sender-receiver";
	}
	return "INVALID";
}

const char *sal_transport_to_string(SalTransport transport) {
	switch (transport) {
		case SalTransportUDP:
			return "udp";
		case SalTransportTCP:
			return "tcp";
		case SalTransportTLS:
			return "tls";
		case SalTransportDTLS:
			return "dtls";
		default: {
			ms_fatal("Unexpected transport [%i]", transport);
			return NULL;
		}
	}
}

SalTransport sal_transport_parse(const char *param) {
	if (!param) return SalTransportUDP;
	if (strcasecmp("udp", param) == 0) return SalTransportUDP;
	if (strcasecmp("tcp", param) == 0) return SalTransportTCP;
	if (strcasecmp("tls", param) == 0) return SalTransportTLS;
	if (strcasecmp("dtls", param) == 0) return SalTransportDTLS;
	ms_error("Unknown transport type[%s], returning UDP", param);
	return SalTransportUDP;
}

/*
static bool_t fmtp_equals(const char *p1, const char *p2){
    if (p1 && p2 && strcmp(p1,p2)==0) return TRUE;
    if (p1==NULL && p2==NULL) return TRUE;
    return FALSE;
}
*/

const char *sal_auth_mode_to_string(SalAuthMode mode) {
	switch (mode) {
		case SalAuthModeBearer:
			return "bearer";
		case SalAuthModeHttpDigest:
			return "digest";
		case SalAuthModeTls:
			return "tls";
	}
	return "invalid";
}

SalAuthInfo *sal_auth_info_new() {
	return ms_new0(SalAuthInfo, 1);
}

SalAuthInfo *sal_auth_info_clone(const SalAuthInfo *auth_info) {
	SalAuthInfo *new_auth_info = sal_auth_info_new();
	new_auth_info->username = auth_info->username ? ms_strdup(auth_info->username) : NULL;
	new_auth_info->userid = auth_info->userid ? ms_strdup(auth_info->userid) : NULL;
	new_auth_info->realm = auth_info->realm ? ms_strdup(auth_info->realm) : NULL;
	new_auth_info->domain = auth_info->realm ? ms_strdup(auth_info->domain) : NULL;
	new_auth_info->password = auth_info->password ? ms_strdup(auth_info->password) : NULL;
	new_auth_info->algorithm = auth_info->algorithm ? ms_strdup(auth_info->algorithm) : NULL;
	return new_auth_info;
}

void sal_auth_info_delete(SalAuthInfo *auth_info) {
	if (auth_info->username) ms_free(auth_info->username);
	if (auth_info->userid) ms_free(auth_info->userid);
	if (auth_info->realm) ms_free(auth_info->realm);
	if (auth_info->domain) ms_free(auth_info->domain);
	if (auth_info->password) ms_free(auth_info->password);
	if (auth_info->ha1) ms_free(auth_info->ha1);
	if (auth_info->certificates) sal_certificates_chain_delete(auth_info->certificates);
	if (auth_info->key) sal_signing_key_delete(auth_info->key);
	if (auth_info->algorithm) ms_free(auth_info->algorithm);
	if (auth_info->authz_server) ms_free(auth_info->authz_server);
	ms_free(auth_info);
}

const char *sal_stream_type_to_string(SalStreamType type) {
	switch (type) {
		case SalAudio:
			return "audio";
		case SalVideo:
			return "video";
		case SalText:
			return "text";
		default:
			return "other";
	}
}

const char *sal_media_proto_to_string(SalMediaProto type) {
	switch (type) {
		case SalProtoRtpAvp:
			return "RTP/AVP";
		case SalProtoRtpSavp:
			return "RTP/SAVP";
		case SalProtoUdpTlsRtpSavp:
			return "UDP/TLS/RTP/SAVP";
		case SalProtoRtpAvpf:
			return "RTP/AVPF";
		case SalProtoRtpSavpf:
			return "RTP/SAVPF";
		case SalProtoUdpTlsRtpSavpf:
			return "UDP/TLS/RTP/SAVPF";
		default:
			return "unknown";
	}
}

SalMediaProto sal_media_proto_from_string(const char *type) {
	if (strcmp(type, "RTP/AVP") == 0) return SalProtoRtpAvp;
	else if (strcmp(type, "RTP/SAVP") == 0) return SalProtoRtpSavp;
	else if (strcmp(type, "UDP/TLS/RTP/SAVP") == 0) return SalProtoUdpTlsRtpSavp;
	else if (strcmp(type, "RTP/AVPF") == 0) return SalProtoRtpAvpf;
	else if (strcmp(type, "RTP/SAVPF") == 0) return SalProtoRtpSavpf;
	else if (strcmp(type, "UDP/TLS/RTP/SAVPF") == 0) return SalProtoUdpTlsRtpSavpf;
	else return SalProtoOther;
}

SalMediaProto linphone_media_encryption_to_sal_media_proto(const LinphoneMediaEncryption media_enc, const bool_t avpf) {
	if ((media_enc == LinphoneMediaEncryptionSRTP) && avpf) return SalProtoRtpSavpf;
	if (media_enc == LinphoneMediaEncryptionSRTP) return SalProtoRtpSavp;
	if ((media_enc == LinphoneMediaEncryptionDTLS) && avpf) return SalProtoUdpTlsRtpSavpf;
	if (media_enc == LinphoneMediaEncryptionDTLS) return SalProtoUdpTlsRtpSavp;
	if (avpf) return SalProtoRtpAvpf;
	return SalProtoRtpAvp;
}

LinphoneMediaEncryption sal_media_proto_to_linphone_media_encryption(const SalMediaProto proto,
                                                                     const bool_t haveZrtpHash) {
	if ((proto == SalProtoRtpSavpf) || (proto == SalProtoRtpSavp)) return LinphoneMediaEncryptionSRTP;
	if ((proto == SalProtoUdpTlsRtpSavpf) || (proto == SalProtoUdpTlsRtpSavp)) return LinphoneMediaEncryptionDTLS;
	if (haveZrtpHash) return LinphoneMediaEncryptionZRTP;
	return LinphoneMediaEncryptionNone;
}

const char *sal_stream_dir_to_string(SalStreamDir type) {
	switch (type) {
		case SalStreamSendRecv:
			return "sendrecv";
		case SalStreamSendOnly:
			return "sendonly";
		case SalStreamRecvOnly:
			return "recvonly";
		case SalStreamInactive:
			return "inactive";
		default:
			return "unknown";
	}
}

const char *sal_media_record_to_string(SalMediaRecord record) {
	switch (record) {
		case SalMediaRecordOff:
			return "off";
		case SalMediaRecordOn:
			return "on";
		case SalMediaRecordPaused:
			return "paused";
		case SalMediaRecordNone:
			return "none";
	}
	return "unknown";
}

const char *sal_reason_to_string(const SalReason reason) {
	switch (reason) {
		case SalReasonDeclined:
			return "SalReasonDeclined";
		case SalReasonBusy:
			return "SalReasonBusy";
		case SalReasonRedirect:
			return "SalReasonRedirect";
		case SalReasonTemporarilyUnavailable:
			return "SalReasonTemporarilyUnavailable";
		case SalReasonNotFound:
			return "SalReasonNotFound";
		case SalReasonDoNotDisturb:
			return "SalReasonDoNotDisturb";
		case SalReasonConditionalRequestFailed:
			return "SalReasonConditionalRequestFailed";
		case SalReasonUnsupportedContent:
			return "SalReasonUnsupportedContent";
		case SalReasonBadEvent:
			return "SalReasonBadEvent";
		case SalReasonForbidden:
			return "SalReasonForbidden";
		case SalReasonUnknown:
			return "SalReasonUnknown";
		case SalReasonServiceUnavailable:
			return "SalReasonServiceUnavailable";
		case SalReasonNotAcceptable:
			return "SalReasonNotAcceptable";
		default:
			return "Unkown reason";
	}
}

const char *sal_presence_status_to_string(const SalPresenceStatus status) {
	switch (status) {
		case SalPresenceOffline:
			return "SalPresenceOffline";
		case SalPresenceOnline:
			return "SalPresenceOnline";
		case SalPresenceBusy:
			return "SalPresenceBusy";
		case SalPresenceBerightback:
			return "SalPresenceBerightback";
		case SalPresenceAway:
			return "SalPresenceAway";
		case SalPresenceOnthephone:
			return "SalPresenceOnthephone";
		case SalPresenceOuttolunch:
			return "SalPresenceOuttolunch";
		case SalPresenceDonotdisturb:
			return "SalPresenceDonotdisturb";
		case SalPresenceMoved:
			return "SalPresenceMoved";
		case SalPresenceAltService:
			return "SalPresenceAltService";
		default:
			return "unknown";
	}
}
const char *sal_privacy_to_string(SalPrivacy privacy) {
	switch (privacy) {
		case SalPrivacyUser:
			return "user";
		case SalPrivacyHeader:
			return "header";
		case SalPrivacySession:
			return "session";
		case SalPrivacyId:
			return "id";
		case SalPrivacyNone:
			return "none";
		case SalPrivacyCritical:
			return "critical";
		default:
			return NULL;
	}
}

static int line_get_value(const char *input, const char *key, char *value, size_t value_size, size_t *read) {
	const char *end = strchr(input, '\n');
	char line[256] = {0};
	char key_candidate[256]; // key_candidate array must have the same size of line array to avoid potential invalid
	                         // writes
	char *equal;
	size_t len;

	if (!end) len = strlen(input);
	else len = (size_t)(end + 1 - input);
	*read = len;
	strncpy(line, input, MIN(len, sizeof(line)));

	equal = strchr(line, '=');
	if (!equal) return FALSE;
	*equal = '\0';

	if (sscanf(line, "%s", key_candidate) != 1) return FALSE;
	if (strcasecmp(key, key_candidate) != 0) return FALSE;

	equal++;
	if (strlen(equal) >= value_size) equal[value_size - 1] = '\0';
	if (sscanf(equal, "%s", value) != 1) return FALSE;
	return TRUE;
}

int sal_lines_get_value(const char *data, const char *key, char *value, size_t value_size) {
	size_t read = 0;

	do {
		if (line_get_value(data, key, value, value_size, &read)) return TRUE;
		data += read;
	} while (read != 0);
	return FALSE;
}
