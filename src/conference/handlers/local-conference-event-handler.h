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
#include "conference/conference-listener.h"
#include "conference/conference-id.h"
#include "xml/conference-info-linphone-extension.h"
#include "xml/conference-info.h"
#include <memory>

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceId;
class ConferenceParticipantDeviceEvent;
class ConferenceParticipantEvent;
class ConferenceSubjectEvent;
class Participant;
class ParticipantDevice;

class LINPHONE_PUBLIC LocalConferenceEventHandler : public ConferenceListenerInterface {
friend class LocalConferenceListEventHandler;
#ifdef LINPHONE_TESTER
	friend class Tester;
#endif
public:
	static Xsd::ConferenceInfo::MediaStatusType mediaDirectionToMediaStatus (LinphoneMediaDirection direction);
	LocalConferenceEventHandler (Conference *conference, ConferenceListener* listener = nullptr);

	void subscribeReceived (LinphoneEvent *lev);
	void subscriptionStateChanged (LinphoneEvent *lev, LinphoneSubscriptionState state);

	std::string getNotifyForId (int notifyId, LinphoneEvent *lev);

//protected:
	void notifyFullState (const std::string &notify, const std::shared_ptr<ParticipantDevice> &device);
	void notifyAllExcept (const std::string &notify, const std::shared_ptr<Participant> &exceptParticipant);
	void notifyAllExceptDevice(const std::string &notify, const std::shared_ptr<ParticipantDevice> &exceptDevice);
	void notifyAll (const std::string &notify);
	std::string createNotifyFullState (LinphoneEvent * lev);
	std::string createNotifyMultipart (int notifyId);
	std::string createNotifyParticipantAdded (const Address & pAddress);
	std::string createNotifyParticipantAdminStatusChanged (const Address & pAddress, bool isAdmin);
	std::string createNotifyParticipantRemoved (const Address & pAddress);
	std::string createNotifyParticipantDeviceAdded (const Address & pAddress, const Address & dAddress);
	std::string createNotifyParticipantDeviceRemoved (const Address & pAddress, const Address & dAddress);
	std::string createNotifyParticipantDeviceMediaCapabilityChanged (const Address & pAddress, const Address & dAddress);
	std::string createNotifyParticipantDeviceStatusChanged (const Address & pAddress, const Address & dAddress);
	std::string createNotifyAvailableMediaChanged (const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities);
	std::string createNotifySubjectChanged ();

	static void notifyResponseCb (const LinphoneEvent *ev);

	/*
	 * This fonction is called each time a full state notification is receied from the focus.
	 */
	virtual void onFullStateReceived () override;
	
	/*
	 * This fonction is called each time a new participant is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant.
	 * @param[in] participant participant added to conference or chat room.
	 */
	virtual void onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new participant is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed participant.
	 * @param[in] participant participant removed from conference or chat room.
	 */
	virtual void onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new admin is set by the focus after full state notification.
	 * @param[in] event informations related to the new admin participant.
	 * @param[in] participant participant whose admin status changed.
	 */
	virtual void onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;

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
	* @param[in] device participant device added to the conference or chat room.
	*/
	virtual void onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	* This fonction is called each time a new participant device is removed by the focus after full state notification.
	* @param[in] event informations related to the removed device's participant.
	* @param[in] device participant device removed from the conference or chat room.
	*/
	virtual void onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	* This fonction is called each time a participant device changes its available media
	* @param[in] event informations related to the device's participant.
	* @param[in] device participant device that changed its media capabilities
	*/
	virtual void onParticipantDeviceMediaCapabilityChanged (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	* This fonction is called each time a new participant device rejoins the conference after full state notification.
	* @param[in] event informations related to the device's participant who rejoined. @notnil
	* @param[in] device participant device who rejoined. @notnil
	*/
	virtual void onParticipantDeviceJoined (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	* This fonction is called each time a new participant device temporarely leaves the conference after full state notification.
	* @param[in] event informations related to the device's participant who temporarely left. @notnil
	* @param[in] device participant device who temporarely left. @notnil
	*/
	virtual void onParticipantDeviceLeft (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	* This fonction is called each time a participant device changes the ephemeral settings
	* @param[in] event informations related to the device's participant.
	*/
	virtual void onEphemeralLifetimeChanged (const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) override;

	/*
	* This fonction is called each time a participant device changes the ephemeral mode
	* @param[in] event informations related to the device's participant.
	*/
	virtual void onEphemeralModeChanged (const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) override;

	/*
	 * This fonction is called each time the conference transitions to a new state
	 * @param[in] state new state of the conference
	 */
	void onStateChanged (LinphonePrivate::ConferenceInterface::State state) override;

protected:

	Conference *conf = nullptr;
	ConferenceListener *confListener ;

private:

	std::string createNotify (Xsd::ConferenceInfo::ConferenceType confInfo, bool isFullState = false);
	std::string createNotifySubjectChanged (const std::string &subject);
	std::string createNotifyEphemeralLifetime (const long & lifetime);
	std::string createNotifyEphemeralMode (const EventLog::Type & type);
	void notifyParticipant (const std::string &notify, const std::shared_ptr<Participant> &participant);
	void notifyParticipantDevice (const std::string &notify, const std::shared_ptr<ParticipantDevice> &device, bool multipart = false);

	std::shared_ptr<Participant> getConferenceParticipant (const Address & address) const;

	void addMediaCapabilities(const std::shared_ptr<ParticipantDevice> & device, Xsd::ConferenceInfo::EndpointType & endpoint);
	void addEndpointStatus(const std::shared_ptr<ParticipantDevice> & device, Xsd::ConferenceInfo::EndpointType & endpoint);
	void addAvailableMediaCapabilities(const LinphoneMediaDirection audioDirection, const LinphoneMediaDirection videoDirection, const LinphoneMediaDirection textDirection, Xsd::ConferenceInfo::ConferenceDescriptionType & confDescr);

	L_DISABLE_COPY(LocalConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_
