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

#ifndef CONFERENCE_PRIVATE_H
#define CONFERENCE_PRIVATE_H

#include <map>

#include "linphone/core.h"
#include "call/call.h"
#include "linphone/conference.h"
#include "conference/conference.h"
#include "conference/handlers/local-audio-video-conference-event-handler.h"

#include "belle-sip/object++.hh"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type of the funtion to pass as callback to linphone_conference_params_set_state_changed_callback()
 * @param conference The conference instance which the state has changed
 * @param new_state The new state of the conferenece
 * @param user_data Pointer pass to user_data while linphone_conference_params_set_state_changed_callback() was being called
 */
typedef void (*LinphoneConferenceStateChangedCb)(LinphoneConference *conference, LinphoneConferenceState new_state, void *user_data);

/**
 * A function to converte a #LinphoneConferenceState into a string
 */
const char *linphone_conference_state_to_string(LinphoneConferenceState state);

/**
 * Get the state of a conference
 */
LinphoneConferenceState linphone_conference_get_state(const LinphoneConference *obj);

/**
 * Set a callback which will be called when the state of the conferenec is switching
 * @param obj A #LinphoneConference object
 * @param cb The callback to call
 * @param user_data Pointer to pass to the user_data parameter of #LinphoneConferenceStateChangedCb
 */
void linphone_conference_set_state_changed_callback(LinphoneConference *obj, LinphoneConferenceStateChangedCb cb, void *user_data);

#ifdef __cplusplus
}
#endif

LINPHONE_BEGIN_NAMESPACE

class Call;
class Participant;
class AudioControlInterface;
class VideoControlInterface;
class MixerSession;

class ConferenceFactoryInterface;
class ConferenceParamsInterface;
class ConferenceParams;

class RemoteConferenceEventHandler;

namespace MediaConference{ // They are in a special namespace because of conflict of generic Conference classes in src/conference/*

class Conference;
class LocalConference;
class RemoteConference;

/*
 * Base class for audio/video conference.
 */

class LINPHONE_PUBLIC Conference : public bellesip::HybridObject<LinphoneConference, Conference>, public LinphonePrivate::Conference {
	friend class LocalAudioVideoConferenceEventHandler;
public:
	Conference(const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);
	virtual ~Conference();

	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::Conference::findParticipant;

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) = 0;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::ConferenceInterface::addParticipant;
	virtual bool addParticipant(const IdentityAddress &participantAddress) override;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;

	bool addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call);

	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::Conference::removeParticipant;
	virtual int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) = 0;
	virtual int removeParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call);
	virtual int removeParticipant(const IdentityAddress &addr) = 0;
	virtual bool removeParticipants (const std::list<std::shared_ptr<LinphonePrivate::Participant>> &participants) override;

	virtual int terminate() = 0;
	virtual void finalizeCreation() = 0;

	virtual int enter() = 0;
	virtual void leave() override = 0;
	virtual bool isIn() const = 0;

	virtual AudioControlInterface * getAudioControlInterface() const = 0;
	virtual VideoControlInterface * getVideoControlInterface() const = 0;
	virtual AudioStream *getAudioStream() = 0; /* Used by the tone manager, revisit.*/

	virtual int getSize() const {return getParticipantCount() + (isIn()?1:0);}

	virtual int startRecording(const char *path) = 0;
	virtual int stopRecording() = 0;

	void setState (LinphonePrivate::ConferenceInterface::State state) override;
	void setStateChangedCallback(LinphoneConferenceStateChangedCb cb, void *userData) {
		mStateChangedCb = cb;
		mUserData = userData;
	}

	virtual void setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) override;

	void setConferenceAddress (const ConferenceAddress &conferenceAddress);

	// TODO: Delete
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::ConferenceInterface::join;
	virtual void join (const IdentityAddress &participantAddress) override;

	virtual std::shared_ptr<LinphonePrivate::Participant> getMe () const override;

	bctbx_list_t *getCallbacksList () const;
	LinphoneConferenceCbs *getCurrentCbs () const;
	void setCurrentCbs (LinphoneConferenceCbs *cbs);
	void addCallbacks (LinphoneConferenceCbs *cbs);
	void removeCallbacks (LinphoneConferenceCbs *cbs);

	void *getUserData () const;
	void setUserData (void *ud);

	virtual void onConferenceTerminated (const IdentityAddress &addr) override;

	virtual void notifyStateChanged (LinphonePrivate::ConferenceInterface::State state) override;

protected:
	std::shared_ptr<LinphonePrivate::Participant> findParticipant(const std::shared_ptr<LinphonePrivate::Call> call) const;

protected:

	LinphoneConferenceStateChangedCb mStateChangedCb = nullptr;
	void *mUserData = nullptr;
	bctbx_list_t *mCallbacks = nullptr;
	LinphoneConferenceCbs *mCurrentCbs = nullptr;
};


/*
 * Class for an audio/video conference running locally.
 */
class LINPHONE_PUBLIC LocalConference: public Conference {
public:
	LocalConference(const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);

	virtual ~LocalConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::ConferenceInterface::addParticipant;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;

	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::Conference::removeParticipant;
	virtual int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual int removeParticipant(const IdentityAddress &addr) override;
	virtual bool update(const ConferenceParamsInterface &params) override;
	virtual int terminate() override;
	virtual void finalizeCreation() override;
	virtual void onConferenceTerminated (const IdentityAddress &addr) override;
	virtual void setSubject (const std::string &subject) override;

	virtual int enter() override;
	virtual void leave() override;
	virtual bool isIn() const override;

	virtual int startRecording(const char *path) override;
	virtual int stopRecording() override;
	
	virtual AudioControlInterface * getAudioControlInterface() const override;
	virtual VideoControlInterface * getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	void subscribeReceived (LinphoneEvent *event);
	void subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state);

	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantAdded (time_t creationTime,  const bool isFullState, const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantRemoved (time_t creationTime,  const bool isFullState, const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantSetAdmin (time_t creationTime,  const bool isFullState, const Address &addr, bool isAdmin);
	std::shared_ptr<ConferenceSubjectEvent> notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject);
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceAdded (time_t creationTime,  const bool isFullState, const Address &addr, const Address &gruu, const std::string name = "");
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceRemoved (time_t creationTime,  const bool isFullState, const Address &addr, const Address &gruu);

	void notifyFullState ();

private:
	void addLocalEndpoint();
	void removeLocalEndpoint();
	std::unique_ptr<MixerSession> mMixerSession;
	bool mIsIn = false;

#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<LocalAudioVideoConferenceEventHandler> eventHandler;
#endif // HAVE_ADVANCED_IM
};

/*
 * Class for an audio/video conference that is running on a remote server.
 */
class LINPHONE_PUBLIC RemoteConference:
	public Conference,
	public ConferenceListenerInterface {
public:
	RemoteConference(const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, const ConferenceId &conferenceId, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);
	virtual ~RemoteConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::ConferenceInterface::addParticipant;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;

	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::Conference::removeParticipant;
	virtual int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) override {
		return -1;
	}
	virtual int removeParticipant(const IdentityAddress &addr) override;
	virtual int terminate() override;
	virtual void finalizeCreation() override;

	virtual int enter() override;
	virtual void leave() override;
	virtual bool isIn() const override;

	virtual int startRecording (const char *path) override {
		return 0;
	}
	virtual int stopRecording() override {
		return 0;
	}
	virtual AudioControlInterface * getAudioControlInterface() const override;
	virtual VideoControlInterface * getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	void notifyReceived (const std::string &body);

	void onStateChanged(LinphonePrivate::ConferenceInterface::State state) override;

#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<RemoteConferenceEventHandler> eventHandler;
#endif // HAVE_ADVANCED_IM
private:
	bool focusIsReady() const;
	bool transferToFocus(std::shared_ptr<LinphonePrivate::Call> call);
	void reset();

	void onFocusCallSateChanged(LinphoneCallState state);
	void onPendingCallStateChanged(std::shared_ptr<LinphonePrivate::Call> call, LinphoneCallState callState);
	void onTransferingCallStateChanged(std::shared_ptr<LinphonePrivate::Call> transfered, LinphoneCallState newCallState);

	static void callStateChangedCb(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message);
	static void transferStateChanged(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state);

	const char *m_focusAddr;
	char *m_focusContact;
	std::shared_ptr<LinphonePrivate::Call> m_focusCall;
	LinphoneCoreCbs *m_coreCbs;
	std::list<std::shared_ptr<LinphonePrivate::Call>> m_pendingCalls;
	std::list<std::shared_ptr<LinphonePrivate::Call>> m_transferingCalls;

};

}// end of namespace MediaConference

LINPHONE_END_NAMESPACE

#endif //CONFERENCE_PRIVATE_H
