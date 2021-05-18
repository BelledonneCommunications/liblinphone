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

#ifndef _L_C_ADDRESS_H_
#define _L_C_ADDRESS_H_

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup linphone_address
 * @{
 */

/**
 * Constructs a #LinphoneAddress object by parsing the user supplied address,
 * given as a string.
 * @param address an address to parse. @notnil
 * @return a LinphoneAddress if parsing is successful, NULL otherwise. @maybenil
 **/
LINPHONE_PUBLIC LinphoneAddress *linphone_address_new (const char *address);

/**
 * Clones a #LinphoneAddress object.
 * @param address a #LinphoneAddress object to clone. @notnil
 * @return a new #LinphoneAddress object. @notnil
 **/
LINPHONE_PUBLIC LinphoneAddress *linphone_address_clone (const LinphoneAddress *address);

/**
 * Increment reference count of #LinphoneAddress object.
 * @param address a #LinphoneAddress object. @notnil
 * @return the same #LinphoneAddress object. @notnil
 **/
LINPHONE_PUBLIC LinphoneAddress *linphone_address_ref (LinphoneAddress *address);

/**
 * Decrement reference count of #LinphoneAddress object. When dropped to zero, memory is freed.
 * @param address a #LinphoneAddress object. @notnil
 **/
LINPHONE_PUBLIC void linphone_address_unref (LinphoneAddress *address);

/**
 * Returns the address scheme, normally "sip".
 * @param address a #LinphoneAddress object. @notnil
 * @return the scheme if any, NULL otherwise. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_address_get_scheme (const LinphoneAddress *address);

/**
 * Returns the display name.
 * @param address a #LinphoneAddress object. @notnil
 * @return the display name if any, NULL otherwise. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_address_get_display_name (const LinphoneAddress *address);

/**
 * Sets the display name.
 * @param address a #LinphoneAddress object. @notnil
 * @param display_name the display name to set. @maybenil
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_address_set_display_name (LinphoneAddress *address, const char *display_name);

/**
 * Returns the username.
 * @param address a #LinphoneAddress object. @notnil
 * @return the username name if any, NULL otherwise. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_address_get_username (const LinphoneAddress *address);

/**
 * Sets the username.
 * @param address a #LinphoneAddress object. @notnil
 * @param username the username to set. @maybenil
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_address_set_username (LinphoneAddress *address, const char *username);

/**
 * Returns the domain name.
 * @param address a #LinphoneAddress object. @notnil
 * @return the domain name if any, NULL otherwise. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_address_get_domain (const LinphoneAddress *address);

/**
 * Sets the domain.
 * @param address a #LinphoneAddress object. @notnil
 * @param domain the domain to set. @maybenil
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_address_set_domain (LinphoneAddress *address, const char *domain);

/**
 * Get port number as an integer value, 0 if not present.
 * @param address a #LinphoneAddress object. @notnil
 * @return the port set in the address or 0 if not present.
 */
LINPHONE_PUBLIC int linphone_address_get_port (const LinphoneAddress *address);

/**
 * Sets the port number.
 * @param address a #LinphoneAddress object. @notnil
 * @param port the port to set in the address
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_address_set_port (LinphoneAddress *address, int port);

/**
 * Get the transport.
 * @param address a #LinphoneAddress object. @notnil
 * @return a #LinphoneTransportType, default value if not set is UDP.
 **/
LINPHONE_PUBLIC LinphoneTransportType linphone_address_get_transport (const LinphoneAddress *address);

/**
 * Set a transport.
 * @param address a #LinphoneAddress object. @notnil
 * @param transport a #LinphoneTransportType
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_address_set_transport (LinphoneAddress *address, LinphoneTransportType transport);

/**
 * Returns whether the address refers to a secure location (sips) or not
 * @param address a #LinphoneAddress object. @notnil
 * @return TRUE if address refers to a secure location, FALSE otherwise
 **/
LINPHONE_PUBLIC bool_t linphone_address_get_secure (const LinphoneAddress *address);

/**
 * Make the address refer to a secure location (sips scheme)
 * @param address A #LinphoneAddress object. @notnil
 * @param enabled TRUE if address is requested to be secure.
 **/
LINPHONE_PUBLIC void linphone_address_set_secure (LinphoneAddress *address, bool_t enabled);

/**
 * returns whether the address is a routable SIP address or not
 * @param address a #LinphoneAddress object. @notnil
 * @return TRUE if it is a routable SIP address, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_address_is_sip (const LinphoneAddress *address);

/**
 * Get the value of the method parameter
 * @param address a #LinphoneAddress object. @notnil
 * @return the value of the parameter or NULL. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_address_get_method_param (const LinphoneAddress *address);

/**
 * Set the value of the method parameter
 * @param address a #LinphoneAddress object. @notnil
 * @param method_param the value to set to the method parameter. @maybenil
 **/
LINPHONE_PUBLIC void linphone_address_set_method_param (LinphoneAddress *address, const char *method_param);

/**
 * Get the password encoded in the address.
 * It is used for basic authentication (not recommended).
 * @param address a #LinphoneAddress object. @notnil
 * @return the password if any, NULL otherwise. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_address_get_password (const LinphoneAddress *address);

/**
 * Set the password encoded in the address.
 * It is used for basic authentication (not recommended).
 * @param address a #LinphoneAddress object. @notnil
 * @param password the password to set. @maybenil
 **/
LINPHONE_PUBLIC void linphone_address_set_password (LinphoneAddress *address, const char *password);

/**
 * Removes address's tags and uri headers so that it is displayable to the user.
 * @param address a #LinphoneAddress object. @notnil
 **/
LINPHONE_PUBLIC void linphone_address_clean (LinphoneAddress *address);

/**
 * Returns the address as a string.
 * The returned char * must be freed by the application. Use ms_free().
 * @param address a #LinphoneAddress object. @notnil
 * @return a string representation of the address. @notnil @tobefreed
 **/
LINPHONE_PUBLIC char *linphone_address_as_string (const LinphoneAddress *address);

/**
 * Returns the SIP uri only as a string, that is display name is removed.
 * The returned char * must be freed by the application. Use ms_free().
 * @param address a #LinphoneAddress object. @notnil
 * @return a string representation of the address. @notnil @tobefreed
 **/
LINPHONE_PUBLIC char *linphone_address_as_string_uri_only (const LinphoneAddress *address);

/**
 * Compare two #LinphoneAddress ignoring tags and headers, basically just domain, username, and port.
 * @param address1 #LinphoneAddress object. @notnil
 * @param address2 #LinphoneAddress object. @notnil
 * @return Boolean value telling if the #LinphoneAddress objects are equal.
 * @see linphone_address_equal()
 **/
LINPHONE_PUBLIC bool_t linphone_address_weak_equal (const LinphoneAddress *address1, const LinphoneAddress *address2);

/**
 * Compare two #LinphoneAddress taking the tags and headers into account.
 * @param address1 #LinphoneAddress object. @notnil
 * @param address2 #LinphoneAddress object. @notnil
 * @return Boolean value telling if the #LinphoneAddress objects are equal.
 * @see linphone_address_weak_equal()
 */
LINPHONE_PUBLIC bool_t linphone_address_equal (const LinphoneAddress *address1, const LinphoneAddress *address2);

/**
 * Get the header encoded in the address.
 * @param address a #LinphoneAddress object. @notnil
 * @param header_name the header name. @notnil
 * @return the header value or NULL if it doesn't exists. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_address_get_header (const LinphoneAddress *address, const char *header_name);

/**
 * Set a header into the address.
 * Headers appear in the URI with '?', such as \<sip:test@linphone.org?SomeHeader=SomeValue\>.
 * @param address a #LinphoneAddress object. @notnil
 * @param header_name the header name. @notnil
 * @param header_value the header value. @maybenil
 **/
LINPHONE_PUBLIC void linphone_address_set_header (LinphoneAddress *address, const char *header_name, const char *header_value);

/**
 * Tell whether a parameter is present in the address
 * @param address a #LinphoneAddress object. @notnil
 * @param param_name The name of the parameter. @notnil
 * @return A boolean value telling whether the parameter is present in the address
 */
LINPHONE_PUBLIC bool_t linphone_address_has_param (const LinphoneAddress *address, const char *param_name);

/**
 * Get the value of a parameter of the address
 * @param address a #LinphoneAddress object. @notnil
 * @param param_name The name of the parameter. @notnil
 * @return The value of the parameter or NULL if it doesn't exists. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_address_get_param (const LinphoneAddress *address, const char *param_name);

/**
 * Set the value of a parameter of the address
 * @param address a #LinphoneAddress object. @notnil
 * @param param_name The name of the parameter. @notnil
 * @param param_value The new value of the parameter. @maybenil
 */
LINPHONE_PUBLIC void linphone_address_set_param (LinphoneAddress *address, const char *param_name, const char *param_value);

LINPHONE_PUBLIC void linphone_address_set_params (LinphoneAddress *address, const char *params);

/**
 * Tell whether a parameter is present in the URI of the address
 * @param address a #LinphoneAddress object. @notnil
 * @param uri_param_name The name of the parameter. @notnil
 * @return A boolean value telling whether the parameter is present in the URI of the address
 */
LINPHONE_PUBLIC bool_t linphone_address_has_uri_param (const LinphoneAddress *address, const char *uri_param_name);

/**
 * Get the value of a parameter of the URI of the address
 * @param address a #LinphoneAddress object. @notnil
 * @param uri_param_name The name of the parameter. @notnil
 * @return The value of the parameter or NULL if it doesn't exists. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_address_get_uri_param (const LinphoneAddress *address, const char *uri_param_name);

/**
 * Set the value of a parameter of the URI of the address
 * @param address a #LinphoneAddress object. @notnil
 * @param uri_param_name The name of the parameter. @notnil
 * @param uri_param_value The new value of the parameter. @maybenil
 */
LINPHONE_PUBLIC void linphone_address_set_uri_param (LinphoneAddress *address, const char *uri_param_name, const char *uri_param_value);

/**
 * Set the value of the parameters of the URI of the address
 * @param[in] address #LinphoneAddress object
 * @param[in] params The parameters string
 */
LINPHONE_PUBLIC void linphone_address_set_uri_params (LinphoneAddress *address, const char *params);

/**
 * Removes the value of a parameter of the URI of the address
 * @param address a #LinphoneAddress object. @notnil
 * @param uri_param_name The name of the parameter. @notnil
 */
LINPHONE_PUBLIC void linphone_address_remove_uri_param (LinphoneAddress *address, const char *uri_param_name);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Destroys a #LinphoneAddress object (actually calls linphone_address_unref()).
 * @deprecated 04/09/17 Use linphone_address_unref() instead
 * @donotwrap
 **/
LINPHONE_DEPRECATED LINPHONE_PUBLIC void linphone_address_destroy (LinphoneAddress *address);

/**
 * Returns true if address refers to a secure location (sips)
 * @deprecated 04/09/17 use linphone_address_get_secure()
 * @donotwrap
 **/
LINPHONE_DEPRECATED LINPHONE_PUBLIC bool_t linphone_address_is_secure (const LinphoneAddress *address);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_ADDRESS_H_
