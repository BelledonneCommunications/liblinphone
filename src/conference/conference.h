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

#ifndef _L_CONFERENCE_H_
#define _L_CONFERENCE_H_

#include <map>

#include "belle-sip/object++.hh"

#include "address/address.h"
#include "conference-cbs.h"
#include "conference/conference-id.h"
#include "conference/conference-interface.h"
#include "conference/conference-listener.h"
#include "conference/conference-params.h"
#include "conference/participant.h"
#include "core/core-accessor.h"
#include "linphone/api/c-conference.h"
#include "linphone/core.h"
#include "linphone/types.h"
#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AudioControlInterface;
class VideoControlInterface;
class MixerSession;
class ConferenceParams;
class Call;
class CallSession;
class CallSessionListener;
class ParticipantDevice;
class AudioDevice;
class ConferenceId;
class Player;

class LINPHONE_PUBLIC Conference : public bellesip::HybridObject<LinphoneConference, Conference>,
                                   public ConferenceInterface,
                                   public ConferenceListener,
                                   public CoreAccessor,
                                   public CallSessionListener,
                                   public CallbacksHolder<ConferenceCbs>,
                                   public UserDataAccessor {
	friend class CallSessionPrivate;
	friend class ServerConferenceEventHandler;
	friend class ClientConferenceEventHandler;
	friend class ClientChatRoom;
	friend class ServerChatRoom;

public:
	static constexpr int sLabelLength = 10;
	static const std::string SecurityModeParameter;
	static const std::string ConfIdParameter;
	static const std::string AdminParameter;
	static const std::string IsFocusParameter;
	static const std::string TextParameter;
	static bool isTerminationState(ConferenceInterface::State state);
	static Address createParticipantAddressForResourceList(const ConferenceInfo::participant_list_t::value_type &p);
	static Address createParticipantAddressForResourceList(const std::shared_ptr<Participant> &p);

	virtual ~Conference();

	virtual int inviteAddresses(const std::list<std::shared_ptr<Address>> &addresses,
	                            const LinphoneCallParams *params) = 0;
	virtual bool dialOutAddresses(const std::list<std::shared_ptr<Address>> &addressList) = 0;
	virtual bool finalizeParticipantAddition(std::shared_ptr<Call> call) = 0;

	std::shared_ptr<Participant> getActiveParticipant() const;

	void setInvitedParticipants(const std::list<std::shared_ptr<Participant>> &invitedParticipants);

	std::shared_ptr<ParticipantDevice> findParticipantDevice(const std::shared_ptr<const CallSession> &session) const;
	std::shared_ptr<ParticipantDevice> findParticipantDevice(const std::shared_ptr<const Address> &pAddr,
	                                                         const std::shared_ptr<const Address> &dAddr) const;
	std::shared_ptr<ParticipantDevice> findParticipantDeviceBySsrc(uint32_t ssrc, LinphoneStreamType type) const;
	std::shared_ptr<ParticipantDevice> findParticipantDeviceByLabel(const LinphoneStreamType type,
	                                                                const std::string &label) const;
	std::shared_ptr<ParticipantDevice> getActiveSpeakerParticipantDevice() const;

	virtual std::shared_ptr<CallSession> getMainSession() const;

	virtual bool addParticipants(const std::list<std::shared_ptr<Call>> &call);

	/* ConferenceInterface */
	std::shared_ptr<Participant> findParticipant(const std::shared_ptr<const CallSession> &session) const;
	std::shared_ptr<Participant> findParticipant(const std::shared_ptr<const Address> &addr) const override;
	std::shared_ptr<Participant> getMe() const override;
	bool isMe(const std::shared_ptr<const Address> &addr) const;

	bool setParticipants(const std::list<std::shared_ptr<Participant>> &&newParticipants);

	void addInvitedParticipant(const std::shared_ptr<Call> &call);
	void addInvitedParticipant(const std::shared_ptr<Address> &address);
	void addInvitedParticipant(const std::shared_ptr<Participant> &participant);
	bool addParticipant(std::shared_ptr<Call> call) override;
	bool addParticipant(const std::shared_ptr<ParticipantInfo> &info) override;
	bool addParticipant(const std::shared_ptr<Address> &participantAddress) override;
	bool addParticipants(const std::list<std::shared_ptr<Address>> &addresses) override;
	virtual bool addParticipantDevice(std::shared_ptr<Call> call);
	virtual void addParticipantDevice(const std::shared_ptr<Participant> &participant,
	                                  const std::shared_ptr<ParticipantDeviceIdentity> &deviceInfo);

	int getParticipantCount() const override;
	const std::list<std::shared_ptr<Participant>> &getParticipants() const override;
	std::list<std::shared_ptr<Address>> getParticipantAddresses() const;
	std::list<std::shared_ptr<ParticipantDevice>> getParticipantDevices(bool includeMe = true) const override;
	std::shared_ptr<Participant> getScreenSharingParticipant() const;
	std::shared_ptr<ParticipantDevice> getScreenSharingDevice() const;

	const std::string &getSubject() const;
	const std::string &getUtf8Subject() const override;

	void join(const std::shared_ptr<Address> &participantAddress) override;
	void leave() override;

	virtual void removeParticipantDevice(const std::shared_ptr<Participant> &participant,
	                                     const std::shared_ptr<Address> &deviceAddress);
	virtual int removeParticipantDevice(const std::shared_ptr<CallSession> &session);
	int removeParticipant(std::shared_ptr<Call> call);
	virtual int removeParticipant(const std::shared_ptr<CallSession> &session, const bool preserveSession);
	virtual int removeParticipant(const std::shared_ptr<Address> &addr) = 0;
	bool removeParticipant(const std::shared_ptr<Participant> &participant) override;
	bool removeParticipants(const std::list<std::shared_ptr<Participant>> &participants) override;
	void clearParticipants();

	virtual void initFromDb(const std::shared_ptr<Participant> &me,
	                        const ConferenceId conferenceId,
	                        const unsigned int lastNotifyId,
	                        bool hasBeenLeft) = 0;
	virtual void init(SalCallOp *op = nullptr, ConferenceListener *confListener = nullptr) = 0;
	virtual void createEventHandler(ConferenceListener *confListener = nullptr, bool addToListEventHandler = false) = 0;
	virtual bool isIn() const = 0;

	// TODO: Delete - Temporary function
	void setParams(std::shared_ptr<ConferenceParams> newParameters) {
		mConfParams = newParameters;
	}

	bool update(const ConferenceParamsInterface &newParameters) override;
	const std::shared_ptr<ConferenceParams> &getCurrentParams() const {
		return mConfParams;
	}

	virtual std::shared_ptr<Address> getConferenceAddress() const override;
	void setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress);

	void setParticipantAdminStatus(const std::shared_ptr<Participant> &participant, bool isAdmin) override;
	void setSubject(const std::string &subject);
	void setUtf8Subject(const std::string &subject) override;

	const std::string &getUsername() const;
	void setUsername(const std::string &username);

	ConferenceLayout getLayout() const;
	void setLayout(const ConferenceLayout layout);

	std::shared_ptr<Account> getAccount();
	void invalidateAccount();
	time_t getStartTime() const;
	int getDuration() const;

	void removeListener(std::shared_ptr<ConferenceListenerInterface> listener) override;
	void addListener(std::shared_ptr<ConferenceListenerInterface> listener) override;

	const ConferenceId &getConferenceId() const override;
	std::optional<std::reference_wrapper<const std::string>> getIdentifier() const;
	inline unsigned int getLastNotify() const {
		return mLastNotify;
	};

	virtual void setLocalParticipantStreamCapability(const LinphoneMediaDirection &direction,
	                                                 const LinphoneStreamType type);

	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantAdded(
	    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant);
	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantRemoved(
	    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant);
	virtual std::shared_ptr<ConferenceParticipantEvent>
	notifyParticipantSetRole(time_t creationTime,
	                         const bool isFullState,
	                         const std::shared_ptr<Participant> &participant,
	                         Participant::Role role);
	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantSetAdmin(
	    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin);
	virtual std::shared_ptr<ConferenceSubjectEvent>
	notifySubjectChanged(time_t creationTime, const bool isFullState, const std::string subject);
	virtual std::shared_ptr<ConferenceAvailableMediaEvent>
	notifyAvailableMediaChanged(time_t creationTime,
	                            const bool isFullState,
	                            const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities);
	virtual std::shared_ptr<ConferenceEphemeralMessageEvent>
	notifyEphemeralModeChanged(time_t creationTime, const bool isFullState, const EventLog::Type type);
	virtual std::shared_ptr<ConferenceEphemeralMessageEvent>
	notifyEphemeralMessageEnabled(time_t creationTime, const bool isFullState, const bool enable);
	virtual std::shared_ptr<ConferenceEphemeralMessageEvent>
	notifyEphemeralLifetimeChanged(time_t creationTime, const bool isFullState, const long lifetime);
	virtual std::shared_ptr<ConferenceNotifiedEvent> notifyAllowedParticipantListChanged(time_t creationTime,
	                                                                                     const bool isFullState);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceAdded(time_t creationTime,
	                             const bool isFullState,
	                             const std::shared_ptr<Participant> &participant,
	                             const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceRemoved(time_t creationTime,
	                               const bool isFullState,
	                               const std::shared_ptr<Participant> &participant,
	                               const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceMediaCapabilityChanged(time_t creationTime,
	                                              const bool isFullState,
	                                              const std::shared_ptr<Participant> &participant,
	                                              const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceMediaAvailabilityChanged(time_t creationTime,
	                                                const bool isFullState,
	                                                const std::shared_ptr<Participant> &participant,
	                                                const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceStateChanged(time_t creationTime,
	                                    const bool isFullState,
	                                    const std::shared_ptr<Participant> &participant,
	                                    const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceScreenSharingChanged(time_t creationTime,
	                                            const bool isFullState,
	                                            const std::shared_ptr<Participant> &participant,
	                                            const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent>
	notifyParticipantDeviceJoiningRequest(time_t creationTime,
	                                      const bool isFullState,
	                                      const std::shared_ptr<Participant> &participant,
	                                      const std::shared_ptr<ParticipantDevice> &participantDevice);

	void notifySpeakingDevice(uint32_t ssrc, bool isSpeaking);
	void notifyMutedDevice(uint32_t ssrc, bool muted);
	void notifyLocalMutedDevices(bool muted);

	virtual void notifyFullState();
	virtual void notifyStateChanged(ConferenceInterface::State state);
	virtual void notifyActiveSpeakerParticipantDevice(const std::shared_ptr<ParticipantDevice> &participantDevice);

	const std::shared_ptr<AbstractChatRoom> getChatRoom() const;

	ConferenceInterface::State getState() const override {
		return mState;
	}
	virtual void setState(ConferenceInterface::State state) override;

	virtual std::shared_ptr<Call> getCall() const = 0;

	const std::map<uint32_t, bool> &getPendingParticipantsMutes() const;

	void setCachedScreenSharingDevice();
	void resetCachedScreenSharingDevice();
	std::shared_ptr<ParticipantDevice> getCachedScreenSharingDevice() const;

	void updateSubjectInConferenceInfo(const std::string &subject) const;
	void updateParticipantInConferenceInfo(const std::shared_ptr<Participant> &participant) const;
	bool updateParticipantInfoInConferenceInfo(std::shared_ptr<ConferenceInfo> &info,
	                                           const std::shared_ptr<Participant> &participant) const;
	void updateSecurityLevelInConferenceInfo(const ConferenceParams::SecurityLevel &level) const;
	void updateParticipantRoleInConferenceInfo(const std::shared_ptr<Participant> &participant) const;

	bool areThumbnailsRequested(bool update) const;

	virtual std::pair<bool, LinphoneMediaDirection> getMainStreamVideoDirection(
	    const std::shared_ptr<CallSession> &session, bool localIsOfferer, bool useLocalParams) const;

	virtual LinphoneMediaDirection verifyVideoDirection(const std::shared_ptr<CallSession> &session,
	                                                    const LinphoneMediaDirection suggestedVideoDirection) const;
	virtual int participantDeviceMediaCapabilityChanged(const std::shared_ptr<CallSession> &session) = 0;
	virtual int participantDeviceMediaCapabilityChanged(const std::shared_ptr<Address> &addr) = 0;
	virtual int participantDeviceMediaCapabilityChanged(const std::shared_ptr<Participant> &participant,
	                                                    const std::shared_ptr<ParticipantDevice> &device) = 0;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<CallSession> &session,
	                                         const LinphoneStreamType type,
	                                         uint32_t ssrc) = 0;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<CallSession> &session,
	                                         uint32_t audioSsrc,
	                                         uint32_t videoSsrc) = 0;

	virtual int participantDeviceAlerting(const std::shared_ptr<CallSession> &session) = 0;
	virtual int participantDeviceAlerting(const std::shared_ptr<Participant> &participant,
	                                      const std::shared_ptr<ParticipantDevice> &device) = 0;
	virtual int participantDeviceJoined(const std::shared_ptr<CallSession> &session) = 0;
	virtual int participantDeviceJoined(const std::shared_ptr<Participant> &participant,
	                                    const std::shared_ptr<ParticipantDevice> &device) = 0;
	virtual int participantDeviceLeft(const std::shared_ptr<CallSession> &session) = 0;
	virtual int participantDeviceLeft(const std::shared_ptr<Participant> &participant,
	                                  const std::shared_ptr<ParticipantDevice> &device) = 0;

	virtual int getParticipantDeviceVolume(const std::shared_ptr<ParticipantDevice> &device) = 0;

	virtual int terminate() = 0;
	virtual void finalizeCreation() = 0;

	virtual int enter() = 0;

	virtual const std::shared_ptr<Address> getOrganizer() const = 0;
	virtual void setOrganizer(const std::shared_ptr<Address> &organizer) const;

	bool isConferenceEnded() const;
	bool isConferenceExpired() const;
	bool isConferenceStarted() const;
	bool isConferenceAvailable() const;

	virtual AudioControlInterface *getAudioControlInterface() const = 0;
	virtual VideoControlInterface *getVideoControlInterface() const = 0;
	virtual AudioStream *getAudioStream() = 0; /* Used by the tone manager, revisit.*/

	void setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	void setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	std::shared_ptr<AudioDevice> getInputAudioDevice() const;
	std::shared_ptr<AudioDevice> getOutputAudioDevice() const;

	virtual int startRecording(const std::string &path) = 0;
	virtual int stopRecording();
	virtual bool isRecording() const;

	bool getMicrophoneMuted() const;
	void setMicrophoneMuted(bool muted);
	float getRecordVolume() const;

	virtual bool isSubscriptionUnderWay() const;

	virtual std::shared_ptr<Player> getPlayer() const;

	virtual const std::shared_ptr<ConferenceInfo> createOrGetConferenceInfo() const;

	bool isChatOnly() const;
	bool supportsMedia() const;

	virtual void handleRefer(SalReferOp *op,
	                         const std::shared_ptr<LinphonePrivate::Address> &referAddr,
	                         const std::string method) = 0;

	void resetLastNotify();

	void setConferenceId(const ConferenceId &conferenceId);

protected:
	explicit Conference(const std::shared_ptr<Core> &core,
	                    std::shared_ptr<CallSessionListener> callSessionListener,
	                    const std::shared_ptr<const ConferenceParams> params);

	std::list<std::shared_ptr<Participant>> mParticipants;
	std::shared_ptr<Participant> mActiveParticipant;
	std::shared_ptr<Participant> mMe;
	std::shared_ptr<ParticipantDevice> mActiveSpeakerDevice = nullptr;
	std::shared_ptr<ParticipantDevice> mCachedScreenSharingDevice = nullptr;

	std::list<std::shared_ptr<ConferenceListenerInterface>> mConfListeners;

	std::weak_ptr<CallSessionListener> mCallSessionListener;

	std::map<ConferenceMediaCapabilities, bool> getMediaCapabilities() const;

	LinphoneStatus updateMainSession(bool modifyParams = true);

	ConferenceId mConferenceId;

	std::list<std::shared_ptr<Participant>> mInvitedParticipants;

	std::shared_ptr<ConferenceParams> mConfParams = nullptr;

	mutable std::shared_ptr<Address> mOrganizer;

	// lastNotify belongs to the conference and not the the event handler.
	// The event handler can access it using the getter
	unsigned int mLastNotify = 0;

	std::string mUsername = "";

	long long mConferenceInfoId = -1;

	ConferenceInterface::State mState = ConferenceInterface::State::None;
	std::map<uint32_t, bool> mPendingParticipantsMutes;

	virtual std::shared_ptr<ConferenceInfo> createConferenceInfo() const;
	virtual std::shared_ptr<ConferenceInfo> createConferenceInfoWithCustomParticipantList(
	    const std::shared_ptr<Address> &organizer,
	    const std::list<std::shared_ptr<Participant>> &invitedParticipants) const;
	virtual std::shared_ptr<ConferenceInfo>
	createConferenceInfoWithCustomParticipantList(const std::shared_ptr<Address> &organizer,
	                                              const ConferenceInfo::participant_list_t &invitedParticipants) const;
	const std::shared_ptr<ConferenceInfo> getUpdatedConferenceInfo() const;
	const std::shared_ptr<ParticipantDevice> getFocusOwnerDevice() const;

	virtual bool sessionParamsAllowThumbnails() const = 0;
	bool updateMinatureRequestedFlag() const;

	mutable bool thumbnailsRequested = true;

	void fillInvitedParticipantList(const ConferenceInfo::participant_list_t infos);
	void fillInvitedParticipantList(SalCallOp *op, const std::shared_ptr<Address> &organizer, bool cancelling);
	std::shared_ptr<Participant> findInvitedParticipant(const std::shared_ptr<const Address> &participantAddress) const;
	std::shared_ptr<ParticipantDevice>
	findInvitedParticipantDevice(const std::shared_ptr<const CallSession> &session) const;
	std::list<std::shared_ptr<const Address>> getInvitedAddresses() const;
	std::list<std::shared_ptr<Participant>> getInvitedParticipants() const;
	void removeInvitedParticipant(const std::shared_ptr<Address> &address);

	std::shared_ptr<Participant> createParticipant(std::shared_ptr<Call> call);
	std::shared_ptr<Participant> createParticipant(std::shared_ptr<const Address> participantAddress);
	virtual std::shared_ptr<ParticipantDevice> createParticipantDevice(std::shared_ptr<Participant> participant,
	                                                                   std::shared_ptr<Call> call);

	std::list<std::shared_ptr<Participant>> getFullParticipantList() const;
	void fillParticipantAttributes(std::shared_ptr<Participant> &p) const;

	void notifyNewDevice(const std::shared_ptr<ParticipantDevice> &device);

	virtual void configure(SalCallOp *op) = 0;
	void inititializeMe();

	void incrementLastNotify();
	void setLastNotify(unsigned int lastNotify);

	void setChatRoom(const std::shared_ptr<AbstractChatRoom> &chatRoom);

	std::unique_ptr<LogContextualizer> getLogContextualizer() override;

private:
	mutable std::shared_ptr<ConferenceInfo> mConferenceInfo;
	std::shared_ptr<AbstractChatRoom> mChatRoom = nullptr;

	L_DISABLE_COPY(Conference);
};

inline std::ostream &operator<<(std::ostream &str, const Conference &conference) {
	const auto &conferenceAddress = conference.getConferenceAddress();
	str << "Conference [" << &conference << "] ("
	    << (conferenceAddress ? conferenceAddress->toString() : std::string("sip:")) << ")";
	return str;
}

class ConferenceLogContextualizer : public CoreLogContextualizer {
public:
	ConferenceLogContextualizer(const Conference &conference) : CoreLogContextualizer(conference) {
		pushTag(conference);
	}
	ConferenceLogContextualizer(const LinphoneConference *conference)
	    : CoreLogContextualizer(*Conference::toCpp(conference)) {
		if (conference) pushTag(*Conference::toCpp(conference));
	}
	virtual ~ConferenceLogContextualizer();

private:
	void pushTag(const Conference &conference);
	bool mPushed = false;
	static constexpr char sTagIdentifier[] = "2.conference.linphone";
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_H_
