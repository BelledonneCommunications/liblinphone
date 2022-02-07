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
		Joining, //an INVITE has been sent
		Present, //the SIP session has been concluded, participant is part of the conference
		OnHold, //the SIP session has been concluded, participant is not media mixed
		Leaving, //A BYE is pending
		Left, //The Session is terminated
		ScheduledForJoining, //Initial state for the server group chatroom, when the participant has not yet been INVITEd.
		ScheduledForLeaving, //Transitional state for a participant that will receive a BYE shortly.
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
	inline const std::string &getLabel () const { return mLabel; }
	inline void setLabel (const std::string &label) { mLabel = label; };
	inline const std::string &getName () const { return mName; }
	inline void setName (const std::string &name) { mName = name; }
	inline std::shared_ptr<CallSession> getSession () const { return mSession; }
	std::shared_ptr<Participant> getParticipant () const;
	void setSession (std::shared_ptr<CallSession> session);
	inline State getState () const { return mState; }
	void setState (State newState);
	AbstractChatRoom::SecurityLevel getSecurityLevel () const;

	inline bool isSubscribedToConferenceEventPackage () const { return mConferenceSubscribeEvent != nullptr; }
	LinphoneEvent *getConferenceSubscribeEvent () const { return mConferenceSubscribeEvent; }
	void setConferenceSubscribeEvent (LinphoneEvent *ev);

	bool isValid () const { return getAddress().isValid(); }
	bool isInConference () const;

	time_t getTimeOfJoining() const;
	void setCapabilityDescriptor(const std::string &capabilities);
	const std::string & getCapabilityDescriptor()const{
		return mCapabilityDescriptor;
	}

	void setSsrc (uint32_t ssrc);
	uint32_t getSsrc () const;

	void *getUserData () const;
	void setUserData (void *ud);

	void setLayout(const ConferenceLayout l) { mLayout = l; };
	ConferenceLayout getLayout() const { return mLayout; };

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

protected:
	Conference *getConference () const;

private:
	std::weak_ptr<Participant> mParticipant;
	mutable IdentityAddress mGruu;
	std::string mName;
	std::string mLabel;
	std::shared_ptr<CallSession> mSession;
	std::string mCapabilityDescriptor;
	ConferenceLayout mLayout = ConferenceLayout::Legacy;
	LinphoneEvent *mConferenceSubscribeEvent = nullptr;
	State mState = State::Joining;
	time_t mTimeOfJoining;
	uint32_t mSsrc = 0;
	bool mSupportAdminMode = false;
	mutable void * mWindowId = NULL;

	std::map<LinphoneStreamType, LinphoneMediaDirection> mediaCapabilities;
	std::map<LinphoneStreamType, bool> streamAvailabilities;

	void *mUserData = nullptr;

	LinphoneMediaDirection computeDeviceMediaDirection(const bool conferenceEnable, const bool callEnable, const LinphoneMediaDirection dir) const;
	bool computeStreamAvailable(const bool conferenceEnable, const bool callEnable, const LinphoneMediaDirection dir) const;

	L_DISABLE_COPY(ParticipantDevice);
};

std::ostream &operator<< (std::ostream &stream, ParticipantDevice::State state);

class ParticipantDeviceCbs : public bellesip::HybridObject<LinphoneParticipantDeviceCbs, ParticipantDeviceCbs>, public Callbacks {
	public:
		LinphoneParticipantDeviceCbsIsSpeakingChangedCb getIsSpeakingChanged()const;
		void setIsSpeakingChanged(LinphoneParticipantDeviceCbsIsSpeakingChangedCb cb);
		LinphoneParticipantDeviceCbsConferenceJoinedCb getConferenceJoined()const;
		void setConferenceJoined(LinphoneParticipantDeviceCbsConferenceJoinedCb cb);
		LinphoneParticipantDeviceCbsConferenceLeftCb getConferenceLeft()const;
		void setConferenceLeft(LinphoneParticipantDeviceCbsConferenceLeftCb cb);
		LinphoneParticipantDeviceCbsStreamCapabilityChangedCb getStreamCapabilityChanged()const;
		void setStreamCapabilityChanged(LinphoneParticipantDeviceCbsStreamCapabilityChangedCb cb);
		LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb getStreamAvailabilityChanged()const;
		void setStreamAvailabilityChanged(LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb cb);
	private:
	LinphoneParticipantDeviceCbsIsSpeakingChangedCb mIsSpeakingChangedCb = nullptr;
	LinphoneParticipantDeviceCbsConferenceJoinedCb mConferenceJoinedCb = nullptr;
	LinphoneParticipantDeviceCbsConferenceLeftCb mConferenceLeftCb = nullptr;
	LinphoneParticipantDeviceCbsStreamCapabilityChangedCb mStreamCapabilityChangedCb = nullptr;
	LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb mStreamAvailabilityChangedCb = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PARTICIPANT_DEVICE_H_
