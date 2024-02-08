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

#ifndef _SAL_MEDIA_DESCRIPTION_H_
#define _SAL_MEDIA_DESCRIPTION_H_

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "c-wrapper/internal/c-sal.h"
#include "linphone/utils/general.h"
#include "ortp/rtpsession.h"
#include "sal/params/sal_media_description_params.h"
#include "sal/sal_stream_bundle.h"
#include "sal/sal_stream_description.h"

LINPHONE_BEGIN_NAMESPACE

class SalStreamBundle;

class LINPHONE_PUBLIC SalMediaDescription {
	friend class MediaSessionPrivate;

public:
	// Adding 2208988800 to convert from UNIX time to NTP
	// UNIX time uses an epoch located at 1/1/1970-00:00h (UTC) and NTP uses 1/1/1900-00:00h.
	// The offset between the two epochs is 70 years (with 17 leap year in between).
	// number of seconds: (70*365 + 17)*86400 = 2208988800
	static constexpr long long ntpToUnix = 2208988800;

	SalMediaDescription(const SalMediaDescriptionParams &descParams);
	SalMediaDescription(belle_sdp_session_description_t *sdp);
	SalMediaDescription(const SalMediaDescription &other);
	virtual ~SalMediaDescription();

	belle_sdp_session_description_t *toSdp() const;

	void addNewBundle(const SalStreamBundle &bundle);

	int lookupMid(const std::string mid) const;
	const SalStreamBundle &getBundleFromMid(const std::string mid) const;
	std::list<int> getTransportOwnerIndexes() const;
	int getIndexOfTransportOwner(const SalStreamDescription &sd) const;

	const SalStreamDescription &findStream(SalMediaProto proto, SalStreamType type) const;
	int findIdxStream(SalMediaProto proto, SalStreamType type) const;
	unsigned int nbStreamsOfType(SalStreamType type) const;
	unsigned int nbActiveStreamsOfType(SalStreamType type) const;
	const SalStreamDescription &getActiveStreamOfType(SalStreamType type, unsigned int idx) const;
	const SalStreamDescription &findSecureStreamOfType(SalStreamType type) const;
	const SalStreamDescription &findBestStream(SalStreamType type) const;
	int findIdxBestStream(SalStreamType type) const;
	const SalStreamDescription &
	findStreamWithSdpAttribute(const SalStreamType type,
	                           const std::vector<std::pair<std::string, std::string>> &attributes) const;
	const SalStreamDescription &
	findStreamWithSdpAttribute(const std::vector<std::pair<std::string, std::string>> &attributes) const;
	const SalStreamDescription &findFirstStreamOfType(SalStreamType type, int startingIdx = -1) const;
	int findFirstStreamIdxOfType(SalStreamType type, int startingIdx = -1) const;
	const std::list<SalStreamDescription> findAllStreamsOfType(SalStreamType type) const;
	int findIdxStreamWithSdpAttribute(const SalStreamType,
	                                  const std::vector<std::pair<std::string, std::string>> &attributes) const;
	int findIdxStreamWithSdpAttribute(const std::vector<std::pair<std::string, std::string>> &attributes) const;
	const SalStreamDescription &findStreamWithLabel(SalStreamType type, const std::string label) const;
	int findIdxStreamWithLabel(SalStreamType type, const std::string label) const;

	const SalStreamDescription &findStreamWithContent(const std::string content) const;
	int findIdxStreamWithContent(const std::string content) const;
	const SalStreamDescription &findStreamWithContent(const std::string content, const SalStreamDir direction) const;
	int findIdxStreamWithContent(const std::string content, const SalStreamDir direction) const;
	const SalStreamDescription &findStreamWithContent(const std::string content, const std::string label) const;
	int findIdxStreamWithContent(const std::string content, const std::string label) const;

	bool isEmpty() const;
	bool isAcceptable() const;

	void setDir(SalStreamDir stream_dir);

	int getNbActiveStreams() const;

	bool hasIceParams() const;
	bool hasDir(const SalStreamDir &stream_dir) const;
	bool hasAvpf() const;
	bool hasImplicitAvpf() const;
	bool hasSrtp() const;
	bool hasDtls() const;
	bool hasZrtp() const;
	bool hasLimeIk() const;
	bool hasIpv6() const;
	const SalMediaDescriptionParams &getParams() const;

	SalMediaDescription &operator=(const SalMediaDescription &other);
	bool operator==(const SalMediaDescription &other) const;
	bool operator!=(const SalMediaDescription &other) const;
	int equal(const SalMediaDescription &otherMd) const;
	int globalEqual(const SalMediaDescription &otherMd) const;
	int compareToChosenConfiguration(const SalMediaDescription &otherMd) const;
	int compareToActualConfiguration(const SalMediaDescription &otherMd) const;

	static const std::string printDifferences(int result);

	size_t getNbStreams() const;
	const std::string &getOriginAddress() const;
	const std::string &getConnectionAddress() const;
	const SalStreamDescription &getStreamAtIdx(unsigned int idx) const;

	void addTcap(const unsigned int &idx, const std::string &value);
	void addTcapToStream(const std::size_t &streamIdx, const unsigned int &idx, const std::string &value);
	const std::string &getTcap(const unsigned int &idx) const;
	const SalStreamDescription::tcap_map_t &getTcaps() const;
	const SalStreamDescription::tcap_map_t getAllTcapForStream(const unsigned int &idx) const;
	unsigned int getFreeTcapIdx() const;

	void addAcap(const unsigned int &idx, const std::string &name, const std::string &value);
	void addAcapToStream(const std::size_t &streamIdx,
	                     const unsigned int &idx,
	                     const std::string &name,
	                     const std::string &value);
	const SalStreamDescription::acap_t &getAcap(const unsigned int &idx) const;
	const SalStreamDescription::acap_map_t &getAcaps() const;
	const SalStreamDescription::acap_map_t getAllAcapForStream(const unsigned int &idx) const;
	unsigned int getFreeAcapIdx() const;

	const SalStreamDescription::cfg_map getCfgsForStream(const unsigned int &idx) const;
	// Creates potential configuration based on stored tcap and acaps
	void createPotentialConfigurationsForStream(const unsigned int &streamIdx,
	                                            const bool delete_session_attributes,
	                                            const bool delete_media_attributes);

	std::string name;
	std::string addr;
	std::string username;
	int bandwidth = 0;
	std::string origin_addr;
	unsigned int session_ver = 0;
	unsigned int session_id = 0;
	SalStreamDir dir = SalStreamSendRecv;
	std::vector<SalStreamDescription> streams;
	SalCustomSdpAttribute *custom_sdp_attributes = nullptr;
	OrtpRtcpXrConfiguration rtcp_xr{};
	std::string ice_ufrag;
	std::string ice_pwd;
	std::list<SalStreamBundle> bundles;
	bool ice_lite = false;
	bool set_nortpproxy = false;
	bool accept_bundles = false; /* Set to true if RTP bundles can be accepted during offer answer. This field has no
	                                appearance on the SDP.*/
	bool haveLimeIk = false;     /**< flag for lime Ik presence */

	SalMediaRecord record = SalMediaRecordNone;

	std::list<std::pair<time_t, time_t>> times;

private:
	SalStreamDescription::acap_map_t acaps;
	SalStreamDescription::tcap_map_t tcaps;

	mutable SalMediaDescriptionParams params;

	std::vector<SalStreamDescription>::const_iterator findFirstStreamItOfType(SalStreamType type,
	                                                                          int startingIdx = -1) const;
	std::vector<SalStreamDescription>::const_iterator
	findStreamItWithSdpAttribute(const std::vector<std::pair<std::string, std::string>> &attributes) const;
	std::vector<SalStreamDescription>::const_iterator
	findStreamItWithSdpAttribute(const SalStreamType type,
	                             const std::vector<std::pair<std::string, std::string>> &attributes) const;
	std::vector<SalStreamDescription>::const_iterator findStreamIt(SalMediaProto proto, SalStreamType type) const;
	std::vector<SalStreamDescription>::const_iterator findStreamItWithLabel(SalStreamType type,
	                                                                        const std::string label) const;
	std::vector<SalStreamDescription>::const_iterator findStreamItWithContent(const std::string content) const;
	std::vector<SalStreamDescription>::const_iterator findStreamItWithContent(const std::string content,
	                                                                          const SalStreamDir direction) const;
	std::vector<SalStreamDescription>::const_iterator findStreamItWithContent(const std::string content,
	                                                                          const std::string label) const;

	/*check for the presence of at least one stream with requested direction */
	bool containsStreamWithDir(const SalStreamDir &stream_dir) const;
	bool containsStreamWithDir(const SalStreamDir &stream_dir, const SalStreamType &type) const;

	bool isNullAddress(const std::string &addr) const;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _SAL_MEDIA_DESCRIPTION_H_
