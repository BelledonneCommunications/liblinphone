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

#ifndef _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_
#define _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_

#include <string>

#include "linphone/types.h"

#include "address/address.h"
#include "core/core-accessor.h"

#include "conference/conference-interface.h"
#include "conference/conference-id.h"
#include "xml/conference-info.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceId;
class ConferenceParticipantDeviceEvent;
class ConferenceParticipantEvent;
class ConferenceSubjectEvent;
class ConferenceAvailableMediaEvent;
class Participant;
class ParticipantDevice;

class LINPHONE_PUBLIC LocalConferenceEventHandler : public ConferenceListenerInterface {
friend class LocalConferenceListEventHandler;
#ifdef LINPHONE_TESTER
	friend class Tester;
#endif
public:
	LocalConferenceEventHandler (Conference *conference);

	void subscribeReceived (LinphoneEvent *lev, bool oneToOne = false);
	void subscriptionStateChanged (LinphoneEvent *lev, LinphoneSubscriptionState state);

	std::string getNotifyForId (int notifyId, bool oneToOne = false);

//protected:
	void notifyFullState (const std::string &notify, const std::shared_ptr<ParticipantDevice> &device);
	void notifyAllExcept (const std::string &notify, const std::shared_ptr<Participant> &exceptParticipant);
	void notifyAll (const std::string &notify);
	std::string createNotifyFullState (bool oneToOne = false);
	std::string createNotifyMultipart (int notifyId);
	std::string createNotifyParticipantAdded (const Address &addr);
	std::string createNotifyParticipantAdminStatusChanged (const Address &addr, bool isAdmin);
	std::string createNotifyParticipantRemoved (const Address &addr);
	std::string createNotifyParticipantDeviceAdded (const Address &addr, const Address &gruu);
	std::string createNotifyParticipantDeviceRemoved (const Address &addr, const Address &gruu);
	std::string createNotifySubjectChanged ();

	static void notifyResponseCb (const LinphoneEvent *ev);

	/*
	 * This fonction is called each time a full state notification is receied from the focus.
	 */
	virtual void onFullStateReceived () override;
	
	/*
	 * This fonction is called each time a new participant is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant.
	 */
	virtual void onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event) override;

	/*
	 * This fonction is called each time a new participant is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed participant.
	 */
	virtual void onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event) override;

	/*
	 * This fonction is called each time a new admin is set by the focus after full state notification.
	 * @param[in] event informations related to the new admin participant.
	 */
	virtual void onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event) override;

	/*
	 * This fonction is called each time a new subject is set by the focus after full state notification.
	 * @param[in] event informations related to the new subject.
	 */
	virtual void onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event) override;

	/*
	 * This fonction is called each time list of available media is modified by the focus after full state notification.
	 * @param[in] event informations related to the new subject.
	 */
	virtual void onAvailableMediaChanged (const std::shared_ptr<ConferenceAvailableMediaEvent> &event) override;
	
	/*
	* This fonction is called each time a new participant device is added by the focus after full state notification.
	* @param[in] event informations related to the added participant's device.
	*/
	virtual void onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) override;
	/*
	* This fonction is called each time a new participant device is removed by the focus after full state notification.
	* @param[in] event informations related to the removed device's participant.
	*/
	virtual void onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) override;

private:

	Conference *conf = nullptr;

	std::string createNotify (Xsd::ConferenceInfo::ConferenceType confInfo, bool isFullState = false);
	std::string createNotifySubjectChanged (const std::string &subject);
	void notifyParticipant (const std::string &notify, const std::shared_ptr<Participant> &participant);
	void notifyParticipantDevice (const std::string &notify, const std::shared_ptr<ParticipantDevice> &device, bool multipart = false);

	L_DISABLE_COPY(LocalConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_
