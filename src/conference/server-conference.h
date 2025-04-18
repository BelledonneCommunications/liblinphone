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

#ifndef _L_LOCAL_CONFERENCE_H_
#define _L_LOCAL_CONFERENCE_H_

#include <list>
#include <queue>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "conference/conference.h"
#include "conference/session/media-session.h"
#include "content/content-manager.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ServerConferenceEventHandler;
class EventSubscribe;
class EventPublish;

class LINPHONE_PUBLIC ServerConference : public Conference, public CoreListener {
	friend ServerChatRoom;

public:
	static constexpr int sConfIdLength = 32;

	ServerConference(const std::shared_ptr<Core> &core,
	                 std::shared_ptr<CallSessionListener> listener,
	                 const std::shared_ptr<ConferenceParams> params);
	virtual ~ServerConference();

	virtual int inviteAddresses(const std::list<std::shared_ptr<Address>> &addresses,
	                            const LinphoneCallParams *params) override;
	virtual bool dialOutAddresses(const std::list<std::shared_ptr<Address>> &addressList) override;
	void inviteDevice(const std::shared_ptr<ParticipantDevice> &device);
	void byeDevice(const std::shared_ptr<ParticipantDevice> &device);

	std::shared_ptr<Participant> addParticipantToList(const std::shared_ptr<Address> &participantAddress);
	void resumeParticipant(const std::shared_ptr<Participant> &participant);

	int getParticipantCount() const override;

	virtual bool addParticipants(const std::list<std::shared_ptr<Call>> &call) override;
	virtual bool addParticipants(const std::list<std::shared_ptr<Address>> &addresses) override;
	virtual bool addParticipant(std::shared_ptr<Call> call) override;
	virtual bool addParticipant(const std::shared_ptr<ParticipantInfo> &info) override;
	virtual bool addParticipant(const std::shared_ptr<Address> &participantAddress) override;
	virtual bool finalizeParticipantAddition(std::shared_ptr<Call> call) override;
	virtual std::shared_ptr<ParticipantDevice> createParticipantDevice(std::shared_ptr<Participant> participant,
	                                                                   std::shared_ptr<Call> call) override;

	virtual int removeParticipant(const std::shared_ptr<CallSession> &session, const bool preserveSession) override;
	virtual int removeParticipant(const std::shared_ptr<Address> &addr) override;
	virtual bool removeParticipant(const std::shared_ptr<Participant> &participant) override;

	virtual void removeParticipantDevice(const std::shared_ptr<Participant> &participant,
	                                     const std::shared_ptr<Address> &deviceAddress) override;
	virtual int removeParticipantDevice(const std::shared_ptr<CallSession> &session) override;

	virtual void setLocalParticipantStreamCapability(const LinphoneMediaDirection &direction,
	                                                 const LinphoneStreamType type) override;

	virtual AudioControlInterface *getAudioControlInterface() const override;
	virtual VideoControlInterface *getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	void publishReceived(std::shared_ptr<EventPublish> event);
	void publishStateChanged(std::shared_ptr<EventPublish> event, LinphonePublishState state);

	void subscribeReceived(const std::shared_ptr<EventSubscribe> &event);
	void subscriptionStateChanged(std::shared_ptr<EventSubscribe> event, LinphoneSubscriptionState state);

	virtual void initFromDb(const std::shared_ptr<Participant> &me,
	                        const ConferenceId conferenceId,
	                        const unsigned int lastNotifyId,
	                        bool hasBeenLeft) override;
	virtual void init(SalCallOp *op = nullptr, ConferenceListener *confListener = nullptr) override;
	virtual bool update(const ConferenceParamsInterface &params) override;
	virtual int terminate() override;
	virtual void finalizeCreation() override;
	virtual void onConferenceTerminated(const std::shared_ptr<Address> &addr) override;
	virtual void onFirstNotifyReceived(const std::shared_ptr<Address> &addr) override;

	virtual const std::shared_ptr<Address> getOrganizer() const override;

	virtual int enter() override;
	virtual void leave() override;
	virtual bool isIn() const override;

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
	virtual std::shared_ptr<ConferenceEphemeralMessageEvent>
	notifyEphemeralModeChanged(time_t creationTime, const bool isFullState, const EventLog::Type type) override;
	virtual std::shared_ptr<ConferenceEphemeralMessageEvent>
	notifyEphemeralMessageEnabled(time_t creationTime, const bool isFullState, const bool enable) override;
	virtual std::shared_ptr<ConferenceEphemeralMessageEvent>
	notifyEphemeralLifetimeChanged(time_t creationTime, const bool isFullState, const long lifetime) override;

	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceJoiningRequest(time_t creationTime,
	                                      const bool isFullState,
	                                      const std::shared_ptr<Participant> &participant,
	                                      const std::shared_ptr<ParticipantDevice> &participantDevice) override;

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
	notifyParticipantDeviceStateChanged(time_t creationTime,
	                                    const bool isFullState,
	                                    const std::shared_ptr<Participant> &participant,
	                                    const std::shared_ptr<ParticipantDevice> &participantDevice) override;

	virtual std::shared_ptr<ConferenceAvailableMediaEvent>
	notifyAvailableMediaChanged(time_t creationTime,
	                            const bool isFullState,
	                            const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceMediaCapabilityChanged(time_t creationTime,
	                                              const bool isFullState,
	                                              const std::shared_ptr<Participant> &participant,
	                                              const std::shared_ptr<ParticipantDevice> &participantDevice) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceScreenSharingChanged(time_t creationTime,
	                                            const bool isFullState,
	                                            const std::shared_ptr<Participant> &participant,
	                                            const std::shared_ptr<ParticipantDevice> &participantDevice) override;

	virtual void notifyFullState() override;
	void confirmCreation();
	bool updateConferenceInformation(SalCallOp *op);
	virtual std::shared_ptr<Call> getCall() const override;

	virtual void notifyStateChanged(ConferenceInterface::State state) override;

	virtual int startRecording(const std::string &path) override;

	virtual int participantDeviceMediaCapabilityChanged(const std::shared_ptr<CallSession> &session) override;
	virtual int participantDeviceMediaCapabilityChanged(const std::shared_ptr<Address> &addr) override;
	virtual int participantDeviceMediaCapabilityChanged(const std::shared_ptr<Participant> &participant,
	                                                    const std::shared_ptr<ParticipantDevice> &device) override;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<CallSession> &session,
	                                         const LinphoneStreamType type,
	                                         uint32_t ssrc) override;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<CallSession> &session,
	                                         uint32_t audioSsrc,
	                                         uint32_t videoSsrc) override;

	virtual int participantDeviceAlerting(const std::shared_ptr<CallSession> &session) override;
	virtual int participantDeviceAlerting(const std::shared_ptr<Participant> &participant,
	                                      const std::shared_ptr<ParticipantDevice> &device) override;
	virtual int participantDeviceJoined(const std::shared_ptr<CallSession> &session) override;
	virtual int participantDeviceJoined(const std::shared_ptr<Participant> &participant,
	                                    const std::shared_ptr<ParticipantDevice> &device) override;
	virtual int participantDeviceLeft(const std::shared_ptr<CallSession> &session) override;
	virtual int participantDeviceLeft(const std::shared_ptr<Participant> &participant,
	                                  const std::shared_ptr<ParticipantDevice> &device) override;

	virtual int getParticipantDeviceVolume(const std::shared_ptr<ParticipantDevice> &device) override;

	virtual void setParticipantAdminStatus(const std::shared_ptr<Participant> &participant, bool isAdmin) override;

	void moveDeviceToPresent(const std::shared_ptr<CallSession> &session);
	void moveDeviceToPresent(const std::shared_ptr<ParticipantDevice> &device);
	void setParticipantDeviceState(const std::shared_ptr<ParticipantDevice> &device,
	                               ParticipantDevice::State state,
	                               bool notify = true);
	void updateParticipantDeviceSession(const std::shared_ptr<ParticipantDevice> &device,
	                                    bool freslyRegistered = false);

	static bool allDevicesLeft(const std::shared_ptr<Participant> &participant);

	void updateParticipantDevices(const std::shared_ptr<Address> &addr,
	                              const std::list<std::shared_ptr<ParticipantDeviceIdentity>> &devices);
	void setParticipantDevicesAtCreation(const std::shared_ptr<Address> &addr,
	                                     const std::list<std::shared_ptr<ParticipantDeviceIdentity>> &devices);
	void setParticipantDevices(const std::shared_ptr<Address> &addr,
	                           const std::list<std::shared_ptr<ParticipantDeviceIdentity>> &devices);
	void updateParticipantsSessions();
	void conclude();
	void requestDeletion();

	void declineSession(const std::shared_ptr<CallSession> &session, LinphoneReason reason);
	void acceptSession(const std::shared_ptr<CallSession> &session);

	virtual std::shared_ptr<Player> getPlayer() const override;

	virtual std::pair<bool, LinphoneMediaDirection> getMainStreamVideoDirection(
	    const std::shared_ptr<CallSession> &session, bool localIsOfferer, bool useLocalParams) const override;

	void onGlobalStateChanged(LinphoneGlobalState state) override;

	void confirmJoining(SalCallOp *op);
	void handleSubjectChange(SalCallOp *op);
	void setUtf8Subject(const std::string &subject) override;

	virtual LinphoneMediaDirection
	verifyVideoDirection(const std::shared_ptr<CallSession> &session,
	                     const LinphoneMediaDirection suggestedVideoDirection) const override;

protected:
	virtual void onCallSessionStateChanged(const std::shared_ptr<CallSession> &session,
	                                       CallSession::State state,
	                                       const std::string &message) override;
	virtual void onCallSessionEarlyFailed(const std::shared_ptr<CallSession> &session, LinphoneErrorInfo *ei) override;

	virtual void onAckReceived(const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) override;

	virtual bool addParticipantDevice(std::shared_ptr<Call> call) override;

private:
	L_DISABLE_COPY(ServerConference);

#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<ServerConferenceEventHandler> mEventHandler;
#endif // HAVE_ADVANCED_IM
	std::unique_ptr<MixerSession> mMixerSession;
	bool mIsIn = false;

	bool initializeParticipants(const std::shared_ptr<Participant> &initiator, SalCallOp *op);
	void addParticipantDevice(const std::shared_ptr<Participant> &participant,
	                          const std::shared_ptr<ParticipantDeviceIdentity> &deviceInfo) override;
	bool addParticipantAndDevice(std::shared_ptr<Call> call);
	bool validateNewParameters(const ConferenceParams &newConfParams) const;
	std::shared_ptr<CallSession> makeSession(const std::shared_ptr<ParticipantDevice> &device,
	                                         const MediaSessionParams *csp);
	void chooseAnotherAdminIfNoneInConference();
	void checkIfTerminated();
	std::list<std::shared_ptr<const Address>> getAllowedAddresses() const;
	virtual void configure(SalCallOp *op) override;
	void enableScreenSharing(const std::shared_ptr<LinphonePrivate::CallSession> &session, bool notify);
	MediaSessionParams *updateParameterForParticipantRemoval(const std::shared_ptr<CallSession> &session) const;
	void terminateConferenceWithReason(const std::shared_ptr<Address> &remoteContactAddress,
	                                   std::shared_ptr<MediaSession> &session,
	                                   LinphoneReason reason,
	                                   int code,
	                                   const std::string &errorMessage);
	int checkServerConfiguration(const std::shared_ptr<Address> &remoteContactAddress,
	                             std::shared_ptr<LinphonePrivate::MediaSession> &session);

	void addLocalEndpoint();
	void removeLocalEndpoint();

	virtual std::shared_ptr<ConferenceInfo> createConferenceInfo() const override;
	bool tryAddMeDevice();

	virtual void createEventHandler(ConferenceListener *confListener = nullptr,
	                                bool addToListEventHandler = false) override;

	bool hasAdminLeft() const;
	bool supportsVideoCapabilities() const;

	void cleanup();

	void onParticipantDeviceLeft(const std::shared_ptr<ParticipantDevice> &device);

	bool checkClientCompatibility(const std::shared_ptr<Call> &call,
	                              const std::shared_ptr<Address> &remoteContactAddress,
	                              bool incomingReceived) const;

	virtual void handleRefer(SalReferOp *op,
	                         const std::shared_ptr<LinphonePrivate::Address> &referAddr,
	                         const std::string method) override;
	virtual bool sessionParamsAllowThumbnails() const override;
	void setConferenceTimes(time_t startTime, time_t endTime);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_CONFERENCE_H_
