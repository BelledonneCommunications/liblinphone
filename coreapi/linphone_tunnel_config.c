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

#include "linphone/tunnel.h"

#include "c-wrapper/c-wrapper.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

struct _LinphoneTunnelConfig {
	belle_sip_object_t base;
	char *host;
	int port;
	int remote_udp_mirror_port;
	int delay;
	void *user_data;
	char *host2;
	int port2;
};

LinphoneTunnelConfig *linphone_tunnel_config_new() {
	LinphoneTunnelConfig *ltc = belle_sip_object_new(LinphoneTunnelConfig);
	ltc->remote_udp_mirror_port = 12345;
	ltc->delay = 1000;
	ltc->port = 443;
	ltc->host2 = NULL;
	ltc->port2 = 0;
	return ltc;
}

void linphone_tunnel_config_set_host(LinphoneTunnelConfig *tunnel, const char *host) {
	if(tunnel->host != NULL) {
		ms_free(tunnel->host);
		tunnel->host = NULL;
	}
	if(host != NULL && strlen(host)) {
		tunnel->host = ms_strdup(host);
	}
}

const char *linphone_tunnel_config_get_host(const LinphoneTunnelConfig *tunnel) {
	return tunnel->host;
}

void linphone_tunnel_config_set_port(LinphoneTunnelConfig *tunnel, int port) {
	tunnel->port = port;
}

int linphone_tunnel_config_get_port(const LinphoneTunnelConfig *tunnel) {
	return tunnel->port;
}

void linphone_tunnel_config_set_host2(LinphoneTunnelConfig *tunnel, const char *host) {
	if(tunnel->host2 != NULL) {
		ms_free(tunnel->host2);
		tunnel->host2 = NULL;
	}
	if(host != NULL && strlen(host)) {
		tunnel->host2 = ms_strdup(host);
	}
}

const char *linphone_tunnel_config_get_host2(const LinphoneTunnelConfig *tunnel) {
	return tunnel->host2;
}

void linphone_tunnel_config_set_port2(LinphoneTunnelConfig *tunnel, int port) {
	tunnel->port2 = port;
}

int linphone_tunnel_config_get_port2(const LinphoneTunnelConfig *tunnel) {
	return tunnel->port2;
}

void linphone_tunnel_config_set_remote_udp_mirror_port(LinphoneTunnelConfig *tunnel, int remote_udp_mirror_port) {
	tunnel->remote_udp_mirror_port = remote_udp_mirror_port;
}

int linphone_tunnel_config_get_remote_udp_mirror_port(const LinphoneTunnelConfig *tunnel) {
	return tunnel->remote_udp_mirror_port;
}

void linphone_tunnel_config_set_delay(LinphoneTunnelConfig *tunnel, int delay) {
	tunnel->delay = delay;
}

int linphone_tunnel_config_get_delay(const LinphoneTunnelConfig *tunnel) {
	return tunnel->delay;
}

static void _linphone_tunnel_config_destroy(LinphoneTunnelConfig *tunnel) {
	if (tunnel->host) ms_free(tunnel->host);
	if (tunnel->host2) ms_free(tunnel->host2);
}

LinphoneTunnelConfig * linphone_tunnel_config_ref(LinphoneTunnelConfig *cfg){
	return (LinphoneTunnelConfig*)belle_sip_object_ref(cfg);
}

void linphone_tunnel_config_unref(LinphoneTunnelConfig *cfg){
	belle_sip_object_unref(cfg);
}

void linphone_tunnel_config_set_user_data(LinphoneTunnelConfig *cfg, void *ud){
	cfg->user_data = ud;
}

void *linphone_tunnel_config_get_user_data(LinphoneTunnelConfig *cfg){
	return cfg->user_data;
}

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneTunnelConfig);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneTunnelConfig, belle_sip_object_t,
	(belle_sip_object_destroy_t)_linphone_tunnel_config_destroy,
	NULL, // clone
	NULL, // marshal
	FALSE
);
