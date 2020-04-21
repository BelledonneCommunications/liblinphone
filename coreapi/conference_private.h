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

#include "linphone/core.h"
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


/**
 * Set a callback which will be called when the state of the conferenec is switching
 * @param params A #LinphoneConferenceParams object
 * @param cb The callback to call
 * @param user_data Pointer to pass to the user_data parameter of #LinphoneConferenceStateChangedCb
 */
void linphone_conference_params_set_state_changed_callback(LinphoneConferenceParams *params, LinphoneConferenceStateChangedCb cb, void *user_data);


LinphoneConference *linphone_local_conference_new(LinphoneCore *core);
LinphoneConference *linphone_local_conference_new_with_params(LinphoneCore *core, const LinphoneConferenceParams *params);
LinphoneConference *linphone_remote_conference_new(LinphoneCore *core);
LinphoneConference *linphone_remote_conference_new_with_params(LinphoneCore *core, const LinphoneConferenceParams *params);

/**
 * Get the state of a conference
 */
LinphoneConferenceState linphone_conference_get_state(const LinphoneConference *obj);

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

class AudioControlInterface;
class MixerSession;

namespace MediaConference{ // They are in a special namespace because of conflict of generic Conference classes in src/conference/*

class Conference;
class LocalConference;
class RemoteConference;


class ConferenceParams : public bellesip::HybridObject<LinphoneConferenceParams, ConferenceParams>{
	friend class Conference;
	friend class LocalConference;
	friend class RemoteConference;
	public:
		ConferenceParams(const ConferenceParams& params) = default;
		ConferenceParams(const LinphoneCore *core = NULL) {
			m_enableVideo = false;
			if(core) {
				const LinphoneVideoPolicy *policy = linphone_core_get_video_policy(core);
				if(policy->automatically_initiate) m_enableVideo = true;
			}
			m_stateChangedCb = NULL;
			m_userData = NULL;
		}
		void enableVideo(bool enable) {m_enableVideo = enable;}
		bool videoEnabled() const {return m_enableVideo;}
		void setStateChangedCallback(LinphoneConferenceStateChangedCb cb, void *userData) {
			m_stateChangedCb = cb;
			m_userData = userData;
		}
		void enableLocalParticipant(bool enable){ mLocalParticipantEnabled = enable; }
		bool localParticipantEnabled()const { return mLocalParticipantEnabled; }
		Object *clone()const override{
			return new ConferenceParams(*this);
		}
		
	private:
		LinphoneConferenceStateChangedCb m_stateChangedCb;
		void *m_userData;
		bool m_enableVideo;
		bool mLocalParticipantEnabled = true;
};

class Conference : public bellesip::HybridObject<LinphoneConference, Conference>{
public:
	class Participant {
	public:
		Participant(LinphoneCall *call) {
			m_uri = linphone_address_clone(linphone_call_get_remote_address(call));
			m_call = call;
		}

		~Participant() {
			linphone_address_unref(m_uri);
		}

		const LinphoneAddress *getUri() const {
			return m_uri;
		}

		LinphoneCall *getCall() const {
			return m_call;
		}

	private:
		Participant(const Participant &src);
		Participant &operator=(const Participant &src);

	private:
		LinphoneAddress *m_uri;
		LinphoneCall *m_call;

		friend class RemoteConference;
	};

	Conference(LinphoneCore *core, const ConferenceParams *params = NULL);
	virtual ~Conference() {}

	const ConferenceParams &getCurrentParams() const {return *m_currentParams;}

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) = 0;
	virtual int addParticipant(LinphoneCall *call) = 0;
	virtual int removeParticipant(LinphoneCall *call) = 0;
	virtual int removeParticipant(const LinphoneAddress *uri) = 0;
	virtual int terminate() = 0;

	virtual int enter() = 0;
	virtual int leave() = 0;
	virtual bool isIn() const = 0;

	virtual AudioControlInterface * getAudioControlInterface() const = 0;
	virtual AudioStream *getAudioStream() = 0; /* Used by the tone manager, revisit.*/

	virtual int getSize() const {return (int)m_participants.size() + (isIn()?1:0);}
	const std::list<Participant *> &getParticipants() const {return m_participants;}

	virtual int startRecording(const char *path) = 0;
	virtual int stopRecording() = 0;

	LinphoneConferenceState getState() const {return m_state;}
	LinphoneCore *getCore()const{
		return m_core;
	}
	static const char *stateToString(LinphoneConferenceState state);

	void setID(const char *conferenceID) {
		m_conferenceID = conferenceID;
	}
	const char *getID() const {
		return m_conferenceID.c_str();
	}

protected:
	void setState(LinphoneConferenceState state);
	Participant *findParticipant(const LinphoneCall *call) const;
	Participant *findParticipant(const LinphoneAddress *uri) const;

protected:
	std::string m_conferenceID;
	LinphoneCore *m_core;
	std::list<Participant *> m_participants;
	std::shared_ptr<ConferenceParams> m_currentParams;
	LinphoneConferenceState m_state;
};

class LocalConference: public Conference {
public:
	LocalConference(LinphoneCore *core, const ConferenceParams *params = NULL);
	virtual ~LocalConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	virtual int addParticipant(LinphoneCall *call) override;
	virtual int removeParticipant(LinphoneCall *call) override;
	virtual int removeParticipant(const LinphoneAddress *uri) override;
	virtual int terminate() override;

	virtual int enter() override;
	virtual int leave() override;
	virtual bool isIn() const override;

	virtual int startRecording(const char *path) override;
	virtual int stopRecording() override;
	
	virtual AudioControlInterface * getAudioControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

private:
	void addLocalEndpoint();
	int remoteParticipantsCount();
	void removeLocalEndpoint();
	std::unique_ptr<MixerSession> mMixerSession;
	bool mIsIn = false;
};

class RemoteConference: public Conference {
public:
	RemoteConference(LinphoneCore *core, const ConferenceParams *params = NULL);
	virtual ~RemoteConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	virtual int addParticipant(LinphoneCall *call) override;
	virtual int removeParticipant(LinphoneCall *call) override {
		return -1;
	}
	virtual int removeParticipant(const LinphoneAddress *uri) override;
	virtual int terminate() override;

	virtual int enter() override;
	virtual int leave() override;
	virtual bool isIn() const override;

	virtual int startRecording (const char *path) override {
		return 0;
	}
	virtual int stopRecording() override {
		return 0;
	}
	virtual AudioControlInterface * getAudioControlInterface() const override;
	virtual AudioStream *getAudioStream() override;
	
private:
	bool focusIsReady() const;
	bool transferToFocus(LinphoneCall *call);
	void reset();

	void onFocusCallSateChanged(LinphoneCallState state);
	void onPendingCallStateChanged(LinphoneCall *call, LinphoneCallState state);
	void onTransferingCallStateChanged(LinphoneCall *transfered, LinphoneCallState newCallState);

	static void callStateChangedCb(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message);
	static void transferStateChanged(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state);

	const char *m_focusAddr;
	char *m_focusContact;
	LinphoneCall *m_focusCall;
	LinphoneCoreCbs *m_coreCbs;
	std::list<LinphoneCall *> m_pendingCalls;
	std::list<LinphoneCall *> m_transferingCalls;
};

}// end of namespace MediaConference

LINPHONE_END_NAMESPACE

#endif //CONFERENCE_PRIVATE_H
