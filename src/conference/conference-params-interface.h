/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#ifndef _L_CONFERENCE_PARAMS_INTERFACE_H_
#define _L_CONFERENCE_PARAMS_INTERFACE_H_

#include <ctime>

#include "address/address.h"

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC ConferenceParamsInterface {
public:
	enum class JoiningMode {
		DialIn = LinphoneConferenceJoiningModeDialIn,
		DialOut = LinphoneConferenceJoiningModeDialOut
	};

	enum class SecurityLevel {
		None = LinphoneConferenceSecurityLevelNone,
		PointToPoint = LinphoneConferenceSecurityLevelPointToPoint,
		EndToEnd = LinphoneConferenceSecurityLevelEndToEnd
	};

	enum class ParticipantListType {
		Closed = LinphoneConferenceParticipantListTypeClosed,
		Open = LinphoneConferenceParticipantListTypeOpen
	};

	virtual ~ConferenceParamsInterface() = default;

	/*Set conference factory address.
	 *If set, Conference is created as an Adhoc conference on a remote conferencing server. Conference state is
	 *CreationPending until conference is instanciated and conference Id available. State is then transitionned to
	 *Created. If not set the conference is instanciated with a local focus. In this case conferenceId must be set.
	 * @param[in] Address of the conference factory (ex: sip:conference-factory@conf.linphone.org).
	 */
	virtual void setConferenceFactoryAddress(const std::shared_ptr<const Address> &address) = 0;

	/*Set focus address of this conference. If set, the Conference is created as an Adhoc conference from a remote
	 *conferencing server
	 * @param[in]  The Address of the conference focus.
	 **/
	virtual void setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress) = 0;

	/*
	 * Set the subject of this conference in UTF8. If not focus,  this operation is only available if the local
	 * participant  #getMe() is admin.
	 * @param[in] subject The new subject to set for the chat room
	 */
	virtual void setUtf8Subject(const std::string &subject) = 0;

	/*
	 * Enable audio media type for a conference
	 * @param enable If true, audio will be enabled during conference
	 */
	virtual void enableAudio(bool enable) = 0;

	/*
	 * Enable video media type for a conference
	 * @param enable If true, video will be enabled during conference
	 */
	virtual void enableVideo(bool enable) = 0;

	/*
	 * Enable chat media type for a conference
	 * @param enable If true, chat will be enabled during conference
	 */
	virtual void enableChat(bool enable) = 0;

	/*
	 * Set conference start time
	 * @param start conference start time as the number of seconds between the desired start time and the 1st of January
	 * 1970 or 0 for immediate start
	 */
	virtual void setStartTime(const time_t &start) = 0;

	/*
	 * Set conference end time
	 * @param end conference end time as the number of seconds between the desired end time and the 1st of January 1970
	 * or 0 for undefined end
	 */
	virtual void setEndTime(const time_t &end) = 0;

	/*
	 * Set participant list type
	 * @param type participant list type
	 */
	virtual void setParticipantListType(const ParticipantListType &type) = 0;

	/*
	 * Set conference security level
	 * @param type conference security level
	 */
	virtual void setSecurityLevel(const SecurityLevel &level) = 0;

	/*
	 * Set participant joining mode
	 * @param type participant joining mode
	 */
	virtual void setJoiningMode(const JoiningMode &type) = 0;
};

std::ostream &operator<<(std::ostream &str, ConferenceParamsInterface::SecurityLevel level);
std::string operator+(const std::string &str, ConferenceParamsInterface::SecurityLevel level);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_PARAMS_INTERFACE_H_
