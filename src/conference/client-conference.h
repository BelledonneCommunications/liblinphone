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

#ifndef _L_REMOTE_CONFERENCE_H_
#define _L_REMOTE_CONFERENCE_H_

#include "conference/conference.h"
#include "core/core-accessor.h"
#ifdef HAVE_ADVANCED_IM
#include "conference/encryption/client-ekt-manager.h"
#endif // HAVE_ADVANCED_IM

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ClientConferenceEventHandler;
class Core;

class LINPHONE_PUBLIC ClientConference : public Conference, public ConferenceListenerInterface {
	friend class ClientChatRoom;

public:
	ClientConference(const std::shared_ptr<Core> &core,
	                 std::shared_ptr<CallSessionListener> listener,
	                 const std::shared_ptr<const ConferenceParams> params);
	virtual ~ClientConference();

	virtual void initFromDb(const std::shared_ptr<Participant> &me,
	                        const ConferenceId conferenceId,
	                        const unsigned int lastNotifyId,
	                        bool hasBeenLeft) override;
	void initWithFocus(const std::shared_ptr<const Address> &focusAddr,
	                   const std::shared_ptr<CallSession> &focusSession,
	                   SalCallOp *op = nullptr,
	                   ConferenceListener *confListener = nullptr);
	virtual int inviteAddresses(const std::list<std::shared_ptr<Address>> &addresses,
	                            const LinphoneCallParams *params) override;
	virtual bool dialOutAddresses(const std::list<std::shared_ptr<Address>> &addressList) override;
	virtual bool addParticipant(const std::shared_ptr<ParticipantInfo> &info) override;
	virtual bool addParticipants(const std::list<std::shared_ptr<Call>> &call) override;
	virtual bool addParticipants(const std::list<std::shared_ptr<Address>> &addresses) override;
	virtual bool addParticipant(std::shared_ptr<Call> call) override;
	virtual bool addParticipant(const std::shared_ptr<Address> &participantAddress) override;
	virtual std::shared_ptr<ParticipantDevice> createParticipantDevice(std::shared_ptr<Participant> participant,
	                                                                   std::shared_ptr<Call> call) override;
	virtual bool finalizeParticipantAddition(std::shared_ptr<Call> call) override;

	virtual int removeParticipant(const std::shared_ptr<CallSession> &session, const bool preserveSession) override;
	virtual int removeParticipant(const std::shared_ptr<Address> &addr) override;
	virtual bool removeParticipant(const std::shared_ptr<Participant> &participant) override;
	virtual int terminate() override;
	virtual void init(SalCallOp *op = nullptr, ConferenceListener *confListener = nullptr) override;
	virtual void finalizeCreation() override;

	virtual int enter() override;
	virtual void join(const std::shared_ptr<Address> &participantAddress) override;
	virtual void leave() override;
	virtual bool isIn() const override;
	virtual const std::shared_ptr<Address> getOrganizer() const override;

	virtual int startRecording(const std::string &path) override;

	virtual void setLocalParticipantStreamCapability(const LinphoneMediaDirection &direction,
	                                                 const LinphoneStreamType type) override;

	virtual AudioControlInterface *getAudioControlInterface() const override;
	virtual VideoControlInterface *getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	void multipartNotifyReceived(const std::shared_ptr<Event> &notifyLev, const Content &content);
	void notifyReceived(const std::shared_ptr<Event> &notifyLev, const Content &content);

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

	virtual void onStateChanged(ConferenceInterface::State state) override;
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

	virtual void onParticipantDeviceScreenSharingChanged(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                                     const std::shared_ptr<ParticipantDevice> &device) override;

	virtual void setParticipantAdminStatus(const std::shared_ptr<Participant> &participant, bool isAdmin) override;

	virtual void onSubjectChanged(const std::shared_ptr<ConferenceSubjectEvent> &event) override;
	virtual void onParticipantSetRole(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                  const std::shared_ptr<Participant> &participant) override;
	virtual void onParticipantSetAdmin(const std::shared_ptr<ConferenceParticipantEvent> &event,
	                                   const std::shared_ptr<Participant> &participant) override;
	virtual bool update(const ConferenceParamsInterface &params) override;

	virtual void notifyStateChanged(ConferenceInterface::State state) override;

	void setMainSession(const std::shared_ptr<CallSession> &session);
	virtual std::shared_ptr<Call> getCall() const override;

	virtual bool isSubscriptionUnderWay() const override;

	virtual void onConferenceCreated(const std::shared_ptr<Address> &addr) override;
	virtual void onConferenceKeywordsChanged(const std::vector<std::string> &keywords) override;
	virtual void onConferenceTerminated(const std::shared_ptr<Address> &addr) override;
	virtual void onParticipantsCleared() override;
	virtual void onSecurityEvent(const std::shared_ptr<ConferenceSecurityEvent> &event) override;
	virtual void onFirstNotifyReceived(const std::shared_ptr<Address> &addr) override;
	virtual void onFullStateReceived() override;

	virtual void onEphemeralModeChanged(const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) override;
	virtual void onEphemeralMessageEnabled(const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) override;
	virtual void onEphemeralLifetimeChanged(const std::shared_ptr<ConferenceEphemeralMessageEvent> &event) override;

#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<ClientConferenceEventHandler> mEventHandler;
#endif // HAVE_ADVANCED_IM

	void requestFullState();

	/* Report the csrc included in the video stream, so that we can notify who is presented on the screen.*/
	void notifyDisplayedSpeaker(uint32_t csrc);
	void notifyLouderSpeaker(uint32_t ssrc);

	void setConferenceId(const ConferenceId &conferenceId);
	void confirmJoining(SalCallOp *op);
	void attachCall(const std::shared_ptr<CallSession> &session);
	AbstractChatRoom::SecurityLevel
	getSecurityLevelExcept(const std::shared_ptr<ParticipantDevice> &ignoredDevice) const;

	void createFocus(const std::shared_ptr<const Address> &focusAddr,
	                 const std::shared_ptr<CallSession> focusSession = nullptr);

	virtual std::pair<bool, LinphoneMediaDirection> getMainStreamVideoDirection(
	    const std::shared_ptr<CallSession> &session, bool localIsOfferer, bool useLocalParams) const override;

	void onCallSessionSetReleased(const std::shared_ptr<CallSession> &session) override;
	void onCallSessionSetTerminated(const std::shared_ptr<CallSession> &session) override;
	void onCallSessionStateChanged(const std::shared_ptr<CallSession> &session,
	                               CallSession::State state,
	                               const std::string &message) override;
	void setUtf8Subject(const std::string &subject) override;

	void subscribe(bool addToListEventHandler, bool unsubscribeFirst = true);
	void unsubscribe();

#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<LinphonePrivate::ClientEktManager> getClientEktManager() const;
#endif // HAVE_ADVANCED_IM

protected:
	void onCallSessionTransferStateChanged(const std::shared_ptr<CallSession> &session,
	                                       CallSession::State state) override;

private:
	void acceptSession(const std::shared_ptr<CallSession> &session);
	std::shared_ptr<CallSession> createSessionTo(const std::shared_ptr<const Address> &sessionTo);
	std::shared_ptr<CallSession> createSession();
	virtual std::shared_ptr<CallSession> getMainSession() const override;
	virtual std::shared_ptr<ConferenceInfo> createConferenceInfo() const override;
	void updateAndSaveConferenceInformations();
	bool focusIsReady() const;
	bool transferToFocus(std::shared_ptr<Call> call);
	void reset();
	void endConference();

	void onFocusCallStateChanged(CallSession::State state, const std::string &message);
	void onPendingCallStateChanged(std::shared_ptr<Call> call, CallSession::State callState);
	std::list<Address> cleanAddressesList(const std::list<std::shared_ptr<Address>> &addresses) const;

	virtual void configure(SalCallOp *op) override;

	bool hasBeenLeft() const;

	virtual void createEventHandler(ConferenceListener *confListener = nullptr,
	                                bool addToListEventHandler = false) override;
	void initializeHandlers(ConferenceListener *confListener, bool addToListEventHandler);

	virtual void handleRefer(SalReferOp *op,
	                         const std::shared_ptr<LinphonePrivate::Address> &referAddr,
	                         const std::string method) override;
	virtual bool sessionParamsAllowThumbnails() const override;

	void callFocus();

	bool mFinalized = false;
	bool mScheduleUpdate = false;
	bool mFullStateUpdate = false;
	bool mFullStateReceived = false;
	std::string mPendingSubject;
	std::shared_ptr<Participant> mFocus;
	std::list<std::shared_ptr<Call>> mPendingCalls;
	std::list<std::shared_ptr<Call>> mTransferingCalls;
	MediaSessionParams *mJoiningParams = nullptr;

	uint32_t mDisplayedSpeaker = 0;
	uint32_t mLouderSpeaker = 0;
	uint32_t mLastNotifiedSsrc = 0;

#ifdef HAVE_ADVANCED_IM
	// end-to-end encryption
	std::shared_ptr<LinphonePrivate::ClientEktManager> mClientEktManager = nullptr;
#endif // HAVE_ADVANCED_IM

	// end-to-end encryption
	std::vector<uint8_t> mEktKey;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONFERENCE_H_
