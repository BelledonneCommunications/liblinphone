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


class LinphoneCallLogImpl implements LinphoneCallLog {

	protected final long nativePtr;
	
	private native long getFrom(long nativePtr);
	private native long getTo(long nativePtr);
	private native boolean isIncoming(long nativePtr);
	private native int getStatus(long nativePtr);
	private native String getStartDate(long nativePtr);
	private native int getCallDuration(long nativePtr);
	private native String getCallId(long nativePtr);
	private native long getTimestamp(long nativePtr);
	private native boolean wasConference(long nativePtr);
	
	LinphoneCallLogImpl(long aNativePtr)  {
		nativePtr = aNativePtr;
	}
	
	public CallDirection getDirection() {
		return isIncoming(nativePtr)?CallDirection.Incoming:CallDirection.Outgoing;
	}

	public LinphoneAddress getFrom() {
		return new LinphoneAddressImpl(getFrom(nativePtr),LinphoneAddressImpl.WrapMode.FromExisting);
	}

	public LinphoneAddress getTo() {
		return new LinphoneAddressImpl(getTo(nativePtr),LinphoneAddressImpl.WrapMode.FromExisting);
	}
	public CallStatus getStatus() {
		return LinphoneCallLog.CallStatus.fromInt(getStatus(nativePtr));
	}

	public long getNativePtr() {
		return nativePtr;
	}
	
	public String getStartDate() {
		return getStartDate(nativePtr);
	}

	public int getCallDuration() {
		return getCallDuration(nativePtr);
	}
	public String getCallId() {
		return getCallId(nativePtr);
	}
	
	public long getTimestamp() {
		return getTimestamp(nativePtr) * 1000;  // Need milliseconds, not seconds
	}
	public boolean wasConference() {
		return wasConference(nativePtr);
	}
}
