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

#ifndef _L_NOTIFY_CONFERENCE_LISTENER_H_
#define _L_NOTIFY_CONFERENCE_LISTENER_H_

#include "conference/conference-interface.h"
#include "event-log/events.h"

LINPHONE_BEGIN_NAMESPACE

class Conference;

class LINPHONE_PUBLIC NotifyConferenceListener : public ConferenceListenerInterface {
public:
	NotifyConferenceListener(Conference *conference) : conf(conference){};
	virtual ~NotifyConferenceListener() = default;

	/*
	 * This fonction is called each time the list of allowed participant is changed while the conference is active
	 * @param[in] event informations related to the change of the participant allowed to join the conference. @notnil
	 */
	virtual void onAllowedParticipantListChanged(const std::shared_ptr<ConferenceNotifiedEvent> &event) override;

	/*
	 * This fonction is called each time a new participant is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant. @notnil
	 * @param[in] participant participant added to conference or chat room. @notnil
	 */
	virtual void onParticipantAdded(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new participant is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed participant. @notnil
	 * @param[in] participant participant removed from conference or chat room. @notnil
	 */
	virtual void onParticipantRemoved(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                  const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a participant changes its role in the conference after full state notification.
	 * @param[in] event informations related to the participant new role. @notnil
	 * @param[in] participant participant whose role changed. @notnil
	 */
	virtual void onParticipantSetRole(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                  const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new admin is set by the focus after full state notification.
	 * @param[in] event informations related to the new admin participant. @notnil
	 * @param[in] participant participant whose admin status changed. @notnil
	 */
	virtual void onParticipantSetAdmin(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                   const std::shared_ptr<Participant> &participant) override;

	/*
	 * This fonction is called each time a new available media set is defined by the focus after full state
	 * notification.
	 * @param[in] event informations related to the new available media set. @notnil
	 */
	virtual void onAvailableMediaChanged(const std::shared_ptr<ConferenceAvailableMediaEvent> &event) override;

	/*
	 * This fonction is called each time a new subject is set by the focus after full state notification.
	 * @param[in] event informations related to the new subject. @notnil
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
	 * This fonction is called each time a new participant device is added by the focus after full state notification.
	 * @param[in] event informations related to the added participant's device. @notnil
	 * @param[in] device participant device added to the conference or chat room. @notnil
	 */
	virtual void onParticipantDeviceAdded(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                      const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a new participant device is removed by the focus after full state notification.
	 * @param[in] event informations related to the removed device's participant. @notnil
	 * @param[in] device participant device removed from the conference or chat room. @notnil
	 */
	virtual void onParticipantDeviceRemoved(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                        const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a new participant device that is not in the allowed participants'list calls a
	 * closed-list conference
	 * @param[in] event informations related to the removed device's participant. @notnil
	 * @param[in] device participant device that is not in the allowed participants'list. @notnil
	 */
	virtual void onParticipantDeviceJoiningRequest(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                               const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a new participant device changed its media availability after full state
	 * notification.
	 * @param[in] event informations related to the device's participant whose media availability changed. @notnil
	 * @param[in] device participant device whose media availability. @notnil
	 */
	virtual void
	onParticipantDeviceMediaAvailabilityChanged(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                            const std::shared_ptr<ParticipantDevice> &device) override;

	/*
	 * This fonction is called each time a new participant device changed its media capability after full state
	 * notification.
	 * @param[in] event informations related to the device's participant whose media capability changed. @notnil
	 * @param[in] device participant device whose media capability. @notnil
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
	 * Notify Conference state changes.
	 ** @param[in] newState the new state of this conference.
	 */
	virtual void onStateChanged(ConferenceInterface::State newState) override;

	/*
	 * Notify Conference full state received.
	 ** @param[in] newState the new state of this conference.
	 */
	virtual void onFullStateReceived() override;

	/*
	 * Notify which participant device is being currently displayed as active speaker.
	 * @param[in] device participant device currently being displayed as active speaker. @notnil
	 */
	virtual void onActiveSpeakerParticipantDevice(const std::shared_ptr<ParticipantDevice> &device) override;

private:
	Conference *conf;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_NOTIFY_CONFERENCE_LISTENER_H_
