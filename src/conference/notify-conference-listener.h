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

#ifndef _L_NOTIFY_CONFERENCE_LISTENER_H_
#define _L_NOTIFY_CONFERENCE_LISTENER_H_

#include "event-log/events.h"
#include "conference/conference-interface.h"

LINPHONE_BEGIN_NAMESPACE

namespace MediaConference{ // They are in a special namespace because of conflict of generic Conference classes in src/conference/*

class Conference;

}


class LINPHONE_PUBLIC NotifyConferenceListener : public ConferenceListenerInterface {
public:
	NotifyConferenceListener (MediaConference::Conference *conference) : conf(conference) {};
	virtual ~NotifyConferenceListener () = default;

	/*
	 * This fonction is called each time a new participant is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant. @notnil
	 * @param[in] participant participant added to conference or chat room. @notnil
	 */
	virtual void onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new participant is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed participant. @notnil
	 * @param[in] participant participant removed from conference or chat room. @notnil
	 */
	virtual void onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new admin is set by the focus after full state notification.
	 * @param[in] event informations related to the new admin participant. @notnil
	 * @param[in] participant participant whose admin status changed. @notnil
	 */
	virtual void onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new subject is set by the focus after full state notification.
	 * @param[in] event informations related to the new subject. @notnil
	 */
	virtual void onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event) override;

	/*
	* This fonction is called each time a new participant device is added by the focus after full state notification.
	* @param[in] event informations related to the added participant's device. @notnil
	* @param[in] device participant device added to the conference or chat room. @notnil
	*/
	virtual void onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	* This fonction is called each time a new participant device is removed by the focus after full state notification.
	* @param[in] event informations related to the removed device's participant. @notnil
	* @param[in] device participant device removed from the conference or chat room. @notnil
	*/
	virtual void onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * Notify Conference state changes.
	 ** @param[in] newState the new state of this conference.
	 */
	virtual void onStateChanged (ConferenceInterface::State newState) override;
	
private:

	MediaConference::Conference *conf;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_NOTIFY_CONFERENCE_LISTENER_H_
