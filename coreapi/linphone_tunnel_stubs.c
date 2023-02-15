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

#include <bctoolbox/defs.h>

#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "linphone/tunnel.h"
#include "private.h"

#include "tester_utils.h" // for linphone_tunnel_is_tunnel_rtp_transport() declaration

LinphoneTunnel *linphone_core_get_tunnel(const LinphoneCore *lc) {
	return lc->tunnel;
}

/*stubs to avoid to have #ifdef TUNNEL_ENABLED in upper layers*/

void linphone_tunnel_destroy(BCTBX_UNUSED(LinphoneTunnel *tunnel)) {
}

LinphoneTunnel *linphone_tunnel_ref(LinphoneTunnel *tunnel) {
	ms_warning("linphone_tunnel_ref() - stubbed, no implementation");
	return tunnel;
}

void linphone_tunnel_unref(BCTBX_UNUSED(LinphoneTunnel *tunnel)) {
	ms_warning("linphone_tunnel_unref() - stubbed, no implementation");
}

void linphone_tunnel_add_server(BCTBX_UNUSED(LinphoneTunnel *tunnel),
                                BCTBX_UNUSED(LinphoneTunnelConfig *tunnel_config)) {
	ms_warning("linphone_tunnel_add_server() - stubbed, no implementation");
}

void linphone_tunnel_remove_server(BCTBX_UNUSED(LinphoneTunnel *tunnel),
                                   BCTBX_UNUSED(LinphoneTunnelConfig *tunnel_config)) {
	ms_warning("linphone_tunnel_remove_server() - stubbed, no implementation");
}

const MSList *linphone_tunnel_get_servers(BCTBX_UNUSED(const LinphoneTunnel *tunnel)) {
	ms_warning("linphone_tunnel_get_servers() - stubbed, no implementation");
	return NULL;
}

void linphone_tunnel_clean_servers(BCTBX_UNUSED(LinphoneTunnel *tunnel)) {
	ms_warning("linphone_tunnel_clean_servers() - stubbed, no implementation");
}

void linphone_tunnel_set_mode(BCTBX_UNUSED(LinphoneTunnel *tunnel), BCTBX_UNUSED(LinphoneTunnelMode mode)) {
	ms_warning("linphone_tunnel_set_mode() - stubbed, no implementation");
}

LinphoneTunnelMode linphone_tunnel_get_mode(BCTBX_UNUSED(const LinphoneTunnel *tunnel)) {
	ms_warning("linphone_tunnel_get_mode() - stubbed, no implementation");
	return LinphoneTunnelModeDisable;
}

void linphone_tunnel_enable_dual_mode(BCTBX_UNUSED(LinphoneTunnel *tunnel), BCTBX_UNUSED(bool_t dual_mode_enabled)) {
	ms_warning("linphone_tunnel_set_dual_mode() - stubbed, no implementation");
}

bool_t linphone_tunnel_dual_mode_enabled(BCTBX_UNUSED(const LinphoneTunnel *tunnel)) {
	ms_warning("linphone_tunnel_get_dual_mode() - stubbed, no implementation");
	return FALSE;
}

bool_t linphone_tunnel_connected(BCTBX_UNUSED(const LinphoneTunnel *tunnel)) {
	return FALSE;
}

bool_t linphone_tunnel_get_activated(BCTBX_UNUSED(const LinphoneTunnel *tunnel)) {
	return FALSE;
}

void linphone_tunnel_set_http_proxy_auth_info(BCTBX_UNUSED(LinphoneTunnel *tunnel),
                                              BCTBX_UNUSED(const char *username),
                                              BCTBX_UNUSED(const char *passwd)) {
}

void linphone_tunnel_set_http_proxy(BCTBX_UNUSED(LinphoneTunnel *tunnel),
                                    BCTBX_UNUSED(const char *host),
                                    BCTBX_UNUSED(int port),
                                    BCTBX_UNUSED(const char *username),
                                    BCTBX_UNUSED(const char *passwd)) {
}

void linphone_tunnel_get_http_proxy(BCTBX_UNUSED(LinphoneTunnel *tunnel),
                                    BCTBX_UNUSED(const char **host),
                                    BCTBX_UNUSED(int *port),
                                    BCTBX_UNUSED(const char **username),
                                    BCTBX_UNUSED(const char **passwd)) {
}

void linphone_tunnel_reconnect(BCTBX_UNUSED(LinphoneTunnel *tunnel)) {
}

void linphone_tunnel_configure(BCTBX_UNUSED(LinphoneTunnel *tunnel)) {
	ms_warning("linphone_tunnel_configure() - stubbed, no implementation");
}

void linphone_tunnel_enable_sip(BCTBX_UNUSED(LinphoneTunnel *tunnel), BCTBX_UNUSED(bool_t enable)) {
	ms_warning("linphone_tunnel_enable_sip() - stubbed, no implementation");
}
bool_t linphone_tunnel_sip_enabled(BCTBX_UNUSED(const LinphoneTunnel *tunnel)) {
	return FALSE;
}

/* Deprecated functions */
void linphone_tunnel_enable(BCTBX_UNUSED(LinphoneTunnel *tunnel), BCTBX_UNUSED(bool_t enabled)) {
}
bool_t linphone_tunnel_enabled(BCTBX_UNUSED(const LinphoneTunnel *tunnel)) {
	return FALSE;
}
void linphone_tunnel_auto_detect(BCTBX_UNUSED(LinphoneTunnel *tunnel)) {
}
bool_t linphone_tunnel_auto_detect_enabled(BCTBX_UNUSED(LinphoneTunnel *tunnel)) {
	return FALSE;
}
void linphone_tunnel_simulate_udp_loss(BCTBX_UNUSED(LinphoneTunnel *tunnel), BCTBX_UNUSED(bool_t enabled)) {
}

void linphone_tunnel_set_username(BCTBX_UNUSED(LinphoneTunnel *tunnel), BCTBX_UNUSED(const char *username)) {
}
const char *linphone_tunnel_get_username(BCTBX_UNUSED(LinphoneTunnel *tunnel)) {
	return NULL;
}
void linphone_tunnel_set_domain(BCTBX_UNUSED(LinphoneTunnel *tunnel), BCTBX_UNUSED(const char *domain)) {
}
const char *linphone_tunnel_get_domain(BCTBX_UNUSED(LinphoneTunnel *tunnel)) {
	return NULL;
}

bool_t linphone_tunnel_is_tunnel_rtp_transport(BCTBX_UNUSED(const LinphoneTunnel *tunnel),
                                               BCTBX_UNUSED(const RtpTransport *tp)) {
	return FALSE;
}
