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
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeAudio);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeVideo);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeText);
}

ParticipantDevice::ParticipantDevice (std::shared_ptr<Participant> participant, const std::shared_ptr<LinphonePrivate::CallSession> &session, const std::string &name)
	: mParticipant(participant), mGruu(participant->getAddress()), mName(name), mSession(session) {
	if (mSession && mSession->getRemoteContactAddress()) {
		setAddress(*mSession->getRemoteContactAddress());
	}
	updateMediaCapabilities();
	updateStreamAvailabilities();
}

ParticipantDevice::ParticipantDevice (std::shared_ptr<Participant> participant, const IdentityAddress &gruu, const std::string &name)
	: mParticipant(participant), mGruu(gruu), mName(name) {
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
	return  mGruu;
}

void ParticipantDevice::setAddress (const IdentityAddress & address) {
	setAddress (address.asAddress());
}

void ParticipantDevice::setAddress (const Address & address) {
	mGruu = address;
	if (address.hasParam("+org.linphone.specs")) {
		const auto &linphoneSpecs = address.getParamValue("+org.linphone.specs");
		setCapabilityDescriptor(linphoneSpecs.substr(1, linphoneSpecs.size()-2));
	}
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

time_t ParticipantDevice::getTimeOfDisconnection () const {
	return mTimeOfDisconnection;
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

bool ParticipantDevice::setSsrc (const LinphoneStreamType type, uint32_t newSsrc) {
	bool changed = false;
	const bool idxFound = (ssrc.find(type) != ssrc.cend());
	if (!idxFound || (ssrc[type] != newSsrc)) {
		ssrc[type] = newSsrc;
		changed = true;
	}

	auto conference = getConference();
	switch (type) {
		case LinphoneStreamTypeAudio:
			if (conference) {
				const auto & pendingParticipantsMutes = conference->getPendingParticipantsMutes();
				auto it = pendingParticipantsMutes.find(newSsrc);
				if (it != pendingParticipantsMutes.end()) {
					conference->notifyMutedDevice(it->first, it->second);
				}
			}
			break;
		case LinphoneStreamTypeVideo:
		case LinphoneStreamTypeText:
		case LinphoneStreamTypeUnknown:
			break;
	}

	if (changed) {
		if (conference) {
			lInfo() << "Setting " << std::string(linphone_stream_type_to_string(type)) << " ssrc of participant device " << getAddress() << " in conference " << conference->getConferenceAddress() << " to " << newSsrc;
		} else {
			lInfo() << "Setting " << std::string(linphone_stream_type_to_string(type)) << " ssrc of participant device " << getAddress() << " to " << newSsrc;
		}
	}

	return changed;
}

uint32_t ParticipantDevice::getSsrc (const LinphoneStreamType type) const {
	try {
		return ssrc.at(type);
	} catch (std::out_of_range&) {
		return 0;
	}
}

void *ParticipantDevice::getUserData () const{
	return mUserData;
}

void ParticipantDevice::setUserData (void *ud) {
	mUserData = ud;
}

const std::string &ParticipantDevice::getCallId () {
	if (mCallId.empty() && mSession) {
		const auto & log = mSession->getLog();
		mCallId = log->getCallId();
	}

	return mCallId;
}

void ParticipantDevice::setCallId (const std::string &callId) {
	mCallId = callId;
}

const std::string &ParticipantDevice::getFromTag () {
	if (mFromTag.empty() && mSession) {
		mFromTag = mSession->getFromTag();
	}

	return mFromTag;
}

void ParticipantDevice::setFromTag (const std::string &tag) {
	mFromTag = tag;
}

const std::string &ParticipantDevice::getToTag () {
	if (mToTag.empty() && mSession) {
		mToTag = mSession->getToTag();
	}

	return mToTag;
}

void ParticipantDevice::setToTag (const std::string &tag) {
	mToTag = tag;
}

bool ParticipantDevice::isLeavingState(const ParticipantDevice::State & state) {
	switch (state) {
		case ParticipantDevice::State::ScheduledForJoining:
		case ParticipantDevice::State::Joining:
		case ParticipantDevice::State::Alerting:
		case ParticipantDevice::State::Present:
		case ParticipantDevice::State::OnHold:
		case ParticipantDevice::State::MutedByFocus:
			return false;
		case ParticipantDevice::State::ScheduledForLeaving:
		case ParticipantDevice::State::Leaving:
		case ParticipantDevice::State::Left:
			return true;
	}
	return false;
}

void ParticipantDevice::setState (State newState, bool notify) {
	if (mState != newState) {
		const auto currentStateLeavingState = ParticipantDevice::isLeavingState(mState);
		const auto newStateLeavingState = ParticipantDevice::isLeavingState(newState);
		// Send NOTIFY only if not transitioning from a leaving state to another one
		const bool sendNotify = !(newStateLeavingState && currentStateLeavingState) && notify;

		if ((newState == ParticipantDevice::State::Present) && (mState != ParticipantDevice::State::OnHold)) {
			setTimeOfJoining(time(nullptr));
		}
		lInfo() << "Moving participant device " << getAddress() << " from state " << mState << " to " << newState;
		mState = newState;
		_linphone_participant_device_notify_state_changed(toC(), (LinphoneParticipantDeviceState)newState);
		const auto & conference = getConference();
		if (conference && sendNotify) {
			conference->notifyParticipantDeviceStateChanged (ms_time(nullptr), false, getParticipant(), getSharedFromThis());
		}
	}
}

ostream &operator<< (ostream &stream, ParticipantDevice::State state) {
	switch (state) {
		case ParticipantDevice::State::ScheduledForJoining:
			return stream << "ScheduledForJoining";
		case ParticipantDevice::State::Joining:
			return stream << "Joining";
		case ParticipantDevice::State::Alerting:
			return stream << "Alerting";
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
		case ParticipantDevice::State::MutedByFocus:
			return stream << "MutedByFocus";
	}
	return stream;
}

void ParticipantDevice::setCapabilityDescriptor(const std::string &capabilities){
	mCapabilityDescriptor = capabilities;
}

void ParticipantDevice::setSession (std::shared_ptr<CallSession> session) {
	mSession = session;
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

LinphoneMediaDirection ParticipantDevice::getStreamDirectionFromSession(const LinphoneStreamType type) const {
	const auto & state = mSession->getState();
	const auto & sessionNotEstablished =
		(state == CallSession::State::Idle) ||
		(state == CallSession::State::IncomingReceived) ||
		(state == CallSession::State::OutgoingProgress) ||
		(state == CallSession::State::OutgoingRinging) ||
		(state == CallSession::State::OutgoingEarlyMedia) ||
		(state == CallSession::State::PushIncomingReceived);

	const MediaSessionParams* participantParams = nullptr;
	if (mSession) {
		if (sessionNotEstablished) {
			if (mSession->getPrivate()->isInConference()) {
				participantParams = static_pointer_cast<MediaSession>(mSession)->getRemoteParams();
			} else {
				participantParams = static_pointer_cast<MediaSession>(mSession)->getMediaParams();
			}
		} else {
			participantParams = static_pointer_cast<MediaSession>(mSession)->getCurrentParams();
		}
	} else {
		participantParams = nullptr;
	}
	LinphoneMediaDirection dir = LinphoneMediaDirectionInvalid;

	if (participantParams) {
		switch (type) {
			case LinphoneStreamTypeAudio:
				dir = participantParams->getAudioDirection();
				break;
			case LinphoneStreamTypeVideo:
				dir = participantParams->getVideoDirection();
				break;
			case LinphoneStreamTypeText:
				dir = LinphoneMediaDirectionSendRecv;
				break;
			case LinphoneStreamTypeUnknown:
				break;
		}
	}

	// Current params stores the negotiated media direction from the local standpoint, hence it must be flipped if it is unidirectional
	if (dir == LinphoneMediaDirectionSendOnly) {
		dir = LinphoneMediaDirectionRecvOnly;
	} else if (dir == LinphoneMediaDirectionRecvOnly) {
		dir = LinphoneMediaDirectionSendOnly;
	}

	return dir;
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
	bool mediaCapabilityChanged = false;
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
		if (isMe) {
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

			textDir = LinphoneMediaDirectionSendRecv;
		} else if (mSession) {
			// A conference server is a passive element, therefore it may happen that the negotiated capabilities are different than those the client requested.
			// For example if the video will be inactive if all clients are RecvOnly and their layout is Grid but as soon as one toggles the video direction to SendRecv, then everybody should be able to get stream in
			auto mMediaSession = static_pointer_cast<MediaSession>(mSession);
			const MediaSessionParams* participantParams = (mSession->getPrivate()->isInConference()) ? mMediaSession->getRemoteParams() : mMediaSession->getMediaParams();
			if (participantParams) {
				audioEnabled = participantParams->audioEnabled();
				videoEnabled = participantParams->videoEnabled();
				textEnabled = participantParams->realtimeTextEnabled();
				audioDir = getStreamDirectionFromSession(LinphoneStreamTypeAudio);
				videoDir = getStreamDirectionFromSession(LinphoneStreamTypeVideo);
				textDir = getStreamDirectionFromSession(LinphoneStreamTypeText);
			}

			if (mSession->getPrivate()->isInConference()) {
				const auto audioSsrc = mMediaSession->getSsrc(LinphoneStreamTypeAudio);
				mediaCapabilityChanged |= setSsrc(LinphoneStreamTypeAudio, audioSsrc);

				const auto videoSsrc = mMediaSession->getSsrc(LinphoneStreamTypeVideo);
				mediaCapabilityChanged |= setSsrc(LinphoneStreamTypeVideo, videoSsrc);
			}
		}
		mediaCapabilityChanged |= setStreamCapability(computeDeviceMediaDirection(conferenceAudioEnabled, audioEnabled, audioDir), LinphoneStreamTypeAudio);
		mediaCapabilityChanged |= setStreamCapability(computeDeviceMediaDirection(conferenceVideoEnabled, videoEnabled, videoDir), LinphoneStreamTypeVideo);
		mediaCapabilityChanged |= setStreamCapability(computeDeviceMediaDirection(conferenceTextEnabled, textEnabled, textDir), LinphoneStreamTypeText);
	} else {
		mediaCapabilityChanged |= setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeAudio);
		mediaCapabilityChanged |= setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeVideo);
		mediaCapabilityChanged |= setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeText);
		mediaCapabilityChanged |= setSsrc(LinphoneStreamTypeAudio, 0);
		mediaCapabilityChanged |= setSsrc(LinphoneStreamTypeVideo, 0);
	}

	return mediaCapabilityChanged;
}

bool ParticipantDevice::computeStreamAvailable(const bool conferenceEnable, const bool callEnable, const LinphoneMediaDirection dir) const {
	const auto & resultDir = computeDeviceMediaDirection(conferenceEnable, callEnable, dir);
	return ((resultDir == LinphoneMediaDirectionSendOnly) || (resultDir == LinphoneMediaDirectionSendRecv));
}

bool ParticipantDevice::updateStreamAvailabilities() {
	const auto & conference = getConference();
	auto streamAvailabilityChanged = false;

	const auto session = getSession() ? getSession() : (conference ? conference->getMainSession() : nullptr);
	if (conference) {
		const auto & conferenceParams = conference->getCurrentParams();
		const auto & conferenceAudioEnabled = conferenceParams.audioEnabled();
		const auto & conferenceVideoEnabled = conferenceParams.videoEnabled();
		const auto & conferenceTextEnabled = conferenceParams.chatEnabled();
		if (session) {
			if (session->getPrivate()->isInConference() && conferenceParams.localParticipantEnabled()) {
				if (conference->isMe(getAddress())) {
					streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceAudioEnabled, conferenceAudioEnabled, getStreamCapability(LinphoneStreamTypeAudio)), LinphoneStreamTypeAudio);
					streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceVideoEnabled, conferenceVideoEnabled, getStreamCapability(LinphoneStreamTypeVideo)), LinphoneStreamTypeVideo);
					streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceTextEnabled, conferenceTextEnabled, getStreamCapability(LinphoneStreamTypeText)), LinphoneStreamTypeText);
				} else  {
					std::shared_ptr<ParticipantDevice> meDev = nullptr;
					if (conferenceParams.getAccount()) {
						char * devAddrStr = linphone_account_get_contact_address(conferenceParams.getAccount()) ? linphone_address_as_string(linphone_account_get_contact_address(conferenceParams.getAccount())) : nullptr;
						if (devAddrStr) {
							Address devAddr(devAddrStr);
							ms_free(devAddrStr);
							meDev = conference->getMe()->findDevice(devAddr);
						}
					}
					auto audioEnabled = (meDev) ? meDev->getStreamAvailability(LinphoneStreamTypeAudio) : conferenceAudioEnabled;
					auto videoEnabled = (meDev) ? meDev->getStreamAvailability(LinphoneStreamTypeVideo) : conferenceVideoEnabled;
					auto textEnabled = (meDev) ? meDev->getStreamAvailability(LinphoneStreamTypeText) : conferenceTextEnabled;
					streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceAudioEnabled, audioEnabled, getStreamCapability(LinphoneStreamTypeAudio)), LinphoneStreamTypeAudio);
					streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceVideoEnabled, videoEnabled, getStreamCapability(LinphoneStreamTypeVideo)), LinphoneStreamTypeVideo);
					streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceTextEnabled, textEnabled, getStreamCapability(LinphoneStreamTypeText)), LinphoneStreamTypeText);
				}
			} else  {
				// A conference server is a passive element, therefore it may happen that the negotiated capabilities are different than those the client requested.
				// For example if the video will be inactive if all clients are RecvOnly and their layout is Grid but as soon as one toggles the video direction to SendRecv, then everybody should be able to get stream in
				const MediaSessionParams* params = (session->getPrivate()->isInConference()) ? static_pointer_cast<MediaSession>(session)->getRemoteParams() : static_pointer_cast<MediaSession>(session)->getMediaParams();

				auto audioEnabled = false;
				auto videoEnabled = false;
				auto textEnabled = false;
				if (params) {
					audioEnabled = params->audioEnabled();
					videoEnabled = params->videoEnabled();
					textEnabled = params->realtimeTextEnabled();
				}

				streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceAudioEnabled, audioEnabled, getStreamCapability(LinphoneStreamTypeAudio)), LinphoneStreamTypeAudio);
				streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceVideoEnabled, videoEnabled, getStreamCapability(LinphoneStreamTypeVideo)), LinphoneStreamTypeVideo);
				streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceTextEnabled, textEnabled, getStreamCapability(LinphoneStreamTypeText)), LinphoneStreamTypeText);
			}
		} else if (conference->isMe(getAddress()) && conferenceParams.localParticipantEnabled()) {
				streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceAudioEnabled, conferenceAudioEnabled, getStreamCapability(LinphoneStreamTypeAudio)), LinphoneStreamTypeAudio);
				streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceVideoEnabled, conferenceVideoEnabled, getStreamCapability(LinphoneStreamTypeVideo)), LinphoneStreamTypeVideo);
				streamAvailabilityChanged |= setStreamAvailability(computeStreamAvailable(conferenceTextEnabled, conferenceTextEnabled, getStreamCapability(LinphoneStreamTypeText)), LinphoneStreamTypeText);
		}
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
	void * windowId = nullptr;
#ifdef VIDEO_ENABLED
	const auto & conference = getConference();
	const auto session = getSession() ? getSession() : (conference ? conference->getMainSession() : nullptr);
	if (!mLabel.empty() && session) {
		windowId = static_pointer_cast<MediaSession>(session)->createNativeVideoWindowId(mLabel);
	} else {
		lError() << "Unable to create a window ID for device " << getAddress() << " because either label is empty (actual " << (mLabel.empty() ? "<not-defined>" : mLabel) << ") or no session is linked to this device (actual " << session << ")";
	}
#endif
	return windowId;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void ParticipantDevice::setWindowId(void * newWindowId) const {
#ifdef VIDEO_ENABLED
	mWindowId = newWindowId;
	const auto & conference = getConference();
	const auto session = getSession() ? getSession() : (conference ? conference->getMainSession() : nullptr);
	if (!mLabel.empty() && session) {
		static_pointer_cast<MediaSession>(session)->setNativeVideoWindowId(mWindowId, mLabel);
	} else {
		lError() << "Unable to set window ID for device " << getAddress() << " because either label is empty (actual " << (mLabel.empty() ? "<not-defined>" : mLabel) << ") or no session is linked to this device (actual " << session << ")";
	}
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

void * ParticipantDevice::getWindowId() const {
	return mWindowId;
}

void ParticipantDevice::setIsSpeaking(bool isSpeaking) {
	mIsSpeaking = isSpeaking;
}

bool ParticipantDevice::getIsSpeaking() const {
	return mIsSpeaking;
}

void ParticipantDevice::setIsMuted(bool isMuted) {
	mIsMuted = isMuted;
}

bool ParticipantDevice::getIsMuted() const {
	return mIsMuted;
}

void ParticipantDevice::setDisconnectionData(bool initiated, int code, LinphoneReason reason) {
	mTimeOfDisconnection = ms_time(NULL);
	if(reason == LinphoneReasonNone) {
		mDisconnectionMethod = initiated ? DisconnectionMethod::Booted : DisconnectionMethod::Departed;
		mDisconnectionReason = std::string();
	} else {
		if (reason == LinphoneReasonBusy) {
			mDisconnectionMethod = DisconnectionMethod::Busy;
		} else {
			mDisconnectionMethod = DisconnectionMethod::Failed;
		}
		mDisconnectionReason = std::string("Reason: SIP;cause=") + std::to_string(code) + ";text=" + std::string(linphone_reason_to_string(reason));
	}
}

LinphoneParticipantDeviceCbsIsSpeakingChangedCb ParticipantDeviceCbs::getIsSpeakingChanged()const{
	return mIsSpeakingChangedCb;
}

void ParticipantDeviceCbs::setIsSpeakingChanged(LinphoneParticipantDeviceCbsIsSpeakingChangedCb cb){
	mIsSpeakingChangedCb = cb;
}

LinphoneParticipantDeviceCbsIsMutedCb ParticipantDeviceCbs::getIsMuted()const{
	return mIsMutedCb;
}

void ParticipantDeviceCbs::setIsMuted(LinphoneParticipantDeviceCbsIsMutedCb cb){
	mIsMutedCb = cb;
}

LinphoneParticipantDeviceCbsStateChangedCb ParticipantDeviceCbs::getStateChanged()const {
	return mStateChangedCb;
}

void ParticipantDeviceCbs::setStateChanged(LinphoneParticipantDeviceCbsStateChangedCb cb){
	mStateChangedCb = cb;
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
