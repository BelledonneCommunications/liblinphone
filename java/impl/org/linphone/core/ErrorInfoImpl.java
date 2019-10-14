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

public class ErrorInfoImpl implements ErrorInfo {
	protected long mNativePtr;
	
	private native void ref(long nativePtr);
	private native void unref(long nativePtr);
	
	private native int getReason(long nativePtr);
	private native String getProtocol(long nativePtr);
	private native int getProtocolCode(long nativePtr);
	private native String getPhrase(long nativePtr);
	private native String getWarnings(long nativePtr);
	private native long getSubErrorInfo(long nativePtr);
	
	private native void setReason(long nativePtr, int reason);
	private native void setProtocol(long nativePtr, String proto);
	private native void setProtocolCode(long nativePtr, int code);
	private native void setPhrase(long nativePtr, String phrase);
	private native void setWarnings(long nativePtr, String warnings);
	private native void setSubErrorInfo(long nativePtr, long other_nativePtr);
	
	public ErrorInfoImpl(long nativePtr, boolean ownsRef){
		if (!ownsRef) ref(nativePtr);
		mNativePtr = nativePtr;
	}
	
	public ErrorInfoImpl(long nativePtr){
		ref(nativePtr);
		mNativePtr = nativePtr;
	}
	
	@Override
	public Reason getReason() {
		return Reason.fromInt(getReason(mNativePtr));
	}
	
	@Override
	public String getProtocol() {
		return getProtocol(mNativePtr);
	}

	@Override
	public int getProtocolCode() {
		return getProtocolCode(mNativePtr);
	}

	@Override
	public String getPhrase() {
		return getPhrase(mNativePtr);
	}
	
	@Override
	public String getWarnings(){
		return getWarnings(mNativePtr);
	}
	
	@Override
	public ErrorInfo getSubErrorInfo(){
		long sub_ei = getSubErrorInfo(mNativePtr);
		return sub_ei != 0 ? new ErrorInfoImpl(sub_ei, false) : null;
	}

	
	@Override
	public void setReason(Reason reason) {
		setReason(mNativePtr, reason.mValue); 
	}
	
	@Override
	public void setProtocol(String proto) {
		setProtocol(mNativePtr, proto);
	}

	@Override
	public void setProtocolCode(int code) {
		setProtocolCode(mNativePtr, code);
	}

	@Override
	public void setPhrase(String phrase) {
		setPhrase(mNativePtr, phrase);
	}
	
	@Override
	public void setWarnings(String warnings){
		setWarnings(mNativePtr, warnings);
	}
	
	@Override
	public void setSubErrorInfo(ErrorInfo ei){
		setSubErrorInfo(mNativePtr, ei != null ? ((ErrorInfoImpl)ei).mNativePtr : 0);
	}
	
	
	@Override
	public String getDetails() {
		return getWarnings();
	}

	protected void finalize() throws Throwable {
		if (mNativePtr != 0){
			unref(mNativePtr);
			mNativePtr = 0;
		}
	}
}
