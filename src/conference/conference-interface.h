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

class LINPHONE_PUBLIC ConferenceInterface {
public:
	virtual ~ConferenceInterface () = default;

	/*
	 *Listener reporting events for this conference. Use this function mainly to add listener when conference is created at the initiative of the focus.
	 */
	virtual void addListener(std::shared_ptr<ConferenceListenerInterface> listener) = 0;

	virtual bool addParticipant (
		const IdentityAddress &participantAddress,
		const CallSessionParams *params,
		bool hasMedia
	) = 0;
	virtual bool addParticipants (
		const std::list<IdentityAddress> &addresses,
		const CallSessionParams *params,
		bool hasMedia
	) = 0;
	virtual bool canHandleParticipants () const = 0;
	virtual std::shared_ptr<Participant> findParticipant (const IdentityAddress &participantAddress) const = 0;
	virtual const IdentityAddress &getConferenceAddress () const = 0;
	virtual std::shared_ptr<Participant> getMe () const = 0;
	virtual int getParticipantCount () const = 0;
	virtual const std::list<std::shared_ptr<Participant>> &getParticipants () const = 0;
	virtual const std::string &getSubject () const = 0;
	virtual void join () = 0;
	virtual void leave () = 0;
	virtual bool removeParticipant (const std::shared_ptr<Participant> &participant) = 0;
	virtual bool removeParticipants (const std::list<std::shared_ptr<Participant>> &participants) = 0;
	virtual void setParticipantAdminStatus (const std::shared_ptr<Participant> &participant, bool isAdmin) = 0;
	virtual void setSubject (const std::string &subject) = 0;
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

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_INTERFACE_H_
