/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#ifndef _CONFERENCE_H_
#define _CONFERENCE_H_

#include "belle-sip/object++.hh"

#include "conference-cbs.h"
#include "conference/conference-interface.h"
#include "conference/conference.h"
#include "linphone/conference.h"
#include "linphone/utils/general.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type of the funtion to pass as callback to linphone_conference_params_set_state_changed_callback()
 * @param conference The conference instance which the state has changed
 * @param new_state The new state of the conferenece
 * @param user_data Pointer pass to user_data while linphone_conference_params_set_state_changed_callback() was being
 * called
 */
typedef void (*LinphoneConferenceStateChangedCb)(LinphoneConference *conference,
                                                 LinphoneConferenceState new_state,
                                                 void *user_data);

/**
 * A function to convert a #LinphoneConferenceState into a string
 */
char *linphone_conference_state_to_string(LinphoneConferenceState state);

/**
 * Set a callback which will be called when the state of the conferenec is switching
 * @param obj A #LinphoneConference object
 * @param cb The callback to call
 * @param user_data Pointer to pass to the user_data parameter of #LinphoneConferenceStateChangedCb
 */
void linphone_conference_set_state_changed_callback(LinphoneConference *obj,
                                                    LinphoneConferenceStateChangedCb cb,
                                                    void *user_data);

#ifdef __cplusplus
}
#endif

LINPHONE_BEGIN_NAMESPACE

class AudioControlInterface;
class VideoControlInterface;
class MixerSession;
class ConferenceParams;
class Call;
class CallSession;
class CallSessionListener;
class ParticipantDevice;
class AudioDevice;
class ConferenceId;

namespace MediaConference { // They are in a special namespace because of conflict of generic Conference classes in
	                        // src/conference/*

/*
 * Base class for audio/video conference.
 */
class LINPHONE_PUBLIC Conference : public bellesip::HybridObject<LinphoneConference, Conference>,
                                   public LinphonePrivate::Conference,
                                   public LinphonePrivate::CallbacksHolder<LinphonePrivate::ConferenceCbs>,
                                   public UserDataAccessor {
#ifdef HAVE_ADVANCED_IM
	friend class LocalAudioVideoConferenceEventHandler;
#endif // HAVE_ADVANCED_IM
public:
	Conference(const std::shared_ptr<Core> &core,
	           const std::shared_ptr<Address> &myAddress,
	           CallSessionListener *listener,
	           const std::shared_ptr<ConferenceParams> params);
	virtual ~Conference();

	virtual int inviteAddresses(const std::list<std::shared_ptr<Address>> &addresses,
	                            const LinphoneCallParams *params) = 0;
	virtual bool dialOutAddresses(const std::list<std::shared_ptr<Address>> &addressList) = 0;
	virtual bool addParticipant(const std::shared_ptr<Address> &participantAddress) override;
	virtual bool addParticipant(const std::shared_ptr<ParticipantInfo> &info) override;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual bool finalizeParticipantAddition(std::shared_ptr<LinphonePrivate::Call> call) = 0;

	virtual bool addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call);

	virtual int removeParticipantDevice(const std::shared_ptr<LinphonePrivate::CallSession> &session);
	int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call);
	virtual int removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> &session,
	                              const bool preserveSession);
	virtual int removeParticipant(const std::shared_ptr<Address> &addr) = 0;
	virtual bool removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) override;

	virtual bool
	removeParticipants(const std::list<std::shared_ptr<LinphonePrivate::Participant>> &participants) override;

	virtual int
	participantDeviceMediaCapabilityChanged(const std::shared_ptr<LinphonePrivate::CallSession> &session) = 0;
	virtual int participantDeviceMediaCapabilityChanged(const std::shared_ptr<Address> &addr) = 0;
	virtual int
	participantDeviceMediaCapabilityChanged(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                        const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) = 0;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> &session,
	                                         const LinphoneStreamType type,
	                                         uint32_t ssrc) = 0;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> &session,
	                                         uint32_t audioSsrc,
	                                         uint32_t videoSsrc) = 0;

	virtual int participantDeviceAlerting(const std::shared_ptr<LinphonePrivate::CallSession> &session) = 0;
	virtual int participantDeviceAlerting(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                      const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) = 0;
	virtual int participantDeviceJoined(const std::shared_ptr<LinphonePrivate::CallSession> &session) = 0;
	virtual int participantDeviceJoined(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                    const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) = 0;
	virtual int participantDeviceLeft(const std::shared_ptr<LinphonePrivate::CallSession> &session) = 0;
	virtual int participantDeviceLeft(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                  const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) = 0;

	virtual int getParticipantDeviceVolume(const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) = 0;

	virtual int terminate() = 0;
	virtual void finalizeCreation() = 0;

	virtual int enter() = 0;
	virtual void leave() override = 0;

	virtual const std::shared_ptr<Address> getOrganizer() const = 0;

	bool isConferenceEnded() const;
	bool isConferenceStarted() const;

	void setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	void setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	std::shared_ptr<AudioDevice> getInputAudioDevice() const;
	std::shared_ptr<AudioDevice> getOutputAudioDevice() const;

	virtual AudioControlInterface *getAudioControlInterface() const = 0;
	virtual VideoControlInterface *getVideoControlInterface() const = 0;
	virtual AudioStream *getAudioStream() = 0; /* Used by the tone manager, revisit.*/

	virtual int getSize() const {
		return getParticipantCount() + (isIn() ? 1 : 0);
	}

	virtual int startRecording(const char *path) = 0;
	virtual int stopRecording() = 0;
	virtual bool isRecording() const = 0;

	void setState(LinphonePrivate::ConferenceInterface::State state) override;
	void setStateChangedCallback(LinphoneConferenceStateChangedCb cb, void *userData) {
		mStateChangedCb = cb;
		mCbUserData = userData;
	}

	virtual void setParticipantAdminStatus(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                       bool isAdmin) override;

	virtual void join() override;
	virtual void join(const std::shared_ptr<Address> &participantAddress) override;
	virtual void onConferenceTerminated(const std::shared_ptr<Address> &addr) override;

	void setID(const std::string &conferenceID) {
		mConferenceID = conferenceID;
	}
	const std::string &getID() const {
		return mConferenceID;
	}

	void setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress);
	void setConferenceId(const ConferenceId &conferenceId);
	virtual void notifyStateChanged(LinphonePrivate::ConferenceInterface::State state) override;

protected:
	ConferenceInfo::participant_list_t mInvitedParticipants;

	// Legacy member
	std::string mConferenceID;

	LinphoneConferenceStateChangedCb mStateChangedCb = nullptr;
	void *mCbUserData = nullptr;
	LinphoneCoreCbs *mCoreCbs;

	static void callStateChanged(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message);
	static void transferStateChanged(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state);

	virtual void
	callStateChangedCb(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message) = 0;
	virtual void
	transferStateChangedCb(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) = 0;
	std::list<std::shared_ptr<Address>> getInvitedAddresses() const;
	ConferenceInfo::participant_list_t getFullParticipantList() const;
	void fillParticipantAttributes(std::shared_ptr<Participant> &p);
};

class ConferenceLogContextualizer : CoreLogContextualizer {
public:
	ConferenceLogContextualizer(const LinphoneConference *conf) : CoreLogContextualizer(*Conference::toCpp(conf)) {
	}
};

} // end of namespace MediaConference

LINPHONE_END_NAMESPACE

#endif // _CONFERENCE_H_
