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

#include "bctoolbox/defs.h"

#include "c-wrapper/internal/c-tools.h"
#include "ldap/ldap-params.h"
#include "linphone/api/c-carddav-params.h"
#include "linphone/api/c-ldap-params.h"
#include "linphone/api/c-remote-contact-directory.h"
#include "search/remote-contact-directory.h"
#include "vcard/carddav-params.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneRemoteContactDirectory *
linphone_remote_contact_directory_ref(LinphoneRemoteContactDirectory *remote_contact_directory) {
	RemoteContactDirectory::toCpp(remote_contact_directory)->ref();
	return remote_contact_directory;
}

void linphone_remote_contact_directory_unref(LinphoneRemoteContactDirectory *remote_contact_directory) {
	RemoteContactDirectory::toCpp(remote_contact_directory)->unref();
}

// =============================================================================

LinphoneRemoteContactDirectoryType
linphone_remote_contact_directory_get_type(const LinphoneRemoteContactDirectory *remote_contact_directory) {
	return RemoteContactDirectory::toCpp(remote_contact_directory)->getType();
}

LinphoneCardDavParams *
linphone_remote_contact_directory_get_card_dav_params(LinphoneRemoteContactDirectory *remote_contact_directory) {
	auto &cardDavParams = RemoteContactDirectory::toCpp(remote_contact_directory)->getCardDavParams();
	return (cardDavParams) ? cardDavParams->toC() : nullptr;
}

LinphoneLdapParams *
linphone_remote_contact_directory_get_ldap_params(LinphoneRemoteContactDirectory *remote_contact_directory) {
	auto &ldapParams = RemoteContactDirectory::toCpp(remote_contact_directory)->getLdapParams();
	return (ldapParams) ? ldapParams->toC() : nullptr;
}

// =============================================================================

const char *
linphone_remote_contact_directory_get_server_url(const LinphoneRemoteContactDirectory *remote_contact_directory) {
	return L_STRING_TO_C(RemoteContactDirectory::toCpp(remote_contact_directory)->getServerUrl());
}

void linphone_remote_contact_directory_set_server_url(LinphoneRemoteContactDirectory *remote_contact_directory,
                                                      const char *server_url) {
	RemoteContactDirectory::toCpp(remote_contact_directory)->setServerUrl(L_C_TO_STRING(server_url));
}

unsigned int
linphone_remote_contact_directory_get_limit(const LinphoneRemoteContactDirectory *remote_contact_directory) {
	return RemoteContactDirectory::toCpp(remote_contact_directory)->getLimit();
}

void linphone_remote_contact_directory_set_limit(LinphoneRemoteContactDirectory *remote_contact_directory,
                                                 unsigned int limit) {
	RemoteContactDirectory::toCpp(remote_contact_directory)->setLimit(limit);
}

unsigned int
linphone_remote_contact_directory_get_min_characters(const LinphoneRemoteContactDirectory *remote_contact_directory) {
	return RemoteContactDirectory::toCpp(remote_contact_directory)->getMinCharactersToStartQuery();
}

void linphone_remote_contact_directory_set_min_characters(LinphoneRemoteContactDirectory *remote_contact_directory,
                                                          unsigned int min) {
	RemoteContactDirectory::toCpp(remote_contact_directory)->setMinCharactersToStartQuery(min);
}

unsigned int
linphone_remote_contact_directory_get_timeout(const LinphoneRemoteContactDirectory *remote_contact_directory) {
	return RemoteContactDirectory::toCpp(remote_contact_directory)->getTimeout();
}

void linphone_remote_contact_directory_set_timeout(LinphoneRemoteContactDirectory *remote_contact_directory,
                                                   unsigned int seconds) {
	RemoteContactDirectory::toCpp(remote_contact_directory)->setTimeout(seconds);
}
