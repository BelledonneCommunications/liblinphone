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

#ifndef _L_CONFERENCE_INTERFACE_H_
#define _L_CONFERENCE_INTERFACE_H_

#include <list>

#include <bctoolbox/defs.h>

#include "event-log/events.h"
#include "linphone/enums/conference-enums.h"
#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Call;
class ConferenceListenerInterface;
class Address;
class CallSessionParams;
class Participant;
class ConferenceFactoryInterface;
class ConferenceParamsInterface;

/*
 * This interface allow an application to manage a multimedia conference. Implementation follows 2 mains rfc4579 ( SIP
 *Call Control - Conferencing for User Agents) and rfc4575 ( A Session Initiation Protocol (SIP) Event Package for
 *Conference State). It can be  either a Focus user agent (I.E local conference) or belong to a remote focus user agent
 *(I.E client conference). <br>
 *
 *	<br>
 * 	Conference is instanciated with ConferenceParams and list of initial participants. ConferenceParams allows to choose
 *beetween local or client conference, and to set initial parameters. A conference is created either by the focus or
 *with a remote focus. <br>
 */
class LINPHONE_PUBLIC ConferenceInterface {
public:
	virtual const ConferenceId &getConferenceId() const = 0;

	/**
	 *Conference live cycle, specilly creation and termination requires interractions between user application and focus
	 *user agent. State is used to indicate the current state of a Conference.
	 */
	enum class State {
		None = LinphoneConferenceStateNone,                 /**< Initial state */
		Instantiated = LinphoneConferenceStateInstantiated, /**< Conference is now instantiated participants can be
		                                                       added, but no focus address is ready yet */
		CreationPending = LinphoneConferenceStateCreationPending, /**< If not focus,  creation request was sent to the
		                                                             server, else conference id allocation is ongoing */
		Created =
		    LinphoneConferenceStateCreated, /**<  Conference was created with a conference id. Communication can start*/
		CreationFailed = LinphoneConferenceStateCreationFailed,         /**< Creation failed */
		TerminationPending = LinphoneConferenceStateTerminationPending, /**< Wait for Conference termination */
		Terminated = LinphoneConferenceStateTerminated, /**< Conference exists on server but not in local //fixme jehan
		                                                   creuser ce point */
		TerminationFailed = LinphoneConferenceStateTerminationFailed, /**< Conference termination failed */
		Deleted =
		    LinphoneConferenceStateDeleted, /**< Conference is deleted on the server //fixme jehan creuser ce point  */
	};

	// casting to int to get rid of the enum compare warning.
	// Here we are comparing two enums serving the same purpose
	static_assert((int)ConferenceInterface::State::Deleted == (int)LinphoneConferenceStateDeleted,
	              "LinphoneConferenceState and ConferenceInterface::State are not synchronized, fix this !");

	virtual ~ConferenceInterface() = default;

	/*
	 *Listener reporting events for this conference. Use this function mainly to add a listener when conference is
	 *created at the initiative of the focus.
	 */
	virtual void addListener(std::shared_ptr<ConferenceListenerInterface> listener) = 0;

	/*
	 *Listener reporting events for this conference. Use this function mainly to remove a listener when conference is
	 *created at the initiative of the focus.
	 */
	virtual void removeListener(std::shared_ptr<ConferenceListenerInterface> listener) = 0;

	/*
	 Get the current State of this conference
	 */
	virtual State getState() const = 0;

	/*
	 * Set the state of this conference or chat room.
	 * @param[in] state The new state to set for the chat room or conference
	 */
	virtual void setState(ConferenceInterface::State state) = 0;

	/*
	 Get the conference ID of this conference.
	 @return The Address of the conference.
	 **/
	virtual std::shared_ptr<Address> getConferenceAddress() const = 0;

	/*
	 * Get the subject of this conference
	 * @return The subject of the chat room in UTF8
	 */
	virtual const std::string &getUtf8Subject() const = 0;

	/*
	 * Set the subject of this conference in UTF8. If not focus,  this operation is only available if the local
	 * participant  #getMe() is admin.
	 * @param[in] subject The new subject to set for the chat room
	 */
	virtual void setUtf8Subject(const std::string &subject) = 0;

	/*
	 * Change the admin status of a participant of this conference (If not focus,  This operation is only available if
	 * the local participant  #getMe() is admin). All participants are notified of change of the admin.
	 * @param[in] participant The Participant for which to change the admin status
	 * @param[in] isAdmin A boolean value telling whether the participant should now be an admin or not
	 */
	virtual void setParticipantAdminStatus(const std::shared_ptr<Participant> &participant, bool isAdmin) = 0;

	/* This fonction add a participant to this conference.<br>
	 *Dependending if focus user agent is local or remote, behavior is defferent. <br>
	 *In both case, a call with a from address corresponding to the participant address is searched first. <br>
	 *<br>
	 <b>Local focus case: </b><br>
	 *If both found and unique found call is updated with 'isfocus' feature parameter in the Contact header field. If
	 not found, focus add participant following the "INVITE: Adding a Participant by the Focus - Dial-Out" as described
	 by RFC4579. <br> <b>Remote focus case: </b><br> *This operation is only available if the local participant #getMe()
	 is admin <br>. *If found, call is transferred to the focus following " REFER: Requesting a User to Dial in to a
	 Conference Using a Conference URI" as described by RFC4579. <br> *if not found, add participant following " REFER:
	 Requesting a Focus to Add a New Resource to a Conference(Dial Out to a New Participant)" as described by RFC4579.
	 <br>
	 @param[in] participantAddress The address of the participant to add to this Conference.
	*/
	virtual bool addParticipant(const std::shared_ptr<Address> &participantAddress) = 0;

	/*
	 * Same as function addParticipant(const std::shared_ptr<Address> &participantAddress), except that call to add is
	 * passed by the user application.
	 * @param[in] call to be added to the conference.
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool addParticipant(std::shared_ptr<Call> call) = 0;

	/*
	 * Same as function addParticipant(const std::shared_ptr<Address> &participantAddress), except the participant
	 * information to add is passed by the user application.
	 * @param[in] participant information to be added to the conference.
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool addParticipant(const std::shared_ptr<ParticipantInfo> &info) = 0;

	/*
	 * Add several participants at once.
	 * @param[in] addresses
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool addParticipants(const std::list<std::shared_ptr<Address>> &addresses) = 0;

	/*
	 * Add local participant to this conference, this fonction is only available for local focus. Behavior is the same
	 * as
	 */
	virtual void join(const std::shared_ptr<Address> &participantAddress) = 0;

	/*
	 * Find a participant  from  its address.
	 * @param[in] participantAddress The address to search in the list of participants of the chat room
	 * @return The participant if found, NULL otherwise.
	 */
	virtual std::shared_ptr<Participant>
	findParticipant(const std::shared_ptr<const Address> &participantAddress) const = 0;

	/*
	 * Get the number of participants in this conference (that is without ourselves).
	 * @return The number of participants in this conference
	 */
	virtual int getParticipantCount() const = 0;

	/*
	 * Get the list of participants in this conference  (that is without ourselves).
	 * @return \std::list<std::shared_ptr<Participant>>
	 */
	virtual const std::list<std::shared_ptr<Participant>> &getParticipants() const = 0;

	/*
	 * Get the list of participant devices in this conference including ourself if in conference.
	 * @return \std::list<std::shared_ptr<ParticipantDevice>>
	 */
	virtual std::list<std::shared_ptr<ParticipantDevice>> getParticipantDevices(bool includeMe = true) const = 0;

	/*
	* Get the participant representing myself in this Conference (I.E local participant).<br>
	*Local participant behavior depends on weither the focus is local or remote. <br>
	*<b>Local focus case: </b><br>
	*Local participant is not mandatory. From value taken during conference managements is focus address.
	<b>Remote focus case: </b><br>
	*Local participant is the Participant of this conference used as From when operations are performed like subject
	change or participant management. local participant is not included in the of participant returned by function.
	Local participant is mandatory to create a client conference conference.
	* @return The participant representing myself in the conference.
	*/
	virtual std::shared_ptr<Participant> getMe() const = 0;

	/*
	 *Remove a participant from this conference.
	 *Dependending if  Focus user agent is local or remote,  behavior is defferent.<br>
	 *<b>Local focus case: </b><br>
	 *Calls corresponding to all devices of this participant are terminated. A new admin is designated if removed
	participant was the only admin<br> <b>Remote focus case: </b><br> *This operation is only available if the local
	participant  #getMe() is admin <br>. *Remove participant following "REFER with BYE: Requesting That the Focus Remove
	a Participant from a Conference"  as described by RFC4579.<br>
	 * @param[in] participantAddress The address to search and remove in the list of participants of this conference
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool removeParticipant(const std::shared_ptr<Participant> &participant) = 0;

	/*
	 *Remove a list of participant from this conference.<br>
	 * @param[in] participants The addresses to search in the list of participants of this conference.
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool removeParticipants(const std::list<std::shared_ptr<Participant>> &participants) = 0;

	/*
	 * Remove the local participant from this conference. <br>
	 *<b>Local focus case: </b><br>
	 * Remainning participant are notified. Conference state does not change.
	 *<b>Remote focus case: </b><br>
	 * The local participant is removed from the conference as any other participant.
	 * Conference is transitioned to state TerminationPending until removal is acknowled by the server.
	 */
	virtual void leave() = 0;

	/*
	 * Call to update conference parameters like media type. If not focus,  this operation is only available if the
	 * local participant  #getMe() is admin. <br>
	 * @param[in] new parameter to applie for this conference.
	 * @return True if everything is OK, False otherwise
	 */
	virtual bool update(const ConferenceParamsInterface &newParameters) = 0;
};

class LINPHONE_PUBLIC ConferenceListenerInterface {
public:
	virtual ~ConferenceListenerInterface() = default;

	/*
	 @return ConferenceInterface the listerner is attached.
	 */
	//	std::shared_ptr<ConferenceInterface> getConference() {return nullptr;}

	/*
	 * Notify Conference state changes.
	 ** @param[in] newState the new state of this conference.
	 */
	virtual void onStateChanged(BCTBX_UNUSED(ConferenceInterface::State newState)) {
	}

	/*
	 * This fonction is called each time a full state notification is receied from the focus.
	 */
	virtual void onFullStateReceived() {
	}

	/*
	 * This fonction is called each time the list of allowed participant is changed while the conference is active
	 * @param[in] event informations related to the change of the participant allowed to join the conference. @notnil
	 */
	virtual void onAllowedParticipantListChanged(BCTBX_UNUSED(const std::shared_ptr<ConferenceNotifiedEvent> &event)) {
	}

	/*
	 * This fonction is called each time a new participant is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant. @notnil
	 * @param[in] participant participant added to conference or chat room. @notnil
	 */
	virtual void onParticipantAdded(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantEvent> &event),
	                                BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	}

	/*
	 * This fonction is called each time a new participant is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed participant. @notnil
	 * @param[in] participant participant removed from conference or chat room. @notnil
	 */
	virtual void onParticipantRemoved(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantEvent> &event),
	                                  BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	}

	/*
	 * This fonction is called each time a participant device changes its available media
	 * @param[in] event informations related to the device's participant.
	 * @param[in] device participant device that changed its media capabilities
	 */
	virtual void onParticipantDeviceMediaCapabilityChanged(
	    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
	    BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)){};

	/*
	 * This fonction is called each time a participant device changes its available media
	 * @param[in] event informations related to the device's participant.
	 * @param[in] device participant device that changed its media availabilities
	 */
	virtual void onParticipantDeviceMediaAvailabilityChanged(
	    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
	    BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)){};

	/*
	 * This fonction is called each time a participant device state changes
	 * @param[in] event informations related to the device's participant whose state changed. @notnil
	 * @param[in] device participant device whose state changed. @notnil
	 */
	virtual void
	    onParticipantDeviceStateChanged(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
	                                    BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)){};

	/*
	 * This fonction is called each time a participant device starts or stops screen sharing
	 * @param[in] event informations related to the device's participant who starts or stops screen sharing. @notnil
	 * @param[in] device participant device who starts or stops screen sharing @notnil
	 */
	virtual void onParticipantDeviceScreenSharingChanged(
	    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
	    BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)){};

	/*
	 * This fonction is called each time a participant device changes the ephemeral mode
	 * @param[in] event informations related to the device's participant.
	 */
	virtual void onEphemeralModeChanged(BCTBX_UNUSED(const std::shared_ptr<ConferenceEphemeralMessageEvent> &event)){};

	/*
	 * This fonction is called each time a participant device enables or disables ephemeral messages when they are
	 * managed by admins
	 * @param[in] event informations related to the device's participant.
	 */
	virtual void
	    onEphemeralMessageEnabled(BCTBX_UNUSED(const std::shared_ptr<ConferenceEphemeralMessageEvent> &event)){};

	/*
	 * This fonction is called each time a participant device changes the ephemeral lifetime when ephemeral messages are
	 * managed by admins
	 * @param[in] event informations related to the device's participant.
	 */
	virtual void
	    onEphemeralLifetimeChanged(BCTBX_UNUSED(const std::shared_ptr<ConferenceEphemeralMessageEvent> &event)){};

	/*
	 * This fonction is called each time a participant changes its role in the conference after full state notification.
	 * @param[in] event informations related to the participant new role. @notnil
	 * @param[in] participant participant whose role changed. @notnil
	 */
	virtual void onParticipantSetRole(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantEvent> &event),
	                                  BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	}

	/*
	 * This fonction is called each time a new admin is set by the focus after full state notification.
	 * @param[in] event informations related to the new admin participant. @notnil
	 * @param[in] participant participant whose admin status changed. @notnil
	 */
	virtual void onParticipantSetAdmin(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantEvent> &event),
	                                   BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	}

	/*
	 * This fonction is called each time a new subject is set by the focus after full state notification.
	 * @param[in] event informations related to the new subject. @notnil
	 */
	virtual void onSubjectChanged(BCTBX_UNUSED(const std::shared_ptr<ConferenceSubjectEvent> &event)) {
	}

	/*
	 * This function is called each time a participant device starts or stops speaking.
	 * @param[in] device the participant device.
	 * @param[in] isSpeaking true if participant device is currently speaking, false otherwise.
	 */
	virtual void onParticipantDeviceIsSpeakingChanged(BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device),
	                                                  BCTBX_UNUSED(bool isSpeaking)) {
	}

	/*
	 * This function is called each time a participant device mutes or unmutes itself.
	 * @param[in] device the participant device.
	 * @param[in] isMuted true if participant device is currently muted, false otherwise.
	 */
	virtual void onParticipantDeviceIsMuted(BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device),
	                                        BCTBX_UNUSED(bool isMuted)) {
	}

	/*
	 * This fonction is called each time list of available media is modified by the focus after full state notification.
	 * @param[in] event informations related to the available media. @notnil
	 */
	virtual void onAvailableMediaChanged(BCTBX_UNUSED(const std::shared_ptr<ConferenceAvailableMediaEvent> &event)) {
	}

	/*
	 * This fonction is called each time a new participant device is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant's device. @notnil
	 * @param[in] device participant device added to the conference or chat room. @notnil
	 */
	virtual void onParticipantDeviceAdded(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
	                                      BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	}

	/*
	 * This fonction is called each time a new participant device is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed device's participant. @notnil
	 * @param[in] device participant device removed from the conference or chat room. @notnil
	 */
	virtual void
	onParticipantDeviceRemoved(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
	                           BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	}

	/*
	 * This fonction is called each time a new participant device that is not in the allowed participants'list calls a
	 * closed-list conference
	 * @param[in] event informations related to the removed device's participant. @notnil
	 * @param[in] device participant device that is not in the allowed participants'list. @notnil
	 */
	virtual void
	onParticipantDeviceJoiningRequest(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
	                                  BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	}

	/*
	 * This fonction is called each time a participant device is being currently displayed as active speaker.
	 * @param[in] device participant device currently being displayed as active speaker. @notnil
	 */
	virtual void onActiveSpeakerParticipantDevice(BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	}
};

/***********************************************************************************************************************/
/* Conference object creation
** Conference object can be either created at  initiative user application using fonction
*ConferenceFactoryInterface::createConference
*/

class LINPHONE_PUBLIC ConferenceFactoryInterface {
	/*
	 * Create a conference object with an initial list of participant based on the provided conference parameters
	 * @param params conference parameters @notnil
	 * @param participants initial list of participants
	 */
	std::shared_ptr<ConferenceInterface> &createConference(const std::shared_ptr<ConferenceParamsInterface> &params,
	                                                       const std::list<std::shared_ptr<Address>> &participants);
};

std::ostream &operator<<(std::ostream &lhs, ConferenceInterface::State e);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_INTERFACE_H_
