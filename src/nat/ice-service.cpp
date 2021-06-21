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
#include "c-wrapper/internal/c-tools.h"
#include "conference/session/streams.h"
#include "conference/session/media-session-p.h"
#include "utils/if-addrs.h"

using namespace::std;

LINPHONE_BEGIN_NAMESPACE

IceService::IceService(StreamsGroup & sg) : mStreamsGroup(sg){
	LinphoneConfig *config = linphone_core_get_config(getCCore());
	
	mAllowLateIce = !!linphone_config_get_int(config, "net", "allow_late_ice", 0);
	mEnableIntegrityCheck = !!linphone_config_get_int(config, "net", "ice_session_enable_message_integrity_check", 1);
	mDontDefaultToStunCandidates = linphone_config_get_int(config, "net", "dont_default_to_stun_candidates", 0);
}

IceService::~IceService(){
	deleteSession();
}

bool IceService::isActive() const{
	return mIceSession != nullptr;
}

bool IceService::isRunning() const{
	if (!isActive()) return false; // No running because it is not active
	return ice_session_state(mIceSession) == IS_Running;
}

bool IceService::hasCompleted() const{
	if (!isActive()) return true; // Completed because nothing to do.
	return ice_session_state(mIceSession) == IS_Completed;
}

MediaSessionPrivate &IceService::getMediaSessionPrivate() const{
	return mStreamsGroup.getMediaSessionPrivate();
}

bool IceService::iceFoundInMediaDescription (const std::shared_ptr<SalMediaDescription> & md) {
	if ((!md->ice_pwd.empty()) && (!md->ice_ufrag.empty()))
		return true;
	for (const auto & stream : md->streams) {
		if ((!stream.getIcePwd().empty()) && (!stream.getIceUfrag().empty())){
			return true;
		}
		
	}
	return false;
}

void IceService::checkSession (IceRole role, bool preferIpv6DefaultCandidates) {
	LinphoneNatPolicy *natPolicy = getMediaSessionPrivate().getNatPolicy();
	if (!natPolicy || !linphone_nat_policy_ice_enabled(natPolicy)){
		return;
	}
	
	if (!mIceSession && mIceWasDisabled){
		/* 
		 * No ICE session because it was disabled previously.
		 * Unless allow_late_ice is TRUE, don't re-create the session.
		 */
		if (!mAllowLateIce) return;
	}
	
	// Already created.
	if (mIceSession)
		return;
	
	mIceSession = ice_session_new();

	// For backward compatibility purposes, shall be enabled by default in the future.
	ice_session_enable_message_integrity_check(mIceSession, mEnableIntegrityCheck);
	if (mDontDefaultToStunCandidates) {
		IceCandidateType types[ICT_CandidateTypeMax];
		types[0] = ICT_HostCandidate;
		types[1] = ICT_RelayedCandidate;
		types[2] = ICT_CandidateInvalid;
		ice_session_set_default_candidates_types(mIceSession, types);
	}
	ice_sesession_set_default_candidates_ip_version(mIceSession, (bool_t)preferIpv6DefaultCandidates);
	ice_session_set_role(mIceSession, role);
}

void IceService::fillLocalMediaDescription(OfferAnswerContext & ctx){
	if (!mIceSession) return;

	if (mGatheringFinished){
		if (ctx.remoteMediaDescription)
			clearUnusedIceCandidates(ctx.localMediaDescription, ctx.remoteMediaDescription, ctx.localIsOfferer);
		
		ice_session_compute_candidates_foundations(mIceSession);
		ice_session_eliminate_redundant_candidates(mIceSession);
		ice_session_choose_default_candidates(mIceSession);
		mGatheringFinished = false;
	}
	updateLocalMediaDescriptionFromIce(ctx.localMediaDescription);
}

void IceService::createStreams(const OfferAnswerContext &params){
	checkSession(params.localIsOfferer ? IR_Controlling : IR_Controlled, getMediaSessionPrivate().getAf() == AF_INET6);
	
	if (!mIceSession) return;
	
	const auto & streams = mStreamsGroup.getStreams();
	for (auto & stream : streams){
		size_t index = stream->getIndex();
		params.scopeStreamToIndex(index);
		bool streamActive = params.getLocalStreamDescription().enabled();
		
		if (!params.localIsOfferer){
			int bundleOwnerIndex = params.remoteMediaDescription->getIndexOfTransportOwner(params.getRemoteStreamDescription());
			if (bundleOwnerIndex != -1 && bundleOwnerIndex != (int) index){
				lInfo() << *stream << " is part of a bundle as secondary stream, ICE not needed.";
				streamActive = false;
			}
		}else{
			RtpInterface *i = dynamic_cast<RtpInterface*>(stream.get());
			if (i && !i->isTransportOwner()){
				lInfo() << *stream << " is currently part of a bundle as secondary stream, ICE not needed.";
				streamActive = false;
			}
		}
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)index);
		if (!cl && streamActive) {
			cl = ice_check_list_new();
			ice_session_add_check_list(mIceSession, cl, static_cast<unsigned int>(index));
			lInfo() << "Created new ICE check list " << cl << " for stream #" << index;
		} else if (cl && !streamActive){
			ice_session_remove_check_list_from_idx(mIceSession, static_cast<unsigned int>(index));
			cl = nullptr;
		}
		stream->setIceCheckList(cl);
		stream->iceStateChanged();
	}
	
	if (!params.localIsOfferer){
		if (params.remoteMediaDescription){
			// This may delete the ice session.
			updateFromRemoteMediaDescription(params.localMediaDescription, params.remoteMediaDescription, true);
		}
	}
	if (!mIceSession){
		/* ICE was disabled. */
		mIceWasDisabled = true;
	}
}

bool IceService::prepare(){
	if (!mIceSession) return false;
	
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

void IceService::gatherLocalCandidates(){
	list<string> localAddrs = IfAddrs::fetchLocalAddresses();
	bool ipv6Allowed = linphone_core_ipv6_enabled(getCCore());
	
	const auto & streams = mStreamsGroup.getStreams();
	for (auto & stream : streams){
		size_t index = stream->getIndex();
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)index);
		if (cl) {
			if ((ice_check_list_state(cl) != ICL_Completed) && !ice_check_list_candidates_gathered(cl)) {
				for (const string & addr : localAddrs){
					int family = addr.find(':') != string::npos ? AF_INET6 : AF_INET;
					if (family == AF_INET6 && !ipv6Allowed) continue;
					ice_add_local_candidate(cl, "host", family, L_STRING_TO_C(addr), stream->getPortConfig().rtpPort, 1, nullptr);
					ice_add_local_candidate(cl, "host", family, L_STRING_TO_C(addr), stream->getPortConfig().rtcpPort, 2, nullptr);
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
	int err = 0;
	
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
		if (linphone_nat_policy_turn_enabled(natPolicy)) {
			ice_session_enable_turn(mIceSession, TRUE);

			if (linphone_nat_policy_tls_turn_transport_enabled(natPolicy)) {
				ice_session_set_turn_transport(mIceSession, "tls");
			} else if (linphone_nat_policy_tcp_turn_transport_enabled(natPolicy)) {
				ice_session_set_turn_transport(mIceSession, "tcp");
			} else {
				ice_session_set_turn_transport(mIceSession, "udp");
			}

			ice_session_set_turn_root_certificate(mIceSession, linphone_core_get_root_ca(core));

			char host[NI_MAXHOST];
			int port = 0;
			linphone_parse_host_port(linphone_nat_policy_get_stun_server(natPolicy), host, sizeof(host), &port);
			ice_session_set_turn_cn(mIceSession, host);
		}
		ice_session_set_stun_auth_requested_cb(mIceSession, MediaSessionPrivate::stunAuthRequestedCb, &getMediaSessionPrivate());
		err = ice_session_gather_candidates(mIceSession, ai->ai_addr, (socklen_t)ai->ai_addrlen) ? 1 : 0;
	} else {
		lInfo() << "ICE: bypass candidates gathering";
	}
	if (err == 0) gatheringFinished();
	return err;
}

bool IceService::checkForIceRestartAndSetRemoteCredentials (const std::shared_ptr<SalMediaDescription> &md, bool isOffer) {
	bool iceRestarted = false;
	string addr = md->addr;
	if ((addr == "0.0.0.0") || (addr == "::0")) {
		restartSession(isOffer ? IR_Controlled : IR_Controlling);
		iceRestarted = true;
	} else {
		for (size_t i = 0; i < md->streams.size(); i++) {
			const auto & stream = md->streams[i];
			IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
			string rtpAddr = stream.rtp_addr;
			if (cl && (rtpAddr == "0.0.0.0")) {
				restartSession(isOffer ? IR_Controlled : IR_Controlling);
				iceRestarted = true;
				break;
			}
		}
	}
	if (!ice_session_remote_ufrag(mIceSession) && !ice_session_remote_pwd(mIceSession)) {
		if (!md->ice_ufrag.empty() && !md->ice_pwd.empty()) {
			ice_session_set_remote_credentials(mIceSession, L_STRING_TO_C(md->ice_ufrag), L_STRING_TO_C(md->ice_pwd));
		}
	} else if (ice_session_remote_credentials_changed(mIceSession, L_STRING_TO_C(md->ice_ufrag), L_STRING_TO_C(md->ice_pwd))) {
		if (!iceRestarted) {
			restartSession( isOffer ? IR_Controlled : IR_Controlling);
			iceRestarted = true;
		}
		if (!md->ice_ufrag.empty() && !md->ice_pwd.empty()) {
			ice_session_set_remote_credentials(mIceSession, L_STRING_TO_C(md->ice_ufrag), L_STRING_TO_C(md->ice_pwd));
		}
	}
	for (size_t i = 0; i < md->streams.size(); i++) {
		const auto & stream = md->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (cl && (!stream.getIcePwd().empty()) && (!stream.getIceUfrag().empty())) {
			if (ice_check_list_remote_credentials_changed(cl, L_STRING_TO_C(stream.getIceUfrag()), L_STRING_TO_C(stream.getIcePwd()))) {
				if (!iceRestarted && ice_check_list_get_remote_ufrag(cl) && ice_check_list_get_remote_pwd(cl)) {
					// Restart only if remote ufrag/paswd was already set.
					restartSession(isOffer ? IR_Controlled : IR_Controlling);
					iceRestarted = true;
				}
				ice_check_list_set_remote_credentials(cl, L_STRING_TO_C(stream.getIceUfrag()), L_STRING_TO_C(stream.getIcePwd()));
			}
		}
	}
	return iceRestarted;
}

void IceService::getIceDefaultAddrAndPort (
	uint16_t componentID,
	const std::shared_ptr<SalMediaDescription> &md,
	const SalStreamDescription & stream,
	std::string & addr,
	int & port
) {
	if (componentID == 1) {
		addr = stream.rtp_addr;
		port = stream.rtp_port;
	} else if (componentID == 2) {
		addr = stream.rtcp_addr;
		port = stream.rtcp_port;
	} else
		return;
	if (addr.empty() == true) addr = md->addr;
}

void IceService::createIceCheckListsAndParseIceAttributes (const std::shared_ptr<SalMediaDescription> &md, bool iceRestarted) {
	for (size_t i = 0; i < md->streams.size(); i++) {
		const auto & stream = md->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (!cl)
			continue;
		if (stream.getIceMismatch()) {
			ice_check_list_set_state(cl, ICL_Failed);
			continue;
		}
		if (stream.rtp_port == 0) {
			ice_session_remove_check_list(mIceSession, cl);
			mStreamsGroup.getStream(i)->setIceCheckList(nullptr);
			continue;
		}
		if ((!stream.getIcePwd().empty()) && (!stream.getIceUfrag().empty()))
			ice_check_list_set_remote_credentials(cl, L_STRING_TO_C(stream.getIceUfrag()), L_STRING_TO_C(stream.getIcePwd()));
		for (const auto & candidate : stream.ice_candidates) {
			bool defaultCandidate = false;
			if (candidate.addr[0] == '\0')
				break;
			if ((candidate.componentID == 0) || (candidate.componentID > 2))
				continue;
			std::string addr = std::string();
			int port = 0;
			getIceDefaultAddrAndPort(static_cast<uint16_t>(candidate.componentID), md, stream, addr, port);
			if ((addr.empty() == false) && (candidate.port == port) && (candidate.addr.length() == addr.length()) && (addr.compare(candidate.addr) == 0))
				defaultCandidate = true;
			int family = AF_INET;
			if (candidate.addr.find(":") != std::string::npos)
				family = AF_INET6;
			ice_add_remote_candidate(
				cl, L_STRING_TO_C(candidate.type), family, L_STRING_TO_C(candidate.addr), candidate.port,
				static_cast<uint16_t>(candidate.componentID),
				candidate.priority, L_STRING_TO_C(candidate.foundation), defaultCandidate
			);
		}
		if (!iceRestarted) {
			bool losingPairsAdded = false;
			for (int j = 0; j < static_cast<int>(stream.ice_remote_candidates.size()); j++) {
				const auto & remoteCandidate = stream.getIceRemoteCandidateAtIndex(static_cast<size_t>(j));
				std::string addr = std::string();
				int port = 0;
				int componentID = j + 1;
				if (remoteCandidate.addr.empty()) break;
				getIceDefaultAddrAndPort(static_cast<uint16_t>(componentID), md, stream, addr, port);

				// If we receive a re-invite with remote-candidates, supply these pairs to the ice check list.
				// They might be valid pairs already selected, or losing pairs.

				int remoteFamily = AF_INET;
				if (remoteCandidate.addr.find(":") != std::string::npos)
					remoteFamily = AF_INET6;
				int family = AF_INET;
				if (addr.find(':') != std::string::npos)
					family = AF_INET6;
				ice_add_losing_pair(cl, static_cast<uint16_t>(j + 1), remoteFamily, L_STRING_TO_C(remoteCandidate.addr), remoteCandidate.port, family, L_STRING_TO_C(addr), port);
				losingPairsAdded = true;
			}
			if (losingPairsAdded)
				ice_check_list_check_completed(cl);
		}
	}
}

void IceService::clearUnusedIceCandidates (const std::shared_ptr<SalMediaDescription> &localDesc, const std::shared_ptr<SalMediaDescription> &remoteDesc, bool localIsOfferer) {
	for (size_t i = 0; i < MIN(remoteDesc->streams.size(), localDesc->streams.size()); i++) {
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (!cl)
			continue;
		const auto & localStream = localDesc->streams[i];
		const auto & stream = remoteDesc->streams[i];
		if ((localIsOfferer && stream.getChosenConfiguration().rtcp_mux && localStream.getChosenConfiguration().rtcp_mux)
			|| (!localIsOfferer && stream.getChosenConfiguration().rtcp_mux)) {
			ice_check_list_remove_rtcp_candidates(cl);
		}
	}
}

void IceService::updateFromRemoteMediaDescription(const std::shared_ptr<SalMediaDescription> & localDesc, const std::shared_ptr<SalMediaDescription> & remoteDesc, bool isOffer) {
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
	for (size_t i = 0; i < remoteDesc->streams.size(); i++) {
		const auto & remoteDescStream = remoteDesc->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (!cl) continue;
		if (!remoteDescStream.enabled() || remoteDescStream.getRtpPort() == 0) {
			/*
			 * rtp_port == 0 is true when it is a secondary stream part of bundle.
			 */
			ice_session_remove_check_list_from_idx(mIceSession, static_cast<unsigned int>(i));
			auto stream = mStreamsGroup.getStream(i);
			stream->setIceCheckList(nullptr);
			stream->iceStateChanged();
			
		}
	}
	clearUnusedIceCandidates(localDesc, remoteDesc, !isOffer);
	ice_session_check_mismatch(mIceSession);

	if (ice_session_nb_check_lists(mIceSession) == 0) {
		deleteSession();
	}
}


void IceService::updateLocalMediaDescriptionFromIce (std::shared_ptr<SalMediaDescription> & desc) {
	if (!mIceSession)
		return;
	IceCandidate *rtpCandidate = nullptr;
	IceCandidate *rtcpCandidate = nullptr;
	bool result = false;
	IceSessionState sessionState = ice_session_state(mIceSession);
	bool usePerStreamUfragPassword = linphone_config_get_bool(linphone_core_get_config(getCCore()),"sip", "ice_password_ufrag_in_media_description", false);
	
	if (sessionState == IS_Completed) {
		IceCheckList *firstCl = nullptr;
		for (size_t i = 0; i < desc->streams.size(); i++) {
			IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
			if (cl) {
				firstCl = cl;
				break;
			}
		}
		if (firstCl)
			result = !!ice_check_list_selected_valid_local_candidate(firstCl, &rtpCandidate, nullptr);
		if (result) {
			desc->addr = rtpCandidate->taddr.ip;
		} else {
			lWarning() << "If ICE has completed successfully, rtp_candidate should be set!";
			ice_dump_valid_list(firstCl);
		}
	}

	if (!usePerStreamUfragPassword){
		desc->ice_pwd = L_C_TO_STRING(ice_session_local_pwd(mIceSession));
		desc->ice_ufrag = L_C_TO_STRING(ice_session_local_ufrag(mIceSession));
	}
	
	for (size_t i = 0; i < desc->streams.size(); i++) {
		auto & stream = desc->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		rtpCandidate = rtcpCandidate = nullptr;
		if (!stream.enabled() || !cl || stream.getRtpPort() == 0)
			continue;
		if (ice_check_list_state(cl) == ICL_Completed) {
			result = !!ice_check_list_selected_valid_local_candidate(ice_session_check_list(mIceSession, (int)i), &rtpCandidate, &rtcpCandidate);
		} else {
			result = !!ice_check_list_default_local_candidate(ice_session_check_list(mIceSession, (int)i), &rtpCandidate, &rtcpCandidate);
		}
		if (result) {
			stream.rtp_addr = L_C_TO_STRING(rtpCandidate->taddr.ip);
			stream.rtp_port = rtpCandidate->taddr.port;
			stream.rtcp_addr = L_C_TO_STRING(rtcpCandidate->taddr.ip);
			stream.rtcp_port = rtcpCandidate->taddr.port;
		} else {
			stream.rtp_addr.clear();
			stream.rtcp_addr.clear();
		}

		if ((strlen(ice_check_list_local_pwd(cl)) != desc->ice_pwd.length()) || (desc->ice_pwd.compare(ice_check_list_local_pwd(cl)) != 0) || usePerStreamUfragPassword) {
			stream.ice_pwd = L_C_TO_STRING(ice_check_list_local_pwd(cl));
		} else {
			stream.ice_pwd.clear();
		}

		if ((strlen(ice_check_list_local_ufrag(cl)) != desc->ice_ufrag.length()) || (desc->ice_ufrag.compare(ice_check_list_local_ufrag(cl))) || usePerStreamUfragPassword) {
			stream.ice_ufrag = L_C_TO_STRING(ice_check_list_local_ufrag(cl));
		} else {
			stream.ice_ufrag.clear();
		}

		stream.ice_mismatch = ice_check_list_is_mismatch(cl);
		if ((ice_check_list_state(cl) == ICL_Running) || (ice_check_list_state(cl) == ICL_Completed)) {
			stream.ice_candidates.clear();
			for (int j = 0; j < (int)bctbx_list_size(cl->local_candidates); j++) {
				IceCandidate *iceCandidate = reinterpret_cast<IceCandidate *>(bctbx_list_nth_data(cl->local_candidates, j));
				std::string defaultAddr = std::string();
				int defaultPort = 0;
				if (iceCandidate->componentID == 1) {
					defaultAddr = stream.rtp_addr;
					defaultPort = stream.rtp_port;
				} else if (iceCandidate->componentID == 2) {
					defaultAddr = stream.rtcp_addr;
					defaultPort = stream.rtcp_port;
				} else
					continue;
				if (defaultAddr.empty() == false)
					defaultAddr = desc->addr;
				// Only include the candidates matching the default destination for each component of the stream if the state is Completed as specified in RFC5245 section 9.1.2.2.
				if (
					ice_check_list_state(cl) == ICL_Completed &&
					!((iceCandidate->taddr.port == defaultPort) && (strlen(iceCandidate->taddr.ip) == defaultAddr.length()) && (defaultAddr.compare(iceCandidate->taddr.ip) == 0))
				)
					continue;
				SalIceCandidate salCandidate;
				salCandidate.foundation = L_C_TO_STRING(iceCandidate->foundation);
				salCandidate.componentID = iceCandidate->componentID;
				salCandidate.priority = iceCandidate->priority;
				salCandidate.type = L_C_TO_STRING(ice_candidate_type(iceCandidate));
				salCandidate.addr = L_C_TO_STRING(iceCandidate->taddr.ip);
				salCandidate.port = iceCandidate->taddr.port;
				if (iceCandidate->base && (iceCandidate->base != iceCandidate)) {
					salCandidate.raddr = L_C_TO_STRING(iceCandidate->base->taddr.ip);
					salCandidate.rport = iceCandidate->base->taddr.port;
				} else {
					salCandidate.raddr.clear();
					salCandidate.rport = 0;
				}
				stream.ice_candidates.push_back(salCandidate);
			}
		}
		if ((ice_check_list_state(cl) == ICL_Completed) && (ice_session_role(mIceSession) == IR_Controlling)) {
			stream.ice_remote_candidates.clear();
			if (ice_check_list_selected_valid_remote_candidate(cl, &rtpCandidate, &rtcpCandidate)) {
				SalIceRemoteCandidate rtp_remote_candidate;
				rtp_remote_candidate.addr = L_C_TO_STRING(rtpCandidate->taddr.ip);
				rtp_remote_candidate.port = rtpCandidate->taddr.port;
				stream.ice_remote_candidates.push_back(rtp_remote_candidate);
				if (rtcpCandidate){
					SalIceRemoteCandidate rtcp_remote_candidate;
					rtcp_remote_candidate.addr = L_C_TO_STRING(rtcpCandidate->taddr.ip);
					rtcp_remote_candidate.port = rtcpCandidate->taddr.port;
					stream.ice_remote_candidates.push_back(rtcp_remote_candidate);
				}
			} else {
				lError() << "ice: Selected valid remote candidates should be present if the check list is in the Completed state";
			}
		} else {
			for (auto & ice_remote_candidate : stream.ice_remote_candidates) {
				ice_remote_candidate.addr.clear();
				ice_remote_candidate.port = 0;
			}
		}
	}
}

void IceService::gatheringFinished () {
	if (!mIceSession)
		return;

	int pingTime = ice_session_average_gathering_round_trip_time(mIceSession);
	if (pingTime >= 0) {
		/* FIXME: is ping time still useful for the MediaSession ? */
		getMediaSessionPrivate().setPingTime(pingTime);
	}
	mGatheringFinished = true;
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
	if (!mIceSession) return;
	gatheringFinished();
}

void IceService::render(const OfferAnswerContext & ctx, CallSession::State targetState){
	if (!mIceSession) return;
	
	updateFromRemoteMediaDescription(ctx.localMediaDescription, ctx.remoteMediaDescription, !ctx.localIsOfferer);
	if (mIceSession && ice_session_state(mIceSession) != IS_Completed) {
		ice_session_start_connectivity_checks(mIceSession);
	}

	if (!mIceSession){
		/* ICE was disabled. */
		mIceWasDisabled = true;
	}

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

void IceService::restartSession (IceRole role) {
	if (!mIceSession)
		return;
	/* We use ice_session_reset(), which is similar to ice_session_restart() but it also clears local candidates.
	 * Indeed, the local candidates are always added back after restart.
	 * This avoids previously discovered and possibly non-working peer-reflexive candidates to be accumulated after successive restarts.
	 */
	ice_session_reset(mIceSession, role);
}

void IceService::resetSession() {
	if (!mIceSession)
		return;
	ice_session_reset(mIceSession, IR_Controlling);
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
		default:
			lError() << "IceService::handleIceEvent() is passed with a non-ICE event.";
		break;
	}
	/* Notify all the streams of the ICE state change, so that they can update their stats and so on. */
	for(auto & stream : mStreamsGroup.getStreams()){
		stream->iceStateChanged();
	}
}

bool IceService::isControlling () const {
	if (!mIceSession)
		return false;
	return ice_session_role(mIceSession) == IR_Controlling;
}

bool IceService::reinviteNeedsDeferedResponse(const std::shared_ptr<SalMediaDescription> & remoteMd){
	if (!mIceSession || (ice_session_state(mIceSession) != IS_Running))
		return false;

	for (size_t i = 0; i < remoteMd->streams.size(); i++) {
		const auto & stream = remoteMd->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (!cl)
			continue;

		if (stream.getIceMismatch()) {
			return false;
		}
		if ((stream.rtp_port == 0) || (ice_check_list_state(cl) != ICL_Running))
			continue;

		for (const auto & ice_remote_candidate : stream.ice_remote_candidates) {
			if (!ice_remote_candidate.addr.empty())
				return true;
		}
	}
	return false;
}


LINPHONE_END_NAMESPACE
