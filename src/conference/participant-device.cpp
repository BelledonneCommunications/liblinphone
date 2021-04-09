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

#include "chat/encryption/encryption-engine.h"
#include "conference/session/call-session-p.h"
#include "conference/params/media-session-params.h"
#include "participant-device.h"
#include "participant.h"
#include "core/core.h"

#include "linphone/event.h"

using namespace std;

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Core;

// =============================================================================

ParticipantDevice::ParticipantDevice () {
	mTimeOfJoining = time(nullptr);
}

ParticipantDevice::ParticipantDevice (Participant *participant, const IdentityAddress &gruu, const string &name)
	: mParticipant(participant), mGruu(gruu), mName(name) {
	mTimeOfJoining = time(nullptr);
}

ParticipantDevice::~ParticipantDevice () {
	if (mConferenceSubscribeEvent)
		linphone_event_unref(mConferenceSubscribeEvent);
}

bool ParticipantDevice::operator== (const ParticipantDevice &device) const {
	return (mGruu == device.getAddress());
}

shared_ptr<Core> ParticipantDevice::getCore () const {
	return mParticipant ? mParticipant->getCore() : nullptr;
}

void ParticipantDevice::setConferenceSubscribeEvent (LinphoneEvent *ev) {
	if (ev) linphone_event_ref(ev);
	if (mConferenceSubscribeEvent){
		linphone_event_unref(mConferenceSubscribeEvent);
		mConferenceSubscribeEvent = nullptr;
	}
	mConferenceSubscribeEvent = ev;
}

AbstractChatRoom::SecurityLevel ParticipantDevice::getSecurityLevel () const {
	auto encryptionEngine = getCore()->getEncryptionEngine();
	if (encryptionEngine)
		return encryptionEngine->getSecurityLevel(mGruu.asString());
	lWarning() << "Asking device security level but there is no encryption engine enabled";
	return AbstractChatRoom::SecurityLevel::ClearText;
}

time_t ParticipantDevice::getTimeOfJoining () const {
	return mTimeOfJoining;
}

bool ParticipantDevice::isInConference() const {
	if (mSession) {
		return mSession->getPrivate()->isInConference();
	} else {
		return false;
	}
}

void *ParticipantDevice::getUserData () const{
	return mUserData;
}

void ParticipantDevice::setUserData (void *ud) {
	mUserData = ud;
}

ostream &operator<< (ostream &stream, ParticipantDevice::State state) {
	switch (state) {
		case ParticipantDevice::State::ScheduledForJoining:
			return stream << "ScheduledForJoining";
		case ParticipantDevice::State::Joining:
			return stream << "Joining";
		case ParticipantDevice::State::Present:
			return stream << "Present";
		case ParticipantDevice::State::ScheduledForLeaving:
			return stream << "ScheduledForLeaving";
		case ParticipantDevice::State::Leaving:
			return stream << "Leaving";
		case ParticipantDevice::State::Left:
			return stream << "Left";
	}
	return stream;
}

void ParticipantDevice::setCapabilityDescriptor(const std::string &capabilities){
	mCapabilityDescriptor = capabilities;
}

void ParticipantDevice::setSession (std::shared_ptr<CallSession> session) {
	mSession = session;
	updateMedia();
}

LinphoneMediaDirection ParticipantDevice::getMediaDirection(const MediaCapabilities capIdx) const {
	return static_cast<LinphoneMediaDirection>(mediaCapabilities[static_cast<int>(capIdx)]);
}

LinphoneMediaDirection ParticipantDevice::getAudioDirection() const {
	return getMediaDirection(MediaCapabilities::Audio);
}

LinphoneMediaDirection ParticipantDevice::getVideoDirection() const {
	return getMediaDirection(MediaCapabilities::Video);
}

LinphoneMediaDirection ParticipantDevice::getTextDirection() const {
	return getMediaDirection(MediaCapabilities::Text);
}

bool ParticipantDevice::setMediaDirection(const LinphoneMediaDirection & direction, const MediaCapabilities capIdx) {
	if (mediaCapabilities[static_cast<int>(capIdx)] != static_cast<int>(direction)) {
		mediaCapabilities[static_cast<int>(capIdx)] = static_cast<int>(direction);
		return true;
	}
	return false;
}

bool ParticipantDevice::setAudioDirection(const LinphoneMediaDirection direction) {
	return setMediaDirection(direction, MediaCapabilities::Audio);
}

bool ParticipantDevice::setVideoDirection(const LinphoneMediaDirection direction) {
	return setMediaDirection(direction, MediaCapabilities::Video);
}

bool ParticipantDevice::setTextDirection(const LinphoneMediaDirection direction) {
	return setMediaDirection(direction, MediaCapabilities::Text);
}

bool ParticipantDevice::updateMedia() {
	const auto & currentParams = static_cast<MediaSessionParams*>(mSession->getCurrentParams());
	bool mediaChanged = false;

	const auto & audioDir = currentParams->getAudioDirection();
	mediaChanged |= setAudioDirection((audioDir == LinphoneMediaDirectionSendOnly) ? LinphoneMediaDirectionInactive : audioDir);

	const auto & videoDir = currentParams->getVideoDirection();
	mediaChanged |= setVideoDirection((videoDir == LinphoneMediaDirectionSendOnly) ? LinphoneMediaDirectionInactive : videoDir);

	const auto & textEnabled = currentParams->realtimeTextEnabled();
	mediaChanged |= setTextDirection((textEnabled) ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive);

	return mediaChanged;
}

LINPHONE_END_NAMESPACE
