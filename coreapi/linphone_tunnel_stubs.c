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

#include "linphone/tunnel.h"
#include "linphone/core.h"
#include "private.h"
#include "linphone/lpconfig.h"

#include "tester_utils.h" // for linphone_tunnel_is_tunnel_rtp_transport() declaration

LinphoneTunnel* linphone_core_get_tunnel(const LinphoneCore *lc){
	return lc->tunnel;
}

/*stubs to avoid to have #ifdef TUNNEL_ENABLED in upper layers*/

void linphone_tunnel_destroy(UNUSED(LinphoneTunnel *tunnel)){
}

LinphoneTunnel *linphone_tunnel_ref(LinphoneTunnel *tunnel) {
	ms_warning("linphone_tunnel_ref() - stubbed, no implementation");
	return tunnel;
}

void linphone_tunnel_unref(UNUSED(LinphoneTunnel *tunnel)) {
	ms_warning("linphone_tunnel_unref() - stubbed, no implementation");
}

void linphone_tunnel_add_server(UNUSED(LinphoneTunnel *tunnel), UNUSED(LinphoneTunnelConfig *tunnel_config)){
	ms_warning("linphone_tunnel_add_server() - stubbed, no implementation");
}

void linphone_tunnel_remove_server(UNUSED(LinphoneTunnel *tunnel), UNUSED(LinphoneTunnelConfig *tunnel_config)){
	ms_warning("linphone_tunnel_remove_server() - stubbed, no implementation");
}

const MSList *linphone_tunnel_get_servers(UNUSED(const LinphoneTunnel *tunnel)){
	ms_warning("linphone_tunnel_get_servers() - stubbed, no implementation");
	return NULL;
}

void linphone_tunnel_clean_servers(UNUSED(LinphoneTunnel *tunnel)){
	ms_warning("linphone_tunnel_clean_servers() - stubbed, no implementation");
}

void linphone_tunnel_set_mode(UNUSED(LinphoneTunnel *tunnel), UNUSED(LinphoneTunnelMode mode)) {
	ms_warning("linphone_tunnel_set_mode() - stubbed, no implementation");
}

LinphoneTunnelMode linphone_tunnel_get_mode(UNUSED(const LinphoneTunnel *tunnel)){
	ms_warning("linphone_tunnel_get_mode() - stubbed, no implementation");
	return LinphoneTunnelModeDisable;
}

void linphone_tunnel_enable_dual_mode(UNUSED(LinphoneTunnel *tunnel), UNUSED(bool_t dual_mode_enabled)) {
	ms_warning("linphone_tunnel_set_dual_mode() - stubbed, no implementation");
}

bool_t linphone_tunnel_dual_mode_enabled(UNUSED(const LinphoneTunnel *tunnel)) {
	ms_warning("linphone_tunnel_get_dual_mode() - stubbed, no implementation");
	return FALSE;
}

bool_t linphone_tunnel_connected(UNUSED(const LinphoneTunnel *tunnel)){
	return FALSE;
}

bool_t linphone_tunnel_get_activated(UNUSED(const LinphoneTunnel *tunnel)){
	return FALSE;
}

void linphone_tunnel_set_http_proxy_auth_info(UNUSED(LinphoneTunnel *tunnel), UNUSED(const char* username),UNUSED(const char* passwd)){
}

void linphone_tunnel_set_http_proxy(UNUSED(LinphoneTunnel*tunnel), UNUSED(const char *host), UNUSED(int port), UNUSED(const char* username),UNUSED(const char* passwd)){
}

void linphone_tunnel_get_http_proxy(UNUSED(LinphoneTunnel*tunnel),UNUSED(const char **host), UNUSED(int *port), UNUSED(const char **username), UNUSED(const char **passwd)){
}

void linphone_tunnel_reconnect(UNUSED(LinphoneTunnel *tunnel)){
}

void linphone_tunnel_configure(UNUSED(LinphoneTunnel *tunnel)){
	ms_warning("linphone_tunnel_configure() - stubbed, no implementation");
}

void linphone_tunnel_enable_sip(UNUSED(LinphoneTunnel *tunnel), UNUSED(bool_t enable)) {
	ms_warning("linphone_tunnel_enable_sip() - stubbed, no implementation");
}
bool_t linphone_tunnel_sip_enabled(UNUSED(const LinphoneTunnel *tunnel)) { return FALSE; }

/* Deprecated functions */
void linphone_tunnel_enable(UNUSED(LinphoneTunnel *tunnel), UNUSED(bool_t enabled)) {}
bool_t linphone_tunnel_enabled(UNUSED(const LinphoneTunnel *tunnel)) { return FALSE; }
void linphone_tunnel_auto_detect(UNUSED(LinphoneTunnel *tunnel)) {}
bool_t linphone_tunnel_auto_detect_enabled(UNUSED(LinphoneTunnel *tunnel)) { return FALSE; }
void linphone_tunnel_simulate_udp_loss(UNUSED(LinphoneTunnel *tunnel), UNUSED(bool_t enabled)) {}


void linphone_tunnel_set_username(UNUSED(LinphoneTunnel *tunnel), UNUSED(const char *username)) {}
const char *linphone_tunnel_get_username(UNUSED(LinphoneTunnel *tunnel)) { return NULL; }
void linphone_tunnel_set_domain(UNUSED(LinphoneTunnel *tunnel), UNUSED(const char *domain)) {}
const char *linphone_tunnel_get_domain(UNUSED(LinphoneTunnel *tunnel)) { return NULL; }

bool_t linphone_tunnel_is_tunnel_rtp_transport(UNUSED(const LinphoneTunnel *tunnel), UNUSED(const RtpTransport *tp)){
	return FALSE;
}

