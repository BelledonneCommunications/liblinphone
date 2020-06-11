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

#include "belle-sip/object++.hh"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LinphoneConferenceClassLocal,
	LinphoneConferenceClassRemote
} LinphoneConferenceClass;

/**
 * List of states used by #LinphoneConference
 */
typedef enum {
	LinphoneConferenceStopped, /*< Initial state */
	LinphoneConferenceStarting, /*< A participant has been added but the conference is not running yet */
	LinphoneConferenceRunning, /*< The conference is running */
	LinphoneConferenceStartingFailed /*< A participant has been added but the initialization of the conference has failed */
} LinphoneConferenceState;
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


LinphoneConferenceParams *linphone_conference_params_new(const LinphoneCore *core);

LinphoneConference *linphone_local_conference_new(LinphoneCore *core, LinphoneAddress * addr);
LinphoneConference *linphone_local_conference_new_with_params(LinphoneCore *core, LinphoneAddress * addr, const LinphoneConferenceParams *params);
LinphoneConference *linphone_remote_conference_new(LinphoneCore *core, LinphoneAddress * addr);
LinphoneConference *linphone_remote_conference_new_with_params(LinphoneCore *core, LinphoneAddress * addr, const LinphoneConferenceParams *params);

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

int linphone_conference_add_participant(LinphoneConference *obj, LinphoneCall *call);
int linphone_conference_remove_participant_with_call(LinphoneConference *obj, LinphoneCall *call);
int linphone_conference_terminate(LinphoneConference *obj);
int linphone_conference_get_size(const LinphoneConference *obj);

int linphone_conference_enter(LinphoneConference *obj);
int linphone_conference_leave(LinphoneConference *obj);
bool_t linphone_conference_is_in(const LinphoneConference *obj);

/* This is actually only used by the ToneManager. TODO: encapsulate this better. */
AudioStream *linphone_conference_get_audio_stream(LinphoneConference *obj);

int linphone_conference_mute_microphone(LinphoneConference *obj, bool_t val);
bool_t linphone_conference_microphone_is_muted(const LinphoneConference *obj);
float linphone_conference_get_input_volume(const LinphoneConference *obj);

int linphone_conference_start_recording(LinphoneConference *obj, const char *path);
int linphone_conference_stop_recording(LinphoneConference *obj);

void linphone_conference_on_call_terminating(LinphoneConference *obj, LinphoneCall *call);

LINPHONE_PUBLIC bool_t linphone_conference_check_class(LinphoneConference *obj, LinphoneConferenceClass _class);

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

class Conference : public bellesip::HybridObject<LinphoneConference, Conference>, public LinphonePrivate::Conference {
public:
	Conference(const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);
	virtual ~Conference() {}

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) = 0;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::ConferenceInterface::addParticipant;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override = 0;

	virtual int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) = 0;
	virtual int removeParticipant(const IdentityAddress &addr) = 0;
	virtual bool removeParticipant (const std::shared_ptr<LinphonePrivate::Participant> &participant) override;
	virtual bool removeParticipants (const std::list<std::shared_ptr<LinphonePrivate::Participant>> &participants) override;

	virtual int terminate() = 0;

	virtual int enter() = 0;
	virtual void leave() override = 0;
	virtual bool isIn() const = 0;

	virtual AudioControlInterface * getAudioControlInterface() const = 0;
	virtual VideoControlInterface * getVideoControlInterface() const = 0;
	virtual AudioStream *getAudioStream() = 0; /* Used by the tone manager, revisit.*/

	virtual int getSize() const {return (int)m_participants.size() + (isIn()?1:0);}
	virtual const std::list<std::shared_ptr<LinphonePrivate::Participant>> &getParticipants() const override {return m_participants;}

	virtual int startRecording(const char *path) = 0;
	virtual int stopRecording() = 0;

	LinphoneConferenceState getState() const {return m_state;}
	static const char *stateToString(LinphoneConferenceState state);

	void setID(const char *conferenceID) {
		m_conferenceID = conferenceID;
	}
	const char *getID() const {
		return m_conferenceID.c_str();
	}
	void setStateChangedCallback(LinphoneConferenceStateChangedCb cb, void *userData) {
		m_stateChangedCb = cb;
		m_userData = userData;
	}

	virtual const ConferenceAddress getConferenceAddress () const override;

	virtual const std::string &getSubject () const override;

	virtual void setSubject (const std::string &subject) override;

	virtual void setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) override;

	// TODO: Delete
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::ConferenceInterface::join;
	virtual void join (const IdentityAddress &participantAddress) override;

	virtual int getParticipantCount () const override;

	virtual std::shared_ptr<LinphonePrivate::Participant> getMe () const override;

protected:
	void setState(LinphoneConferenceState state);
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::ConferenceInterface::findParticipant;
	std::shared_ptr<LinphonePrivate::Participant> findParticipant(const std::shared_ptr<LinphonePrivate::Call> call) const;

protected:
	std::string m_conferenceID;
	std::list<std::shared_ptr<LinphonePrivate::Participant>> m_participants;

	// Temporary member to store participant,call pairs
	// TODO: Remove once conference merge is finished
	std::map<std::shared_ptr<LinphonePrivate::Participant>, std::shared_ptr<LinphonePrivate::Call>> m_callTable;
	LinphoneConferenceState m_state;
	LinphoneConferenceStateChangedCb m_stateChangedCb = nullptr;
	void *m_userData = nullptr;
};


/*
 * Class for an audio/video conference running locally.
 */
class LocalConference: public Conference {
public:
	LocalConference(const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);

	virtual ~LocalConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::ConferenceInterface::addParticipant;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;

	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::MediaConference::Conference::removeParticipant;
	virtual int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual int removeParticipant(const IdentityAddress &addr) override;
	virtual bool update(const ConferenceParamsInterface &params) override;
	virtual int terminate() override;

	virtual int enter() override;
	virtual void leave() override;
	virtual bool isIn() const override;

	virtual int startRecording(const char *path) override;
	virtual int stopRecording() override;
	
	virtual AudioControlInterface * getAudioControlInterface() const override;
	virtual VideoControlInterface * getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

private:
	void addLocalEndpoint();
	int remoteParticipantsCount();
	void removeLocalEndpoint();
	std::unique_ptr<MixerSession> mMixerSession;
	bool mIsIn = false;
};

/*
 * Class for an audio/video conference that is running on a remote server.
 */
class RemoteConference:
	public Conference,
	public ConferenceListenerInterface {
public:
	RemoteConference(const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);
	virtual ~RemoteConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::ConferenceInterface::addParticipant;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::MediaConference::Conference::removeParticipant;
	virtual int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) override {
		return -1;
	}
	virtual int removeParticipant(const IdentityAddress &addr) override;
	virtual int terminate() override;

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

	std::shared_ptr<RemoteConferenceEventHandler> eventHandler;
private:
	bool focusIsReady() const;
	bool transferToFocus(std::shared_ptr<LinphonePrivate::Call> call);
	void reset();

	void onFocusCallSateChanged(LinphoneCallState state);
	void onPendingCallStateChanged(std::shared_ptr<LinphonePrivate::Call> call, LinphoneCallState state);
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
