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
package org.linphone.core;

/**
 * The TunnelConfig interface allows to configure information about a tunnel server (voip anti blocking).
 * @author smorlat
 *
 */
public interface TunnelConfig {
	/**
	 * Get the hostname of the tunnel server
	 * @return
	 */
	String getHost();
	/**
	 * Set the hostname (or ip address) of the tunnel server.
	 * @param host
	 */
	void setHost(String host);
	/**
	 * Get the port where to connect.
	 * @return
	 */
	int getPort();
	/**
	 * Set the port where to connect to the tunnel server.
	 * When not set, the default value is used (443).
	 * @param port
	 */
	void setPort(int port);
	/**
	 * Get the hostname of the second tunnel server
	 * @return
	 */
	String getHost2();
	/**
	 * Set the hostname (or ip address) of the second tunnel server.
	 * @param host
	 */
	void setHost2(String host);
	/**
	 * Get the port where to connect to the second tunnel server.
	 * @return
	 */
	int getPort2();
	/**
	 * Set the port where to connect to the second tunnel server.
	 * When not set, the default value is used (443).
	 * @param port
	 */
	void setPort2(int port);
	/**
	 * Get the remote udp mirror port, which is used to check udp connectivity of the network.
	 * @return
	 */
	int getRemoteUdpMirrorPort();
	/**
	 * Set the udp mirror port, which is used to check udp connectivity.
	 * When not set, a default value of 12345 is used.
	 * @param remoteUdpMirrorPort
	 */
	void setRemoteUdpMirrorPort(int remoteUdpMirrorPort);
	/**
	 * Get the maximum amount of time for waiting for UDP packets to come back during 
	 * the UDP connectivity check, in milliseconds.
	 * 
	 * @return
	 */
	int getDelay();
	/**
	 * Set the maximum amount of time for waiting for UDP packets to come back during 
	 * the UDP connectivity check, in milliseconds.
	 */
	void setDelay(int delay);
}
