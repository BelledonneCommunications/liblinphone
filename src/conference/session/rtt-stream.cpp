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
#include "bctoolbox/defs.h"

#include "streams.h"
#include "media-session.h"
#include "media-session-p.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "call/call-p.h"
#include "conference/participant.h"
#include "conference/params/media-session-params-p.h"

#include "linphone/core.h"

using namespace ::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * MS2RTTStream implementation.
 */

MS2RTTStream::MS2RTTStream(StreamsGroup &sg, const OfferAnswerContext &params) : MS2Stream(sg, params){
	string bindIp = getBindIp();
	mStream = text_stream_new2(getCCore()->factory, bindIp.empty() ? nullptr : bindIp.c_str(), mPortConfig.rtpPort, mPortConfig.rtcpPort);
	initializeSessions(&mStream->ms);
}

void MS2RTTStream::realTimeTextCharacterReceived (MSFilter *f, unsigned int id, void *arg) {
	CallSessionListener *listener = getMediaSessionPrivate().getCallSessionListener();
	
	if (id == MS_RTT_4103_RECEIVED_CHAR) {
		RealtimeTextReceivedCharacter *data = static_cast<RealtimeTextReceivedCharacter *>(arg);
		if (listener)
			listener->onRealTimeTextCharacterReceived(getMediaSession().getSharedFromThis(), data);
	}
}

void MS2RTTStream::sRealTimeTextCharacterReceived (void *userData, MSFilter *f, unsigned int id, void *arg) {
	MS2RTTStream *zis = static_cast<MS2RTTStream *>(userData);
	zis->realTimeTextCharacterReceived(f, id, arg);
}

void MS2RTTStream::prepare(){
	MS2Stream::prepare();
	text_stream_prepare_text(mStream);
}

void MS2RTTStream::finishPrepare(){
	MS2Stream::finishPrepare();
	text_stream_unprepare_text(mStream);
}

void MS2RTTStream::render(const OfferAnswerContext &params, CallSession::State targetState){
	const SalStreamDescription *tstream = params.resultStreamDescription;
	
	if (tstream->dir == SalStreamInactive || tstream->rtp_port == 0){
		stop();
		return;
	}
	
	const char *rtpAddr = tstream->rtp_addr[0] != '\0' ? tstream->rtp_addr : params.resultMediaDescription->addr;
	const char *rtcpAddr = tstream->rtcp_addr[0] != '\0' ? tstream->rtcp_addr : params.resultMediaDescription->addr;
	int usedPt = -1;
	RtpProfile * textProfile = makeProfile(params.resultMediaDescription, tstream, &usedPt);
	if (usedPt == -1){
		lError() << "No payload type was accepted for text stream.";
		stop();
		return;
	}
	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedRealtimeTextCodec(rtp_profile_get_payload(textProfile, usedPt));
	getMediaSessionPrivate().getCurrentParams()->enableRealtimeText(true);
	bool isMulticast = !!ms_is_multicast(rtpAddr);
	
	MS2Stream::render(params, targetState);
	text_stream_start(mStream, textProfile, rtpAddr, tstream->rtp_port, rtcpAddr,
		(linphone_core_rtcp_enabled(getCCore()) && !isMulticast) ? (tstream->rtcp_port ? tstream->rtcp_port : tstream->rtp_port + 1) : 0, usedPt);
	ms_filter_add_notify_callback(mStream->rttsink, sRealTimeTextCharacterReceived, this, false);
	mStartCount++;
}

void MS2RTTStream::stop(){
	MS2Stream::stop();
	text_stream_stop(mStream);
	/* In mediastreamer2, stop actually stops and destroys. We immediately need to recreate the stream object for later use, keeping the 
	 * sessions (for RTP, SRTP, ZRTP etc) that were setup at the beginning. */
	mStream = text_stream_new_with_sessions(getCCore()->factory, &mSessions);
}

MS2RTTStream::~MS2RTTStream(){
	text_stream_stop(mStream);
}

MediaStream *MS2RTTStream::getMediaStream()const{
	return &mStream->ms;
}

void MS2RTTStream::handleEvent(const OrtpEvent *ev){
}




LINPHONE_END_NAMESPACE
