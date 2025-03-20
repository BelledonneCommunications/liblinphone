/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
 *
 * This file is part of mediastreamer2
 * (see https://gitlab.linphone.org/BC/public/mediastreamer2).
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

#include "encryption-status.h"

#include "linphone/misc.h"
#include "logger/logger.h"

bool EncryptionStatus::isDowngradedComparedTo(const EncryptionStatus &other) const {
	return compareMediaEncryption(other) || compareZrtpAuthTagAlgo(other) || compareZrtpCipherAlgo(other) ||
	       compareZrtpHashAlgo(other) || compareZrtpKeyAgreementAlgo(other) || compareZrtpSasAlgo(other) ||
	       compareSrtpSendSuite(other) || compareSrtpRecvSuite(other) || compareSrtpSendSource(other) ||
	       compareSrtpRecvSource(other) || compareInnerSrtpSendSuite(other) || compareInnerSrtpRecvSuite(other) ||
	       compareInnerSrtpSendSource(other) || compareInnerSrtpRecvSource(other);
}

LinphoneMediaEncryption EncryptionStatus::getMediaEncryption() const {
	// When send and receive source are different, setting is not complete (on going nego), returns invalid
	auto sendSource = mSrtpInfo.send_source;
	auto recvSource = mSrtpInfo.recv_source;
	if (sendSource != recvSource) {
		return LinphoneMediaEncryptionNone;
	}
	switch (sendSource) {
		case MSSrtpKeySourceSDES:
			return LinphoneMediaEncryptionSRTP;
		case MSSrtpKeySourceZRTP:
			return LinphoneMediaEncryptionZRTP;
		case MSSrtpKeySourceDTLS:
			return LinphoneMediaEncryptionDTLS;
		case MSSrtpKeySourceUnknown:
		case MSSrtpKeySourceUnavailable:
		case MSSrtpKeySourceEKT:
		default:
			return LinphoneMediaEncryptionNone;
	}
}

const ZrtpAlgo *EncryptionStatus::getZrtpAlgo() const {
	return &mZrtpAlgo;
}

const SrtpInfo *EncryptionStatus::getSrtpInfo() const {
	return &mSrtpInfo;
}

const SrtpInfo *EncryptionStatus::getInnerSrtpInfo() const {
	return &mInnerSrtpInfo;
}

void EncryptionStatus::setZrtpCipherAlgo(int cipherAlgo) {
	mZrtpAlgo.cipher_algo = cipherAlgo;
}

void EncryptionStatus::setZrtpKeyAgreementAlgo(int keyAgreementAlgo) {
	mZrtpAlgo.key_agreement_algo = keyAgreementAlgo;
}

void EncryptionStatus::setZrtpHashAlgo(int hashAlgo) {
	mZrtpAlgo.hash_algo = hashAlgo;
}

void EncryptionStatus::setZrtpAuthTagAlgo(int authTagAlgo) {
	mZrtpAlgo.auth_tag_algo = authTagAlgo;
}

void EncryptionStatus::setZrtpSasAlgo(int sasAlgo) {
	mZrtpAlgo.sas_algo = sasAlgo;
}

const char *EncryptionStatus::getZrtpCipherAlgo() const {
	switch (mZrtpAlgo.cipher_algo) {
		case MS_ZRTP_CIPHER_INVALID:
			return "invalid";
		case MS_ZRTP_CIPHER_AES1:
			return "AES-128";
		case MS_ZRTP_CIPHER_AES2:
			return "AES-192";
		case MS_ZRTP_CIPHER_AES3:
			return "AES-256";
		case MS_ZRTP_CIPHER_2FS1:
			return "TwoFish-128";
		case MS_ZRTP_CIPHER_2FS2:
			return "TwoFish-192";
		case MS_ZRTP_CIPHER_2FS3:
			return "TwoFish-256";
		default:
			return "Unknown Algo";
	}
}

const char *EncryptionStatus::getZrtpKeyAgreementAlgo() const {
	switch (mZrtpAlgo.key_agreement_algo) {
		case MS_ZRTP_KEY_AGREEMENT_INVALID:
			return "invalid";
		case MS_ZRTP_KEY_AGREEMENT_DH2K:
			return "DHM-2048";
		case MS_ZRTP_KEY_AGREEMENT_EC25:
			return "ECDH-256";
		case MS_ZRTP_KEY_AGREEMENT_DH3K:
			return "DHM-3072";
		case MS_ZRTP_KEY_AGREEMENT_EC38:
			return "ECDH-384";
		case MS_ZRTP_KEY_AGREEMENT_EC52:
			return "ECDH-521";
		case MS_ZRTP_KEY_AGREEMENT_X255:
			return "X25519";
		case MS_ZRTP_KEY_AGREEMENT_X448:
			return "X448";
		case MS_ZRTP_KEY_AGREEMENT_K255:
			return "KEM-X25519";
		case MS_ZRTP_KEY_AGREEMENT_K448:
			return "KEM-X448";
		case MS_ZRTP_KEY_AGREEMENT_MLK1:
			return "MLKEM-512";
		case MS_ZRTP_KEY_AGREEMENT_MLK2:
			return "MLKEM-768";
		case MS_ZRTP_KEY_AGREEMENT_MLK3:
			return "MLKEM-1024";
		case MS_ZRTP_KEY_AGREEMENT_KYB1:
			return "KYBER-512";
		case MS_ZRTP_KEY_AGREEMENT_KYB2:
			return "KYBER-768";
		case MS_ZRTP_KEY_AGREEMENT_KYB3:
			return "KYBER-1024";
		case MS_ZRTP_KEY_AGREEMENT_HQC1:
			return "HQC-128";
		case MS_ZRTP_KEY_AGREEMENT_HQC2:
			return "HQC-192";
		case MS_ZRTP_KEY_AGREEMENT_HQC3:
			return "HQC-256";
		case MS_ZRTP_KEY_AGREEMENT_K255_MLK512:
			return "X25519/MLKem512";
		case MS_ZRTP_KEY_AGREEMENT_K255_KYB512:
			return "X25519/Kyber512";
		case MS_ZRTP_KEY_AGREEMENT_K255_HQC128:
			return "X25519/HQC128";
		case MS_ZRTP_KEY_AGREEMENT_K448_MLK1024:
			return "X448/MLKem1024";
		case MS_ZRTP_KEY_AGREEMENT_K448_KYB1024:
			return "X448/Kyber1024";
		case MS_ZRTP_KEY_AGREEMENT_K448_HQC256:
			return "X448/HQC256";
		case MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128:
			return "X25519/Kyber512/HQC128";
		case MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256:
			return "X448/Kyber1024/HQC256";
		default:
			return "Unknown Algo";
	}
}

bool EncryptionStatus::isZrtpKeyAgreementAlgoPostQuantum() const {
	switch (mZrtpAlgo.key_agreement_algo) {
		case MS_ZRTP_KEY_AGREEMENT_MLK1:
		case MS_ZRTP_KEY_AGREEMENT_MLK2:
		case MS_ZRTP_KEY_AGREEMENT_MLK3:
		case MS_ZRTP_KEY_AGREEMENT_KYB1:
		case MS_ZRTP_KEY_AGREEMENT_KYB2:
		case MS_ZRTP_KEY_AGREEMENT_KYB3:
		case MS_ZRTP_KEY_AGREEMENT_HQC1:
		case MS_ZRTP_KEY_AGREEMENT_HQC2:
		case MS_ZRTP_KEY_AGREEMENT_HQC3:
		case MS_ZRTP_KEY_AGREEMENT_K255_MLK512:
		case MS_ZRTP_KEY_AGREEMENT_K255_KYB512:
		case MS_ZRTP_KEY_AGREEMENT_K255_HQC128:
		case MS_ZRTP_KEY_AGREEMENT_K448_MLK1024:
		case MS_ZRTP_KEY_AGREEMENT_K448_KYB1024:
		case MS_ZRTP_KEY_AGREEMENT_K448_HQC256:
		case MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128:
		case MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256:
			return true;
		default:
			return false;
	}
}

const char *EncryptionStatus::getZrtpHashAlgo() const {
	switch (mZrtpAlgo.hash_algo) {
		case MS_ZRTP_HASH_INVALID:
			return "invalid";
		case MS_ZRTP_HASH_S256:
			return "SHA-256";
		case MS_ZRTP_HASH_S384:
			return "SHA-384";
		case MS_ZRTP_HASH_N256:
			return "SHA3-256";
		case MS_ZRTP_HASH_N384:
			return "SHA3-384";
		case MS_ZRTP_HASH_S512:
			return "SHA-512";
		default:
			return "Unknown Algo";
	}
}

const char *EncryptionStatus::getZrtpAuthTagAlgo() const {
	switch (mZrtpAlgo.auth_tag_algo) {
		case MS_ZRTP_AUTHTAG_INVALID:
			return "invalid";
		case MS_ZRTP_AUTHTAG_HS32:
			return "HMAC-SHA1-32";
		case MS_ZRTP_AUTHTAG_HS80:
			return "HMAC-SHA1-80";
		case MS_ZRTP_AUTHTAG_SK32:
			return "Skein-32";
		case MS_ZRTP_AUTHTAG_SK64:
			return "Skein-64";
		case MS_ZRTP_AUTHTAG_GCM:
			return "GCM";
		default:
			return "Unknown Algo";
	}
}

const char *EncryptionStatus::getZrtpSasAlgo() const {
	switch (mZrtpAlgo.sas_algo) {
		case MS_ZRTP_SAS_INVALID:
			return "invalid";
		case MS_ZRTP_SAS_B32:
			return "Base32";
		case MS_ZRTP_SAS_B256:
			return "PGP-WordList";
		default:
			return "Unknown Algo";
	}
}

void EncryptionStatus::setSrtpSendSuite(int sendSuite) {
	mSrtpInfo.send_suite = sendSuite;
}

void EncryptionStatus::setSrtpSendSource(int sendSource) {
	mSrtpInfo.send_source = sendSource;
}

void EncryptionStatus::setSrtpRecvSuite(int recvSuite) {
	mSrtpInfo.recv_suite = recvSuite;
}

void EncryptionStatus::setSrtpRecvSource(int recvSource) {
	mSrtpInfo.recv_source = recvSource;
}

int EncryptionStatus::getSrtpSendSuite() const {
	return mSrtpInfo.send_suite;
}

int EncryptionStatus::getSrtpSendSource() const {
	return mSrtpInfo.send_source;
}

int EncryptionStatus::getSrtpRecvSuite() const {
	return mSrtpInfo.recv_suite;
}

int EncryptionStatus::getSrtpRecvSource() const {
	return mSrtpInfo.recv_source;
}

void EncryptionStatus::setInnerSrtpSendSuite(int sendSuite) {
	mInnerSrtpInfo.send_suite = sendSuite;
}

void EncryptionStatus::setInnerSrtpSendSource(int sendSource) {
	mInnerSrtpInfo.send_source = sendSource;
}

void EncryptionStatus::setInnerSrtpRecvSuite(int recvSuite) {
	mInnerSrtpInfo.recv_suite = recvSuite;
}

void EncryptionStatus::setInnerSrtpRecvSource(int recvSource) {
	mInnerSrtpInfo.recv_source = recvSource;
}

void EncryptionStatus::setWorstSecurityAlgo(const EncryptionStatus &other) {
	if (!compareZrtpSasAlgo(other)) {
		mZrtpAlgo.sas_algo = other.mZrtpAlgo.sas_algo;
	}
	if (!compareZrtpKeyAgreementAlgo(other)) {
		mZrtpAlgo.key_agreement_algo = other.mZrtpAlgo.key_agreement_algo;
	}
	if (!compareZrtpHashAlgo(other)) {
		mZrtpAlgo.hash_algo = other.mZrtpAlgo.hash_algo;
	}
	if (!compareZrtpCipherAlgo(other)) {
		mZrtpAlgo.cipher_algo = other.mZrtpAlgo.cipher_algo;
	}
	if (!compareZrtpAuthTagAlgo(other)) {
		mZrtpAlgo.auth_tag_algo = other.mZrtpAlgo.auth_tag_algo;
	}
	if (!compareSrtpSendSuite(other)) {
		mSrtpInfo.send_suite = other.mSrtpInfo.send_suite;
	}
	if (!compareSrtpRecvSuite(other)) {
		mSrtpInfo.recv_suite = other.mSrtpInfo.recv_suite;
	}
	if (!compareSrtpSendSource(other)) {
		mSrtpInfo.send_source = other.mSrtpInfo.send_source;
	}
	if (!compareSrtpRecvSource(other)) {
		mSrtpInfo.recv_source = other.mSrtpInfo.recv_source;
	}
	if (!compareInnerSrtpSendSuite(other)) {
		mInnerSrtpInfo.send_suite = other.mInnerSrtpInfo.send_suite;
	}
	if (!compareInnerSrtpRecvSuite(other)) {
		mInnerSrtpInfo.recv_suite = other.mInnerSrtpInfo.recv_suite;
	}
	if (!compareInnerSrtpSendSource(other)) {
		mInnerSrtpInfo.send_source = other.mInnerSrtpInfo.send_source;
	}
	if (!compareInnerSrtpRecvSource(other)) {
		mInnerSrtpInfo.recv_source = other.mInnerSrtpInfo.recv_source;
	}
}

bool EncryptionStatus::compareMediaEncryption(const EncryptionStatus &other) const {
	const auto otherMediaEncryption = other.getMediaEncryption();
	const auto thisMediaEncryption = getMediaEncryption();
	bool ret = false;
	switch (otherMediaEncryption) {
		case LinphoneMediaEncryptionNone:
			ret = false;
			break;
		case LinphoneMediaEncryptionSRTP:
		case LinphoneMediaEncryptionDTLS:
			ret = thisMediaEncryption == LinphoneMediaEncryptionNone;
			break;
		case LinphoneMediaEncryptionZRTP:
			ret = thisMediaEncryption != LinphoneMediaEncryptionZRTP;
			break;
	}
	if (ret) {
		lInfo() << __func__ << " : Media encryption changed - [Actual] "
		        << linphone_media_encryption_to_string(thisMediaEncryption) << " < [Before] "
		        << linphone_media_encryption_to_string(otherMediaEncryption);
	}
	return ret;
}

bool EncryptionStatus::compareZrtpCipherAlgo(const EncryptionStatus &other) const {
	bool ret = false;
	switch (other.mZrtpAlgo.cipher_algo) {
		case MS_ZRTP_CIPHER_INVALID:
			ret = mZrtpAlgo.cipher_algo < other.mZrtpAlgo.cipher_algo;
			break;
		case MS_ZRTP_CIPHER_AES1:
		case MS_ZRTP_CIPHER_2FS1:
			ret = mZrtpAlgo.cipher_algo < MS_ZRTP_CIPHER_AES1;
			break;
		case MS_ZRTP_CIPHER_AES2:
		case MS_ZRTP_CIPHER_2FS2:
			ret = mZrtpAlgo.cipher_algo < MS_ZRTP_CIPHER_AES2;
			break;
		case MS_ZRTP_CIPHER_AES3:
		case MS_ZRTP_CIPHER_2FS3:
			ret = mZrtpAlgo.cipher_algo < MS_ZRTP_CIPHER_AES3;
			break;
		default:
			ret = true;
			break;
	}
	if (ret)
		lInfo() << __func__ << " : ZRTP cipher algo changed - [Actual] " << mZrtpAlgo.cipher_algo << " < [Before] "
		        << other.mZrtpAlgo.cipher_algo;
	return ret;
}

bool EncryptionStatus::compareZrtpKeyAgreementAlgo(const EncryptionStatus &other) const {
	bool ret = false;
	switch (other.mZrtpAlgo.key_agreement_algo) {
		case MS_ZRTP_KEY_AGREEMENT_INVALID:
		case MS_ZRTP_KEY_AGREEMENT_DH2K:
		case MS_ZRTP_KEY_AGREEMENT_DH3K:
			ret = mZrtpAlgo.key_agreement_algo < other.mZrtpAlgo.key_agreement_algo;
			break;
		case MS_ZRTP_KEY_AGREEMENT_X255:
		case MS_ZRTP_KEY_AGREEMENT_K255:
			ret = mZrtpAlgo.key_agreement_algo < MS_ZRTP_KEY_AGREEMENT_X255;
			break;
		case MS_ZRTP_KEY_AGREEMENT_X448:
		case MS_ZRTP_KEY_AGREEMENT_K448:
			ret = mZrtpAlgo.key_agreement_algo < MS_ZRTP_KEY_AGREEMENT_X448;
			break;
		case MS_ZRTP_KEY_AGREEMENT_K255_KYB512:
		case MS_ZRTP_KEY_AGREEMENT_K255_HQC128:
			ret = mZrtpAlgo.key_agreement_algo < MS_ZRTP_KEY_AGREEMENT_K255_KYB512;
			break;
		case MS_ZRTP_KEY_AGREEMENT_K448_KYB1024:
		case MS_ZRTP_KEY_AGREEMENT_K448_HQC256:
			ret = mZrtpAlgo.key_agreement_algo < MS_ZRTP_KEY_AGREEMENT_K448_KYB1024;
			break;
		case MS_ZRTP_KEY_AGREEMENT_K255_KYB512_HQC128:
		case MS_ZRTP_KEY_AGREEMENT_K448_KYB1024_HQC256:
			ret = mZrtpAlgo.key_agreement_algo < other.mZrtpAlgo.key_agreement_algo;
			break;
		default:
			ret = true;
			break;
	}
	if (ret)
		lInfo() << __func__ << " : ZRTP key agreement algo changed - [Actual] " << mZrtpAlgo.key_agreement_algo
		        << " < [Before] " << other.mZrtpAlgo.key_agreement_algo;
	return ret;
}

bool EncryptionStatus::compareZrtpHashAlgo(const EncryptionStatus &other) const {
	const bool ret = mZrtpAlgo.hash_algo < other.mZrtpAlgo.hash_algo;
	if (ret)
		lInfo() << __func__ << " : ZRTP hash algo changed - [Actual] " << mZrtpAlgo.hash_algo << " < [Before] "
		        << other.mZrtpAlgo.hash_algo;
	return ret;
}

bool EncryptionStatus::compareZrtpAuthTagAlgo(const EncryptionStatus &other) const {
	const bool ret = mZrtpAlgo.auth_tag_algo < other.mZrtpAlgo.auth_tag_algo;
	if (ret)
		lInfo() << __func__ << " : ZRTP auth tag changed - [Actual] " << mZrtpAlgo.auth_tag_algo << " < [Before] "
		        << other.mZrtpAlgo.auth_tag_algo;
	return ret;
}

bool EncryptionStatus::compareZrtpSasAlgo(const EncryptionStatus &other) const {
	const bool ret = mZrtpAlgo.sas_algo < other.mZrtpAlgo.sas_algo;
	if (ret)
		lInfo() << __func__ << " : ZRTP SAS algo changed - [Actual] " << mZrtpAlgo.sas_algo << " < [Before] "
		        << other.mZrtpAlgo.sas_algo;
	return ret;
}

bool EncryptionStatus::compareSrtpSendSuite(const EncryptionStatus &other) const {
	const bool ret = mSrtpInfo.send_suite < other.mSrtpInfo.send_suite;
	if (ret)
		lInfo() << __func__ << " : SRTP send suite changed - [Actual] " << mSrtpInfo.send_suite << " < [Before] "
		        << other.mSrtpInfo.send_suite;
	return ret;
}

bool EncryptionStatus::compareSrtpRecvSuite(const EncryptionStatus &other) const {
	const bool ret = mSrtpInfo.recv_suite < other.mSrtpInfo.recv_suite;
	if (ret)
		lInfo() << __func__ << " : SRTP recv suite changed - [Actual] " << mSrtpInfo.recv_suite << " < [Before] "
		        << other.mSrtpInfo.recv_suite;
	return ret;
}

bool EncryptionStatus::compareSrtpSendSource(const EncryptionStatus &other) const {
	const bool ret = mSrtpInfo.send_source < other.mSrtpInfo.send_source;
	if (ret)
		lInfo() << __func__ << " : SRTP send source changed - [Actual] " << mSrtpInfo.send_source << " < [Before] "
		        << other.mSrtpInfo.send_source;
	return ret;
}

bool EncryptionStatus::compareSrtpRecvSource(const EncryptionStatus &other) const {
	const bool ret = mSrtpInfo.recv_source < other.mSrtpInfo.recv_source;
	if (ret)
		lInfo() << __func__ << " : SRTP recv source changed - [Actual] " << mSrtpInfo.recv_source << " < [Before] "
		        << other.mSrtpInfo.recv_source;
	return ret;
}

bool EncryptionStatus::compareInnerSrtpSendSuite(const EncryptionStatus &other) const {
	const bool ret = mInnerSrtpInfo.send_suite < other.mInnerSrtpInfo.send_suite;
	if (ret)
		lInfo() << __func__ << " : Inner SRTP send suite changed - [Actual] " << mInnerSrtpInfo.send_suite
		        << " < [Before] " << other.mInnerSrtpInfo.send_suite;
	return ret;
}

bool EncryptionStatus::compareInnerSrtpRecvSuite(const EncryptionStatus &other) const {
	const bool ret = mSrtpInfo.recv_suite < other.mInnerSrtpInfo.recv_suite;
	if (ret)
		lInfo() << __func__ << " : Inner SRTP recv suite changed - [Actual] " << mSrtpInfo.recv_suite << " < [Before] "
		        << other.mInnerSrtpInfo.recv_suite;
	return ret;
}

bool EncryptionStatus::compareInnerSrtpSendSource(const EncryptionStatus &other) const {
	const bool ret = mInnerSrtpInfo.send_source < other.mInnerSrtpInfo.send_source;
	if (ret)
		lInfo() << __func__ << " : Inner SRTP send source changed - [Actual] " << mInnerSrtpInfo.send_source
		        << " < [Before] " << other.mInnerSrtpInfo.send_source;
	return ret;
}

bool EncryptionStatus::compareInnerSrtpRecvSource(const EncryptionStatus &other) const {
	const bool ret = mInnerSrtpInfo.recv_source < other.mInnerSrtpInfo.recv_source;
	if (ret)
		lInfo() << __func__ << " : Inner SRTP recv source changed - [Actual] " << mInnerSrtpInfo.recv_source
		        << " < [Before] " << other.mInnerSrtpInfo.recv_source;
	return ret;
}
