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

#include "sal/sal_stream_configuration.h"
#include "bellesip_sal/sal_impl.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/utils/utils.h"
#include "utils/payload-type-handler.h"

LINPHONE_BEGIN_NAMESPACE

#define keywordcmp(key, b) strncmp(key, b, sizeof(key))

SalStreamConfiguration::SalStreamConfiguration() {
	pad[0] = false;
	pad[1] = false;

	payloads.clear();
	crypto.clear();
}

SalStreamConfiguration::~SalStreamConfiguration() {
	PayloadTypeHandler::clearPayloadList(payloads);
}

SalStreamConfiguration::SalStreamConfiguration(const SalStreamConfiguration &other) {
	proto = other.proto;
	proto_other = other.proto_other;
	rtp_ssrc = other.rtp_ssrc;
	rtcp_cname = other.rtcp_cname;
	for (const auto &pt : other.payloads) {
		payloads.push_back(payload_type_clone(pt));
	}
	ptime = other.ptime;
	maxptime = other.maxptime;
	dir = other.dir;
	crypto = other.crypto;
	max_rate = other.max_rate;
	bundle_only = other.bundle_only;
	implicit_rtcp_fb = other.implicit_rtcp_fb;
	pad[0] = other.pad[0];
	pad[1] = other.pad[1];
	rtcp_fb = other.rtcp_fb;
	rtcp_xr = other.rtcp_xr;
	mid = other.mid;
	mid_rtp_ext_header_id = other.mid_rtp_ext_header_id;
	mixer_to_client_extension_id = other.mixer_to_client_extension_id;
	client_to_mixer_extension_id = other.client_to_mixer_extension_id;
	frame_marking_extension_id = other.frame_marking_extension_id;
	conference_ssrc = other.conference_ssrc;
	set_nortpproxy = other.set_nortpproxy;
	rtcp_mux = other.rtcp_mux;
	haveZrtpHash = other.haveZrtpHash;
	haveLimeIk = other.haveLimeIk;
	memcpy(zrtphash, other.zrtphash, sizeof(zrtphash));
	dtls_fingerprint = other.dtls_fingerprint;
	dtls_role = other.dtls_role;
	ttl = other.ttl;
	index = other.index;
	tcapIndex = other.tcapIndex;
	acapIndexes = other.acapIndexes;
	delete_media_attributes = other.delete_media_attributes;
	delete_session_attributes = other.delete_session_attributes;
}

SalStreamConfiguration &SalStreamConfiguration::operator=(const SalStreamConfiguration &other) {
	proto = other.proto;
	proto_other = other.proto_other;
	rtp_ssrc = other.rtp_ssrc;
	rtcp_cname = other.rtcp_cname;
	replacePayloads(other.payloads);
	ptime = other.ptime;
	maxptime = other.maxptime;
	dir = other.dir;
	crypto = other.crypto;
	max_rate = other.max_rate;
	bundle_only = other.bundle_only;
	implicit_rtcp_fb = other.implicit_rtcp_fb;
	pad[0] = other.pad[0];
	pad[1] = other.pad[1];
	rtcp_fb = other.rtcp_fb;
	rtcp_xr = other.rtcp_xr;
	mid = other.mid;
	mid_rtp_ext_header_id = other.mid_rtp_ext_header_id;
	mixer_to_client_extension_id = other.mixer_to_client_extension_id;
	client_to_mixer_extension_id = other.client_to_mixer_extension_id;
	frame_marking_extension_id = other.frame_marking_extension_id;
	conference_ssrc = other.conference_ssrc;
	set_nortpproxy = other.set_nortpproxy;
	rtcp_mux = other.rtcp_mux;
	haveZrtpHash = other.haveZrtpHash;
	haveLimeIk = other.haveLimeIk;
	memcpy(zrtphash, other.zrtphash, sizeof(zrtphash));
	dtls_fingerprint = other.dtls_fingerprint;
	dtls_role = other.dtls_role;
	ttl = other.ttl;
	index = other.index;
	tcapIndex = other.tcapIndex;
	acapIndexes = other.acapIndexes;
	delete_media_attributes = other.delete_media_attributes;
	delete_session_attributes = other.delete_session_attributes;

	return *this;
}

bool SalStreamConfiguration::isRecvOnly(const OrtpPayloadType *p) {
	return (p->flags & PAYLOAD_TYPE_FLAG_CAN_RECV) && !(p->flags & PAYLOAD_TYPE_FLAG_CAN_SEND);
}

bool SalStreamConfiguration::isSamePayloadType(const PayloadType *p1, const PayloadType *p2) {
	if (p1->type != p2->type) return false;
	if (strcmp(p1->mime_type, p2->mime_type) != 0) return false;
	if (p1->clock_rate != p2->clock_rate) return false;
	if (p1->channels != p2->channels) return false;
	if (payload_type_get_number(p1) != payload_type_get_number(p2)) return false;
	/*
	 Do not compare fmtp right now: they are modified internally when the call is started
	*/
	/*
	if (!fmtp_equals(p1->recv_fmtp,p2->recv_fmtp) ||
	    !fmtp_equals(p1->send_fmtp,p2->send_fmtp))
	    return false;
	*/
	return true;
}

bool SalStreamConfiguration::isSamePayloadList(const std::list<PayloadType *> &l1, const std::list<PayloadType *> &l2) {
	auto p1 = l1.cbegin();
	auto p2 = l2.cbegin();

	for (; (p1 != l1.cend() && p2 != l2.cend()); ++p1, ++p2) {
		if (!isSamePayloadType(*p1, *p2)) return false;
	}

	if (p1 != l1.cend()) {
		/*skip possible recv-only payloads*/
		for (; p1 != l1.cend() && isRecvOnly(*p1); ++p1) {
			ms_message("Skipping recv-only payload type...");
		}
	}

	if (p1 != l1.cend() || p2 != l2.cend()) {
		/*means one list is longer than the other*/
		return false;
	}

	return true;
}

bool SalStreamConfiguration::operator==(const SalStreamConfiguration &other) const {
	return equal(other) == SAL_MEDIA_DESCRIPTION_UNCHANGED;
}

bool SalStreamConfiguration::operator!=(const SalStreamConfiguration &other) const {
	return !(*this == other);
}

int SalStreamConfiguration::equal(const SalStreamConfiguration &other) const {
	int result = SAL_MEDIA_DESCRIPTION_UNCHANGED;

	/* A different proto should result in SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED but the encryption change
	   needs a stream restart for now, so use SAL_MEDIA_DESCRIPTION_CODEC_CHANGED */
	if (proto != other.proto) result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	for (auto crypto1 = crypto.cbegin(), crypto2 = other.crypto.cbegin();
	     (crypto1 != crypto.cend() && crypto2 != other.crypto.cend()); ++crypto1, ++crypto2) {
		if ((crypto1->tag != crypto2->tag) || (crypto1->algo != crypto2->algo)) {
			result |= SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		}
		if (crypto1->master_key.compare(crypto2->master_key)) {
			result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
		}
	}

	const auto thisCryptoSize = crypto.size();
	const auto otherCryptoSize = other.crypto.size();
	if (thisCryptoSize != otherCryptoSize) {
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_POLICY_CHANGED;
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
	}

	if (((thisCryptoSize > 0) && (otherCryptoSize == 0)) || ((thisCryptoSize == 0) && (otherCryptoSize > 0))) {
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED;
	}

	if (!isSamePayloadList(payloads, other.payloads)) {
		result |= SAL_MEDIA_DESCRIPTION_CODEC_CHANGED;
	}
	// Codec changed if either ptime is valid (i.e. greater than 0) and the other is not
	if (((ptime > 0) ^ (other.ptime > 0))) result |= SAL_MEDIA_DESCRIPTION_PTIME_CHANGED;
	// If both ptimes are valid, check that their valid is the same
	if ((ptime > 0) && (other.ptime > 0) && (ptime != other.ptime)) result |= SAL_MEDIA_DESCRIPTION_PTIME_CHANGED;
	if (dir != other.dir) result |= SAL_MEDIA_DESCRIPTION_DIRECTION_CHANGED;

	/*DTLS*/
	if (dtls_role != other.dtls_role) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;

	if (((dtls_role == SalDtlsRoleInvalid) && (other.dtls_role != SalDtlsRoleInvalid)) ||
	    ((dtls_role != SalDtlsRoleInvalid) && (other.dtls_role == SalDtlsRoleInvalid)))
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED;
	if (dtls_fingerprint.compare(other.dtls_fingerprint) != 0) result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;

	/*ZRTP*/
	if (haveZrtpHash != other.haveZrtpHash) {
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_TYPE_CHANGED;
	}
	if (haveZrtpHash && other.haveZrtpHash && (strcmp((const char *)zrtphash, (const char *)other.zrtphash) != 0))
		result |= SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;

	/* Extensions */
	if (mixer_to_client_extension_id != other.mixer_to_client_extension_id)
		result |= SAL_MEDIA_DESCRIPTION_MIXER_TO_CLIENT_EXTENSION_CHANGED;
	if (client_to_mixer_extension_id != other.client_to_mixer_extension_id)
		result |= SAL_MEDIA_DESCRIPTION_CLIENT_TO_MIXER_EXTENSION_CHANGED;
	if (frame_marking_extension_id != other.frame_marking_extension_id)
		result |= SAL_MEDIA_DESCRIPTION_FRAME_MARKING_EXTENSION_CHANGED;

	return result;
}

void SalStreamConfiguration::disable() {
	/* Remove potential bundle parameters. A disabled stream is moved out of the bundle. */
	mid.clear();
	bundle_only = false;
	dir = SalStreamInactive;
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamConfiguration::hasAvpf() const {
	switch (proto) {
		case SalProtoRtpAvpf:
		case SalProtoRtpSavpf:
		case SalProtoUdpTlsRtpSavpf:
			return true;
		case SalProtoRtpAvp:
		case SalProtoRtpSavp:
		case SalProtoUdpTlsRtpSavp:
		case SalProtoOther:
			return false;
	}
	return false;
}

bool SalStreamConfiguration::hasImplicitAvpf() const {
	return implicit_rtcp_fb;
}

/*these are switch case, so that when a new proto is added we can't forget to modify this function*/
bool SalStreamConfiguration::hasSrtp() const {
	switch (proto) {
		case SalProtoRtpSavp:
		case SalProtoRtpSavpf:
			return true;
		case SalProtoRtpAvp:
		case SalProtoRtpAvpf:
		case SalProtoUdpTlsRtpSavpf:
		case SalProtoUdpTlsRtpSavp:
		case SalProtoOther:
			return false;
	}
	return false;
}

bool SalStreamConfiguration::hasDtls() const {
	switch (proto) {
		case SalProtoUdpTlsRtpSavpf:
		case SalProtoUdpTlsRtpSavp:
			return true;
		case SalProtoRtpSavp:
		case SalProtoRtpSavpf:
		case SalProtoRtpAvp:
		case SalProtoRtpAvpf:
		case SalProtoOther:
			return false;
	}
	return false;
}

bool SalStreamConfiguration::hasZrtpHash() const {
	return (haveZrtpHash == 1);
}

const uint8_t *SalStreamConfiguration::getZrtpHash() const {
	return zrtphash;
}

bool SalStreamConfiguration::hasZrtp() const {
	if (haveZrtpHash == 1) {
		switch (proto) {
			case SalProtoRtpAvp:
			case SalProtoRtpAvpf:
				return true;
			case SalProtoUdpTlsRtpSavpf:
			case SalProtoUdpTlsRtpSavp:
			case SalProtoRtpSavp:
			case SalProtoRtpSavpf:
			case SalProtoOther:
				return false;
		}
	}
	return false;
}

bool SalStreamConfiguration::hasLimeIk() const {
	if (haveLimeIk == 1) return true;
	return false;
}

const SalMediaProto &SalStreamConfiguration::getProto() const {
	return proto;
}

const std::string SalStreamConfiguration::getProtoAsString() const {
	if (proto == SalProtoOther) return proto_other;
	else return LinphonePrivate::Utils::toString(proto);
}

SalStreamDir SalStreamConfiguration::getDirection() const {
	return dir;
}

const std::list<OrtpPayloadType *> &SalStreamConfiguration::getPayloads() const {
	return payloads;
}

const int &SalStreamConfiguration::getMaxRate() const {
	return max_rate;
}

const std::string &SalStreamConfiguration::getMid() const {
	return mid;
}

const int &SalStreamConfiguration::getMidRtpExtHeaderId() const {
	return mid_rtp_ext_header_id;
}

void SalStreamConfiguration::enableAvpfForStream() {
	for (auto &pt : payloads) {
		payload_type_set_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
	}
}
void SalStreamConfiguration::disableAvpfForStream() {
	for (auto &pt : payloads) {
		payload_type_unset_flag(pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
	}
}

void SalStreamConfiguration::mergeAcaps(const std::list<std::list<unsigned int>> &acaps) {
	// Avoid adding duplicates
	for (const auto &newIdxs : acaps) {
		bool found = false;
		for (const auto &idxs : acapIndexes) {
			found |= ((idxs.size() == newIdxs.size()) && std::equal(idxs.begin(), idxs.end(), newIdxs.begin()));
		}
		if (!found) {
			acapIndexes.push_back(newIdxs);
		}
	}
}

const std::list<std::list<unsigned int>> &SalStreamConfiguration::getAcapIndexes() const {
	return acapIndexes;
}

const unsigned int &SalStreamConfiguration::getTcapIndex() const {
	return tcapIndex;
}

std::string SalStreamConfiguration::getSdpString() const {

	std::string acapString;
	// Iterate over all acaps sets. For every set, get the index of all its members
	for (const auto &acapSet : acapIndexes) {
		// Do not append | on first element
		if (!acapString.empty()) {
			acapString.append("|");
		}
		const auto &firstAcapIdx = acapSet.front();
		for (const auto &acapIdx : acapSet) {
			if (acapIdx != firstAcapIdx) {
				acapString.append(",");
			}
			if (acapIdx != 0) {
				acapString.append(std::to_string(acapIdx));
			}
		}
	}

	std::string tcapString;

	if (tcapIndex != 0) {
		tcapString = std::to_string(tcapIndex);
	}

	std::string deleteAttrs;
	if (delete_media_attributes && delete_session_attributes) {
		deleteAttrs = "-ms";
	} else if (delete_session_attributes) {
		deleteAttrs = "-s";
	} else if (delete_media_attributes) {
		deleteAttrs = "-m";
	}

	std::string sdpString;

	if (!deleteAttrs.empty() && !acapString.empty()) {
		sdpString += "a=" + deleteAttrs + ":" + acapString;
	} else if (!deleteAttrs.empty()) {
		sdpString += "a=" + deleteAttrs;
	} else if (!acapString.empty()) {
		sdpString += "a=" + acapString;
	}

	if (!tcapString.empty()) {
		if (!sdpString.empty()) {
			sdpString += " ";
		}
		sdpString += "t=" + tcapString;
	}
	return sdpString;
}

const int &SalStreamConfiguration::getMixerToClientExtensionId() const {
	return mixer_to_client_extension_id;
}

const int &SalStreamConfiguration::getClientToMixerExtensionId() const {
	return client_to_mixer_extension_id;
}

const int &SalStreamConfiguration::getFrameMarkingExtensionId() const {
	return frame_marking_extension_id;
}

std::string SalStreamConfiguration::cryptoToSdpValue(const SalSrtpCryptoAlgo &crypto) {
	std::string sdpValue;
	MSCryptoSuiteNameParams desc;
	if (ms_crypto_suite_to_name_params(crypto.algo, &desc) == 0) {
		sdpValue = std::to_string(crypto.tag) + " " + desc.name + " inline:" + crypto.master_key;
		if (desc.params) {
			sdpValue += " ";
			sdpValue += desc.params;
		}
	}

	return sdpValue;
}

void SalStreamConfiguration::replacePayloads(const std::list<OrtpPayloadType *> &newPayloads) {
	PayloadTypeHandler::clearPayloadList(payloads);
	for (const auto &pt : newPayloads) {
		payloads.push_back(payload_type_clone(pt));
	}
}

std::string SalStreamConfiguration::getSetupAttributeForDtlsRole(const SalDtlsRole &role) {
	std::string setupAttrValue;
	switch (role) {
		case SalDtlsRoleIsClient:
			setupAttrValue = "active";
			break;
		case SalDtlsRoleIsServer:
			setupAttrValue = "passive";
			break;
		case SalDtlsRoleInvalid:
			break;
		case SalDtlsRoleUnset:
		default:
			setupAttrValue = "actpass";
			break;
	}
	return setupAttrValue;
}

SalDtlsRole SalStreamConfiguration::getDtlsRoleFromSetupAttribute(const std::string setupAttr) {
	SalDtlsRole role = SalDtlsRoleInvalid;
	if (setupAttr.compare("actpass") == 0) {
		role = SalDtlsRoleUnset;
	} else if (setupAttr.compare("active") == 0) {
		role = SalDtlsRoleIsClient;
	} else if (setupAttr.compare("passive") == 0) {
		role = SalDtlsRoleIsServer;
	}
	return role;
}

SalSrtpCryptoAlgo SalStreamConfiguration::fillStrpCryptoAlgoFromString(const std::string &value) {
	unsigned int tag;
	char name[257] = {0}, masterKey[129] = {0}, parameters[257] = {0};
	const auto nb = sscanf(value.c_str(), "%u %256s inline:%128s %256[A-Z_ ]", &tag, name, masterKey, parameters);

	SalSrtpCryptoAlgo keyEnc;
	keyEnc.algo = MS_CRYPTO_SUITE_INVALID;
	if (nb >= 3) {
		MSCryptoSuiteNameParams np;
		np.name = name;
		np.params = parameters[0] != '\0' ? parameters : NULL;
		const auto cs = ms_crypto_suite_build_from_name_params(&np);

		keyEnc.algo = cs;
		if (cs == MS_CRYPTO_SUITE_INVALID) {
			ms_warning("Failed to parse crypto-algo: '%s'", name);
		} else {
			keyEnc.tag = tag;
			keyEnc.master_key = masterKey;
			// Erase all characters after | if it is found
			size_t sep = keyEnc.master_key.find("|");
			if (sep != std::string::npos)
				keyEnc.master_key.erase(keyEnc.master_key.begin() + static_cast<long>(sep), keyEnc.master_key.end());
		}
	} else {
		lError() << "Unable to extract crypto key informations from crypto argument value " << value;
	}
	return keyEnc;
}

LINPHONE_END_NAMESPACE
