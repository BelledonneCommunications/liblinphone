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

#ifndef _SAL_ENUMS_H_
#define _SAL_ENUMS_H_

#define SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN 64
#define SAL_MEDIA_DESCRIPTION_MAX_ICE_FOUNDATION_LEN 32
#define SAL_MEDIA_DESCRIPTION_MAX_ICE_TYPE_LEN 6

#define SAL_MEDIA_DESCRIPTION_MAX_ICE_CANDIDATES 20

#define SAL_MEDIA_DESCRIPTION_MAX_ICE_REMOTE_CANDIDATES 2

#define SAL_MEDIA_DESCRIPTION_MAX_ICE_UFRAG_LEN 256
#define SAL_MEDIA_DESCRIPTION_MAX_ICE_PWD_LEN 256

#define SAL_CRYPTO_ALGO_MAX 4

/*sufficient for 256bit keys encoded in base 64*/
#define SAL_SRTP_KEY_SIZE 128

struct SalCustomSdpAttribute;

typedef struct SalCustomSdpAttribute SalCustomSdpAttribute;

typedef enum{
	SalStreamSendRecv,
	SalStreamSendOnly,
	SalStreamRecvOnly,
	SalStreamInactive
}SalStreamDir;

typedef enum {
	SalAudio,
	SalVideo,
	SalText,
	SalOther
} SalStreamType;

typedef enum{
	SalProtoRtpAvp,
	SalProtoRtpSavp,
	SalProtoRtpAvpf,
	SalProtoRtpSavpf,
	SalProtoUdpTlsRtpSavp,
	SalProtoUdpTlsRtpSavpf,
	SalProtoOther
}SalMediaProto;

typedef struct SalIceCandidate {
	char addr[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
	char raddr[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
	char foundation[SAL_MEDIA_DESCRIPTION_MAX_ICE_FOUNDATION_LEN];
	char type[SAL_MEDIA_DESCRIPTION_MAX_ICE_TYPE_LEN];
	unsigned int componentID;
	unsigned int priority;
	int port;
	int rport;
} SalIceCandidate;

typedef struct SalIceRemoteCandidate {
	char addr[SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN];
	int port;
} SalIceRemoteCandidate;

typedef struct SalSrtpCryptoAlgo {
	unsigned int tag;
	MSCryptoSuite algo;
	char master_key[SAL_SRTP_KEY_SIZE + 1];
} SalSrtpCryptoAlgo;

typedef enum {
	SalDtlsRoleInvalid,
	SalDtlsRoleIsServer,
	SalDtlsRoleIsClient,
	SalDtlsRoleUnset
} SalDtlsRole;

typedef enum {
	SalMulticastInactive=0,
	SalMulticastSender,
	SalMulticastReceiver,
	SalMulticastSenderReceiver
} SalMulticastRole;



#endif // ifndef _SAL_ENUMS_H_
