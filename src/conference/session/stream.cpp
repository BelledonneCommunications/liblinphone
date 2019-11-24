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
#include "call/call-p.h"
#include "conference/participant.h"
#include "utils/payload-type-handler.h"
#include "conference/params/media-session-params-p.h"

#include "linphone/core.h"


using namespace::std;

LINPHONE_BEGIN_NAMESPACE



void OfferAnswerContext::scopeStreamToIndex(size_t index) const{
	streamIndex = index;
	localStreamDescription = localMediaDescription ? &localMediaDescription->streams[index] : nullptr;
	remoteStreamDescription = remoteMediaDescription ? &remoteMediaDescription->streams[index] : nullptr;
	resultStreamDescription = resultMediaDescription ? &resultMediaDescription->streams[index] : nullptr;
}

void OfferAnswerContext::dupFrom(const OfferAnswerContext &ctx){
	OfferAnswerContext oldCtx = *this; // Transfers *this to a temporary object.
	localMediaDescription = ctx.localMediaDescription ? sal_media_description_ref(ctx.localMediaDescription) : nullptr;
	remoteMediaDescription = ctx.remoteMediaDescription ? sal_media_description_ref(const_cast<SalMediaDescription*>(ctx.remoteMediaDescription)) : nullptr;
	resultMediaDescription = ctx.resultMediaDescription ? sal_media_description_ref(const_cast<SalMediaDescription*>(ctx.resultMediaDescription)) : nullptr;
	// if the temporary oldCtx owns media descriptions, they will be unrefed by the destructor here.
}

void OfferAnswerContext::scopeStreamToIndexWithDiff(size_t index, const OfferAnswerContext &previousCtx) const{
	scopeStreamToIndex(index);
	previousCtx.scopeStreamToIndex(index);
	
	if (previousCtx.localMediaDescription){
		localStreamDescriptionChanges = sal_media_description_global_equals(previousCtx.localMediaDescription, localMediaDescription)
		| sal_stream_description_equals(previousCtx.localStreamDescription, localStreamDescription);
	}else localStreamDescriptionChanges = 0;
	if (previousCtx.resultMediaDescription){
		resultStreamDescriptionChanges = sal_media_description_global_equals(previousCtx.resultMediaDescription, resultMediaDescription)
		| sal_stream_description_equals(previousCtx.resultStreamDescription, resultStreamDescription);
	}else resultStreamDescriptionChanges = 0;
}

void OfferAnswerContext::clear(){
	if (mOwnsMediaDescriptions){
		if (localMediaDescription) sal_media_description_unref(localMediaDescription);
		if (remoteMediaDescription) sal_media_description_unref(const_cast<SalMediaDescription*>(remoteMediaDescription));
		if (resultMediaDescription) sal_media_description_unref(const_cast<SalMediaDescription*>(resultMediaDescription));
	}
	localMediaDescription = nullptr;
	remoteMediaDescription = nullptr;
	resultMediaDescription = nullptr;
	localStreamDescription = nullptr;
	remoteStreamDescription = nullptr;
	resultStreamDescription = nullptr;
	localStreamDescriptionChanges = 0;
	resultStreamDescriptionChanges = 0;
	mOwnsMediaDescriptions = false;
}

OfferAnswerContext::~OfferAnswerContext(){
	clear();
}



/*
 * Stream implementation.
 */


Stream::Stream(StreamsGroup &sg, const OfferAnswerContext &params) : mStreamsGroup(sg), mStreamType(params.localStreamDescription->type), mIndex(params.streamIndex){
	setPortConfig();
	fillMulticastMediaAddresses();
}

LinphoneCore *Stream::getCCore()const{
	return getCore().getCCore();
}

Core &Stream::getCore()const{
	return *mStreamsGroup.getMediaSession().getCore();
}

MediaSession &Stream::getMediaSession()const{
	return mStreamsGroup.getMediaSession();
}

MediaSessionPrivate &Stream::getMediaSessionPrivate()const{
	return *getMediaSession().getPrivate();
}

void Stream::fillLocalMediaDescription(OfferAnswerContext & ctx){
}

void Stream::prepare(){
	mState = Preparing;
}

void Stream::finishPrepare(){
	mState = Stopped;
}

void Stream::tryEarlyMediaForking(const OfferAnswerContext &ctx){
}

void Stream::render(const OfferAnswerContext & ctx, CallSession::State targetState){
	mState = Running;
}

void Stream::sessionConfirmed(const OfferAnswerContext &ctx){
}

void Stream::stop(){
	mState = Stopped;
}

void Stream::setRandomPortConfig () {
	mPortConfig.rtpPort = -1;
	mPortConfig.rtcpPort = -1;
}

int Stream::selectRandomPort (pair<int, int> portRange) {
	unsigned int rangeSize = static_cast<unsigned int>(portRange.second - portRange.first);
	
	for (int nbTries = 0; nbTries < 100; nbTries++) {
		bool alreadyUsed = false;
		unsigned int randomInRangeSize = (bctbx_random() % rangeSize) & (unsigned int)~0x1; /* Select an even number */
		int triedPort = ((int)randomInRangeSize) + portRange.first;
		/*If portRange.first is even, the triedPort will be even too. The one who configures a port range that starts with an odd number will
		 * get odd RTP port numbers.*/
		
		for (const bctbx_list_t *elem = linphone_core_get_calls(getCCore()); elem != nullptr; elem = bctbx_list_next(elem)) {
			LinphoneCall *lcall = reinterpret_cast<LinphoneCall *>(bctbx_list_get_data(elem));
			shared_ptr<MediaSession> session = static_pointer_cast<MediaSession>(L_GET_CPP_PTR_FROM_C_OBJECT(lcall)->getPrivate()->getActiveSession());
			if (session->getPrivate()->getStreamsGroup().isPortUsed(triedPort)) {
				alreadyUsed = true;
				break;
			}
		}
		if (!alreadyUsed){
			lInfo() << "Port " << triedPort << " randomly taken from range [ " << portRange.first << " , " << portRange.second << "]";
			return triedPort;
		}
	}

	lError() << "Could not find any free port!";
	return -1;
}

int Stream::selectFixedPort (pair<int, int> portRange) {
	for (int triedPort = portRange.first; triedPort < (portRange.first + 100); triedPort += 2) {
		bool alreadyUsed = false;
		for (const bctbx_list_t *elem = linphone_core_get_calls(getCCore()); elem != nullptr; elem = bctbx_list_next(elem)) {
			LinphoneCall *lcall = reinterpret_cast<LinphoneCall *>(bctbx_list_get_data(elem));
			shared_ptr<MediaSession> session = static_pointer_cast<MediaSession>(L_GET_CPP_PTR_FROM_C_OBJECT(lcall)->getPrivate()->getActiveSession());
			if (session->getPrivate()->getStreamsGroup().isPortUsed(triedPort)) {
				alreadyUsed = true;
				break;
			}
		}
		if (!alreadyUsed)
			return triedPort;
	}

	lError() << "Could not find any free port !";
	return -1;
}

void Stream::setPortConfig(pair<int, int> portRange) {
	if ((portRange.first <= 0) && (portRange.second <= 0)) {
		setRandomPortConfig();
	} else {
		if (portRange.first == portRange.second) {
			/* Fixed port */
			mPortConfig.rtpPort = selectFixedPort(portRange);
		} else {
			/* Select random port in the specified range */
			mPortConfig.rtpPort = selectRandomPort(portRange);
		}
	}
	if (mPortConfig.rtpPort == -1) setRandomPortConfig();
	else mPortConfig.rtcpPort = mPortConfig.rtpPort + 1;
}

void Stream::setPortConfig(){
	int minPort = 0, maxPort = 0;
	switch(getType()){
		case SalAudio:
			linphone_core_get_audio_port_range(getCCore(), &minPort, &maxPort);
		break;
		case SalVideo:
			linphone_core_get_video_port_range(getCCore(), &minPort, &maxPort);
		break;
		case SalText:
			linphone_core_get_text_port_range(getCCore(), &minPort, &maxPort);
		break;
		case SalOther:
		break;
	}
	setPortConfig(make_pair(minPort, maxPort));
}

void Stream::fillMulticastMediaAddresses () {
	mPortConfig.multicastIp.clear();
	if (getType() == SalAudio && getMediaSession().getPrivate()->getParams()->audioMulticastEnabled()){
		mPortConfig.multicastIp = linphone_core_get_audio_multicast_addr(getCCore());
	} else if (getType() == SalVideo && getMediaSession().getPrivate()->getParams()->videoMulticastEnabled()){
		mPortConfig.multicastIp = linphone_core_get_video_multicast_addr(getCCore());
	}
}

bool Stream::isPortUsed(int port)const{
	return port == mPortConfig.rtpPort || port == mPortConfig.rtcpPort;
}

IceAgent & Stream::getIceAgent()const{
	return mStreamsGroup.getIceAgent();
}

const string & Stream::getPublicIp() const{
	if (!mPortConfig.multicastIp.empty()){
			return mPortConfig.multicastIp;
	}
	return getMediaSessionPrivate().getMediaLocalIp();
}

LINPHONE_END_NAMESPACE
