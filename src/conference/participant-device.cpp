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

#include "participant-device.h"
#include "chat/encryption/encryption-engine.h"
#include "conference/conference.h"
#include "conference/params/media-session-params-p.h"
#include "conference/params/media-session-params.h"
#include "conference/session/call-session-p.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "core/core.h"
#include "participant.h"
#include "private_functions.h"

#include "linphone/api/c-event.h"

using namespace std;

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Core;

// =============================================================================

ParticipantDevice::ParticipantDevice() {
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeAudio);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeVideo);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeText);
}

ParticipantDevice::ParticipantDevice(std::shared_ptr<Participant> participant,
                                     const std::shared_ptr<CallSession> &session,
                                     const std::string &name)
    : mParticipant(participant), mGruu(Address::create(participant->getAddress()->getUri())), mName(name),
      mSession(session) {
	if (mSession && mSession->getRemoteContactAddress()) {
		setAddress(mSession->getRemoteContactAddress());
	}
	updateMediaCapabilities();
	updateStreamAvailabilities();
}

ParticipantDevice::ParticipantDevice(std::shared_ptr<Participant> participant,
                                     const std::shared_ptr<const Address> &gruu,
                                     const std::string &name)
    : mParticipant(participant), mGruu(Address::create(gruu->getUri())), mName(name) {
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeAudio);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeVideo);
	setStreamCapability(LinphoneMediaDirectionInactive, LinphoneStreamTypeText);
}

ParticipantDevice::~ParticipantDevice() {
	if (mConferenceSubscribeEvent) mConferenceSubscribeEvent->unref();
}

bool ParticipantDevice::operator==(const ParticipantDevice &device) const {
	return (*getAddress() == *device.getAddress());
}

std::shared_ptr<Conference> ParticipantDevice::getConference() const {
	return getParticipant() ? getParticipant()->getConference() : nullptr;
}

shared_ptr<Core> ParticipantDevice::getCore() const {
	return getParticipant() ? getParticipant()->getCore() : nullptr;
}

std::shared_ptr<Address> ParticipantDevice::getAddress() const {
	return mGruu;
}

void ParticipantDevice::setAddress(const std::shared_ptr<Address> &address) {
	if (mGruu) {
		auto conference = getConference();
		lInfo() << "Changing address of " << *this << " in " << *conference << " from " << *mGruu << " to " << *address;
	}
	mGruu = Address::create(address->getUri());
	if (address->hasParam("+org.linphone.specs")) {
		const auto &linphoneSpecs = address->getParamValue("+org.linphone.specs");
		setCapabilityDescriptor(linphoneSpecs.substr(1, linphoneSpecs.size() - 2));
	}
}

std::shared_ptr<Participant> ParticipantDevice::getParticipant() const {
	if (mParticipant.expired()) {
		lWarning() << "The participant owning " << *this << " has already been deleted";
	}
	shared_ptr<Participant> participant = mParticipant.lock();
	if (!participant) {
		lWarning() << "Unable to get the participant owning the " << *this;
		return nullptr;
	}
	return participant;
}

void ParticipantDevice::clearChangingSubscribeEvent() {
	mChangingSubscribeEvent = false;
}

bool ParticipantDevice::isChangingSubscribeEvent() const {
	return mChangingSubscribeEvent;
}

void ParticipantDevice::setConferenceSubscribeEvent(const shared_ptr<EventSubscribe> &ev) {
	if (ev) ev->ref();
	if (mConferenceSubscribeEvent) {
		mChangingSubscribeEvent = true;
		mConferenceSubscribeEvent->unref();
		mConferenceSubscribeEvent = nullptr;
	}
	mConferenceSubscribeEvent = ev;
}

AbstractChatRoom::SecurityLevel ParticipantDevice::getSecurityLevel() const {
	auto encryptionEngine = getCore()->getEncryptionEngine();
	if (encryptionEngine) return encryptionEngine->getSecurityLevel(mGruu->asStringUriOnly());
	lWarning() << "Asking security level of " << *this << " in " << *getConference()
	           << " but there is no encryption engine enabled";
	return AbstractChatRoom::SecurityLevel::ClearText;
}

time_t ParticipantDevice::getTimeOfJoining() const {
	return mTimeOfJoining;
}

time_t ParticipantDevice::getTimeOfDisconnection() const {
	return mTimeOfDisconnection;
}

bool ParticipantDevice::isInConference() const {
	const auto &conference = getConference();
	if (conference) {
		const auto &isMe = conference->isMe(mGruu);
		if (isMe) {
			return conference->isIn();
		} else {
			return (getState() == ParticipantDevice::State::Present);
		}
	}
	return false;
}

bool ParticipantDevice::setSsrc(const LinphoneStreamType type, uint32_t newSsrc) {
	bool changed = false;
	const bool idxFound = (streams.find(type) != streams.cend());
	if (!idxFound || (streams[type].ssrc != newSsrc)) {
		streams[type].ssrc = newSsrc;
		changed = true;
	}
	auto conference = getConference();
	switch (type) {
		case LinphoneStreamTypeAudio:
			if (conference) {
				const auto &pendingParticipantsMutes = conference->getPendingParticipantsMutes();
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
			lInfo() << "Setting " << std::string(linphone_stream_type_to_string(type)) << " ssrc of " << *this << " in "
			        << *conference << " to " << newSsrc;
		} else {
			lInfo() << "Setting " << std::string(linphone_stream_type_to_string(type)) << " ssrc of " << *this << " to "
			        << newSsrc;
		}
	}

	return changed;
}

uint32_t ParticipantDevice::getSsrc(const LinphoneStreamType type) const {
	try {
		return streams.at(type).ssrc;
	} catch (std::out_of_range &) {
		return 0;
	}
}

bool ParticipantDevice::setThumbnailStreamSsrc(uint32_t newSsrc) {
	bool changed = false;
	const auto currentSsrc = thumbnailStream.ssrc;
	if (currentSsrc != newSsrc) {
		thumbnailStream.ssrc = newSsrc;
		changed = true;
	}
	auto conference = getConference();
	if (changed) {
		if (conference) {
			lInfo() << "Setting thumbnail stream ssrc of " << *this << " in " << *conference << " to " << newSsrc;
		} else {
			lInfo() << "Setting thumbnail stream ssrc of " << *this << " to " << newSsrc;
		}
	}
	return changed;
}

uint32_t ParticipantDevice::getThumbnailStreamSsrc() const {
	return thumbnailStream.ssrc;
}

void *ParticipantDevice::getUserData() const {
	return mUserData;
}

void ParticipantDevice::setUserData(void *ud) {
	mUserData = ud;
}

const std::string &ParticipantDevice::getCallId() {
	if (mCallId.empty() && mSession) {
		const auto &log = mSession->getLog();
		mCallId = log->getCallId();
	}

	return mCallId;
}

void ParticipantDevice::setCallId(const std::string &callId) {
	mCallId = callId;
}

const std::string &ParticipantDevice::getFromTag() {
	if (mFromTag.empty() && mSession) {
		mFromTag = mSession->getFromTag();
	}

	return mFromTag;
}

void ParticipantDevice::setFromTag(const std::string &tag) {
	mFromTag = tag;
}

const std::string &ParticipantDevice::getToTag() {
	if (mToTag.empty() && mSession) {
		mToTag = mSession->getToTag();
	}

	return mToTag;
}

void ParticipantDevice::setToTag(const std::string &tag) {
	mToTag = tag;
}

bool ParticipantDevice::isLeavingState(const ParticipantDevice::State &state) {
	switch (state) {
		case ParticipantDevice::State::ScheduledForJoining:
		case ParticipantDevice::State::Joining:
		case ParticipantDevice::State::Alerting:
		case ParticipantDevice::State::Present:
		case ParticipantDevice::State::OnHold:
		case ParticipantDevice::State::RequestingToJoin:
		case ParticipantDevice::State::MutedByFocus:
			return false;
		case ParticipantDevice::State::ScheduledForLeaving:
		case ParticipantDevice::State::Leaving:
		case ParticipantDevice::State::Left:
			return true;
	}
	return false;
}

void ParticipantDevice::setState(State newState, bool notify) {
	if (mState != newState) {
		const auto currentStateLeavingState = ParticipantDevice::isLeavingState(mState);
		const auto newStateLeavingState = ParticipantDevice::isLeavingState(newState);
		// Send NOTIFY only if not transitioning from a leaving state to another one
		const bool sendNotify = !(newStateLeavingState && currentStateLeavingState) && notify;

		if ((newState == ParticipantDevice::State::Present) && (mState != ParticipantDevice::State::OnHold)) {
			setTimeOfJoining(time(nullptr));
		}
		if (getCore() != nullptr && linphone_core_get_global_state(getCore()->getCCore()) !=
		                                LinphoneGlobalStartup) { // When creating participant device from database
			lInfo() << "Moving " << *this << " from state " << mState << " to " << newState;
		}
		mState = newState;
		_linphone_participant_device_notify_state_changed(toC(), (LinphoneParticipantDeviceState)newState);
		const auto &conference = getConference();
		if (conference && sendNotify) {
			conference->notifyParticipantDeviceStateChanged(ms_time(nullptr), false, getParticipant(),
			                                                getSharedFromThis());
		}
	}
}

bool ParticipantDevice::enableScreenSharing(bool enabled) {
	bool changed = (screenSharingEnabled() != enabled);
	if (changed) {
		lInfo() << *this << " " << std::string(enabled ? "starts" : "stops") << " sharing its screen";
		mIsScreenSharing = enabled;
		if (mSession && mSession->getPrivate()->isInConference()) {
			if (enabled) {
				char label[LinphonePrivate::Conference::sLabelLength];
				belle_sip_random_token(label, sizeof(label));
				setStreamLabel(label, LinphoneStreamTypeVideo);
			} else {
				setStreamLabel(getThumbnailStreamLabel(), LinphoneStreamTypeVideo);
			}
		}
		_linphone_participant_device_notify_screen_sharing_enabled(toC(), enabled);
	}
	return changed;
}

bool ParticipantDevice::screenSharingEnabled() const {
	return mIsScreenSharing;
}

ostream &operator<<(ostream &stream, ParticipantDevice::State state) {
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
		case ParticipantDevice::State::RequestingToJoin:
			return stream << "RequestingToJoin";
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

void ParticipantDevice::setCapabilityDescriptor(const std::string &capabilities) {
	mCapabilityDescriptor = capabilities;
}

void ParticipantDevice::setSession(std::shared_ptr<CallSession> session) {
	lInfo() << "Assigning session " << session << " to " << *this << " in " << *getConference();
	mSession = session;
}

const std::string &ParticipantDevice::getStreamLabel(const LinphoneStreamType type) const {
	try {
		return streams.at(type).label;
	} catch (std::out_of_range &) {
		return Utils::getEmptyConstRefObject<string>();
	}
}

bool ParticipantDevice::setStreamLabel(const std::string &streamLabel, const LinphoneStreamType type) {
	const bool idxFound = (streams.find(type) != streams.cend());
	if (!idxFound || (streams[type].label != streamLabel)) {
		auto conference = getConference();
		lInfo() << "Setting label of " << std::string(linphone_stream_type_to_string(type)) << " stream of " << *this
		        << " in " << *conference << " to " << streamLabel;
		streams[type].label = streamLabel;
		return true;
	}
	return false;
}

const std::string &ParticipantDevice::getThumbnailStreamLabel() const {
	return thumbnailStream.label;
}

bool ParticipantDevice::setThumbnailStreamLabel(const std::string &streamLabel) {
	if (thumbnailStream.label != streamLabel) {
		auto conference = getConference();
		lInfo() << "Setting label of the thumbnail stream of " << *this << " in " << *conference << " to "
		        << streamLabel;
		thumbnailStream.label = streamLabel;
		return true;
	}
	return false;
}

LinphoneMediaDirection ParticipantDevice::getStreamCapability(const LinphoneStreamType type) const {
	try {
		return streams.at(type).direction;
	} catch (std::out_of_range &) {
		return LinphoneMediaDirectionInactive;
	}
}

bool ParticipantDevice::setStreamCapability(const LinphoneMediaDirection &direction, const LinphoneStreamType type) {
	const bool idxFound = (streams.find(type) != streams.cend());
	if (!idxFound || (streams[type].direction != direction)) {
		streams[type].direction = direction;
		_linphone_participant_device_notify_stream_capability_changed(toC(), direction, type);
		return true;
	}
	return false;
}

LinphoneMediaDirection ParticipantDevice::getThumbnailStreamCapability() const {
	return thumbnailStream.direction;
}

bool ParticipantDevice::setThumbnailStreamCapability(const LinphoneMediaDirection &direction) {
	if (thumbnailStream.direction != direction) {
		thumbnailStream.direction = direction;
		_linphone_participant_device_notify_thumbnail_stream_capability_changed(toC(), direction);
		return true;
	}
	return false;
}

LinphoneMediaDirection ParticipantDevice::getStreamDirectionFromSession(const LinphoneStreamType type) const {
	const auto &state = mSession->getState();
	const auto &sessionNotEstablished =
	    (state == CallSession::State::Idle) || (state == CallSession::State::IncomingReceived) ||
	    (state == CallSession::State::OutgoingInit) || (state == CallSession::State::OutgoingProgress) ||
	    (state == CallSession::State::OutgoingRinging) || (state == CallSession::State::OutgoingEarlyMedia) ||
	    (state == CallSession::State::PushIncomingReceived);

	const MediaSessionParams *participantParams = nullptr;
	auto isInLocalConference = false;
	if (mSession) {
		isInLocalConference = mSession->getPrivate()->isInConference();
		const auto mMediaSession = dynamic_pointer_cast<MediaSession>(mSession);
		if (mMediaSession && sessionNotEstablished) {
			if (mSession->getPrivate()->isInConference()) {
				participantParams = mMediaSession->getRemoteParams();
			} else {
				participantParams = mMediaSession->getMediaParams();
			}
		} else {
			participantParams = mMediaSession->getCurrentParams();
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

	// Current params store the negotiated media direction from the local standpoint, hence it must be flipped if it is
	// unidirectional.
	if (isInLocalConference) {
		if (dir == LinphoneMediaDirectionSendOnly) {
			dir = LinphoneMediaDirectionRecvOnly;
		} else if (dir == LinphoneMediaDirectionRecvOnly) {
			dir = LinphoneMediaDirectionSendOnly;
		}
	}

	return dir;
}

bool ParticipantDevice::getStreamAvailability(const LinphoneStreamType type) const {
	try {
		return streams.at(type).available;
	} catch (std::out_of_range &) {
		return false;
	}
}

bool ParticipantDevice::setStreamAvailability(const bool available, const LinphoneStreamType type) {
	const bool idxFound = (streams.find(type) != streams.cend());
	if (!idxFound || (streams[type].available != available)) {
		streams[type].available = available;
		_linphone_participant_device_notify_stream_availability_changed(toC(), (available ? TRUE : FALSE), type);
		return true;
	}
	return false;
}

bool ParticipantDevice::getThumbnailStreamAvailability() const {
	return thumbnailStream.available;
}

bool ParticipantDevice::setThumbnailStreamAvailability(const bool available) {
	if (thumbnailStream.available != available) {
		thumbnailStream.available = available;
		_linphone_participant_device_notify_thumbnail_stream_availability_changed(toC(), (available ? TRUE : FALSE));
		return true;
	}
	return false;
}

LinphoneMediaDirection ParticipantDevice::computeDeviceMediaDirection(const bool conferenceEnable,
                                                                      const bool callEnable,
                                                                      const LinphoneMediaDirection dir) const {
	if (conferenceEnable && callEnable) {
		return dir;
	}

	return LinphoneMediaDirectionInactive;
}

std::set<LinphoneStreamType> ParticipantDevice::updateMediaCapabilities() {
	std::set<LinphoneStreamType> mediaCapabilityChanged;
	const auto &conference = getConference();

	bool updateSsrc = true;
	uint32_t audioSsrc = 0;
	uint32_t videoSsrc = 0;
	uint32_t thumbnailSsrc = 0;
	uint32_t textSsrc = 0;
	auto resultAudioDir = LinphoneMediaDirectionInactive;
	auto resultVideoDir = LinphoneMediaDirectionInactive;
	auto resultThumbnailDir = LinphoneMediaDirectionInactive;
	auto resultTextDir = LinphoneMediaDirectionInactive;

	if (conference) {
		const auto isMe = conference->isMe(mGruu);
		const auto &conferenceParams = conference->getCurrentParams();
		const auto &conferenceAudioEnabled = conferenceParams->audioEnabled();
		const auto &conferenceVideoEnabled = conferenceParams->videoEnabled();
		const auto &conferenceTextEnabled = conferenceParams->chatEnabled();
		auto audioEnabled = false;
		auto videoEnabled = false;
		auto textEnabled = false;
		auto audioDir = LinphoneMediaDirectionInactive;
		auto videoDir = LinphoneMediaDirectionInactive;
		auto thumbnailDir = LinphoneMediaDirectionInactive;
		auto textDir = LinphoneMediaDirectionInactive;
		if (isMe) {
			const auto &cCore = getCore()->getCCore();
			audioEnabled = true;
			videoEnabled = linphone_core_video_enabled(cCore);
			textEnabled = true;
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
			if ((videoDir == LinphoneMediaDirectionSendOnly) || (videoDir == LinphoneMediaDirectionSendRecv)) {
				thumbnailDir = LinphoneMediaDirectionSendOnly;
			} else {
				thumbnailDir = LinphoneMediaDirectionInactive;
			}

			textDir = LinphoneMediaDirectionSendRecv;
			updateSsrc = false;
		} else if (mSession) {
			// A conference server is a passive element, therefore it may happen that the negotiated capabilities are
			// different than those the client requested. For example if the video will be inactive if all clients are
			// RecvOnly and their layout is Grid but as soon as one toggles the video direction to SendRecv, then
			// everybody should be able to get stream in
			auto mMediaSession = dynamic_pointer_cast<MediaSession>(mSession);
			if (mMediaSession) {
				bool isInConference = mSession->getPrivate()->isInConference();
				const MediaSessionParams *participantParams =
				    (isInConference) ? mMediaSession->getRemoteParams() : mMediaSession->getMediaParams();
				if (participantParams) {
					audioEnabled = participantParams->audioEnabled();
					videoEnabled = participantParams->videoEnabled();
					textEnabled = participantParams->realtimeTextEnabled();
					audioDir = getStreamDirectionFromSession(LinphoneStreamTypeAudio);
					videoDir = getStreamDirectionFromSession(LinphoneStreamTypeVideo);
					textDir = getStreamDirectionFromSession(LinphoneStreamTypeText);
					thumbnailDir = mMediaSession->getDirectionOfStream(
					    MediaSessionPrivate::ThumbnailVideoContentAttribute, getThumbnailStreamLabel());
				}

				updateSsrc = isInConference;
				if (isInConference) {
					audioSsrc = mMediaSession->getSsrc(LinphoneStreamTypeAudio);
					videoSsrc = mMediaSession->getSsrc(LinphoneStreamTypeVideo);
					thumbnailSsrc = mMediaSession->getSsrc(MediaSessionPrivate::ThumbnailVideoContentAttribute);
					textSsrc = mMediaSession->getSsrc(LinphoneStreamTypeText);
				}
			}
		}
		resultAudioDir = computeDeviceMediaDirection(conferenceAudioEnabled, audioEnabled, audioDir);
		resultVideoDir = computeDeviceMediaDirection(conferenceVideoEnabled, videoEnabled, videoDir);
		resultThumbnailDir = computeDeviceMediaDirection(conferenceVideoEnabled, videoEnabled, thumbnailDir);
		resultTextDir = computeDeviceMediaDirection(conferenceTextEnabled, textEnabled, textDir);
	}

	if (setStreamCapability(resultAudioDir, LinphoneStreamTypeAudio)) {
		mediaCapabilityChanged.insert(LinphoneStreamTypeAudio);
	}
	if (setStreamCapability(resultVideoDir, LinphoneStreamTypeVideo)) {
		mediaCapabilityChanged.insert(LinphoneStreamTypeVideo);
	}
	if (setThumbnailStreamCapability(resultThumbnailDir)) {
		mediaCapabilityChanged.insert(LinphoneStreamTypeVideo);
	}
	if (setStreamCapability(resultTextDir, LinphoneStreamTypeText)) {
		mediaCapabilityChanged.insert(LinphoneStreamTypeText);
	}

	if (updateSsrc) {
		bool updateAudioSsrc = ((resultAudioDir != LinphoneMediaDirectionInactive) || (audioSsrc == 0));
		if (setSsrc(LinphoneStreamTypeAudio, audioSsrc) && updateAudioSsrc) {
			mediaCapabilityChanged.insert(LinphoneStreamTypeAudio);
		}

		bool updateVideoSsrc = ((resultVideoDir != LinphoneMediaDirectionInactive) || (videoSsrc == 0));
		if (setSsrc(LinphoneStreamTypeVideo, videoSsrc) && updateVideoSsrc) {
			mediaCapabilityChanged.insert(LinphoneStreamTypeVideo);
		}

		bool updateThumbnailSsrc = (resultThumbnailDir != LinphoneMediaDirectionInactive);
		if (setThumbnailStreamSsrc(thumbnailSsrc) && updateThumbnailSsrc) {
			mediaCapabilityChanged.insert(LinphoneStreamTypeVideo);
		}

		bool updateTextSsrc = ((resultTextDir != LinphoneMediaDirectionInactive) || (textSsrc == 0));
		if (setSsrc(LinphoneStreamTypeText, textSsrc) && updateTextSsrc) {
			mediaCapabilityChanged.insert(LinphoneStreamTypeText);
		}
	}

	return mediaCapabilityChanged;
}

bool ParticipantDevice::computeStreamAvailable(const bool conferenceEnable,
                                               const bool callEnable,
                                               const LinphoneMediaDirection dir) const {
	const auto &resultDir = computeDeviceMediaDirection(conferenceEnable, callEnable, dir);
	return ((resultDir == LinphoneMediaDirectionSendOnly) || (resultDir == LinphoneMediaDirectionSendRecv));
}

std::set<LinphoneStreamType> ParticipantDevice::updateStreamAvailabilities() {
	const auto &conference = getConference();
	std::set<LinphoneStreamType> streamAvailabilityChanged;

	if (conference) {
		const auto isMe = conference->isMe(mGruu);
		const auto &conferenceParams = conference->getCurrentParams();
		const auto &conferenceAudioEnabled = conferenceParams->audioEnabled();
		const auto &conferenceVideoEnabled = conferenceParams->videoEnabled();
		const auto &conferenceTextEnabled = conferenceParams->chatEnabled();
		const auto session = getSession() ? getSession() : (conference ? conference->getMainSession() : nullptr);
		if (session) {
			auto audioEnabled = false;
			auto videoEnabled = false;
			auto textEnabled = false;
			auto mMediaSession = dynamic_pointer_cast<MediaSession>(session);
			const auto isInLocalConference = session->getPrivate()->isInConference();
			if (isInLocalConference && conferenceParams->localParticipantEnabled()) {
				if (isMe) {
					audioEnabled = conferenceAudioEnabled;
					videoEnabled = conferenceVideoEnabled;
					textEnabled = conferenceTextEnabled;
				} else {
					std::shared_ptr<ParticipantDevice> meDev = nullptr;
					if (conferenceParams->getAccount()) {
						const auto &devAddr = conferenceParams->getAccount()->getContactAddress();
						if (devAddr) {
							meDev = conference->getMe()->findDevice(devAddr);
						}
					}
					audioEnabled =
					    (meDev) ? meDev->getStreamAvailability(LinphoneStreamTypeAudio) : conferenceAudioEnabled;
					videoEnabled =
					    (meDev) ? meDev->getStreamAvailability(LinphoneStreamTypeVideo) : conferenceVideoEnabled;
					textEnabled =
					    (meDev) ? meDev->getStreamAvailability(LinphoneStreamTypeText) : conferenceTextEnabled;
				}
			} else {
				// A conference server is a passive element, therefore it may happen that the negotiated capabilities
				// are different than those the client requested. For example the video will be inactive if all
				// clients are RecvOnly and their layout is Grid but as soon as one toggles the video direction to
				// SendRecv, then everybody should be able to get stream in
				if (mMediaSession) {
					const MediaSessionParams *params =
					    (isInLocalConference) ? mMediaSession->getRemoteParams() : mMediaSession->getMediaParams();
					if (params) {
						audioEnabled = params->audioEnabled();
						videoEnabled = params->videoEnabled();
						textEnabled = params->realtimeTextEnabled();
					}
				}
			}
			if (setStreamAvailability(computeStreamAvailable(conferenceAudioEnabled, audioEnabled,
			                                                 getStreamCapability(LinphoneStreamTypeAudio)),
			                          LinphoneStreamTypeAudio)) {
				streamAvailabilityChanged.insert(LinphoneStreamTypeAudio);
			}

			// Video is enabled if the thumbnail can be request or the participant is screensharing
			if (setStreamAvailability(computeStreamAvailable(conferenceVideoEnabled, videoEnabled,
			                                                 getStreamCapability(LinphoneStreamTypeVideo)),
			                          LinphoneStreamTypeVideo)) {
				streamAvailabilityChanged.insert(LinphoneStreamTypeVideo);
			}
			bool thumbnailRequested = conference->areThumbnailsRequested(false);
			// If RTP bundle is disabled in client conference session, then the thumbnail stream is not available for
			// the participants
			bool videoRequested = (thumbnailRequested ||
			                       (isMe && (isInLocalConference ||
			                                 (mMediaSession && mMediaSession->getMediaParams()->rtpBundleEnabled()))));
			if (setThumbnailStreamAvailability(
			        videoRequested &&
			        computeStreamAvailable(conferenceVideoEnabled, videoEnabled, getThumbnailStreamCapability()))) {
				streamAvailabilityChanged.insert(LinphoneStreamTypeVideo);
			}
			if (setStreamAvailability(computeStreamAvailable(conferenceTextEnabled, textEnabled,
			                                                 getStreamCapability(LinphoneStreamTypeText)),
			                          LinphoneStreamTypeText)) {
				streamAvailabilityChanged.insert(LinphoneStreamTypeText);
			}
		} else if (isMe && conferenceParams->localParticipantEnabled()) {
			if (setStreamAvailability(computeStreamAvailable(conferenceAudioEnabled, conferenceAudioEnabled,
			                                                 getStreamCapability(LinphoneStreamTypeAudio)),
			                          LinphoneStreamTypeAudio)) {
				streamAvailabilityChanged.insert(LinphoneStreamTypeAudio);
			}
			if (setStreamAvailability(computeStreamAvailable(conferenceVideoEnabled, conferenceVideoEnabled,
			                                                 getStreamCapability(LinphoneStreamTypeVideo)),
			                          LinphoneStreamTypeVideo)) {
				streamAvailabilityChanged.insert(LinphoneStreamTypeVideo);
			}
			if (setThumbnailStreamAvailability(computeStreamAvailable(conferenceVideoEnabled, conferenceVideoEnabled,
			                                                          getThumbnailStreamCapability()))) {
				streamAvailabilityChanged.insert(LinphoneStreamTypeVideo);
			}
			if (setStreamAvailability(computeStreamAvailable(conferenceTextEnabled, conferenceTextEnabled,
			                                                 getStreamCapability(LinphoneStreamTypeText)),
			                          LinphoneStreamTypeText)) {
				streamAvailabilityChanged.insert(LinphoneStreamTypeText);
			}
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

void ParticipantDevice::videoDisplayErrorOccurred(int error_code) {
	_linphone_participant_device_notify_video_display_error_occurred(toC(), error_code);
}

void *ParticipantDevice::createWindowId() {
	void *windowId = nullptr;
#ifdef VIDEO_ENABLED
	const auto &conference = getConference();
	const auto session = getSession() ? getSession() : (conference ? conference->getMainSession() : nullptr);
	if (session) {
		auto s = static_pointer_cast<MediaSession>(session);
		const auto &label = (s->requestThumbnail(getSharedFromThis())) ? getThumbnailStreamLabel()
		                                                               : getStreamLabel(LinphoneStreamTypeVideo);
		// Empty label is used only for main stream which is handled by the call.
		if (label.empty() || !mGruu) {
			lError() << "Unable to create a window ID for " << *this << " because no label is associated to it";
		} else {
			windowId = static_pointer_cast<MediaSession>(session)->createNativeVideoWindowId(
			    label, conference->isMe(mGruu), true);
		}
	} else {
		lError() << "Unable to create a window ID for " << *this << " because no session is linked to this device";
	}
#endif
	return windowId;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void ParticipantDevice::setWindowId(void *newWindowId) {
#ifdef VIDEO_ENABLED
	mWindowId = newWindowId;
	const auto &conference = getConference();
	const auto session = getSession() ? getSession() : (conference ? conference->getMainSession() : nullptr);
	if (session) {
		auto s = static_pointer_cast<MediaSession>(session);
		const auto &label = (s->requestThumbnail(getSharedFromThis())) ? getThumbnailStreamLabel()
		                                                               : getStreamLabel(LinphoneStreamTypeVideo);
		// Empty label is used only for main stream which is handled by the call.
		if (label.empty() || !mGruu) {
			lError() << "Unable to set a window ID for device " << *this << " because no label is associated to it";
		} else {
			const auto isMe = conference->isMe(mGruu);
			if (isMe) {
				linphone_core_set_native_preview_window_id(getCore()->getCCore(), mWindowId);
			} else {
				s->setNativeVideoWindowId(mWindowId, label, isMe, true);
			}
		}
	} else {
		lError() << "Unable to set a window ID for device " << *this << " because no session is linked to this device";
	}
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

void *ParticipantDevice::getWindowId() const {
	return mWindowId;
}

void ParticipantDevice::setSendAddedNotify(bool sendNotify) {
	mSendAddedNotify = sendNotify;
}

bool ParticipantDevice::addedNotifySent() const {
	return mSendAddedNotify;
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
	if (reason == LinphoneReasonNone) {
		mDisconnectionMethod = initiated ? DisconnectionMethod::Booted : DisconnectionMethod::Departed;
		mDisconnectionReason = std::string();
	} else {
		if (reason == LinphoneReasonBusy) {
			mDisconnectionMethod = DisconnectionMethod::Busy;
		} else {
			mDisconnectionMethod = DisconnectionMethod::Failed;
		}
		mDisconnectionReason = std::string("Reason: SIP;cause=") + std::to_string(code) +
		                       ";text=" + std::string(linphone_reason_to_string(reason));
	}
}

LinphoneParticipantDeviceCbsIsSpeakingChangedCb ParticipantDeviceCbs::getIsSpeakingChanged() const {
	return mIsSpeakingChangedCb;
}

void ParticipantDeviceCbs::setIsSpeakingChanged(LinphoneParticipantDeviceCbsIsSpeakingChangedCb cb) {
	mIsSpeakingChangedCb = cb;
}

LinphoneParticipantDeviceCbsIsMutedCb ParticipantDeviceCbs::getIsMuted() const {
	return mIsMutedCb;
}

void ParticipantDeviceCbs::setIsMuted(LinphoneParticipantDeviceCbsIsMutedCb cb) {
	mIsMutedCb = cb;
}
LinphoneParticipantDeviceCbsScreenSharingChangedCb ParticipantDeviceCbs::getScreenSharingChanged() const {
	return mScreenSharingCb;
}

void ParticipantDeviceCbs::setScreenSharingChanged(LinphoneParticipantDeviceCbsScreenSharingChangedCb cb) {
	mScreenSharingCb = cb;
}

LinphoneParticipantDeviceCbsStateChangedCb ParticipantDeviceCbs::getStateChanged() const {
	return mStateChangedCb;
}

void ParticipantDeviceCbs::setStateChanged(LinphoneParticipantDeviceCbsStateChangedCb cb) {
	mStateChangedCb = cb;
}

LinphoneParticipantDeviceCbsStreamCapabilityChangedCb ParticipantDeviceCbs::getStreamCapabilityChanged() const {
	return mStreamCapabilityChangedCb;
}

void ParticipantDeviceCbs::setStreamCapabilityChanged(LinphoneParticipantDeviceCbsStreamCapabilityChangedCb cb) {
	mStreamCapabilityChangedCb = cb;
}

LinphoneParticipantDeviceCbsThumbnailStreamCapabilityChangedCb
ParticipantDeviceCbs::getThumbnailStreamCapabilityChanged() const {
	return mThumbnailStreamCapabilityChangedCb;
}

void ParticipantDeviceCbs::setThumbnailStreamCapabilityChanged(
    LinphoneParticipantDeviceCbsThumbnailStreamCapabilityChangedCb cb) {
	mThumbnailStreamCapabilityChangedCb = cb;
}

LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb ParticipantDeviceCbs::getStreamAvailabilityChanged() const {
	return mStreamAvailabilityChangedCb;
}

void ParticipantDeviceCbs::setStreamAvailabilityChanged(LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb cb) {
	mStreamAvailabilityChangedCb = cb;
}

LinphoneParticipantDeviceCbsThumbnailStreamAvailabilityChangedCb
ParticipantDeviceCbs::getThumbnailStreamAvailabilityChanged() const {
	return mThumbnailStreamAvailabilityChangedCb;
}

void ParticipantDeviceCbs::setThumbnailStreamAvailabilityChanged(
    LinphoneParticipantDeviceCbsThumbnailStreamAvailabilityChangedCb cb) {
	mThumbnailStreamAvailabilityChangedCb = cb;
}

LinphoneParticipantDeviceCbsVideoDisplayErrorOccurredCb ParticipantDeviceCbs::getVideoDisplayErrorOccurred() const {
	return mVideoDisplayErrorOccurredCb;
}

void ParticipantDeviceCbs::setVideoDisplayErrorOccurred(LinphoneParticipantDeviceCbsVideoDisplayErrorOccurredCb cb) {
	mVideoDisplayErrorOccurredCb = cb;
}

LINPHONE_END_NAMESPACE
