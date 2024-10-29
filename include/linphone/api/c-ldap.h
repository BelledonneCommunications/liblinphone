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

#ifndef LINPHONE_LDAP_H
#define LINPHONE_LDAP_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ldap
 * @{
 */

/**
 * Create a new #LinphoneLdap.
 *
 Call * linphone_ldap_set_params() to store the new LDAP configuration.
 *
 * @param lc The #LinphoneCore object. @maybenil
 * @return The newly created #LinphoneLdap object. @notnil @tobefreed
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 */
LINPHONE_PUBLIC LinphoneLdap *linphone_ldap_new(LinphoneCore *lc);

/**
 * Create a new #LinphoneLdap, associate it with the #LinphoneLdapParams and store it into the configuration file.
 *
 * @param lc The #LinphoneCore object. @notnil
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The newly created #LinphoneLdap object. @notnil @tobefreed
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 */
LINPHONE_PUBLIC LinphoneLdap *linphone_ldap_new_with_params(LinphoneCore *lc, LinphoneLdapParams *params);

/**
 * Take a reference on a #LinphoneLdap.
 * @param ldap The #LinphoneLdap object. @notnil
 * @return the same #LinphoneLdap object. @notnil
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 */
LINPHONE_PUBLIC LinphoneLdap *linphone_ldap_ref(LinphoneLdap *ldap);

/**
 * Release a #LinphoneLdap.
 * @param ldap The #LinphoneLdap object. @notnil
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 */
LINPHONE_PUBLIC void linphone_ldap_unref(LinphoneLdap *ldap);

/**
 * Set the #LinphoneLdapParams used by this #LinphoneLdap. The parameters will be saved in the configuration file.
 *
 * @param ldap The #LinphoneLdap object. @notnil
 * @param params The #LinphoneLdapParams object. @notnil
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 */
LINPHONE_PUBLIC void linphone_ldap_set_params(LinphoneLdap *ldap, LinphoneLdapParams *params);

/**
 * Get the #LinphoneLdapParams as read-only object.
 * To make changes, clone the returned object using linphone_ldap_params_clone() method,
 * make your changes on it and apply them using with linphone_ldap_set_params().
 * @param ldap The #LinphoneLdap object. @notnil
 * @return The #LinphoneLdapParams attached to this ldap. @notnil
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 */
LINPHONE_PUBLIC LinphoneLdapParams *linphone_ldap_get_params(LinphoneLdap *ldap);

/**
 * Get the #LinphoneCore object to which is associated the #LinphoneLdap.
 * @param ldap The #LinphoneLdap object. @notnil
 * @return The #LinphoneCore object to which is associated the #LinphoneLdap. @notnil
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 **/
LINPHONE_PUBLIC LinphoneCore *linphone_ldap_get_core(LinphoneLdap *ldap);

/**
 * Set the index associated to the #LinphoneLdap.
 * @param ldap The #LinphoneLdap object. @notnil
 * @param index The index of the Ldap. Can be -1 : it will be determined on save.
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 **/
LINPHONE_PUBLIC void linphone_ldap_set_index(LinphoneLdap *ldap, int index);

/**
 * Get the index of the #LinphoneLdap.
 * @param ldap The #LinphoneLdap object. @notnil
 * @return The index of the Ldap
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 **/
LINPHONE_PUBLIC int linphone_ldap_get_index(const LinphoneLdap *ldap);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_LDAP_H */
