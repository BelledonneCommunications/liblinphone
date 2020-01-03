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

#include "private.h"

#include "ice-service.h"
#include "conference/session/streams.h"
#include "conference/session/media-session-p.h"

#ifdef HAVE_GETIFADDRS
#include <sys/types.h>
#include <ifaddrs.h>
#endif

using namespace::std;

LINPHONE_BEGIN_NAMESPACE

IceService::IceService(StreamsGroup & sg) : mStreamsGroup(sg){
}

IceService::~IceService(){
	deleteSession();
}

bool IceService::isActive() const{
	return mIceSession != nullptr;
}

bool IceService::hasCompleted() const{
	if (!isActive()) return true; // Completed because nothing to do.
	return ice_session_state(mIceSession) == IS_Completed;
}

MediaSessionPrivate &IceService::getMediaSessionPrivate() const{
	return mStreamsGroup.getMediaSessionPrivate();
}

bool IceService::iceFoundInMediaDescription (const SalMediaDescription *md) {
	if ((md->ice_pwd[0] != '\0') && (md->ice_ufrag[0] != '\0'))
		return true;
	for (int i = 0; i < md->nb_streams; i++) {
		const SalStreamDescription *stream = &md->streams[i];
		if ((stream->ice_pwd[0] != '\0') && (stream->ice_ufrag[0] != '\0')){
			return true;
		}
		
	}
	return false;
}

void IceService::checkSession (IceRole role) {
	LinphoneNatPolicy *natPolicy = getMediaSessionPrivate().getNatPolicy();
	if (!natPolicy || !linphone_nat_policy_ice_enabled(natPolicy)){
		return;
	}
	
	// Already created.
	if (mIceSession)
		return;

	LinphoneConfig *config = linphone_core_get_config(getCCore());
	
	if (lp_config_get_int(config, "net", "force_ice_disablement", 0)){
		lWarning()<<"ICE is disabled in this version";
		return;
	}
	
	mIceSession = ice_session_new();

	// For backward compatibility purposes, shall be enabled by default in the future.
	ice_session_enable_message_integrity_check(
		mIceSession,
		!!lp_config_get_int(config, "net", "ice_session_enable_message_integrity_check", 1)
	);
	if (lp_config_get_int(config, "net", "dont_default_to_stun_candidates", 0)) {
		IceCandidateType types[ICT_CandidateTypeMax];
		types[0] = ICT_HostCandidate;
		types[1] = ICT_RelayedCandidate;
		types[2] = ICT_CandidateInvalid;
		ice_session_set_default_candidates_types(mIceSession, types);
	}
	ice_session_set_role(mIceSession, role);
}

void IceService::fillLocalMediaDescription(OfferAnswerContext & ctx){
	updateLocalMediaDescriptionFromIce(ctx.localMediaDescription);
}

void IceService::createStreams(const OfferAnswerContext &params){
	checkSession(params.localIsOfferer ? IR_Controlling : IR_Controlled);
	if (!mIceSession) return;
	if (!params.localIsOfferer){
		// This may delete the ice session.
		updateFromRemoteMediaDescription(params.localMediaDescription, params.remoteMediaDescription, false);
	}
}

bool IceService::prepare(){
	if (!mIceSession) return false;
	
	const auto & streams = mStreamsGroup.getStreams();
	for (auto & stream : streams){
		size_t index = stream->getIndex();
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)index);
		if (!cl) {
			cl = ice_check_list_new();
			ice_session_add_check_list(mIceSession, cl, static_cast<unsigned int>(index));
			lInfo() << "Created new ICE check list for stream #" << index;
		}
		stream->setIceCheckList(cl);
	}
	// Start ICE gathering if needed.
	if (!ice_session_candidates_gathered(mIceSession)) {
		int err = gatherIceCandidates();
		if (err == 0) {
			// Ice candidates gathering wasn't started, but we can proceed with the call anyway.
			return false;
		} else if (err == -1) {
			deleteSession();
			return false;
		}
		return true;
	}
	return false;
}

LinphoneCore *IceService::getCCore()const{
	return mStreamsGroup.getCCore();
}

#ifdef HAVE_GETIFADDRS
list<string> IceService::fetchWithGetIfAddrs(bool ipv6Allowed)const{
	list<string> ret;
	struct ifaddrs **ifap = nullptr;
	
	if (getifaddrs(&ifap) == 0){
		struct ifaddrs *ifaddr;
		for (ifaddr = *ifap; ifaddr != nullptr; ifaddr = ifaddr->ifa_next){
			if (ifaddr->ifa_flags & ){
				struct sockaddr *saddr = ifaddr->ifa_addr;
				char addr[INET6_ADDRSTRLEN] = { 0 };
				if (!saddr){
					lError << "NULL sockaddr returned by getifaddrs().";
					continue;
				}
				switch (saddr->sa_family){
					case AF_INET:
						if (inet_ntop(AF_INET, &((struct sockaddr_in*)saddr)->sin_addr, addr, sizeof(addr)) != nullptr){
							ret.push_back(addr);
						}else{
							lError() << "inet_ntop() failed with AF_INET: " << strerror(errno);
						}
					break;
					case AF_INET6:
						if (!ipv6Allowed) continue;
						if (inet_ntop(AF_INET6, &((struct sockaddr_in6*)saddr)->sin6_addr, addr, sizeof(addr)) != nullptr){
							ret.push_back(addr);
						}else{
							lError() << "inet_ntop() failed with AF_INET6: " << strerror(errno);
						}
					break;
					default:
						// ignored.
					break;
				}
			}
		}
		freeifaddrs(ifap);
	}else{
		lError() << "getifaddrs(): " << strerror(errno);
	}
}
#endif

list<string> IceService::fetchLocalAddresses()const{
	list<string> ret;
	bool ipv6Allowed = linphone_core_ipv6_enabled(getCCore());
	
#ifdef HAVE_GETIFADDRS
	ret = fetchWithGetIfAddrs(ipv6Allowed);
#endif
	/*
	 * FIXME: implement here code for WIN32 that fetches all addresses of all interfaces.
	 */
	
	/*
	 * Finally if none of the above methods worked, fallback with linphone_core_get_local_ip() that uses the socket/connect/getsockname method
	 * to get the local ip address that has the route to public internet.
	 */
	if (ret.empty()){
		char localAddr[LINPHONE_IPADDR_SIZE];
		if (ipv6Allowed){
			if (linphone_core_get_local_ip_for(AF_INET6, nullptr, localAddr) == 0) {
				ret.push_back(localAddr);
			}else{
				lError() << "IceService::fetchLocalAddresses(): Fail to get default IPv6";
			}
		}
		if (linphone_core_get_local_ip_for(AF_INET, nullptr, localAddr) == 0){
			ret.push_back(localAddr);
		}else{
			lError() << "IceService::fetchLocalAddresses(): Fail to get default IPv4";
		}
	}
	return ret;
}

void IceService::gatherLocalCandidates(){
	list<string> localAddrs = fetchLocalAddresses();
	
	const auto & streams = mStreamsGroup.getStreams();
	for (auto & stream : streams){
		size_t index = stream->getIndex();
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)index);
		if (cl) {
			if ((ice_check_list_state(cl) != ICL_Completed) && !ice_check_list_candidates_gathered(cl)) {
				for (const string & addr : localAddrs){
					int family = addr.find(':') != string::npos ? AF_INET6 : AF_INET;
					ice_add_local_candidate(cl, "host", family, addr.c_str(), stream->getPortConfig().rtpPort, 1, nullptr);
					ice_add_local_candidate(cl, "host", family, addr.c_str(), stream->getPortConfig().rtcpPort, 2, nullptr);
				}
			}
		}
	}
}

/** Return values:
 *  1: STUN gathering is started
 *  0: no STUN gathering is started, but it's ok to proceed with ICE anyway (with local candidates only or because STUN gathering was already done before)
 * -1: no gathering started and something went wrong with local candidates. There is no way to start the ICE session.
 */
int IceService::gatherIceCandidates () {
	const struct addrinfo *ai = nullptr;
	LinphoneNatPolicy *natPolicy = getMediaSessionPrivate().getNatPolicy();
	if (natPolicy && linphone_nat_policy_stun_server_activated(natPolicy)) {
		ai = linphone_nat_policy_get_stun_server_addrinfo(natPolicy);
		if (ai)
			ai = getIcePreferredStunServerAddrinfo(ai);
		else
			lWarning() << "Failed to resolve STUN server for ICE gathering, continuing without STUN";
	} else
		lWarning() << "ICE is used without STUN server";
	LinphoneCore *core = getCCore();
	ice_session_enable_forced_relay(mIceSession, core->forced_ice_relay);
	ice_session_enable_short_turn_refresh(mIceSession, core->short_turn_refresh);

	// Gather local host candidates.
	gatherLocalCandidates();
	
	if (ai && natPolicy && linphone_nat_policy_stun_server_activated(natPolicy)) {
		string server = linphone_nat_policy_get_stun_server(natPolicy);
		lInfo() << "ICE: gathering candidates from [" << server << "] using " << (linphone_nat_policy_turn_enabled(natPolicy) ? "TURN" : "STUN");
		// Gather local srflx candidates.
		ice_session_enable_turn(mIceSession, linphone_nat_policy_turn_enabled(natPolicy));
		ice_session_set_stun_auth_requested_cb(mIceSession, MediaSessionPrivate::stunAuthRequestedCb, &getMediaSessionPrivate());
		return ice_session_gather_candidates(mIceSession, ai->ai_addr, (socklen_t)ai->ai_addrlen) ? 1 : 0;
	} else {
		lInfo() << "ICE: bypass candidates gathering";
		ice_session_compute_candidates_foundations(mIceSession);
		ice_session_eliminate_redundant_candidates(mIceSession);
		ice_session_choose_default_candidates(mIceSession);
	}
	return 0;
}

bool IceService::checkForIceRestartAndSetRemoteCredentials (const SalMediaDescription *md, bool isOffer) {
	bool iceRestarted = false;
	string addr = md->addr;
	if ((addr == "0.0.0.0") || (addr == "::0")) {
		ice_session_restart(mIceSession, isOffer ? IR_Controlled : IR_Controlling);
		iceRestarted = true;
	} else {
		for (int i = 0; i < md->nb_streams; i++) {
			const SalStreamDescription *stream = &md->streams[i];
			IceCheckList *cl = ice_session_check_list(mIceSession, i);
			string rtpAddr = stream->rtp_addr;
			if (cl && (rtpAddr == "0.0.0.0")) {
				ice_session_restart(mIceSession, isOffer ? IR_Controlled : IR_Controlling);
				iceRestarted = true;
				break;
			}
		}
	}
	if (!ice_session_remote_ufrag(mIceSession) && !ice_session_remote_pwd(mIceSession)) {
		ice_session_set_remote_credentials(mIceSession, md->ice_ufrag, md->ice_pwd);
	} else if (ice_session_remote_credentials_changed(mIceSession, md->ice_ufrag, md->ice_pwd)) {
		if (!iceRestarted) {
			ice_session_restart(mIceSession, isOffer ? IR_Controlled : IR_Controlling);
			iceRestarted = true;
		}
		ice_session_set_remote_credentials(mIceSession, md->ice_ufrag, md->ice_pwd);
	}
	for (int i = 0; i < md->nb_streams; i++) {
		const SalStreamDescription *stream = &md->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, i);
		if (cl && (stream->ice_pwd[0] != '\0') && (stream->ice_ufrag[0] != '\0')) {
			if (ice_check_list_remote_credentials_changed(cl, stream->ice_ufrag, stream->ice_pwd)) {
				if (!iceRestarted && ice_check_list_get_remote_ufrag(cl) && ice_check_list_get_remote_pwd(cl)) {
					// Restart only if remote ufrag/paswd was already set.
					ice_session_restart(mIceSession, isOffer ? IR_Controlled : IR_Controlling);
					iceRestarted = true;
				}
				ice_check_list_set_remote_credentials(cl, stream->ice_ufrag, stream->ice_pwd);
			}
		}
	}
	return iceRestarted;
}

void IceService::getIceDefaultAddrAndPort (
	uint16_t componentID,
	const SalMediaDescription *md,
	const SalStreamDescription *stream,
	const char **addr,
	int *port
) {
	if (componentID == 1) {
		*addr = stream->rtp_addr;
		*port = stream->rtp_port;
	} else if (componentID == 2) {
		*addr = stream->rtcp_addr;
		*port = stream->rtcp_port;
	} else
		return;
	if ((*addr)[0] == '\0') *addr = md->addr;
}

void IceService::createIceCheckListsAndParseIceAttributes (const SalMediaDescription *md, bool iceRestarted) {
	for (int i = 0; i < md->nb_streams; i++) {
		const SalStreamDescription *stream = &md->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, i);
		if (!cl)
			continue;
		if (stream->ice_mismatch) {
			ice_check_list_set_state(cl, ICL_Failed);
			continue;
		}
		if (stream->rtp_port == 0) {
			ice_session_remove_check_list(mIceSession, cl);
			mStreamsGroup.getStream((size_t)i)->setIceCheckList(nullptr);
			continue;
		}
		if ((stream->ice_pwd[0] != '\0') && (stream->ice_ufrag[0] != '\0'))
			ice_check_list_set_remote_credentials(cl, stream->ice_ufrag, stream->ice_pwd);
		for (int j = 0; j < SAL_MEDIA_DESCRIPTION_MAX_ICE_CANDIDATES; j++) {
			bool defaultCandidate = false;
			const SalIceCandidate *candidate = &stream->ice_candidates[j];
			if (candidate->addr[0] == '\0')
				break;
			if ((candidate->componentID == 0) || (candidate->componentID > 2))
				continue;
			const char *addr = nullptr;
			int port = 0;
			getIceDefaultAddrAndPort(static_cast<uint16_t>(candidate->componentID), md, stream, &addr, &port);
			if (addr && (candidate->port == port) && (strlen(candidate->addr) == strlen(addr)) && (strcmp(candidate->addr, addr) == 0))
				defaultCandidate = true;
			int family = AF_INET;
			if (strchr(candidate->addr, ':'))
				family = AF_INET6;
			ice_add_remote_candidate(
				cl, candidate->type, family, candidate->addr, candidate->port,
				static_cast<uint16_t>(candidate->componentID),
				candidate->priority, candidate->foundation, defaultCandidate
			);
		}
		if (!iceRestarted) {
			bool losingPairsAdded = false;
			for (int j = 0; j < SAL_MEDIA_DESCRIPTION_MAX_ICE_REMOTE_CANDIDATES; j++) {
				const SalIceRemoteCandidate *remoteCandidate = &stream->ice_remote_candidates[j];
				const char *addr = nullptr;
				int port = 0;
				int componentID = j + 1;
				if (remoteCandidate->addr[0] == '\0') break;
				getIceDefaultAddrAndPort(static_cast<uint16_t>(componentID), md, stream, &addr, &port);

				// If we receive a re-invite with remote-candidates, supply these pairs to the ice check list.
				// They might be valid pairs already selected, or losing pairs.

				int remoteFamily = AF_INET;
				if (strchr(remoteCandidate->addr, ':'))
					remoteFamily = AF_INET6;
				int family = AF_INET;
				if (strchr(addr, ':'))
					family = AF_INET6;
				ice_add_losing_pair(cl, static_cast<uint16_t>(j + 1), remoteFamily, remoteCandidate->addr, remoteCandidate->port, family, addr, port);
				losingPairsAdded = true;
			}
			if (losingPairsAdded)
				ice_check_list_check_completed(cl);
		}
	}
}

void IceService::clearUnusedIceCandidates (const SalMediaDescription *localDesc, const SalMediaDescription *remoteDesc) {
	if (!localDesc)
		return;
	for (int i = 0; i < remoteDesc->nb_streams; i++) {
		const SalStreamDescription *localStream = &localDesc->streams[i];
		const SalStreamDescription *stream = &remoteDesc->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, i);
		if (!cl || !localStream)
			continue;
		if (stream->rtcp_mux && localStream->rtcp_mux) {
			ice_check_list_remove_rtcp_candidates(cl);
		}
	}
}

void IceService::updateFromRemoteMediaDescription(const SalMediaDescription *localDesc, const SalMediaDescription *remoteDesc, bool isOffer) {
	if (!mIceSession)
		return;

	if (!iceFoundInMediaDescription(remoteDesc)) {
		// Response from remote does not contain mandatory ICE attributes, delete the session.
		deleteSession();
		return;
	}

	// Check for ICE restart and set remote credentials.
	bool iceRestarted = checkForIceRestartAndSetRemoteCredentials(remoteDesc, isOffer);

	// Create ICE check lists if needed and parse ICE attributes.
	createIceCheckListsAndParseIceAttributes(remoteDesc, iceRestarted);
	for (int i = 0; i < remoteDesc->nb_streams; i++) {
		const SalStreamDescription *stream = &remoteDesc->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, i);
		if (!cl) continue;
		if (!sal_stream_description_active(stream)) {
			ice_session_remove_check_list_from_idx(mIceSession, static_cast<unsigned int>(i));
			mStreamsGroup.getStream(i)->setIceCheckList(nullptr);
		}
	}
	clearUnusedIceCandidates(localDesc, remoteDesc);
	ice_session_check_mismatch(mIceSession);

	if (ice_session_nb_check_lists(mIceSession) == 0) {
		deleteSession();
	}
}


void IceService::updateLocalMediaDescriptionFromIce (SalMediaDescription *desc) {
	if (!mIceSession)
		return;
	IceCandidate *rtpCandidate = nullptr;
	IceCandidate *rtcpCandidate = nullptr;
	bool result = false;
	IceSessionState sessionState = ice_session_state(mIceSession);
	if (sessionState == IS_Completed) {
		IceCheckList *firstCl = nullptr;
		for (int i = 0; i < desc->nb_streams; i++) {
			IceCheckList *cl = ice_session_check_list(mIceSession, i);
			if (cl) {
				firstCl = cl;
				break;
			}
		}
		if (firstCl)
			result = !!ice_check_list_selected_valid_local_candidate(firstCl, &rtpCandidate, nullptr);
		if (result) {
			strncpy(desc->addr, rtpCandidate->taddr.ip, sizeof(desc->addr));
		} else {
			lWarning() << "If ICE has completed successfully, rtp_candidate should be set!";
			ice_dump_valid_list(firstCl);
		}
	}

	strncpy(desc->ice_pwd, ice_session_local_pwd(mIceSession), sizeof(desc->ice_pwd)-1);
	strncpy(desc->ice_ufrag, ice_session_local_ufrag(mIceSession), sizeof(desc->ice_ufrag)-1);
	
	for (int i = 0; i < desc->nb_streams; i++) {
		SalStreamDescription *stream = &desc->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, i);
		rtpCandidate = rtcpCandidate = nullptr;
		if (!sal_stream_description_active(stream) || !cl)
			continue;
		if (ice_check_list_state(cl) == ICL_Completed) {
			result = !!ice_check_list_selected_valid_local_candidate(ice_session_check_list(mIceSession, i), &rtpCandidate, &rtcpCandidate);
		} else {
			result = !!ice_check_list_default_local_candidate(ice_session_check_list(mIceSession, i), &rtpCandidate, &rtcpCandidate);
		}
		if (result) {
			strncpy(stream->rtp_addr, rtpCandidate->taddr.ip, sizeof(stream->rtp_addr) -1);
			strncpy(stream->rtcp_addr, rtcpCandidate->taddr.ip, sizeof(stream->rtcp_addr) -1);
			stream->rtp_port = rtpCandidate->taddr.port;
			stream->rtcp_port = rtcpCandidate->taddr.port;
		} else {
			memset(stream->rtp_addr, 0, sizeof(stream->rtp_addr));
			memset(stream->rtcp_addr, 0, sizeof(stream->rtcp_addr));
		}
		if ((strlen(ice_check_list_local_pwd(cl)) != strlen(desc->ice_pwd)) || (strcmp(ice_check_list_local_pwd(cl), desc->ice_pwd)))
			strncpy(stream->ice_pwd, ice_check_list_local_pwd(cl), sizeof(stream->ice_pwd) - 1);
		else
			memset(stream->ice_pwd, 0, sizeof(stream->ice_pwd));
		if ((strlen(ice_check_list_local_ufrag(cl)) != strlen(desc->ice_ufrag)) || (strcmp(ice_check_list_local_ufrag(cl), desc->ice_ufrag)))
			strncpy(stream->ice_ufrag, ice_check_list_local_ufrag(cl), sizeof(stream->ice_ufrag) -1 );
		else
			memset(stream->ice_pwd, 0, sizeof(stream->ice_pwd));
		stream->ice_mismatch = ice_check_list_is_mismatch(cl);
		if ((ice_check_list_state(cl) == ICL_Running) || (ice_check_list_state(cl) == ICL_Completed)) {
			memset(stream->ice_candidates, 0, sizeof(stream->ice_candidates));
			int nbCandidates = 0;
			for (int j = 0; j < MIN((int)bctbx_list_size(cl->local_candidates), SAL_MEDIA_DESCRIPTION_MAX_ICE_CANDIDATES); j++) {
				SalIceCandidate *salCandidate = &stream->ice_candidates[nbCandidates];
				IceCandidate *iceCandidate = reinterpret_cast<IceCandidate *>(bctbx_list_nth_data(cl->local_candidates, j));
				const char *defaultAddr = nullptr;
				int defaultPort = 0;
				if (iceCandidate->componentID == 1) {
					defaultAddr = stream->rtp_addr;
					defaultPort = stream->rtp_port;
				} else if (iceCandidate->componentID == 2) {
					defaultAddr = stream->rtcp_addr;
					defaultPort = stream->rtcp_port;
				} else
					continue;
				if (defaultAddr[0] == '\0')
					defaultAddr = desc->addr;
				// Only include the candidates matching the default destination for each component of the stream if the state is Completed as specified in RFC5245 section 9.1.2.2.
				if (
					ice_check_list_state(cl) == ICL_Completed &&
					!((iceCandidate->taddr.port == defaultPort) && (strlen(iceCandidate->taddr.ip) == strlen(defaultAddr)) && (strcmp(iceCandidate->taddr.ip, defaultAddr) == 0))
				)
					continue;
				strncpy(salCandidate->foundation, iceCandidate->foundation, sizeof(salCandidate->foundation) - 1);
				salCandidate->componentID = iceCandidate->componentID;
				salCandidate->priority = iceCandidate->priority;
				strncpy(salCandidate->type, ice_candidate_type(iceCandidate), sizeof(salCandidate->type) - 1);
				strncpy(salCandidate->addr, iceCandidate->taddr.ip, sizeof(salCandidate->addr) - 1);
				salCandidate->port = iceCandidate->taddr.port;
				if (iceCandidate->base && (iceCandidate->base != iceCandidate)) {
					strncpy(salCandidate->raddr, iceCandidate->base->taddr.ip, sizeof(salCandidate->raddr) - 1);
					salCandidate->rport = iceCandidate->base->taddr.port;
				}
				nbCandidates++;
			}
		}
		if ((ice_check_list_state(cl) == ICL_Completed) && (ice_session_role(mIceSession) == IR_Controlling)) {
			memset(stream->ice_remote_candidates, 0, sizeof(stream->ice_remote_candidates) -1);
			if (ice_check_list_selected_valid_remote_candidate(cl, &rtpCandidate, &rtcpCandidate)) {
				strncpy(stream->ice_remote_candidates[0].addr, rtpCandidate->taddr.ip, sizeof(stream->ice_remote_candidates[0].addr) -1);
				stream->ice_remote_candidates[0].port = rtpCandidate->taddr.port;
				strncpy(stream->ice_remote_candidates[1].addr, rtcpCandidate->taddr.ip, sizeof(stream->ice_remote_candidates[1].addr) -1);
				stream->ice_remote_candidates[1].port = rtcpCandidate->taddr.port;
			} else
				lError() << "ice: Selected valid remote candidates should be present if the check list is in the Completed state";
		} else {
			for (int j = 0; j < SAL_MEDIA_DESCRIPTION_MAX_ICE_REMOTE_CANDIDATES; j++) {
				stream->ice_remote_candidates[j].addr[0] = '\0';
				stream->ice_remote_candidates[j].port = 0;
			}
		}
	}
}

void IceService::gatheringFinished () {
	const SalMediaDescription *rmd = mStreamsGroup.getCurrentOfferAnswerContext().remoteMediaDescription;
	if (rmd)
		clearUnusedIceCandidates(mStreamsGroup.getCurrentOfferAnswerContext().localMediaDescription, rmd);
	if (!mIceSession)
		return;

	ice_session_compute_candidates_foundations(mIceSession);
	ice_session_eliminate_redundant_candidates(mIceSession);
	ice_session_choose_default_candidates(mIceSession);

	int pingTime = ice_session_average_gathering_round_trip_time(mIceSession);
	if (pingTime >= 0) {
		/* FIXME: is ping time still useful for the MediaSession ? */
		getMediaSessionPrivate().setPingTime(pingTime);
	}
}


/**
 * Choose the preferred IP address to use to contact the STUN server from the list of IP addresses
 * the DNS resolution returned. If a NAT64 address is present, use it, otherwise if an IPv4 address
 * is present, use it, otherwise use an IPv6 address if it is present.
 */
const struct addrinfo *IceService::getIcePreferredStunServerAddrinfo (const struct addrinfo *ai) {
	// Search for NAT64 addrinfo.
	const struct addrinfo *it = ai;
	while (it) {
		if (it->ai_family == AF_INET6) {
			struct sockaddr_storage ss;
			socklen_t sslen = sizeof(ss);
			memset(&ss, 0, sizeof(ss));
			bctbx_sockaddr_remove_nat64_mapping(it->ai_addr, (struct sockaddr *)&ss, &sslen);
			if (ss.ss_family == AF_INET) break;
		}
		it = it->ai_next;
	}
	const struct addrinfo *preferredAi = it;
	if (!preferredAi) {
		// Search for IPv4 addrinfo.
		it = ai;
		while (it) {
			if (it->ai_family == AF_INET)
				break;
			if ((it->ai_family == AF_INET6) && (it->ai_flags & AI_V4MAPPED))
				break;
			it = it->ai_next;
		}
		preferredAi = it;
	}
	if (!preferredAi) {
		// Search for IPv6 addrinfo.
		it = ai;
		while (it) {
			if (it->ai_family == AF_INET6)
				break;
			it = it->ai_next;
		}
		preferredAi = it;
	}
	return preferredAi;
}

void IceService::finishPrepare(){
	gatheringFinished();
}

void IceService::render(const OfferAnswerContext & ctx, CallSession::State targetState){
	if (mIceSession && ice_session_state(mIceSession) != IS_Completed)
		ice_session_start_connectivity_checks(mIceSession);
}

void IceService::sessionConfirmed(const OfferAnswerContext &ctx){
}

void IceService::stop(){
	//Nothing to do. The ice session can survive.
}

void IceService::finish(){
	deleteSession();
}

void IceService::deleteSession () {
	if (!mIceSession)
		return;
	/* clear all check lists */
	for (auto & stream : mStreamsGroup.getStreams())
		stream->setIceCheckList(nullptr);
	ice_session_destroy(mIceSession);
	mIceSession = nullptr;
}

void IceService::setListener(IceServiceListener *listener){
	mListener = listener;
}

bool IceService::hasCompletedCheckList () const {
	if (!mIceSession)
		return false;
	switch (ice_session_state(mIceSession)) {
		case IS_Completed:
		case IS_Failed:
			return !!ice_session_has_completed_check_list(mIceSession);
		default:
			return false;
	}
}

void IceService::handleIceEvent(const OrtpEvent *ev){
	OrtpEventType evt = ortp_event_get_type(ev);
	const OrtpEventData *evd = ortp_event_get_data(const_cast<OrtpEvent*>(ev));
	
	switch (evt){
		case ORTP_EVENT_ICE_SESSION_PROCESSING_FINISHED:
			if (hasCompletedCheckList()) {
				if (mListener) mListener->onIceCompleted(*this);
			}
		break;
		case ORTP_EVENT_ICE_GATHERING_FINISHED:
			if (!evd->info.ice_processing_successful)
				lWarning() << "No STUN answer from [" << linphone_nat_policy_get_stun_server(getMediaSessionPrivate().getNatPolicy()) << "], continuing without STUN";
			mStreamsGroup.finishPrepare();
			if (mListener) mListener->onGatheringFinished(*this);
		break;
		case ORTP_EVENT_ICE_LOSING_PAIRS_COMPLETED:
			if (mListener) mListener->onLosingPairsCompleted(*this);
		break;
		case ORTP_EVENT_ICE_RESTART_NEEDED:
			if (mListener) mListener->onIceRestartNeeded(*this);
		break;
	}
}


LINPHONE_END_NAMESPACE
