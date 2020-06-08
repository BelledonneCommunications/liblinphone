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

#ifndef _L_CONFERENCE_INTERFACE_H_
#define _L_CONFERENCE_INTERFACE_H_

#include <list>
#include "event-log/events.h"
#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceAvailableMediaEvent;
class ConferenceListenerInterface;
class IdentityAddress;
class CallSessionParams;
class Participant;
class ConferenceFactoryInterface;
class ConferenceParamsInterface;

/*
 * This interface allow an application to manage a multimedia conference. Implementation follows 2 mains rfc4579 ( SIP Call Control - Conferencing for User Agents) and rfc4575 ( A Session Initiation Protocol (SIP) Event Package for Conference State). It can be  either a Focus user agent (I.E local conference) or belong to a remote focus user agent (I.E remote conference). <br>
*
*	<br>
* 	Conference is instanciated with ConferenceParams and list of initial participants. ConferenceParams allows to choose beetween local or remote conference, and to set initial parameters.
 *	A conference is created either by the focus or with a remote focus. <br>
 */
class LINPHONE_PUBLIC ConferenceInterface {
public:

	// TODO: Start Delete

	virtual const ConferenceId &getConferenceId () const = 0;
	virtual bool addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) = 0;

	virtual bool addParticipants (const std::list<IdentityAddress> &addresses, const CallSessionParams *params, bool hasMedia) = 0;

	virtual void join () = 0;
	// TODO: End Delete



	/**
	 *Conference live cycle, specilly creation and termination requires interractions between user application and focus user agent.
	 * State is used to indicate the current state of a Conference.
	 */
//	enum class State{
//		None, /**< Initial state */
//		Instantiated, /**< Conference is now instantiated participants can be added, but no focus address is ready yet */
//		CreationPending, /**< If not focus,  creation request was sent to the server, else conference id allocation is ongoing */
//		Created, /**<  Conference was created with a conference id. Communication can start*/
//		CreationFailed, /**< Creation failed */
//		TerminationPending, /**< Wait for Conference termination */
//		Terminated, /**< Conference exists on server but not in local //fixme jehan creuser ce point */
//		TerminationFailed, /**< Conference termination failed */
//		Deleted /**< Conference is deleted on the server //fixme jehan creuser ce point  */
//	};

	virtual ~ConferenceInterface () = default;

	/*
	 *Listener reporting events for this conference. Use this function mainly to add listener when conference is created at the initiative of the focus.
	 */
	virtual void addListener(std::shared_ptr<ConferenceListenerInterface> listener) = 0;
	
	/*
	 Get the current State of this conference
	 */
//	virtual State getState () const = 0;

	/*
	 Get the conference ID of this conference.
	 @return The Address of the conference.
	 **/
	//virtual const Address &getConferenceId () const = 0;
	virtual const ConferenceAddress &getConferenceAddress () const = 0;

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
	virtual void setParticipantAdminStatus (const std::shared_ptr<Participant> &participant, bool isAdmin) = 0;

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
	virtual bool addParticipant (std::shared_ptr<Call> call) = 0;

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
	virtual std::shared_ptr<Participant> findParticipant (const IdentityAddress &participantAddress) const = 0;

	/*
	* Get the number of participants in this conference (that is without ourselves).
	* @return The number of participants in this conference
	*/
	virtual int getParticipantCount () const = 0;

	/*
	* Get the list of participants in this conference  (that is without ourselves).
	* @return \std::list<std::shared_ptr<Participant>>
	*/
	virtual const std::list<std::shared_ptr<Participant>> &getParticipants () const = 0;
	
	/*
	* Get the participant representing myself in this Conference (I.E local participant).<br>
	*Local participant behavior depends on weither the focus is local or remote. <br>
	*<b>Local focus case: </b><br>
	*Local participant is not mandatory. From value taken during conference managements is focus address.
	<b>Remote focus case: </b><br>
	*Local participant is the Participant of this conference used as From when operations are performed like subject change or participant management. local participant is not included in the of participant returned by function. Local participant is mandatory to create a remote conference conference.
	* @return The participant representing myself in the conference.
	*/
	virtual std::shared_ptr<Participant> getMe () const = 0;

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
	virtual bool removeParticipant (const std::shared_ptr<Participant> &participant) = 0;

	/*
	 *Remove a list of participant from this conference.<br>
	 * @param[in] participants The addresses to search in the list of participants of this conference.
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool removeParticipants (const std::list<std::shared_ptr<Participant>> &participants) = 0;

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
	//virtual bool update(const ConferenceParamsInterface &newParameters) = 0;
	virtual bool update(const ConferenceParamsInterface &newParameters) { return false; };

};

class LINPHONE_PUBLIC ConferenceListenerInterface {
public:
	virtual ~ConferenceListenerInterface () = default;

	/*
	 @return ConferenceInterface the listerner is attached.
	 */
	std::shared_ptr<ConferenceInterface> getConference() {return nullptr;}

	/*
	 * Notify Conference state changes.
	 ** @param[in] newState the new state of this conference.
	 */
//	virtual void onStateChanged (ConferenceInterface::State newState) {}
	
	/*
	 * This fonction is called each time a full state notification is receied from the focus.
	 */
	virtual void onFullStateReceived () {}
	
	/*
	 * This fonction is called each time a new participant is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant.
	 */
	virtual void onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event) {}

	/*
	 * This fonction is called each time a new participant is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed participant.
	 */
	virtual void onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event) {}

	/*
	 * This fonction is called each time a new admin is set by the focus after full state notification.
	 * @param[in] event informations related to the new admin participant.
	 */
	virtual void onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event) {}

	/*
	 * This fonction is called each time a new subject is set by the focus after full state notification.
	 * @param[in] event informations related to the new subject.
	 */
	virtual void onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event) {}

	/*
	 * This fonction is called each time list of available media is modified by the focus after full state notification.
	 * @param[in] event informations related to the new subject.
	 */
	virtual void onAvailableMediaChanged (const std::shared_ptr<ConferenceAvailableMediaEvent> &event) {}
	
	/*
	* This fonction is called each time a new participant device is added by the focus after full state notification.
	* @param[in] event informations related to the added participant's device.
	*/
	virtual void onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) {}
	/*
	* This fonction is called each time a new participant device is removed by the focus after full state notification.
	* @param[in] event informations related to the removed device's participant.
	*/
	virtual void onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) {}
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
	virtual void setConferenceAddress (const Address conferenceAddress) = 0;

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
 * Event used to repport media availability changes from a conference.
 **/
class ConferenceAvailableMediaEvent :  public EventLog {
	/*
	 * Can be audio, video, text
	 *@return list of available media type for this conference;
	 */
	const std::list<std::string> &getAvailableMediaType () const;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_INTERFACE_H_
