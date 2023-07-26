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
#include "bctoolbox/defs.h"

#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant.h"
#include "core/core.h"
#include "media-session-p.h"
#include "media-session.h"
#include "ms2-streams.h"

#include "linphone/core.h"

using namespace ::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * MS2RTTStream implementation.
 */

MS2RTTStream::MS2RTTStream(StreamsGroup &sg, const OfferAnswerContext &params) : MS2Stream(sg, params) {
	string bindIp = getBindIp();
	mStream = text_stream_new2(getCCore()->factory, bindIp.empty() ? nullptr : bindIp.c_str(), mPortConfig.rtpPort,
	                           mPortConfig.rtcpPort);
	initializeSessions(&mStream->ms);
}

void MS2RTTStream::configure(BCTBX_UNUSED(const OfferAnswerContext &params)) {
}

void MS2RTTStream::realTimeTextCharacterReceived(BCTBX_UNUSED(MSFilter *f), unsigned int id, void *arg) {
	if (id == MS_RTT_4103_RECEIVED_CHAR) {
		RealtimeTextReceivedCharacter *data = static_cast<RealtimeTextReceivedCharacter *>(arg);
		getMediaSession().notifyRealTimeTextCharacterReceived(data);
	}
}

void MS2RTTStream::sRealTimeTextCharacterReceived(void *userData, MSFilter *f, unsigned int id, void *arg) {
	MS2RTTStream *zis = static_cast<MS2RTTStream *>(userData);
	zis->realTimeTextCharacterReceived(f, id, arg);
}

bool MS2RTTStream::prepare() {
	MS2Stream::prepare();
	if (isTransportOwner()) {
		text_stream_prepare_text(mStream);
	}
	return false;
}

void MS2RTTStream::finishPrepare() {
	MS2Stream::finishPrepare();
	text_stream_unprepare_text(mStream);
}

void MS2RTTStream::render(const OfferAnswerContext &params, CallSession::State targetState) {
	const auto &tstream = params.getResultStreamDescription();
	bool basicChangesHandled = handleBasicChanges(params, targetState);

	if (basicChangesHandled) {
		if (getState() == Running) MS2Stream::render(params, targetState);
		return;
	}

	MS2Stream::render(params, targetState);
	RtpAddressInfo dest;
	getRtpDestination(params, &dest);
	int usedPt = -1;
	RtpProfile *textProfile = makeProfile(params.resultMediaDescription, tstream, &usedPt);
	if (usedPt == -1) {
		lError() << "No payload type was accepted for text stream.";
		stop();
		return;
	}
	OrtpPayloadType *pt = rtp_profile_get_payload(textProfile, usedPt);
	getMediaSessionPrivate().getCurrentParams()->getPrivate()->setUsedRealtimeTextCodec(
	    pt ? PayloadType::create(getCore().getSharedFromThis(), pt) : nullptr);
	getMediaSessionPrivate().getCurrentParams()->enableRealtimeText(true);

	unsigned int interval = getMediaSessionPrivate().getParams()->realtimeTextKeepaliveInterval();
	getMediaSessionPrivate().getCurrentParams()->setRealtimeTextKeepaliveInterval(interval);

	text_stream_start(mStream, textProfile, dest.rtpAddr.c_str(), dest.rtpPort, dest.rtcpAddr.c_str(), dest.rtcpPort,
	                  usedPt);
	ms_filter_add_notify_callback(mStream->rttsink, sRealTimeTextCharacterReceived, this, false);
	ms_filter_call_method(mStream->rttsource, MS_RTT_4103_SOURCE_SET_KEEP_ALIVE_INTERVAL, &interval);
	mInternalStats.number_of_starts++;
}

void MS2RTTStream::stop() {
	MS2Stream::stop();
	text_stream_stop(mStream);
	/* In mediastreamer2, stop actually stops and destroys. We immediately need to recreate the stream object for later
	 * use, keeping the sessions (for RTP, SRTP, ZRTP etc) that were setup at the beginning. */
	mStream = text_stream_new_with_sessions(getCCore()->factory, &mSessions);
}

void MS2RTTStream::finish() {
	if (mStream) {
		text_stream_stop(mStream);
		mStream = nullptr;
	}
}

MS2RTTStream::~MS2RTTStream() {
	finish();
}

MediaStream *MS2RTTStream::getMediaStream() const {
	return &mStream->ms;
}

void MS2RTTStream::handleEvent(BCTBX_UNUSED(const OrtpEvent *ev)) {
}

void MS2RTTStream::initZrtp() {
}

std::string MS2RTTStream::getLabel() const {
	return std::string();
}

void MS2RTTStream::startZrtp() {
}

LINPHONE_END_NAMESPACE
