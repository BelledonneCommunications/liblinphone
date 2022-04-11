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

#ifndef _L_CONFERENCE_ENUMS_H_
#define _L_CONFERENCE_ENUMS_H_

// =============================================================================
/**
 * #LinphoneConferenceState is used to indicate the current state of a conference.
 * @ingroup conference
 */
typedef enum _LinphoneConferenceState {
	LinphoneConferenceStateNone = 0,               /**< Initial state. */
	LinphoneConferenceStateInstantiated = 1,       /**< Conference is now instantiated locally. */
	LinphoneConferenceStateCreationPending = 2,    /**< One creation request was sent to the service. */
	LinphoneConferenceStateCreated = 3,            /**< Conference was created on the service. */
	LinphoneConferenceStateCreationFailed = 4,     /**< Conference creation on service failed. */
	LinphoneConferenceStateTerminationPending = 5, /**< Wait for conference termination. */
	LinphoneConferenceStateTerminated = 6, /**< The conference is terminated locally, though it may still exist on the
	                                          service for other participants. */
	LinphoneConferenceStateTerminationFailed = 7, /**< Conference termination failed. */
	LinphoneConferenceStateDeleted = 8,           /**< Conference was deleted locally and on the service. */
} LinphoneConferenceState;

/**
 * #LinphoneConferenceLayout is used to indicate the layout used by the conference.
 * @ingroup conference
 */
typedef enum _LinphoneConferenceLayout {
	LinphoneConferenceLayoutGrid = 0,          /**< Grid - each participant is given an equal sized image size */
	LinphoneConferenceLayoutActiveSpeaker = 1, /**< Active speaker - participant who speaks is prominently displayed in
	                                              the center of the screen and other participants are minimized */
} LinphoneConferenceLayout;

/**
 * @brief Mode of joining conference
 * @warning It is only valid at the creation of the conferece
 *
 * @ingroup conference
 */
typedef enum _LinphoneConferenceJoiningMode {
	LinphoneConferenceJoiningModeDialIn = 0,  /**< Participants must dial the conference server */
	LinphoneConferenceJoiningModeDialOut = 1, /**< Conference server dials participants */
} LinphoneConferenceJoiningMode;

/**
 * @brief Type of conference participant list.
 * When participant list is closed, no more participants can be added other than those declared when creating the
conference
 * When participant list is open, other people can join the conference upon invitation of a participant
no more participants can be added other than those declared when creating the conference
 *
 * @ingroup conference
 */
typedef enum _LinphoneConferenceParticipantListType {
	LinphoneConferenceParticipantListTypeClosed =
	    0, /**< Only participants in the initiating INVITE are allowed to join the conference */
	LinphoneConferenceParticipantListTypeOpen =
	    1, /**< All devices calling the conference URI are allowed to join the conference */
} LinphoneConferenceParticipantListType;

/**
 * @brief Conference minimum security level
 *
 * @ingroup conference
 */
typedef enum _LinphoneConferenceSecurityLevel {
	LinphoneConferenceSecurityLevelNone = 0,		 /**< No security */
	LinphoneConferenceSecurityLevelPointToPoint = 1, /**< Point-to-point encryption */
	LinphoneConferenceSecurityLevelEndToEnd = 2,	 /**< End-to-end encryption */
} LinphoneConferenceSecurityLevel;

/**
 * @brief Describes conference scheduler possible states.
 *
 * It is notified via the conference_scheduler_state_changed callback in #LinphoneConferenceSchedulerCbs.
 * @ingroup conference
 **/
typedef enum _LinphoneConferenceSchedulerState {
	LinphoneConferenceSchedulerStateIdle = 0,  /**< Default state of a freshly created #LinphoneConferenceScheduler */
	LinphoneConferenceSchedulerStateError = 1, /**< An error has happened during conference creation */
	LinphoneConferenceSchedulerStateAllocationPending = 2, /**< Conference creation is in progress */
	LinphoneConferenceSchedulerStateReady = 3,             /**< Confererence has been created */
	LinphoneConferenceSchedulerStateUpdating = 4,          /**< Conference has been updated */
} LinphoneConferenceSchedulerState;

// WARNING: Conference info state may be stored in the database, therefore do not modify the integer value of the
// enumeration items
/**
 * #LinphoneConferenceInfoState is used to list all states of a conference info object
 * @ingroup conference
 */
typedef enum _LinphoneConferenceInfoState {
	LinphoneConferenceInfoStateNew = 0,       /**< New conference */
	LinphoneConferenceInfoStateUpdated = 1,   /**< Conference has been updated */
	LinphoneConferenceInfoStateCancelled = 2, /**< Canceling a conference */
} LinphoneConferenceInfoState;
#endif // ifndef _L_CONFERENCE_ENUMS_H_
