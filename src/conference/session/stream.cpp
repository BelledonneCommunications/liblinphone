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
#include "streams.h"
#include "utils/payload-type-handler.h"

#include "linphone/core.h"

using namespace ::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * Stream implementation.
 */

Stream::Stream(StreamsGroup &sg, const OfferAnswerContext &params)
    : mStreamsGroup(sg), mStreamType(params.getLocalStreamDescription().type), mIndex(params.streamIndex) {
	setPortConfig();
	initMulticast(params);
	memset(&mInternalStats, 0, sizeof(mInternalStats));
}

void Stream::resetMain() {
	mIsMain = false;
}

void Stream::setMain() {
	mIsMain = true;
}

LinphoneCore *Stream::getCCore() const {
	return getCore().getCCore();
}

Core &Stream::getCore() const {
	return *getMediaSession().getCore();
}

MediaSession &Stream::getMediaSession() const {
	return mStreamsGroup.getMediaSession();
}

MediaSessionPrivate &Stream::getMediaSessionPrivate() const {
	return *getMediaSession().getPrivate();
}

void Stream::fillLocalMediaDescription(BCTBX_UNUSED(OfferAnswerContext &ctx)) {
}

bool Stream::prepare() {
	mState = Preparing;
	return false;
}

void Stream::finishPrepare() {
	mState = Stopped;
}

void Stream::tryEarlyMediaForking(BCTBX_UNUSED(const OfferAnswerContext &ctx)) {
}

void Stream::render(BCTBX_UNUSED(const OfferAnswerContext &ctx), BCTBX_UNUSED(CallSession::State state)) {
	mState = Running;
}

void Stream::sessionConfirmed(BCTBX_UNUSED(const OfferAnswerContext &ctx)) {
}

void Stream::stop() {
	mState = Stopped;
	mInternalStats.number_of_stops++;
}

void Stream::setIceCheckList(BCTBX_UNUSED(IceCheckList *cl)) {
}

void Stream::iceStateChanged() {
}

void Stream::goClearAckSent() {
}

void Stream::confirmGoClear() {
}

void Stream::connectToMixer(StreamMixer *mixer) {
	mMixer = mixer;
}

void Stream::disconnectFromMixer() {
	mMixer = nullptr;
}

StreamMixer *Stream::getMixer() const {
	return mMixer;
}

void Stream::setRandomPortConfig() {
	mPortConfig.rtpPort = -1;
	mPortConfig.rtcpPort = -1;
}

int Stream::selectRandomPort(pair<int, int> portRange) {
	auto max = portRange.second;
	auto min = portRange.first;
	auto range = (max - min);
	if (range < 0) {
		lError() << "Unable to select a random port within range [ " << min << " , " << max
		         << "] because the maximum value is lower than the minimum value";
		return -1;
	}

	unsigned int rangeSize = static_cast<unsigned int>(range);

	for (int nbTries = 0; nbTries < 100; nbTries++) {
		bool alreadyUsed = false;
		unsigned int randomInRangeSize = (bctbx_random() % rangeSize) & (unsigned int)~0x1; /* Select an even number */
		int triedPort = ((int)randomInRangeSize) + min;
		/*If min is even, the triedPort will be even too. The one who configures a port range that starts with an odd
		 * number will get odd RTP port numbers.*/

		for (const bctbx_list_t *elem = linphone_core_get_calls(getCCore()); elem != nullptr;
		     elem = bctbx_list_next(elem)) {
			LinphoneCall *lcall = static_cast<LinphoneCall *>(bctbx_list_get_data(elem));
			shared_ptr<MediaSession> session =
			    static_pointer_cast<MediaSession>(LinphonePrivate::Call::toCpp(lcall)->getActiveSession());
			if (session->getPrivate()->getStreamsGroup().isPortUsed(triedPort)) {
				alreadyUsed = true;
				break;
			}
		}
		if (!alreadyUsed) {
			lInfo() << "Port " << triedPort << " randomly taken from range [ " << min << " , " << max << "]";
			return triedPort;
		}
	}

	lError() << "Could not find any free port!";
	return -1;
}

int Stream::selectFixedPort(pair<int, int> portRange) {
	for (int triedPort = portRange.first; triedPort < (portRange.first + 100); triedPort += 2) {
		bool alreadyUsed = false;
		for (const bctbx_list_t *elem = linphone_core_get_calls(getCCore()); elem != nullptr;
		     elem = bctbx_list_next(elem)) {
			LinphoneCall *lcall = static_cast<LinphoneCall *>(bctbx_list_get_data(elem));
			shared_ptr<MediaSession> session =
			    static_pointer_cast<MediaSession>(LinphonePrivate::Call::toCpp(lcall)->getActiveSession());
			if (session->getPrivate()->getStreamsGroup().isPortUsed(triedPort)) {
				alreadyUsed = true;
				break;
			}
		}
		if (!alreadyUsed) return triedPort;
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

pair<int, int> Stream::getPortRange(LinphoneCore *core, const SalStreamType type) {
	int minPort = 0, maxPort = 0;
	switch (type) {
		case SalAudio:
			linphone_core_get_audio_port_range(core, &minPort, &maxPort);
			break;
		case SalVideo:
			linphone_core_get_video_port_range(core, &minPort, &maxPort);
			break;
		case SalText:
			linphone_core_get_text_port_range(core, &minPort, &maxPort);
			break;
		case SalOther:
			break;
	}

	return make_pair(minPort, maxPort);
}

void Stream::setPortConfig() {
	setPortConfig(Stream::getPortRange(getCCore(), getType()));
}

void Stream::initMulticast(const OfferAnswerContext &params) {
	mPortConfig.multicastRole = params.getLocalStreamDescription().multicast_role;
	lInfo() << *this << ": multicast role is [" << sal_multicast_role_to_string(mPortConfig.multicastRole) << "]";

	if (mPortConfig.multicastRole == SalMulticastReceiver) {
		mPortConfig.multicastIp = params.getRemoteStreamDescription().rtp_addr;
		mPortConfig.rtpPort = params.getRemoteStreamDescription().rtp_port;
		mPortConfig.rtcpPort = 0; /* RTCP is disabled for multicast */
	} else if (mPortConfig.multicastRole == SalMulticastSender) {
		if (getType() == SalAudio && getMediaSession().getPrivate()->getParams()->audioMulticastEnabled()) {
			mPortConfig.multicastIp = linphone_core_get_audio_multicast_addr(getCCore());
		} else if (getType() == SalVideo && getMediaSession().getPrivate()->getParams()->videoMulticastEnabled()) {
			mPortConfig.multicastIp = linphone_core_get_video_multicast_addr(getCCore());
		}
		/* multicastRtpPort is the one that will be advertised.
		 * However, the socket that will send to the multicast address/port does need need to bind on this port.
		 * It should not to avoid port conflicts with multicast receivers that may run on the same host.
		 * The bind() will be done on a random port instead.
		 */
		mPortConfig.multicastRtpPort = mPortConfig.rtpPort;
		if (mPortConfig.multicastRtpPort == -1) {
			/* we have to choose the multicast port now and the system can't choose it for us.*/
			mPortConfig.multicastRtpPort = selectRandomPort(make_pair(1024, 65535));
		}
		setRandomPortConfig();
	}
}

bool Stream::isPortUsed(int port) const {
	return port == mPortConfig.rtpPort || port == mPortConfig.rtcpPort || port == mPortConfig.multicastRtpPort;
}

IceService &Stream::getIceService() const {
	return mStreamsGroup.getIceService();
}

const string &Stream::getPublicIp() const {
	if (!mPortConfig.multicastIp.empty()) {
		return mPortConfig.multicastIp;
	}
	return getMediaSessionPrivate().getMediaLocalIp();
}

void Stream::finish() {
}

LINPHONE_END_NAMESPACE
