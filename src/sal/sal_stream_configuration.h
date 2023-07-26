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

#ifndef _SAL_STREAM_CONFIGURATION_H_
#define _SAL_STREAM_CONFIGURATION_H_

#include <list>
#include <vector>

#include "ortp/rtpsession.h"

#include "c-wrapper/internal/c-sal.h"
#include "linphone/utils/general.h"

#define SAL_MEDIA_DESCRIPTION_MAX_ICE_ADDR_LEN 64
#define SAL_MEDIA_DESCRIPTION_MAX_ICE_FOUNDATION_LEN 32
#define SAL_MEDIA_DESCRIPTION_MAX_ICE_TYPE_LEN 6

typedef struct SalIceCandidate {
	std::string addr;
	std::string raddr;
	std::string foundation;
	std::string type;
	unsigned int componentID = 0;
	unsigned int priority = 0;
	int port = 0;
	int rport = 0;
} SalIceCandidate;

typedef struct SalIceRemoteCandidate {
	std::string addr;
	int port = 0;
} SalIceRemoteCandidate;

typedef struct SalSrtpCryptoAlgo {
	unsigned int tag = 0;
	MSCryptoSuite algo = MS_CRYPTO_SUITE_INVALID;
	std::string master_key;
} SalSrtpCryptoAlgo;

LINPHONE_BEGIN_NAMESPACE

class SalMediaDescription;
class SalStreamDescription;
class SalStreamBundle;
class MediaSessionPrivate;
class MS2Stream;
class IceService;
class SalCallOp;
class OfferAnswerEngine;

class LINPHONE_PUBLIC SalStreamConfiguration {
	friend class SalStreamDescription;
	friend class SalStreamBundle;
	friend class MediaSession;
	friend class MediaSessionPrivate;
	friend class MS2Stream;
	friend class IceService;
	friend class SalCallOp;
	friend class OfferAnswerEngine;

public:
	SalStreamConfiguration();
	SalStreamConfiguration(const SalStreamConfiguration &other);
	virtual ~SalStreamConfiguration();
	SalStreamConfiguration &operator=(const SalStreamConfiguration &other);
	int equal(const SalStreamConfiguration &other) const;
	bool operator==(const SalStreamConfiguration &other) const;
	bool operator!=(const SalStreamConfiguration &other) const;
	void disable();

	/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
	bool hasAvpf() const;
	bool hasIpv6() const;
	bool hasImplicitAvpf() const;

	/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
	bool hasSrtp() const;
	bool hasDtls() const;
	bool hasZrtp() const;
	bool hasLimeIk() const;

	bool hasZrtpHash() const;
	const uint8_t *getZrtpHash() const;

	const std::list<OrtpPayloadType *> &getPayloads() const;
	const int &getMaxRate() const;
	const std::string &getMid() const;
	const int &getMidRtpExtHeaderId() const;

	const SalStreamType &getType() const;
	const std::string getTypeAsString() const;
	const SalMediaProto &getProto() const;
	const std::string getProtoAsString() const;
	SalStreamDir getDirection() const;
	std::string getSdpString() const;

	const int &getMixerToClientExtensionId() const;
	const int &getClientToMixerExtensionId() const;
	const int &getFrameMarkingExtensionId() const;

	void replacePayloads(const std::list<OrtpPayloadType *> &newPayloads);

	void addToSdpMediaDescription(belle_sdp_media_description_t *media_desc) const;

	void mergeAcaps(const std::list<std::list<unsigned int>> &acaps);
	const std::list<std::list<unsigned int>> &getAcapIndexes() const;
	const unsigned int &getTcapIndex() const;

	static std::string cryptoToSdpValue(const SalSrtpCryptoAlgo &crypto);
	static SalSrtpCryptoAlgo fillStrpCryptoAlgoFromString(const std::string &value);

private:
	unsigned int index = 0;
	SalMediaProto proto = SalProtoRtpAvp;
	std::string proto_other;
	unsigned int rtp_ssrc = 0;
	std::string rtcp_cname;
	std::list<OrtpPayloadType *> payloads; /**<list of OrtpPayloadType */
	int ptime = 0;
	int maxptime = 0;
	SalStreamDir dir = SalStreamInactive;
	std::vector<SalSrtpCryptoAlgo> crypto;
	int max_rate = 0;
	bool bundle_only = false;
	bool implicit_rtcp_fb = false;
	bool pad[2]; /* Use me */
	OrtpRtcpFbConfiguration rtcp_fb{};
	OrtpRtcpXrConfiguration rtcp_xr{};
	SalCustomSdpAttribute *custom_sdp_attributes = nullptr;
	std::string mid;               /* Media line identifier for RTP bundle mode */
	int mid_rtp_ext_header_id = 0; /* Identifier for the MID field in the RTP extension header */
	int mixer_to_client_extension_id = 0;
	int client_to_mixer_extension_id = 0;
	int frame_marking_extension_id = 0;
	uint32_t conference_ssrc = 0;
	bool set_nortpproxy = false; /*Formely set by ICE to indicate to the proxy that it has nothing to do*/
	bool rtcp_mux = false;
	uint8_t haveZrtpHash = 0; /**< flag for zrtp hash presence */
	uint8_t haveLimeIk = 0;   /**< flag for lime Ik presence */
	uint8_t zrtphash[128] = {0};
	std::string dtls_fingerprint;
	SalDtlsRole dtls_role = SalDtlsRoleInvalid;
	int ttl = 0; /*for multicast -1 to disable*/

	bool delete_media_attributes = false;
	bool delete_session_attributes = false;
	unsigned int tcapIndex = 0;
	std::list<std::list<unsigned int>> acapIndexes;

	static bool isRecvOnly(const OrtpPayloadType *p);
	static bool isSamePayloadType(const OrtpPayloadType *p1, const OrtpPayloadType *p2);
	static bool isSamePayloadList(const std::list<OrtpPayloadType *> &l1, const std::list<OrtpPayloadType *> &l2);
	static std::string getSetupAttributeForDtlsRole(const SalDtlsRole &role);
	static SalDtlsRole getDtlsRoleFromSetupAttribute(const std::string setupAtte);

	bool isBundleOnly() const {
		return bundle_only;
	}
	void enableAvpfForStream();
	void disableAvpfForStream();
};

LINPHONE_END_NAMESPACE

#endif // ifndef _SAL_STREAM_CONFIGURATION_H_
