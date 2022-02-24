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

#include "streams.h"
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
#include "mixers.h"
#include "mediastreamer2/msanalysedisplay.h"
#include "mediastreamer2/mediastream.h"

#include <iomanip>

using namespace::std;

LINPHONE_BEGIN_NAMESPACE



/*
 * StreamsGroup implementation
 */

StreamsGroup::StreamsGroup(MediaSession &session) : mMediaSession(session){
	mIceService.reset(new IceService(*this));
}

StreamsGroup::~StreamsGroup(){
	finish();
}

IceService & StreamsGroup::getIceService()const{
	return *mIceService;
}

Stream * StreamsGroup::createStream(const OfferAnswerContext &params){
	Stream *ret = nullptr;
	const auto & payloads = params.getLocalStreamDescription().getPayloads();
	if (!params.getLocalStreamDescription().enabled()) {
		lInfo() << "Disabled stream at index " << params.streamIndex;
		return nullptr;
	}
	
	SalStreamType type = params.getLocalStreamDescription().type;
	// Do not create video stream if no payload is in local media description
	if (!payloads.empty()) {
		switch(type){
			case SalAudio:
				ret = new MS2AudioStream(*this, params);
			break;
			case SalVideo:
		#ifdef VIDEO_ENABLED
				ret = new MS2VideoStream(*this, params);
		#endif
			break;
			case SalText:
				ret = new MS2RTTStream(*this, params);
			break;
			case SalOther:
			break;
		}
	}
	if (!ret){
		lError() << "Could not create Stream of type " << sal_stream_type_to_string(type);
		return nullptr;
	}
	lInfo() << "Created stream of type " << sal_stream_type_to_string(type) << " at index " << params.streamIndex << ": " << *ret;
	
	if ((decltype(mStreams)::size_type)params.streamIndex >= mStreams.size()) mStreams.resize(params.streamIndex + 1);
	if (mStreams[params.streamIndex] != nullptr){
		lInfo() << "Stream at index " << params.streamIndex << " is being replaced.";
	}
	mStreams[params.streamIndex].reset(ret);
	/* Attach a mixer to this new stream, if any. */
	attachMixers();
	return ret;
}

void StreamsGroup::fillLocalMediaDescription(OfferAnswerContext & params){
	for (auto &stream : mStreams){
		if (!stream) continue;
		params.scopeStreamToIndex(stream->getIndex());
		stream->fillLocalMediaDescription(params);
	}
	mIceService->fillLocalMediaDescription(params);
}

void StreamsGroup::createStreams(const OfferAnswerContext &params){
	size_t index;
	const auto & localMd = params.localMediaDescription;
	for(index = 0; index < localMd->streams.size(); ++index){
		Stream *s = nullptr;
		params.scopeStreamToIndexWithDiff(index, mCurrentOfferAnswerState);
		
		if (params.localStreamDescriptionChanges) {
			const std::string differences = SalMediaDescription::printDifferences(params.localStreamDescriptionChanges);
			lInfo() << "Local stream description has changed: " << differences;
		}
		if (index >= mStreams.size() || ((s = mStreams[index].get()) == nullptr)){
			s = createStream(params);
		} else if (s) {
			if (s->getType() != params.getLocalStreamDescription().type){
				lError() << "Inconsistency detected while creating streams. Type has changed from " <<
					sal_stream_type_to_string(s->getType()) << " to " << 
					sal_stream_type_to_string(params.getLocalStreamDescription().type) << "!";
			}else if (params.localStreamDescriptionChanges & SAL_MEDIA_DESCRIPTION_NETWORK_XXXCAST_CHANGED ){
				/*
				* Special case: due to implementation constraint, it is necessary to instanciate a new Stream when changing 
				* the cast (uni or multi).
				*/
				s->stop();
				s = createStream(params);
			}
		}
	}
	mIceService->createStreams(params);
}

bool StreamsGroup::prepare(){
	if (mFinished){
		lError() << "StreamsGroup finished, cannot be used anymore.";
		return false;
	}
	for (auto &stream : mStreams){
		if (!stream) continue;
		if (stream->getState() == Stream::Stopped){
			stream->prepare();
		}
	}
	return mIceService->prepare();
}

void StreamsGroup::finishPrepare(){
	for (auto &stream : mStreams){
		if (!stream) continue;
		if (stream->getState() == Stream::Preparing){
			stream->finishPrepare();
		}
	}
	mIceService->finishPrepare();
}

void StreamsGroup::render(const OfferAnswerContext &constParams, CallSession::State targetState){
	if (mFinished){
		lError() << "StreamsGroup finished, cannot be used anymore.";
		return;
	}
	OfferAnswerContext params;
	params.copyFrom(constParams);
	
	if (params.remoteMediaDescription == nullptr){
		/* This can happen when we receive a 200Ok without SDP, after early media. In this case we use the previously
		 * provided remote media description.*/
		params.remoteMediaDescription = mCurrentOfferAnswerState.remoteMediaDescription;
	}
	
	for(auto &stream : mStreams){
		if (!stream) continue;
		Stream *streamPtr = stream.get();
		lInfo() << "StreamsGroup " << this << " rendering " << *stream;
		params.scopeStreamToIndexWithDiff(stream->getIndex(), mCurrentOfferAnswerState);

		if (params.localStreamDescriptionChanges) {
			const std::string differences = SalMediaDescription::printDifferences(params.localStreamDescriptionChanges);
			lInfo() << "Local stream description has changed: " << differences;
		}
		if (params.resultStreamDescriptionChanges) {
			const std::string differences = SalMediaDescription::printDifferences(params.resultStreamDescriptionChanges);
			lInfo() << "Result stream description has changed: " << differences;
		}
		if (streamPtr->getState() == Stream::Preparing)
			streamPtr->finishPrepare();

		streamPtr->render(params, targetState);
	}
	if (!mBandwidthReportTimer){
		mBandwidthReportTimer = getCore().createTimer([this](){ this->computeAndReportBandwidth(); return true; }, 1000 , "StreamsGroup timer");
	}
	
	for(auto &hook : mPostRenderHooks){
		hook();
	}
	mPostRenderHooks.clear();
	
	mIceService->render(params, targetState);
	const char *mode = linphone_config_get_string(linphone_core_get_config(getCore().getCCore()), "rtp", "dtls_srtp_start", "ice");
	if (strcmp(mode, "immediate") == 0 || getIceService().hasCompleted()){
		/* Should not start dtls until ice is completed */
		startDtls(params);
	}
	/* Save the state of the offer-answer, so that we are later able to monitor differences in next render() calls. */
	mCurrentOfferAnswerState.dupFrom(params);
	mCurrentSessionState = targetState;
}

void StreamsGroup::sessionConfirmed(const OfferAnswerContext &params){
	for (auto &stream  : mStreams){
		if (!stream) continue;
		mCurrentOfferAnswerState.scopeStreamToIndex(stream->getIndex());
		stream->sessionConfirmed(mCurrentOfferAnswerState);
	}
}

void StreamsGroup::stop(){
	if (mFinished){
		lError() << "StreamsGroup finished, cannot be used anymore.";
		abort();
		return;
	}

	if (mBandwidthReportTimer){
		getCore().destroyTimer(mBandwidthReportTimer);
		mBandwidthReportTimer = nullptr;
	}
	for(auto &stream : mStreams){
		if (stream && stream->getState() != Stream::Stopped)
			stream->stop();
	}
	mIceService->stop();
}

Stream * StreamsGroup::getStream(size_t index){
	if (index >=  mStreams.size()){
		lError() << "No stream at index " << index;
		return nullptr;
	}
	return mStreams[index].get();
}

bool StreamsGroup::isPortUsed(int port)const{
	if (port == -1) return false;
	for(auto &stream : mStreams){
		if (stream && stream->isPortUsed(port)) return true;
	}
	return false;
}

LinphoneCore *StreamsGroup::getCCore()const{
	return mMediaSession.getCore()->getCCore();
}

Core & StreamsGroup::getCore()const{
	return *mMediaSession.getCore();
}

MediaSessionPrivate &StreamsGroup::getMediaSessionPrivate()const{
	return *getMediaSession().getPrivate();
}

int StreamsGroup::updateAllocatedAudioBandwidth (const PayloadType *pt, int maxbw) {
	mAudioBandwidth = PayloadTypeHandler::getAudioPayloadTypeBandwidth(pt, maxbw);
	lInfo() << "Audio bandwidth for StreamsGroup [" << this << "] is " << mAudioBandwidth;
	return mAudioBandwidth;
}

int StreamsGroup::getVideoBandwidth (const std::shared_ptr<SalMediaDescription> & md, const SalStreamDescription & desc) {
	int remoteBandwidth = 0;
	if (desc.bandwidth > 0)
		remoteBandwidth = desc.bandwidth;
	else if (md->bandwidth > 0) {
		/* Case where b=AS is given globally, not per stream */
		remoteBandwidth = PayloadTypeHandler::getRemainingBandwidthForVideo(md->bandwidth, mAudioBandwidth);
	}
	return PayloadTypeHandler::getMinBandwidth(
		PayloadTypeHandler::getRemainingBandwidthForVideo(linphone_core_get_upload_bandwidth(getCCore()), mAudioBandwidth), remoteBandwidth);
}

void StreamsGroup::zrtpStarted(Stream *mainZrtpStream){
	for (auto &stream : mStreams){
		if (!stream) continue;
		if (stream.get() != mainZrtpStream) stream->zrtpStarted(mainZrtpStream);
	}
	propagateEncryptionChanged();
}

bool StreamsGroup::allStreamsEncrypted () const {
	int activeStreamsCount = 0;
	for (auto &stream : mStreams){
		if (!stream) continue;
		if (stream->getState() == Stream::Running){
			++activeStreamsCount;
			if (!stream->isEncrypted()){
				return false;
			}
		}
	}
	return activeStreamsCount > 0;
}

void StreamsGroup::propagateEncryptionChanged () {
	getMediaSessionPrivate().propagateEncryptionChanged();
}

void StreamsGroup::authTokenReady(const string &authToken, bool verified) {
	mAuthToken = authToken;
	mAuthTokenVerified = verified;
	lInfo() << "Authentication token is " << mAuthToken << "(" << (mAuthTokenVerified ? "verified" : "unverified") << ")";
}

void StreamsGroup::setAuthTokenVerified(bool value){
	MS2Stream *s = lookupMainStreamInterface<MS2Stream>(SalAudio);
	if (!s || s->getState() != Stream::Running){
		lError() << "StreamsGroup::setAuthTokenVerified(): No audio stream or not started";
		return;
	}
	MSZrtpContext *zrtp_context = s->getZrtpContext();
	if (!zrtp_context) {
		lError() << "StreamsGroup::setAuthenticationTokenVerified(): No zrtp context";
		return;
	}
	// SAS verified
	if (value) {
		ms_zrtp_sas_verified(zrtp_context);
	} else { // SAS rejected
		ms_zrtp_sas_reset_verified(zrtp_context);
	}
	mAuthTokenVerified = value;
}

Stream * StreamsGroup::lookupStream(const SalStreamType type, const std::string & label) const {
	for (auto &s : mStreams){
		if (!s) continue;
		const auto streamLabel = s->getLabel();
		if ((s->getType() == type) && (label.compare(streamLabel) == 0)) {
			return s.get();
		}
	}
	return nullptr;
}

VideoStream *StreamsGroup::lookupItcStream(VideoStream *refStream) const {
#ifdef VIDEO_ENABLED
	for (auto &stream : mStreams){
		if (!stream) continue;
		const auto streamLabel = stream->getLabel();
		if ((stream->getType() == SalVideo) && (refStream->label && strcmp(refStream->label, streamLabel.c_str()) == 0)) {
			MS2Stream *s  =  dynamic_cast<MS2Stream *>(stream.get());
			MediaStream *ms = s->getMediaStream();
			if (ms) {
				VideoStream *vs = (VideoStream *)ms;
				if (vs != refStream)
					return vs;
			}
		}
	}
#endif // VIDEO_ENABLED
	return nullptr;
}


bool StreamsGroup::compareVideoColor(MSMireControl &cl, MediaStreamDir dir) const {
	for (auto &stream : mStreams){
		if (!stream) continue;
		MS2Stream *s  =  dynamic_cast<MS2Stream *>(stream.get());
		if(stream->getType() == SalVideo) {
			MediaStream *ms = s->getMediaStream();
			if (ms && media_stream_get_direction(ms) == dir) {
				VideoStream *vs = (VideoStream *)ms;
				if (vs->output && ms_filter_get_id(vs->output)== MS_ANALYSE_DISPLAY_ID){
					return (ms_filter_call_method(vs->output, MS_ANALYSE_DISPLAY_COMPARE_COLOR, &cl) == 0) ;
				}
			}
		}
	}
	return false;
}

bool StreamsGroup::checkRtpSession() const {
	for (auto &stream : mStreams){
		if (!stream) continue;
		MS2Stream *s  =  dynamic_cast<MS2Stream *>(stream.get());
		if(stream->getType() == SalVideo) {
			MediaStream *ms = s->getMediaStream();
			RtpSession *rtp_session = ms->sessions.rtp_session;
			if (!rtp_session) {
				lInfo() << "StreamsGroup::checkRtpSession(): session empty";
				return false;
			}
			const rtp_stats_t *rtps = rtp_session_get_stats(rtp_session);
			switch (media_stream_get_direction(ms)) {
				case MediaStreamRecvOnly:
					// Can be 0 if it's not attached with filter
					break;
				case MediaStreamSendOnly:
					if (rtps->packet_sent < 5) {
						return false;
					}
					break;
				case MediaStreamSendRecv:
					if (rtps->packet_recv < 5 || rtps->packet_sent < 5) {
						return false;
					}
					break;
				default:
					break;
			}
		}
	}
	return true;
}

Stream * StreamsGroup::lookupMainStream(SalStreamType type){
	for (auto &stream : mStreams){
		if (!stream) continue;
		if (stream->isMain() && stream->getType() == type){
			return stream.get();
		}
	}
	return nullptr;
}

Stream * StreamsGroup::lookupVideoStream ( MediaStreamDir dir) {
	for (auto &stream : mStreams){
		if (stream && (stream->getType() == SalVideo)){
			Stream *s = stream.get();
			MS2Stream *iface = dynamic_cast<MS2Stream*>(s);
			if (media_stream_get_direction(iface->getMediaStream()) == dir) {
				return stream.get();
			}
		}
	}
	return nullptr;
}

Stream * StreamsGroup::lookupVideoStream ( MSFilterId id) {
	for (auto &stream : mStreams){
		if (stream && (stream->getType() == SalVideo)){
			MS2Stream *s  =  dynamic_cast<MS2Stream *>(stream.get());
			MediaStream *ms = s->getMediaStream();
			VideoStream *vs = (VideoStream *)ms;
			if (vs && vs->source && ms_filter_get_id(vs->source)== id){
				return stream.get();
			}
		}
	}
	return nullptr;
}

void StreamsGroup::tryEarlyMediaForking(const OfferAnswerContext &params) {
	for (auto & s : mStreams) {
		if (!s) continue;
		params.scopeStreamToIndex(s->getIndex());
		const auto & refStream = params.getResultStreamDescription();
		if (!refStream.enabled() || refStream.getDirection() == SalStreamInactive)
			continue;
		
		const auto & newStream = params.getRemoteStreamDescription();
		
		if ((refStream.type == newStream.type) && !refStream.getPayloads().empty() && !newStream.getPayloads().empty()) {
			const OrtpPayloadType *refpt = refStream.getPayloads().front();
			const OrtpPayloadType *newpt = newStream.getPayloads().front();
			if ((strcmp(refpt->mime_type, newpt->mime_type) == 0) && (refpt->clock_rate == newpt->clock_rate)
				&& (payload_type_get_number(refpt) == payload_type_get_number(newpt))) {
					s->tryEarlyMediaForking(params);
			}
		}
	}
}

void StreamsGroup::finishEarlyMediaForking(){
	for (auto &stream : mStreams){
		if (!stream) continue;
		stream->finishEarlyMediaForking();
	}
}

bool StreamsGroup::isStarted()const{
	for( auto & stream : mStreams){
		if (!stream) continue;
		if (stream->getState() == Stream::Running) return true;
	}
	return false;
}

void StreamsGroup::clearStreams(){
	stop();
	mIceService.reset(new IceService(*this));
	mStreams.clear();
	mCurrentOfferAnswerState.clear();
}

size_t StreamsGroup::getActiveStreamsCount() const{
	size_t ret = 0;
	for( auto & stream : mStreams){
		if (!stream) continue;
		if (stream->getState() == Stream::Running) ++ret;
	}
	return ret;
}

bool StreamsGroup::isMuted() const{
	for (auto & stream : mStreams){
		if (!stream) continue;
		if (stream->getState() == Stream::Running){
			if (stream->isMuted() == false) return false;
		}
	}
	return true;
}

template< typename _functor>
float StreamsGroup::computeOverallQuality(_functor func){
	float globalRating = -1.0f;
	int countedStreams = 0;
	for (auto &stream : mStreams){
		if (!stream) continue;
		countedStreams++;
		float streamRating = func(stream.get());
		if (streamRating != -1.0f){
			if (globalRating == -1.0f){
				globalRating = streamRating;
			}else{
				globalRating += streamRating;
			}
		}
	}
	return countedStreams != 0 ? (globalRating / (float)countedStreams) : globalRating;
}

float StreamsGroup::getAverageQuality(){
	return computeOverallQuality(mem_fun(&Stream::getAverageQuality));
}

float StreamsGroup::getCurrentQuality(){
	return computeOverallQuality(mem_fun(&Stream::getCurrentQuality));
}

void StreamsGroup::startDtls(const OfferAnswerContext &params){
	for( auto & stream : mStreams){
		if (stream){
			params.scopeStreamToIndex(stream->getIndex());
			stream->startDtls(params);
		}
	}
}

int StreamsGroup::getAvpfRrInterval()const{
	int interval = 0;
	for( auto & stream : mStreams){
		if (!stream) continue;
		RtpInterface *i = dynamic_cast<MS2Stream*>(stream.get());
		if (i && i->getAvpfRrInterval() > interval) 
			interval = i->getAvpfRrInterval();
	}
	return interval;
}

bool StreamsGroup::avpfEnabled() const{
	bool ret = false;
	for( auto & stream : mStreams){
		if (!stream) continue;
		RtpInterface *i = dynamic_cast<MS2Stream*>(stream.get());
		if (i && stream->getState() == Stream::Running){
			if (!i->avpfEnabled()){
				return false;
			}
			ret = true;
		}
	}
	return ret;
}

void StreamsGroup::refreshSockets(){
	forEach<Stream>(mem_fun(&Stream::refreshSockets));
}

void StreamsGroup::computeAndReportBandwidth(){
	forEach<Stream>(mem_fun(&Stream::updateBandwidthReports));
	
	if (!bctbx_log_level_enabled(BCTBX_LOG_DOMAIN, BCTBX_LOG_MESSAGE)) return;
	
	ostringstream ostr;
	bool introDone = false;
	
	for (auto &stream : mStreams){
		if (!stream) continue;
		if (stream->getState() != Stream::Running) continue;
		LinphoneCallStats *stats = stream->getStats();
		if (!introDone){
			ostr << "Bandwidth usage for CallSession [" << &getMediaSession() << "]:" << endl << fixed << setprecision(2);
			introDone = true;
		}
		ostr << "\tStream #" << stream->getIndex() << " (" << sal_stream_type_to_string(stream->getType()) << ") | cpu: " << stream->getCpuUsage() << "% |" << " RTP : [d="
			<< linphone_call_stats_get_download_bandwidth(stats) << ",u=" << linphone_call_stats_get_upload_bandwidth(stats) << "] "
			<< "RTCP: [d=" << linphone_call_stats_get_rtcp_download_bandwidth(stats) << ",u=" << linphone_call_stats_get_rtcp_upload_bandwidth(stats) << "] ";
		
		float est_bw = linphone_call_stats_get_estimated_download_bandwidth(stats);
		if (est_bw > 0.0) ostr << "Est max d=" << est_bw;
		ostr << " (kbits/sec)" << endl;
	}
	lInfo() << ostr.str();
}

void StreamsGroup::addPostRenderHook(const std::function<void()> &l){
	mPostRenderHooks.push_back(l);
}

void StreamsGroup::setStreamMain(size_t index, const bool force){
	Stream *s = getStream(index);
	if (s){
		SalStreamType type = s->getType();
		// Make sure there is not already a "main" stream; which would be a programmer fault.
		Stream *other = lookupMainStream(type);
		if (other != nullptr && other != s){
			if (force) {
				other->resetMain();
			} else {
				lError() << "StreamsGroup::setStreamMain(): error, the main attribute has already been set on another stream.";
				return;
			}
		}
		s->setMain();
	}
}

void StreamsGroup::finish(){
	if (mFinished) return;
	lInfo() << "StreamsGroup::finish() called.";
	stop(); //For the paranoid: normally it should be done already.
	mIceService->finish(); // finish ICE first, as it has actions on the streams.
	for (auto & ss : mSharedServices) ss.second->checkDestroy();
	mSharedServices.clear();
	forEach<Stream>(mem_fun(&Stream::finish));
	mFinished = true;
}

void StreamsGroup::attachMixers(){
	if (!mMixerSession) return;
	for (auto & stream : mStreams){
		if (!stream) continue;
		if (stream->getMixer() == nullptr){
			StreamMixer *mixer = mMixerSession->getMixerByType(stream->getType());
			if (mixer) stream->connectToMixer(mixer);
		}
	}
}

void StreamsGroup::detachMixers(){
	for (auto & stream : mStreams){
		if (!stream) continue;
		if (stream->getMixer() != nullptr){
			stream->disconnectFromMixer();
		}
	}
}

void StreamsGroup::joinMixerSession(MixerSession *mixerSession){
	if (mMixerSession) lFatal() << "StreamsGroup::joinMixerSession() already joined !";
	mMixerSession = mixerSession;
	attachMixers();
}

void StreamsGroup::unjoinMixerSession(){
	if (!mMixerSession) lFatal() << "StreamsGroup::unjoinMixerSession() not joined !";
	detachMixers();
	mMixerSession = nullptr;
}


LINPHONE_END_NAMESPACE
