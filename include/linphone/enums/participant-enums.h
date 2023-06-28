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

#ifndef _L_PARTICIPANT_ENUMS_H_
#define _L_PARTICIPANT_ENUMS_H_
// =============================================================================

/**
 * #LinphoneParticipantRole is used to define a role of a participant within a conference
 * @ingroup conference
 */
typedef enum _LinphoneParticipantRole {
	LinphoneParticipantRoleSpeaker = 0,  /**< participant is a speaker in the conference */
	LinphoneParticipantRoleListener = 1, /**< participant is a listener in the conference. He/She cannot speak */
	LinphoneParticipantRoleUnknown = 2   /**< participant role is unknown */
} LinphoneParticipantRole;

#endif // ifndef _L_PARTICIPANT_ENUMS_H_
