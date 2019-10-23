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

#include "streams.h"
#include "media-session.h"
#include "media-session-p.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "call/call-p.h"

#include "linphone/core.h"


using namespace::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * StreamGroup implementation
 */

StreamsGroup::StreamsGroup(MediaSession &session) : mMediaSession(session){
}

Stream * StreamsGroup::createStream(SalStreamType type, size_t index){
	Stream *ret = nullptr;
	switch(type){
		case SalAudio:
			ret = new MS2AudioStream(*this, index);
		break;
		case SalVideo:
			ret = new MS2VideoStream(*this, index);
		break;
		case SalText:
			ret = new MS2RealTimeTextStream(*this, index);
		break;
		case SalOther:
		break;
	}
	if (!ret){
		lError() << "Could not create Stream of type " << sal_stream_type_to_string(type);
		return nullptr;
	}
	
	if (index >= mStreams.size()) mStreams.resize(index+1);
	mStreams[index].reset(ret);
	return ret;
}

Stream * StreamsGroup::getStream(size_t index){
	if (index >=  mStreams.size()){
		lError() << "Bad stream index " << index;
		return nullptr;
	}
	return mStreams[index].get();
}

std::list<Stream*> StreamsGroup::getStreams(){
	list<Stream*> ret;
	for (auto &s : mStreams){
		if (s) ret.push_back(s.get());
	}
	return ret;
}

bool StreamsGroup::isPortUsed(int port)const{
	if (port == -1) return false;
	for(auto &stream : mStreams){
		if (stream && stream->isPortUsed(port)) return true;
	}
	return false;
}


/*
 * Stream implementation.
 */


Stream::Stream(StreamsGroup &sg, SalStreamType type, size_t index) : mStreamsGroup(sg), mStreamType(type), mIndex(index){
	setPortConfig();
	fillMulticastMediaAddresses();
}

LinphoneCore *Stream::getCCore()const{
	return mStreamsGroup.getMediaSession().getCore()->getCCore();
}

MediaSession &Stream::getMediaSession()const{
	return mStreamsGroup.getMediaSession();
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



/*
 * MS2Stream implementation
 */

MS2Stream::MS2Stream(StreamsGroup &sg, SalStreamType type, size_t index) : Stream(sg, type, index){
	memset(&mSessions, 0, sizeof(mSessions));
	
}

void MS2Stream::start(SalStreamDescription *streamDesc, CallSession::State targetState){
}
void MS2Stream::stop(){
}
void MS2Stream::update(SalStreamDescription *oldSd, SalStreamDescription *newSd){
}
void MS2Stream::startDtls(){
}

MS2Stream::~MS2Stream(){
}


void MS2Stream::setPortConfigFromRtpSession () {
	mPortConfig.rtpPort = rtp_session_get_local_port(mSessions.rtp_session);
	mPortConfig.rtcpPort = rtp_session_get_local_rtcp_port(mSessions.rtp_session);
}

/*
 * MS2AudioStream implementation.
 */

MS2AudioStream::MS2AudioStream(StreamsGroup &sg, size_t index) : MS2Stream(sg, SalAudio, index){
}

MS2AudioStream::~MS2AudioStream(){
}

/*
 * MS2VideoStream implemenation
 */

MS2VideoStream::MS2VideoStream(StreamsGroup &sg, size_t index) : MS2Stream(sg, SalVideo, index){
}

/*
 * MS2RealTimeTextStream implemenation.
 */

MS2RealTimeTextStream::MS2RealTimeTextStream(StreamsGroup &sg, size_t index) : MS2Stream(sg, SalText, index){
}



LINPHONE_END_NAMESPACE
