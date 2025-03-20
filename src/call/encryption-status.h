/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "mediastreamer2/ms_srtp.h"
#include "mediastreamer2/zrtp.h"

#include "linphone/types.h"

typedef struct _ZrtpAlgo ZrtpAlgo;
struct _ZrtpAlgo {
	int cipher_algo;        /**< Id of the cipher algorithms */
	int key_agreement_algo; /**< Id of the key agreement algorithm */
	int hash_algo;          /**< Id of the hash algorithm */
	int auth_tag_algo;      /**< Id of the authentication tag algorithm */
	int sas_algo;           /**< Id of the SAS algorithm */
};

typedef struct _SrtpInfo SrtpInfo;
struct _SrtpInfo {
	int send_suite;  /**< srtp crypto suite used by sender channel, cast from MSCryptoSuite defined in ms_srtp.h */
	int send_source; /**< source of srtp key material used by sender channel, cast from MSSrtpKeySource defined in
	                    ms_srtp.h*/
	int recv_suite;  /**< srtp crypto suite used by receiver channel, cast from MSCryptoSuite defined in ms_srtp.h */
	int recv_source; /**< source of srtp key material used by receiver channel, cast from MSSrtpKeySource defined in
	                    ms_srtp.h*/
};

class EncryptionStatus {
public:
	EncryptionStatus() = default;
	EncryptionStatus(const EncryptionStatus &other) = default;

	EncryptionStatus &operator=(const EncryptionStatus &other) {
		mZrtpAlgo = other.mZrtpAlgo;
		mInnerSrtpInfo = other.mInnerSrtpInfo;
		mSrtpInfo = other.mSrtpInfo;
		return *this;
	}

	bool isDowngradedComparedTo(const EncryptionStatus &other) const;

	LinphoneMediaEncryption getMediaEncryption() const;
	const ZrtpAlgo *getZrtpAlgo() const;
	const SrtpInfo *getSrtpInfo() const;
	const SrtpInfo *getInnerSrtpInfo() const;

	/* ZRTP algorithms */
	void setZrtpCipherAlgo(int cipherAlgo);
	void setZrtpKeyAgreementAlgo(int keyAgreementAlgo);
	void setZrtpHashAlgo(int hashAlgo);
	void setZrtpAuthTagAlgo(int authTagAlgo);
	void setZrtpSasAlgo(int sasAlgo);
	const char *getZrtpCipherAlgo() const;
	const char *getZrtpKeyAgreementAlgo() const;
	bool isZrtpKeyAgreementAlgoPostQuantum() const;
	const char *getZrtpHashAlgo() const;
	const char *getZrtpAuthTagAlgo() const;
	const char *getZrtpSasAlgo() const;

	/* SRTP information */
	void setSrtpSendSuite(int sendSuite);
	void setSrtpSendSource(int sendSource);
	void setSrtpRecvSuite(int recvSuite);
	void setSrtpRecvSource(int recvSource);
	int getSrtpSendSuite() const;
	int getSrtpSendSource() const;
	int getSrtpRecvSuite() const;
	int getSrtpRecvSource() const;

	/* Inner SRTP information */
	void setInnerSrtpSendSuite(int sendSuite);
	void setInnerSrtpSendSource(int sendSource);
	void setInnerSrtpRecvSuite(int recvSuite);
	void setInnerSrtpRecvSource(int recvSource);

	void setWorstSecurityAlgo(const EncryptionStatus &other);

private:
	bool compareMediaEncryption(const EncryptionStatus &other) const;
	bool compareZrtpCipherAlgo(const EncryptionStatus &other) const;
	bool compareZrtpKeyAgreementAlgo(const EncryptionStatus &other) const;
	bool compareZrtpHashAlgo(const EncryptionStatus &other) const;
	bool compareZrtpAuthTagAlgo(const EncryptionStatus &other) const;
	bool compareZrtpSasAlgo(const EncryptionStatus &other) const;
	bool compareSrtpSendSuite(const EncryptionStatus &other) const;
	bool compareSrtpRecvSuite(const EncryptionStatus &other) const;
	bool compareSrtpSendSource(const EncryptionStatus &other) const;
	bool compareSrtpRecvSource(const EncryptionStatus &other) const;
	bool compareInnerSrtpSendSuite(const EncryptionStatus &other) const;
	bool compareInnerSrtpRecvSuite(const EncryptionStatus &other) const;
	bool compareInnerSrtpSendSource(const EncryptionStatus &other) const;
	bool compareInnerSrtpRecvSource(const EncryptionStatus &other) const;

	ZrtpAlgo mZrtpAlgo{};      /**< information on the ZRTP exchange updated once it is performed(when the SAS is
	                            available), this is valid only on the audio stream */
	SrtpInfo mInnerSrtpInfo{}; /**< information on the SRTP crypto suite and source of key material used
	                            on this stream for inner encryption when double encryption is on */
	SrtpInfo mSrtpInfo{};      /**< information on the SRTP crypto suite and source of key material used on
	                            this stream */
};