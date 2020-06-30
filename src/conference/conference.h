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

#ifndef _L_CONFERENCE_H_
#define _L_CONFERENCE_H_

#include "linphone/types.h"
#include "linphone/core.h"

#include "address/address.h"

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

class ConferenceParams : public bellesip::HybridObject<LinphoneConferenceParams, ConferenceParams>, public ConferenceParamsInterface {
	friend class MediaConference::Conference;
	friend class MediaConference::LocalConference;
	friend class MediaConference::RemoteConference;
	public:
		ConferenceParams(const ConferenceParams& params) = default;
		ConferenceParams(const LinphoneCore *core = NULL) {
			if(core) {
				const LinphoneVideoPolicy *policy = linphone_core_get_video_policy(core);
				if(policy->automatically_initiate) m_enableVideo = true;
			}
		}

		Object *clone()const override{
			return new ConferenceParams(*this);
		}

		virtual void setConferenceFactoryAddress (const Address &address) override { m_factoryAddress = address; };
		const Address getFactoryAddress() const { return m_factoryAddress; };

		virtual void enableVideo(bool enable) override {m_enableVideo = enable;}
		bool videoEnabled() const {return m_enableVideo;}

		virtual void  enableAudio(bool enable) override {m_enableAudio = enable;};
		bool audioEnabled() const {return m_enableAudio;}

		virtual void  enableChat(bool enable) override {m_enableChat = enable;};
		bool chatEnabled() const {return m_enableChat;}

		void enableLocalParticipant (bool enable) { mLocalParticipantEnabled = enable; }
		bool localParticipantEnabled() const { return mLocalParticipantEnabled; }

		virtual void setConferenceAddress (const ConferenceAddress conferenceAddress) override { m_conferenceAddress = conferenceAddress; };
		const ConferenceAddress getConferenceAddress() const { return m_conferenceAddress; };

		virtual void setSubject (const std::string &subject) override { m_subject = subject; };
		const std::string &getSubject() const { return m_subject; };

		virtual void setMe (const IdentityAddress &participantAddress) override { m_me = participantAddress;};
		const IdentityAddress &getMe() const { return m_me; };

	private:
		bool m_enableVideo = false;
		bool m_enableAudio = false;
		bool m_enableChat = false;
		bool mLocalParticipantEnabled = true;
		ConferenceAddress m_conferenceAddress = ConferenceAddress();
		//Address m_conferenceAddress = Address();
		Address m_factoryAddress = Address();
		std::string m_subject = "";
		IdentityAddress m_me = IdentityAddress();
};

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
	~Conference();

	std::shared_ptr<Participant> getActiveParticipant () const;

	std::shared_ptr<Participant> findParticipant (const std::shared_ptr<const CallSession> &session) const;
	std::shared_ptr<ParticipantDevice> findParticipantDevice (const std::shared_ptr<const CallSession> &session) const;

	// TODO: Start Delete
	virtual void join () override;
	// TODO: End Delete

	/* ConferenceInterface */
	std::shared_ptr<Participant> findParticipant (const IdentityAddress &addr) const override;
	std::shared_ptr<Participant> getMe () const override;
	bool addParticipant (std::shared_ptr<Call> call) override;
	bool addParticipant (const IdentityAddress &participantAddress) override;
	bool addParticipants (const std::list<IdentityAddress> &addresses) override;
	int getParticipantCount () const override;
	const std::list<std::shared_ptr<Participant>> &getParticipants () const override;
	const std::string &getSubject () const override;
	void join (const IdentityAddress &participantAddress) override;
	void leave () override;
	bool removeParticipant (const std::shared_ptr<Participant> &participant) override;
	bool removeParticipants (const std::list<std::shared_ptr<Participant>> &participants) override;
	bool update(const ConferenceParamsInterface &newParameters) override;
	const ConferenceParams &getCurrentParams() const {return *confParams;}

	virtual const ConferenceAddress getConferenceAddress () const override;
	void setConferenceAddress (const ConferenceAddress &conferenceAddress);

	void setParticipantAdminStatus (const std::shared_ptr<Participant> &participant, bool isAdmin) override;
	void setSubject (const std::string &subject) override;

	std::string getResourceLists (const std::list<IdentityAddress> &addresses) const;
	static std::list<IdentityAddress> parseResourceLists (const Content &content);

	void addListener(std::shared_ptr<ConferenceListenerInterface> listener) override {
		confListeners.push_back(listener);
	}

	const ConferenceId &getConferenceId () const override;
	inline unsigned int getLastNotify () const { return lastNotify; };

	void subscribeReceived (LinphoneEvent *event);

	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantAdded (time_t creationTime, const bool isFullState, const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantRemoved (time_t creationTime, const bool isFullState, const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantSetAdmin (time_t creationTime, const bool isFullState, const Address &addr, bool isAdmin);
	std::shared_ptr<ConferenceSubjectEvent> notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject);
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceAdded (time_t creationTime, const bool isFullState, const Address &addr, const Address &gruu, const std::string name = "");
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceRemoved (time_t creationTime, const bool isFullState, const Address &addr, const Address &gruu);

	void notifyFullState ();
	void notifyStateChanged (LinphonePrivate::ConferenceInterface::State state);

	LinphonePrivate::ConferenceInterface::State getState() const override {return state;}
	virtual void setState(LinphonePrivate::ConferenceInterface::State state) override;

	void clearParticipants();

protected:
	explicit Conference (
		const std::shared_ptr<Core> &core,
		const IdentityAddress &myAddress,
		CallSessionListener *listener,
		const std::shared_ptr<ConferenceParams> params
	);

	bool isMe (const IdentityAddress &addr) const;

	std::list<std::shared_ptr<Participant>> participants;

	std::shared_ptr<Participant> activeParticipant;
	std::shared_ptr<Participant> me;

	std::list<std::shared_ptr<ConferenceListenerInterface>> confListeners;

	CallSessionListener *listener = nullptr;

	void setLastNotify (unsigned int lastNotify);
	void resetLastNotify ();
	void setConferenceId (const ConferenceId &conferenceId);

	ConferenceId conferenceId;

	std::shared_ptr<ConferenceParams> confParams = nullptr;

	// lastNotify belongs to the conference and not the the event handler.
	// The event handler can access it using the getter
	unsigned int lastNotify = 0;

	ConferenceInterface::State state = ConferenceInterface::State::None;

private:
	L_DISABLE_COPY(Conference);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_H_
