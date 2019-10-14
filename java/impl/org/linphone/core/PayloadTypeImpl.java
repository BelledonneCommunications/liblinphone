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

class PayloadTypeImpl implements PayloadType {

	public final long nativePtr;
	
	private native String toString(long ptr);
	private native String getMime(long ptr);
	private native int getRate(long ptr);

	protected PayloadTypeImpl(long aNativePtr)  {
		nativePtr = aNativePtr;
	}
	
	public int getRate() {
		return getRate(nativePtr);
	}

	public String getMime() {
		return getMime(nativePtr);
	}
	
	public String toString() {
		return toString(nativePtr);
	}
	
	private native void setRecvFmtp(long ptr, String fmtp);
	@Override
	public void setRecvFmtp(String fmtp) {
		setRecvFmtp(nativePtr,fmtp);
	}
	private native String getRecvFmtp(long ptr);
	@Override
	public String getRecvFmtp() {
		return getRecvFmtp(nativePtr);
	}
	
	private native void setSendFmtp(long ptr, String fmtp);
	@Override
	public void setSendFmtp(String fmtp) {
		setSendFmtp(nativePtr,fmtp);
	}
	private native String getSendFmtp(long ptr);
	@Override
	public String getSendFmtp() {
		return getSendFmtp(nativePtr);
	}
}
