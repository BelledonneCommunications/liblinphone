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
	virtual const ConferenceId &getConferenceId () const = 0;
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

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_INTERFACE_H_
