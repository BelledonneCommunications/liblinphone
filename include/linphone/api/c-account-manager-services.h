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

#ifndef LINPHONE_ACCOUNT_MANAGER_SERVICES_H
#define LINPHONE_ACCOUNT_MANAGER_SERVICES_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup account_creator
 * @{
 */

/**
 * Takes a reference on a #LinphoneAccountManagerServices.
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @return the same #LinphoneAccountManagerServices object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServices *
linphone_account_manager_services_ref(LinphoneAccountManagerServices *ams);

/**
 * Releases a #LinphoneAccountManagerServices.
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 */
LINPHONE_PUBLIC void linphone_account_manager_services_unref(LinphoneAccountManagerServices *ams);

// -----------------------------------------------------------------------------

/**
 * Sets the preferred language to get error messages from account manager (if supported, otherwise will be english).
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param language The language (en, fr, etc...) you'd like to have error messages in (if possible). @maybenil
 */
LINPHONE_PUBLIC void linphone_account_manager_services_set_language(LinphoneAccountManagerServices *ams,
                                                                    const char *language);

/**
 * Gets the configurer preferred language, if set.
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @return the language previously set, if any (otherwise NULL). @maybenil
 */
LINPHONE_PUBLIC const char *linphone_account_manager_services_get_language(const LinphoneAccountManagerServices *ams);

// -----------------------------------------------------------------------------

/**
 * Requests a push notification to be sent to device, containing a valid account creation token.
 * Provider, param & prid can be obtained from linphone_core_get_push_notification_config(),
 * but on iOS may need some modifications (depending on debug mode for example).
 * Once the token is obtained, you can call linphone_account_manager_services_create_new_account_using_token_request().
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param pn_provider The provider, for example "apns.dev". @notnil
 * @param pn_param The parameters, for example "ABCD1234.org.linphone.phone.remote". @notnil
 * @param pn_prid The prid, also known as push token. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_send_account_creation_token_by_push_request(
    LinphoneAccountManagerServices *ams, const char *pn_provider, const char *pn_param, const char *pn_prid);

/**
 * Requests a push notification to be sent to device, containing a valid account recovery token.
 * Provider, param & prid can be obtained from linphone_core_get_push_notification_config(),
 * but on iOS may need some modifications (depending on debug mode for example).
 * Once the token is obtained, you can use it to open the recovery webpage on the flexisip account manager
 * at https://account_manager.domain.tld/recovery/phone/<recovery token>?phone=<phone number>
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param pn_provider The provider, for example "apns.dev". @notnil
 * @param pn_param The parameters, for example "ABCD1234.org.linphone.phone.remote". @notnil
 * @param pn_prid The prid, also known as push token. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_send_account_recovery_token_by_push_request(
    LinphoneAccountManagerServices *ams, const char *pn_provider, const char *pn_param, const char *pn_prid);

/**
 * Requests a an account creation request token that once validated using the URL returned by this method upon success,
 * will allow you to call
 * linphone_account_manager_services_create_get_account_creation_token_from_request_token_request() to obtain a valid
 * account creation token. Once the account creation token is obtained, you can call
 * linphone_account_manager_services_create_new_account_using_token_request().
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_account_creation_request_token_request(
    LinphoneAccountManagerServices *ams);

/**
 * Converts an account creation request token obtained by
 * linphone_account_manager_services_request_account_creation_request_token to an account creation token. The obtained
 * token can be used to call linphone_account_manager_services_create_new_account_using_token_request().
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param request_token the token obtained previously and validated using your web browser. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_account_creation_token_from_request_token_request(
    LinphoneAccountManagerServices *ams, const char *request_token);

// -----------------------------------------------------------------------------

/**
 * Creates an account using an account creation token,
 * for example obtained using linphone_account_manager_services_create_send_account_creation_token_by_push_request().
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param username the username of the newly created account. @notnil
 * @param password the password to use for the newly created account. @notnil
 * @param algorithm the algorithm to use to hash the password (must be either MD5 or SHA-256). @notnil
 * @param token the account creation token obtained previously. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_new_account_using_token_request(LinphoneAccountManagerServices *ams,
                                                                         const char *username,
                                                                         const char *password,
                                                                         const char *algorithm,
                                                                         const char *token);

// -----------------------------------------------------------------------------

/**
 * Requests a code to be sent to a given phone number, that can be used later to associate said phone number
 * to an account using linphone_account_manager_services_create_link_phone_number_to_account_using_code_request().
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param sip_identity the SIP identity URI that identifies the account to which you want to link the phone number to.
 * @notnil
 * @param phone_number the phone number to which to send the code by SMS. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_send_phone_number_linking_code_by_sms_request(
    LinphoneAccountManagerServices *ams, const LinphoneAddress *sip_identity, const char *phone_number);

/**
 * Validates the link between a phone number and an account
 * using a code received by SMS after calling
 * linphone_account_manager_services_create_send_phone_number_linking_code_by_sms_request()
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param sip_identity the SIP identity URI that identifies the account to which you want to link the phone number to.
 * @notnil
 * @param code the code received by SMS. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_link_phone_number_to_account_using_code_request(
    LinphoneAccountManagerServices *ams, const LinphoneAddress *sip_identity, const char *code);

/**
 * Requests a code to be sent to a given email address, that can be used later to associate said email
 * to an account using linphone_account_manager_services_create_link_email_to_account_using_code_request().
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param sip_identity the SIP identity URI that identifies the account to which you want to link the email address to.
 * @notnil
 * @param email_address the email address to which to send the code to. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_send_email_linking_code_by_email_request(LinphoneAccountManagerServices *ams,
                                                                                  const LinphoneAddress *sip_identity,
                                                                                  const char *email_address);

/**
 * Validates the link between an email address and an account using a code received by email after calling
 * linphone_account_manager_services_create_send_email_linking_code_by_email_request()
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param sip_identity the SIP identity URI that identifies the account to which you want to link the email address to.
 * @notnil
 * @param code the code received by email. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_link_email_to_account_using_code_request(LinphoneAccountManagerServices *ams,
                                                                                  const LinphoneAddress *sip_identity,
                                                                                  const char *code);

// -----------------------------------------------------------------------------

/**
 * Requests the list of devices currently known.
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param sip_identity the SIP identity URI that identifies your account for which you want the devices list. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_devices_list_request(LinphoneAccountManagerServices *ams,
                                                                  const LinphoneAddress *sip_identity);

/**
 * Requests to delete a device from the list of currently known devices.
 * @param ams The #LinphoneAccountManagerServices object. @notnil
 * @param sip_identity the SIP identity URI that identifies your account for which you want the devices list. @notnil
 * @param device the #LinphoneAccountDevice to delete. @notnil
 * @return the #LinphoneAccountManagerServicesRequest request object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *linphone_account_manager_services_create_delete_device_request(
    LinphoneAccountManagerServices *ams, const LinphoneAddress *sip_identity, const LinphoneAccountDevice *device);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_ACCOUNT_MANAGER_SERVICES_H */
