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

#ifndef _LOCAL_CONFERENCE_H_
#define _LOCAL_CONFERENCE_H_

#include "conference.h"
#include "linphone/utils/general.h"

LINPHONE_BEGIN_NAMESPACE

#ifdef HAVE_ADVANCED_IM
class LocalAudioVideoConferenceEventHandler;
#endif // HAVE_ADVANCED_IM
class EventSubscribe;
class EventPublish;

namespace MediaConference { // They are in a special namespace because of conflict of generic Conference classes in
	                        // src/conference/*

/*
 * Class for an audio/video conference running locally.
 */
class LINPHONE_PUBLIC LocalConference : public Conference {
public:
	LocalConference(const std::shared_ptr<Core> &core,
	                const std::shared_ptr<Address> &myAddress,
	                CallSessionListener *listener,
	                const std::shared_ptr<ConferenceParams> params);
	LocalConference(const std::shared_ptr<Core> &core, SalCallOp *op);

	virtual ~LocalConference();

	virtual int inviteAddresses(const std::list<std::shared_ptr<Address>> &addresses,
	                            const LinphoneCallParams *params) override;
	virtual bool dialOutAddresses(const std::list<std::shared_ptr<Address>> &addressList) override;
	virtual bool addParticipants(const std::list<std::shared_ptr<LinphonePrivate::Call>> &call) override;
	virtual bool addParticipants(const std::list<std::shared_ptr<Address>> &addresses) override;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual bool addParticipant(const std::shared_ptr<ParticipantInfo> &info) override;
	virtual bool addParticipant(const std::shared_ptr<Address> &participantAddress) override;
	virtual bool finalizeParticipantAddition(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual bool addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) override;

	virtual int removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> &session,
	                              const bool preserveSession) override;
	virtual int removeParticipant(const std::shared_ptr<Address> &addr) override;
	virtual bool removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) override;
	virtual bool update(const ConferenceParamsInterface &params) override;
	virtual int terminate() override;
	virtual void finalizeCreation() override;
	virtual void onConferenceTerminated(const std::shared_ptr<Address> &addr) override;
	virtual void setSubject(const std::string &subject) override;
	virtual const std::shared_ptr<Address> getOrganizer() const override;

	virtual int enter() override;
	virtual void leave() override;
	virtual bool isIn() const override;

	virtual int startRecording(const char *path) override;

	virtual void setLocalParticipantStreamCapability(const LinphoneMediaDirection &direction,
	                                                 const LinphoneStreamType type) override;

	virtual AudioControlInterface *getAudioControlInterface() const override;
	virtual VideoControlInterface *getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	void publishReceived(std::shared_ptr<EventPublish> event);
	void publishStateChanged(std::shared_ptr<EventPublish> event, LinphonePublishState state);

	void subscribeReceived(std::shared_ptr<EventSubscribe> event);
	void subscriptionStateChanged(std::shared_ptr<EventSubscribe> event, LinphoneSubscriptionState state);

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

	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantAdded(
	    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) override;
	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantRemoved(
	    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) override;
	virtual std::shared_ptr<ConferenceParticipantEvent>
	notifyParticipantSetAdmin(time_t creationTime,
	                          const bool isFullState,
	                          const std::shared_ptr<Participant> &participant,
	                          bool isAdmin) override;
	virtual std::shared_ptr<ConferenceSubjectEvent>
	notifySubjectChanged(time_t creationTime, const bool isFullState, const std::string subject) override;
	virtual std::shared_ptr<ConferenceAvailableMediaEvent>
	notifyAvailableMediaChanged(time_t creationTime,
	                            const bool isFullState,
	                            const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceAdded(time_t creationTime,
	                             const bool isFullState,
	                             const std::shared_ptr<Participant> &participant,
	                             const std::shared_ptr<ParticipantDevice> &participantDevice) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceRemoved(time_t creationTime,
	                               const bool isFullState,
	                               const std::shared_ptr<Participant> &participant,
	                               const std::shared_ptr<ParticipantDevice> &participantDevice) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceScreenSharingChanged(time_t creationTime,
	                                            const bool isFullState,
	                                            const std::shared_ptr<Participant> &participant,
	                                            const std::shared_ptr<ParticipantDevice> &participantDevice) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceMediaCapabilityChanged(time_t creationTime,
	                                              const bool isFullState,
	                                              const std::shared_ptr<Participant> &participant,
	                                              const std::shared_ptr<ParticipantDevice> &participantDevice) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceStateChanged(time_t creationTime,
	                                    const bool isFullState,
	                                    const std::shared_ptr<Participant> &participant,
	                                    const std::shared_ptr<ParticipantDevice> &participantDevice) override;

	virtual void notifyFullState() override;

	virtual void setParticipantAdminStatus(const std::shared_ptr<LinphonePrivate::Participant> &participant,
	                                       bool isAdmin) override;

	virtual void notifyStateChanged(LinphonePrivate::ConferenceInterface::State state) override;

	void initWithOp(SalCallOp *op);
	void confirmCreation();
	void updateConferenceInformation(SalCallOp *op);
	virtual std::shared_ptr<Call> getCall() const override;

	std::shared_ptr<Player> getPlayer() const override;

	virtual std::pair<bool, LinphoneMediaDirection> getMainStreamVideoDirection(
	    const std::shared_ptr<CallSession> &session, bool localIsOfferer, bool useLocalParams) const override;

protected:
	virtual bool sessionParamsAllowThumbnails() const override;
	virtual void
	callStateChangedCb(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message) override;
	virtual void
	transferStateChangedCb(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state) override;

private:
	std::unique_ptr<MixerSession> mMixerSession;
	bool mIsIn = false;
	std::shared_ptr<Address> organizer;
	static constexpr int confIdLength = 10;
#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<LocalAudioVideoConferenceEventHandler> eventHandler;
#endif // HAVE_ADVANCED_IM

	bool validateNewParameters(const LinphonePrivate::ConferenceParams &newConfParams) const;
	bool updateAllParticipantSessionsExcept(const std::shared_ptr<CallSession> &session);
	void updateParticipantsSessions();
	void updateParticipantDeviceSession(const std::shared_ptr<ParticipantDevice> &device,
	                                    bool freshlyRegistered = false);
	void acceptSession(const std::shared_ptr<CallSession> &session);
	std::shared_ptr<CallSession> makeSession(const std::shared_ptr<ParticipantDevice> &device);
	void chooseAnotherAdminIfNoneInConference();
	void checkIfTerminated();
	std::list<std::shared_ptr<Address>> getAllowedAddresses() const;
	void configure(SalCallOp *op);
	void fillInvitedParticipantList(SalCallOp *op, bool cancelling);
	void enableScreenSharing(const std::shared_ptr<LinphonePrivate::CallSession> &session, bool notify);

	void addLocalEndpoint();
	void removeLocalEndpoint();

	virtual std::shared_ptr<ConferenceInfo> createConferenceInfo() const override;
	bool tryAddMeDevice();

	void createEventHandler();
};

} // end of namespace MediaConference

LINPHONE_END_NAMESPACE

#endif // _LOCAL_CONFERENCE_H_
