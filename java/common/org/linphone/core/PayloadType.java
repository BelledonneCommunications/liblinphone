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

public interface PayloadType {
	/**
	 * Obtain the registered mime-type (actually submime) of the PayloadType. For example: "H264", "speex"...
	 * @return the (sub) mime type.
	 */
	String getMime();
	
	/**
	 * Return the RTP clockrate. It is usually the same as the audio sampling rate, and 90000 for video payload types.
	 * @return
	 */
	int getRate();
	
	/**
	 * Set format parameter string wished for incoming stream. It is advertised in SDP.
	 * @param fmtp the fmtp string, like "octet-align=1;mode-set=4,5,6,7"
	 */
	void setRecvFmtp(String fmtp);
	
	/**
	 * Return the format parameters wished for incoming stream.
	 * @return the format parameter string.
	 */
	String getRecvFmtp();
	
	/**
	 * Set the format parameter effective for the outgoing stream (unusual).
	 * @param fmtp
	 */
	void setSendFmtp(String fmtp);
	
	/**
	 * Return the format parameter effective for the outgoing stream.
	 * @return
	 */
	String getSendFmtp();
}
