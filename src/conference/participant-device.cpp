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
#include "conference/session/media-session.h"
#include "conference/params/media-session-params.h"
#include "conference/params/media-session-params-p.h"
#include "participant-device.h"
#include "participant.h"
#include "core/core.h"
#include "private_functions.h"

#include "linphone/event.h"

using namespace std;

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Core;

// =============================================================================

ParticipantDevice::ParticipantDevice () {
	mTimeOfJoining = time(nullptr);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeAudio);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeVideo);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeText);
}

ParticipantDevice::ParticipantDevice (std::shared_ptr<Participant> participant, const std::shared_ptr<LinphonePrivate::CallSession> &session, const std::string &name)
	: mParticipant(participant), mGruu(IdentityAddress()), mName(name), mSession(session) {
	mTimeOfJoining = time(nullptr);
	if (mSession && mSession->getRemoteContactAddress()) {
		mGruu = IdentityAddress(*mSession->getRemoteContactAddress());
	}
	updateMediaCapabilities();
}

ParticipantDevice::ParticipantDevice (std::shared_ptr<Participant> participant, const IdentityAddress &gruu, const std::string &name)
	: mParticipant(participant), mGruu(gruu), mName(name) {
	mTimeOfJoining = time(nullptr);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeAudio);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeVideo);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeText);
}

ParticipantDevice::~ParticipantDevice () {
	if (mConferenceSubscribeEvent)
		linphone_event_unref(mConferenceSubscribeEvent);
}

bool ParticipantDevice::operator== (const ParticipantDevice &device) const {
	return (getAddress() == device.getAddress());
}

Conference* ParticipantDevice::getConference () const {
	return getParticipant() ? getParticipant()->getConference() : nullptr;
}

shared_ptr<Core> ParticipantDevice::getCore () const {
	return getParticipant() ? getParticipant()->getCore() : nullptr;
}

const IdentityAddress & ParticipantDevice::getAddress() const {
	if (!mGruu.isValid() && mSession && mSession->getRemoteContactAddress()) {
		mGruu = IdentityAddress(*mSession->getRemoteContactAddress());
	}
	return  mGruu;
}

std::shared_ptr<Participant> ParticipantDevice::getParticipant() const {
	if (mParticipant.expired()) {
		lWarning() << "The participant owning device " << getAddress().asString() << " has already been deleted";
	}
	shared_ptr<Participant> participant = mParticipant.lock();
	if (!participant) {
		lWarning() << "Unable to get the participant owning the device";
		return nullptr;
	}
	return participant;
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
		return encryptionEngine->getSecurityLevel(getAddress().asString());
	lWarning() << "Asking device security level but there is no encryption engine enabled";
	return AbstractChatRoom::SecurityLevel::ClearText;
}

time_t ParticipantDevice::getTimeOfJoining () const {
	return mTimeOfJoining;
}

bool ParticipantDevice::isInConference() const {
	const auto & conference = getConference();
	if (conference) {
		const auto & isMe = conference->isMe(getAddress());
		if (isMe) {
			return conference->isIn();
		} else {
			return (getState() == ParticipantDevice::State::Present);
		}
	}
	return false;
}

void ParticipantDevice::setSsrc (uint32_t ssrc) {
	mSsrc = ssrc;
}

uint32_t ParticipantDevice::getSsrc () const {
	return mSsrc;
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
		case ParticipantDevice::State::OnHold:
			return stream << "OnHold";
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
	// Estimate media capabilities based on call session
	updateMediaCapabilities();
}

LinphoneMediaDirection ParticipantDevice::getStreamCapability(const LinphoneStreamType type) const {
	try {
		return mediaCapabilities.at(type);
	} catch (std::out_of_range&) {
		return LinphoneMediaDirectionInactive;
	}
}

bool ParticipantDevice::setStreamCapability(const LinphoneMediaDirection & direction, const LinphoneStreamType type) {
	const bool idxFound = (mediaCapabilities.find(type) != mediaCapabilities.cend());
	if (!idxFound || (mediaCapabilities[type] != direction)) {
		mediaCapabilities[type] = direction;
		_linphone_participant_device_notify_stream_capability_changed(toC(), direction, type);
		return true;
	}
	return false;
}

bool ParticipantDevice::getStreamAvailability(const LinphoneStreamType type) const {
	try {
		return streamAvailabilities.at(type);
	} catch (std::out_of_range&) {
		return false;
	}
}

bool ParticipantDevice::setStreamAvailability(const bool available, const LinphoneStreamType type) {
	const bool idxFound = (mediaCapabilities.find(type) != mediaCapabilities.cend());
	if (!idxFound || (streamAvailabilities[type] != available)) {
		streamAvailabilities[type] = available;
		_linphone_participant_device_notify_stream_availability_changed(toC(), (available ? TRUE : FALSE), type);
		return true;
	}
	return false;
}

LinphoneMediaDirection ParticipantDevice::computeDeviceMediaDirection(const bool conferenceEnable, const bool callEnable, const LinphoneMediaDirection dir) const {
	if (conferenceEnable && callEnable) {
		return dir;
	}

	return LinphoneMediaDirectionInactive;
}

bool ParticipantDevice::updateMediaCapabilities() {
	bool mediaChanged = false;
	const auto & conference = getConference();

	if (conference) {
		const auto & isMe = conference->isMe(getAddress());
		const auto & conferenceParams = conference->getCurrentParams();
		const auto & conferenceAudioEnabled = conferenceParams.audioEnabled();
		const auto & conferenceVideoEnabled = conferenceParams.videoEnabled();
		const auto & conferenceTextEnabled = conferenceParams.chatEnabled();
		auto audioEnabled = false;
		auto videoEnabled = false;
		auto textEnabled = false;
		auto audioDir = LinphoneMediaDirectionInactive;
		auto videoDir = LinphoneMediaDirectionInactive;
		auto textDir = LinphoneMediaDirectionInactive;
		if (mSession) {
			const MediaSessionParams* participantParams = static_pointer_cast<MediaSession>(mSession)->getRemoteParams();
			if (participantParams) {
				audioEnabled = participantParams->audioEnabled();
				videoEnabled = participantParams->videoEnabled();
				textEnabled = participantParams->realtimeTextEnabled();
				audioDir = participantParams->getAudioDirection();
				videoDir = participantParams->getVideoDirection();
			}
		} else if (isMe) {
			audioEnabled = true;
			videoEnabled = linphone_core_video_enabled(getCore()->getCCore());
			textEnabled = true;
			const auto & cCore = getCore()->getCCore();
			audioDir = LinphoneMediaDirectionSendRecv;

			const auto captureEnabled = linphone_core_video_capture_enabled(cCore);
			const auto displayEnabled = linphone_core_video_display_enabled(cCore);
			if (captureEnabled && displayEnabled) {
				videoDir = LinphoneMediaDirectionSendRecv;
			} else if (captureEnabled && !displayEnabled) {
				videoDir = LinphoneMediaDirectionSendOnly;
			} else if (!captureEnabled && displayEnabled) {
				videoDir = LinphoneMediaDirectionRecvOnly;
			} else {
				videoDir = LinphoneMediaDirectionInactive;
			}
		}
		textDir = LinphoneMediaDirectionSendRecv;
		mediaChanged |= setStreamCapability(computeDeviceMediaDirection(conferenceAudioEnabled, audioEnabled, audioDir), LinphoneStreamTypeAudio);
		mediaChanged |= setStreamCapability(computeDeviceMediaDirection(conferenceVideoEnabled, videoEnabled, videoDir), LinphoneStreamTypeVideo);
		mediaChanged |= setStreamCapability(computeDeviceMediaDirection(conferenceTextEnabled, textEnabled, textDir), LinphoneStreamTypeText);
	} else {
		mediaChanged |= setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeAudio);
		mediaChanged |= setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeVideo);
		mediaChanged |= setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeText);
	}

	return mediaChanged;
}

bool ParticipantDevice::computeStreamAvailable(const bool conferenceEnable, const bool callEnable, const LinphoneMediaDirection dir) const {
	return (computeDeviceMediaDirection(conferenceEnable, callEnable, dir) != LinphoneMediaDirectionInactive);
}

bool ParticipantDevice::updateStreamAvailabilities() {
	const auto & conference = getConference();
	auto streamAvailabilityChanged = false;

	const auto session = conference ? conference->getMainSession() : nullptr;
	if (session) {

		const MediaSessionParams* params = static_pointer_cast<MediaSession>(session)->getCurrentParams();

		auto audioEnabled = false;
		auto videoEnabled = false;
		auto textEnabled = false;
		if (params) {
			audioEnabled = params->audioEnabled();
			videoEnabled = params->videoEnabled();
			textEnabled = params->realtimeTextEnabled();
		}

		const auto & conferenceParams = conference->getCurrentParams();

		streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceParams.audioEnabled(), audioEnabled, getStreamCapability(LinphoneStreamTypeAudio)), LinphoneStreamTypeAudio);
		streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceParams.videoEnabled(), videoEnabled, getStreamCapability(LinphoneStreamTypeVideo)), LinphoneStreamTypeVideo);
		streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceParams.chatEnabled(), textEnabled, getStreamCapability(LinphoneStreamTypeText)), LinphoneStreamTypeText);
	}

	return streamAvailabilityChanged;

}

bool ParticipantDevice::adminModeSupported() const {
	return mSupportAdminMode;

}

void ParticipantDevice::enableAdminModeSupport(bool support) {
	mSupportAdminMode = support;

}

void * ParticipantDevice::createWindowId() const {
	if (!mWindowId && !mLabel.empty() && mSession)
		return static_pointer_cast<MediaSession>(mSession)->createNativeVideoWindowId(mLabel);
	else
		return nullptr;
}

void ParticipantDevice::setWindowId(void * newWindowId) {
#ifdef VIDEO_ENABLED
	mWindowId = newWindowId;
	if (!mLabel.empty() && mSession) {
		static_pointer_cast<MediaSession>(mSession)->setNativeVideoWindowId(mWindowId, mLabel);
	}
#endif
}

void * ParticipantDevice::getWindowId() const {
	return mWindowId;
}

LinphoneParticipantDeviceCbsIsSpeakingChangedCb ParticipantDeviceCbs::getIsSpeakingChanged()const{
	return mIsSpeakingChangedCb;
}

void ParticipantDeviceCbs::setIsSpeakingChanged(LinphoneParticipantDeviceCbsIsSpeakingChangedCb cb){
	mIsSpeakingChangedCb = cb;
}

LinphoneParticipantDeviceCbsConferenceJoinedCb ParticipantDeviceCbs::getConferenceJoined()const {
	return mConferenceJoinedCb;
}

void ParticipantDeviceCbs::setConferenceJoined(LinphoneParticipantDeviceCbsConferenceJoinedCb cb) {
	mConferenceJoinedCb = cb;
}

LinphoneParticipantDeviceCbsConferenceLeftCb ParticipantDeviceCbs::getConferenceLeft()const {
	return mConferenceLeftCb;
}

void ParticipantDeviceCbs::setConferenceLeft(LinphoneParticipantDeviceCbsConferenceLeftCb cb){
	mConferenceLeftCb = cb;
}

LinphoneParticipantDeviceCbsStreamCapabilityChangedCb ParticipantDeviceCbs::getStreamCapabilityChanged()const {
	return mStreamCapabilityChangedCb;
}

void ParticipantDeviceCbs::setStreamCapabilityChanged(LinphoneParticipantDeviceCbsStreamCapabilityChangedCb cb) {
	mStreamCapabilityChangedCb = cb;
}

LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb ParticipantDeviceCbs::getStreamAvailabilityChanged()const {
	return mStreamAvailabilityChangedCb;
}

void ParticipantDeviceCbs::setStreamAvailabilityChanged(LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb cb) {
	mStreamAvailabilityChangedCb = cb;
}


LINPHONE_END_NAMESPACE
