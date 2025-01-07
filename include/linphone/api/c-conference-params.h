/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef LINPHONE_CONFERENCE_PARAMS_H
#define LINPHONE_CONFERENCE_PARAMS_H

#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup conference
 * @{
 */

/**
 * Create an object of type a #LinphoneConferenceParams.
 * @param core The #LinphoneCore to pass to the constructor. @notnil
 * @return The created #LinphoneConferenceParams. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceParams *linphone_conference_params_new(LinphoneCore *core);

/**
 * Take a reference on a #LinphoneConferenceParams.
 * @param params The #LinphoneConferenceParams to ref. @notnil
 * @return The freshly refed #LinphoneConferenceParams. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceParams *linphone_conference_params_ref(LinphoneConferenceParams *params);

/**
 * Release a #LinphoneConferenceParams.
 * @param params The #LinphoneConferenceParams to release. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_params_unref(LinphoneConferenceParams *params);

/**
 * Free a #LinphoneConferenceParams
 * @param params #LinphoneConferenceParams to free @notnil
 * @deprecated 17/03/2017 Use linphone_conference_params_unref() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_conference_params_free(LinphoneConferenceParams *params);

/**
 * Clone a #LinphoneConferenceParams
 * @param params The #LinphoneConferenceParams to clone @notnil
 * @return An allocated #LinphoneConferenceParams with the same parameters than params @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceParams *linphone_conference_params_clone(const LinphoneConferenceParams *params);

/**
 * Set the conference subject
 * @param params A #LinphoneConferenceParams @notnil
 * @param subject conference subject @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_params_set_subject(LinphoneConferenceParams *params, const char *subject);

/**
 * Set the conference subject as an UTF8 string.
 * @param params A #LinphoneConferenceParams @notnil
 * @param subject conference subject @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_params_set_subject_utf8(LinphoneConferenceParams *params, const char *subject);

/**
 * Get the conference subject
 * @param params A #LinphoneConferenceParams @notnil
 * @return conference subject. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_params_get_subject(const LinphoneConferenceParams *params);

/**
 * Get the conference subject as an UTF-8 string.
 * @param params A #LinphoneConferenceParams @notnil
 * @return conference subject. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_params_get_subject_utf8(const LinphoneConferenceParams *params);

/**
 * Enable audio capabilities
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If TRUE, audio will be enabled during conference
 * @deprecated 16/12/2021 Use linphone_conference_params_enable_audio() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_conference_params_set_audio_enabled(LinphoneConferenceParams *params,
                                                                                      bool_t enable);

/**
 * Enable audio capabilities
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If TRUE, audio will be enabled during conference
 */
LINPHONE_PUBLIC void linphone_conference_params_enable_audio(LinphoneConferenceParams *params, bool_t enable);

/**
 * Check whether audio capabilities are enabled
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference supports audio capabilities, FALSE otherwise
 * @deprecated 16/12/2021 Use linphone_conference_params_audio_enabled() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t
linphone_conference_params_is_audio_enabled(const LinphoneConferenceParams *params);

/**
 * Check whether audio capabilities are enabled
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference supports audio capabilities, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_audio_enabled(const LinphoneConferenceParams *params);

/**
 * Enable video capabilities
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If TRUE, video will be enabled during conference
 * @deprecated 16/12/2021 Use linphone_conference_params_enable_video() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_conference_params_set_video_enabled(LinphoneConferenceParams *params,
                                                                                      bool_t enable);

/**
 * Check whether video capabilities are enabled
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference supports video capabilities, FALSE otherwise
 * @deprecated 16/12/2021 Use linphone_conference_params_video_enabled() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t
linphone_conference_params_is_video_enabled(const LinphoneConferenceParams *params);

/**
 * Enable video capabilities
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If TRUE, video will be enabled during conference
 */
LINPHONE_PUBLIC void linphone_conference_params_enable_video(LinphoneConferenceParams *params, bool_t enable);

/**
 * Check whether video capabilities are enabled
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference supports video capabilities, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_video_enabled(const LinphoneConferenceParams *params);

/**
 * Enable chat capabilities
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If TRUE, chat is enabled during conference
 * @deprecated 16/12/2021 Use linphone_conference_params_enable_chat() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_conference_params_set_chat_enabled(LinphoneConferenceParams *params,
                                                                                     bool_t enable);

/**
 * Enable chat capabilities
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable If TRUE, chat is enabled during conference
 */
LINPHONE_PUBLIC void linphone_conference_params_enable_chat(LinphoneConferenceParams *params, bool_t enable);

/**
 * Check whether chat capabilities are enabled
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference supports chat capabilities, FALSE otherwise
 * @deprecated 16/12/2021 Use linphone_conference_params_chat_enabled() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t
linphone_conference_params_is_chat_enabled(const LinphoneConferenceParams *params);

/**
 * Check whether chat capabilities are enabled
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference supports chat capabilities, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_chat_enabled(const LinphoneConferenceParams *params);

/**
 * Sets the account for the conference
 * @param params A #LinphoneConferenceParams @notnil
 * @param account a pointer to the account. @maybenil
 * @warning The account can only be changed upon creation of a conference when calling
 * linphone_core_create_conference_with_params
 */
LINPHONE_PUBLIC void linphone_conference_params_set_account(LinphoneConferenceParams *params, LinphoneAccount *account);

/**
 * Returns the account for the conference
 * @param params A #LinphoneConferenceParams @notnil
 * @return a pointer to the account or NULL if it is not set. @maybenil
 */
LINPHONE_PUBLIC LinphoneAccount *linphone_conference_params_get_account(const LinphoneConferenceParams *params);

/**
 * Returns the proxy configuration for the conference
 * @param params A #LinphoneConferenceParams @notnil
 * @return a pointer to the proxy configuration or NULL if it is not set. @maybenil
 * @deprecated 11/01/2022 Use linphone_conference_params_get_account() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneProxyConfig *
linphone_conference_params_get_proxy_cfg(const LinphoneConferenceParams *params);

/**
 * Enable local participant to enter the conference.
 * The local participant is the one driving the local #LinphoneCore. It uses the local sound devices.
 * The default value is TRUE. Setting to FALSE is mostly helpful when using liblinphone on a server application.
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable TRUE if local participant is automatically added to the conference, FALSE otherwise
 * @deprecated 16/12/2021 Use linphone_conference_params_enable_local_participant() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void
linphone_conference_params_set_local_participant_enabled(LinphoneConferenceParams *params, bool_t enable);

/**
 * Enable local participant to enter the conference.
 * The local participant is the one driving the local #LinphoneCore. It uses the local sound devices.
 * The default value is TRUE. Setting to FALSE is mostly helpful when using liblinphone on a server application.
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable TRUE if local participant is automatically added to the conference, FALSE otherwise
 */
LINPHONE_PUBLIC void linphone_conference_params_enable_local_participant(LinphoneConferenceParams *params,
                                                                         bool_t enable);

/**
 * Returns whether local participant has to enter the conference.
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if local participant is by default part of the conference, FALSE otherwise
 * @deprecated 16/12/2021 Use linphone_conference_params_local_participant_enabled() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t
linphone_conference_params_is_local_participant_enabled(const LinphoneConferenceParams *params);

/**
 * Enable local participant to enter the conference.
 * The local participant is the one driving the local #LinphoneCore. It uses the local sound devices.
 * The default value is TRUE. Setting to FALSE is mostly helpful when using liblinphone on a server application.
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable TRUE if local participant is automatically added to the conference, FALSE otherwise
 */
LINPHONE_PUBLIC void linphone_conference_params_enable_local_participant(LinphoneConferenceParams *params,
                                                                         bool_t enable);

/**
 * Returns whether local participant has to enter the conference.
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if local participant is by default part of the conference, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_local_participant_enabled(const LinphoneConferenceParams *params);

/**
 * Enable conference with one participant.
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable TRUE if conference can have only one participant, FALSE otherwise
 * @deprecated 16/12/2021 Use linphone_conference_params_enable_one_participant_conference() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void
linphone_conference_params_set_one_participant_conference_enabled(LinphoneConferenceParams *params, bool_t enable);

/**
 * Enable conference with one participant.
 * @param params A #LinphoneConferenceParams @notnil
 * @param enable TRUE if conference can have only one participant, FALSE otherwise
 */
LINPHONE_PUBLIC void linphone_conference_params_enable_one_participant_conference(LinphoneConferenceParams *params,
                                                                                  bool_t enable);

/**
 * Returns whether conference can have only one participant
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference can have only one participant, FALSE otherwise
 * @deprecated 16/12/2021 Use linphone_conference_params_one_participant_conference_enabled() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t
linphone_conference_params_is_one_participant_conference_enabled(const LinphoneConferenceParams *params);

/**
 * Returns whether conference can have only one participant
 * @param params A #LinphoneConferenceParams @notnil
 * @return TRUE if the conference can have only one participant, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t
linphone_conference_params_one_participant_conference_enabled(const LinphoneConferenceParams *params);

/**
 * Retrieve the desired security level of the conference.
 * @param params The #LinphoneConferenceParams object. @notnil
 * @return The desired security level of the conference.
 */
LINPHONE_PUBLIC LinphoneConferenceSecurityLevel
linphone_conference_params_get_security_level(const LinphoneConferenceParams *params);

/**
 * Set the desired security level of the conference.
 * @param params The #LinphoneConferenceParams object. @notnil
 * @param security_level The desired security level of the conference.
 */
LINPHONE_PUBLIC void linphone_conference_params_set_security_level(LinphoneConferenceParams *params,
                                                                   LinphoneConferenceSecurityLevel security_level);

/**
 * Set the conference factory address of the conference.
 * By default when creating a new conference, the factory address will come from the current proxy configuration.
 * If NULL then the conference will be local else it will be a client conference.
 * @param params The #LinphoneConferenceParams object. @notnil
 * @param address the conference factory address. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_params_set_conference_factory_address(LinphoneConferenceParams *params,
                                                                               const LinphoneAddress *address);

/**
 * Get the conference factory address of the conference that has been set.
 * @param params The #LinphoneConferenceParams object. @notnil
 * @return the factory address conference description. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *
linphone_conference_params_get_conference_factory_address(const LinphoneConferenceParams *params);

/**
 * Set the description of the conference (utf8)
 * @param params The #LinphoneConferenceParams object. @notnil
 * @param description the conference description. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_params_set_description_utf8(LinphoneConferenceParams *params,
                                                                     const char *description);

/**
 * Get conference description (utf8).
 * @param params The #LinphoneConferenceParams object. @notnil
 * @return the conference description. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_conference_params_get_description_utf8(const LinphoneConferenceParams *params);

/**
 * Set the participant list type
 * @param params The #LinphoneConferenceParams object. @notnil
 * @param type Participant list type #LinphoneConferenceParticipantListType. This allows to restrict the access to the
 * conference to a selected set of participants
 */
LINPHONE_PUBLIC void linphone_conference_params_set_participant_list_type(LinphoneConferenceParams *params,
                                                                          LinphoneConferenceParticipantListType type);

/**
 * Set the conference as hidden. This means that the contact address will not have any conference releated attribute
 * such as isfocus, the conference ID and the admin status.
 * @param params The #LinphoneConferenceParams object. @notnil
 * @param hidden Boolean that states whether the conference is hidden or not
 */
LINPHONE_PUBLIC void linphone_conference_params_set_hidden(LinphoneConferenceParams *params, bool_t hidden);

/**
 * Get the value of the hidden flag
 * @param params The #LinphoneConferenceParams object. @notnil
 * @return whether the conference is hidden or not
 */
LINPHONE_PUBLIC bool_t linphone_conference_params_is_hidden(const LinphoneConferenceParams *params);

/**
 * Get the participant list type
 * @param params The #LinphoneConferenceParams object. @notnil
 * @return participant list type #LinphoneConferenceParticipantListType.
 */
LINPHONE_PUBLIC LinphoneConferenceParticipantListType
linphone_conference_params_get_participant_list_type(const LinphoneConferenceParams *params);

/**
 * Returns whether the given parameters are valid or not.
 * @param params The #LinphoneConferenceParams object. @notnil
 * @return TRUE if the given parameters are valid, FALSE otherwise
 **/
LINPHONE_PUBLIC bool_t linphone_conference_params_is_valid(const LinphoneConferenceParams *params);

/**
 * Get the group chat status of the text capabilities of the conference associated with the given parameters.
 * @param params The #LinphoneConferenceParams object. @notnil
 * @return TRUE if group chat is enabled, FALSE if one-to-one
 **/
LINPHONE_PUBLIC bool_t linphone_conference_params_group_enabled(const LinphoneConferenceParams *params);

/**
 * Enables or disables group chat for the text capabilities of the conference associated with the given parameters.
 * @param params The #LinphoneConferenceParams object. @notnil
 * @param group TRUE to enable group chat, FALSE to disable (resulting in one-to-one text capabilities of
 * the conference)
 **/
LINPHONE_PUBLIC void linphone_conference_params_enable_group(LinphoneConferenceParams *params, bool_t group);

/**
 * Get the chat parameters
 * @param params The #LinphoneConferenceParams object. @notnil
 * @return the chat parameters if chat capabilities are on, NULL otherwise @maybenil
 **/
LINPHONE_PUBLIC LinphoneChatParams *linphone_conference_params_get_chat_params(const LinphoneConferenceParams *params);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // LINPHONE_CONFERENCE_PARAMS_H
