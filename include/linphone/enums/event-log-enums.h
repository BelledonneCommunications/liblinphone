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

#ifndef _L_EVENT_LOG_ENUMS_H_
#define _L_EVENT_LOG_ENUMS_H_

// =============================================================================

#define L_ENUM_VALUES_EVENT_LOG_TYPE(F) \
	F(None /**< No defined event */) \
	F(ConferenceCreated /**< Conference (created) event */) \
	F(ConferenceTerminated /**< Conference (terminated) event */) \
	F(ConferenceCallStart /**< Conference call (start) event */) \
	F(ConferenceCallEnd /**< Conference call (end) event */) \
	F(ConferenceChatMessage /**< Conference chat message event */) \
	F(ConferenceParticipantAdded /**< Conference participant (added) event */) \
	F(ConferenceParticipantRemoved /**< Conference participant (removed) event */) \
	F(ConferenceParticipantSetAdmin /**< Conference participant (set admin) event */) \
	F(ConferenceParticipantUnsetAdmin /**< Conference participant (unset admin) event */) \
	F(ConferenceParticipantDeviceAdded /**< Conference participant device (added) event */) \
	F(ConferenceParticipantDeviceRemoved /**< Conference participant device (removed) event */) \
	F(ConferenceSubjectChanged /**< Conference subject event */) \
	F(ConferenceSecurityEvent /**< Conference encryption security event*/) \
	F(ConferenceEphemeralMessageLifetimeChanged /**< Conference ephemeral message (ephemeral message lifetime changed) event */) \
	F(ConferenceEphemeralMessageEnabled /**< Conference ephemeral message (ephemeral message enabled) event */) \
	F(ConferenceEphemeralMessageDisabled /**< Conference ephemeral message (ephemeral message disabled) event */) \

#endif // ifndef _L_EVENT_LOG_ENUMS_H_
