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

#ifndef _L_CONFERENCE_ENUMS_H_
#define _L_CONFERENCE_ENUMS_H_

// =============================================================================
/**
 * #LinphoneConferenceState is used to indicate the current state of a conference.
 * @ingroup conference
 */
typedef enum _LinphoneConferenceState{
	LinphoneConferenceStateNone = 0,		/**< Initial state */
	LinphoneConferenceStateInstantiated = 1,	/**< Conference is now instantiated on local */
	LinphoneConferenceStateCreationPending = 2,	/**< One creation request was sent to the server */
	LinphoneConferenceStateCreated = 3,		/**< Conference was created on the server */
	LinphoneConferenceStateCreationFailed = 4,	/**< Conference creation failed */
	LinphoneConferenceStateTerminationPending = 5,	/**< Wait for conference termination */
	LinphoneConferenceStateTerminated = 6,		/**< Conference exists on server but not in local */
	LinphoneConferenceStateTerminationFailed = 7,	/**< Conference termination failed */
	LinphoneConferenceStateDeleted = 8,		 /**< Conference was deleted on the server */
} LinphoneConferenceState;

/**
 * #LinphoneConferenceLayout is used to indicate the layout used by the conference.
 * @ingroup conference
 */
typedef enum _LinphoneConferenceLayout{
	LinphoneConferenceLayoutNone,          /**< None - participant who speaks is prominently displayed in the center of the screen */
	LinphoneConferenceLayoutActiveSpeaker, /**< Active speaker - participant who speaks is prominently displayed in the center of the screen and other participants are minimized */
	LinphoneConferenceLayoutGrid,          /**< Grid - each participant is given an equal sized image size */
} LinphoneConferenceLayout;

/**
 * @brief Type of conference participant list.
 * When participant list is closed, no more participants can be added other than those declared when creating the conference
 * When participant list is open, other people can join the conference upon invitation of a participant
no more participants can be added other than those declared when creating the conference
 *
 * @ingroup conference
 */
typedef enum _LinphoneConferenceParticipantListType{
	LinphoneConferenceParticipantListTypeClosed = 0, /**< Only participants in the initiating INVITE are allowed to join the conference */
	LinphoneConferenceParticipantListTypeOpen = 1, /**< All devices calling the conference URI are allowed to join the conference */
} LinphoneConferenceParticipantListType;

/**
 * @brief Describes conference scheduler possible states.
 *
 * It is notified via the conference_scheduler_state_changed callback in #LinphoneConferenceSchedulerCbs.
 * @ingroup conference
**/
typedef enum _LinphoneConferenceSchedulerState {
	LinphoneConferenceSchedulerStateIdle = 0, /**< Default state of a freshly created #LinphoneConferenceScheduler */
	LinphoneConferenceSchedulerStateError = 1, /**< An error has happened during conference creation */
	LinphoneConferenceSchedulerStateAllocationPending = 2, /**< Conference creation is in progress */
	LinphoneConferenceSchedulerStateReady = 3, /**< Confererence has been created */
	LinphoneConferenceSchedulerStateUpdating = 4, /**< Conference has been updated */
} LinphoneConferenceSchedulerState;

#endif // ifndef _L_CONFERENCE_ENUMS_H_
