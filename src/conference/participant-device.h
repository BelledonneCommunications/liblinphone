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

#ifndef _L_PARTICIPANT_DEVICE_H_
#define _L_PARTICIPANT_DEVICE_H_

#include <string>
#include <ctime>

#include <belle-sip/object++.hh>

#include "conference/conference-enums.h"

#include "address/identity-address.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "chat/encryption/encryption-engine.h"

#include "linphone/types.h"
#include "linphone/utils/general.h"

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-sal.h"
#include "linphone/api/c-types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSession;
class Core;
class Participant;
class ParticipantDeviceCbs;

class ParticipantDevice : public bellesip::HybridObject<LinphoneParticipantDevice, ParticipantDevice>, public UserDataAccessor, public CallbacksHolder<ParticipantDeviceCbs> {
public:
	enum class State {
		Joining = LinphoneParticipantDeviceStateJoining,
		Present = LinphoneParticipantDeviceStatePresent,
		Leaving = LinphoneParticipantDeviceStateLeaving,
		Left = LinphoneParticipantDeviceStateLeft,
		ScheduledForJoining = LinphoneParticipantDeviceStateScheduledForJoining,
		ScheduledForLeaving = LinphoneParticipantDeviceStateScheduledForLeaving,
		OnHold = LinphoneParticipantDeviceStateOnHold,
		Alerting = LinphoneParticipantDeviceStateAlerting,
		MutedByFocus = LinphoneParticipantDeviceStateMutedByFocus,
	};

	enum class JoiningMethod {
		DialedIn = LinphoneParticipantDeviceJoiningMethodDialedIn,
		DialedOut = LinphoneParticipantDeviceJoiningMethodDialedOut,
		FocusOwner = LinphoneParticipantDeviceJoiningMethodFocusOwner
	};

	enum class DisconnectionMethod {
		Booted = LinphoneParticipantDeviceDisconnectionMethodBooted,
		Departed = LinphoneParticipantDeviceDisconnectionMethodDeparted,
		Busy = LinphoneParticipantDeviceDisconnectionMethodBusy,
		Failed = LinphoneParticipantDeviceDisconnectionMethodFailed
	};

	ParticipantDevice ();
	explicit ParticipantDevice (std::shared_ptr<Participant> participant, const std::shared_ptr<LinphonePrivate::CallSession> &session, const std::string &name = "");
	explicit ParticipantDevice (std::shared_ptr<Participant> participant, const IdentityAddress &gruu, const std::string &name = "");
	virtual ~ParticipantDevice ();
	// non clonable object
	ParticipantDevice *clone() const override { return nullptr; }

	bool operator== (const ParticipantDevice &device) const;

	std::shared_ptr<Core> getCore () const;

	const IdentityAddress & getAddress () const;
	void setAddress (const IdentityAddress & address);
	void setAddress (const Address & address);
	inline const std::string &getLabel () const { return mLabel; }
	inline void setLabel (const std::string &label) { mLabel = label; };
	const std::string &getCallId ();
	void setCallId (const std::string &callId);
	const std::string &getFromTag ();
	void setFromTag (const std::string &tag);
	const std::string &getToTag ();
	void setToTag (const std::string &tag);
	inline const std::string &getName () const { return mName; }
	inline void setName (const std::string &name) { mName = name; }
	inline std::shared_ptr<CallSession> getSession () const { return mSession; }
	std::shared_ptr<Participant> getParticipant () const;
	void setSession (std::shared_ptr<CallSession> session);
	inline State getState () const { return mState; }
	void setState (State newState, bool notify = true);
	inline void setJoiningMethod (JoiningMethod joiningMethod) { mJoiningMethod = joiningMethod; };
	inline JoiningMethod getJoiningMethod () const { return mJoiningMethod; };
	void setDisconnectionData(bool initiated, int code, LinphoneReason reason);
	inline void setDisconnectionMethod (DisconnectionMethod disconnectionMethod) { mDisconnectionMethod = disconnectionMethod; };
	inline DisconnectionMethod getDisconnectionMethod () const { return mDisconnectionMethod; };
	inline const std::string & getDisconnectionReason () const { return mDisconnectionReason; }
	inline void setDisconnectionReason (const std::string &disconnectionReason) { mDisconnectionReason = disconnectionReason; }
	AbstractChatRoom::SecurityLevel getSecurityLevel () const;

	inline bool isSubscribedToConferenceEventPackage () const { return mConferenceSubscribeEvent != nullptr; }
	LinphoneEvent *getConferenceSubscribeEvent () const { return mConferenceSubscribeEvent; }
	void setConferenceSubscribeEvent (LinphoneEvent *ev);

	bool isValid () const { return getAddress().isValid(); }
	bool isInConference () const;

	inline void setTimeOfJoining(time_t joiningTime) { mTimeOfJoining = joiningTime; }
	time_t getTimeOfJoining() const;
	inline void setTimeOfDisconnection(time_t disconnectionTime) { mTimeOfDisconnection = disconnectionTime; }
	time_t getTimeOfDisconnection() const;
	void setCapabilityDescriptor(const std::string &capabilities);
	const std::string & getCapabilityDescriptor()const{
		return mCapabilityDescriptor;
	}

	bool setSsrc (const LinphoneStreamType type, uint32_t newSsrc);
	uint32_t getSsrc (const LinphoneStreamType type) const;

	void *getUserData () const;
	void setUserData (void *ud);

	// Media getters and setters
	bool updateMediaCapabilities();
	bool updateStreamAvailabilities();

	bool adminModeSupported() const;
	void enableAdminModeSupport(bool support);

	void * createWindowId() const;
	void setWindowId(void * newWindowId) const;
	void * getWindowId() const;

	bool setStreamCapability(const LinphoneMediaDirection & direction, const LinphoneStreamType type);
	LinphoneMediaDirection getStreamCapability(const LinphoneStreamType type) const;

	bool setStreamAvailability(const bool available, const LinphoneStreamType type);
	bool getStreamAvailability(const LinphoneStreamType type) const;

	void setIsSpeaking(bool isSpeaking);
	bool getIsSpeaking() const;

	void setIsMuted(bool isMuted);
	bool getIsMuted() const;

	static bool isLeavingState(const ParticipantDevice::State & state);

protected:
	Conference *getConference () const;

private:
	std::weak_ptr<Participant> mParticipant;
	IdentityAddress mGruu;
	std::string mName;
	std::string mLabel;
	std::shared_ptr<CallSession> mSession;
	std::string mCapabilityDescriptor;
	std::string mCallId;
	std::string mFromTag;
	std::string mToTag;
	LinphoneEvent *mConferenceSubscribeEvent = nullptr;
	State mState = State::Joining;
	JoiningMethod mJoiningMethod = JoiningMethod::DialedIn;
	DisconnectionMethod mDisconnectionMethod = DisconnectionMethod::Departed;
	std::string mDisconnectionReason = std::string();
	time_t mTimeOfJoining = -1;
	time_t mTimeOfDisconnection = -1;
	bool mSupportAdminMode = false;
	mutable void * mWindowId = NULL;
	bool mIsMuted = false;
	bool mIsSpeaking = false;

	std::map<LinphoneStreamType, LinphoneMediaDirection> mediaCapabilities;
	std::map<LinphoneStreamType, bool> streamAvailabilities;
	std::map<LinphoneStreamType, uint32_t> ssrc;

	void *mUserData = nullptr;

	LinphoneMediaDirection computeDeviceMediaDirection(const bool conferenceEnable, const bool callEnable, const LinphoneMediaDirection dir) const;
	bool computeStreamAvailable(const bool conferenceEnable, const bool callEnable, const LinphoneMediaDirection dir) const;
	LinphoneMediaDirection getStreamDirectionFromSession(const LinphoneStreamType type) const;

	L_DISABLE_COPY(ParticipantDevice);
};

std::ostream &operator<< (std::ostream &stream, ParticipantDevice::State state);

class ParticipantDeviceCbs : public bellesip::HybridObject<LinphoneParticipantDeviceCbs, ParticipantDeviceCbs>, public Callbacks {
	public:
		LinphoneParticipantDeviceCbsIsSpeakingChangedCb getIsSpeakingChanged()const;
		void setIsSpeakingChanged(LinphoneParticipantDeviceCbsIsSpeakingChangedCb cb);
		LinphoneParticipantDeviceCbsIsMutedCb getIsMuted()const;
		void setIsMuted(LinphoneParticipantDeviceCbsIsMutedCb cb);
		LinphoneParticipantDeviceCbsStateChangedCb getStateChanged()const;
		void setStateChanged(LinphoneParticipantDeviceCbsStateChangedCb cb);
		LinphoneParticipantDeviceCbsStreamCapabilityChangedCb getStreamCapabilityChanged()const;
		void setStreamCapabilityChanged(LinphoneParticipantDeviceCbsStreamCapabilityChangedCb cb);
		LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb getStreamAvailabilityChanged()const;
		void setStreamAvailabilityChanged(LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb cb);
	private:
	LinphoneParticipantDeviceCbsIsSpeakingChangedCb mIsSpeakingChangedCb = nullptr;
	LinphoneParticipantDeviceCbsIsMutedCb mIsMutedCb = nullptr;
	LinphoneParticipantDeviceCbsStateChangedCb mStateChangedCb = nullptr;
	LinphoneParticipantDeviceCbsStreamCapabilityChangedCb mStreamCapabilityChangedCb = nullptr;
	LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb mStreamAvailabilityChangedCb = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PARTICIPANT_DEVICE_H_
