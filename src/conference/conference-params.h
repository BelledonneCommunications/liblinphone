/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#ifndef _L_CONFERENCE_PARAMS_H_
#define _L_CONFERENCE_PARAMS_H_

#include <string>

#include "conference/conference-interface.h"
#include "core/core.h"
#include "linphone/core.h"
#include "address/address.h"

#include "belle-sip/object++.hh"

LINPHONE_BEGIN_NAMESPACE

namespace MediaConference{ // They are in a special namespace because of conflict of generic Conference classes in src/conference/*
class Conference;
class LocalConference;
class RemoteConference;
}

class LINPHONE_PUBLIC ConferenceParams : public bellesip::HybridObject<LinphoneConferenceParams, ConferenceParams>, public ConferenceParamsInterface {
	friend class MediaConference::Conference;
	friend class MediaConference::LocalConference;
	friend class MediaConference::RemoteConference;
	public:
		/**
		 * Conference layout
		 */
		enum class Layout{
			ActiveSpeaker = LinphoneConferenceLayoutActiveSpeaker, /**< Active speaker - participant who speaks is prominently displayed in the center of the screen and other participants are minimized */
			Grid = LinphoneConferenceLayoutGrid, /**< Grid - each participant is given an equal sized image size */
		};

		ConferenceParams(const ConferenceParams& params);
		ConferenceParams(const LinphoneCore *core = NULL);

		ConferenceParams *clone()const override{
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

		void enableLocalParticipant (bool enable) { m_localParticipantEnabled = enable; }
		bool localParticipantEnabled() const { return m_localParticipantEnabled; }

		void enableOneParticipantConference (bool enable) { m_allowOneParticipantConference = enable; }
		bool oneParticipantConferenceEnabled() const { return m_allowOneParticipantConference; }

		virtual void setConferenceAddress (const ConferenceAddress conferenceAddress) override { m_conferenceAddress = conferenceAddress; };
		const ConferenceAddress & getConferenceAddress() const { return m_conferenceAddress; };

		virtual void setSubject (const std::string &subject) override { m_subject = subject; };
		const std::string &getSubject() const { return m_subject; };

		virtual void setMe (const IdentityAddress &participantAddress) override { m_me = participantAddress;};
		const IdentityAddress &getMe() const { return m_me; };

		void setLayout(const Layout l) { m_layout = l; };
		Layout getLayout() const { return m_layout; };

	private:
		bool m_enableVideo = false;
		bool m_enableAudio = false;
		bool m_enableChat = false;
		bool m_localParticipantEnabled = true;
		bool m_allowOneParticipantConference = false;
		ConferenceAddress m_conferenceAddress = ConferenceAddress();
		Layout m_layout = Layout::ActiveSpeaker;
		Address m_factoryAddress = Address();
		std::string m_subject = "";
		IdentityAddress m_me = IdentityAddress();
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_PARAMS_H_
