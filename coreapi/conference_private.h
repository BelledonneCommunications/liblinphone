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

#include "c-wrapper/c-wrapper.h"
#include "linphone/core.h"
#include "call/call.h"
#include "linphone/conference.h"
#include "c-wrapper/internal/c-tools.h"

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



LinphoneConference *linphone_local_conference_new(LinphoneCore *core);
LinphoneConference *linphone_local_conference_new_with_params(LinphoneCore *core, const LinphoneConferenceParams *params);
LinphoneConference *linphone_remote_conference_new(LinphoneCore *core);
LinphoneConference *linphone_remote_conference_new_with_params(LinphoneCore *core, const LinphoneConferenceParams *params);

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

namespace MediaConference{ // They are in a special namespace because of conflict of generic Conference classes in src/conference/*

class Conference;
class LocalConference;
class RemoteConference;

class ConferenceFactoryInterface;
class ConferenceParamsInterface;

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
		}
		void enableVideo(bool enable) {m_enableVideo = enable;}
		bool videoEnabled() const {return m_enableVideo;}
		void enableLocalParticipant(bool enable){ mLocalParticipantEnabled = enable; }
		bool localParticipantEnabled()const { return mLocalParticipantEnabled; }
		Object *clone()const override{
			return new ConferenceParams(*this);
		}
		
	private:
		bool m_enableVideo;
		bool mLocalParticipantEnabled = true;
};

/*
 * This interface allow an application to manage a multimedia conference. Implementation follows 2 mains rfc4579 ( SIP Call Control - Conferencing for User Agents) and rfc4575 ( A Session Initiation Protocol (SIP) Event Package for Conference State). It can be  either a Focus user agent (I.E local conference) or belong to a remote focus user agent (I.E remote conference). <br>
*
*	<br>
* 	Conference is instanciated with ConferenceParams and list of initial participants. ConferenceParams allows to choose beetween local or remote conference, and to set initial parameters.
 *	A conference is created either by the focus or with a remote focus. <br>
 */
class LINPHONE_PUBLIC ConferenceInterface {
public:
	/**
	 *Conference live cycle, specilly creation and termination requires interractions between user application and focus user agent.
	 * State is used to indicate the current state of a Conference.
	 */
	enum class State{
		None, /**< Initial state */
		Instantiated, /**< Conference is now instantiated participants can be added, but no focus address is ready yet */
		CreationPending, /**< If not focus,  creation request was sent to the server, else conference id allocation is ongoing */
		Created, /**<  Conference was created with a conference id. Communication can start*/
		CreationFailed, /**< Creation failed */
		TerminationPending, /**< Wait for Conference termination */
		Terminated, /**< Conference exists on server but not in local //fixme jehan creuser ce point */
		TerminationFailed, /**< Conference termination failed */
		Deleted /**< Conference is deleted on the server //fixme jehan creuser ce point  */
	};

	virtual ~ConferenceInterface () = default;

	/*
	 *Listener reporting events for this conference. Use this function mainly to add listener when conference is created at the initiative of the focus.
	 */
	virtual void addListener(std::shared_ptr<ConferenceListenerInterface> &listener) = 0;
	
	/*
	 Get the current State of this conference
	 */
//	virtual State getState () const = 0;

	/*
	 Get the conference ID of this conference.
	 @return The Address of the conference.
	 **/
	virtual const Address &getConferenceId () const = 0;

	/*
	* Get the subject of this conference
	* @return The subject of the chat room
	*/
	virtual const std::string &getSubject () const = 0;

	/*
	* Set the subject of this conference. If not focus,  this operation is only available if the local participant  #getMe() is admin.
	* @param[in] subject The new subject to set for the chat room
	*/
	virtual void setSubject (const std::string &subject) = 0;

	/*
	 * Change the admin status of a participant of this conference (If not focus,  This operation is only available if the local participant  #getMe() is admin). All participants are notified of subject change.
	 * @param[in] participant The Participant for which to change the admin status
	 * @param[in] isAdmin A boolean value telling whether the participant should now be an admin or not
	 */
	virtual void setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) = 0;

	/* This fonction add a participant to this conference.<br>
	 *Dependending if focus user agent is local or remote, behavior is defferent. <br>
	 *In both case, a call with a from address corresponding to the participant address is searched first. <br>
	 *<br>
	 <b>Local focus case: </b><br>
	 *If both found and unique found call is updated with 'isfocus' feature parameter in the Contact header field. If not found, focus add participant following the "INVITE: Adding a Participant by the Focus - Dial-Out" as described by RFC4579. <br>
	 <b>Remote focus case: </b><br>
	 *This operation is only available if the local participant  #getMe() is admin <br>.
	 *If found, call is transfered to the focus following " REFER: Requesting a User to Dial in to a Conference Using a Conference URI" as described by RFC4579. <br>
	 *if not found, add participant following " REFER: Requesting a Focus to Add a New Resource to a Conference(Dial Out to a New Participant)" as described by RFC4579. <br>
	 @param[in] participantAddress The address of the participant to add to this Conference.
	*/
	virtual bool addParticipant (const IdentityAddress &participantAddress) = 0;
	
	/*
	 * Same as function addParticipant(const IdentityAddress &participantAddress), except that call to add is passed by the user application.
	 * @param[in] call to be added tot he conference.
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool addParticipant (std::shared_ptr<LinphonePrivate::Call> call) = 0;

	/*
	* Add several participants at once.
	* @param[in] addresses
	* @return True if everything is OK, False otherwise
	*/
	virtual bool addParticipants (const std::list<IdentityAddress> &addresses) = 0;

	/*
	 * Add local participant to this conference, this fonction is only available for local focus. Behavior is the same as
	 */
	 virtual void join (const IdentityAddress &participantAddress) = 0;

	/*
	* Find a participant  from  its address.
	* @param[in] participantAddress The address to search in the list of participants of the chat room
	* @return The participant if found, NULL otherwise.
	*/
	virtual std::shared_ptr<LinphonePrivate::Participant> findParticipant (const IdentityAddress &participantAddress) const = 0;

	/*
	* Get the number of participants in this conference (that is without ourselves).
	* @return The number of participants in this conference
	*/
	virtual int getParticipantCount () const = 0;

	/*
	* Get the list of participants in this conference  (that is without ourselves).
	* @return \std::list<std::shared_ptr<LinphonePrivate::Participant>>
	*/
	virtual const std::list<std::shared_ptr<LinphonePrivate::Participant>> &getParticipants () const = 0;
	
	/*
	* Get the participant representing myself in this Conference (I.E local participant).<br>
	*Local participant behavior depends on weither the focus is local or remote. <br>
	*<b>Local focus case: </b><br>
	*Local participant is not mandatory. From value taken during conference managements is focus address.
	<b>Remote focus case: </b><br>
	*Local participant is the Participant of this conference used as From when operations are performed like subject change or participant management. local participant is not included in the of participant returned by function. Local participant is mandatory to create a remote conference conference.
	* @return The participant representing myself in the conference.
	*/
	virtual std::shared_ptr<LinphonePrivate::Participant> getMe () const = 0;

	/*
	 *Remove a participant from this conference.
	 *Dependending if  Focus user agent is local or remote,  behavior is defferent.<br>
	 *<b>Local focus case: </b><br>
	 *Calls corresponding to all devices of this participant are terminated. A new admin is designated if removed participant was the only admin<br>
	<b>Remote focus case: </b><br>
	 *This operation is only available if the local participant  #getMe() is admin <br>.
	 *Remove participant following "REFER with BYE: Requesting That the Focus Remove a Participant from a Conference"  as described by RFC4579.<br>
	 * @param[in] participantAddress The address to search and remove in the list of participants of this conference
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool removeParticipant (const std::shared_ptr<LinphonePrivate::Participant> &participant) = 0;

	/*
	 *Remove a list of participant from this conference.<br>
	 * @param[in] participants The addresses to search in the list of participants of this conference.
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool removeParticipants (const std::list<std::shared_ptr<LinphonePrivate::Participant>> &participants) = 0;

	/*
	 * Remove the local participant from this conference. <br>
	 *<b>Local focus case: </b><br>
	 * Remainning participant are notified. Conference state does not change.
	 *<b>Remote focus case: </b><br>
	 * The local participant is removed from the conference as any other participant.
	 * Conference is transitioned to state TerminationPending until removal is acknowled by the server.
	 */
	virtual void leave () = 0;

	/*
	 * Call to update conference parameters like media type. If not focus,  this operation is only available if the local participant  #getMe() is admin. <br>
	 * @param[in] new parameter to applie for this conference.
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool update(const ConferenceParamsInterface &newParameters) = 0;

};

class ConferenceParamsInterface {
public:
	virtual ~ConferenceParamsInterface () = default;
	
	/*Set conference factory address.
	 *If set, Conference is created as an Adhoc conference on a remote conferencing server. Conference state is CreationPending until conference is instanciated and conference Id available. State is then transitionned to Created.
	 *If not set the conference is instanciated with a local focus. In this case conferenceId must be set.
	 * @param[in] Address of the conference factory (ex: sip:conference-factory@conf.linphone.org).
	 */
	virtual void setConferenceFactoryAddress (const Address &address) = 0;
	
	/*Set focus address of this conference. If set, the Conference is created as an Adhoc conference from a remote conferencing server
	 * @param[in]  The Address of the conference focus.
	 **/
	virtual  void setConferenceId (const Address conferenceId) = 0;
	/*
	* Set the subject of this conference. If not focus,  this operation is only available if the local participant  #getMe() is admin.
	* @param[in] subject The new subject to set for the chat room
	*/
	virtual void setSubject (const std::string &subject) = 0;
	
	/*
	*Set participant representing myself in this Conference.
	*If set this participant is added to the conference
	* @param[in]  participantAddress of the conference focus.
	*/
	virtual void setMe (const IdentityAddress &participantAddress) = 0;
	
	/*
	* Enable audio media type for a conference
	* @param enable If true, audio will be enabled during conference
	*/
	virtual void  enableAudio(bool enable) = 0;

	/*
	* Enable video media type for a conference
	* @param enable If true, video will be enabled during conference
	*/
	virtual void  enableVideo(bool enable) = 0;

	/*
	* Enable chat media type for a conference
	* @param enable If true, chat will be enabled during conference
	*/
	virtual void  enableChat(bool enable) = 0;
};


/***********************************************************************************************************************/
 /* Conference object creation
 ** Conference object can be either created at  initiative user application using fonction ConferenceFactoryInterface::createConference
 */

class LINPHONE_PUBLIC ConferenceFactoryInterface {
	/*
	* Create a conference object with an initial list of participant based on the provided conference parameters
	 
	*/
	std::shared_ptr<ConferenceInterface>& createConference(const std::shared_ptr<ConferenceParamsInterface> &params,
	const std::list<IdentityAddress> &participants);
};

//typedef enum _LinphoneConferenceState {} LinphoneConferenceState; // same as ConferenceInterface::State

/*
 * Callback prototype telling that a #LinphoneConference state has changed.
 * When a call from /To a focus is in Connected State, a new conferencing object is instanciated.
 * @param[in] lc #LinphoneCore object
 * @param[in] Conference The #LinphoneConference object for which the state has changed
 */
//typedef void (*LinphoneCoreCbsConferenceStateChangedCb) (LinphoneCore *lc, LinphoneConference *conf, LinphoneConferenceState state);

/*
 * Base class for audio/video conference.
 */

class Conference : public bellesip::HybridObject<LinphoneConference, Conference>, public ConferenceInterface {
public:
	Conference(LinphoneCore *core, const ConferenceParams *params = NULL);
	virtual ~Conference() {}

	const ConferenceParams &getCurrentParams() const {return *m_currentParams;}

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) = 0;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::MediaConference::ConferenceInterface::addParticipant;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override = 0;
	virtual bool addParticipant (const LinphonePrivate::IdentityAddress &participantAddress) override;
	virtual bool addParticipants (const std::list<IdentityAddress> &addresses) override;

	virtual int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) = 0;
	virtual int removeParticipant(const LinphoneAddress *uri) = 0;
	virtual bool removeParticipant (const std::shared_ptr<LinphonePrivate::Participant> &participant) override;
	virtual bool removeParticipants (const std::list<std::shared_ptr<LinphonePrivate::Participant>> &participants) override;

	virtual int updateParams(const ConferenceParams &params) = 0;
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
	void setStateChangedCallback(LinphoneConferenceStateChangedCb cb, void *userData) {
		m_stateChangedCb = cb;
		m_userData = userData;
	}

	virtual const Address &getConferenceId () const override;

	virtual const std::string &getSubject () const override;

	virtual void setSubject (const std::string &subject) override;

	virtual void setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) override;

	virtual void join (const IdentityAddress &participantAddress) override;

	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::MediaConference::ConferenceInterface::findParticipant;
	virtual std::shared_ptr<LinphonePrivate::Participant> findParticipant (const IdentityAddress &participantAddress) const override;

	virtual int getParticipantCount () const override;

	virtual std::shared_ptr<LinphonePrivate::Participant> getMe () const override;

	virtual bool update(const ConferenceParamsInterface &newParameters) override;

protected:
	void setState(LinphoneConferenceState state);
	std::shared_ptr<LinphonePrivate::Participant> findParticipant(const std::shared_ptr<LinphonePrivate::Call> call) const;
	std::shared_ptr<LinphonePrivate::Participant> findParticipant(const LinphoneAddress *uri) const;

protected:
	std::string m_conferenceID;
	LinphoneCore *m_core;
	std::list<std::shared_ptr<LinphonePrivate::Participant>> m_participants;

	// Temporary member to store participant,call pairs
	// TODO: Remove once conference merge is finished
	std::map<std::shared_ptr<LinphonePrivate::Participant>, std::shared_ptr<LinphonePrivate::Call>> m_callTable;
	std::shared_ptr<ConferenceParams> m_currentParams;
	LinphoneConferenceState m_state;
	LinphoneConferenceStateChangedCb m_stateChangedCb = nullptr;
	void *m_userData = nullptr;
};


/*
 * Class for an audio/video conference running locally.
 */
class LocalConference: public Conference {
public:
	LocalConference(LinphoneCore *core, const ConferenceParams *params = NULL);
	virtual ~LocalConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::MediaConference::ConferenceInterface::addParticipant;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;

	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::MediaConference::Conference::removeParticipant;
	virtual int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual int removeParticipant(const LinphoneAddress *uri) override;
	virtual int updateParams(const ConferenceParams &params) override;
	virtual int terminate() override;

	virtual int enter() override;
	virtual void leave() override;
	virtual bool isIn() const override;

	virtual int startRecording(const char *path) override;
	virtual int stopRecording() override;
	
	virtual AudioControlInterface * getAudioControlInterface() const override;
	virtual VideoControlInterface * getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	virtual void addListener(std::shared_ptr<ConferenceListenerInterface> &listener)  override;
	
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
class RemoteConference: public Conference {
public:
	RemoteConference(LinphoneCore *core, const ConferenceParams *params = NULL);
	virtual ~RemoteConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::MediaConference::ConferenceInterface::addParticipant;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::MediaConference::Conference::removeParticipant;
	virtual int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call) override {
		return -1;
	}
	virtual int removeParticipant(const LinphoneAddress *uri) override;
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
	virtual int updateParams(const ConferenceParams &params) override;
	virtual AudioControlInterface * getAudioControlInterface() const override;
	virtual VideoControlInterface * getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	virtual void addListener(std::shared_ptr<ConferenceListenerInterface> &listener)  override;
	
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
