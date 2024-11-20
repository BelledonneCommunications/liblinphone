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

#ifndef LINPHONE_REMOTE_CONTACT_DIRECTORY_H
#define LINPHONE_REMOTE_CONTACT_DIRECTORY_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup contacts
 * @{
 */

/**
 * Takes a reference on a #LinphoneRemoteContactDirectory.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @return the same #LinphoneRemoteContactDirectory object. @notnil
 */
LINPHONE_PUBLIC LinphoneRemoteContactDirectory *
linphone_remote_contact_directory_ref(LinphoneRemoteContactDirectory *params);

/**
 * Releases a #LinphoneRemoteContactDirectory.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 */
LINPHONE_PUBLIC void linphone_remote_contact_directory_unref(LinphoneRemoteContactDirectory *params);

/*****************************************************************************************************************/

/**
 * Gets the type of remote contact directory (LDAP, CardDAV).
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @return the #LinphoneRemoteContactDirectoryType of this remote contact directory.
 */
LINPHONE_PUBLIC LinphoneRemoteContactDirectoryType
linphone_remote_contact_directory_get_type(const LinphoneRemoteContactDirectory *params);

/**
 * Gets the CardDAV params if linphone_remote_contact_directory_get_type() returns CardDAV.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @return the #LinphoneCardDavParams or NULL if not of CardDAV type. @maybenil
 */
LINPHONE_PUBLIC LinphoneCardDavParams *
linphone_remote_contact_directory_get_card_dav_params(const LinphoneRemoteContactDirectory *params);

/**
 * Gets the LDAP params if linphone_remote_contact_directory_get_type() returns LDAP.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @return the #LinphoneLdapParams or NULL if not of LDAP type. @maybenil
 */
LINPHONE_PUBLIC LinphoneLdapParams *
linphone_remote_contact_directory_get_ldap_params(const LinphoneRemoteContactDirectory *params);

/**
 * Gets the configured remote contact directory server URL.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @return the remote contact directory server URL. @maybenil
 */
LINPHONE_PUBLIC const char *
linphone_remote_contact_directory_get_server_url(const LinphoneRemoteContactDirectory *params);

/**
 * Sets the server URL to use to reach the remote contact directory server.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @param server_url the remote contact directory server URL. @maybenil
 */
LINPHONE_PUBLIC void linphone_remote_contact_directory_set_server_url(LinphoneRemoteContactDirectory *params,
                                                                      const char *server_url);

/**
 * Gets the maximum number of results to fetch, 0 means no limit.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @return the maximum number of results, 0 means no limit.
 */
LINPHONE_PUBLIC unsigned int linphone_remote_contact_directory_get_limit(const LinphoneRemoteContactDirectory *params);

/**
 * Sets the maximum number of results to fetch, 0 means no limit.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @param limit the maximum number of results to return.
 */
LINPHONE_PUBLIC void linphone_remote_contact_directory_set_limit(LinphoneRemoteContactDirectory *params,
                                                                 unsigned int limit);

/**
 * Gets the minimum number of characters to have before sending the query to the server.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @return The mininum number of characters to have before sending the query.
 */
LINPHONE_PUBLIC unsigned int
linphone_remote_contact_directory_get_min_characters(const LinphoneRemoteContactDirectory *params);

/**
 * Sets the minimum number of characters to have before sending the query to the server.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @param min the minimum characters to have in user input filter before sending the query.
 */
LINPHONE_PUBLIC void linphone_remote_contact_directory_set_min_characters(LinphoneRemoteContactDirectory *params,
                                                                          unsigned int min);

/**
 * Gets the timeout (in seconds) after which the query is abandonned.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @return The timeout (in seconds) after which the query is considered to be timed-out.
 */
LINPHONE_PUBLIC unsigned int
linphone_remote_contact_directory_get_timeout(const LinphoneRemoteContactDirectory *params);

/**
 * Sets the timeout (in seconds) after which the query is abandonned.
 * @param params The #LinphoneRemoteContactDirectory object. @notnil
 * @param seconds the number of seconds before considering the query as timed-out.
 */
LINPHONE_PUBLIC void linphone_remote_contact_directory_set_timeout(LinphoneRemoteContactDirectory *params,
                                                                   unsigned int seconds);

/*****************************************************************************************************************/

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_REMOTE_CONTACT_DIRECTORY_H */
