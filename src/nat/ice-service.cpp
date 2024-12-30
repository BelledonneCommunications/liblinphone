/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include <bctoolbox/defs.h>

#include "private.h"

#include "c-wrapper/internal/c-tools.h"
#include "conference/session/media-session-p.h"
#include "conference/session/streams.h"
#include "ice-service.h"
#include "utils/if-addrs.h"

#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

using namespace ::std;

LINPHONE_BEGIN_NAMESPACE

IceService::IceService(StreamsGroup &sg) : mStreamsGroup(sg) {
	LinphoneConfig *config = linphone_core_get_config(getCCore());
	mAllowLateIce = !!linphone_config_get_int(config, "net", "allow_late_ice", 0);
	mEnableIntegrityCheck = !!linphone_config_get_int(config, "net", "ice_session_enable_message_integrity_check", 1);
	mDontDefaultToStunCandidates = linphone_config_get_int(config, "net", "dont_default_to_stun_candidates", 0);
}

IceService::~IceService() {
	deleteSession();
}

bool IceService::isActive() const {
	return mIceSession != nullptr;
}

bool IceService::isRunning() const {
	if (!isActive()) return false; // No running because it is not active
	return ice_session_state(mIceSession) == IS_Running;
}

bool IceService::hasCompleted() const {
	if (!isActive()) return true; // Completed because nothing to do.
	return ice_session_state(mIceSession) == IS_Completed;
}

MediaSessionPrivate &IceService::getMediaSessionPrivate() const {
	return mStreamsGroup.getMediaSessionPrivate();
}

bool IceService::iceFoundInMediaDescription(const std::shared_ptr<SalMediaDescription> &md) {
	if ((!md->ice_pwd.empty()) && (!md->ice_ufrag.empty())) return true;
	for (const auto &stream : md->streams) {
		if ((!stream.getIcePwd().empty()) && (!stream.getIceUfrag().empty())) {
			return true;
		}
	}
	return false;
}

void IceService::checkSession(IceRole role, bool preferIpv6DefaultCandidates) {
	const auto natPolicy = getMediaSessionPrivate().getNatPolicy();
	if (!natPolicy || !natPolicy->iceEnabled()) {
		return;
	}

	if (!mIceSession && mIceWasDisabled) {
		/*
		 * No ICE session because it was disabled previously.
		 * Unless allow_late_ice is TRUE, don't re-create the session.
		 */
		if (!mAllowLateIce) return;
	}

	// Already created.
	if (mIceSession) return;

	mIceSession = ice_session_new();
	ice_session_set_default_candidates_ip_version(mIceSession, (bool_t)preferIpv6DefaultCandidates);
	// For backward compatibility purposes, shall be enabled by default in the future.
	ice_session_enable_message_integrity_check(mIceSession, mEnableIntegrityCheck);
	ice_session_set_role(mIceSession, role);
}

bool IceService::hasRelayCandidates(const SalMediaDescription &md) const {
	for (size_t i = 0; i < md.streams.size(); i++) {
		const auto &stream = md.streams[i];
		if (stream.rtp_port == 0) continue;
		bool goodForStream = false;
		for (const auto &candidate : stream.ice_candidates) {
			if (candidate.type == "relay") {
				goodForStream = true;
				break;
			}
		}
		if (!goodForStream) return false;
	}
	return true;
}

void IceService::chooseDefaultCandidates(const OfferAnswerContext &ctx) {
	IceCandidateType types[ICT_CandidateTypeMax];

	if (mDontDefaultToStunCandidates) {
		types[0] = ICT_HostCandidate;
		types[1] = ICT_RelayedCandidate;
		types[2] = ICT_CandidateInvalid;
	} else {
		/* In the case of an offer from remote, if the offer has relay candidates, prefer STUN as default candidate
		 * so that the TURN relay is used one side only.
		 * Otherwise, prefer the Relay as default candidate since it is supposed to always work.
		 */
		if (!ctx.localIsOfferer && ctx.remoteMediaDescription && hasRelayCandidates(*ctx.remoteMediaDescription)) {
			types[0] = ICT_ServerReflexiveCandidate;
			types[1] = ICT_RelayedCandidate;
		} else {
			types[0] = ICT_RelayedCandidate;
			types[1] = ICT_ServerReflexiveCandidate;
		}
		types[2] = ICT_HostCandidate;
		types[3] = ICT_CandidateInvalid;
	}
	ice_session_set_default_candidates_types(mIceSession, types);

	ice_session_choose_default_candidates(mIceSession);
}

void IceService::fillLocalMediaDescription(OfferAnswerContext &ctx) {
	if (!mIceSession) {
		/* fillLocalMediaDescription() is invoked multiple times. If ICE decides to shutdown in between, make sure
		 * everything set previously is cleared.*/
		ctx.localMediaDescription->ice_ufrag.clear();
		ctx.localMediaDescription->ice_pwd.clear();
		for (auto &stream : ctx.localMediaDescription->streams) {
			stream.ice_ufrag.clear();
			stream.ice_pwd.clear();
			stream.ice_candidates.clear();
		}
		return;
	}

	if (mGatheringFinished) {
		if (ctx.remoteMediaDescription)
			clearUnusedIceCandidates(ctx.localMediaDescription, ctx.remoteMediaDescription, ctx.localIsOfferer);

		ice_session_compute_candidates_foundations(mIceSession);
		ice_session_eliminate_redundant_candidates(mIceSession);
		chooseDefaultCandidates(ctx);
		mGatheringFinished = false;
	}
	updateLocalMediaDescriptionFromIce(ctx.localMediaDescription);
}

void IceService::createStreams(const OfferAnswerContext &params) {
	checkSession(params.localIsOfferer ? IR_Controlling : IR_Controlled, getMediaSessionPrivate().getAf() == AF_INET6);

	if (!mIceSession) return;

	const auto &streams = mStreamsGroup.getStreams();
	for (auto &stream : streams) {
		if (!stream) continue;

		size_t index = stream->getIndex();
		params.scopeStreamToIndex(index);

		const auto streamDesc = params.getLocalStreamDescription();
		bool streamActive = streamDesc.enabled() && (streamDesc.getDirection() != SalStreamInactive);

		/* When rtp bundle is activated or going to be activated, we don't need ICE for the stream.*/
		if (!params.localIsOfferer) {
			int bundleOwnerIndex =
			    params.remoteMediaDescription->getIndexOfTransportOwner(params.getRemoteStreamDescription());
			if (params.localMediaDescription->accept_bundles && bundleOwnerIndex != -1 &&
			    bundleOwnerIndex != (int)index) {
				lInfo() << *stream << " is part of a bundle as secondary stream, ICE not needed.";
				streamActive = false;
			}
		} else {
			RtpInterface *i = dynamic_cast<RtpInterface *>(stream.get());
			if (streamDesc.isBundleOnly() || (i && !i->isTransportOwner())) {
				lInfo() << *stream << " is currently part of a bundle as secondary stream, ICE not needed.";
				streamActive = false;
			}
		}

		IceCheckList *cl = ice_session_check_list(mIceSession, (int)index);

		if (!cl && streamActive) {
			cl = ice_check_list_new();
			ice_session_add_check_list(mIceSession, cl, static_cast<unsigned int>(index));
			lInfo() << "Created new ICE check list " << cl << " for stream #" << index;
		} else if (cl && !streamActive) {
			ice_session_remove_check_list_from_idx(mIceSession, static_cast<unsigned int>(index));
			cl = nullptr;
		}
		stream->setIceCheckList(cl);
		stream->iceStateChanged();
	}

	if (!params.localIsOfferer) {
		if (params.remoteMediaDescription) {
			// This may delete the ice session.
			updateFromRemoteMediaDescription(params.localMediaDescription, params.remoteMediaDescription, true);
		}
	}
	if (!mIceSession) {
		/* ICE was disabled. */
		mIceWasDisabled = true;
	}
}

bool IceService::needIceGathering() {
	// Start ICE gathering if needed.
	if (!ice_session_candidates_gathered(mIceSession)) {
		mInsideGatherIceCandidates = true;
		int err = gatherIceCandidates();
		mInsideGatherIceCandidates = false;
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

bool IceService::prepare() {

	if (!mIceSession) return false;

	const auto natPolicy = getMediaSessionPrivate().getNatPolicy();
	if (natPolicy && natPolicy->turnEnabled() && natPolicy->needToUpdateTurnConfiguration()) {
		natPolicy->updateTurnConfiguration([this](BCTBX_UNUSED(bool ignored)) {
			bool ret = needIceGathering();
			if (!ret) notifyEndOfPrepare();
		});
		return true;
	}
	return needIceGathering();
}

LinphoneCore *IceService::getCCore() const {
	return mStreamsGroup.getCCore();
}

int IceService::gatherLocalCandidates() {
	list<string> localAddrs = IfAddrs::fetchLocalAddresses();
	bool ipv6Allowed = linphone_core_ipv6_enabled(getCCore());
	const auto &mediaLocalIp = getMediaSessionPrivate().getMediaLocalIp();
	const auto it = std::find(localAddrs.cbegin(), localAddrs.cend(), mediaLocalIp);
	if (it == localAddrs.cend()) {
		// Add media local IP address if not already in the list in order to always include the default candidate
		localAddrs.push_back(mediaLocalIp);
	}

#if defined(__APPLE__) && TARGET_OS_IPHONE
	if (getPlatformHelpers(getCCore())->getNetworkType() == PlatformHelpers::NetworkType::Wifi &&
	    !hasLocalNetworkPermission(localAddrs))
		return -1;
#endif
	const auto &streams = mStreamsGroup.getStreams();
	for (auto &stream : streams) {
		if (!stream) continue;
		size_t index = stream->getIndex();
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)index);
		if (cl) {
			if (getMediaSessionPrivate().mandatoryRtpBundleEnabled()) {
				lInfo() << "Rtp bundle is mandatory, rtcp-mux enabled and RTCP candidates skipped.";
				rtp_session_enable_rtcp_mux(cl->rtp_session, TRUE);
			}
			if ((ice_check_list_state(cl) != ICL_Completed) && !ice_check_list_candidates_gathered(cl)) {
				for (const string &addr : localAddrs) {
					int family = addr.find(':') != string::npos ? AF_INET6 : AF_INET;
					if (family == AF_INET6 && !ipv6Allowed) continue;
					ice_add_local_candidate(cl, "host", family, L_STRING_TO_C(addr), stream->getPortConfig().rtpPort, 1,
					                        nullptr);
					if (!rtp_session_rtcp_mux_enabled(cl->rtp_session)) {
						ice_add_local_candidate(cl, "host", family, L_STRING_TO_C(addr),
						                        stream->getPortConfig().rtcpPort, 2, nullptr);
					}
				}
			}
		}
	}
	return 0;
}

void IceService::addPredefinedSflrxCandidates(const std::shared_ptr<NatPolicy> &natPolicy) {
	if (!natPolicy) return;
	bool ipv6Allowed = linphone_core_ipv6_enabled(getCCore());
	const string &ipv4 = natPolicy->getNatV4Address();
	const string &ipv6 = natPolicy->getNatV6Address();
	if (ipv4.empty() && ipv6.empty()) return;
	const auto &streams = mStreamsGroup.getStreams();
	for (auto &stream : streams) {
		if (!stream) continue;
		size_t index = stream->getIndex();
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)index);
		if (cl && ice_check_list_state(cl) != ICL_Completed && !ice_check_list_candidates_gathered(cl)) {
			if (!ipv4.empty()) {
				ice_add_local_candidate(cl, "srflx", AF_INET, L_STRING_TO_C(ipv4), stream->getPortConfig().rtpPort,
				                        ICE_RTP_COMPONENT_ID, nullptr);
			}
			if (!ipv6.empty() && ipv6Allowed) {
				ice_add_local_candidate(cl, "srflx", AF_INET6, L_STRING_TO_C(ipv6), stream->getPortConfig().rtpPort,
				                        ICE_RTP_COMPONENT_ID, nullptr);
			}
			if (!rtp_session_rtcp_mux_enabled(cl->rtp_session)) {
				if (!ipv4.empty()) {
					ice_add_local_candidate(cl, "srflx", AF_INET, L_STRING_TO_C(ipv4), stream->getPortConfig().rtcpPort,
					                        ICE_RTCP_COMPONENT_ID, nullptr);
				}
				if (!ipv6.empty() && ipv6Allowed) {
					ice_add_local_candidate(cl, "srflx", AF_INET6, L_STRING_TO_C(ipv6),
					                        stream->getPortConfig().rtcpPort, ICE_RTCP_COMPONENT_ID, nullptr);
				}
			}
		}
	}
	ice_session_set_base_for_srflx_candidates(mIceSession);
	lInfo() << "Configuration-defined server reflexive candidates added to check lists.";
}

int IceService::gatherSflrxIceCandidates(const struct addrinfo *stunServerAi) {
	int err = 0;
	const auto &natPolicy = getMediaSessionPrivate().getNatPolicy();
	if (stunServerAi) stunServerAi = getIcePreferredStunServerAddrinfo(stunServerAi);
	else lWarning() << "Failed to resolve STUN server for ICE gathering, continuing without STUN";
	if (stunServerAi) {
		LinphoneCore *core = getCCore();
		const string &server = natPolicy->getStunServer();
		lInfo() << "ICE: gathering candidates from [" << server << "] using "
		        << (natPolicy->turnEnabled() ? "TURN" : "STUN");
		// Gather local srflx candidates.
		if (natPolicy->turnEnabled()) {
			ice_session_enable_turn(mIceSession, TRUE);

			if (natPolicy->turnTlsEnabled()) {
				ice_session_set_turn_transport(mIceSession, "tls");
			} else if (natPolicy->turnTcpEnabled()) {
				ice_session_set_turn_transport(mIceSession, "tcp");
			} else {
				ice_session_set_turn_transport(mIceSession, "udp");
			}

			ice_session_set_turn_root_certificate(mIceSession, linphone_core_get_root_ca(core));

			char host[NI_MAXHOST];
			int port = 0;
			linphone_parse_host_port(server.c_str(), host, sizeof(host), &port);
			ice_session_set_turn_cn(mIceSession, host);
		}
		ice_session_set_stun_auth_requested_cb(mIceSession, MediaSessionPrivate::stunAuthRequestedCb,
		                                       &getMediaSessionPrivate());
		err = ice_session_gather_candidates(mIceSession, stunServerAi->ai_addr, (socklen_t)stunServerAi->ai_addrlen)
		          ? 1
		          : 0;
	} else {
		lInfo() << "ICE: bypass server-reflexive candidates gathering";
	}
	if (err == 0) gatheringFinished();
	return err;
}

/** Return values:
 *  1: STUN gathering is started
 *  0: no STUN gathering is started, but it's ok to proceed with ICE anyway (with local candidates only or because STUN
 * gathering was already done before) -1: no gathering started and something went wrong with local candidates. There is
 * no way to start the ICE session.
 */
int IceService::gatherIceCandidates() {
	LinphoneCore *core = getCCore();

	// Gather local host candidates.
	if (gatherLocalCandidates() == -1) {
		lError() << "Local network permission is not granted, ICE must be disabled.";
		return -1;
	}
	ice_session_enable_forced_relay(mIceSession, core->forced_ice_relay);
	ice_session_enable_short_turn_refresh(mIceSession, core->short_turn_refresh);

	const auto &natPolicy = getMediaSessionPrivate().getNatPolicy();
	addPredefinedSflrxCandidates(natPolicy);
	if (natPolicy && natPolicy->stunServerActivated()) {
		mSflrxGatheringStatus = 1; // Assume gathering is in progress.
		mAsyncStunResolverHandle = natPolicy->getStunServerAddrinfoAsync([this](const struct addrinfo *stunServerAi) {
			mAsyncStunResolverHandle = 0;
			mSflrxGatheringStatus = gatherSflrxIceCandidates(stunServerAi);
			if (mSflrxGatheringStatus != 1 && !mInsideGatherIceCandidates) {
				/* case where ICE gathering is finally not in progress, but prepare() returned true to
				 * indicate an in-progress operation because of the DNS resolution of stun server.*/
				notifyEndOfPrepare();
			}
		});
		return mSflrxGatheringStatus;
	} else {
		lInfo() << "ICE is used without STUN server";
		gatheringFinished();
	}
	return 0;
}

bool IceService::checkForIceRestartAndSetRemoteCredentials(const std::shared_ptr<SalMediaDescription> &md,
                                                           bool isOffer) {
	bool iceRestarted = false;
	string addr = md->addr;
	if ((addr == "0.0.0.0") || (addr == "::0")) {
		restartSession(isOffer ? IR_Controlled : IR_Controlling);
		iceRestarted = true;
	} else {
		for (size_t i = 0; i < md->streams.size(); i++) {
			const auto &stream = md->streams[i];
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
	} else if (ice_session_remote_credentials_changed(mIceSession, L_STRING_TO_C(md->ice_ufrag),
	                                                  L_STRING_TO_C(md->ice_pwd))) {
		if (!iceRestarted) {
			restartSession(isOffer ? IR_Controlled : IR_Controlling);
			iceRestarted = true;
		}
		if (!md->ice_ufrag.empty() && !md->ice_pwd.empty()) {
			ice_session_set_remote_credentials(mIceSession, L_STRING_TO_C(md->ice_ufrag), L_STRING_TO_C(md->ice_pwd));
		}
	}
	for (size_t i = 0; i < md->streams.size(); i++) {
		const auto &stream = md->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (cl && (!stream.getIcePwd().empty()) && (!stream.getIceUfrag().empty())) {
			if (ice_check_list_remote_credentials_changed(cl, L_STRING_TO_C(stream.getIceUfrag()),
			                                              L_STRING_TO_C(stream.getIcePwd()))) {
				if (!iceRestarted && ice_check_list_get_remote_ufrag(cl) && ice_check_list_get_remote_pwd(cl)) {
					// Restart only if remote ufrag/paswd was already set.
					restartSession(isOffer ? IR_Controlled : IR_Controlling);
					iceRestarted = true;
				}
				ice_check_list_set_remote_credentials(cl, L_STRING_TO_C(stream.getIceUfrag()),
				                                      L_STRING_TO_C(stream.getIcePwd()));
			}
		}
	}
	return iceRestarted;
}

void IceService::getIceDefaultAddrAndPort(uint16_t componentID,
                                          const std::shared_ptr<SalMediaDescription> &md,
                                          const SalStreamDescription &stream,
                                          std::string &addr,
                                          int &port) {
	if (componentID == 1) {
		addr = stream.rtp_addr;
		port = stream.rtp_port;
	} else if (componentID == 2) {
		addr = stream.rtcp_addr;
		port = stream.rtcp_port;
	} else return;
	if (addr.empty() == true) addr = md->addr;
}

void IceService::createIceCheckListsAndParseIceAttributes(const std::shared_ptr<SalMediaDescription> &md,
                                                          bool iceRestarted) {
	for (size_t i = 0; i < md->streams.size(); i++) {
		const auto &stream = md->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (!cl) continue;
		if (stream.getIceMismatch()) {
			ice_check_list_set_state(cl, ICL_Failed);
			continue;
		}
		if ((stream.rtp_port == 0) || (stream.getDirection() == SalStreamInactive)) {
			ice_session_remove_check_list(mIceSession, cl);
			mStreamsGroup.getStream(i)->setIceCheckList(nullptr);
			continue;
		}
		if ((!stream.getIcePwd().empty()) && (!stream.getIceUfrag().empty()))
			ice_check_list_set_remote_credentials(cl, L_STRING_TO_C(stream.getIceUfrag()),
			                                      L_STRING_TO_C(stream.getIcePwd()));
		for (const auto &candidate : stream.ice_candidates) {
			bool defaultCandidate = false;
			if (candidate.addr[0] == '\0') break;
			if ((candidate.componentID == 0) || (candidate.componentID > 2)) continue;
			std::string addr = std::string();
			int port = 0;
			getIceDefaultAddrAndPort(static_cast<uint16_t>(candidate.componentID), md, stream, addr, port);
			if ((addr.empty() == false) && (candidate.port == port) && (addr.compare(candidate.addr) == 0))
				defaultCandidate = true;
			int family = AF_INET;
			if (candidate.addr.find(":") != std::string::npos) family = AF_INET6;
			ice_add_remote_candidate(cl, L_STRING_TO_C(candidate.type), family, L_STRING_TO_C(candidate.addr),
			                         candidate.port, static_cast<uint16_t>(candidate.componentID), candidate.priority,
			                         L_STRING_TO_C(candidate.foundation), defaultCandidate);
		}
		if (!iceRestarted) {
			bool losingPairsAdded = false;
			for (int j = 0; j < static_cast<int>(stream.ice_remote_candidates.size()); j++) {
				const auto &remoteCandidate = stream.getIceRemoteCandidateAtIndex(static_cast<size_t>(j));
				std::string addr = std::string();
				int port = 0;
				int componentID = j + 1;
				if (remoteCandidate.addr.empty()) break;
				getIceDefaultAddrAndPort(static_cast<uint16_t>(componentID), md, stream, addr, port);

				// If we receive a re-invite with remote-candidates, supply these pairs to the ice check list.
				// They might be valid pairs already selected, or losing pairs.

				int remoteFamily = AF_INET;
				if (remoteCandidate.addr.find(":") != std::string::npos) remoteFamily = AF_INET6;
				int family = AF_INET;
				if (addr.find(':') != std::string::npos) family = AF_INET6;
				ice_add_losing_pair(cl, static_cast<uint16_t>(j + 1), remoteFamily, L_STRING_TO_C(remoteCandidate.addr),
				                    remoteCandidate.port, family, L_STRING_TO_C(addr), port);
				losingPairsAdded = true;
			}
			if (losingPairsAdded) ice_check_list_check_completed(cl);
		}
	}
}

void IceService::clearUnusedIceCandidates(const std::shared_ptr<SalMediaDescription> &localDesc,
                                          const std::shared_ptr<SalMediaDescription> &remoteDesc,
                                          bool localIsOfferer) {
	for (size_t i = 0; i < MIN(remoteDesc->streams.size(), localDesc->streams.size()); i++) {
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (!cl) continue;
		const auto &localStream = localDesc->streams[i];
		const auto &stream = remoteDesc->streams[i];
		if ((stream.getChosenConfiguration().rtcp_mux && localStream.getChosenConfiguration().rtcp_mux) ||
		    (!localIsOfferer && stream.getChosenConfiguration().rtcp_mux &&
		     !stream.getChosenConfiguration().mid.empty() && localDesc->accept_bundles)) {
			/* RTCP candidates must be dropped under these two ORd conditions:
			 * - rtcp_mux is advertised locally and remotely
			 * - when answering to an offer, when rtcp_mux is advertised together with RTP bundle remotely and we accept
			 * RTP bundle (because rtcp-mux is mandatory with bundles)
			 */
			ice_check_list_remove_rtcp_candidates(cl);
			rtp_session_enable_rtcp_mux(cl->rtp_session, TRUE);
		}
	}
}

void IceService::updateFromRemoteMediaDescription(const std::shared_ptr<SalMediaDescription> &localDesc,
                                                  const std::shared_ptr<SalMediaDescription> &remoteDesc,
                                                  bool isOffer) {
	if (!mIceSession) return;

	if (!iceFoundInMediaDescription(remoteDesc)) {
		// Response from remote does not contain mandatory ICE attributes, delete the session.
		deleteSession();
		return;
	}

	// Check for ICE restart and set remote credentials.
	bool iceRestarted = checkForIceRestartAndSetRemoteCredentials(remoteDesc, isOffer);

	// Create ICE check lists if needed and parse ICE attributes.
	createIceCheckListsAndParseIceAttributes(remoteDesc, iceRestarted);
	size_t i;
	for (i = 0; i < mStreamsGroup.getStreams().size(); i++) {
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (!cl) continue;
		if (i < remoteDesc->streams.size()) {
			const auto &remoteDescStream = remoteDesc->streams[i];
			if (remoteDescStream.enabled() && remoteDescStream.getRtpPort() != 0 &&
			    remoteDescStream.getDirection() != SalStreamInactive) {
				/*
				 * rtp_port == 0 is true when it is a secondary stream part of bundle.
				 */
				/* Stream still needs ICE */
				continue;
			}
		}
		/* This stream is unused or no longer needs ICE, remove its check list */
		ice_session_remove_check_list_from_idx(mIceSession, static_cast<unsigned int>(i));
		auto stream = mStreamsGroup.getStream(i);
		stream->setIceCheckList(nullptr);
		stream->iceStateChanged();
	}
	clearUnusedIceCandidates(localDesc, remoteDesc, !isOffer);
	ice_session_check_mismatch(mIceSession);

	if (ice_session_nb_check_lists(mIceSession) == 0) {
		deleteSession();
	}
}

void IceService::updateLocalMediaDescriptionFromIce(std::shared_ptr<SalMediaDescription> &desc) {
	if (!mIceSession) return;
	IceCandidate *rtpCandidate = nullptr;
	IceCandidate *rtcpCandidate = nullptr;
	bool result = false;
	IceSessionState sessionState = ice_session_state(mIceSession);
	bool usePerStreamUfragPassword = linphone_config_get_bool(linphone_core_get_config(getCCore()), "sip",
	                                                          "ice_password_ufrag_in_media_description", false);

	if (sessionState == IS_Completed) {
		IceCheckList *firstCl = nullptr;
		for (size_t i = 0; i < desc->streams.size(); i++) {
			IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
			if (cl) {
				firstCl = cl;
				break;
			}
		}
		if (firstCl) result = !!ice_check_list_selected_valid_local_candidate(firstCl, &rtpCandidate, nullptr);
		if (result) {
			desc->addr = rtpCandidate->taddr.ip;
		} else {
			lWarning() << "If ICE has completed successfully, rtp_candidate should be set!";
			ice_dump_valid_list(firstCl);
		}
	}

	if (!usePerStreamUfragPassword) {
		desc->ice_pwd = L_C_TO_STRING(ice_session_local_pwd(mIceSession));
		desc->ice_ufrag = L_C_TO_STRING(ice_session_local_ufrag(mIceSession));
	}

	for (size_t i = 0; i < desc->streams.size(); i++) {
		auto &stream = desc->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		rtpCandidate = rtcpCandidate = nullptr;
		if (!stream.enabled() || !cl || (stream.getRtpPort() == 0) || (stream.getDirection() == SalStreamInactive))
			continue;
		if (ice_check_list_state(cl) == ICL_Completed) {
			result = !!ice_check_list_selected_valid_local_candidate(ice_session_check_list(mIceSession, (int)i),
			                                                         &rtpCandidate, &rtcpCandidate);
			if (!result) lError() << "No selected valid local candidate but check list is completed, this is a bug.";
		} else {
			result = !!ice_check_list_default_local_candidate(ice_session_check_list(mIceSession, (int)i),
			                                                  &rtpCandidate, &rtcpCandidate);
			if (result) {
				lInfo() << "RTP default candidate is " << L_C_TO_STRING(rtpCandidate->taddr.ip);
			} else {
				lWarning() << "No RTP default candidate.";
			}
		}
		if (result) {
			stream.rtp_addr = L_C_TO_STRING(rtpCandidate->taddr.ip);
			stream.rtp_port = rtpCandidate->taddr.port;
			if (rtcpCandidate) {
				stream.rtcp_addr = L_C_TO_STRING(rtcpCandidate->taddr.ip);
				stream.rtcp_port = rtcpCandidate->taddr.port;
			}
		} else {
			stream.rtp_addr.clear();
			stream.rtcp_addr.clear();
		}

		if (desc->ice_pwd.compare(ice_check_list_local_pwd(cl)) != 0 || usePerStreamUfragPassword) {
			stream.ice_pwd = L_C_TO_STRING(ice_check_list_local_pwd(cl));
		} else {
			stream.ice_pwd.clear();
		}

		if (desc->ice_ufrag.compare(ice_check_list_local_ufrag(cl)) != 0 || usePerStreamUfragPassword) {
			stream.ice_ufrag = L_C_TO_STRING(ice_check_list_local_ufrag(cl));
		} else {
			stream.ice_ufrag.clear();
		}

		stream.ice_mismatch = ice_check_list_is_mismatch(cl);
		list<IceCandidate *> candidatesToInclude;
		if ((ice_check_list_state(cl) == ICL_Running)) {
			// Include all candidates
			for (bctbx_list_t *elem = cl->local_candidates; elem != nullptr; elem = elem->next) {
				IceCandidate *iceCandidate = static_cast<IceCandidate *>(elem->data);
				candidatesToInclude.push_back(iceCandidate);
			}
		} else if (ice_check_list_state(cl) == ICL_Completed) {
			// Only include the nominated candidates.
			if (rtpCandidate) candidatesToInclude.push_back(rtpCandidate);
			/* In rtcp-mux or bundle mode, the rtcpCandidate returned as the same componentID as the rtpCandidate. It
			 * doesn't need to be included in the offer.*/
			if (rtcpCandidate && (!rtpCandidate || rtcpCandidate->componentID != rtpCandidate->componentID))
				candidatesToInclude.push_back(rtcpCandidate);
		}
		if (!candidatesToInclude.empty()) {
			stream.ice_candidates.clear();
			for (auto iceCandidate : candidatesToInclude) {
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
				if (rtcpCandidate) {
					SalIceRemoteCandidate rtcp_remote_candidate;
					rtcp_remote_candidate.addr = L_C_TO_STRING(rtcpCandidate->taddr.ip);
					rtcp_remote_candidate.port = rtcpCandidate->taddr.port;
					stream.ice_remote_candidates.push_back(rtcp_remote_candidate);
				}
			} else {
				lError() << "IceService: Selected valid remote candidates should be present if the check list is in "
				            "the Completed state. This is a BUG !";
			}
		} else {
			for (auto &ice_remote_candidate : stream.ice_remote_candidates) {
				ice_remote_candidate.addr.clear();
				ice_remote_candidate.port = 0;
			}
		}
	}
}

void IceService::gatheringFinished() {
	if (!mIceSession) return;

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
const struct addrinfo *IceService::getIcePreferredStunServerAddrinfo(const struct addrinfo *ai) {
	// Search for NAT64 addrinfo.
	const struct addrinfo *it = ai;
	for (it = ai; it != nullptr; it = it->ai_next) {
		if (it->ai_family == AF_INET6) {
			struct sockaddr_storage ss;
			socklen_t sslen = sizeof(ss);
			memset(&ss, 0, sizeof(ss));
			bctbx_sockaddr_remove_nat64_mapping(it->ai_addr, (struct sockaddr *)&ss, &sslen);
			if (ss.ss_family == AF_INET) break;
		}
	}
	const struct addrinfo *preferredAi = it;
	if (!preferredAi) {
		// Search for IPv4 addrinfo.
		for (it = ai; it != nullptr; it = it->ai_next) {
			char ip_port[128] = {0};
			bctbx_addrinfo_to_printable_ip_address(it, ip_port, sizeof(ip_port) - 1);
			if (it->ai_family == AF_INET) break;
			if (it->ai_family == AF_INET6) {
				struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)it->ai_addr;
				if (IN6_IS_ADDR_V4MAPPED(&in6->sin6_addr)) {
					break;
				}
			}
		}
		preferredAi = it;
	}
	if (!preferredAi) {
		// Search for IPv6 addrinfo.
		for (it = ai; it != nullptr; it = it->ai_next) {
			if (it->ai_family == AF_INET6) break;
		}
		preferredAi = it;
	}
	return preferredAi;
}

void IceService::finishPrepare() {
	if (!mIceSession) return;
	auto natPolicy = getMediaSessionPrivate().getNatPolicy();
	if (natPolicy) natPolicy->cancelTurnConfigurationUpdate();
	gatheringFinished();
}

void IceService::render(const OfferAnswerContext &ctx, BCTBX_UNUSED(CallSession::State state)) {
	if (!mIceSession) return;

	updateFromRemoteMediaDescription(ctx.localMediaDescription, ctx.remoteMediaDescription, !ctx.localIsOfferer);
	if (mIceSession && ice_session_state(mIceSession) != IS_Completed) {
		ice_session_start_connectivity_checks(mIceSession);
	}

	if (!mIceSession) {
		/* ICE was disabled. */
		mIceWasDisabled = true;
	}
}

void IceService::sessionConfirmed(BCTBX_UNUSED(const OfferAnswerContext &ctx)) {
}

void IceService::stop() {
	// Nothing to do. The ice session can survive.
}

void IceService::finish() {
	deleteSession();
}

void IceService::deleteSession() {
	if (!mIceSession) return;
	if (mAsyncStunResolverHandle != 0) {
		auto natPolicy = getMediaSessionPrivate().getNatPolicy();
		if (natPolicy) natPolicy->cancelAsync(mAsyncStunResolverHandle);
		mAsyncStunResolverHandle = 0;
	}
	/* clear all check lists */
	for (auto &stream : mStreamsGroup.getStreams()) {
		if (stream) {
			stream->setIceCheckList(nullptr);
		}
	}
	ice_session_destroy(mIceSession);
	mIceSession = nullptr;
}

void IceService::setListener(IceServiceListener *listener) {
	mListener = listener;
}

void IceService::restartSession(IceRole role) {
	if (!mIceSession) return;
	/* We use ice_session_reset(), which is similar to ice_session_restart() but it also clears local candidates.
	 * Indeed, the local candidates are always added back after restart.
	 * This avoids previously discovered and possibly non-working peer-reflexive candidates to be accumulated after
	 * successive restarts.
	 */
	ice_session_reset(mIceSession, role);
}

void IceService::resetSession() {
	if (!mIceSession) return;
	ice_session_reset(mIceSession, IR_Controlling);
}

bool IceService::hasCompletedCheckList() const {
	if (!mIceSession) return false;
	switch (ice_session_state(mIceSession)) {
		case IS_Completed:
		case IS_Failed:
			return !!ice_session_has_completed_check_list(mIceSession);
		default:
			return false;
	}
}

void IceService::notifyEndOfPrepare() {
	mStreamsGroup.finishPrepare();
	if (mListener) mListener->onGatheringFinished(*this);
}

void IceService::handleIceEvent(const OrtpEvent *ev) {
	OrtpEventType evt = ortp_event_get_type(ev);
	const OrtpEventData *evd = ortp_event_get_data(const_cast<OrtpEvent *>(ev));
	switch (evt) {
		case ORTP_EVENT_ICE_SESSION_PROCESSING_FINISHED:
			if (hasCompletedCheckList()) {
				if (mListener) mListener->onIceCompleted(*this);
			}
			break;
		case ORTP_EVENT_ICE_GATHERING_FINISHED:
			if (!evd->info.ice_processing_successful) {
				lWarning() << "No STUN answer from [" << getMediaSessionPrivate().getNatPolicy()->getStunServer()
				           << "], continuing without STUN";
			}
			notifyEndOfPrepare();
			break;
		case ORTP_EVENT_ICE_LOSING_PAIRS_COMPLETED:
			if (mListener) mListener->onLosingPairsCompleted(*this);
			break;
		case ORTP_EVENT_ICE_RESTART_NEEDED:
			if (mListener) mListener->onIceRestartNeeded(*this);
			break;
		case ORTP_EVENT_ICE_CHECK_LIST_PROCESSING_FINISHED:
		case ORTP_EVENT_ICE_CHECK_LIST_DEFAULT_CANDIDATE_VERIFIED:
			break;
		default:
			lError() << "IceService::handleIceEvent() is passed with a non-ICE event.";
			break;
	}
	/* Notify all the streams of the ICE state change, so that they can update their stats and so on. */
	for (auto &stream : mStreamsGroup.getStreams()) {
		if (!stream) continue;
		stream->iceStateChanged();
	}
}

bool IceService::isControlling() const {
	if (!mIceSession) return false;
	return ice_session_role(mIceSession) == IR_Controlling;
}

bool IceService::reinviteNeedsDeferedResponse(const std::shared_ptr<SalMediaDescription> &remoteMd) {
	if (!mIceSession || (ice_session_state(mIceSession) != IS_Running)) return false;

	for (size_t i = 0; i < remoteMd->streams.size(); i++) {
		const auto &stream = remoteMd->streams[i];
		IceCheckList *cl = ice_session_check_list(mIceSession, (int)i);
		if (!cl) continue;

		if (stream.getIceMismatch()) {
			return false;
		}
		if ((stream.rtp_port == 0) || (ice_check_list_state(cl) != ICL_Running)) continue;

		for (const auto &ice_remote_candidate : stream.ice_remote_candidates) {
			if (!ice_remote_candidate.addr.empty()) return true;
		}
	}
	return false;
}

bool IceService::hasLocalNetworkPermission() {
	return hasLocalNetworkPermission(IfAddrs::fetchLocalAddresses());
}

bool IceService::checkLocalNetworkPermission(const string &localAddr) {
	ssize_t error;
	struct addrinfo *res = nullptr;
	struct addrinfo hints = {0};
	bctbx_socket_t sock = (ortp_socket_t)-1;
	struct sockaddr_storage selfAddr;
	socklen_t selfAddrLen = sizeof(selfAddr);
	static const int timeout = 200; /*ms*/
	string message("coucou");
	uint64_t begin;
	bool result = false;

	lInfo() << "Checking local network permission with address " << localAddr;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_NUMERICHOST;

	error = bctbx_getaddrinfo(localAddr.c_str(), "0", &hints, &res);
	if (error != 0) {
		lError() << "bctbx_getaddrinfo() failed with error [" << gai_strerror((int)error)
		         << "], unable to check local network permission.";
		goto end;
	}
	sock = bctbx_socket(res->ai_family, res->ai_socktype, IPPROTO_UDP);
	if (sock == (ortp_socket_t)-1) {
		lError() << "Socket creation failed: " << getSocketError();
		goto end;
	}
	bctbx_socket_set_non_blocking(sock);
	error = bctbx_bind(sock, res->ai_addr, (socklen_t)res->ai_addrlen);
	if (error == -1) {
		lError() << "Cannot bind socket:" << getSocketError();
		goto end;
	}
	error = bctbx_getsockname(sock, (struct sockaddr *)&selfAddr, &selfAddrLen);
	if (error == -1) {
		lError() << "getsockname() failed:" << getSocketError();
		goto end;
	}

	begin = ms_get_cur_time_ms();
	do {
		uint8_t buffer[128];
		struct sockaddr_storage ss;
		socklen_t slen = sizeof(ss);

		error = bctbx_sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr *)&selfAddr, selfAddrLen);
		if (error == -1) {
			lError() << "Cannot sendto():" << getSocketError();
			goto end;
		}
		ms_usleep(1000);
		error = bctbx_recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&ss, &slen);
		if (error > 0) {
			result = true;
			break;
		} else if (error == -1 && !(getSocketErrorCode() == BCTBX_EWOULDBLOCK || getSocketErrorCode() == EAGAIN)) {
			lError() << "recvfrom() failed: " << getSocketError();
			break;
		}

	} while (ms_get_cur_time_ms() - begin < timeout);
end:
	if (sock != 1) bctbx_socket_close(sock);
	if (res) bctbx_freeaddrinfo(res);
	return result;
}

/*
 * The local network permission check is done by simply sending a packet to itself.
 */
bool IceService::hasLocalNetworkPermission(const std::list<std::string> &localAddrs) {
	string localAddr4, localAddr6;

	if (localAddrs.empty()) {
		lError() << "Cannot check the local network permission because the local network addresses are unknown.";
		return false;
	}
	/* Select the first IPv4 and IPv6 addresses */
	for (auto addr : localAddrs) {
		if (addr.find(':') == string::npos && localAddr4.empty()) {
			/* not an IPv6 address */
			localAddr4 = addr;
		} else if (addr.find(":") != string::npos && localAddr6.empty()) {
			localAddr6 = addr;
		}
	}
	if (checkLocalNetworkPermission(localAddr4)) {
		lInfo() << "Local network permission is apparently granted (checked with " << localAddr4 << " )";
		return true;
	}
	if (checkLocalNetworkPermission(localAddr6)) {
		lInfo() << "Local network permission is apparently granted (checked with " << localAddr4 << " )";
		return true;
	}
	lInfo() << "Local network permission seems not granted.";
	return false;
}

LINPHONE_END_NAMESPACE
