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

#ifndef _REMOTE_CONFERENCE_H_
#define _REMOTE_CONFERENCE_H_

#include "conference.h"
#include "linphone/utils/general.h"

LINPHONE_BEGIN_NAMESPACE

#ifdef HAVE_ADVANCED_IM
class RemoteConferenceEventHandler;
#endif // HAVE_ADVANCED_IM
class Event;

namespace MediaConference { // They are in a special namespace because of conflict of generic Conference classes in
	                        // src/conference/*

/*
 * Class for an audio/video conference that is running on a remote server.
 */
class LINPHONE_PUBLIC RemoteConference : public Conference, public ConferenceListenerInterface {
public:
	RemoteConference(const std::shared_ptr<Core> &core,
	                 const std::shared_ptr<Address> &focusAddr,
	                 const ConferenceId &conferenceId,
	                 CallSessionListener *listener,
	                 const std::shared_ptr<ConferenceParams> params);
	RemoteConference(const std::shared_ptr<Core> &core,
	                 const std::shared_ptr<Address> &meAddr,
	                 CallSessionListener *listener,
	                 const std::shared_ptr<LinphonePrivate::ConferenceParams> params);
	virtual ~RemoteConference();

	void initWithInvitees(const std::shared_ptr<LinphonePrivate::Call> &focusCall,
	                      const ConferenceInfo::participant_list_t &invitees,
	                      const ConferenceId &conferenceId);
	void initWithInvitees(const std::shared_ptr<Address> confAddr,
	                      const std::shared_ptr<Address> focusAddr,
	                      const std::shared_ptr<LinphonePrivate::CallSession> focusSession,
	                      const ConferenceInfo::participant_list_t &invitees,
	                      const ConferenceId &conferenceId);
	virtual int inviteAddresses(const std::list<std::shared_ptr<Address>> &addresses,
	                            const LinphoneCallParams *params) override;
	virtual bool dialOutAddresses(const std::list<std::shared_ptr<Address>> &addressList) override;
	virtual bool addParticipant(const std::shared_ptr<ParticipantInfo> &info) override;
	virtual bool addParticipants(const std::list<std::shared_ptr<LinphonePrivate::Call>> &call) override;
	virtual bool addParticipants(const std::list<std::shared_ptr<Address>> &addresses) override;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual bool addParticipant(const std::shared_ptr<Address> &participantAddress) override;
	virtual bool addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual bool finalizeParticipantAddition(std::shared_ptr<LinphonePrivate::Call> call) override;

	virtual int removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> &session,
	                              const bool preserveSession) override;
	virtual int removeParticipant(const std::shared_ptr<Address> &addr) override;
	virtual bool removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) override;
	virtual int terminate() override;
	virtual void finalizeCreation() override;

	virtual int enter() override;
	virtual void leave() override;
	virtual bool isIn() const override;
	virtual const std::shared_ptr<Address> getOrganizer() const override;

	virtual int startRecording(const char *path) override;

	virtual void setLocalParticipantStreamCapability(const LinphoneMediaDirection &direction,
	                                                 const LinphoneStreamType type) override;

	virtual AudioControlInterface *getAudioControlInterface() const override;
	virtual VideoControlInterface *getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	void multipartNotifyReceived(const std::shared_ptr<Event> &notifyLev, const Content &content);
	void notifyReceived(const std::shared_ptr<Event> &notifyLev, const Content &content);

	virtual int
	participantDeviceMediaCapabilityChanged(const std::shared_ptr<LinphonePrivate::CallSession> &session) override;
	virtual int participantDeviceMediaCapabilityChanged(const std::shared_ptr<Address> &addr) override;
	virtual int
	participantDeviceMediaCapabilityChanged(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                        const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) override;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> &session,
	                                         const LinphoneStreamType type,
	                                         uint32_t ssrc) override;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> &session,
	                                         uint32_t audioSsrc,
	                                         uint32_t videoSsrc) override;

	virtual int participantDeviceAlerting(const std::shared_ptr<LinphonePrivate::CallSession> &session) override;
	virtual int participantDeviceAlerting(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                      const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) override;
	virtual int participantDeviceJoined(const std::shared_ptr<LinphonePrivate::CallSession> &session) override;
	virtual int participantDeviceJoined(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                    const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) override;
	virtual int participantDeviceLeft(const std::shared_ptr<LinphonePrivate::CallSession> &session) override;
	virtual int participantDeviceLeft(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                  const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) override;

	virtual int getParticipantDeviceVolume(const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) override;

	virtual void onStateChanged(LinphonePrivate::ConferenceInterface::State state) override;
	virtual void onParticipantAdded(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                const std::shared_ptr<Participant> &participant) override;
	virtual void onParticipantRemoved(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                  const std::shared_ptr<Participant> &participant) override;

	virtual void onParticipantDeviceAdded(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                      const std::shared_ptr<ParticipantDevice> &device) override;
	virtual void onParticipantDeviceRemoved(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                        const std::shared_ptr<ParticipantDevice> &device) override;
	virtual void onParticipantDeviceStateChanged(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                             const std::shared_ptr<ParticipantDevice> &device) override;
	virtual void
	onParticipantDeviceMediaAvailabilityChanged(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                            const std::shared_ptr<ParticipantDevice> &device) override;
	virtual void onAvailableMediaChanged(const std::shared_ptr<ConferenceAvailableMediaEvent> &event) override;
	virtual void onFullStateReceived() override;

	virtual void setParticipantAdminStatus(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                       bool isAdmin) override;

	virtual void onSubjectChanged(const std::shared_ptr<ConferenceSubjectEvent> &event) override;
	virtual void onParticipantSetRole(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                  const std::shared_ptr<Participant> &participant) override;
	virtual void setSubject(const std::string &subject) override;
	virtual bool update(const ConferenceParamsInterface &params) override;

	virtual void notifyStateChanged(LinphonePrivate::ConferenceInterface::State state) override;

	void setMainSession(const std::shared_ptr<LinphonePrivate::CallSession> &session);
	virtual std::shared_ptr<Call> getCall() const override;

	virtual void onConferenceTerminated(const std::shared_ptr<Address> &addr) override;
	virtual void onParticipantsCleared() override;

#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<RemoteConferenceEventHandler> eventHandler;
#endif // HAVE_ADVANCED_IM

	/* Report the csrc included in the video stream, so that we can notify who is presented on the screen.*/
	void notifyDisplayedSpeaker(uint32_t csrc);
	void notifyLouderSpeaker(uint32_t ssrc);

protected:
	virtual void
	callStateChangedCb(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message) override;
	virtual void
	transferStateChangedCb(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) override;

private:
	virtual const std::shared_ptr<CallSession> getMainSession() const override;
	virtual std::shared_ptr<ConferenceInfo> createConferenceInfo() const override;
	void updateAndSaveConferenceInformations();
	bool focusIsReady() const;
	bool transferToFocus(std::shared_ptr<LinphonePrivate::Call> call);
	void reset();
	void endConference();

	void onFocusCallStateChanged(LinphoneCallState state);
	void onPendingCallStateChanged(std::shared_ptr<LinphonePrivate::Call> call, LinphoneCallState callState);
	void onTransferingCallStateChanged(std::shared_ptr<LinphonePrivate::Call> transfered,
	                                   LinphoneCallState newCallState);
	std::list<std::shared_ptr<Address>> cleanAddressesList(const std::list<std::shared_ptr<Address>> &addresses) const;
	void createFocus(const std::shared_ptr<Address> focusAddr,
	                 const std::shared_ptr<LinphonePrivate::CallSession> focusSession = nullptr);

	bool finalized = false;
	bool scheduleUpdate = false;
	bool fullStateReceived = false;
	std::string pendingSubject;
	std::shared_ptr<Participant> focus;
	std::list<std::shared_ptr<LinphonePrivate::Call>> m_pendingCalls;
	std::list<std::shared_ptr<LinphonePrivate::Call>> m_transferingCalls;

	uint32_t displayedSpeaker = 0;
	uint32_t louderSpeaker = 0;
	uint32_t lastNotifiedSsrc = 0;

	// end-to-end encryption
	std::vector<uint8_t> mEktKey;
};

} // end of namespace MediaConference

LINPHONE_END_NAMESPACE

#endif // _REMOTE_CONFERENCE_H_
