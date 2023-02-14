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

#include "linphone/types.h"
#include "linphone/core.h"

#include "address/address.h"

#include "conference/conference-params.h"
#include "conference/conference-interface.h"
#include "conference/conference-listener.h"
#include "core/core-accessor.h"

#include "belle-sip/object++.hh"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSession;
class CallSessionListener;
class CallSessionPrivate;
class Content;
class ParticipantDevice;
class LocalConferenceEventHandler;


namespace MediaConference{ // They are in a special namespace because of conflict of generic Conference classes in src/conference/*
	class Conference;
	class LocalConference;
	class RemoteConference;
}

class LINPHONE_PUBLIC Conference :
	public ConferenceInterface,
	public ConferenceListener,
	public CoreAccessor {
	friend class CallSessionPrivate;
	friend class LocalConferenceEventHandler;
	friend class LocalAudioVideoConferenceEventHandler;
	friend class RemoteConferenceEventHandler;
	friend class ClientGroupChatRoomPrivate;
	friend class ClientGroupChatRoom;
	friend class ServerGroupChatRoomPrivate;
	friend class ServerGroupChatRoom;
public:

	static constexpr int labelLength = 10;
	~Conference();

	std::shared_ptr<Participant> getActiveParticipant () const;

	std::shared_ptr<ParticipantDevice> findParticipantDevice (const std::shared_ptr<const CallSession> &session) const;
	std::shared_ptr<ParticipantDevice> findParticipantDevice (const IdentityAddress &pAddr, const IdentityAddress &dAddr) const;
	std::shared_ptr<ParticipantDevice> findParticipantDeviceByLabel (const std::string &label) const;
	std::shared_ptr<ParticipantDevice> getActiveSpeakerParticipantDevice() const;

	virtual const std::shared_ptr<CallSession> getMainSession() const;

	// TODO: Start Delete
	virtual void join () override;
	// TODO: End Delete

	virtual bool addParticipants(const std::list<std::shared_ptr<LinphonePrivate::Call>> &call);

	/* ConferenceInterface */
	std::shared_ptr<Participant> findParticipant (const IdentityAddress &addr) const override;
	std::shared_ptr<Participant> getMe () const override;
	bool isMe (const IdentityAddress &addr) const;
	bool addParticipant (std::shared_ptr<Call> call) override;
	bool addParticipant (const IdentityAddress &participantAddress) override;
	bool addParticipants (const std::list<IdentityAddress> &addresses) override;
	int getParticipantCount () const override;
	const std::list<std::shared_ptr<Participant>> &getParticipants () const override;
	const std::list<std::shared_ptr<ParticipantDevice>> getParticipantDevices () const override;
	const std::string &getSubject () const override;
	const std::string getUtf8Subject () const override;
	void join (const IdentityAddress &participantAddress) override;
	void leave () override;
	bool removeParticipant (const std::shared_ptr<Participant> &participant) override;
	bool removeParticipants (const std::list<std::shared_ptr<Participant>> &participants) override;

	virtual bool isIn() const = 0;

	bool update(const ConferenceParamsInterface &newParameters) override;
	const ConferenceParams &getCurrentParams() const {return *confParams;}

	virtual const ConferenceAddress & getConferenceAddress () const override;
	void setConferenceAddress (const ConferenceAddress &conferenceAddress);

	void setParticipantAdminStatus (const std::shared_ptr<Participant> &participant, bool isAdmin) override;
	void setSubject (const std::string &subject) override;
	void setUtf8Subject (const std::string &subject) override;

	const std::string &getUsername () const;
	void setUsername (const std::string &username);

	ConferenceLayout getLayout() const;
	void setLayout(const ConferenceLayout layout);

	time_t getStartTime() const;
	int getDuration() const;

	void addListener(std::shared_ptr<ConferenceListenerInterface> listener) override {
		confListeners.push_back(listener);
	}

	const ConferenceId &getConferenceId () const override;
	inline unsigned int getLastNotify () const { return lastNotify; };

	void subscribeReceived (LinphoneEvent *event);

	virtual void setLocalParticipantStreamCapability(const LinphoneMediaDirection & direction, const LinphoneStreamType type);

	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantAdded (time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant);
	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantRemoved (time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant);
	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantSetAdmin (time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin);
	virtual std::shared_ptr<ConferenceSubjectEvent> notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject);
	virtual std::shared_ptr<ConferenceAvailableMediaEvent> notifyAvailableMediaChanged (time_t creationTime, const bool isFullState, const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities);
	virtual std::shared_ptr<ConferenceEphemeralMessageEvent> notifyEphemeralModeChanged (time_t creationTime,  const bool isFullState, const EventLog::Type type);
	virtual std::shared_ptr<ConferenceEphemeralMessageEvent> notifyEphemeralMessageEnabled (time_t creationTime, const bool isFullState, const bool enable);
	virtual std::shared_ptr<ConferenceEphemeralMessageEvent> notifyEphemeralLifetimeChanged (time_t creationTime,  const bool isFullState, const long lifetime);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceAdded (time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceRemoved (time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceMediaCapabilityChanged (time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceMediaAvailabilityChanged (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice);
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceStateChanged (time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice);

	void notifySpeakingDevice (uint32_t ssrc, bool isSpeaking);
	void notifyMutedDevice (uint32_t ssrc, bool muted);
	void notifyLocalMutedDevices (bool muted);

	virtual void notifyFullState ();
	virtual void notifyStateChanged (LinphonePrivate::ConferenceInterface::State state);
	virtual void notifyActiveSpeakerParticipantDevice(const std::shared_ptr<ParticipantDevice> &participantDevice);

	LinphonePrivate::ConferenceInterface::State getState() const override {return state;}
	virtual void setState(LinphonePrivate::ConferenceInterface::State state) override;

	virtual std::shared_ptr<Call> getCall() const = 0;

	const std::map<uint32_t, bool> & getPendingParticipantsMutes() const;

	void clearParticipants();

#ifdef HAVE_DB_STORAGE
	void updateSubjectInConferenceInfo(const std::string & subject) const;
	void updateParticipantsInConferenceInfo(const IdentityAddress & participantAddress) const;
#endif // HAVE_DB_STORAGE

protected:
	explicit Conference (
		const std::shared_ptr<Core> &core,
		const IdentityAddress &myAddress,
		CallSessionListener *listener,
		const std::shared_ptr<ConferenceParams> params
	);

	std::list<std::shared_ptr<Participant>> participants;

	std::shared_ptr<Participant> activeParticipant;
	std::shared_ptr<Participant> me;
	std::shared_ptr<ParticipantDevice> activeSpeakerDevice = nullptr;

	std::list<std::shared_ptr<ConferenceListenerInterface>> confListeners;

	CallSessionListener *listener = nullptr;

	void setLastNotify (unsigned int lastNotify);
	void resetLastNotify ();
	void setConferenceId (const ConferenceId &conferenceId);

	std::map<ConferenceMediaCapabilities, bool> getMediaCapabilities() const;

	LinphoneStatus updateMainSession();

	ConferenceId conferenceId;

	std::shared_ptr<ConferenceParams> confParams = nullptr;

	// lastNotify belongs to the conference and not the the event handler.
	// The event handler can access it using the getter
	unsigned int lastNotify = 0;

	std::string mUsername = "";
	time_t startTime = 0;

	ConferenceInterface::State state = ConferenceInterface::State::None;
	std::map<uint32_t, bool> pendingParticipantsMutes;

	virtual std::shared_ptr<ConferenceInfo> createOrGetConferenceInfo() const;
	virtual std::shared_ptr<ConferenceInfo> createConferenceInfo(const IdentityAddress & organizer, const std::list<IdentityAddress> invitedParticipants) const;

private:
	L_DISABLE_COPY(Conference);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_H_
