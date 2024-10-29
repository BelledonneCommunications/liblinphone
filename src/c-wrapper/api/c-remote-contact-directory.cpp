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
#include "linphone/api/c-remote-contact-directory.h"
#include "search/remote-contact-directory.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneRemoteContactDirectory *linphone_remote_contact_directory_ref(LinphoneRemoteContactDirectory *params) {
	RemoteContactDirectory::toCpp(params)->ref();
	return params;
}

void linphone_remote_contact_directory_unref(LinphoneRemoteContactDirectory *params) {
	RemoteContactDirectory::toCpp(params)->unref();
}

// =============================================================================

LinphoneRemoteContactDirectoryType
linphone_remote_contact_directory_get_type(const LinphoneRemoteContactDirectory *params) {
	return RemoteContactDirectory::toCpp(params)->getType();
}

LinphoneCardDavParams *
linphone_remote_contact_directory_get_card_dav_params(const LinphoneRemoteContactDirectory *params) {
	auto cardDavParams = RemoteContactDirectory::toCpp(params)->getCardDavParams();
	return (cardDavParams) ? cardDavParams->toC() : nullptr;
}

LinphoneLdapParams *linphone_remote_contact_directory_get_ldap_params(const LinphoneRemoteContactDirectory *params) {
	auto ldapParams = RemoteContactDirectory::toCpp(params)->getLdapParams();
	return (ldapParams) ? ldapParams->toC() : nullptr;
}

// =============================================================================

const char *linphone_remote_contact_directory_get_server_url(const LinphoneRemoteContactDirectory *params) {
	return L_STRING_TO_C(RemoteContactDirectory::toCpp(params)->getServerUrl());
}

void linphone_remote_contact_directory_set_server_url(LinphoneRemoteContactDirectory *params, const char *server_url) {
	RemoteContactDirectory::toCpp(params)->setServerUrl(L_C_TO_STRING(server_url));
}

unsigned int linphone_remote_contact_directory_get_limit(const LinphoneRemoteContactDirectory *params) {
	return RemoteContactDirectory::toCpp(params)->getLimit();
}

void linphone_remote_contact_directory_set_limit(LinphoneRemoteContactDirectory *params, unsigned int limit) {
	RemoteContactDirectory::toCpp(params)->setLimit(limit);
}

unsigned int linphone_remote_contact_directory_get_min_characters(const LinphoneRemoteContactDirectory *params) {
	return RemoteContactDirectory::toCpp(params)->getMinCharactersToStartQuery();
}

void linphone_remote_contact_directory_set_min_characters(LinphoneRemoteContactDirectory *params, unsigned int min) {
	RemoteContactDirectory::toCpp(params)->setMinCharactersToStartQuery(min);
}

unsigned int linphone_remote_contact_directory_get_timeout(const LinphoneRemoteContactDirectory *params) {
	return RemoteContactDirectory::toCpp(params)->getTimeout();
}

void linphone_remote_contact_directory_set_timeout(LinphoneRemoteContactDirectory *params, unsigned int seconds) {
	RemoteContactDirectory::toCpp(params)->setTimeout(seconds);
}