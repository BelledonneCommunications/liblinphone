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

#ifndef LINPHONE_NAT_POLICY_H_
#define LINPHONE_NAT_POLICY_H_


#include "linphone/types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup network_parameters
 * @{
 */

/**
 * Acquire a reference to the #LinphoneNatPolicy object.
 * @param policy #LinphoneNatPolicy object. @notnil
 * @return The same #LinphoneNatPolicy object. @notnil
**/
LINPHONE_PUBLIC LinphoneNatPolicy * linphone_nat_policy_ref(LinphoneNatPolicy *policy);

/**
 * Release reference to the #LinphoneNatPolicy object.
 * @param policy #LinphoneNatPolicy object. @notnil
**/
LINPHONE_PUBLIC void linphone_nat_policy_unref(LinphoneNatPolicy *policy);

/**
 * Retrieve the user pointer associated with the #LinphoneNatPolicy object.
 * @param policy #LinphoneNatPolicy object. @notnil
 * @return The user pointer associated with the #LinphoneNatPolicy object. @maybenil
**/
LINPHONE_PUBLIC void *linphone_nat_policy_get_user_data(const LinphoneNatPolicy *policy);

/**
 * Assign a user pointer to the #LinphoneNatPolicy object.
 * @param policy #LinphoneNatPolicy object. @notnil
 * @param user_data The user pointer to associate with the #LinphoneNatPolicy object. @maybenil
**/
LINPHONE_PUBLIC void linphone_nat_policy_set_user_data(LinphoneNatPolicy *policy, void *user_data);

/**
 * Clear a NAT policy (deactivate all protocols and unset the STUN server).
 * @param policy #LinphoneNatPolicy object. @notnil
 */
LINPHONE_PUBLIC void linphone_nat_policy_clear(LinphoneNatPolicy *policy);

/**
 * Tell whether STUN is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return Boolean value telling whether STUN is enabled.
 */
LINPHONE_PUBLIC bool_t linphone_nat_policy_stun_enabled(const LinphoneNatPolicy *policy);

/**
 * Enable STUN.
 * If TURN is also enabled, TURN will be used instead of STUN.
 * @param policy #LinphoneNatPolicy object @notnil
 * @param enable Boolean value telling whether to enable STUN.
 */
LINPHONE_PUBLIC void linphone_nat_policy_enable_stun(LinphoneNatPolicy *policy, bool_t enable);

/**
 * Tell whether TURN is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return Boolean value telling whether TURN is enabled.
 */
LINPHONE_PUBLIC bool_t linphone_nat_policy_turn_enabled(const LinphoneNatPolicy *policy);

/**
 * Enable TURN.
 * If STUN is also enabled, it is ignored and TURN is used.
 * @param policy #LinphoneNatPolicy object @notnil
 * @param enable Boolean value telling whether to enable TURN.
 */
LINPHONE_PUBLIC void linphone_nat_policy_enable_turn(LinphoneNatPolicy *policy, bool_t enable);

/**
 * Tell whether ICE is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return Boolean value telling whether ICE is enabled.
 */
LINPHONE_PUBLIC bool_t linphone_nat_policy_ice_enabled(const LinphoneNatPolicy *policy);

/**
 * Enable ICE.
 * ICE can be enabled without STUN/TURN, in which case only the local candidates will be used.
 * @param policy #LinphoneNatPolicy object @notnil
 * @param enable Boolean value telling whether to enable ICE.
 */
LINPHONE_PUBLIC void linphone_nat_policy_enable_ice(LinphoneNatPolicy *policy, bool_t enable);

/**
 * Tell whether uPnP is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return Boolean value telling whether uPnP is enabled.
 */
LINPHONE_PUBLIC bool_t linphone_nat_policy_upnp_enabled(const LinphoneNatPolicy *policy);

/**
 * Enable uPnP.
 * This has the effect to disable every other policies (ICE, STUN and TURN).
 * @param policy #LinphoneNatPolicy object @notnil
 * @param enable Boolean value telling whether to enable uPnP.
 */
LINPHONE_PUBLIC void linphone_nat_policy_enable_upnp(LinphoneNatPolicy *policy, bool_t enable);

/**
 * Get the STUN/TURN server to use with this NAT policy.
 * Used when STUN or TURN are enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return The STUN server used by this NAT policy. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_nat_policy_get_stun_server(const LinphoneNatPolicy *policy);

/**
 * Set the STUN/TURN server to use with this NAT policy.
 * Used when STUN or TURN are enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @param stun_server The STUN server to use with this NAT policy. @maybenil
 */
LINPHONE_PUBLIC void linphone_nat_policy_set_stun_server(LinphoneNatPolicy *policy, const char *stun_server);

/**
 * Get the username used to authenticate with the STUN/TURN server.
 * The authentication will search for a #LinphoneAuthInfo with this username.
 * If it is not set the username of the currently used #LinphoneProxyConfig is used to search for a LinphoneAuthInfo.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return The username used to authenticate with the STUN/TURN server. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_nat_policy_get_stun_server_username(const LinphoneNatPolicy *policy);

/**
 * Set the username used to authenticate with the STUN/TURN server.
 * The authentication will search for a #LinphoneAuthInfo with this username.
 * If it is not set the username of the currently used #LinphoneProxyConfig is used to search for a LinphoneAuthInfo.
 * @param policy #LinphoneNatPolicy object @notnil
 * @param username The username used to authenticate with the STUN/TURN server. @maybenil
 */
LINPHONE_PUBLIC void linphone_nat_policy_set_stun_server_username(LinphoneNatPolicy *policy, const char *username);

/**
 * Start a STUN server DNS resolution.
 * @param policy #LinphoneNatPolicy object @notnil
 */
LINPHONE_PUBLIC void linphone_nat_policy_resolve_stun_server(LinphoneNatPolicy *policy);

/**
 * Get the addrinfo representation of the STUN server address.
 * WARNING: This function may block for up to 1 second.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return addrinfo representation of the STUN server address.
 * @donotwrap
 */
LINPHONE_PUBLIC const struct addrinfo * linphone_nat_policy_get_stun_server_addrinfo(LinphoneNatPolicy *policy);

/**
 * Enable UDP TURN transport.
 * Used when TURN is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @param enable Boolean value telling whether to enable UDP TURN transport.
 */
LINPHONE_PUBLIC void linphone_nat_policy_enable_udp_turn_transport(LinphoneNatPolicy *policy, bool_t enable);

/**
 * Tells whether UDP TURN transport is enabled.
 * Used when TURN is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return Boolean value telling whether UDP TURN transport is enabled.
 */
LINPHONE_PUBLIC bool_t linphone_nat_policy_udp_turn_transport_enabled(LinphoneNatPolicy *policy);

/**
 * Enable TCP TURN transport.
 * Used when TURN is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @param enable Boolean value telling whether to enable TCP TURN transport.
 */
LINPHONE_PUBLIC void linphone_nat_policy_enable_tcp_turn_transport(LinphoneNatPolicy *policy, bool_t enable);

/**
 * Tells whether TCP TURN transport is enabled.
 * Used when TURN is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return Boolean value telling whether TCP TURN transport is enabled.
 */
LINPHONE_PUBLIC bool_t linphone_nat_policy_tcp_turn_transport_enabled(LinphoneNatPolicy *policy);

/**
 * Enable TLS TURN transport.
 * Used when TURN is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @param enable Boolean value telling whether to enable TLS TURN transport.
 */
LINPHONE_PUBLIC void linphone_nat_policy_enable_tls_turn_transport(LinphoneNatPolicy *policy, bool_t enable);

/**
 * Tells whether TLS TURN transport is enabled.
 * Used when TURN is enabled.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return Boolean value telling whether TLS TURN transport is enabled.
 */
LINPHONE_PUBLIC bool_t linphone_nat_policy_tls_turn_transport_enabled(LinphoneNatPolicy *policy);

/**
 * Returns the #LinphoneCore object managing this nat policy, if any.
 * @param policy #LinphoneNatPolicy object @notnil
 * @return the #LinphoneCore object associated. @notnil
 */
LINPHONE_PUBLIC LinphoneCore *linphone_nat_policy_get_core(const LinphoneNatPolicy *policy);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_NAT_POLICY_H_ */
