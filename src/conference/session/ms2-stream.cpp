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

#include <bctoolbox/defs.h>

#include "ms2-streams.h"
#include "media-session.h"
#include "media-session-p.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "conference/participant.h"
#include "utils/payload-type-handler.h"
#include "conference/params/media-session-params-p.h"
#include "nat/ice-service.h"

#include "linphone/core.h"


using namespace::std;

LINPHONE_BEGIN_NAMESPACE


MSBandwidthController *BandwithControllerService::getBandWidthController(){
	return mBandwidthController;
}

void BandwithControllerService::initialize(){
	lInfo() << "StreamsGroup's shared bandwidth controller created.";
	mBandwidthController = ms_bandwidth_controller_new();
}

void BandwithControllerService::destroy(){
	ms_bandwidth_controller_destroy(mBandwidthController);
	mBandwidthController = nullptr;
}


/*
 * MS2Stream implementation
 */

MS2Stream::MS2Stream(StreamsGroup &sg, const OfferAnswerContext &params) : Stream(sg, params){
	memset(&mSessions, 0, sizeof(mSessions));
	initMulticast(params);
	mStats = _linphone_call_stats_new();
	_linphone_call_stats_set_type(mStats, (LinphoneStreamType)getType());
	_linphone_call_stats_set_received_rtcp(mStats, nullptr);
	_linphone_call_stats_set_sent_rtcp(mStats, nullptr);
	_linphone_call_stats_set_ice_state(mStats, LinphoneIceStateNotActivated);
	/* Install the BandwithControllerService, that olds the ms2 MSBandwidthController, which is needed to manage
	 * the audio and the video stream together, when in a conference only. 
	 * For individual calls, the MSBandwidthController of the LinphoneCore is used.
	 * The reasons are both:
	 * - the MSBandwidthController doesn't manage conferences for the moment, only one audio and/or one video stream.
	 * - when running server-side, the bandwidth is considered as unlimited locally. There is hence no way to consider that there could be 
	 * interaction between streams from different connected participants.
	 */
	sg.installSharedService<BandwithControllerService>();
}

void MS2Stream::removeFromBundle(){
	if (mRtpBundle){
		rtp_bundle_remove_session(mRtpBundle, mSessions.rtp_session);
		if (mOwnsBundle){
			RtpBundle *bundle = mRtpBundle;
			getGroup().addPostRenderHook( [bundle](){
				rtp_bundle_delete(bundle);
			});
			mOwnsBundle = false;
			getMediaSessionPrivate().getCurrentParams()->enableRtpBundle(false);
		}
		mRtpBundle = nullptr;
		mBundleOwner = nullptr;
	}
}

void MS2Stream::initRtpBundle(const OfferAnswerContext &params){
	const auto & resultStreamDesc = params.getResultStreamDescription();
	int index = params.resultMediaDescription->getIndexOfTransportOwner(resultStreamDesc);
	if (index == -1) {
		lInfo() << *this << " is not part of any bundle";
		removeFromBundle();
		return ; /*No bundle to handle */
	}
		
	mBundleOwner = dynamic_cast<MS2Stream*>(getGroup().getStream((size_t)index));
	if (!mBundleOwner){
		lError() << "Could not locate the stream owning the bundle's transport.";
		removeFromBundle();
		return;
	}
	RtpBundle * bundle = mBundleOwner->createOrGetRtpBundle(resultStreamDesc);
	if (bundle && mBundleOwner != this && mRtpBundle == nullptr){
		const auto & mid = resultStreamDesc.getChosenConfiguration().getMid();
		lInfo() << "Stream " << *this << " added to rtp bundle " << bundle << " with mid '" << mid << "'";
		rtp_bundle_add_session(bundle, L_STRING_TO_C(mid), mSessions.rtp_session);
		mRtpBundle = bundle;
		mOwnsBundle = false;
		getMediaSessionPrivate().getCurrentParams()->enableRtpBundle(true);
	}

	// It is necessary to call this function after adding the session to the bundle so the SDES contains the MID item
	string userAgent = linphone_core_get_user_agent(getCCore());
	rtp_session_set_source_description(mSessions.rtp_session, getMediaSessionPrivate().getMe()->getAddress().asString().c_str(), NULL, NULL, NULL, NULL, userAgent.c_str(), NULL);
}

RtpBundle *MS2Stream::createOrGetRtpBundle(const SalStreamDescription & sd){
	if (!mRtpBundle){
		mRtpBundle = rtp_bundle_new();
		const auto & mid = sd.getChosenConfiguration().getMid();
		const auto & mid_rtp_ext_header_id = sd.getChosenConfiguration().getMidRtpExtHeaderId();
		lInfo() << "Stream " << *this << " with mid '" << mid << "'is the owner of rtp bundle " << mRtpBundle;
		rtp_bundle_add_session(mRtpBundle, L_STRING_TO_C(mid), mSessions.rtp_session);
		rtp_bundle_set_mid_extension_id(mRtpBundle, mid_rtp_ext_header_id);
		mOwnsBundle = true;
		getMediaSessionPrivate().getCurrentParams()->enableRtpBundle(true);
	}
	return mRtpBundle;
}

void MS2Stream::setIceCheckList(IceCheckList *cl){
	mIceCheckList = cl;
	MediaStream *stream = getMediaStream();
	if (stream){
		rtp_session_set_pktinfo(mSessions.rtp_session, cl != nullptr);
		rtp_session_set_symmetric_rtp(mSessions.rtp_session, (cl == nullptr) ? linphone_core_symmetric_rtp_enabled(getCCore()) : false);
		media_stream_set_ice_check_list(stream, cl);
	}
	if (!cl){
		updateIceInStats();
	}
}

string MS2Stream::getBindIp(){
	string bindIp = linphone_config_get_string(linphone_core_get_config(getCCore()), "rtp", "bind_address", "");
	
	if (!mPortConfig.multicastIp.empty()){
		if (mPortConfig.multicastRole == SalMulticastSender) {
			/* As multicast sender, we must decide a local interface to use to send multicast, and bind to it */
			char multicastBindIp[LINPHONE_IPADDR_SIZE] = {0};
			linphone_core_get_local_ip_for((mPortConfig.multicastIp.find_first_of(':') == string::npos) ? AF_INET : AF_INET6, nullptr, multicastBindIp);
			bindIp = mPortConfig.multicastBindIp = multicastBindIp;
		} else {
			/* Otherwise we shall use an address family of the same family of the multicast address, because
			 * dual stack socket and multicast don't work well on Mac OS (linux is OK, as usual). */
			bindIp = (mPortConfig.multicastIp.find_first_of(':') == string::npos) ? "0.0.0.0" : "::0";
		}
	}else if (bindIp.empty()){
		/*If ipv6 is not enabled, listen to 0.0.0.0. The default behavior of mediastreamer when no IP is passed is to try ::0, and in
		 * case of failure try 0.0.0.0 . But we don't want this if IPv6 is explicitely disabled.*/
		if (!linphone_core_ipv6_enabled(getCCore())){
			bindIp = "0.0.0.0";
		}
	}
	return bindIp;
}

bool MS2Stream::encryptionFound(const SalStreamDescription::tcap_map_t & caps, const LinphoneMediaEncryption encEnum) const {
	const auto & it = std::find_if(caps.cbegin(), caps.cend(), [this, &encEnum] (const auto & cap) {
		return (cap.second.compare(sal_media_proto_to_string(linphone_media_encryption_to_sal_media_proto(encEnum, (this->getMediaSessionPrivate().getParams()->avpfEnabled() ? TRUE : FALSE)))) == 0);
	});
	return (it != caps.end());
}

void MS2Stream::addAcapToStream(std::shared_ptr<SalMediaDescription> & desc, const PotentialCfgGraph::session_description_base_cap::key_type & streamIdx, const std::string & attrName, const std::string & attrValue) {
	const auto & acaps = desc->getAllAcapForStream(streamIdx);
	const auto nameValueMatch = std::find_if(acaps.cbegin(), acaps.cend(), [&attrName, &attrValue] (const auto & cap) {
		const auto & nameValuePair = cap.second;
		const auto & name = nameValuePair.first;
		const auto & value = nameValuePair.second;
		return ((name.compare(attrName) == 0) && (value.compare(attrValue) == 0));
	});
	// Do not add duplicates acaps
	if (nameValueMatch == acaps.cend()) {
		const auto & idx = desc->getFreeAcapIdx();
		lInfo() << "Adding attribute protocol " << attrName << " with value " << attrValue << " to stream " << streamIdx << " at index " << idx;
		desc->addAcapToStream(streamIdx, idx, attrName, attrValue);
	}
}

void MS2Stream::fillLocalMediaDescription(OfferAnswerContext & ctx){
	auto & localDesc = const_cast<SalStreamDescription &>(ctx.getLocalStreamDescription());
	localDesc.rtp_addr = getPublicIp();
	localDesc.rtcp_addr = getPublicIp();
	
	if (localDesc.rtp_port == SAL_STREAM_DESCRIPTION_PORT_TO_BE_DETERMINED && !localDesc.getPayloads().empty()){
		/* Don't fill ports if no codecs are defined. The stream is not valid and should be disabled.*/
		localDesc.rtp_port = mPortConfig.rtpPort;
		localDesc.rtcp_port = mPortConfig.rtcpPort;
	}
	if (!isTransportOwner()){
		/* A secondary stream part of a bundle must set port to zero and add the bundle-only attribute. */
		localDesc.rtp_port = 0;
		localDesc.setBundleOnly(TRUE);
	}
	
	localDesc.cfgs[localDesc.getChosenConfigurationIndex()].rtp_ssrc = rtp_session_get_send_ssrc(mSessions.rtp_session);

	// The negotiated encryption must remain unchanged if:
	// - internal update
	// - pausing a call
	// - resuming a call
	const auto mediaEncryption = (((getMediaSession().getState() == CallSession::State::Resuming) || (getMediaSession().getState() == CallSession::State::Pausing) || getMediaSessionPrivate().getParams()->getPrivate()->getInternalCallUpdate()) ? getMediaSessionPrivate().getNegotiatedMediaEncryption() : getMediaSessionPrivate().getParams()->getMediaEncryption());
	const bool addZrtpAttributes = (mediaEncryption == LinphoneMediaEncryptionZRTP) && ((localDesc.getChosenConfiguration().getProto() == SalProtoRtpAvp) || (localDesc.getChosenConfiguration().getProto() == SalProtoRtpAvpf));
	if (addZrtpAttributes) {
		/* set the hello hash */
		uint8_t enableZrtpHash = false;
		uint8_t zrtphash[128];
		// Initialize ZRTP if not already done
		// This may happen when adding a stream through a reINVITE
		if (!mSessions.zrtp_context) {
			initZrtp();
			// Copy newly created zrtp context into mSessions
			media_stream_reclaim_sessions(getMediaStream(), &mSessions);
		}
		if (mSessions.zrtp_context) {
			ms_zrtp_getHelloHash(mSessions.zrtp_context, zrtphash, sizeof(zrtphash));
			/* Turn on the flag to use it if ZRTP is set */
			enableZrtpHash = addZrtpAttributes;
		}

		localDesc.setZrtpHash(enableZrtpHash, zrtphash);
	}
	if (localDesc.hasDtls()) {
		/* Get the self fingerprint from call (it's computed at stream init) */
		/* If we are offering, SDP will have actpass setup attribute when role is unset, if we are responding the result mediadescription will be set to SalDtlsRoleIsClient */
		localDesc.setDtls(SalDtlsRoleUnset, mDtlsFingerPrint);
	} else {
		localDesc.setDtls(SalDtlsRoleInvalid);
	}
	/* In case we were offered multicast, we become multicast receiver. The local media description must reflect this. */
	localDesc.multicast_role = mPortConfig.multicastRole;

	fillPotentialCfgGraph(ctx);

	Stream::fillLocalMediaDescription(ctx);
}

void MS2Stream::fillPotentialCfgGraph(OfferAnswerContext & ctx){
	auto & localMediaDesc = ctx.localMediaDescription;
	const auto & streamIndex = static_cast<unsigned int>(ctx.streamIndex);
	if (localMediaDesc) {
		const auto & tcaps = localMediaDesc->getAllTcapForStream(streamIndex);

		if (!tcaps.empty()) {
			const auto & stream = localMediaDesc->getStreamIdx(streamIndex);
			const auto & supportedEncs = stream.getSupportedEncryptionsInPotentialCfgs();

			for (const auto & enc : supportedEncs) {

				// Create acaps and cfgs for supported transport protocols using capability negotiation (RFC5939) attributes
				const bool found = encryptionFound(tcaps, enc);

				if (found) {
					if (enc == LinphoneMediaEncryptionDTLS) {
						// acap for DTLS
						const std::string fingerprintAttrName("fingerprint");

						// Create DTLS context if not found
						if (!mSessions.dtls_context) {
							MediaStream *ms = getMediaStream();
							initDtlsParams (ms);
							// Copy newly created dtls context into mSessions
							media_stream_reclaim_sessions(ms, &mSessions);
						}
						addAcapToStream(localMediaDesc, streamIndex, fingerprintAttrName, mDtlsFingerPrint);

						const std::string ssrcAttrName("ssrc");
						const auto rtpSsrc = rtp_session_get_send_ssrc(mSessions.rtp_session);
						const auto rtcpCname = getMediaSessionPrivate().getMe()->getAddress().asString();
						const std::string ssrcAttribute = std::to_string(rtpSsrc) + " cname:" + rtcpCname;
						addAcapToStream(localMediaDesc, streamIndex, ssrcAttrName, ssrcAttribute);

						const std::string setupAttrName("setup");
						/* If we are offering, SDP will have actpass setup attribute when role is unset, if we are responding the result mediadescription will be set to SalDtlsRoleIsClient */
						const std::string setupAttribute = "actpass";
						addAcapToStream(localMediaDesc, streamIndex, setupAttrName, setupAttribute);
					} else if (enc == LinphoneMediaEncryptionZRTP) {
						// acap for ZRTP
						// Create ZRTP context if not found
						if (!mSessions.zrtp_context) {
							MediaStream *ms = getMediaStream();
							Stream *stream = getGroup().lookupMainStream(getType());
							if (getType() == SalVideo) {
#ifdef VIDEO_ENABLED
								MS2VideoStream *msv = dynamic_cast<MS2VideoStream*>(stream);
								msv->initZrtp();
#endif // VIDEO_ENABLED
							} else if (getType() == SalAudio) {
								MS2AudioStream *msa = dynamic_cast<MS2AudioStream*>(stream);
								msa->initZrtp();
							}
							// Copy newly created zrtp context into mSessions
							media_stream_reclaim_sessions(ms, &mSessions);
						}

						if (mSessions.zrtp_context) {
							const std::string attrName("zrtp-hash");
							uint8_t zrtphash[128];
							ms_zrtp_getHelloHash(mSessions.zrtp_context, zrtphash, sizeof(zrtphash));
							addAcapToStream(localMediaDesc, streamIndex, attrName, (const char *)zrtphash);
						} else {
							lInfo() << "Unable to find zrtp session for stream " << streamIndex;
						}
					} else if (enc == LinphoneMediaEncryptionSRTP) {
						// acap for SRTP
						const MSCryptoSuite *suites = linphone_core_get_srtp_crypto_suites_array(getCCore());
						const std::string attrName("crypto");
						// Copy acap crypto attributes
						const auto & acaps = localMediaDesc->getAllAcapForStream(streamIndex);
						const auto & cryptoCap = std::find_if(acaps.cbegin(), acaps.cend(), [&attrName] (const auto & cap) {
							const auto & nameValuePair = cap.second;
							const auto & name = nameValuePair.first;
							return (name.compare(attrName) == 0);
						});

						const auto & actualCfg = stream.getActualConfiguration();
						const auto & actualCfgCrypto = actualCfg.crypto;
						if (!actualCfgCrypto.empty()) {
							// Copy crypto attributes from actual configuration
							for (const auto & c : actualCfgCrypto) {
								MSCryptoSuiteNameParams desc;
								if (ms_crypto_suite_to_name_params(c.algo,&desc)==0){
									const auto attrValue = SalStreamConfiguration::cryptoToSdpValue(c);
									addAcapToStream(localMediaDesc, streamIndex, attrName, attrValue);
								}
							}
						} else if (cryptoCap == acaps.cend()) {
							// If no crypto attribute is found, generate it
							for (size_t j = 0; (suites != nullptr) && (suites[j] != MS_CRYPTO_SUITE_INVALID); j++) {
								SalSrtpCryptoAlgo crypto;
								getMediaSessionPrivate().setupEncryptionKey(crypto, suites[j], static_cast<unsigned int>(j) + 1);
								MSCryptoSuiteNameParams desc;
								if (ms_crypto_suite_to_name_params(crypto.algo,&desc)==0){
									const auto nameValueMatch = std::find_if(acaps.cbegin(), acaps.cend(), [&attrName, &desc] (const auto & cap) {
										const auto & nameValuePair = cap.second;
										const auto & name = nameValuePair.first;
										const auto & value = nameValuePair.second;
										return ((name.compare(attrName) == 0) && (value.find(desc.name) != std::string::npos));
									});
									char attrValue[128];
									if (desc.params) {
										snprintf ( attrValue, sizeof ( attrValue )-1, "%d %s inline:%s %s", crypto.tag, desc.name, crypto.master_key.c_str(),desc.params);
									} else {
										snprintf ( attrValue, sizeof ( attrValue )-1, "%d %s inline:%s", crypto.tag, desc.name, crypto.master_key.c_str() );
									}
									// Do not add duplicates acaps
									if (nameValueMatch == acaps.cend()) {
										addAcapToStream(localMediaDesc, streamIndex, attrName, attrValue);
									}
								} else {
									lError() << "Unable to create parameters for crypto attribute with tag " << crypto.tag << " and master key " << crypto.master_key;
								}
							}
						}
					} else if (enc == LinphoneMediaEncryptionNone) {
						lInfo() << "No acap to add to stream description for encryption " << linphone_media_encryption_to_string(enc);
					}
				}

			}
			localMediaDesc->createPotentialConfigurationsForStream(streamIndex, false, false);
		}
	}
}

void MS2Stream::refreshSockets(){
	rtp_session_refresh_sockets(mSessions.rtp_session);
}

void MS2Stream::initMulticast(const OfferAnswerContext &params) {
	mPortConfig.multicastRole = params.getLocalStreamDescription().multicast_role;
	
	if (mPortConfig.multicastRole == SalMulticastReceiver){
		mPortConfig.multicastIp = params.getRemoteStreamDescription().rtp_addr;
		mPortConfig.rtpPort = params.getRemoteStreamDescription().rtp_port;
		mPortConfig.rtcpPort = 0; /*RTCP deactivated in multicast*/
	}
	
	lInfo() << *this << ": multicast role is ["
		<< sal_multicast_role_to_string(mPortConfig.multicastRole) << "]";
}

void MS2Stream::configureRtpSessionForRtcpFb (const OfferAnswerContext &params) {
	if (getType() != SalAudio && getType() != SalVideo) return; //No AVPF for other than audio/video
	
	const auto & resultStreamDesc = params.getResultStreamDescription();
	rtp_session_enable_avpf_feature(mSessions.rtp_session, ORTP_AVPF_FEATURE_GENERIC_NACK, !!resultStreamDesc.getChosenConfiguration().rtcp_fb.generic_nack_enabled);
	rtp_session_enable_avpf_feature(mSessions.rtp_session, ORTP_AVPF_FEATURE_TMMBR, !!resultStreamDesc.getChosenConfiguration().rtcp_fb.tmmbr_enabled);
}

void MS2Stream::configureRtpSessionForRtcpXr(const OfferAnswerContext &params) {
	OrtpRtcpXrConfiguration currentConfig;
	const OrtpRtcpXrConfiguration *remoteConfig = &params.getRemoteStreamDescription().getChosenConfiguration().rtcp_xr;
	auto & localDesc = params.getLocalStreamDescription();
	if (localDesc.getDirection() == SalStreamInactive)
		return;
	else if (localDesc.getDirection() == SalStreamRecvOnly) {
		/* Use local config for unilateral parameters and remote config for collaborative parameters */
		memcpy(&currentConfig, &localDesc.getChosenConfiguration().rtcp_xr, sizeof(currentConfig));
		currentConfig.rcvr_rtt_mode = remoteConfig->rcvr_rtt_mode;
		currentConfig.rcvr_rtt_max_size = remoteConfig->rcvr_rtt_max_size;
	} else
		memcpy(&currentConfig, remoteConfig, sizeof(currentConfig));
	
	rtp_session_configure_rtcp_xr(mSessions.rtp_session, &currentConfig);
}

void MS2Stream::configureAdaptiveRateControl (const OfferAnswerContext &params) {
	if (getState() == Stream::Running){
		return; // If stream is already running, these things are not expected to change.
	}
	bool videoWillBeUsed = false;
	MediaStream *ms = getMediaStream();
	const auto & vstream = params.resultMediaDescription->findBestStream(SalVideo);
	if ((vstream != Utils::getEmptyConstRefObject<SalStreamDescription>()) && (vstream.getDirection() != SalStreamInactive) && !vstream.getPayloads().empty()) {
		/* When video is used, do not make adaptive rate control on audio, it is stupid */
		videoWillBeUsed = true;
	}
	bool enabled = !!linphone_core_adaptive_rate_control_enabled(getCCore());
	if (!enabled) {
		media_stream_enable_adaptive_bitrate_control(ms, false);
		return;
	}
	bool isAdvanced = true;
	string algo = linphone_core_get_adaptive_rate_algorithm(getCCore());
	if (algo == "basic")
		isAdvanced = false;
	else if (algo == "advanced")
		isAdvanced = true;
	
	if (isAdvanced && !params.getResultStreamDescription().getChosenConfiguration().rtcp_fb.tmmbr_enabled) {
		lWarning() << "Advanced adaptive rate control requested but avpf-tmmbr is not activated in this stream. Reverting to basic rate control instead";
		isAdvanced = false;
	}
	if (isAdvanced) {
		lInfo() << "Setting up advanced rate control";
		if (getMixer() == nullptr){
			// Use the core's bandwidth controller.
			ms_bandwidth_controller_add_stream(getCCore()->bw_controller, ms);
		}else{
			// Use the streamsgroup's shared bandwidth controller.
			ms_bandwidth_controller_add_stream(getGroup().getSharedService<BandwithControllerService>()->getBandWidthController(), ms);
		}
		media_stream_enable_adaptive_bitrate_control(ms, false);
	} else {
		media_stream_set_adaptive_bitrate_algorithm(ms, MSQosAnalyzerAlgorithmSimple);
		if (getType() == SalAudio && videoWillBeUsed) {
			/* If this is an audio stream but video is going to be used, there is no need to perform
			 * basic rate control on the audio stream, just the video stream. */
			enabled = false;
		}
		media_stream_enable_adaptive_bitrate_control(ms, enabled);
	}
}

void MS2Stream::tryEarlyMediaForking(const OfferAnswerContext &ctx){
	RtpSession *session = mSessions.rtp_session;
	const auto & newStream = ctx.getRemoteStreamDescription();
	std::string rtpAddr = (newStream.rtp_addr.empty() == false) ? newStream.rtp_addr : ctx.remoteMediaDescription->addr;
	std::string rtcpAddr = (newStream.rtcp_addr.empty() == false) ? newStream.rtcp_addr : ctx.remoteMediaDescription->addr;
	if (!ms_is_multicast(rtpAddr.c_str())){
		rtp_session_set_symmetric_rtp(session, false); // Disable symmetric RTP when auxiliary destinations are added.
		rtp_session_add_aux_remote_addr_full(session, rtpAddr.c_str(), newStream.rtp_port, rtcpAddr.c_str(), newStream.rtcp_port);
		mUseAuxDestinations = true;
	}
	Stream::tryEarlyMediaForking(ctx);
}

void MS2Stream::finishEarlyMediaForking(){
	if (mUseAuxDestinations){
		rtp_session_set_symmetric_rtp(mSessions.rtp_session, linphone_core_symmetric_rtp_enabled(getCCore()));
		rtp_session_clear_aux_remote_addr(mSessions.rtp_session);
		mUseAuxDestinations = false;
	}
}

/* 
 * This function is used by derived implementations that need to extract the destination of RTP/RTCP streams
 * from the result media description.
 * Indeed, when RTP bundle mode is ON, this information is to be taken in the transport owner stream.
 */
void MS2Stream::getRtpDestination(const OfferAnswerContext &params, RtpAddressInfo *info){
	auto stream = params.getResultStreamDescription();
	if (mRtpBundle && !mOwnsBundle){
		if (!mBundleOwner){
			lError() << "Bundle owner shall be set !";
		}else{
			stream = params.resultMediaDescription->getStreamIdx(static_cast<unsigned int>(mBundleOwner->getIndex()));
		}
	}
	
	info->rtpAddr = stream.rtp_addr.empty() == false ? stream.rtp_addr : params.resultMediaDescription->addr;
	bool isMulticast = !!ms_is_multicast(info->rtpAddr.c_str());
	info->rtpPort = stream.rtp_port;
	info->rtcpAddr = stream.rtcp_addr.empty() == false ? stream.rtcp_addr : info->rtpAddr;
	info->rtcpPort = (linphone_core_rtcp_enabled(getCCore()) && !isMulticast) ? (stream.rtcp_port ? stream.rtcp_port : stream.rtp_port + 1) : 0;
}

/*
 * Handle some basic session changes.
 * Return true everything was handled, false otherwise, in which case the caller will have to restart the stream.
 */
bool MS2Stream::handleBasicChanges(const OfferAnswerContext &params, CallSession::State targetState){
	const auto & stream = params.getResultStreamDescription();
	
	if ((stream == Utils::getEmptyConstRefObject<SalStreamDescription>()) || stream.getDirection() == SalStreamInactive || !stream.enabled()){
		/* In this case all we have to do is to ensure that the stream is stopped. */
		if (getState() != Stopped) stop();
		return true;
	}
	if (getState() == Stream::Running){
		if (getMediaSessionPrivate().getCurrentParams()->getPrivate()->getInConference() != getMediaSessionPrivate().getParams()->getPrivate()->getInConference()){
			lInfo() << "Stream needs to be restarted because of a change in its conference membership attribute.";
			lInfo() << "Will become joined to a conference: " << (getMediaSessionPrivate().getParams()->getPrivate()->getInConference() ? "yes" : "no");
			stop();
			return false;
		}
		int changesToHandle = params.resultStreamDescriptionChanges;
		if (params.resultStreamDescriptionChanges & SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED){
			updateDestinations(params);
			changesToHandle &= ~SAL_MEDIA_DESCRIPTION_NETWORK_CHANGED;
		}
		if (params.resultStreamDescriptionChanges & SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED){
			updateCryptoParameters(params);
			changesToHandle &= ~SAL_MEDIA_DESCRIPTION_CRYPTO_KEYS_CHANGED;
		}
		// SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED monitors the number of streams, it is ignored here.
		changesToHandle &= ~SAL_MEDIA_DESCRIPTION_STREAMS_CHANGED;
		
		if (changesToHandle == 0){
			// We've handled everything.
			if (params.resultStreamDescriptionChanges){
				lInfo() << "Stream updated, no need to restart.";
			}
			return true;
		}
	}else if (getState() == Stream::Stopped){
		/* Already stopped, nothing to do.*/
		return false;
	}
	/* Otherwise these changes shall be handled by a full restart of the stream. */
	stop();
	return false;
}

void MS2Stream::render(const OfferAnswerContext &params, CallSession::State targetState){
	const auto & stream = params.getResultStreamDescription();
	std::string rtpAddr = (stream.rtp_addr.empty() == false) ? stream.rtp_addr : params.resultMediaDescription->addr;
	bool isMulticast = !!ms_is_multicast(rtpAddr.c_str());
	MediaStream *ms = getMediaStream();
	
	if (getIceService().isActive() || (getMediaSessionPrivate().getParams()->earlyMediaSendingEnabled() 
		&& (targetState == CallSession::State::OutgoingEarlyMedia))) {
		rtp_session_set_symmetric_rtp(mSessions.rtp_session, false);
	}

	if (getState() == Stream::Stopped){
		/* These things below are not expected to change while the stream is running. */
		media_stream_set_max_network_bitrate(getMediaStream(), mOutputBandwidth * 1000);
		if (isMulticast)
			rtp_session_set_multicast_ttl(mSessions.rtp_session, stream.getChosenConfiguration().ttl);
		rtp_session_enable_rtcp_mux(mSessions.rtp_session, stream.getChosenConfiguration().rtcp_mux);

		setupSrtp(params);

		ms_media_stream_sessions_set_encryption_mandatory(&ms->sessions, getMediaSessionPrivate().isEncryptionMandatory());
		configureRtpSessionForRtcpFb(params);
		configureRtpSessionForRtcpXr(params);
		configureAdaptiveRateControl(params);
		
		if (stream.getChosenConfiguration().dtls_role != SalDtlsRoleInvalid){ /* If DTLS is available at both end points */
			/* Give the peer certificate fingerprint to dtls context */
			ms_dtls_srtp_set_peer_fingerprint(ms->sessions.dtls_context, L_STRING_TO_C(params.getRemoteStreamDescription().getChosenConfiguration().dtls_fingerprint));
		}
	}
	
	switch(targetState){
		case CallSession::State::IncomingEarlyMedia:
			BCTBX_NO_BREAK;
		case CallSession::State::OutgoingEarlyMedia:
			/* don't accept to send real-live media in early media stage by default.*/
			if (!getMediaSessionPrivate().getParams()->earlyMediaSendingEnabled()) {
				lInfo() << "Early media sending not allowed, will send silence and dummy video instead.";
				mMuted = true;
			}else{
				lInfo() << "Early media sending allowed, will send real live sound and video.";
			}
		break;
		case CallSession::State::StreamsRunning:
			mMuted = false;
			finishEarlyMediaForking();
		break;
		default:
		break;
	}
	startEventHandling();
	initRtpBundle(params);
	setIceCheckList(mIceCheckList); // do it after enabling bundles
	Stream::render(params, targetState);
}


OrtpJitterBufferAlgorithm MS2Stream::jitterBufferNameToAlgo (const string &name) {
	if (name == "basic") return OrtpJitterBufferBasic;
	if (name == "rls") return OrtpJitterBufferRecursiveLeastSquare;
	lError() << "Invalid jitter buffer algorithm: " << name;
	return OrtpJitterBufferRecursiveLeastSquare;
}

void MS2Stream::applyJitterBufferParams (RtpSession *session) {
	LinphoneConfig *config = linphone_core_get_config(getCCore());
	JBParameters params;
	rtp_session_get_jitter_buffer_params(session, &params);
	params.min_size = linphone_config_get_int(config, "rtp", "jitter_buffer_min_size", 40);
	params.max_size = linphone_config_get_int(config, "rtp", "jitter_buffer_max_size", 500);
	params.max_packets = params.max_size * 200 / 1000; /* Allow 200 packet per seconds, quite large */
	const char *algo = linphone_config_get_string(config, "rtp", "jitter_buffer_algorithm", "rls");
	params.buffer_algorithm = jitterBufferNameToAlgo(algo ? algo : "");
	params.refresh_ms = linphone_config_get_int(config, "rtp", "jitter_buffer_refresh_period", 5000);
	params.ramp_refresh_ms = linphone_config_get_int(config, "rtp", "jitter_buffer_ramp_refresh_period", 5000);
	params.ramp_step_ms = linphone_config_get_int(config, "rtp", "jitter_buffer_ramp_step", 20);
	params.ramp_threshold = linphone_config_get_int(config, "rtp", "jitter_buffer_ramp_threshold", 70);

	switch (getType()) {
		case SalAudio:
		case SalText: /* Let's use the same params for text as for audio */
			params.nom_size = linphone_core_get_audio_jittcomp(getCCore());
			params.adaptive = linphone_core_audio_adaptive_jittcomp_enabled(getCCore());
			break;
		case SalVideo:
			params.nom_size = linphone_core_get_video_jittcomp(getCCore());
			params.adaptive = linphone_core_video_adaptive_jittcomp_enabled(getCCore());
			break;
		default:
			lError() << "applyJitterBufferParams(): should not happen";
			break;
	}
	params.enabled = params.nom_size > 0;
	if (params.enabled) {
		if (params.min_size > params.nom_size)
			params.min_size = params.nom_size;
		if (params.max_size < params.nom_size)
			params.max_size = params.nom_size;
	}
	rtp_session_set_jitter_buffer_params(session, &params);
}

void MS2Stream::configureRtpSession(RtpSession *session){
	rtp_session_enable_network_simulation(session, &getCCore()->net_conf.netsim_params);
	applyJitterBufferParams(session);
	string userAgent = linphone_core_get_user_agent(getCCore());
	rtp_session_set_source_description(session, getMediaSessionPrivate().getMe()->getAddress().asString().c_str(), NULL, NULL, NULL, NULL, userAgent.c_str(), NULL);
	rtp_session_set_symmetric_rtp(session, linphone_core_symmetric_rtp_enabled(getCCore()));
	
	if (getType() == SalVideo){
		int videoRecvBufSize = linphone_config_get_int(linphone_core_get_config(getCCore()), "video", "recv_buf_size", 0);
		if (videoRecvBufSize > 0)
			rtp_session_set_recv_buf_size(session, videoRecvBufSize);
	}
}

void MS2Stream::setupDtlsParams (MediaStream *ms) {
	if (getMediaSessionPrivate().getParams()->getMediaEncryption() == LinphoneMediaEncryptionDTLS) {
		initDtlsParams (ms);
	}
}

void MS2Stream::initDtlsParams (MediaStream *ms) {
	if (ms) {
		MSDtlsSrtpParams dtlsParams = { 0 };
		
		/* TODO : search for a certificate with CNAME=sip uri(retrieved from variable me) or default : linphone-dtls-default-identity */
		/* This will parse the directory to find a matching fingerprint or generate it if not found */
		/* returned string must be freed */
		char *certificate = nullptr;
		char *key = nullptr;
		char *fingerprint = nullptr;

		sal_certificates_chain_parse_directory(&certificate, &key, &fingerprint,
			linphone_core_get_user_certificates_path(getCCore()), "linphone-dtls-default-identity", SAL_CERTIFICATE_RAW_FORMAT_PEM, true, true);
		if (fingerprint) {
			if (getMediaSessionPrivate().getDtlsFingerprint().empty()){
				getMediaSessionPrivate().setDtlsFingerprint(fingerprint);
			}
			mDtlsFingerPrint = fingerprint;
			ms_free(fingerprint);
		}
		if (key && certificate) {
			dtlsParams.pem_certificate = certificate;
			dtlsParams.pem_pkey = key;
			dtlsParams.role = MSDtlsSrtpRoleUnset; /* Default is unset, then check if we have a result SalMediaDescription */
			media_stream_enable_dtls(ms, &dtlsParams);
			ms_free(certificate);
			ms_free(key);
		} else {
			/* Check if encryption forced, if yes, stop call */
			if (linphone_core_is_media_encryption_mandatory(getCCore())) {
				lError() << "Unable to retrieve or generate DTLS certificate and key - DTLS disabled, call is going to be terminated";
				LinphoneErrorInfo *ei = linphone_error_info_new();
				linphone_error_info_set_reason(ei, LinphoneReasonNotAcceptable);
				getMediaSession().terminate(ei);
				linphone_error_info_unref(ei);
			} else {
				lError() << "Unable to retrieve or generate DTLS certificate and key - DTLS disabled";
			}
		}
	}
}

void MS2Stream::startDtls(const OfferAnswerContext &params){
	if (mDtlsStarted) {
		lWarning() << "DTLS engine on stream session [" << &mSessions << "] is already started";
		return;
	}
	const auto & resultStreamDesc = params.getResultStreamDescription();
	if (!resultStreamDesc.hasDtls()) return;

	if (resultStreamDesc.getChosenConfiguration().dtls_role == SalDtlsRoleInvalid){
		lWarning() << "Unable to start DTLS engine on stream session [" << &mSessions << "], Dtls role in resulting media description is invalid";
	}else {
		if (!isTransportOwner()){
			/* RTP bundle mode: there must be only one DTLS association per transport. */
			return;
		}
		/* Workaround for buggy openssl versions that send DTLS packets bigger than the MTU. We need to increase the recv buf size of the RtpSession.*/
		int recv_buf_size = linphone_config_get_int(linphone_core_get_config(getCCore()),"rtp", "dtls_recv_buf_size", 5000);
		rtp_session_set_recv_buf_size(mSessions.rtp_session, recv_buf_size);
		
		/* If DTLS is available at both end points */
		/* Give the peer certificate fingerprint to dtls context */
		ms_dtls_srtp_set_peer_fingerprint(mSessions.dtls_context, L_STRING_TO_C(params.getRemoteStreamDescription().getChosenConfiguration().dtls_fingerprint));
		ms_dtls_srtp_set_role(mSessions.dtls_context, (resultStreamDesc.getChosenConfiguration().dtls_role == SalDtlsRoleIsClient) ? MSDtlsSrtpRoleIsClient : MSDtlsSrtpRoleIsServer); /* Set the role to client */
		ms_dtls_srtp_start(mSessions.dtls_context); /* Then start the engine, it will send the DTLS client Hello */
		mDtlsStarted = true;
	}
}

void MS2Stream::initializeSessions(MediaStream *stream){
	if (mPortConfig.multicastRole == SalMulticastReceiver){
		if (!mPortConfig.multicastIp.empty())
			media_stream_join_multicast_group(stream, mPortConfig.multicastIp.c_str());
		else
			lError() << "Cannot join multicast group if multicast ip is not set";
	}
	
	configureRtpSession(stream->sessions.rtp_session);
	setupDtlsParams(stream);
	
	if (mPortConfig.rtpPort == -1){
		// Case where we requested random ports from the system. Now that they are allocated, get them.
		mPortConfig.rtpPort = rtp_session_get_local_port(stream->sessions.rtp_session);
		mPortConfig.rtcpPort = rtp_session_get_local_rtcp_port(stream->sessions.rtp_session);
	}
	int dscp = -1;
	switch(getType()){
		case SalAudio:
			dscp = linphone_core_get_audio_dscp(getCCore());
		break;
		case SalVideo:
			dscp = linphone_core_get_video_dscp(getCCore());
		break;
		default:
		break;
		
	}
	if (dscp != -1)
		media_stream_set_dscp(stream, dscp);
	
	mOrtpEvQueue = ortp_ev_queue_new();
	rtp_session_register_event_queue(stream->sessions.rtp_session, mOrtpEvQueue);
	
	media_stream_reclaim_sessions(stream, &mSessions);
	
}

void MS2Stream::setupSrtp(const OfferAnswerContext &params) {
	const auto & localStreamDesc = params.getLocalStreamDescription();
	const auto & resultStreamDesc = params.getResultStreamDescription();
	MediaStream * ms = getMediaStream();

	if (resultStreamDesc.hasSrtp()){
		// Valid local tags are > 0
		int cryptoIdx = Sal::findCryptoIndexFromTag(localStreamDesc.getChosenConfiguration().crypto, static_cast<unsigned char>(resultStreamDesc.getChosenConfiguration().crypto_local_tag));
		if (cryptoIdx >= 0) {
			MSCryptoSuite algo = resultStreamDesc.getChosenConfiguration().crypto[0].algo;
			ms_media_stream_sessions_set_srtp_send_key_b64(&ms->sessions, algo, L_STRING_TO_C(localStreamDesc.getChosenConfiguration().crypto[(size_t)cryptoIdx].master_key));
			ms_media_stream_sessions_set_srtp_recv_key_b64(&ms->sessions, algo, L_STRING_TO_C(resultStreamDesc.getChosenConfiguration().crypto[0].master_key));
		} else {
			lWarning() << "Failed to find local crypto algo with tag: " << resultStreamDesc.getChosenConfiguration().crypto_local_tag;
		}
	} else if (mSessions.srtp_context && (getMediaSessionPrivate().getNegotiatedMediaEncryption() == LinphoneMediaEncryptionNone)) {
		// If the stream has SRTP disabled, set the key to a NULL value
		MSCryptoSuite algo = MS_CRYPTO_SUITE_INVALID;
		ms_media_stream_sessions_set_srtp_send_key_b64(&ms->sessions, algo, NULL);
		ms_media_stream_sessions_set_srtp_recv_key_b64(&ms->sessions, algo, NULL);
	}
}

void MS2Stream::updateCryptoParameters(const OfferAnswerContext &params) {
	const auto & resultStreamDesc = params.getResultStreamDescription();
	MediaStream * ms = getMediaStream();

	setupSrtp(params);

	if (resultStreamDesc.hasZrtp()) {
		if (!mSessions.zrtp_context) {
			initZrtp();
			// Copy newly created zrtp context into mSessions
			media_stream_reclaim_sessions(ms, &mSessions);
		}
	} else if (mSessions.zrtp_context) {
		media_stream_reset_zrtp_context(ms);
	}

	if (resultStreamDesc.hasDtls()) {
		if (!mSessions.dtls_context) {
			MediaStream *ms = getMediaStream();
			initDtlsParams (ms);
			// Copy newly created dtls context into mSessions
			media_stream_reclaim_sessions(ms, &mSessions);
		}

		startDtls(params);
	} else {
		mDtlsStarted = false;
		if (mSessions.dtls_context) {
			ms_dtls_srtp_reset_context(mSessions.dtls_context);
		}
	}

}

void MS2Stream::updateDestinations(const OfferAnswerContext &params) {
	const auto & resultStreamDesc = params.getResultStreamDescription();
	if (resultStreamDesc.rtp_port == 0 && resultStreamDesc.isBundleOnly()){
		/* we can ignore */
		return;
	}
	
	std::string rtpAddr = (resultStreamDesc.rtp_addr.empty() == false) ? resultStreamDesc.rtp_addr : params.resultMediaDescription->addr;
	std::string rtcpAddr = (resultStreamDesc.rtcp_addr.empty() == false) ? resultStreamDesc.rtcp_addr : params.resultMediaDescription->addr;
	lInfo() << "Change audio stream destination: RTP=" << rtpAddr << ":" << resultStreamDesc.rtp_port << " RTCP=" << rtcpAddr << ":" << resultStreamDesc.rtcp_port;
	rtp_session_set_remote_addr_full(mSessions.rtp_session, rtpAddr.c_str(), resultStreamDesc.rtp_port, rtcpAddr.c_str(), resultStreamDesc.rtcp_port);
}

void MS2Stream::startEventHandling(){
	if (mTimer) return;
	mTimer = getCore().createTimer([this](){
			handleEvents();
			return true;
		}, sEventPollIntervalMs, "Stream event processing timer");
}

void MS2Stream::stopEventHandling(){
	if (mTimer){
		getCore().destroyTimer(mTimer);
		mTimer = nullptr;
	}
}

bool MS2Stream::prepare(){
	if (getCCore()->rtptf) {
		RtpTransport *meta_rtp;
		RtpTransport *meta_rtcp;
		rtp_session_get_transports(mSessions.rtp_session, &meta_rtp, &meta_rtcp);
		LinphoneCoreRtpTransportFactoryFunc rtpFunc = nullptr, rtcpFunc = nullptr;
		void *rtpFuncData = nullptr, *rtcpFuncData = nullptr;
		
		switch(getType()){
			case SalAudio:
				rtpFunc = getCCore()->rtptf->audio_rtp_func;
				rtpFuncData = getCCore()->rtptf->audio_rtp_func_data;
				rtcpFunc = getCCore()->rtptf->audio_rtcp_func;
				rtcpFuncData = getCCore()->rtptf->audio_rtcp_func_data;
			break;
			case SalVideo:
				rtpFunc = getCCore()->rtptf->video_rtp_func;
				rtpFuncData = getCCore()->rtptf->video_rtp_func_data;
				rtcpFunc = getCCore()->rtptf->video_rtcp_func;
				rtcpFuncData = getCCore()->rtptf->video_rtcp_func_data;
			break;
			case SalText:
			break;
			case SalOther:
			break;
		}
		
		if (!meta_rtp_transport_get_endpoint(meta_rtp)) {
			lInfo() << this << " using custom RTP transport endpoint";
			meta_rtp_transport_set_endpoint(meta_rtp, rtpFunc(rtpFuncData, mPortConfig.rtpPort));
		}
		if (!meta_rtp_transport_get_endpoint(meta_rtcp))
			meta_rtp_transport_set_endpoint(meta_rtcp, rtcpFunc(rtcpFuncData, mPortConfig.rtcpPort));
	}
	setIceCheckList(mIceCheckList);
	startEventHandling();
	Stream::prepare();
	return false;
}

void MS2Stream::finishPrepare(){
	Stream::finishPrepare();
	stopEventHandling();
}

int MS2Stream::getIdealAudioBandwidth (const std::shared_ptr<SalMediaDescription> & md, const SalStreamDescription & desc) {
	int remoteBandwidth = 0;
	if (desc.bandwidth > 0)
		remoteBandwidth = desc.bandwidth;
	else if (md->bandwidth > 0) {
		/* Case where b=AS is given globally, not per stream */
		remoteBandwidth = md->bandwidth;
	}
	int uploadBandwidth = 0;
	bool forced = false;
	if (getMediaSessionPrivate().getParams()->getPrivate()->getUpBandwidth() > 0) {
		forced = true;
		uploadBandwidth = getMediaSessionPrivate().getParams()->getPrivate()->getUpBandwidth();
	} else
		uploadBandwidth = linphone_core_get_upload_bandwidth(getCCore());
	uploadBandwidth = PayloadTypeHandler::getMinBandwidth(uploadBandwidth, remoteBandwidth);
	if ((md->nbActiveStreamsOfType(SalVideo) == 0) || forced)
		return uploadBandwidth;
	
	/*
	 * This a default heuristic to choose a target upload bandwidth for an audio stream, the
	 * remaining can then be allocated for video.
	 */
	if (PayloadTypeHandler::bandwidthIsGreater(uploadBandwidth, 512))
		uploadBandwidth = 100;
	else if (PayloadTypeHandler::bandwidthIsGreater(uploadBandwidth, 256))
		uploadBandwidth = 64;
	else if (PayloadTypeHandler::bandwidthIsGreater(uploadBandwidth, 128))
		uploadBandwidth = 40;
	else if (PayloadTypeHandler::bandwidthIsGreater(uploadBandwidth, 0))
		uploadBandwidth = 24;
	return uploadBandwidth;
}

RtpProfile * MS2Stream::makeProfile(const std::shared_ptr<SalMediaDescription> & md, const SalStreamDescription & desc, int *usedPt) {
	if (mRtpProfile){
		rtp_profile_destroy(mRtpProfile);
		mRtpProfile = nullptr;
	}
	*usedPt = -1;
	int bandwidth = 0;
	if (desc.type == SalAudio)
		bandwidth = getIdealAudioBandwidth(md, desc);
	else if (desc.type == SalVideo)
		bandwidth = getGroup().getVideoBandwidth(md, desc);

	bool first = true;
	RtpProfile *profile = rtp_profile_new("Call profile");
	for (const auto & pt : desc.getPayloads()) {
		/* Make a copy of the payload type, so that we left the ones from the SalStreamDescription unchanged.
		 * If the SalStreamDescription is freed, this will have no impact on the running streams. */
		auto clonedPt = payload_type_clone(pt);
		int upPtime = 0;
		if ((clonedPt->flags & PAYLOAD_TYPE_FLAG_CAN_SEND) && first) {
			/* First codec in list is the selected one */
			if (desc.type == SalAudio) {
				bandwidth = getGroup().updateAllocatedAudioBandwidth(clonedPt, bandwidth);
				upPtime = getMediaSessionPrivate().getParams()->getPrivate()->getUpPtime();
				if (!upPtime)
					upPtime = linphone_core_get_upload_ptime(getCCore());
			}
			first = false;
		}
		if (*usedPt == -1) {
			/* Don't select telephone-event as a payload type */
			if (strcasecmp(clonedPt->mime_type, "telephone-event") != 0)
				*usedPt = payload_type_get_number(clonedPt);
		}
		if (clonedPt->flags & PAYLOAD_TYPE_BITRATE_OVERRIDE) {
			lInfo() << "Payload type [" << clonedPt->mime_type << "/" << clonedPt->clock_rate << "] has explicit bitrate [" << (clonedPt->normal_bitrate / 1000) << "] kbit/s";
			clonedPt->normal_bitrate = PayloadTypeHandler::getMinBandwidth(clonedPt->normal_bitrate, bandwidth * 1000);
		} else
			clonedPt->normal_bitrate = bandwidth * 1000;
		if (desc.getChosenConfiguration().maxptime > 0) {// follow the same schema for maxptime as for ptime. (I.E add it to fmtp)
			ostringstream os;
			os << "maxptime=" << desc.getChosenConfiguration().maxptime;
			payload_type_append_send_fmtp(clonedPt, os.str().c_str());
		}
		if (desc.getChosenConfiguration().ptime > 0)
			upPtime = desc.getChosenConfiguration().ptime;
		if (upPtime > 0) {
			ostringstream os;
			os << "ptime=" << upPtime;
			payload_type_append_send_fmtp(clonedPt, os.str().c_str());
		}
		int number = payload_type_get_number(clonedPt);
		if (rtp_profile_get_payload(profile, number))
			lWarning() << "A payload type with number " << number << " already exists in profile!";
		else
			rtp_profile_set_payload(profile, number, clonedPt);
	}
	mRtpProfile = profile;
	mOutputBandwidth = bandwidth;
	return profile;
}


void MS2Stream::updateStats(){
	if (mSessions.rtp_session) {
		const rtp_stats_t *rtpStats = rtp_session_get_stats(mSessions.rtp_session);
		if (rtpStats)
			_linphone_call_stats_set_rtp_stats(mStats, rtpStats);
	}
	float quality = media_stream_get_average_quality_rating(getMediaStream());
	LinphoneCallLog *log = getMediaSession().getLog();
	if (quality >= 0) {
		if (log->quality == -1.0)
			log->quality = quality;
		else
			log->quality *= quality / 5.0f;
	}
}

LinphoneCallStats *MS2Stream::getStats(){
	MediaStream *ms = getMediaStream();
	if (ms) linphone_call_stats_update(mStats, ms);
	return mStats;
}

void MS2Stream::stop(){
	CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
	
	if (listener){
		int statsType = -1;
		switch(getType()){
			case SalAudio: statsType = LINPHONE_CALL_STATS_AUDIO; break;
			case SalVideo: statsType = LINPHONE_CALL_STATS_VIDEO; break;
			case SalText: statsType = LINPHONE_CALL_STATS_TEXT; break;
			default:
				break;
			
		}
		
		if (statsType != -1) listener->onUpdateMediaInfoForReporting(getMediaSession().getSharedFromThis(), statsType);
		
	}
	if (getMixer() == nullptr){
		ms_bandwidth_controller_remove_stream(getCCore()->bw_controller, getMediaStream());
	}else{
		ms_bandwidth_controller_remove_stream(getGroup().getSharedService<BandwithControllerService>()->getBandWidthController(), getMediaStream());
	}
	updateStats();
	handleEvents();
	stopEventHandling();
	media_stream_reclaim_sessions(getMediaStream(), &mSessions);
	rtp_session_set_profile(mSessions.rtp_session, &av_profile);
	Stream::stop();
	
	/* At this time the derived class hasn't yet stopped it streams.
	 * the RTP Profile objects can't be destroyed until the stream is completely stopped.
	 * As a result we do it later*/
	RtpProfile *rtpProfile = mRtpProfile;
	RtpProfile *rtpIoProfile = mRtpIoProfile;
	getCore().doLater( [rtpProfile, rtpIoProfile](){
		if (rtpProfile) rtp_profile_destroy(rtpProfile);
		if (rtpIoProfile) rtp_profile_destroy(rtpIoProfile);
	});
	mRtpProfile = nullptr;
	mRtpIoProfile = nullptr;
}

void MS2Stream::notifyStatsUpdated () {
	CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
	if (_linphone_call_stats_get_updated(mStats)) {
		switch (_linphone_call_stats_get_updated(mStats)) {
			case LINPHONE_CALL_STATS_RECEIVED_RTCP_UPDATE:
			case LINPHONE_CALL_STATS_SENT_RTCP_UPDATE:
				if (listener) {
					listener->onRtcpUpdateForReporting(getMediaSession().getSharedFromThis(), getType());
				}
				break;
			default:
				break;
		}
		if (listener)
			listener->onStatsUpdated(getMediaSession().getSharedFromThis(), mStats);
		_linphone_call_stats_set_updated(mStats, 0);
	}
}

void MS2Stream::iceStateChanged(){
	updateIceInStats();
}

void MS2Stream::updateIceInStats(LinphoneIceState state){
	lInfo() << "ICE state is " << linphone_ice_state_to_string(state) << " for " << *this;
	_linphone_call_stats_set_ice_state(mStats, state);
}

void MS2Stream::updateIceInStats(){
	/* Special case for rtp bundle: we report the ice state of the transport owner. */
	if (mRtpBundle && !mOwnsBundle && mBundleOwner && mBundleOwner->mStats){
		updateIceInStats(linphone_call_stats_get_ice_state(mBundleOwner->mStats));
		return;
	}
	
	if (!mIceCheckList){
		updateIceInStats(LinphoneIceStateNotActivated);
		return;
	}
	if (ice_check_list_state(mIceCheckList) == ICL_Failed) {
		updateIceInStats(LinphoneIceStateFailed);
		return;
	}
	if (ice_check_list_state(mIceCheckList) == ICL_Running) {
		updateIceInStats(LinphoneIceStateInProgress);
		return;
	}
	/* Otherwise we are in ICL_Completed state. */

	switch (ice_check_list_selected_valid_candidate_type(mIceCheckList)) {
		case ICT_HostCandidate:
			updateIceInStats(LinphoneIceStateHostConnection);
			break;
		case ICT_ServerReflexiveCandidate:
		case ICT_PeerReflexiveCandidate:
			updateIceInStats(LinphoneIceStateReflexiveConnection);
			break;
		case ICT_RelayedCandidate:
			updateIceInStats(LinphoneIceStateRelayConnection);
			break;
		case ICT_CandidateInvalid:
		case ICT_CandidateTypeMax:
			// Shall not happen.
			L_ASSERT(false);
			break;
	}
}

void MS2Stream::dtlsEncryptionChanged(){
	getGroup().propagateEncryptionChanged();
}

void MS2Stream::handleEvents () {
	MediaStream *ms = getMediaStream(), *newMs = ms;
	if (ms) {
		switch(ms->type){
			case MSAudio:
				audio_stream_iterate((AudioStream *)ms);
				break;
			case MSVideo:
#ifdef VIDEO_ENABLED
				video_stream_iterate((VideoStream *)ms);
#endif
				break;
			case MSText:
				text_stream_iterate((TextStream *)ms);
				break;
			default:
				lError() << "handleStreamEvents(): unsupported stream type";
				return;
		}
	}
	OrtpEvent *ev;
	
	while ((ev = ortp_ev_queue_get(mOrtpEvQueue)) != nullptr) {
		OrtpEventType evt = ortp_event_get_type(ev);
		OrtpEventData *evd = ortp_event_get_data(ev);

		/*This MUST be done before any call to "linphone_call_stats_fill" since it has ownership over evd->packet*/
		if (evt == ORTP_EVENT_RTCP_PACKET_RECEIVED) {
			do {
				if (evd->packet && rtcp_is_RTPFB(evd->packet)) {
					if (rtcp_RTPFB_get_type(evd->packet) == RTCP_RTPFB_TMMBR) {
						CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
						listener->onTmmbrReceived(getMediaSession().getSharedFromThis(), (int)getIndex(), (int)rtcp_RTPFB_tmmbr_get_max_bitrate(evd->packet));
					}
				}
			} while (rtcp_next_packet(evd->packet));
			rtcp_rewind(evd->packet);
		}
		if (ms)
			linphone_call_stats_fill(mStats, ms, ev);
		switch(evt){
			case ORTP_EVENT_ZRTP_ENCRYPTION_CHANGED:
				if (getType() != SalAudio || !isMain()){
					getGroup().propagateEncryptionChanged();
				}
			break;
			case ORTP_EVENT_DTLS_ENCRYPTION_CHANGED:
				dtlsEncryptionChanged();
			break;
			case ORTP_EVENT_ICE_SESSION_PROCESSING_FINISHED:
			case ORTP_EVENT_ICE_GATHERING_FINISHED:
			case ORTP_EVENT_ICE_LOSING_PAIRS_COMPLETED:
			case ORTP_EVENT_ICE_RESTART_NEEDED:
				/* ICE events are notified directly to the IceService. */
				getIceService().handleIceEvent(ev);
				newMs = getMediaStream();// Ice can change the stream and free the old one. Ensure to have an existing stream.
			break;
		}
		notifyStatsUpdated();
	
		/* Let subclass handle the event.*/
		handleEvent(ev);
		ortp_event_destroy(ev);
		if( newMs != ms){// Stream have been changed. Rehandle events with this one
			handleEvents();
			return;
		}
	}
}

bool MS2Stream::isEncrypted() const{
	if (!isTransportOwner()){
		if (mBundleOwner){
			return mBundleOwner->isEncrypted(); /* We must refer to the stream that owns the Rtp bundle.*/
		}else{
			lError() << "MS2Stream::isEncrypted(): no bundle owner !";
		}
		return false;
	}
	return media_stream_secured(getMediaStream());
}

bool MS2Stream::isMuted()const{
	return mMuted;
}

RtpSession* MS2Stream::createRtpIoSession() {
	LinphoneConfig *config = linphone_core_get_config(getCCore());
	const char *config_section = getType() == SalAudio ? "sound" : "video";
	const char *rtpmap = linphone_config_get_string(config, config_section, "rtp_map", getType() == SalAudio ? "pcmu/8000/1" : "vp8/90000");
	OrtpPayloadType *pt = rtp_profile_get_payload_from_rtpmap(mRtpProfile, rtpmap);
	if (!pt)
		return nullptr;
	string profileName = string("RTP IO ") + string(config_section) + string(" profile");
	mRtpIoProfile = rtp_profile_new(profileName.c_str());
	int ptnum = linphone_config_get_int(config, config_section, "rtp_ptnum", 0);
	rtp_profile_set_payload(mRtpIoProfile, ptnum, payload_type_clone(pt));
	const char *localIp = linphone_config_get_string(config, config_section, "rtp_local_addr", "127.0.0.1");
	int localPort = linphone_config_get_int(config, config_section, "rtp_local_port", 17076);
	RtpSession *rtpSession = ms_create_duplex_rtp_session(localIp, localPort, -1, ms_factory_get_mtu(getCCore()->factory));
	rtp_session_set_profile(rtpSession, mRtpIoProfile);
	const char *remoteIp = linphone_config_get_string(config, config_section, "rtp_remote_addr", "127.0.0.1");
	int remotePort = linphone_config_get_int(config, config_section, "rtp_remote_port", 17078);
	rtp_session_set_remote_addr_and_port(rtpSession, remoteIp, remotePort, -1);
	rtp_session_enable_rtcp(rtpSession, false);
	rtp_session_set_payload_type(rtpSession, ptnum);
	int jittcomp = linphone_config_get_int(config, config_section, "rtp_jittcomp", 0); /* 0 means no jitter buffer */
	rtp_session_set_jitter_compensation(rtpSession, jittcomp);
	rtp_session_enable_jitter_buffer(rtpSession, (jittcomp > 0));
	bool symmetric = !!linphone_config_get_int(config, config_section, "rtp_symmetric", 0);
	rtp_session_set_symmetric_rtp(rtpSession, symmetric);
	return rtpSession;
}

std::pair<RtpTransport*, RtpTransport*> MS2Stream::getMetaRtpTransports(){
	RtpTransport *metaRtp = nullptr;
	RtpTransport *metaRtcp = nullptr;
	rtp_session_get_transports(mSessions.rtp_session, &metaRtp, &metaRtcp);
	return make_pair(metaRtp, metaRtcp);
}

MSZrtpContext *MS2Stream::getZrtpContext()const{
	return mSessions.zrtp_context;
}

float MS2Stream::getAverageQuality(){
	MediaStream *ms = getMediaStream();
	if (!ms) {
		lError() << "MS2Stream::getAverageQuality(): no stream.";
		return 0.0;
	}
	return media_stream_get_average_quality_rating(ms);
}

float MS2Stream::getCurrentQuality(){
	MediaStream *ms = getMediaStream();
	if (!ms) {
		lError() << "MS2Stream::getCurrentQuality(): no stream.";
		return 0.0;
	}
	return media_stream_get_quality_rating(getMediaStream());
}

void MS2Stream::updateBandwidthReports(){
	MediaStream * ms = getMediaStream();
	bool active = ms ? (media_stream_get_state(ms) == MSStreamStarted) : false;
	_linphone_call_stats_set_download_bandwidth(mStats, active ? (float)(media_stream_get_down_bw(ms) * 1e-3) : 0.f);
	_linphone_call_stats_set_upload_bandwidth(mStats, active ? (float)(media_stream_get_up_bw(ms) * 1e-3) : 0.f);
	_linphone_call_stats_set_rtcp_download_bandwidth(mStats, active ? (float)(media_stream_get_rtcp_down_bw(ms) * 1e-3) : 0.f);
	_linphone_call_stats_set_rtcp_upload_bandwidth(mStats, active ? (float)(media_stream_get_rtcp_up_bw(ms) * 1e-3) : 0.f);
	_linphone_call_stats_set_ip_family_of_remote(mStats,
		active ? (ortp_stream_is_ipv6(&mSessions.rtp_session->rtp.gs) ? LinphoneAddressFamilyInet6 : LinphoneAddressFamilyInet) : LinphoneAddressFamilyUnspec);

	if (getCCore()->send_call_stats_periodical_updates) {
		CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
		if (active)
			linphone_call_stats_update(mStats, ms);
		_linphone_call_stats_set_updated(mStats, _linphone_call_stats_get_updated(mStats) | LINPHONE_CALL_STATS_PERIODICAL_UPDATE);
		if (listener)
			listener->onStatsUpdated(getMediaSession().getSharedFromThis(), mStats);
		_linphone_call_stats_set_updated(mStats, 0);
	}
}

float MS2Stream::getCpuUsage()const{
	MediaStream *ms = getMediaStream();
	if (ms->sessions.ticker == nullptr) return 0.0f;
	return ms_ticker_get_average_load(ms->sessions.ticker);
}

void MS2Stream::finish(){
	if (mRtpBundle && mOwnsBundle){
		rtp_bundle_delete(mRtpBundle);
		mRtpBundle = nullptr;
	}
	if (mOrtpEvQueue){
		rtp_session_unregister_event_queue(mSessions.rtp_session, mOrtpEvQueue);
		ortp_ev_queue_flush(mOrtpEvQueue);
		ortp_ev_queue_destroy(mOrtpEvQueue);
		mOrtpEvQueue = nullptr;
	}
	ms_media_stream_sessions_uninit(&mSessions);
	Stream::finish();
}

bool MS2Stream::avpfEnabled() const{
	return media_stream_avpf_enabled(getMediaStream());
}

bool MS2Stream::bundleEnabled() const{
	return mRtpBundle != nullptr;
}

bool MS2Stream::isTransportOwner() const{
	bool ret = mRtpBundle == nullptr || mOwnsBundle;
	return ret;
}

int MS2Stream::getAvpfRrInterval()const{
	MediaStream *ms = getMediaStream();
	return ms && media_stream_get_state(ms) == MSStreamStarted ? media_stream_get_avpf_rr_interval(ms) : 0;
}

void MS2Stream::connectToMixer(StreamMixer *mixer){
	bool wasRunning = getState() == Running;
	if (wasRunning) stop();
	Stream::connectToMixer(mixer);
	if (wasRunning){
		render(getGroup().getCurrentOfferAnswerContext().scopeStreamToIndex(getIndex()),
			getGroup().getCurrentSessionState());
	}
}

void MS2Stream::disconnectFromMixer(){
	bool wasRunning = false;
	if (getState() == Running){
		stop();
		wasRunning = true;
	}
	Stream::disconnectFromMixer();
	if (wasRunning){
		// Call render to take changes into account immediately.
		render(getGroup().getCurrentOfferAnswerContext().scopeStreamToIndex(getIndex()),
			getGroup().getCurrentSessionState());
	}
}

MS2Stream::~MS2Stream(){
	finish();
	linphone_call_stats_unref(mStats);
	mStats = nullptr;
	
}

LINPHONE_END_NAMESPACE
