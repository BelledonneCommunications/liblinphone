/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#ifndef _SAL_MEDIA_DESCRIPTION_H_
#define _SAL_MEDIA_DESCRIPTION_H_

#include "ortp/rtpsession.h"
#include "sal/sal_stream_bundle.h"

typedef struct SalMediaDescription {
	public:
		SalMediaDescription();
		SalMediaDescription(const SalMediaDescription & other);
		~SalMediaDescription();
		void init();
		void destroy();

		SalStreamBundle * addNewBundle();

		int lookupMid(const std::string mid) const;
		const SalStreamBundle *getBundleFromMid(const std::string mid) const;
		int getIndexOfTransportOwner(const SalStreamDescription *sd) const;

		SalMediaDescription * ref();
		void unref();

		const SalStreamDescription *findStream(SalMediaProto proto, SalStreamType type) const;
		unsigned int nbActiveStreamsOfType(SalStreamType type) const;
		const SalStreamDescription * getActiveStreamOfType(SalStreamType type, unsigned int idx) const;
		const SalStreamDescription * findSecureStreamOfType(SalStreamType type) const;
		const SalStreamDescription * findBestStream(SalStreamType type) const;

		bool_t isEmpty() const;

		void setDir(SalStreamDir stream_dir);

		int getNbActiveStreams() const;

		bool hasDir(const SalStreamDir & stream_dir) const;
		bool_t hasAvpf() const;
		bool_t hasImplicitAvpf() const;
		bool_t hasSrtp() const;
		bool_t hasDtls() const;
		bool_t hasZrtp() const;
		bool_t hasIpv6() const;

		bool operator==(const SalMediaDescription & other) const;
		int equal(const SalMediaDescription & otherMd) const;
		int globalEqual(const SalMediaDescription & otherMd) const;

		static const std::string printDifferences(int result);

		size_t getNbStreams() const;
		const std::string & getAddress() const;
		const SalStreamDescription * getStreamIdx(unsigned int idx) const;

	int refcount = 0;
	std::string name;
	std::string addr;
	std::string username;
	int bandwidth = 0;
	unsigned int session_ver = 0;
	unsigned int session_id = 0;
	SalStreamDir dir = SalStreamSendRecv;
	std::vector<SalStreamDescription> streams;
	SalCustomSdpAttribute *custom_sdp_attributes = nullptr;
	OrtpRtcpXrConfiguration rtcp_xr;
	std::string ice_ufrag;
	std::string ice_pwd;
	bctbx_list_t *bundles = nullptr; /* list of SalStreamBundle */
	bool_t ice_lite = FALSE;
	bool_t set_nortpproxy = FALSE;
	bool_t accept_bundles = FALSE; /* Set to TRUE if RTP bundles can be accepted during offer answer. This field has no appearance on the SDP.*/
	std::vector<bool_t> pad;

	private:
		/*check for the presence of at least one stream with requested direction */
		bool containsStreamWithDir(const SalStreamDir & stream_dir) const; 

		bool isNullAddress(const std::string & addr) const;

} SalMediaDescription;

#endif // ifndef _SAL_MEDIA_DESCRIPTION_H_
