/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#ifndef _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_
#define _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_

#include <memory>
#include <string>

#include "linphone/types.h"

#include "address/address.h"
#include "core/core-accessor.h"
#include "event/event-publish.h"
#include "event/event-subscribe.h"

#include "conference/conference-id.h"
#include "conference/conference-interface.h"
#include "conference/conference-listener.h"
#include "xml/conference-info-linphone-extension.h"
#include "xml/conference-info.h"

#include "content/content.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceId;
class ConferenceParticipantDeviceEvent;
class ConferenceParticipantEvent;
class ConferenceSubjectEvent;
class Participant;
class ParticipantDevice;

class LINPHONE_PUBLIC ServerConferenceEventHandler : public std::enable_shared_from_this<ServerConferenceEventHandler>,
                                                     public ConferenceListenerInterface {
	friend class ServerConferenceListEventHandler;
#ifdef LINPHONE_TESTER
	friend class Tester;
#endif
public:
	ServerConferenceEventHandler(std::shared_ptr<Conference> conf, ConferenceListener *listener = nullptr);

	void publishStateChanged(const std::shared_ptr<EventPublish> &ev, LinphonePublishState state);

	LinphoneStatus subscribeReceived(const std::shared_ptr<EventSubscribe> &ev);
	void subscriptionStateChanged(const std::shared_ptr<EventSubscribe> &ev, LinphoneSubscriptionState state);

	std::shared_ptr<Content> getNotifyForId(int notifyId, const std::shared_ptr<EventSubscribe> &ev);

	// protected:
	void notifyFullState(const std::shared_ptr<Content> &notify, const std::shared_ptr<ParticipantDevice> &device);
	void notifyAllExcept(const std::shared_ptr<Content> &notify, const std::shared_ptr<Participant> &exceptParticipant);
	void notifyAllExceptDevice(const std::shared_ptr<Content> &notify,
	                           const std::shared_ptr<ParticipantDevice> &exceptDevice);
	void notifyAll(const std::shared_ptr<Content> &notify);
	void notifyOnlyAdmins(const std::shared_ptr<Content> &notify);
	std::shared_ptr<Content> createNotifyFullState(const std::shared_ptr<EventSubscribe> &ev);
	std::shared_ptr<Content> createNotifyMultipart(int notifyId);

	// Conference
	std::string createNotifyAvailableMediaChanged(const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities);
	std::string createNotifySubjectChanged();

	// Participant
	std::string createNotifyParticipantAdded(const std::shared_ptr<Address> &pAddress);
	std::string createNotifyParticipantAdminStatusChanged(const std::shared_ptr<Address> &pAddress, bool isAdmin);
	std::string createNotifyParticipantRemoved(const std::shared_ptr<Address> &pAddress);

	// Participant device
	std::string createNotifyParticipantDeviceAdded(const std::shared_ptr<Address> &pAddress,
	                                               const std::shared_ptr<Address> &dAddress);
	std::string createNotifyParticipantDeviceRemoved(const std::shared_ptr<Address> &pAddress,
	                                                 const std::shared_ptr<Address> &dAddress);
	std::string createNotifyParticipantDeviceDataChanged(const std::shared_ptr<Address> &pAddress,
	                                                     const std::shared_ptr<Address> &dAddress);

	static void notifyResponseCb(LinphoneEvent *lev);

	/*
	 * This fonction is called each time a full state notification is receied from the focus.
	 */
	virtual void onFullStateReceived() override;

	/*
	 * This fonction is called each time a new participant is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant.
	 * @param[in] participant participant added to conference or chat room.
	 */
	virtual void onParticipantAdded(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new participant is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed participant.
	 * @param[in] participant participant removed from conference or chat room.
	 */
	virtual void onParticipantRemoved(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                  const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new admin is set by the focus after full state notification.
	 * @param[in] event informations related to the new admin participant.
	 * @param[in] participant participant whose admin status changed.
	 */
	virtual void onParticipantSetAdmin(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                   const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new subject is set by the focus after full state notification.
	 * @param[in] event informations related to the new subject.
	 */
	virtual void onSubjectChanged(const std::shared_ptr<ConferenceSubjectEvent> &event) override;

	/*
	 * This function is called each time a participant device starts or stops speaking.
	 * @param[in] device the participant device.
	 * @param[in] isSpeaking true if participant device is currently speaking, false otherwise.
	 */
	virtual void onParticipantDeviceIsSpeakingChanged(const std::shared_ptr<ParticipantDevice> &device,
	                                                  bool isSpeaking) override;

	/*
	 * This function is called each time a participant device mutes or unmutes itself.
	 * @param[in] device the participant device.
	 * @param[in] isMuted true if participant device is currently muted, false otherwise.
	 */
	virtual void onParticipantDeviceIsMuted(const std::shared_ptr<ParticipantDevice> &device, bool isMuted) override;

	/*
	 * This fonction is called each time list of available media is modified by the focus after full state notification.
	 * @param[in] event informations related to the new subject.
	 */
	virtual void onAvailableMediaChanged(const std::shared_ptr<ConferenceAvailableMediaEvent> &event) override;

	/*
	 * This fonction is called each time a new participant device that is not in the allowed participants'list calls a
	 * closed-list conference
	 * @param[in] event informations related to the removed device's participant. @notnil
	 * @param[in] device participant device that is not in the allowed participants'list. @notnil
	 */
	virtual void onParticipantDeviceJoiningRequest(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                               const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a new participant device is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant's device.
	 * @param[in] device participant device added to the conference or chat room.
	 */
	virtual void onParticipantDeviceAdded(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                      const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a new participant device is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed device's participant.
	 * @param[in] device participant device removed from the conference or chat room.
	 */
	virtual void onParticipantDeviceRemoved(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                        const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a participant device changes its available media
	 * @param[in] event informations related to the device's participant.
	 * @param[in] device participant device that changed its media capabilities
	 */
	virtual void
	onParticipantDeviceMediaCapabilityChanged(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                          const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a participant device state changes
	 * @param[in] event informations related to the device's participant whose state changed. @notnil
	 * @param[in] device participant device whose state changed. @notnil
	 */
	virtual void onParticipantDeviceStateChanged(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                             const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a participant device starts or stops screen sharing
	 * @param[in] event informations related to the device's participant who starts or stops screen sharing. @notnil
	 * @param[in] device participant device who starts or stops screen sharing @notnil
	 */
	virtual void onParticipantDeviceScreenSharingChanged(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                                     const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a participant device changes the ephemeral settings
	 * @param[in] event informations related to the device's participant.
	 */
	virtual void onEphemeralLifetimeChanged(const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) override;

	/*
	 * This fonction is called each time a participant device changes the ephemeral mode
	 * @param[in] event informations related to the device's participant.
	 */
	virtual void onEphemeralModeChanged(const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) override;

	/*
	 * This fonction is called each time the conference transitions to a new state
	 * @param[in] state new state of the conference
	 */
	void onStateChanged(LinphonePrivate::ConferenceInterface::State state) override;

	/*
	 * Notify which participant device is being currently displayed as active speaker.
	 * @param[in] device participant device currently being displayed as active speaker. @notnil
	 */
	virtual void onActiveSpeakerParticipantDevice(const std::shared_ptr<ParticipantDevice> &device) override;

protected:
	// TODO: use weak_ptr
	std::weak_ptr<Conference> conference;
	ConferenceListener *confListener;

private:
	std::string createNotify(Xsd::ConferenceInfo::ConferenceType confInfo, bool isFullState = false);
	std::string createNotifySubjectChanged(const std::string &subject);
	std::string createNotifyEphemeralLifetime(const long &lifetime);
	std::string createNotifyEphemeralMode(const EventLog::Type &type);
	std::shared_ptr<Content> makeContent(const std::string &xml);
	void notifyParticipant(const std::shared_ptr<Content> &notify, const std::shared_ptr<Participant> &participant);
	void notifyParticipantDevice(const std::shared_ptr<Content> &content,
	                             const std::shared_ptr<ParticipantDevice> &device);

	std::shared_ptr<Participant> getConferenceParticipant(const std::shared_ptr<Address> &address) const;

	void addProtocols(const std::shared_ptr<ParticipantDevice> &device, Xsd::ConferenceInfo::EndpointType &endpoint);
	void addMediaCapabilities(const std::shared_ptr<ParticipantDevice> &device,
	                          Xsd::ConferenceInfo::EndpointType &endpoint);
	void addEndpointSessionInfo(const std::shared_ptr<ParticipantDevice> &device,
	                            Xsd::ConferenceInfo::EndpointType &endpoint);
	void addEndpointCallInfo(const std::shared_ptr<ParticipantDevice> &device,
	                         Xsd::ConferenceInfo::EndpointType &endpoint);
	void addAvailableMediaCapabilities(const LinphoneMediaDirection audioDirection,
	                                   const LinphoneMediaDirection videoDirection,
	                                   const LinphoneMediaDirection textDirection,
	                                   Xsd::ConferenceInfo::ConferenceDescriptionType &confDescr);

	Xsd::XmlSchema::DateTime timeTToDateTime(const time_t &unixTime) const;

	std::shared_ptr<Conference> getConference() const;
	L_DISABLE_COPY(ServerConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_
